/*!
  \file util.h
  Utility functions and classes for various purposes.
 */

#ifndef DECONZ_UTIL_H
#define DECONZ_UTIL_H

/*
 * Copyright (c) 2012-2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

/*!
    \defgroup utils Utils
    \section utils_intro Utility functions and classes.
    \brief Utility functions and classes for various purposes.

    The module is work in progress and meant to replace all platform specific and
    Qt based functionality.
*/

#include <QString>
#include "deconz/declspec.h"

/*! \brief Various data types as defined in ZigBee ZCL specification.
    \ingroup utils

    To handle data type in payloads use deCONZ::ZclDataTypeId_t
*/
namespace deCONZ {


enum StorageLocation
{
    HomeLocation,
    ApplicationsLocation,
    ApplicationsDataLocation,
    DocumentsLocation,
    ZcldbLocation,
    ConfigLocation,
    NodeCacheLocation,
    RuntimeLocation,
    DdfLocation,
    DdfUserLocation,
    DdfBundleLocation,
    DdfBundleUserLocation
};

int DECONZ_DLLSPEC appArgumentNumeric(const QString &arg, int defaultValue);
QString DECONZ_DLLSPEC appArgumentString(const QString &arg, const QString &defaultValue);
QString DECONZ_DLLSPEC getStorageLocation(StorageLocation location);
bool DECONZ_DLLSPEC isVirtualMachine();
} // namespace deCONZ

#define U_INVALID_UNICODE_CODEPOINT 0x20000000

extern "C" {

DECONZ_DLLSPEC const char * U_utf8_codepoint(const char *text, unsigned *codepoint);

} // extern "C"

#endif // DECONZ_UTIL_H
