/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "u_assert.h"
#include "u_memory.h"

void *U_memalign(void *p, unsigned align)
{
    uintptr_t num;
    uintptr_t algn;
    void *p1;

    U_ASSERT(align == 1 || align == 4 || align == 8 || align == 16 || align == 32 || align == 64);

    num = (uintptr_t)p;
    algn = align;
    p1 = (void*)((num + (algn - 1)) & ~(algn - 1));

    U_ASSERT(p <= p1);
    U_ASSERT((((uintptr_t)p1) & (align - 1)) == 0);
    return p1;
}

void *U_memset(void *p, int c, unsigned long n)
{
    return memset(p, c, n);
}

int U_memcmp(const void *a, const void *b, unsigned long n)
{
    return memcmp(a, b, n);
}

unsigned U_strlen(const char *str)
{
    return strlen(str);
}

void *U_memcpy(void *dst, const void *src, unsigned long n)
{
    return memcpy(dst, src, n);
}

void *U_Alloc(unsigned long size)
{
    void *p;

    p = malloc(size);
    if (p)
    {
        U_memset(p, 0, size);
    }

    return p;
}

int U_Free(void *p)
{
    U_ASSERT(p != 0);

    if (p)
    {
        free(p);
        return 1;
    }
    return 0;
}
