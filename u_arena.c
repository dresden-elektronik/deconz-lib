/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include "u_assert.h"
#include "u_arena.h"
#include "u_memory.h"

#define U_ARENA_INVALID_PTR 0xFFFFFFFFU
#define U_ARENA_SIZE_MASK 0x7FFFFFFFU
#define U_ARENA_STATIC_MEM_FLAG 0x80000000U

/*
 * arena._total_size STATIC_MEM_FLAG is set in _total_size when the memory
 * is not owned by the arena.
 */

void U_InitArena(U_Arena *arena, unsigned long size)
{
    U_ASSERT((size & U_ARENA_SIZE_MASK) == size);
    arena->size = 0;
    arena->_total_size = size;
    arena->buf = U_Alloc(size);
}

void U_InitArenaStatic(U_Arena *arena, void *mem, unsigned long size)
{
	U_ASSERT(mem);
    U_ASSERT((size & U_ARENA_SIZE_MASK) == size);
    arena->size = 0;
    arena->_total_size = size | U_ARENA_STATIC_MEM_FLAG;
    arena->buf = mem;
}

void *U_AllocArena(U_Arena *arena, unsigned long size, unsigned alignment)
{
    unsigned char *p;
    unsigned char *end;
    unsigned long *size_hdr;
    unsigned long total;

    if (size == 0)
    	return 0;

    total = arena->_total_size & U_ARENA_SIZE_MASK;

	U_ASSERT(arena);
    U_ASSERT(arena->buf);
    U_ASSERT(total > 0);

    U_ASSERT((arena->size + 32 + size) < total); // enough space #1
    if (total < (arena->size + 32 + size))
        return 0;

    p = arena->buf;
    p += arena->size;
    p = U_memalign(p, alignment);

    // embed size header before memory
    size_hdr = (unsigned long*)p;
    *size_hdr = size;
    p += sizeof(*size_hdr);

    end = arena->buf;
    end += (arena->_total_size & U_ARENA_SIZE_MASK);

    if ((end - p) > size)
    {
        arena->size = (unsigned long)(p - (unsigned char*)arena->buf);
        arena->size += size;
        return p;
    }

    U_ASSERT(0 && "U_AllocArena() mem exhausted");

    return 0;
}

void U_FreeArena(U_Arena *arena)
{
	U_ASSERT(arena);
    U_ASSERT(arena->buf);
    if (arena->buf && (arena->_total_size & U_ARENA_STATIC_MEM_FLAG) == 0)
        U_Free(arena->buf);

    U_memset(arena, 0, sizeof(*arena));
}
