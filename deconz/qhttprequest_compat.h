#ifndef QHTTPREQUEST_COMPAT_H
#define QHTTPREQUEST_COMPAT_H

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <QLatin1String>
#include "deconz/declspec.h"

class QString;
class QHttpRequestHeaderPrivate;

namespace Http {

    enum HttpStatus
    {
        HttpStatusOk = 200,
        HttpStatusBadRequest = 400,
        HttpStatusMethodNotAllowed = 405,
        HttpStatusPayloadTooLarge = 413,
        HttpStatusUriTooLong = 414,
        HttpStatusRequestHeaderFieldsTooLarge = 431
    };
}

enum HttpMethod
{
    HttpUnkown  = 0,
    HttpGet     = 1,
    HttpPut     = 2,
    HttpPost    = 3,
    HttpPatch   = 4,
    HttpDelete  = 5,
    HttpOptions = 6,
    HttpHead = 6
};

class DECONZ_DLLSPEC QHttpRequestHeader
{
public:
    QHttpRequestHeader();
    QHttpRequestHeader(const QHttpRequestHeader &other);
    /*! Main Constructor to build the object.
        The \p buf is copied to the internal memory and processed,
        it must contain all relevant HTTP header data including key value pairs.
     */
    QHttpRequestHeader(const char *buf, size_t size);
    QHttpRequestHeader(const QString &method, const QString &path);
    QHttpRequestHeader& operator=(const QHttpRequestHeader &other);
    ~QHttpRequestHeader();
    bool hasKey(const QLatin1String &key) const;
    size_t contentLength() const;
    QLatin1String path() const;
    QLatin1String pathAt(size_t i) const;
    size_t pathComponentsCount() const;
    QLatin1String method() const;
    HttpMethod httpMethod() const;
    QLatin1String value(const QLatin1String &key) const;
    QLatin1String url() const;

    /*! Reuses the internal memory without reallocation.
        Same as creating a new QHttpRequestHeader and assign it to the old one.
     */
    bool update(const char *buf, size_t size);

    Http::HttpStatus parseStatus() const;

private:
    QHttpRequestHeaderPrivate *d_ptr = nullptr;
};

#endif // QHTTPREQUEST_COMPAT_H

