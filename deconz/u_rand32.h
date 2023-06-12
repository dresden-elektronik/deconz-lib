#ifndef U_RAND32_H
#define U_RAND32_H

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include "deconz/declspec.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! Sets pseudo random number generator seed value. */
DECONZ_DLLSPEC void U_rand32_seed(unsigned seed);


/*! Returns 32-bit random number.
    This must NOT be used for security related random data. */
DECONZ_DLLSPEC unsigned U_rand32(void);

#ifdef __cplusplus
}
#endif

#endif /* U_RAND32_H */
