/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include "deconz/u_rand32.h"

static unsigned _u_rand;

void U_rand32_seed(unsigned seed)
{
    _u_rand = seed;
}

/*  LCG generator (BSD)
    https://tfetimes.com/c-linear-congruential-generator
*/
unsigned U_rand32(void)
{
    const unsigned a = 1103515245;
    const unsigned c = 12345;
    const unsigned m = 2147483648;
    _u_rand = (a * _u_rand + c) % m;
    return _u_rand;
}
