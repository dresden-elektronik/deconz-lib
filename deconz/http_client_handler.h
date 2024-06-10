#ifndef DECONZ_HTTP_CLIENT_HANDLER_H
#define DECONZ_HTTP_CLIENT_HANDLER_H

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <deconz/types.h>
#include "deconz/declspec.h"

class QTcpSocket;
class QHttpRequestHeader;

namespace deCONZ
{

DECONZ_DLLSPEC int HttpSend(unsigned int handle, const void *buf, unsigned len);

class DECONZ_DLLSPEC HttpClientHandler
{
public:
    HttpClientHandler();
    virtual ~HttpClientHandler();
    virtual bool isHttpTarget(const QHttpRequestHeader &hdr) = 0;
    /*!
     * \brief handleHttpRequest
     * \param hdr
     * \param sock
     * \return 0 on success
     */
    virtual int handleHttpRequest(const QHttpRequestHeader &hdr, QTcpSocket *sock) = 0;
    virtual void clientGone(QTcpSocket *sock) = 0;
};

int registerHttpClientHandler(deCONZ::HttpClientHandler *handler);

} // namespace deCONZ

#endif // DECONZ_HTTP_CLIENT_HANDLER_H
