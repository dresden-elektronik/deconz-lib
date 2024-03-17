#include <catch2/catch_test_macros.hpp>
#include <string.h>
#include "deconz/u_sha256.h"

static const char *data1 = "This is some test data\n";

static const unsigned char result1[U_SHA256_HASH_SIZE] = {
       0x70, 0xdc, 0x63, 0xc4, 0x70, 0x5d, 0x42, 0x52,
       0x70, 0x0b, 0x0f, 0x8f, 0xc3, 0x60, 0xd9, 0x6c,
       0xf4, 0x63, 0x55, 0x90, 0x25, 0xbc, 0x74, 0xd7,
       0xc6, 0x5d, 0x66, 0x67, 0x92, 0x82, 0x31, 0x55
};

TEST_CASE( "SHA256 calculation test", "[sha256]" )
{
    unsigned char hash[U_SHA256_HASH_SIZE];
    long sz = strlen(data1);

    REQUIRE(1 == U_Sha256(data1, sz, hash));
    REQUIRE(0 == memcmp(hash, result1, U_SHA256_HASH_SIZE));
}
