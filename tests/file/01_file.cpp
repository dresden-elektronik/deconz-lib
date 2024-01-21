#include <catch2/catch_test_macros.hpp>
#include <string.h>
#include "deconz/file.h"

static FS_File fp;
static const char *tmp_filename = "01_file.tmp";
static const char *txt = "Hello World!\n";
char buf[4096];

TEST_CASE( "Write and read file content", "[file]" )
{
    FS_DeleteFile(tmp_filename);

    long sz = strlen(txt);
    memset(buf, 0, sizeof(buf));

    REQUIRE(1 == FS_OpenFile(&fp, FS_MODE_RW, tmp_filename));
    REQUIRE(FS_WriteFile(&fp, txt, sz) == sz);
    REQUIRE(FS_CloseFile(&fp) == 1);

    REQUIRE(FS_OpenFile(&fp, FS_MODE_R, tmp_filename) == 1);

    REQUIRE(FS_GetFileSize(&fp) == sz);
    REQUIRE(FS_ReadFile(&fp, buf, sizeof(buf)-1) == sz);
    REQUIRE(FS_CloseFile(&fp) == 1);

    REQUIRE(memcmp(buf, txt, sz) == 0);
}
