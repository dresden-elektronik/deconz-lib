#ifndef DECONZ_APS_PRIVATE_H
#define DECONZ_APS_PRIVATE_H

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

#define APS_INVALID_NODE_ID 0xFFFFUL

namespace deCONZ {
DECONZ_DLLSPEC const char * ApsStatusToString(unsigned char status);
}

#endif // DECONZ_APS_PRIVATE_H
