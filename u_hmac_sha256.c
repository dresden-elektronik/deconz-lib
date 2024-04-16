/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include "deconz/u_assert.h"
#include "deconz/u_hmac_sha256.h"
#include "deconz/u_sha256.h"

/*
#define HMAC_SHA256_TEST
*/

#define SHA256_BLOCK_SIZE (512 / 8)

#ifdef HMAC_SHA256_TEST

/*
 * Test vectors: https://datatracker.ietf.org/doc/html/rfc4231
 */
void U_HmacSha256Test1(void)
{
    unsigned i;
    unsigned char wrk[512];
    unsigned char result[U_SHA256_HASH_SIZE];

    const char key[20] = {0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
                        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
                        0x0b, 0x0b, 0x0b, 0x0b };
    const char *msg = "Hi There";

    unsigned char expect[U_SHA256_HASH_SIZE] = {
        0xb0, 0x34, 0x4c, 0x61, 0xd8, 0xdb, 0x38, 0x53,
        0x5c, 0xa8, 0xaf, 0xce, 0xaf, 0x0b, 0xf1, 0x2b,
        0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83, 0x3d, 0xa7,
        0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7,
    };

    if (U_HmacSha256(key, sizeof(key), msg, 8, wrk, sizeof(wrk), result) == 0)
    {
        U_ASSERT(0 && "HMAC-SHA256 test1 failed");
    }

    for (i = 0; i < U_SHA256_HASH_SIZE; i++)
    {
        U_ASSERT(expect[i] == result[i]);
    }
}

void U_HmacSha256Test2(void)
{
    unsigned i;
    unsigned char wrk[512];
    unsigned char result[U_SHA256_HASH_SIZE];

    const char key[4] = { 0x4a, 0x65, 0x66, 0x65 };
    const char *msg = "what do ya want for nothing?";

    unsigned char expect[U_SHA256_HASH_SIZE] = {
        0x5b, 0xdc, 0xc1, 0x46, 0xbf, 0x60, 0x75, 0x4e,
        0x6a, 0x04, 0x24, 0x26, 0x08, 0x95, 0x75, 0xc7,
        0x5a, 0x00, 0x3f, 0x08, 0x9d, 0x27, 0x39, 0x83,
        0x9d, 0xec, 0x58, 0xb9, 0x64, 0xec, 0x38, 0x43
    };

    if (U_HmacSha256(key, sizeof(key), msg, 28, wrk, sizeof(wrk), result) == 0)
    {
        U_ASSERT(0 && "HMAC-SHA256 test2 failed");
    }

    for (i = 0; i < U_SHA256_HASH_SIZE; i++)
    {
        U_ASSERT(expect[i] == result[i]);
    }
}

void U_HmacSha256Test3(void)
{
    unsigned i;
    unsigned char wrk[512];
    unsigned char result[U_SHA256_HASH_SIZE];

    char key[20];
    char msg[50];

    for (i = 0; i < sizeof(key); i++)
        key[i] = 0xaa;

    for (i = 0; i < sizeof(msg); i++)
        msg[i] = 0xdd;

    unsigned char expect[U_SHA256_HASH_SIZE] = {
        0x77, 0x3e, 0xa9, 0x1e, 0x36, 0x80, 0x0e, 0x46,
        0x85, 0x4d, 0xb8, 0xeb, 0xd0, 0x91, 0x81, 0xa7,
        0x29, 0x59, 0x09, 0x8b, 0x3e, 0xf8, 0xc1, 0x22,
        0xd9, 0x63, 0x55, 0x14, 0xce, 0xd5, 0x65, 0xfe
    };

    if (U_HmacSha256(key, sizeof(key), msg, sizeof(msg), wrk, sizeof(wrk), result) == 0)
    {
        U_ASSERT(0 && "HMAC-SHA256 test3 failed");
    }

    for (i = 0; i < U_SHA256_HASH_SIZE; i++)
    {
        U_ASSERT(expect[i] == result[i]);
    }
}

void U_HmacSha256Test4(void)
{
    unsigned i;
    unsigned char wrk[512];
    unsigned char result[U_SHA256_HASH_SIZE];

    char key[25];
    char msg[50];

    for (i = 0; i < sizeof(key); i++)
        key[i] = i + 1;

    for (i = 0; i < sizeof(msg); i++)
        msg[i] = 0xcd;

    unsigned char expect[U_SHA256_HASH_SIZE] = {
        0x82, 0x55, 0x8a, 0x38, 0x9a, 0x44, 0x3c, 0x0e,
        0xa4, 0xcc, 0x81, 0x98, 0x99, 0xf2, 0x08, 0x3a,
        0x85, 0xf0, 0xfa, 0xa3, 0xe5, 0x78, 0xf8, 0x07,
        0x7a, 0x2e, 0x3f, 0xf4, 0x67, 0x29, 0x66, 0x5b
    };

    if (U_HmacSha256(key, sizeof(key), msg, sizeof(msg), wrk, sizeof(wrk), result) == 0)
    {
        U_ASSERT(0 && "HMAC-SHA256 test4 failed");
    }

    for (i = 0; i < U_SHA256_HASH_SIZE; i++)
    {
        U_ASSERT(expect[i] == result[i]);
    }
}

void U_HmacSha256Test6(void)
{
    unsigned i;
    unsigned msglen;
    unsigned char wrk[512];
    unsigned char result[U_SHA256_HASH_SIZE];

    char key[131];
    const char *msg = "Test Using Larger Than Block-Size Key - Hash Key First";

    for (i = 0; i < sizeof(key); i++)
        key[i] = 0xaa;

    for (msglen = 0; msg[msglen]; msglen++)
        ;

    unsigned char expect[U_SHA256_HASH_SIZE] = {
        0x60, 0xe4, 0x31, 0x59, 0x1e, 0xe0, 0xb6, 0x7f,
        0x0d, 0x8a, 0x26, 0xaa, 0xcb, 0xf5, 0xb7, 0x7f,
        0x8e, 0x0b, 0xc6, 0x21, 0x37, 0x28, 0xc5, 0x14,
        0x05, 0x46, 0x04, 0x0f, 0x0e, 0xe3, 0x7f, 0x54
    };

    if (U_HmacSha256(key, sizeof(key), msg, msglen, wrk, sizeof(wrk), result) == 0)
    {
        U_ASSERT(0 && "HMAC-SHA256 test6 failed");
    }

    for (i = 0; i < U_SHA256_HASH_SIZE; i++)
    {
        U_ASSERT(expect[i] == result[i]);
    }
}

#endif /* HMAC_SHA256_TEST */

void U_HmacSha256Test(void)
{
#ifdef HMAC_SHA256_TEST

    U_HmacSha256Test1();
    U_HmacSha256Test2();
    U_HmacSha256Test3();
    U_HmacSha256Test4();

    U_HmacSha256Test6();

#endif
}


/*
 *
 * https://datatracker.ietf.org/doc/html/rfc2104
 */

int U_HmacSha256(const void *key,
                 unsigned keysize,
                 const void *msg,
                 unsigned msgsize,
                 unsigned char *wrk,
                 unsigned wrksize,
                 unsigned char *result)
{
    unsigned i;
    unsigned char *k;
    unsigned char *h1;
    unsigned char *step1;

    U_ASSERT(key);
    U_ASSERT(keysize != 0);
    U_ASSERT(msg);
    U_ASSERT(msgsize != 0);
    U_ASSERT(wrk);
    U_ASSERT(wrksize >= (SHA256_BLOCK_SIZE * 2) + msgsize);

    if (wrksize < (SHA256_BLOCK_SIZE * 2) + msgsize)
        return 0; // not enough memory

    step1 = wrk;

    /* Applications that use keys longer than B bytes will first hash the key
       using H and then use the resultant L byte string as the actual key to HMAC. */

    if (keysize > SHA256_BLOCK_SIZE)
    {
        if (U_Sha256(key, keysize, step1) == 0)
            return 0;

        i = U_SHA256_HASH_SIZE;
    }
    else
    {
        for (i = 0; i < keysize && i < SHA256_BLOCK_SIZE; i++)
            step1[i] = ((const unsigned char*)key)[i];
    }

    /* (1) append zeros to the end of K to create a B byte string
        (e.g., if K is of length 20 bytes and B=64, then K will be
         appended with 44 zero bytes 0x00) */
    for (; i < SHA256_BLOCK_SIZE; i++)
        step1[i] = 0;

    k = &step1[SHA256_BLOCK_SIZE];

    /* (2) XOR (bitwise exclusive-OR) the B byte string computed in step
        (1) with ipad */
    for (i = 0; i < SHA256_BLOCK_SIZE; i++)
        k[i] = step1[i] ^ 0x36;

    /*(3) append the stream of data 'text' to the B byte string resulting
        from step (2) */
    for (i = 0; i < msgsize; i++)
        k[SHA256_BLOCK_SIZE + i] = ((const unsigned char*)msg)[i];

    h1 = &k[SHA256_BLOCK_SIZE + U_SHA256_HASH_SIZE + msgsize]; /* U_SHA256_HASH_SIZE to have extra space for step 6 */
    /*(4) apply H to the stream generated in step (3) */
    if (U_Sha256(k, SHA256_BLOCK_SIZE + msgsize, h1) == 0)
        return 0;

    /*(5) XOR (bitwise exclusive-OR) the B byte string computed in
        step (1) with opad */
    for (i = 0; i < SHA256_BLOCK_SIZE; i++)
        k[i] = step1[i] ^ 0x5C;


    /*(6) append the H result from step (4) to the B byte string
        resulting from step (5) */
    for (i = 0; i < U_SHA256_HASH_SIZE; i++)
        k[SHA256_BLOCK_SIZE + i] = h1[i];

    /*(7) apply H to the stream generated in step (6) and output
        the result */
    if (U_Sha256(k, SHA256_BLOCK_SIZE + U_SHA256_HASH_SIZE, result) == 1)
        return 1;

    return 0;
}
