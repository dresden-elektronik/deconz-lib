#ifndef DECONZ_UTIL_H
#define DECONZ_UTIL_H

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <QString>
#include <QMetaType> // QVariantList, QVariantMap
#include "deconz/declspec.h"

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
    DdfUserLocation
};

int DECONZ_DLLSPEC appArgumentNumeric(const QString &arg, int defaultValue);
QString DECONZ_DLLSPEC appArgumentString(const QString &arg, const QString &defaultValue);
QString DECONZ_DLLSPEC jsonStringFromMap(const QVariantMap &map);
QString DECONZ_DLLSPEC jsonStringFromList(const QVariantList &ls);
QString DECONZ_DLLSPEC getStorageLocation(StorageLocation location);
bool DECONZ_DLLSPEC isVirtualMachine();
} // namespace deCONZ

#define U_INVALID_UNICODE_CODEPOINT 0x20000000

extern "C" {

/*! Convert utf-8 to unicode code point.

    Returns pointer to remainder of text. 'codepoint' is a valid codepoint
    or set to U_INVALID_UNICODE_CODEPOINT for invalid utf8.
 */
DECONZ_DLLSPEC const char * U_utf8_codepoint(const char *text, unsigned *codepoint);

} // extern "C"


#endif // DECONZ_UTIL_H
