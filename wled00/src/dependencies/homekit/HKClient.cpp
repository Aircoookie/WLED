#include "HKClient.h"

#ifdef ARDUINO_ARCH_ESP32

#include <mdns.h>
#include <mbedtls/base64.h>
#include <mbedtls/sha512.h>
#include <sodium/crypto_aead_chacha20poly1305.h>

#include "HKDF.h"
#include "HTTPUtils.h"

#define HK_MAX_ENCRYPTION_SIZE 1024
#define HK_AAD_SIZE 2

HKClient::HKClient() {}

void HKClient::set_client(WiFiClient new_client, int index) {
    disconnect_client();
    client = new_client;
    client_index = index;
}

bool HKClient::client_connected() {
    return client;
}

void HKClient::disconnect_client() {
    client.stop();
    secured_pairing = NULL;
    decryption_count = 0;
    encryption_count = 0;
}

bool HKClient::session_secured() {
    return secured_pairing != NULL;
}

bool HKClient::session_verified() {
    return secured_pairing->allocated;
}

bool HKClient::payload_available() {
    return client && client.available();
}

//////////////////////
//  Handle new data //
//////////////////////

void HKClient::process_request() {
    if (!client || !client.available()) {
        return;
    }

    EHK_DEBUGF("\n\nNew request from: %s\n", client.remoteIP().toString().c_str());

    int buf_len = client.available();
    char http_buf[buf_len + 1];
    int http_len = 0;

    if (secured_pairing) {
        char encrypted[buf_len];
        client.readBytes(encrypted, buf_len);
        http_len = decrypt_message(http_buf, encrypted, buf_len);
    } else {
        http_len = client.readBytes(http_buf, buf_len);
    }

    if (!http_len) {
        EHK_DEBUG("HTTP Buffer is null/empty.");
        send_bad_request();
        return;
    }

    http_buf[http_len] = '\0';

    HKUri path = parse_path_request(http_buf);
    HTTPMethod method = parse_method_request(http_buf);

    if (path == HKUri::UNKNOWN || method == HTTPMethod::UNKNOWN) {
        EHK_DEBUGLN("Request does not have path or method.");
        send_bad_request();
        return;
    }

    EHK_DEBUGF("URI: %s\n", HKUri_to_str(path));
    EHK_DEBUGF("Method: %s\n", HTTPMethod_to_str(method));

    if (method == HTTPMethod::POST || method == HTTPMethod::PUT) {
        size_t http_body_len = parse_content_len(http_buf, http_len);
        char http_body[http_body_len + 1];
        int parse_failed = parse_body_request(http_body, http_buf, http_body_len);
        if (parse_failed) {
            send_bad_request();
            return;
        }

        switch (path) {
            case HKUri::PAIR_SETUP:
                post_pair_setup_request((const uint8_t *) http_body, http_body_len);
                break;
            case HKUri::PAIR_VERIFY:
                post_pair_verify_request((const uint8_t *) http_body, http_body_len);
                break;
            case HKUri::PAIRINGS:
                post_pairings_request((const uint8_t *) http_body, http_body_len);
                break;
            case HKUri::CHARACTERISTICS:
                put_characteristics_request(http_body, http_body_len);
                break;
            default:
                send_not_found_request();
                break;
        };
    } else if (method == HTTPMethod::GET) {
        switch (path) {
            case HKUri::ACCESSORIES:
                get_accessories_request();
                break;   
            case HKUri::CHARACTERISTICS:
            {
                int ids_count = parse_characteristics_query_count(http_buf);
                if (ids_count == -1) {
                    send_bad_request();
                    return;
                }
                int ids[ids_count];
                if (parse_characteristics_query(ids, ids_count, http_buf)) {
                    send_bad_request();
                    return;
                }
                get_characteristics(ids, ids_count);
                break;
            }
            default:
                send_not_found_request();
                break;
        }
    }
}

//////////////////////////
//  Web server handlers //
//////////////////////////

// Post Pair-Setup

void HKClient::post_pair_setup_request(const uint8_t *data, size_t len) { 
    auto& hk_store = HKStore::get_instance();

    TLV tlv8;
    int unpacked = tlv8.deserialize(data, len);
    if (!unpacked) {
        EHK_DEBUGLN("THERE WAS AN ERROR UNPACKING TLV!!!!");
        send_bad_request();
        return;
    }

    int tlv_state = tlv8.val(kTLVType_State);
    if(tlv_state == -1) {
        EHK_DEBUGLN("*** ERROR: Missing <M#> State TLV");
        send_bad_request();
        return;
    }

    if (hk_store.has_admin_pairings()) {
        EHK_DEBUG("\n*** ERROR: Device already paired!\n\n");
        tlv8.clear();                                         // clear TLV records
        tlv8.val(kTLVType_State, tlv_state + 1);                  // set response STATE to requested state+1 (which should match the state that was expected by the controller)
        tlv8.val(kTLVType_Error, tagError_Unavailable);       // set Error=Unavailable
        send_tlv_response(&tlv8);                                       // send response to client
        return;
    }

    EHK_DEBUGF("Found <M%d>.  Expected <M%d>\n", tlv_state, hk_store.get_pair_status());

    if(tlv_state != hk_store.get_pair_status()) {               // error: Device is not yet paired, but out-of-sequence pair-setup STATE was received
        Serial.print("\n*** ERROR: Out-of-Sequence Pair-Setup request!\n\n");
        tlv8.clear();                                           // clear TLV records
        tlv8.val(kTLVType_State, tlv_state + 1);                // set response STATE to requested state+1 (which should match the state that was expected by the controller)
        tlv8.val(kTLVType_Error, tagError_Unknown);             // set Error=Unknown (there is no specific error type for out-of-sequence steps)
        hk_store.set_pair_status(pairState_M1);                 // reset pairStatus to first step of unpaired accessory (M1)
        send_tlv_response(&tlv8);                               // send response to client
        return;
    };

    switch(tlv_state) {                                             // valid and in-sequence Pair-Setup STATE received -- process request!  (HAP Section 5.6)
        case pairState_M1:                                          // 'SRP Start Request' 
        {
            int method = tlv8.val(kTLVType_Method);
            tlv8.clear();                                           // clear TLV records
            tlv8.val(kTLVType_State, pairState_M2);                 // set State=<M2>
            if(method != 0) {                                       // error: "Pair Setup" method must always be 0 to indicate setup without MiFi Authentification (HAP Table 5-3)
                EHK_DEBUG("\n*** ERROR: Pair Method not 0.\n\n");
                tlv8.val(kTLVType_Error, tagError_Unknown);                 // set Error = Unavailable
            } else {
                hk_store.srp.generate_public_key();                         // create accessory public key from Pair-Setup code
                hk_store.srp.load_tlv(&tlv8, kTLVType_PublicKey);           // load server public key, B
                hk_store.srp.load_tlv(&tlv8, kTLVType_Salt);                // load salt, s
                hk_store.set_pair_status(pairState_M3);
            }

            send_tlv_response(&tlv8);                                       // send response to client
            break;
        }
        case pairState_M3:                                                      // 'SRP Verify Request'
        {
            if(!hk_store.srp.write_tlv(&tlv8, kTLVType_PublicKey) || !hk_store.srp.write_tlv(&tlv8, kTLVType_Proof)) {  // try to write ios Public key and Proof into SRP
                EHK_DEBUG("\n*** ERROR: One or both of the required 'PublicKey' and 'Proof' TLV records for this step is bad or missing\n\n");
                tlv8.clear();                                                   // clear TLV records
                tlv8.val(kTLVType_State, pairState_M4);                         // set State=<M4>
                tlv8.val(kTLVType_Error, tagError_Unknown);                     // set Error=Unknown (there is no specific error type for missing/bad TLV data)
                hk_store.set_pair_status(pairState_M1);
                send_tlv_response(&tlv8);                                       // send response to client
                return;
            }

            tlv8.clear();                                                   // clear TLV records
            tlv8.val(kTLVType_State, pairState_M4);                         // set State = <M4>

            hk_store.srp.compute_key();                                         // compute shared session (secret) key.

            if(!hk_store.srp.verify_proof()) {                                  // verify client proof
                EHK_DEBUG("\n*** ERROR: SRP Proof Verification Failed\n\n");
                tlv8.val(kTLVType_Error, tagError_Authentication);              // set Error=Authentication
                hk_store.set_pair_status(pairState_M1);
            } else {
                hk_store.srp.generate_proof();                              // M1 has been successully verified; now create accessory proof M2
                hk_store.srp.load_tlv(&tlv8, kTLVType_Proof);               // load M2 counter-proof
                hk_store.set_pair_status(pairState_M5);                     // set next expected pair-state request from client                
            }
            send_tlv_response(&tlv8);                                                  // send response to client
            break;
        }

        case pairState_M5:                                                  // 'Exchange Request'
        {
            if(!tlv8.buf(kTLVType_EncryptedData)) {            
                EHK_DEBUG("\n*** ERROR: Required 'EncryptedData' TLV record for this step is bad or missing\n\n");
                tlv8.clear();                                           // clear TLV records
                tlv8.val(kTLVType_State, pairState_M6);                  // set State = <M6>
                tlv8.val(kTLVType_Error, tagError_Unknown);              // set Error = Unknown (there is no specific error type for missing/bad TLV data)
                hk_store.set_pair_status(pairState_M1);
                send_tlv_response(&tlv8);                                                  // send response to client
                return;
            };

            // 1.) get srp shared secret key
            hk_store.srp.get_shared_secret(ps_keys.srp_shared_secret);

            // 2.) use the shared secret key to create a shared session
            create_hkdf(ps_keys.session_key, ps_keys.srp_shared_secret, 64, "Pair-Setup-Encrypt-Salt", "Pair-Setup-Encrypt-Info");

            // Validate iOS's device's authTag by extracting it from encryptedData, last 16 bytes

            uint8_t decrypted_data[1024];
            unsigned long long decryption_len;

            // use generated hkdf session_key to decrypt encryptedData TLV with padded nonce="PS-Msg05"

            int decryption_ret = crypto_aead_chacha20poly1305_ietf_decrypt(
                            decrypted_data, &decryption_len, NULL,
                            tlv8.buf(kTLVType_EncryptedData), tlv8.len(kTLVType_EncryptedData), NULL, 0,
                            (const unsigned char *) HK_CHACHA_SETUP_MSG5, ps_keys.session_key);

            if (decryption_ret != 0) {
                EHK_DEBUGF("\n*** ERROR: Exchange-Request Authentication Failed: code: %d\n\n", decryption_ret);
                tlv8.clear();                                                           // clear TLV records
                tlv8.val(kTLVType_State, pairState_M6);                                 // set State = <M6>
                tlv8.val(kTLVType_Error, tagError_Authentication);                      // set Error = Authentication
                hk_store.set_pair_status(pairState_M1);
                send_tlv_response(&tlv8);                                               // send response to client
                return;        
            }

            if (!tlv8.deserialize(decrypted_data, decryption_len)) {
                EHK_DEBUG("\n*** ERROR: Can't parse decrypted data into separate TLV records\n\n");
                tlv8.clear();                                         // clear TLV records
                tlv8.val(kTLVType_State, pairState_M6);                // set State=<M6>
                tlv8.val(kTLVType_Error, tagError_Unknown);           // set Error=Unknown (there is no specific error type for missing/bad TLV data)
                hk_store.set_pair_status(pairState_M1);
                send_tlv_response(&tlv8);                                       // send response to client
                return;
            }

            if(!tlv8.buf(kTLVType_Identifier) || !tlv8.buf(kTLVType_PublicKey) || !tlv8.buf(kTLVType_Signature)){            
                EHK_DEBUG("\n*** ERROR: One or more of required 'Identifier,' 'PublicKey,' and 'Signature' TLV records for this step is bad or missing\n\n");
                tlv8.clear();                                         // clear TLV records
                tlv8.val(kTLVType_State, pairState_M6);                // set State=<M6>
                tlv8.val(kTLVType_Error, tagError_Unknown);           // set Error=Unknown (there is no specific error type for missing/bad TLV data)
                hk_store.set_pair_status(pairState_M1);
                send_tlv_response(&tlv8);                                       // send response to client
                return;
            };

            // Derive iOSDeviceX from srp shared secret key.   
            // 5.6.6.1 step 3
            size_t ios_device_x_len = 32;
            uint8_t ios_device_x[ios_device_x_len];
            create_hkdf(ios_device_x, ps_keys.srp_shared_secret, 64, "Pair-Setup-Controller-Sign-Salt", "Pair-Setup-Controller-Sign-Info");

            int ios_dev_pair_id_len = tlv8.len(kTLVType_Identifier);
            char * ios_dev_pair_id = (char *) tlv8.buf(kTLVType_Identifier);

            int ios_dev_ltpk_len = tlv8.len(kTLVType_PublicKey);
            uint8_t * ios_device_ltpk = tlv8.buf(kTLVType_PublicKey);

            /* Construct iOSDeviceInfo by concatenating
            * iOSDeviceX
            * iOSDevicePairingID (ctrl_id)
            * iOSDeviceLTPK (ltpkc)
            */
            size_t ios_dev_info_len = ios_device_x_len + ios_dev_pair_id_len + ios_dev_ltpk_len;
            uint8_t ios_device_info[ios_dev_info_len];

            memcpy(ios_device_info, ios_device_x, ios_device_x_len);
            memcpy(ios_device_info + ios_device_x_len, ios_dev_pair_id, ios_dev_pair_id_len);
            memcpy(ios_device_info + ios_device_x_len + ios_dev_pair_id_len, ios_device_ltpk, ios_dev_ltpk_len);

            uint8_t * ios_device_sig = tlv8.buf(kTLVType_Signature);
            int verify_sig_ret = crypto_sign_ed25519_verify_detached(ios_device_sig, ios_device_info, ios_dev_info_len, ios_device_ltpk);
            if (verify_sig_ret != 0) {
                EHK_DEBUG("\n*** ERROR: LTPK Signature Verification Failed\n\n");
                tlv8.clear();                                         // clear TLV records
                tlv8.val(kTLVType_State, pairState_M6);                // set State=<M6>
                tlv8.val(kTLVType_Error, tagError_Authentication);    // set Error=Authentication
                hk_store.set_pair_status(pairState_M1);
                send_tlv_response(&tlv8);                                       // send response to client
                return;                
            }

            if(!hk_store.add_pairing(ios_dev_pair_id, ios_dev_pair_id_len, ios_device_ltpk, true)) {
                tlv8.clear();                                         // clear TLV records
                tlv8.val(kTLVType_State, pairState_M6);                // set State=<M6>
                tlv8.val(kTLVType_Error, tagError_MaxPeers);    // set Error=Authentication
                hk_store.set_pair_status(pairState_M1);
                send_tlv_response(&tlv8);                                       // send response to client
            }

            // Generate M6 response

            size_t accessory_x_len = 32;
            uint8_t accessory_x[accessory_x_len];
            create_hkdf(accessory_x, ps_keys.srp_shared_secret, 64, "Pair-Setup-Accessory-Sign-Salt", "Pair-Setup-Accessory-Sign-Info");       // derive accessoryX from SRP Shared Secret using HKDF 

            char acc_pair_id[HK_ACCESSORY_ID_STR_SIZE];
            hk_store.get_serialized_accessory_id(acc_pair_id);
            size_t acc_pair_id_len = HK_ACCESSORY_ID_STR_SIZE - 1;

            size_t acc_ltpk_len = crypto_sign_ed25519_PUBLICKEYBYTES;
            accessory_keys acc_keys;
            if (!hk_store.get_accessory_keys(&acc_keys)) {
                EHK_DEBUGLN("\n\nError: could not get accessory keys for pair-setup m5 response.\n");
                tlv8.clear();                                         // clear TLV records
                tlv8.val(kTLVType_State, pairState_M6);                // set State=<M6>
                tlv8.val(kTLVType_Error, tagError_Unknown);    // set Error=Authentication
                hk_store.set_pair_status(pairState_M1);
                send_tlv_response(&tlv8);                                       // send response to client
                return;
            }

            // Construct AccessoryInfo
            // - AccessoryX 32
            // - AccessoryPairingID 
            // - AccessoryLTPK 

            size_t accessory_info_len = accessory_x_len + acc_pair_id_len + acc_ltpk_len;
            uint8_t accessory_info[accessory_info_len];

            memcpy(accessory_info, accessory_x, accessory_x_len);
            memcpy(accessory_info + accessory_x_len, acc_pair_id, acc_pair_id_len);
            memcpy(accessory_info + accessory_x_len + acc_pair_id_len, acc_keys.ltpk, acc_ltpk_len);

            tlv8.clear();       // clear existing TLV records

            int sig_ret_val = crypto_sign_ed25519_detached(tlv8.buf(kTLVType_Signature, crypto_sign_ed25519_BYTES), NULL, accessory_info, accessory_info_len, acc_keys.ltsk); // produce signature of accessoryInfo using AccessoryLTSK (Ed25519 long-term secret key)

            if (sig_ret_val != 0) {
                EHK_DEBUGLN("\n\nCould not sign acccessory_info\n\n");
                tlv8.clear();                                         // clear TLV records
                tlv8.val(kTLVType_State, pairState_M6);                // set State=<M6>
                tlv8.val(kTLVType_Error, tagError_Unknown);    // set Error=Authentication
                hk_store.set_pair_status(pairState_M1);
                send_tlv_response(&tlv8);                                       // send response to client
                return;
            }

            memcpy(tlv8.buf(kTLVType_Identifier, acc_pair_id_len), acc_pair_id, acc_pair_id_len);            // set Identifier TLV record as accessoryPairingID
            memcpy(tlv8.buf(kTLVType_PublicKey, acc_ltpk_len), acc_keys.ltpk, acc_ltpk_len);                                      // set PublicKey TLV record as accessoryLTPK

            // tlv8.print();

            size_t sub_tlv_len = tlv8.serialize(NULL);                 // get size of buffer needed to store sub-TLV 
            uint8_t sub_tlv[sub_tlv_len];
            tlv8.serialize(sub_tlv);                      // create sub-TLV by packing Identifier, PublicKey, and Signature TLV records together

            tlv8.clear();         // clear existing TLV records

            // Final step is to encrypt the subTLV data using the same session_key as above with ChaCha20-Poly1305 

            EHK_DEBUG("------- ENCRYPTING SUB-TLVS -------\n");

            unsigned long long encrypted_len = 0;

            int enc_ret_val = crypto_aead_chacha20poly1305_ietf_encrypt(
                tlv8.buf(kTLVType_EncryptedData), &encrypted_len, 
                sub_tlv, sub_tlv_len, 
                NULL, 0, NULL,
                (const unsigned char *) HK_CHACHA_SETUP_MSG6, ps_keys.session_key
            );

            if (enc_ret_val != 0) {
                EHK_DEBUGLN("\n\nCould not encrypt accessory-x.\n\n");
                tlv8.clear();                                         // clear TLV records
                tlv8.val(kTLVType_State, pairState_M6);                // set State=<M6>
                tlv8.val(kTLVType_Error, tagError_Unknown);    // set Error=Authentication
                hk_store.set_pair_status(pairState_M1);
                send_tlv_response(&tlv8);                                       // send response to client
                return;
            }

            EHK_DEBUG("---------- END SUB-TLVS! ----------\n");

            tlv8.val(kTLVType_State, pairState_M6);                 // set State=<M6>
            tlv8.buf(kTLVType_EncryptedData, encrypted_len);        // set length of EncryptedData TLV record, which should now include the Authentication Tag at the end as required by HAP

            send_tlv_response(&tlv8);                               // send response to client

            mdns_service_txt_item_set("_hap","_tcp","sf","0");      // broadcast new status

            EHK_DEBUG("\n*** ACCESSORY PAIRED! ***\n");      
            break;
        }
    } // switch
    return;
}

// Post Pair-Verify

void HKClient::post_pair_verify_request(const uint8_t * data, size_t len) {
    auto& hk_store = HKStore::get_instance();

    TLV tlv8;

    int unpacked = tlv8.deserialize(data, len);

    if (!unpacked) {
        EHK_DEBUGLN("THERE WAS AN ERROR UNPACKING TLV!!!!");
        send_bad_request();
        return;
    }

    // tlv8.print();

    int tlv_state = tlv8.val(kTLVType_State);

    if(tlv_state == -1) {                                           // missing STATE TLV
        EHK_DEBUGLN("*** ERROR: Missing <M#> State TLV");
        send_bad_request();
        return;
    }

    if (!hk_store.has_admin_pairings()) {
        EHK_DEBUG("\n*** ERROR: Device already paired!\n\n");
        tlv8.clear();                                         // clear TLV records
        tlv8.val(kTLVType_State, tlv_state + 1);                  // set response STATE to requested state+1 (which should match the state that was expected by the controller)
        tlv8.val(kTLVType_Error,tagError_Unavailable);       // set Error=Unavailable
        send_tlv_response(&tlv8);                                       // send response to client
        return;
    }

    EHK_DEBUGF("Found <M%d>.\n", tlv_state);

    switch (tlv_state) {
        case pairState_M1: 
        {
            if (!tlv8.buf(kTLVType_PublicKey)) {
                EHK_DEBUG("\n*** ERROR: Public key is not found in tlv.\n\n");
                tlv8.clear();                                         // clear TLV records
                tlv8.val(kTLVType_State, pairState_M2);                // set State=<M6>
                tlv8.val(kTLVType_Error, tagError_Unknown);           // set Error=Unknown (there is no specific error type for missing/bad TLV data)
                send_tlv_response(&tlv8);                                       // send response to client
                return;
            }

            accessory_keys acc_keys;
            if (!hk_store.get_accessory_keys(&acc_keys)) {
                EHK_DEBUGLN("\n\nError: could not get accessory keys.\n");
                send_bad_request();
                return;
            }

            // generate curve25519 for accessory 5.7.2 - 1
            // generate random secret key using hardware generator, basically calling crypto_box_keypair
            uint8_t acc_curve_sk[crypto_scalarmult_curve25519_BYTES];
            esp_fill_random(acc_curve_sk, crypto_scalarmult_curve25519_BYTES);

            if (crypto_scalarmult_curve25519_base(pv_keys.acc_curve_pk, acc_curve_sk) != 0) {
                EHK_DEBUGLN("Failed to generate acc_pk Curve25519");
                tlv8.clear();                                         // clear TLV records
                tlv8.val(kTLVType_State, pairState_M2);                 // set State=<M6>
                tlv8.val(kTLVType_Error, tagError_Unknown);             // set Error=Unknown (there is no specific error type for missing/bad TLV data)
                send_tlv_response(&tlv8);                               // send response to client
                return;
            }

            // 5.7.2 - 2
            // Save ios cuve public key so we can use in M3
            memcpy(pv_keys.ios_curve_pk, tlv8.buf(kTLVType_PublicKey), crypto_scalarmult_curve25519_BYTES);

            // Create shared secret with accessory curve secret and ios curve pk

            if(crypto_scalarmult_curve25519(pv_keys.shared_secret, acc_curve_sk, pv_keys.ios_curve_pk) != 0) {
                EHK_DEBUGLN("Failed to generate pair verify shared secret out of acc_sk and ios_curve_pk Curve25519");
                tlv8.clear();                                         // clear TLV records
                tlv8.val(kTLVType_State, pairState_M2);                 // set State=<M6>
                tlv8.val(kTLVType_Error, tagError_Unknown);             // set Error=Unknown (there is no specific error type for missing/bad TLV data)
                send_tlv_response(&tlv8);                               // send response to client
                return;
            }

            char accessory_id_str[HK_ACCESSORY_ID_STR_SIZE];
            hk_store.get_serialized_accessory_id(accessory_id_str);
            size_t acc_id_str_len = HK_ACCESSORY_ID_STR_SIZE - 1;

            size_t acc_curve_pk_len = crypto_scalarmult_curve25519_BYTES;
            size_t ios_curve_pk_len = crypto_scalarmult_curve25519_BYTES;

            // Create accessory info from accessory public curve key, string-id, and ios curve public key
            // 5.7.2 - 3

            /* Construct AccessoryInfo by concatenating
            * a. Accessory's Curve25519 Public Key
            * b. Accessory ID
            * c. Controller's Curve25519 Public Key received in M1
            */

            size_t acc_info_len = acc_curve_pk_len + acc_id_str_len + ios_curve_pk_len;
            uint8_t accessory_info[acc_info_len];

            memcpy(accessory_info, pv_keys.acc_curve_pk, acc_curve_pk_len);
            memcpy(accessory_info + acc_curve_pk_len, accessory_id_str, acc_id_str_len);
            memcpy(accessory_info + acc_curve_pk_len + acc_id_str_len, pv_keys.ios_curve_pk, ios_curve_pk_len);

            tlv8.clear();

            // 5.7.2 - 4 & 5

            // Use Ed25519 to generate AccesorySignature by signing AccessoryInfo with its Accessory LTSK
            crypto_sign_ed25519_detached(tlv8.buf(kTLVType_Signature, crypto_sign_ed25519_BYTES), NULL, accessory_info, acc_info_len, acc_keys.ltsk); // Sign accessory_info

            // Write accessory identifier to tlv
            memcpy(tlv8.buf(kTLVType_Identifier, acc_id_str_len), accessory_id_str, acc_id_str_len);

            // serialize the data and save it in order to encrypt it. 
            size_t sub_tlv_len = tlv8.serialize(NULL);
            uint8_t sub_tlv[sub_tlv_len];
            sub_tlv_len = tlv8.serialize(sub_tlv);

            tlv8.clear();

            // 5.7.2 step 6
            create_hkdf(pv_keys.session_key, pv_keys.shared_secret, 32,  "Pair-Verify-Encrypt-Salt", "Pair-Verify-Encrypt-Info");

            // 5.7.2 - step 7
            unsigned long long encryption_len;
            int encrypt_ret_val = crypto_aead_chacha20poly1305_ietf_encrypt(
                tlv8.buf(kTLVType_EncryptedData), &encryption_len, 
                sub_tlv, sub_tlv_len, 
                NULL, 0, NULL, 
                (const unsigned char *) HK_CHACHA_VERIFY_MSG2, pv_keys.session_key
            );

            if (encrypt_ret_val != 0) {
                EHK_DEBUGLN("\nError: could not encrypt message for pair-verify M2.\n");
                tlv8.clear();                                         // clear TLV records
                tlv8.val(kTLVType_State, pairState_M2);                 // set State=<M6>
                tlv8.val(kTLVType_Error, tagError_Unknown);             // set Error=Unknown (there is no specific error type for missing/bad TLV data)
                send_tlv_response(&tlv8);                               // send response to client
            }

            // 5.7.2 - step 8
            tlv8.buf(kTLVType_EncryptedData, encryption_len);
            tlv8.val(kTLVType_State, pairState_M2);
            memcpy(tlv8.buf(kTLVType_PublicKey, crypto_scalarmult_curve25519_BYTES), pv_keys.acc_curve_pk, crypto_scalarmult_curve25519_BYTES);

            // 5.7.2 - step 9
            send_tlv_response(&tlv8);
            break;
        }

        case pairState_M3:
        {
            if(!tlv8.buf(kTLVType_EncryptedData)) {            
                EHK_DEBUG("\n*** ERROR: Required 'EncryptedData' TLV record for this step is bad or missing\n\n");
                tlv8.clear();                                           // clear TLV records
                tlv8.val(kTLVType_State, pairState_M4);                  // set State = <M6>
                tlv8.val(kTLVType_Error, tagError_Unknown);              // set Error = Unknown (there is no specific error type for missing/bad TLV data)
                send_tlv_response(&tlv8);                                                  // send response to client
                return;
            };

            // Validate iOS's device's authTag by extracting it from encryptedData, last 16 bytes
            // 5.7.4 - step 1 & 2
            uint8_t* encrypted_data = tlv8.buf(kTLVType_EncryptedData);
            int encryption_size = tlv8.len(kTLVType_EncryptedData);

            uint8_t decrypted_data[1024];
            unsigned long long decryption_len;

            int decryption_ret = crypto_aead_chacha20poly1305_ietf_decrypt(
                            decrypted_data, &decryption_len, NULL,
                            encrypted_data, encryption_size, NULL, 0,
                            (const unsigned char *) HK_CHACHA_VERIFY_MSG3, pv_keys.session_key);

            if (decryption_ret != 0) {
                EHK_DEBUG("\n*** ERROR: Verify Authentication Failed\n\n");
                tlv8.clear();                                                           // clear TLV records
                tlv8.val(kTLVType_State, pairState_M4);                                 // set State = <M6>
                tlv8.val(kTLVType_Error, tagError_Authentication);                      // set Error = Authentication
                send_tlv_response(&tlv8);                                               // send response to client
                return;        
            }

            if (!tlv8.deserialize(decrypted_data, decryption_len)) {
                EHK_DEBUG("\n*** ERROR: Can't parse decrypted data into separate TLV records\n\n");
                tlv8.clear();                                         // clear TLV records
                tlv8.val(kTLVType_State, pairState_M4);                // set State=<M6>
                tlv8.val(kTLVType_Error, tagError_Unknown);           // set Error=Unknown (there is no specific error type for missing/bad TLV data)
                send_tlv_response(&tlv8);                                       // send response to client
                return;
            }

            if(!tlv8.buf(kTLVType_Identifier) || !tlv8.buf(kTLVType_Signature)){            
                EHK_DEBUG("\n*** ERROR: One or more of required 'Identifier, and 'Signature' TLV records for this step is bad or missing\n\n");
                tlv8.clear();                                         // clear TLV records
                tlv8.val(kTLVType_State, pairState_M4);                // set State=<M6>
                tlv8.val(kTLVType_Error, tagError_Unknown);           // set Error=Unknown (there is no specific error type for missing/bad TLV data)
                send_tlv_response(&tlv8);                                       // send response to client
                return;
            }

            // 5.7.4 - step 3
            int ios_dev_id_len = tlv8.len(kTLVType_Identifier);
            char ios_dev_id[ios_dev_id_len + 1];
            ios_dev_id[ios_dev_id_len] = '\0';
            memcpy(ios_dev_id, tlv8.buf(kTLVType_Identifier), ios_dev_id_len);

            pairing_info * pairing = hk_store.find_pairing(ios_dev_id);
            if (!pairing) {
                EHK_DEBUG("\n*** ERROR: Unrecognized iOS Pairing ID\n\n");
                tlv8.clear();                                       // clear TLV records
                tlv8.val(kTLVType_State, pairState_M4);              // set State=<M4>
                tlv8.val(kTLVType_Error, tagError_Authentication);   // set Error=Authentication
                send_tlv_response(&tlv8);                           // send response to client
                return;
            }

            size_t ios_dev_curve_pk_len = crypto_scalarmult_curve25519_BYTES;
            size_t acc_curve_pk_len = crypto_scalarmult_curve25519_BYTES;

            // 5.7.4 - step 4
            /* Construct iOSDeviceInfo by concatenating
            * a. Controllers's Curve25519 Public Key (received in M1)
            * b. Controller ID
            * c. Accessory's Curve25519 Public Key (sent in M2)
            */
            size_t ios_device_info_len = ios_dev_curve_pk_len + ios_dev_id_len + acc_curve_pk_len;
            uint8_t ios_device_info[ios_dev_id_len];
            memcpy(ios_device_info, pv_keys.ios_curve_pk, ios_dev_curve_pk_len);
            memcpy(ios_device_info + ios_dev_curve_pk_len, pairing->device_pair_id, ios_dev_id_len);
            memcpy(ios_device_info + ios_dev_curve_pk_len + ios_dev_id_len, pv_keys.acc_curve_pk, acc_curve_pk_len);

            int verify_sig_ret = crypto_sign_ed25519_verify_detached(tlv8.buf(kTLVType_Signature), ios_device_info, ios_device_info_len, pairing->device_ltpk);
            if (verify_sig_ret != 0) {
                EHK_DEBUG("\n*** ERROR: LTPK Signature Verification Failed\n\n");
                tlv8.clear();                                         // clear TLV records
                tlv8.val(kTLVType_State, pairState_M4);                // set State=<M6>
                tlv8.val(kTLVType_Error, tagError_Authentication);    // set Error=Authentication
                send_tlv_response(&tlv8);                                       // send response to client
                return;                
            }

            tlv8.clear();
            tlv8.val(kTLVType_State, pairState_M4);
            send_tlv_response(&tlv8);

            // Allows encryption
            create_hkdf(s_keys.encrypt_key, pv_keys.shared_secret, 32, "Control-Salt", "Control-Read-Encryption-Key");
            // Allows decryption
            create_hkdf(s_keys.decrypt_key, pv_keys.shared_secret, 32, "Control-Salt", "Control-Write-Encryption-Key");

            encryption_count = 0;
            decryption_count = 0;

            secured_pairing = pairing;
        }
    }
}

// POST /pairings

void HKClient::post_pairings_request(const uint8_t * data, size_t data_len) {
    if (!secured_pairing) {
        send_unauthorized_request();
        return;
    }

        TLV tlv8;

    int unpacked = tlv8.deserialize(data, data_len);

    if (!unpacked) {
        EHK_DEBUGLN("There was an error deserializing tlv for post_pairings_request.");
        send_bad_request();
        return;
    }

    if (tlv8.val(kTLVType_State) != 1) {
        EHK_DEBUGLN("** Error: 'State is not present in TLV record");
        send_bad_request();
        return;
    }

    auto& hk_store = HKStore::get_instance();

    int method = tlv8.val(kTLVType_Method);

    switch (method) {
        case 3:
        {
            if(!tlv8.buf(kTLVType_Identifier) || !tlv8.buf(kTLVType_PublicKey) || !tlv8.buf(kTLVType_Permissions)){            
                Serial.print("\n*** ERROR: One or more of required 'Identifier,' 'PublicKey,' and 'Permissions' TLV records for this step is bad or missing\n\n");
                tlv8.clear();                                         // clear TLV records
                tlv8.val(kTLVType_State, pairState_M2);                // set State=<M2>
                tlv8.val(kTLVType_Error, tagError_Unknown);           // set Error=Unknown (there is no specific error type for missing/bad TLV data)
                break;
            }

            // 5.10.2 - 2
            if(!secured_pairing->is_admin){
                EHK_DEBUG("\n*** ERROR: Controller making request does not have admin privileges to add/update other Controllers\n\n");
                tlv8.clear();                                         // clear TLV records
                tlv8.val(kTLVType_State, pairState_M2);                // set State = <M2>
                tlv8.val(kTLVType_Error, tagError_Authentication);    // set Error = Authentication
                break;
            }

            int pair_id_len = tlv8.len(kTLVType_Identifier);
            char pair_id[pair_id_len + 1];
            memcpy(pair_id, tlv8.buf(kTLVType_Identifier), pair_id_len);
            pair_id[pair_id_len] = '\0';

            pairing_info * found_pairing = hk_store.find_pairing(pair_id);

            // 5.10.2 - 3
            if (found_pairing) {
                int permission = tlv8.val(kTLVType_Permissions);
                tlv8.clear();
                tlv8.val(kTLVType_State, pairState_M2);
                // 5.10.2 - 3a
                if (!memcmp(secured_pairing->device_ltpk, found_pairing->device_ltpk, crypto_sign_ed25519_PUBLICKEYBYTES)) {
                    // This updates all contents from the matched pairing
                    hk_store.add_pairing(pair_id, strlen(pair_id), nullptr, permission);
                } else {
                    // 5.10.2 - 3b
                    tlv8.val(kTLVType_Error, tagError_Unknown);         // set Error=Unknown
                }
                break;
            }

            // 5.10.2 - 4
            if (!hk_store.add_pairing(pair_id, strlen(pair_id), tlv8.buf(kTLVType_PublicKey), tlv8.val(kTLVType_Permissions) > 0)) {
                EHK_DEBUG("\n*** ERROR: There is no more space to add another pairing.\n\n");
                tlv8.clear();                                         // clear TLV records
                tlv8.val(kTLVType_State, pairState_M2);                // set State = <M2>
                tlv8.val(kTLVType_Error, tagError_MaxPeers);    // set Error = Authentication
                break;
            }

            tlv8.clear();
            tlv8.val(kTLVType_State, pairState_M2);
            break;
        }
        case 4:
        {
            if (!tlv8.buf(kTLVType_Identifier)) {
                EHK_DEBUG("\n*** ERROR: Required 'Identifier' TLV record for this step is bad or missing\n\n");
                tlv8.clear();
                tlv8.val(kTLVType_State, pairState_M2);
                tlv8.val(kTLVType_Error, tagError_Unknown);
                break;
            }

            if (!secured_pairing) {
                EHK_DEBUG("\n*** ERROR: Controller making request does not have admin privileges to remove pairings\n\n");
                tlv8.clear();                                         // clear TLV records
                tlv8.val(kTLVType_State, pairState_M2);                // set State=<M2>
                tlv8.val(kTLVType_Error, tagError_Authentication);    // set Error=Authentication
                break;
            }

            int pair_id_len = tlv8.len(kTLVType_Identifier);
            char pair_id[pair_id_len + 1];
            memcpy(pair_id, tlv8.buf(kTLVType_Identifier), pair_id_len);
            pair_id[pair_id_len] = '\0';

            hk_store.remove_pairing(pair_id);

            tlv8.clear();
            tlv8.val(kTLVType_State, pairState_M2);

            break;
        }
        default:
            break;
    };

    send_tlv_response(&tlv8);
    hk_store.refresh_connections = true;
}

// GET /accessories

void HKClient::get_accessories_request() {
    if (!secured_pairing) {
        send_unauthorized_request();
    }

    auto& hk_store = HKStore::get_instance();

    int json_len = 0;
    char * json = hk_store.serialize_accessory(&json_len);
    if (!json || !json_len) {
        return send_bad_request();
    }

    char * header = NULL;
    asprintf(&header, header_200_app_json, json_len);

    if (!header) {
        send_bad_request();
    } else {
        send_encrypted(header, (uint8_t *) json, json_len);
    }

    delete[] header;
    delete[] json;
}

// Put Characteristics

void HKClient::put_characteristics_request(const char * buf, size_t buf_len) {
    if (!secured_pairing) {
        EHK_DEBUG("\n\nCannot access put_characteristics without permission.\n\n");
        send_unauthorized_request();
        return;
    }

    auto& hk_store = HKStore::get_instance();

    char * output_buf = NULL;
    int output_buf_len = hk_store.deserialize_characteristics(buf, buf_len, output_buf);
    if (output_buf_len > 0) {
        char * header = NULL;
        EHK_DEBUGLN("There were errors present when putting values in characteristics.");
        asprintf(&header, header_207_fmt, output_buf_len);
        send_encrypted(header, (uint8_t *) output_buf, output_buf_len);
        delete[] header;
    } else if (output_buf_len == 0) {
        send_encrypted(header_204_fmt);
    } else {
        EHK_DEBUGLN("\n\nThere was an error deserializing characteristics.");
        send_bad_request();
    }

    hk_store.ignore_connection = client_index;
    delete[] output_buf;
}

// get characteristics

void HKClient::get_characteristics(int * ids, size_t ids_size) {
    if (!secured_pairing) {
        EHK_DEBUG("\n\nCannot access characteristics without permission.\n\n");
        send_unauthorized_request();
        return;
    }

    auto& hk_store = HKStore::get_instance();

    int json_len = 0;
    char * json = hk_store.serialize_characteristics(&json_len, ids, ids_size);

    if (!json || !json_len) {
        send_bad_request();
        return;
    }

    char * header = NULL;
    asprintf(&header, header_200_app_json, json_len);
    send_encrypted(header, (uint8_t *) json, json_len);
    delete[] header;
}

// Error Responses

void HKClient::send_unauthorized_request() {
    const char * payload = header_470_fmt;
    if (secured_pairing) {
        send_encrypted(payload);
    } else {
        client.print(payload);
    }
    delay(1);
    disconnect_client();
}

void HKClient::send_not_found_request() {
    const char * payload = header_404_fmt;

    if (secured_pairing) {
        send_encrypted(payload);
    } else {
        client.print(payload);
    }
    delay(1);
    disconnect_client();
}

void HKClient::send_bad_request() {
    const char * payload = header_400_fmt;

    if (secured_pairing) {
        send_encrypted(payload);
    } else {
        client.print(payload);
    }
    delay(1);
    disconnect_client();
}

void HKClient::send_tlv_response(TLV * tlv8) {
    int tlv8_length = tlv8->serialize(NULL);      // return number of bytes needed to serialize TLV records into a buffer
    uint8_t tlv8_data[tlv8_length];                  // create buffer
    tlv8->serialize(tlv8_data);                    // serialize TLV records into buffer

    char * header = NULL;
    asprintf(&header, header_200_app_tlv8, tlv8_length);

    if (secured_pairing) {
        send_encrypted(header, tlv8_data, tlv8_length);
    } else {
        client.print(header);
        client.write(tlv8_data, tlv8_length);
    }
    free(header);

    EHK_DEBUG("\n>>>>>>>>>> ");
    EHK_DEBUG(client.remoteIP().toString().c_str());
    EHK_DEBUG(" >>>>>>>>>>\n");
    EHK_DEBUG("------------ SENT TLV8 Response! --------------\n");
}

int HKClient::decrypt_message(char * http_buf, const char * encrypted, int encrypted_len) {
    int offset_encrypt = 0;
    int offset_decrypt = 0;

    char * decrypted = http_buf;

    while (offset_encrypt < encrypted_len) {
        const unsigned char * encrypt = (const unsigned char *) encrypted + offset_encrypt;
        unsigned char * decrypt = (unsigned char *) decrypted + offset_decrypt;

        int message_len = encrypt[0] + encrypt[1] * 256;

        uint8_t nonce[crypto_aead_chacha20poly1305_IETF_NPUBBYTES] = { 0, };
        nonce[4] = decryption_count % 256;
        nonce[5] = decryption_count++ / 256;

        unsigned long long decryption_size = 0;
        int decrypt_ret = crypto_aead_chacha20poly1305_ietf_decrypt(
            decrypt, &decryption_size,
            NULL,
            encrypt + HK_AAD_SIZE, message_len + crypto_aead_chacha20poly1305_ietf_ABYTES, 
            encrypt, HK_AAD_SIZE,
            // nonce
            (const unsigned char *) nonce,
            s_keys.decrypt_key);

        if (decrypt_ret != 0) {
            EHK_DEBUG("\n\n*** ERROR: Can't Decrypt Message\n\n");
            return 0;
        }

        offset_decrypt += decryption_size;
        offset_encrypt += HK_AAD_SIZE + message_len + crypto_aead_chacha20poly1305_ABYTES;
    }

    decrypted[offset_decrypt] = '\0';

    return offset_decrypt;
}

int HKClient::send_encrypted(const char * header, const uint8_t * body, int body_len) {
    bool has_body = body != NULL && body_len > 0;
    int msg_len = strlen(header);

    if (has_body) {
        msg_len += body_len;
    }

    uint8_t msg[msg_len];
    memcpy(msg, header, strlen(header));

    if (has_body) {
        memcpy(msg + strlen(header), body, body_len);
    }

    // Encryption Attributes

    uint8_t nonce[crypto_aead_chacha20poly1305_IETF_NPUBBYTES] = { 0, };
    int encrypted_size = msg_len + (msg_len / HK_MAX_ENCRYPTION_SIZE + 1) * (HK_AAD_SIZE + crypto_aead_chacha20poly1305_ietf_ABYTES);
    uint8_t encrypted[encrypted_size];

    uint8_t * decrypted_ptr = msg;
    uint8_t * encrypted_prt = encrypted;

    while (msg_len > 0) {
        int chunk_size = min(msg_len, HK_MAX_ENCRYPTION_SIZE);

        uint8_t aad[HK_AAD_SIZE];
        aad[0] = chunk_size % 256;
        aad[1] = chunk_size / 256;

        memcpy(encrypted_prt, aad, HK_AAD_SIZE);
        encrypted_prt += HK_AAD_SIZE;

        nonce[4] = encryption_count % 256;
        nonce[5] = encryption_count++ / 256;

        int encrypt_ret = crypto_aead_chacha20poly1305_ietf_encrypt(
            encrypted_prt, NULL,
            decrypted_ptr, chunk_size,
            aad, HK_AAD_SIZE, NULL,
            nonce, s_keys.encrypt_key
        );

        if (encrypt_ret != 0){
            EHK_DEBUGLN("\n\nThere was an error encrypting message to send to client.\n");
            return 0;
        }

        msg_len -= chunk_size;
        decrypted_ptr += chunk_size;
        encrypted_prt += chunk_size + crypto_aead_chacha20poly1305_ietf_ABYTES;
    }

    int sent = 0;
    if ((sent = client.write(encrypted, encrypted_size)) != encrypted_size) {
        EHK_DEBUGF("Did not send full encryption to server. Sent: %d\n\n", sent);
    }
    return 1;
}

#endif