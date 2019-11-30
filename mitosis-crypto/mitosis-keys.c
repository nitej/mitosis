#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "mitosis-crypto.h"

bool mitosis_crypto_init(mitosis_crypto_context_t* context, bool left) {
    bool result = true;
    uint8_t ikm[sizeof((uint8_t[])MITOSIS_MASTER_SECRET_SEED)] = MITOSIS_MASTER_SECRET_SEED;
    uint8_t prk[MITOSIS_HMAC_OUTPUT_SIZE];

    result =
        mitosis_hkdf_extract(
            ikm, sizeof(ikm),
            (uint8_t*)(left ? MITOSIS_LEFT_SALT : MITOSIS_RIGHT_SALT),
            (left ? sizeof(MITOSIS_LEFT_SALT) : sizeof(MITOSIS_RIGHT_SALT)),
            prk);
    if(!result) {
        return result;
    }

    result =
        mitosis_hkdf_expand(
            prk, sizeof(prk),
            (uint8_t*)MITOSIS_ENCRYPT_KEY_INFO, sizeof(MITOSIS_ENCRYPT_KEY_INFO),
            context->encrypt.ctr.key, sizeof(context->encrypt.ctr.key));
    if(!result) {
        return result;
    }

    result =
        mitosis_hkdf_expand(
            prk, sizeof(prk),
            (uint8_t*)MITOSIS_NONCE_INFO,
            sizeof(MITOSIS_NONCE_INFO),
            context->encrypt.ctr.iv_bytes, sizeof(context->encrypt.ctr.iv_bytes));
    if(!result) {
        return result;
    }

    // Initialize counter to zero.
    context->encrypt.ctr.iv.counter = 0;

    // prk can be overwritten here because mitosis_hkdf_expand is done with it
    // by the time that output is being written.
    result =
        mitosis_hkdf_expand(
            prk, sizeof(prk),
            (uint8_t*)MITOSIS_CMAC_KEY_INFO, sizeof(MITOSIS_CMAC_KEY_INFO),
            prk, AES_BLOCK_SIZE);
    if(!result) {
        return result;
    }

    result = mitosis_cmac_init(&(context->cmac), prk, sizeof(prk));
    if(!result) {
        return result;
    }

    result = mitosis_aes_ecb_init(&(context->encrypt.ecb));

    return result;
}
