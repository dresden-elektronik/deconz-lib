#include "deconz/u_assert.h"
#include "deconz/u_ecc.h"
#include "deconz/u_sha256.h"
#include "deconz/u_memory.h"
#include "deconz/u_random.h"
#include "uECC.h"

#define SHA256_BLOCK_LENGTH  64
#define SHA256_DIGEST_LENGTH 32

/* needed for uECC_sign_deterministic() */
typedef struct SHA256_HashContext
{
    uECC_HashContext uECC;
    int sha256err;
    unsigned long pos;
    unsigned char buf[256];
} SHA256_HashContext;

static void init_SHA256(const uECC_HashContext *base)
{
    SHA256_HashContext *ctx = (SHA256_HashContext *)base;
    ctx->sha256err = 0;
    ctx->pos = 0;
}

static void update_SHA256(const uECC_HashContext *base, const uint8_t *msg, unsigned msg_size)
{
    SHA256_HashContext *ctx = (SHA256_HashContext *)base;

    if (ctx->pos + msg_size < sizeof(ctx->buf))
    {
        if (msg_size)
        {
            U_memcpy(&ctx->buf[ctx->pos], msg, msg_size);
            ctx->pos += msg_size;
        }
    }
    else
    {
        U_ASSERT(0 && "SHA256 buffer too small");
        ctx->sha256err = 1;
    }
}

static void finish_SHA256(const uECC_HashContext *base, uint8_t *hash_result)
{
    SHA256_HashContext *ctx = (SHA256_HashContext *)base;

    U_Sha256(&ctx->buf[0], ctx->pos, hash_result);
}

static int uECC_RNG_Callback(unsigned char *dest, unsigned size)
{
    return U_RandomBytes(dest, size);
}

int U_ECC_CreateKeyPairSecp256k1(U_ECC_PrivateKeySecp256k1 *privkey, U_ECC_PublicKeySecp256k1 *pubkey)
{
    const struct uECC_Curve_t *curve;
    unsigned char public_key[64]; /* uncompressed public key*/

    if (!privkey || !pubkey)
        return 0;

    uECC_set_rng(uECC_RNG_Callback);
    curve = uECC_secp256k1();

    if (uECC_make_key(public_key, privkey->key, curve) != 1)
    {
        return 0;
    }

    uECC_compress(public_key, pubkey->key, curve);

    return 1;
}

int U_ECC_SignSecp256K1(const U_ECC_PrivateKeySecp256k1 *privkey,
                        const unsigned char *msghash,
                        unsigned hashsize,
                        U_ECC_SignatureSecp256k1 *sig)
{
    SHA256_HashContext shactx;
    uECC_HashContext *ctx;
    const struct uECC_Curve_t *curve;
    unsigned char public_key[64]; /* uncompressed public key*/
    uint8_t tmp[2 * SHA256_DIGEST_LENGTH + SHA256_BLOCK_LENGTH];

    if (!privkey || !sig)
        return 0;

    if (!msghash || hashsize != SHA256_DIGEST_LENGTH)
        return 0;

    curve = uECC_secp256k1();
    uECC_set_rng(uECC_RNG_Callback);

    if (uECC_compute_public_key(privkey->key, public_key, curve) != 1)
        return 0;

    /* note: uECC_HashContext is embedded in SHA256_HashContext */
    shactx.sha256err = 0;
    ctx = &shactx.uECC;
    ctx->init_hash = init_SHA256;
    ctx->update_hash = update_SHA256;
    ctx->finish_hash = finish_SHA256;
    ctx->block_size = SHA256_BLOCK_LENGTH;
    ctx->result_size = SHA256_DIGEST_LENGTH;
    ctx->tmp = &tmp[0];

    if (uECC_sign_deterministic(privkey->key,
                                msghash,
                                hashsize,
                                ctx,
                                sig->sig,
                                curve) != 1 || shactx.sha256err)
    {
        return 0;
    }

    return 1;
}

int U_ECC_VerifySignatureSecp256k1(
                            const U_ECC_PublicKeySecp256k1 *pubkey,
                            const U_ECC_SignatureSecp256k1 *sig,
                            const unsigned char *msghash,
                            unsigned hashsize)
{
    const struct uECC_Curve_t *curve;
    unsigned char public_key[64]; /* uncompressed public key*/

    if (!pubkey || !sig)
        return 0;

    if (!msghash || hashsize != SHA256_DIGEST_LENGTH)
        return 0;

    curve = uECC_secp256k1();
    uECC_set_rng(uECC_RNG_Callback);

    uECC_decompress(pubkey->key, public_key, curve);

    if (uECC_verify(public_key, msghash, hashsize, sig->sig, curve) == 1)
        return 1;

    return 0;
}
