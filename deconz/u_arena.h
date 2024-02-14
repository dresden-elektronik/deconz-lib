/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef U_ARENA_H
#define U_ARENA_H

#ifndef U_LIBAPI
#ifdef _WIN32
#ifdef USE_ULIB_SHARED
    #define U_LIBAPI  __declspec(dllimport)
  #endif
  #ifdef BUILD_ULIB_SHARED
    #define U_LIBAPI  __declspec(dllexport)
  #endif
#endif
#endif /* ! defined(U_LIBAPI) */

#ifndef U_LIBAPI
#define U_LIBAPI
#endif

typedef struct U_Arena
{
    void *buf;
    unsigned long size;
    unsigned long _total_size;
} U_Arena;


#define U_ARENA_ALIGN_1   1
#define U_ARENA_ALIGN_8   8
#define U_ARENA_ALIGN_16  16

#ifdef __cplusplus
extern "C" {
#endif

U_LIBAPI void U_InitArena(U_Arena *arena, unsigned long size);
U_LIBAPI void U_InitArenaStatic(U_Arena *arena, void *mem, unsigned long size);
U_LIBAPI void *U_AllocArena(U_Arena *arena, unsigned long size, unsigned alignment);
U_LIBAPI void U_FreeArena(U_Arena *arena);

#ifdef __cplusplus
}
#endif

#endif /* U_ARENA_H */
