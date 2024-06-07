/*
 * Copyright (c) 2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <stdio.h>
#include <stdlib.h> /* malloc, free */
#include <string.h> /* memset */
#include <stdint.h>

#include "deconz/atom_table.h"

#define ATOM_PAGE_SIZE (4096 - 8)

typedef struct AT_Page
{
    struct AT_Page *next;
    unsigned char data[ATOM_PAGE_SIZE];
} AT_Page;

/*
 * TODO(mpi): make thread safe via u_threads.
 */
static unsigned atom_table_size;
static unsigned char *atom_page_beg;
static unsigned char *atom_page_end;
static unsigned atom_pages_byte_count;
static AT_Page *atom_pages;
static AT_Atom *atom_table;
static unsigned atom_count;

/*
 * https://maskray.me/blog/2023-04-12-elf-hash-function
 *
 * PJW hash adapted from musl libc.
 */
static uint32_t AT_Hash(const void *s0, unsigned size)
{
    uint32_t h;
    const unsigned char *s;

    h = 0;
    s = (const unsigned char*)s0;

    while (size--)
    {
        h = 16 * h + *s++;
        h ^= h >> 24 & 0xF0;
    }
    return h & 0xfffffff;
}

void *AT_Alloc(unsigned long size)
{
    return malloc(size);
}

void AT_Free(void *ptr)
{
    if (ptr)
        free(ptr);
}

static void AT_AllocPage(void)
{
    AT_Page *page;

    page = (AT_Page*)AT_Alloc(sizeof(*page));
    page->next = atom_pages;
    atom_pages = page;

    atom_page_beg = &page->data[0];
    atom_page_end = &page->data[sizeof(page->data)];

    atom_pages_byte_count += sizeof(*page);
//    printf("atom pages: %u bytes\n", atom_pages_byte_count);
}

static unsigned char *AT_AllocPageData(unsigned size)
{
    unsigned char *p = 0;

    size += 1; /* terminating '\0' */
    if (atom_page_beg + size > atom_page_end)
    {
        AT_AllocPage();
    }

    if (atom_page_beg + size <= atom_page_end)
    {
        p = atom_page_beg;
        atom_page_beg += size;
    }
    return p;
}

void AT_Init(unsigned max_atoms)
{
    unsigned i;
    unsigned long size;

    atom_table_size = max_atoms;
    atom_pages = 0;
    atom_pages_byte_count = 0;
    atom_count = 0;

    size = max_atoms * sizeof(*atom_table);
    atom_table = (AT_Atom*)AT_Alloc(size);

    for (i = 0; i < atom_table_size; i++)
    {
        atom_table[i].len = 0;
        atom_table[i].data = 0;
    }

    atom_table[0].len = 3;
    atom_table[0].data = AT_AllocPageData(3); /* allocates also '\0' */;
    atom_table[0].data[0] = 'N';
    atom_table[0].data[1] = 'U';
    atom_table[0].data[2] = 'L';
    atom_table[0].data[3] = '\0';
}

void AT_Destroy(void)
{
    AT_Page *np;

    for (; atom_pages;)
    {
        np = atom_pages;
        atom_pages = np->next;
        AT_Free(np);
    }

    atom_pages_byte_count = 0;

    AT_Free(atom_table);
    atom_table = 0;
    atom_table_size = 0;
    atom_count = 0;
}

int AT_AddAtomString(const void *data)
{
    unsigned size = 0;
    const char *s = data;

    if (s)
    {
        for (size = 0; s[size]; size++)
        {}
        return AT_AddAtom(data, size, 0);
    }

    return 0;
}

int AT_AddAtom(const void *data, unsigned size, AT_AtomIndex *ati)
{
    unsigned i;
    unsigned idx;
    unsigned long hash;
    const unsigned char *bytes;
    AT_Atom *a;

    if (data && size && size <= AT_MAX_ATOM_SIZE && atom_count < atom_table_size)
    {
        hash = AT_Hash(data, size);

        for (i = 0; i < atom_table_size; i++)
        {
            idx = (hash + i) % atom_table_size;
            a = &atom_table[idx];
            if (a->len == size)
            {
                if (memcmp(data, a->data, size) == 0)
                {
                    /* already existing */
                    if (ati)
                        ati->index = idx;
                    return 1;
                }
            }
            else if (a->len == 0)
            {
                a->data = AT_AllocPageData(size); /* allocates also '\0' */
                if (a->data)
                {
                    a->len = size;
                    bytes = data;
                    for (i = 0; i < size; i++)
                        a->data[i] = bytes[i];

                    a->data[size] = '\0';
//                    printf("add atom[%u] %s\n", atom_count, (const char*)a->data);
                    atom_count++;

                    if (ati)
                        ati->index = idx;

                    return 1;
                }
            }
        }
    }

    if (ati)
        ati->index = 0;

    return 0;
}

int AT_GetAtomIndex(const void *data, unsigned size, AT_AtomIndex *ati)
{
    unsigned i;
    unsigned idx;
    unsigned long hash;
    AT_Atom *a;

    if (data && size && size <= AT_MAX_ATOM_SIZE && ati)
    {
        hash = AT_Hash(data, size);

        for (i = 0; i < atom_table_size; i++)
        {
            idx = (hash + i) % atom_table_size;
            a = &atom_table[idx];
            if (a->len == size)
            {
                if (memcmp(data, a->data, size) == 0)
                {
                    ati->index = idx;
                    return 1;
                }
            }
            else if (a->len == 0)
            {
                break; /* nothing found */
            }
        }
    }

    if (ati)
        ati->index = 0;

    return 0;
}

AT_Atom AT_GetAtomByIndex(AT_AtomIndex ati)
{
    if (ati.index < atom_table_size)
    {
        return atom_table[ati.index];
    }

    AT_Atom a;
    a.data = 0;
    a.len = 0;
    return a;
}
