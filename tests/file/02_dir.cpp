#include <catch2/catch_test_macros.hpp>
#include "deconz/file.h"

TEST_CASE( "Open and read current directory", "[dir]" )
{
    FS_Dir dir;

    REQUIRE(1 == FS_OpenDir(&dir, "."));
    REQUIRE(1 == FS_ReadDir(&dir));
    REQUIRE(dir.entry.name[0] != '\0');
    REQUIRE(dir.entry.type != FS_TYPE_UNKNOWN);
    REQUIRE(1 == FS_CloseDir(&dir));
}
