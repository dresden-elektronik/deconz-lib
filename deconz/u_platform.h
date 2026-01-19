/*
 * Copyright (c) 2026 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef U_PLATFORM_H
#define U_PLATFORM_H


#ifndef PL_MACOS
#ifdef __APPLE__
#define PL_MACOS
#endif
#endif

#ifndef PL_WINDOWS
#if defined(__WIN32__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(_MSC_VER)
#define PL_WINDOWS
#endif
#endif

#ifndef PL_LINUX
#ifdef __linux__
#define PL_LINUX
#endif
#endif

#ifndef PL_UNIX
#if defined(PL_MACOS) || defined(PL_LINUX)
#define PL_UNIX
#endif
#endif

#endif // U_PLATFORM_H
