#include "HKServer.h"

#ifdef ARDUINO_ARCH_ESP32

#include <mdns.h>
#include <mbedtls/sha512.h>
#include <mbedtls/base64.h>
#include <sodium/randombytes.h>

#include "my_config.h"

// CONFIG

#define CONFIG_ID 1  // If characteristics change, update this + 1
#define CONFIG_MODEL_NAME "WLED-ESP32"
#define CONFIG_DISPLAY_NAME "WLED Server"  // change with optional second argument in homeSpan.begin()
#define CONFIG_HOST_NAME "WLED-LED"
#define CONFIG_HOST_NAME_SUFFIX "ESP32"

bool HKServer::begin() {
    EHK_DEBUGLN("Starting HKServer");

    // Allocate HKData

    HKStore::get_instance();

    // Allocate clients
    hk_clients = (HKClient **) calloc(MAX_CLIENTS, sizeof(HKClient *));

    for (int i = 0; i < MAX_CLIENTS; i++) {
        hk_clients[i] = new HKClient;
    }

    server = new WiFiServer(SERVER_PORT);
    server->begin();

    EHK_DEBUGLN("HomeKit Server Started.");
    return true;
};

void HKServer::poll() {
    if (!server) {
        EHK_DEBUGLN("HomeKit Server has not been initialized yet...");
        return;
    }

    if (connected) {
        if (WiFi.status() != WL_CONNECTED) {
            connected = false;
        }
    } else {
        // It was disconnected so now check again if it's now connected.
        if (WiFi.status() == WL_CONNECTED) {
            connected = true;
            // Setup mdns since it is now connected
            setup_mdns();
        }
    }

    auto& hk_store = HKStore::get_instance();

    // Update characteristic regardless if the server is connected or not.
    if (device_callback && device_callback->updated_values()) {
        EHK_DEBUGLN("\n-- Notify iOS devices of value changed.\n");
        hk_store.set_lightbulb_values(device_callback->is_on(), device_callback->get_hue(), device_callback->get_saturation(), device_callback->get_brightness());
        device_callback->handled_update();
    }

    if (hk_store.refresh_connections) {
        hk_store.refresh_connections = false;
        refresh_connections();
    }

    WiFiClient new_client = server->available();   // Listen for incoming clients
    if (new_client) {
        EHK_DEBUGLN("Received new client.");
        int next_index = get_free_slot();
        // Remove last connected device so we can accomodate this client.
        if (next_index == -1) {
            next_index = randombytes_uniform(MAX_CLIENTS);
            hk_clients[next_index]->disconnect_client();
        }

        hk_clients[next_index]->set_client(new_client, next_index);
        hk_store.set_pair_status(pairState_M1);
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        HKClient * hap_client = hk_clients[i];
        if (hap_client->payload_available()) {
            hap_client->process_request();

            if (!hap_client->client_connected()) {
                EHK_DEBUGLN("\nClient Disconnecting...\n");
            }
        }
    }

    event_notify(); // Notify any changes in char values.
};

void HKServer::reconnect() {
    connected = false;
}

void HKServer::setup_mdns() {
    // setup mdns that was configured with WLED.

    auto& hk_store = HKStore::get_instance();

    char qrID[5] = HK_SETUP_ID;

    EHK_DEBUG("\nStarting MDNS...\n\n");
    EHK_DEBUG("Server Port: ");
    EHK_DEBUG(SERVER_PORT);
    EHK_DEBUG("\nDisplay Name:  ");
    EHK_DEBUG(CONFIG_DISPLAY_NAME);
    EHK_DEBUG("\nModel Name:    ");
    EHK_DEBUG(CONFIG_MODEL_NAME);
    EHK_DEBUG("\nSetup ID:      ");
    EHK_DEBUG(qrID);
    EHK_DEBUG("\n\n");

    // add MDNS (Bonjour) TXT records for configurable as well as fixed values (HAP Table 6-7)

    char accessory_id[HK_ACCESSORY_ID_STR_SIZE];
    hk_store.get_serialized_accessory_id(accessory_id);
    char config_num[16];
    sprintf(config_num, "%d", CONFIG_ID);
    char cat_id[3];
    sprintf(cat_id, "%d", LIGHT_CATEGORY_ID);

    mdns_service_add(NULL, "_hap", "_tcp", SERVER_PORT, NULL, 0);       // advertise HAP service on specified port
    mdns_service_txt_item_set("_hap", "_tcp", "c#", config_num);        // Accessory Current Configuration Number (updated whenever config of HAP Accessory Attribute Database is updated)
    mdns_service_txt_item_set("_hap", "_tcp", "ff", "0");               // HAP Pairing Feature flags.  MUST be "0" to specify Pair Setup method (HAP Table 5-3) without MiFi Authentification
    mdns_service_txt_item_set("_hap", "_tcp", "id", accessory_id);      // string version of Accessory ID in form XX:XX:XX:XX:XX:XX (HAP Section 5.4)
    mdns_service_txt_item_set("_hap", "_tcp", "md", CONFIG_MODEL_NAME); // Accessory Model Name
    mdns_service_txt_item_set("_hap", "_tcp", "pv", "1.1");             // HAP version - MUST be set to "1.1" (HAP Section 6.6.3)
    mdns_service_txt_item_set("_hap", "_tcp", "s#", "1");               // HAP current state - MUST be set to "1"
    mdns_service_txt_item_set("_hap", "_tcp", "ci", cat_id);             // Accessory Category (HAP Section 13.1)
    mdns_service_txt_item_set("_hap", "_tcp", "sf", hk_store.has_admin_pairings() ? "0" : "1");  // set Status Flag = 1 (Table 6-8) if it has not been paired

    uint8_t hashInput[22];
    uint8_t hashOutput[64];
    char setupHash[9];
    size_t len;

    memcpy(hashInput, qrID, 4);                                              // Create the Seup ID for use with optional QR Codes.  This is an undocumented feature of HAP R2!
    memcpy(hashInput + 4, accessory_id, 17);                                 // Step 1: Concatenate 4-character Setup ID and 17-character Accessory ID into hashInput
    mbedtls_sha512_ret(hashInput, 21, hashOutput, 0);                        // Step 2: Perform SHA-512 hash on combined 21-byte hashInput to create 64-byte hashOutput
    mbedtls_base64_encode((uint8_t *) setupHash, 9, &len, hashOutput, 4);    // Step 3: Encode the first 4 bytes of hashOutput in base64, which results in an 8-character, null-terminated, setupHash
    mdns_service_txt_item_set("_hap", "_tcp", "sh", setupHash);              // Step 4: broadcast the resulting Setup Hash
}

int HKServer::get_free_slot() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!hk_clients[i]->client_connected()) {
            return i;
        }
    }
    return -1;
}

void HKServer::event_notify() {
    char * header = NULL;
    int json_len = 0;

    auto& hk_store = HKStore::get_instance();
    char * json_events = hk_store.serialize_events(&json_len);

    if (!json_events || !json_len) {
        // No new value changes.
        goto deallocate;
    }

    static const char* header_event_200_fmt = 
        "EVENT/1.0 200 OK\r\n"
        "Content-Type: application/hap+json\r\n"
        "Content-Length: %d\r\n"
        "\r\n";

    if (!asprintf(&header, header_event_200_fmt, json_len)) {
        goto deallocate;
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        HKClient * hk_client = hk_clients[i];
        if (hk_client->client_connected() && i != hk_store.ignore_connection) {
            hk_client->send_encrypted(header, (uint8_t *) json_events, json_len);
        }
    }

    // If an ios device changed characteristic value, notify WLED.
    if (hk_store.ignore_connection != -1 && device_callback != nullptr) {
        EHK_DEBUGLN("-- Notify WLED of value Changed.");
        bool on = 0;
        int hue = 0;
        int sat = 0;
        int bri = 0;
        hk_store.get_lightbulb_values(&on, &hue, &sat, &bri);

        device_callback->_bri_callback(bri);
        device_callback->_on_callback(on);
        device_callback->_color_callback(hue, sat);
    }

    hk_store.ignore_connection = -1;

    deallocate:
        delete[] header;
        delete[] json_events;
}

void HKServer::refresh_connections() {
    auto& hk_store = HKStore::get_instance();

    for (int i = 0; i < MAX_CLIENTS; i++) {
        HKClient * hk_client = hk_clients[i];
        if (hk_client->client_connected()) {
            if (!hk_store.has_admin_pairings() || (hk_client->session_secured() && !hk_client->session_verified())) {
                hk_client->disconnect_client();
            }
        }
    }
}

void HKServer::add_device(HKDevice * new_device) {
    device_callback = new_device;
}

#endif