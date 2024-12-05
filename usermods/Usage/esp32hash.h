//
// Created by will on 03/12/24.
//

#ifndef ESP32HASH_H
#define ESP32HASH_H

#include "mbedtls/md.h"

inline String sha1(const String& data) {
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA1;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (const unsigned char*)data.c_str(), data.length());
    unsigned char hash[20];
    mbedtls_md_finish(&ctx, hash);
    mbedtls_md_free(&ctx);

    String hashStr((const char*)nullptr);
    hashStr.reserve(20 * 2 + 1);

    for(unsigned char i : hash) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02x", i);
        hashStr += hex;
    }

    return hashStr;
}

#endif //ESP32HASH_H
