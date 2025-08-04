/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <array>
#include <QString>
#include <cstring>
#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
#include <strings.h> // strncasecmp
#endif
#include "deconz/qhttprequest_compat.h"

static constexpr char nullByte = '\0';
static constexpr QLatin1String emptyString{&nullByte, int(0)};

enum
{
    MaxHeaderSize = 2048,
    MaxMethodLength = 10,
    MaxUrlLength = 160
};

/*! Helper class to determine components of an URL.
 */
class UrlDescriptor
{
    enum
    {
        MaxLength = 196,
        MaxComponents = 10,
    };

    struct Component
    {
        uint8_t offset = 0;
        uint8_t length = 0;
    };

    const char *m_buf = &nullByte;
    quint16 m_compCount = 0;
    uint8_t m_query = UINT8_MAX;
    uint8_t m_length = 0;
    std::array<Component, MaxComponents> m_comp{};

public:
    QLatin1String path() const;
    QLatin1String component(size_t i) const;
    Http::HttpStatus parseUrl(const char *buf, size_t length);
    size_t componentCount() const { return m_compCount; }
};

QLatin1String UrlDescriptor::path() const
{
    if (m_length == 0)
    {
        return emptyString;
    }
    else if (m_query < m_length)
    {
        return QLatin1String(m_buf, m_query);
    }
    else
    {
        return QLatin1String(m_buf, m_length);
    }
}

Http::HttpStatus UrlDescriptor::parseUrl(const char *buf, size_t length)
{
    m_length = 0;
    m_buf = &nullByte;
    m_compCount = 0;
    m_query = UINT8_MAX;
    m_comp = {};

    if (length > UINT8_MAX) // offset can only hold 8bit
        return Http::HttpStatusUriTooLong;

    m_buf = buf;
    m_length = (uint8_t)length;

    size_t compPos = 0;
    for (size_t i = 0; i < length; i++)
    {
        if (m_buf[i] == '\0')
            break;

        auto &comp = m_comp[compPos];

        if (m_buf[i] == '/')
        {
            if (comp.length > 0)
            {
//                m_buf[i] = '\0';
                compPos++;
                if (compPos == m_comp.size())
                {
                    return Http::HttpStatusRequestHeaderFieldsTooLarge; // not at the end, next component wouldn't fit
                }

                m_comp[compPos].offset = (uint8_t)i + 1;
                m_comp[compPos].length = 0;
            }
            else
            {
                comp.offset = (uint8_t)i + 1;
            }
        }
        else if (m_buf[i] == '?') // start of query string
        {
//            m_buf[i] = '\0';
            m_query = (uint8_t)i;
            break;
        }
        else
        {
            comp.length++;
        }
    }

    for (const auto &comp : m_comp)
    {
        if (comp.length != 0)
        {
            m_compCount++;
        }
    }

    return Http::HttpStatusOk;
}

class QHttpRequestHeaderPrivate
{

public:
    QHttpRequestHeaderPrivate() :
        isValid(0),
        httpMethod(HttpUnkown),
        _reserved(0)
        { }

    void init(const char *buf, size_t size);

    struct
    {
        uint8_t isValid : 1;
        uint8_t httpMethod : 3;
        uint8_t _reserved : 4;
    };

    UrlDescriptor urlDescriptor;
    QLatin1String cMethod{emptyString};
    QLatin1String cUrl{emptyString};
    QLatin1String lastKey{emptyString};
    QLatin1String lastKeyVal{emptyString};

    Http::HttpStatus parseStatus = Http::HttpStatusBadRequest;
    uint16_t rawSize = 0;
    uint64_t keyValuesPos = 0;
    std::array<char, MaxHeaderSize> raw{};
};

QHttpRequestHeader::QHttpRequestHeader() :
    d_ptr(new QHttpRequestHeaderPrivate)
{

}

/*! Copy constructor. */
QHttpRequestHeader::QHttpRequestHeader(const QHttpRequestHeader &other) :
    d_ptr(new QHttpRequestHeaderPrivate(*other.d_ptr))
{

}

/*! Copy assignment operator. */
QHttpRequestHeader& QHttpRequestHeader::operator=(const QHttpRequestHeader &other)
{
    // Self assignment?
    if (this == &other)
    {
        return *this;
    }

    *this->d_ptr = *other.d_ptr;
    return *this;
}

/*! Similar to \c strtok this finds the next token within range \p maxLength.
    As side effect the token gets '\0' terminated.
 */
QLatin1String findNextToken(char *pos, int maxLength)
{
    for (const char *beg = pos; (pos - beg) < maxLength; pos++)
    {
        if (*pos == ' ' || *pos == '\r' || *pos == '\n')
        {
            *pos = '\0';
            return QLatin1String(beg, pos - beg);
        }
    }

    return emptyString;
}

static QLatin1String trimmed(QLatin1String str)
{
    if (str.size() == 0 || !str.data())
        return str;

    const char *start = str.data();
    const char *end = str.data() + str.size();

    while (isspace(*start) && start < end)
    {
        start++;
    }

    while (end > start && isspace(end[-1]))
    {
        end--;
    }

    return QLatin1String(start, end - start);
}

static bool equalsCaseInsensitive(QLatin1String a, QLatin1String b)
{
    if (a.size() != b.size())
    {
        return false;
    }

    if (a.size() == 0)
    {
        return false;
    }

#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
    return strncasecmp(a.data(), b.data(), a.size()) == 0;
#else
    return a.startsWith(b, Qt::CaseInsensitive);
#endif
}

/*! \returns The value for a given \p key in the HTTP header key: value section.
    \returns An empty QLatin1String if not found.
 */
static QLatin1String getKeyValue(const char *pos, size_t size, const QLatin1String &key, QLatin1String &k1)
{
    QLatin1String result = emptyString;

    if (key.size() == 0)
    {
        return result;
    }

    const char *end = pos + size;

    while (pos < end)
    {
        const char *hdrEnd = std::strchr(pos, '\r');
        if (!hdrEnd)
            break;

        if (hdrEnd == pos)
            break;

        const char *mid = std::strchr(pos, ':');
        if (!mid)
        {
            pos = hdrEnd + 1;
            continue;
        }

        k1 = trimmed(QLatin1String(pos, mid - pos));
        if (equalsCaseInsensitive(k1, key))
        {
            mid++;

            result = trimmed(QLatin1String(mid, hdrEnd - mid));

            if (result.size() > 0)
            {
//                fprintf(stderr, "HTTP found '%.*s : %.*s' (%p, offset: %zu)\n", k1.size(), k1.data(), v1.size(), v1.data(), pos, size_t(pos - beg));
                return result;
            }
            result = emptyString;
            break;
        }

        pos = hdrEnd + 1;
    }

    k1 = emptyString;
    return result;
}

QHttpRequestHeader::QHttpRequestHeader(const char *buf, size_t size) :
    d_ptr(new QHttpRequestHeaderPrivate)
{
    d_ptr->init(buf, size);
}

QHttpRequestHeader::QHttpRequestHeader(const QString &method, const QString &path) :
    d_ptr(new QHttpRequestHeaderPrivate)
{
    const auto arr = QString("%1 %2 HTTP/1.1\r\n\r\n").arg(method, path).toUtf8();
    d_ptr->init(arr.constData(), arr.size());
}

QHttpRequestHeader::~QHttpRequestHeader()
{
    delete d_ptr;
    d_ptr = nullptr;
}

bool QHttpRequestHeader::hasKey(const QLatin1String &key) const
{
    if (d_ptr->isValid)
    {
        if (equalsCaseInsensitive(d_ptr->lastKey, key))
        {
            return d_ptr->lastKeyVal.size() != 0;
        }

        const char *pos = d_ptr->raw.data() + d_ptr->keyValuesPos;
        const size_t size = d_ptr->raw.size() - d_ptr->keyValuesPos;
        d_ptr->lastKeyVal = getKeyValue(pos, size, key, d_ptr->lastKey);
        return d_ptr->lastKeyVal.size() != 0;
    }
    return false;
}

size_t QHttpRequestHeader::contentLength() const
{
    const auto val = value(QLatin1String("Content-Length"));

    if (val.size() > 0)
    {
        bool ok;
        const uint i = QString(val).toUInt(&ok);
        if (ok)
            return i;
    }
    return 0;
}

QLatin1String QHttpRequestHeader::path() const
{
    return d_ptr->urlDescriptor.path();
}

QLatin1String QHttpRequestHeader::pathAt(size_t i) const
{
    return d_ptr->urlDescriptor.component(i);
}

size_t QHttpRequestHeader::pathComponentsCount() const
{
    return d_ptr->urlDescriptor.componentCount();
}

QLatin1String QHttpRequestHeader::method() const
{
    return d_ptr->cMethod;
}

HttpMethod QHttpRequestHeader::httpMethod() const
{
    return HttpMethod(d_ptr->httpMethod);
}

QLatin1String QHttpRequestHeader::value(const QLatin1String &key) const
{
    if (d_ptr->isValid)
    {
        if (equalsCaseInsensitive(d_ptr->lastKey, key))
        {
            return d_ptr->lastKeyVal;
        }

        const char *pos = d_ptr->raw.data() + d_ptr->keyValuesPos;
        const size_t size = d_ptr->raw.size() - d_ptr->keyValuesPos;
        d_ptr->lastKeyVal = getKeyValue(pos, size, key, d_ptr->lastKey);
        return d_ptr->lastKeyVal;
    }

    return emptyString;
}

QLatin1String QHttpRequestHeader::url() const
{
    return d_ptr->cUrl;
}

bool QHttpRequestHeader::update(const char *buf, size_t size)
{
    d_ptr->init(buf, size);
    return d_ptr->isValid == 1;
}

Http::HttpStatus QHttpRequestHeader::parseStatus() const
{
    return d_ptr->parseStatus;
}

QLatin1String UrlDescriptor::component(size_t i) const
{
    if (i < m_compCount)
    {
        Q_ASSERT(i < m_comp.size());
        const auto c = m_comp[i];
        Q_ASSERT(c.length > 0);

        return QLatin1String(&m_buf[c.offset], c.length);
    }

    return emptyString;
}

void checkMissingSlashAfterApi(char *buf)
{
    if (memcmp(buf, "/api", 4) != 0)
        return;

    if (buf[4] == '/') // valid
        return;

    const char *end = std::strchr(buf, '\r');
    if (!end || end > (buf + MaxUrlLength) || end[1] != '\n')
        return;

    char *insPos = &buf[4];
    memmove(insPos + 1, insPos, end - insPos); // overwrite '\r' but that's ok
    *insPos = '/'; // insert missing slash
}

void QHttpRequestHeaderPrivate::init(const char *buf, size_t size)
{
    isValid = 0;
    httpMethod = HttpUnkown;
    parseStatus = Http::HttpStatusBadRequest;

    if (size == 0)
    {
        return;
    }

    if (size >= raw.size() || size > UINT16_MAX)
    {
        parseStatus = Http::HttpStatusRequestHeaderFieldsTooLarge;
        return;
    }

    rawSize = (uint16_t)size;

    memcpy(raw.data(), buf, size);
    raw[size] = '\0';

    char *pos = raw.data();
    char *end = raw.data() + raw.size();

    cMethod = findNextToken(pos, MaxMethodLength);
    if (cMethod.size() == 0)
    {
        return;
    }

    if      (cMethod == QLatin1String("GET")) { httpMethod = HttpGet; }
    else if (cMethod == QLatin1String("PUT")) { httpMethod = HttpPut; }
    else if (cMethod == QLatin1String("POST")) {

     httpMethod = HttpPost;
    }
    else if (cMethod == QLatin1String("DELETE")) { httpMethod = HttpDelete; }
    else if (cMethod == QLatin1String("PATCH")) { httpMethod = HttpPatch; }
    else if (cMethod == QLatin1String("OPTIONS")) { httpMethod = HttpOptions; }
    else if (cMethod == QLatin1String("HEAD")) { httpMethod = HttpHead; }
    else
    {
        parseStatus = Http::HttpStatusMethodNotAllowed;
        return;
    }

    pos += cMethod.size() + 1;

    checkMissingSlashAfterApi(pos);

    cUrl = findNextToken(pos, MaxUrlLength);

    if (cUrl.size() == 0)
    {
        if ((end - pos) > MaxUrlLength)
        {
            parseStatus = Http::HttpStatusUriTooLong;
        }
        return;
    }

    parseStatus = urlDescriptor.parseUrl(pos, cUrl.size());
    if (parseStatus != Http::HttpStatusOk)
    {
        return;
    }
    parseStatus = Http::HttpStatusBadRequest;

    pos += cUrl.size() + 1;

    const QLatin1String version2 = findNextToken(pos, 16);

    if (version2.size() == 0)
    {
        return;
    }

    pos += version2.size() + 1;


    while (isspace(*pos) && pos < end)
    {
        pos++;
    }

    keyValuesPos = pos - raw.data();
    parseStatus = Http::HttpStatusOk;

    isValid = 1;
}
