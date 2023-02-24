// https://github.com/slompf18/esp32_hap/blob/master/src/crypto/hk_srp.c
// https://github.com/secure-remote-password/test-vectors
#pragma once

#include "HKTLV.h"

#include <mbedtls/bignum.h>
#include <stdint.h>

class HKSRP {
   private:

    // HAP Accessory info

    mbedtls_mpi N;  // N                            - 3072-bit Group pre-defined prime used for all SRP-6A calculations (384 bytes)
    mbedtls_mpi g;  // g                            - pre-defined generator for the specified 3072-bit Group (g=5)
    mbedtls_mpi k;  // k = H(N | PAD(g))            - SRP-6A multiplier (which is different from versions SRP-6 or SRP-3)
    mbedtls_mpi s;  // s                            - randomly-generated salt (16 bytes)
    mbedtls_mpi x;  // x = H(s | H(I | ":" | P))    - private key salted, double-hash of username and password (64 bytes)
    mbedtls_mpi v;  // v = g^x %N                   - SRP-6A verifier (max 384 bytes)
    mbedtls_mpi b;  // b                            - randomly-generated private key for this HAP accessory (i.e. the SRP Server) (32 bytes)
    mbedtls_mpi B;  // B = k*v + g^b %N             - public key for this accessory (max 384 bytes)
    mbedtls_mpi u;  // u = H(PAD(A) | PAB(B))       - "u", derived of A and B (64 bytes)
    mbedtls_mpi S;  // S = (A*v^u)^b %N             - SRP shared "premaster secret" key, computed by client and server
    mbedtls_mpi K;  // K = H( S )                   - SRP SHARED SECRET KEY (64 bytes)

    //  HAP Client values.

    mbedtls_mpi A;    // A                          - public key RECEIVED from HAP Client (max 384 bytes)
    mbedtls_mpi M1;   // M1                         - proof RECEIVED from HAP Client (64 bytes)
    mbedtls_mpi M2;   // M2 = H(A, M1, K)           - accessory's counter-proof to send to HAP Client after M1=M1V has been verified (64 bytes)

    // Temporary structures internal use only.

    mbedtls_mpi t1;
    mbedtls_mpi t2;
    mbedtls_mpi t3;
    mbedtls_mpi _rr;  // _rr                          - temporary "helper" for large exponential modulus calculations

    const char username[11] = "Pair-Setup";
    const uint8_t g3072 = 5;

    void generate_private_key();

    void print(mbedtls_mpi * mpi);

   public:
    HKSRP();

    int get_shared_secret(uint8_t * buf);

    void generate_verifier(const char* verify_code);
    void generate_public_key();
    void compute_key();
    void generate_proof();

    int verify_proof();


    int load_tlv(TLV* tlv, kTLVType tag);
    int write_tlv(TLV* tlv, kTLVType tag);

    void print_all();
};