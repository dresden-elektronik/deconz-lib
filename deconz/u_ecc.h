/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef U_ECC_H
#define U_ECC_H

#include "deconz/declspec.h"

typedef struct U_ECC_PrivateKeySecp256k1
{
    unsigned char key[32];
} U_ECC_PrivateKeySecp256k1;

typedef struct U_ECC_PublicKeySecp256k1
{
    unsigned char key[33]; /* compressed public key */
} U_ECC_PublicKeySecp256k1;

typedef struct U_ECC_SignatureSecp256k1
{
    unsigned char sig[64];
} U_ECC_SignatureSecp256k1;

#ifdef __cplusplus
extern "C" {
#endif

DECONZ_DLLSPEC int U_ECC_CreateKeyPairSecp256k1(U_ECC_PrivateKeySecp256k1 *, U_ECC_PublicKeySecp256k1 *);

DECONZ_DLLSPEC int U_ECC_SignSecp256K1(
                            const U_ECC_PrivateKeySecp256k1 *privkey,
                            const unsigned char *msghash,
                            unsigned hashsize,
                            U_ECC_SignatureSecp256k1 *sig);

DECONZ_DLLSPEC int U_ECC_VerifySignatureSecp256k1(
                            const U_ECC_PublicKeySecp256k1 *pubkey,
                            const U_ECC_SignatureSecp256k1 *sig,
                            const unsigned char *msghash,
                            unsigned hashsize);

#ifdef __cplusplus
}
#endif

#endif /* U_ECC_H */
