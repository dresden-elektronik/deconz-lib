#include <catch2/catch_test_macros.hpp>
#include "deconz/buffer_pool.h"

#define N_FRAMES 8

static BP_Page pages[N_FRAMES];
static BP_Frame frames[N_FRAMES];
static BP_BufferPool buffer_pool;
static BP_BufferPool *bp = &buffer_pool;
static const char *bp_path = "01_fetch.bp";

TEST_CASE( "Alloc page 1", "[buffer_pool]" )
{
    BP_PageData dat;
    dat.data = nullptr;

    REQUIRE(BP_Init(bp, bp_path, frames, pages, N_FRAMES) == 1);
    REQUIRE(BP_Truncate(bp, 1) == 1);
    REQUIRE(BP_AllocPage(&buffer_pool, &dat) == 1);
    REQUIRE(dat.page_id == 1);
    REQUIRE(dat.data != 0);

    BP_Destroy(bp);
}

TEST_CASE( "Init page 0", "[buffer_pool]" )
{
    BP_PageData dat;

    dat.page_id = ~0;
    dat.data = nullptr;

    REQUIRE(BP_Init(bp, bp_path, frames, pages, N_FRAMES) == 1);
    REQUIRE(buffer_pool.n_frames == N_FRAMES);
    REQUIRE(BP_Truncate(bp, 0) == 1);
    REQUIRE(BP_AllocPage(&buffer_pool, &dat) == 1);
    REQUIRE(BP_LoadPage(bp, 0, &dat) == 1);
    REQUIRE(dat.page_id == 0);
    REQUIRE(dat.data != 0);

    dat.page_id = ~0;
    dat.data = nullptr;

    REQUIRE(BP_LoadPage(bp, 0, &dat) == 1);
    REQUIRE(dat.page_id == 0);
    REQUIRE(dat.data != 0);

    dat.data[0] = 0x66;
    dat.data[3] = 0xAA;
    BP_MarkPageDirty(bp, 0);
    BP_Flush(bp);
    BP_Destroy(bp);
}

TEST_CASE( "Init page 1", "[buffer_pool]" )
{
    BP_PageData dat;
    bp_page_id page_id = 1;

    dat.page_id = ~0;
    dat.data = nullptr;

    REQUIRE(BP_Init(bp, bp_path, frames, pages, N_FRAMES) == 1);
    REQUIRE(BP_Truncate(bp, 2));
    REQUIRE(BP_LoadPage(&buffer_pool, page_id, &dat) == 1);
    REQUIRE(dat.page_id == 1);
    REQUIRE(dat.data != 0);

    dat.data[0] = '0';
    dat.data[1] = 'K';
    dat.data[2] = '_';
    dat.data[3] = '_';
    BP_MarkPageDirty(&buffer_pool, page_id);
    BP_Flush(&buffer_pool);
    BP_Destroy(bp);
}

TEST_CASE( "Fail load non existing page", "[buffer_pool]" )
{
    BP_PageData dat;
    bp_page_id page_id = 0;

    dat.page_id = ~0;
    dat.data = nullptr;

    REQUIRE(BP_Init(bp, bp_path, frames, pages, N_FRAMES) == 1);
    REQUIRE(BP_Truncate(bp, 0) == 1);
    REQUIRE(BP_LoadPage(&buffer_pool, page_id, &dat) == 0);
    REQUIRE(dat.page_id == page_id);
    REQUIRE(dat.data == 0);

    BP_Destroy(bp);
}
