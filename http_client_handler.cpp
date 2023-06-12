/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include "deconz/http_client_handler.h"

const char *HttpStatusOk           = "200 OK"; // OK
const char *HttpStatusAccepted     = "202 Accepted"; // Accepted but not complete
const char *HttpStatusBadRequest   = "400 Bad Request"; // Malformed request
const char *HttpStatusForbidden    = "403 Forbidden"; // Understand request but no permission
const char *HttpStatusNotFound     = "404 Not Found"; // Requested uri not found
const char *HttpContentHtml        = "text/html; charset=utf-8";
const char *HttpContentCss         = "text/css";
const char *HttpContentJson        = "application/json; charset=utf-8";
const char *HttpContentJS          = "text/javascript";
const char *HttpContentPNG         = "image/png";
const char *HttpContentJPG         = "image/jpg";
const char *HttpContentSVG         = "image/svg+xml";

namespace deCONZ {

HttpClientHandler::HttpClientHandler()
{
}

HttpClientHandler::~HttpClientHandler()
{
}

} // namespace deCONZ
