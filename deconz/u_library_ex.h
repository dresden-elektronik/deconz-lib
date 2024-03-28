/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef U_LIBRARY_EX_H
#define U_LIBRARY_EX_H

#include "u_library.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! Improved version of U_library_open() that looks up platform specific
    paths and library names, so the API client doesn't have to. */
U_LIBAPI void *U_library_open_ex(const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* U_LIBRARY_EX_H */
