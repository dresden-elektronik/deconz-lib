#ifndef DECONZ_DECLSPEC_H
#define DECONZ_DECLSPEC_H

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef DECONZ_DLLSPEC

#ifdef _WIN32

#ifdef BUILD_ULIB_SHARED
    #define DECONZ_DLLSPEC  __declspec(dllexport)
#else
    #define DECONZ_DLLSPEC  __declspec(dllimport)
#endif

#else // Unix

    #define DECONZ_DLLSPEC

#endif

#endif /* ! defined DECONZ_DLLSPEC */


#endif // DECONZ_DECLSPEC_H
