#include <stdio.h>
#include "deconz/buffer_pool.h"

static BP_Page *BP_GetPagePtr(BP_BufferPool *bp, bp_page_id pagenum)
{
    unsigned i;
    BP_Page *page;

    /* TODO this part could use a hash table to not do linear search */
    for (i = 0; i < bp->n_frames; i++)
    {
        page = &bp->pages[i];
        if (page->page == pagenum && page->flags & BP_PAGE_FLAG_LOADED)
        {
            return page;
        }
    }

    return 0;
}

static void BP_StatPageFile(BP_BufferPool *bp)
{
    long n;
    n = FS_GetFileSize(&bp->file);
    bp->n_pages_in_file = 0;

    if (n >= BP_PAGE_SIZE)
        bp->n_pages_in_file = n / BP_PAGE_SIZE;
}

static int BP_WriteDirtyPage(BP_BufferPool *bp, bp_page_id pagenum)
{
    BP_Page *page;
    BP_Frame *frame;

    page = BP_GetPagePtr(bp, pagenum);

    if (page && (page->flags & BP_PAGE_FLAG_DIRTY))
    {
        frame = &bp->frames[page->frame];
        if (FS_SeekFile(&bp->file, pagenum * BP_PAGE_SIZE, FS_SEEK_SET))
        {
            if (FS_WriteFile(&bp->file, frame->data, BP_PAGE_SIZE) == BP_PAGE_SIZE)
            {
                page->flags &= ~BP_PAGE_FLAG_DIRTY;
                return 1;
            }
        }
    }

    return 0;
}

int BP_Init(BP_BufferPool *bp, const char *path, BP_Frame *frames, BP_Page *pages, unsigned n_frames)
{
    unsigned i;

    bp->frames = 0;
    bp->pages = 0;
    bp->n_frames = 0;
    bp->file.fd = 0;
    bp->clock_cursor = 0;
    bp->n_pages_in_file = 0;

    if (FS_OpenFile(&bp->file, FS_MODE_RW, path))
    {
        bp->frames = frames;
        bp->pages = pages;
        bp->n_frames = n_frames;

        for (i = 0; i < n_frames; i++)
        {
            pages[i].flags = 0;
            pages[i].frame = 0;
            pages[i].page = 0;
        }

        BP_StatPageFile(bp);
        return 1;
    }

    return 0;
}

void BP_Flush(BP_BufferPool *bp)
{
    unsigned i;

    for (i = 0; i < bp->n_frames; i++)
    {
        if (bp->pages[i].flags & BP_PAGE_FLAG_DIRTY)
            BP_WriteDirtyPage(bp, bp->pages[i].page);
    }
}

void BP_Destroy(BP_BufferPool *bp)
{
    FS_CloseFile(&bp->file);
    bp->n_frames = 0;
    bp->frames = 0;
    bp->pages = 0;
}

int BP_LoadPage(BP_BufferPool *bp, bp_page_id pagenum, BP_PageData *dat)
{
    long n;
    unsigned i;
    bp_page_id dirty_page;
    BP_Page *page;
    BP_Frame *frame;

    dat->page_id = pagenum;
    dat->data = 0;

    page = BP_GetPagePtr(bp, pagenum);
    if (page)
    {
        page->flags |= BP_PAGE_FLAG_ACCESS;
        dat->data = bp->frames[page->frame].data;
        return 1;
    }

again:
    dirty_page = bp->n_pages_in_file; /* invalid */

    for (n = 0; n < (long)bp->n_frames; bp->clock_cursor++, n++)
    {
        i = bp->clock_cursor % bp->n_frames;
        page = &bp->pages[i];

        if (page->flags & BP_PAGE_FLAG_DIRTY)
        {
            if (dirty_page == bp->n_pages_in_file)
                dirty_page = page->page;

            continue;
        }

/*        if (page->flags & BP_PAGE_FLAG_LOCKED)
            continue;*/

        if (page->flags & BP_PAGE_FLAG_ACCESS)
        {
            page->flags &= ~BP_PAGE_FLAG_ACCESS;
            continue;
        }

        break;
    }

    if (n == (long)bp->n_frames)
    {
        /*
         * If there is a dirty page write it to disk to make room.
         */
        if (dirty_page < bp->n_pages_in_file)
        {
            if (BP_WriteDirtyPage(bp, dirty_page))
                goto again;
        }

        return 0; /* TODO handle no space found */
    }

    page->flags = 0;
    page->frame = i;
    page->page = pagenum;
    frame = &bp->frames[i];

    if (FS_SeekFile(&bp->file, pagenum * 4096, FS_SEEK_SET))
    {
        n = FS_ReadFile(&bp->file, frame->data, BP_PAGE_SIZE);
        if (n == BP_PAGE_SIZE)
        {
            page->flags = BP_PAGE_FLAG_LOADED;
            dat->data = frame->data;
            return 1;
        }
    }

    return 0;
}

void BP_MarkPageDirty(BP_BufferPool *bp, bp_page_id pagenum)
{
    BP_Page *page;

    page = BP_GetPagePtr(bp, pagenum);
    if (page)
        page->flags |= BP_PAGE_FLAG_DIRTY;
}

int BP_AllocPage(BP_BufferPool *bp, BP_PageData *dat)
{
    if (FS_TruncateFile(&bp->file, (bp->n_pages_in_file + 1) * BP_PAGE_SIZE))
    {
        dat->page_id = bp->n_pages_in_file;
        bp->n_pages_in_file += 1;

        if (BP_LoadPage(bp, dat->page_id, dat))
            return 1;
    }

    return 0;
}

int BP_Truncate(BP_BufferPool *bp, unsigned n)
{
    if (FS_TruncateFile(&bp->file, n * BP_PAGE_SIZE))
    {
        BP_StatPageFile(bp);
        return 1;
    }

    return 0;
}