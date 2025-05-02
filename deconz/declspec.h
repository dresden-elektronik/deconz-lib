#ifndef DECONZ_DECLSPEC_H
#define DECONZ_DECLSPEC_H

/*
 * Copyright (c) 2012-2025 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef DECONZ_DLLSPEC

#ifdef _WIN32

    #define DECONZ_EXPORT  __declspec(dllexport)

#ifdef BUILD_ULIB_SHARED
    #define DECONZ_DLLSPEC  __declspec(dllexport)
#else
    #define DECONZ_DLLSPEC  __declspec(dllimport)
#endif

#else // Unix

#ifdef __GNUC__
#define DECONZ_EXPORT __attribute__ ((visibility("default")))
#define DECONZ_DLLSPEC __attribute__ ((visibility("default")))
#else
#define DECONZ_EXPORT
#define DECONZ_DLLSPEC
#endif

#endif

#endif /* ! defined DECONZ_DLLSPEC */
/*
 *    DECONZ_API_DEPRECATED void foo();
 */

#ifdef __GNUC__
#define DECONZ_API_DEPRECATED __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define DECONZ_API_DEPRECATED __declspec(deprecated)
#else
#define DECONZ_API_DEPRECATED
#endif


#endif // DECONZ_DECLSPEC_H
