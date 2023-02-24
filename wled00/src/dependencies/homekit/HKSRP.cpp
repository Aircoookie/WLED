#include "HKSRP.h"

#include <mbedtls/base64.h>
#include <mbedtls/sha512.h>
#include <sodium/randombytes.h>

#include "HKDebug.h"

#ifdef ARDUINO_ARCH_ESP32

const uint16_t N_size = 384;

HKSRP::HKSRP() {
    // 3072-bit group N (per RFC5054, Appendix A)
    const uint8_t srp_N[N_size] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc9, 0x0f, 0xda, 0xa2,
        0x21, 0x68, 0xc2, 0x34, 0xc4, 0xc6, 0x62, 0x8b, 0x80, 0xdc, 0x1c, 0xd1,
        0x29, 0x02, 0x4e, 0x08, 0x8a, 0x67, 0xcc, 0x74, 0x02, 0x0b, 0xbe, 0xa6,
        0x3b, 0x13, 0x9b, 0x22, 0x51, 0x4a, 0x08, 0x79, 0x8e, 0x34, 0x04, 0xdd,
        0xef, 0x95, 0x19, 0xb3, 0xcd, 0x3a, 0x43, 0x1b, 0x30, 0x2b, 0x0a, 0x6d,
        0xf2, 0x5f, 0x14, 0x37, 0x4f, 0xe1, 0x35, 0x6d, 0x6d, 0x51, 0xc2, 0x45,
        0xe4, 0x85, 0xb5, 0x76, 0x62, 0x5e, 0x7e, 0xc6, 0xf4, 0x4c, 0x42, 0xe9,
        0xa6, 0x37, 0xed, 0x6b, 0x0b, 0xff, 0x5c, 0xb6, 0xf4, 0x06, 0xb7, 0xed,
        0xee, 0x38, 0x6b, 0xfb, 0x5a, 0x89, 0x9f, 0xa5, 0xae, 0x9f, 0x24, 0x11,
        0x7c, 0x4b, 0x1f, 0xe6, 0x49, 0x28, 0x66, 0x51, 0xec, 0xe4, 0x5b, 0x3d,
        0xc2, 0x00, 0x7c, 0xb8, 0xa1, 0x63, 0xbf, 0x05, 0x98, 0xda, 0x48, 0x36,
        0x1c, 0x55, 0xd3, 0x9a, 0x69, 0x16, 0x3f, 0xa8, 0xfd, 0x24, 0xcf, 0x5f,
        0x83, 0x65, 0x5d, 0x23, 0xdc, 0xa3, 0xad, 0x96, 0x1c, 0x62, 0xf3, 0x56,
        0x20, 0x85, 0x52, 0xbb, 0x9e, 0xd5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6d,
        0x67, 0x0c, 0x35, 0x4e, 0x4a, 0xbc, 0x98, 0x04, 0xf1, 0x74, 0x6c, 0x08,
        0xca, 0x18, 0x21, 0x7c, 0x32, 0x90, 0x5e, 0x46, 0x2e, 0x36, 0xce, 0x3b,
        0xe3, 0x9e, 0x77, 0x2c, 0x18, 0x0e, 0x86, 0x03, 0x9b, 0x27, 0x83, 0xa2,
        0xec, 0x07, 0xa2, 0x8f, 0xb5, 0xc5, 0x5d, 0xf0, 0x6f, 0x4c, 0x52, 0xc9,
        0xde, 0x2b, 0xcb, 0xf6, 0x95, 0x58, 0x17, 0x18, 0x39, 0x95, 0x49, 0x7c,
        0xea, 0x95, 0x6a, 0xe5, 0x15, 0xd2, 0x26, 0x18, 0x98, 0xfa, 0x05, 0x10,
        0x15, 0x72, 0x8e, 0x5a, 0x8a, 0xaa, 0xc4, 0x2d, 0xad, 0x33, 0x17, 0x0d,
        0x04, 0x50, 0x7a, 0x33, 0xa8, 0x55, 0x21, 0xab, 0xdf, 0x1c, 0xba, 0x64,
        0xec, 0xfb, 0x85, 0x04, 0x58, 0xdb, 0xef, 0x0a, 0x8a, 0xea, 0x71, 0x57,
        0x5d, 0x06, 0x0c, 0x7d, 0xb3, 0x97, 0x0f, 0x85, 0xa6, 0xe1, 0xe4, 0xc7,
        0xab, 0xf5, 0xae, 0x8c, 0xdb, 0x09, 0x33, 0xd7, 0x1e, 0x8c, 0x94, 0xe0,
        0x4a, 0x25, 0x61, 0x9d, 0xce, 0xe3, 0xd2, 0x26, 0x1a, 0xd2, 0xee, 0x6b,
        0xf1, 0x2f, 0xfa, 0x06, 0xd9, 0x8a, 0x08, 0x64, 0xd8, 0x76, 0x02, 0x73,
        0x3e, 0xc8, 0x6a, 0x64, 0x52, 0x1f, 0x2b, 0x18, 0x17, 0x7b, 0x20, 0x0c,
        0xbb, 0xe1, 0x17, 0x57, 0x7a, 0x61, 0x5d, 0x6c, 0x77, 0x09, 0x88, 0xc0,
        0xba, 0xd9, 0x46, 0xe2, 0x08, 0xe2, 0x4f, 0xa0, 0x74, 0xe5, 0xab, 0x31,
        0x43, 0xdb, 0x5b, 0xfc, 0xe0, 0xfd, 0x10, 0x8e, 0x4b, 0x82, 0xd1, 0x20,
        0xa9, 0x3a, 0xd2, 0xca, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };

    mbedtls_mpi_init(&N);
    mbedtls_mpi_init(&g);
    mbedtls_mpi_init(&s);
    mbedtls_mpi_init(&x);
    mbedtls_mpi_init(&v);
    mbedtls_mpi_init(&A);
    mbedtls_mpi_init(&b);
    mbedtls_mpi_init(&B);
    mbedtls_mpi_init(&S);
    mbedtls_mpi_init(&k);
    mbedtls_mpi_init(&u);
    mbedtls_mpi_init(&K);
    mbedtls_mpi_init(&M1);
    mbedtls_mpi_init(&M2);
    mbedtls_mpi_init(&_rr);
    mbedtls_mpi_init(&t1);
    mbedtls_mpi_init(&t2);
    mbedtls_mpi_init(&t3);

    int ret = 0;

    // load N and g into mpi structures

    ret += mbedtls_mpi_read_binary(&N, srp_N, sizeof(srp_N));
    ret += mbedtls_mpi_lset(&g, g3072);

    // compute k = SHA512( N | PAD(g) )

    uint8_t N_buf[N_size];
    ret += mbedtls_mpi_write_binary(&N, N_buf, sizeof(N_buf));

    uint8_t g_buf[N_size] = { 0 };              // g buf with pad;
    ret += mbedtls_mpi_write_binary(&g, g_buf, sizeof(g_buf));

    uint8_t k_sha_buffer[64];                   // k hashed
    mbedtls_sha512_context ctx;

    mbedtls_sha512_init(&ctx);
    ret += mbedtls_sha512_starts_ret(&ctx, 0);
    ret += mbedtls_sha512_update_ret(&ctx, N_buf, sizeof(srp_N));
    ret += mbedtls_sha512_update_ret(&ctx, g_buf, sizeof(g_buf));
    ret += mbedtls_sha512_finish_ret(&ctx, k_sha_buffer);
    mbedtls_sha512_free(&ctx);

    ret += mbedtls_mpi_read_binary(&k, k_sha_buffer, sizeof(k_sha_buffer));  // load hash result into mpi structure k

    if (ret < 0) {
        EHK_DEBUGLN("\n\nThere was an error generating sha512 for k.\n\n");
        return;
    }
}

void HKSRP::generate_verifier(const char * setup_code) {
    EHK_DEBUGLN("Generating verifier");

    uint8_t salt[16];
    randombytes_buf(salt, sizeof(salt));
    mbedtls_mpi_read_binary(&s, salt, sizeof(salt));

    // compute x = SHA512( s | SHA512( I | ":" | P ) )

    int ret = 0;

    // 1.) first generate SHA512( I | ":" | P)

    char password[11];
    sprintf(password, "%.3s-%.2s-%.3s", setup_code, setup_code + 3, setup_code + 5);

    EHK_DEBUGF("\nGENERATED PASSWORD: %s\n", password);

    uint8_t icp_sha_buf[64];
    mbedtls_sha512_context ctx;

    mbedtls_sha512_init(&ctx);
    ret += mbedtls_sha512_starts_ret(&ctx, 0);
    ret += mbedtls_sha512_update_ret(&ctx, (const unsigned char *) username, strlen(username));
    ret += mbedtls_sha512_update_ret(&ctx, (const unsigned char *) ":", 1);
    ret += mbedtls_sha512_update_ret(&ctx, (const unsigned char *) password, strlen(password));
    ret += mbedtls_sha512_finish_ret(&ctx, icp_sha_buf);
    mbedtls_sha512_free(&ctx);

    if (ret < 0) {
        EHK_DEBUGLN("There was an error generating sha512 for Pair-Setup and setup code.");
        return;
    }

    // 2.) Next, generate SHA512( s | Icp_HASHED )

    uint8_t x_sha_buf[64];

    mbedtls_sha512_init(&ctx);
    ret += mbedtls_sha512_starts_ret(&ctx, 0);
    ret += mbedtls_sha512_update_ret(&ctx, salt, sizeof(salt));
    ret += mbedtls_sha512_update_ret(&ctx, icp_sha_buf, sizeof(icp_sha_buf));
    ret += mbedtls_sha512_finish_ret(&ctx, x_sha_buf);
    mbedtls_sha512_free(&ctx);

    if (ret < 0) {
        EHK_DEBUGLN("There was an error generating sha512 for salt and ucp.");
        return;
    }

    // 3.) Finally, write computation to x.

    mbedtls_mpi_read_binary(&x, x_sha_buf, sizeof(x_sha_buf));      // x = private key

    // compute v = g^x % N

    mbedtls_mpi_exp_mod(&v, &g, &x, &N, &_rr);                      // v = password verifier
}

void HKSRP::generate_private_key() {
    EHK_DEBUGLN("Generating private key");
    uint8_t private_key[32];

    esp_fill_random(private_key, sizeof(private_key));
    mbedtls_mpi_read_binary(&b, private_key, sizeof(private_key));
}

void HKSRP::generate_public_key() {
    EHK_DEBUGLN("Generating public key");
    generate_private_key();                         // Create Private Key

    // compute B = ((k * v) + g^b) % N              // Create public key out of private key

    mbedtls_mpi_mul_mpi(&t1, &k, &v);               // t1 = k * v
    mbedtls_mpi_exp_mod(&t2, &g, &b, &N, &_rr);     // t2 = g^b % N
    mbedtls_mpi_add_mpi(&t3, &t1, &t2);             // t3 = t1 + t2
    mbedtls_mpi_mod_mpi(&B, &t3, &N);               // B = t3 % N      = ACCESSORY PUBLIC KEY
}

void HKSRP::compute_key() {
    EHK_DEBUGLN("Generating shared session key");

    int ret = 0;
    mbedtls_sha512_context ctx;

    // compute u = SHA512( PAD(A) | PAD(B) )

    uint8_t A_buf[N_size] = { 0 };
    uint8_t B_buf[N_size] = { 0 };

    mbedtls_mpi_write_binary(&A, A_buf, sizeof(A_buf));
    mbedtls_mpi_write_binary(&B, B_buf, sizeof(B_buf));

    uint8_t u_sha_buf[64];

    mbedtls_sha512_init(&ctx);
    ret += mbedtls_sha512_starts_ret(&ctx, 0);
    ret += mbedtls_sha512_update_ret(&ctx, A_buf, sizeof(A_buf));
    ret += mbedtls_sha512_update_ret(&ctx, B_buf, sizeof(B_buf));
    ret += mbedtls_sha512_finish_ret(&ctx, u_sha_buf);
    mbedtls_sha512_free(&ctx);

    if (ret < 0) {
        EHK_DEBUGLN("Error computing u.");
        return;
    }

    mbedtls_mpi_read_binary(&u, u_sha_buf, sizeof(u_sha_buf));          // load hash result into mpi structure u

    // compute S = (A * v^u) ^b % N                                     // PreMaster Secret

    mbedtls_mpi_exp_mod(&t1, &v, &u, &N, &_rr);  // t1 = v^u % N
    mbedtls_mpi_mul_mpi(&t2, &A, &t1);           // t2 = A * t1
    mbedtls_mpi_mod_mpi(&t1, &t2, &N);           // t1 = t2 %N  (this is needed to reduce size of t2 before next calculation)
    mbedtls_mpi_exp_mod(&S, &t1, &b, &N, &_rr);  // S = t1^b %N

    // compute K = SHA512( S )                                          // Shared Session Key

    uint16_t S_size = mbedtls_mpi_size(&S);
    uint8_t S_buf[S_size];
    uint8_t S_hashed[64];

    mbedtls_mpi_write_binary(&S, S_buf, sizeof(S_buf));
    mbedtls_sha512_ret(S_buf, sizeof(S_buf), S_hashed, 0);              // create hash of data
    mbedtls_mpi_read_binary(&K, S_hashed, sizeof(S_hashed));            // load hash result into mpi structure K.  This is the SRP SHARED SECRET KEY
}

int HKSRP::get_shared_secret(uint8_t * buf) {
    return !mbedtls_mpi_write_binary(&K, buf, 64);
}

int HKSRP::verify_proof() {
    EHK_DEBUGLN("Verifying proof.");

    // Generate M1 and see if it matches with client's M1. 
    // M1 = H(H(N) XOR H(g) | H(I) | s | A | B | K)

    uint16_t N_size_stored = mbedtls_mpi_size(&N);
    uint8_t N_buf[N_size_stored];
    mbedtls_mpi_write_binary(&N, N_buf, sizeof(N_buf));

    uint8_t N_hash_buf[64];
    mbedtls_sha512_ret(N_buf, sizeof(N_buf), N_hash_buf, 0);

    uint8_t g_size = mbedtls_mpi_size(&g);
    uint8_t g_buf[g_size];
    mbedtls_mpi_write_binary(&g, g_buf, sizeof(g_buf));

    uint8_t g_hash_buf[64];
    mbedtls_sha512_ret(g_buf, sizeof(g_buf), g_hash_buf, 0);

    uint8_t N_h_xor_g_h[64];
    for (uint8_t i = 0; i < 64; i++) {
        N_h_xor_g_h[i] = N_hash_buf[i] ^ g_hash_buf[i];
    }

    uint8_t username_hashed[64];
    mbedtls_sha512_ret((const unsigned char *) username, strlen(username), username_hashed, 0);

    uint16_t s_size = mbedtls_mpi_size(&s);
    uint8_t s_buf[s_size];
    mbedtls_mpi_write_binary(&s, s_buf, sizeof(s_buf));

    uint16_t A_mpi_size = mbedtls_mpi_size(&A);
    uint8_t A_buf[A_mpi_size];
    mbedtls_mpi_write_binary(&A, A_buf, sizeof(A_buf));

    uint16_t B_mpi_size = mbedtls_mpi_size(&B);
    uint8_t B_buf[B_mpi_size];
    mbedtls_mpi_write_binary(&B, B_buf, sizeof(B_buf));

    uint8_t K_buf[64];
    mbedtls_mpi_write_binary(&K, K_buf, sizeof(K_buf));

    int ret = 0;
    uint8_t calculated_m1[64];
    mbedtls_sha512_context ctx;

    mbedtls_sha512_init(&ctx);
    ret += mbedtls_sha512_starts_ret(&ctx, 0);
    ret += mbedtls_sha512_update_ret(&ctx, N_h_xor_g_h, sizeof(N_h_xor_g_h));
    ret += mbedtls_sha512_update_ret(&ctx, username_hashed, sizeof(username_hashed));
    ret += mbedtls_sha512_update_ret(&ctx, s_buf, sizeof(s_buf));
    ret += mbedtls_sha512_update_ret(&ctx, A_buf, sizeof(A_buf));
    ret += mbedtls_sha512_update_ret(&ctx, B_buf, sizeof(B_buf));
    ret += mbedtls_sha512_update_ret(&ctx, K_buf, sizeof(K_buf));
    ret += mbedtls_sha512_finish_ret(&ctx, calculated_m1);
    mbedtls_sha512_free(&ctx);

    if (ret < 0) {
        EHK_DEBUGLN("Could not generate hash to validate proof.");
        return 0;
    }

    mbedtls_mpi computed_m1;
    mbedtls_mpi_init(&computed_m1);
    mbedtls_mpi_read_binary(&computed_m1, calculated_m1, sizeof(calculated_m1));

    if (mbedtls_mpi_cmp_mpi(&computed_m1, &M1) != 0) {
        EHK_DEBUGLN("\n\nComputed M1 and client M1 do not match.\n\n");
        mbedtls_mpi_free(&computed_m1);
        return 0;
    }

    mbedtls_mpi_free(&computed_m1);
    return 1;
}

void HKSRP::generate_proof() {
    EHK_DEBUGLN("Generating proof");
    // compute M2 = H( A | M1 | K )

    uint16_t A_size = mbedtls_mpi_size(&A);
    uint8_t A_buf[A_size];
    uint8_t M1_buf[64];
    uint8_t K_buf[64];

    mbedtls_mpi_write_binary(&A, A_buf, sizeof(A_buf));
    mbedtls_mpi_write_binary(&M1, M1_buf, sizeof(M1_buf));
    mbedtls_mpi_write_binary(&K, K_buf, sizeof(K_buf));

    uint8_t M2_hashed[64];

    int ret = 0;
    mbedtls_sha512_context ctx;

    mbedtls_sha512_init(&ctx);
    ret += mbedtls_sha512_starts_ret(&ctx, 0);
    ret += mbedtls_sha512_update_ret(&ctx, A_buf, sizeof(A_buf));
    ret += mbedtls_sha512_update_ret(&ctx, M1_buf, sizeof(M1_buf));
    ret += mbedtls_sha512_update_ret(&ctx, K_buf, sizeof(K_buf));
    ret += mbedtls_sha512_finish_ret(&ctx, M2_hashed);
    mbedtls_sha512_free(&ctx);

    if (ret < 0) {
        EHK_DEBUGLN("Error computing M2.");
        return;
    }

    mbedtls_mpi_read_binary(&M2, M2_hashed, sizeof(M2_hashed));
}

/////////// 
// SRP TLV 

int HKSRP::load_tlv(TLV * tlv, kTLVType tag) {
    mbedtls_mpi * mpi = NULL;

    uint16_t bytes = 0;

    switch (tag) {
    case kTLVType_PublicKey:
        mpi = &B;
        bytes = N_size;
        break;
    case kTLVType_Salt:
        mpi = &s;
        bytes = 16;
        break;
    case kTLVType_Proof:
        mpi = &M2;
        bytes = 64;
        break;
    default:
        return 0;
    }

    if (!mpi || !bytes) {
        return 0;
    }

    uint8_t * buf = tlv->buf(tag, bytes);

    if(!buf) {
        return 0;
    }

    return !mbedtls_mpi_write_binary(mpi, buf, bytes);
}

int HKSRP::write_tlv(TLV * tlv, kTLVType tag) {
    mbedtls_mpi * mpi = NULL;

    switch (tag) {
    case kTLVType_PublicKey:
        mpi = &A;           // Write HKClient's Public key
        break;
    case kTLVType_Proof:
        mpi = &M1;          // Write HKClient's Proof.
        break;
    default:
        return 0;
    }

    int bytes = tlv->len(tag);
 
    if (!mpi || bytes <= 0) {
        return 0;
    }

    mbedtls_mpi_read_binary(mpi, tlv->buf(tag), bytes);
    return(1);
}

void HKSRP::print_all() {
    EHK_DEBUG("\n\nN - Prime Number: \n");
    print(&N);
    EHK_DEBUG("\n\ng - generator: \n");
    print(&g);
    EHK_DEBUG("\n\nk - multiplier: \n");
    print(&k);
    EHK_DEBUG("\n\ns - salt: \n");
    print(&s);
    EHK_DEBUG("\n\nx - private key of username|password: \n");
    print(&x);
    EHK_DEBUG("\n\nv - verifier: \n");
    print(&v);
    EHK_DEBUG("\n\nb - server private key: \n");
    print(&b);
    EHK_DEBUG("\n\nB - server public key: \n");
    print(&B);
    EHK_DEBUG("\n\nu - u-factor: \n");
    print(&u);
    EHK_DEBUG("\n\nS - PreMaster Secret: \n");
    print(&S);
    EHK_DEBUG("\n\nK - Shared Session Key: \n");
    print(&K);
    EHK_DEBUG("\n\nA - Public Client Key: \n");
    print(&A);
    EHK_DEBUG("\n\nM1 - Proof client key: \n");
    print(&M1);
    EHK_DEBUG("\n\nM2 - proof server key: \n");
    print(&M2);
    EHK_DEBUGLN("");
}

void HKSRP::print(mbedtls_mpi *mpi) {
    char sBuf[2000];
    size_t sLen;

    mbedtls_mpi_write_string(mpi, 16, sBuf, 2000, &sLen);

    Serial.print((sLen-1)/2);         // subtract 1 for null-terminator, and then divide by 2 to get number of bytes (e.g. 4F = 2 characters, but represents just one mpi byte)
    Serial.print(" ");
    Serial.println(sBuf);
}

#endif