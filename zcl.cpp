/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <QCoreApplication>
#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStack>
#include <QTextStream>
#include <QXmlStreamReader>
#include <array>
#include <tuple>
#ifdef Q_OS_UNIX
#include <unistd.h>
#endif
#include "deconz/aps.h"
#include "deconz/zcl.h"
#include "deconz/dbg_trace.h"
#include "deconz/util.h"
#include "deconz/mem_pool.h"
#include "zcl_private.h"

namespace {
    enum ParseSection {
        InNone,
        InDomain,
        InDevice,
        InProfile,
        InCluster,
        InClusterServer,
        InClusterClient,
        InCommand,
        InCommandPayload,
        InAttribute,
        InAttributeSet,
        InEnumeration
    };
}

static deCONZ::ZclDataBase *_zclDB = nullptr;

namespace deCONZ {

template <typename T, typename U>
static T boundChecked(T oldValue, U value)
{
    static_assert (std::is_integral<T>::value, "T must be integral");
    static_assert (std::is_integral<U>::value, "U must be integral");
    static_assert (std::is_signed<T>::value == std::is_signed<U>::value, "T+U differs in signed types");

    if (value > std::numeric_limits<T>::max())
    {
        DBG_Assert(value <= std::numeric_limits<T>::max());
        return oldValue;
    }
    else if (std::is_signed<T>::value && value < std::numeric_limits<T>::min())
    {
        DBG_Assert(value >= std::numeric_limits<T>::min());
        return oldValue;
    }

    return static_cast<T>(value);
}

QDataStream & operator<<(QDataStream &ds, ManufacturerCode_t mfcode)
{
    ds << static_cast<quint16>(mfcode);
    return ds;
}

QDataStream & operator>>(QDataStream &ds, ManufacturerCode_t &mfcode)
{
    quint16 x;
    ds >> x;
    if (ds.status() != QDataStream::ReadPastEnd)
    {
        mfcode = ManufacturerCode_t(x);
    }
    return ds;
}

QDataStream &operator<<(QDataStream &ds, ZclAttributeId_t id)
{
    ds << static_cast<quint16>(id);
    return ds;
}

QDataStream &operator>>(QDataStream &ds, ZclAttributeId_t &id)
{
    quint16 x;
    ds >> x;
    if (ds.status() != QDataStream::ReadPastEnd)
    {
        id = ZclAttributeId_t(x);
    }
    return ds;
}

QDataStream & operator<<(QDataStream &ds, ZclClusterId_t id)
{
    ds << static_cast<quint16>(id);
    return ds;
}

QDataStream & operator>>(QDataStream &ds, ZclClusterId_t &id)
{
    quint16 x;
    ds >> x;
    if (ds.status() != QDataStream::ReadPastEnd)
    {
        id = ZclClusterId_t(x);
    }
    return ds;
}

QDataStream & operator<<(QDataStream &ds, ZclCommandId_t id)
{
    ds << static_cast<quint8>(id);
    return ds;
}

QDataStream & operator>>(QDataStream &ds, ZclCommandId_t &id)
{
    quint8 x;
    ds >> x;
    if (ds.status() != QDataStream::ReadPastEnd)
    {
        id = ZclCommandId_t(x);
    }
    return ds;
}

static ZclMemory *zclMem = nullptr;
static ZclMemoryPrivate *zclMemPriv = nullptr;

class ZclMemoryPrivate
{
public:
    ~ZclMemoryPrivate();

    std::tuple<
        std::array<ZclAttributePrivate*, ZclAttributePrivate::PoolSize>,
        std::array<ZclFramePrivate*, ZclFramePrivate::PoolSize>
    > mem{};
};

ZclMemoryPrivate::~ZclMemoryPrivate()
{
    for (auto &i : MEM_GetAllocContainer<ZclAttributePrivate>(zclMemPriv->mem))
    {
        if (i) { delete i; }
    }

    for (auto &i : MEM_GetAllocContainer<ZclFramePrivate>(zclMemPriv->mem))
    {
        if (i) { delete i; }
    }
}

ZclMemory::ZclMemory() :
    d(new ZclMemoryPrivate)
{
    Q_ASSERT_X(zclMem == nullptr, "ZclMemory::ZclMemory()", "Already initialized");
    zclMem = this; // singleton
    zclMemPriv = d; // quick ref
}

ZclMemory::~ZclMemory()
{
    Q_ASSERT(zclMem);
    delete d;
    zclMem = nullptr;
    zclMemPriv = nullptr;
}

ZclAttributePrivate::ZclAttributePrivate() :
    m_id(0xFFFF),
    m_dataType(0xFF),
    m_subType(0xFF),
    m_access(ZclRead),
    m_enumerationId(0xFF),
    m_numericBase(10),
    m_required(false),
    m_avail(true),
    m_lastRead((time_t)-1),
    m_listSizeAttr(0xFFFF),
    m_listSize(0),
    m_minReportInterval(0),
    m_maxReportInterval(0xFFFF),
    m_reportTimeout(0),
    m_formatHint(ZclAttribute::DefaultFormat),
    m_manufacturerCode(0)
{
    m_valueState.idx = 0;
    m_valueState.bitmap = 0;
    m_numericValue.u64 = 0;
}

ZclAttribute::ZclAttribute() :
    // delegate constructor
    ZclAttribute(0xFFFF, ZclNoData, QLatin1String(""), ZclRead, false)
{
}

ZclAttribute::ZclAttribute(uint16_t id, uint8_t type, const QString &name, ZclAccess access, bool required)
{
    d_ptr = MEM_AllocItem<ZclAttributePrivate>(&zclMemPriv->mem);

    Q_D(ZclAttribute);

    d->m_id = id;
    d->m_dataType = type;
    d->m_subType = 0xFF;
    d->m_name = name;
    d->m_access = access;
    d->m_enumerationId = 0xFF;
    d->m_numericBase = 10;
    d->m_required = required;
    d->m_avail = true;
    d->m_lastRead = (time_t)-1;
    d->m_listSizeAttr = 0xFFFF;
    d->m_listSize = 0;
    d->m_attrSetId = 0xFFFF;
    d->m_attrSetManufacturerCode = 0;
    d->m_manufacturerCode = 0;
    d->m_minReportInterval = 0;
    d->m_maxReportInterval = 0xFFFF;
    d->m_reportTimeout = 0;
    d->m_reportableChange.u64 = 0;
    d->m_formatHint = DefaultFormat;
    d->m_rangeMin = 0;
    d->m_rangeMax = 0;
    d->m_valueState.idx = 0;
    d->m_valueState.bitmap = 0;
    d->m_numericValue.u64 = 0;
    d->m_description.clear();
    d->m_valueNames.clear();
    d->m_valuePos.clear();
    d->m_value = QVariant();

    switch (type)
    {
    case ZclIeeeAddress:
    case Zcl8BitBitMap:
    case Zcl16BitBitMap:
    case Zcl24BitBitMap:
    case Zcl32BitBitMap:
    case Zcl40BitBitMap:
    case Zcl48BitBitMap:
    case Zcl56BitBitMap:
    case Zcl64BitBitMap:
        d->m_numericBase = 16;
        break;

    default:
        break;
    }
}

ZclAttribute::ZclAttribute(ZclAttributeId_t id, ZclDataTypeId_t type, const QString &name, ZclAccess access, bool required) :
    // delegate constructor
    ZclAttribute(static_cast<quint16>(id), static_cast<quint8>(type), name, access, required)

{
    Q_ASSERT(d_ptr);
}

ZclAttribute::ZclAttribute(const ZclAttribute &other)
{
    Q_ASSERT(this != &other);
    d_ptr = MEM_AllocItem<ZclAttributePrivate>(&zclMemPriv->mem);
    *d_ptr = *other.d_ptr;
}

ZclAttribute::ZclAttribute(ZclAttribute &&other) noexcept :
    d_ptr(other.d_ptr)
{
    Q_ASSERT(d_ptr);
    Q_ASSERT(other.d_ptr);
    other.d_ptr = nullptr;
}

ZclAttribute &ZclAttribute::operator=(const ZclAttribute &other)
{
    // Self assignment?
    if (this == &other)
    {
        return *this;
    }

    Q_ASSERT(other.d_ptr);
    Q_ASSERT(d_ptr);
    *d_ptr = *other.d_ptr;
    return *this;
}

ZclAttribute &ZclAttribute::operator=(ZclAttribute &&other) noexcept
{
    // Self assignment?
    if (this == &other)
    {
        return *this;
    }

    Q_ASSERT(other.d_ptr);

    if (d_ptr)
    {
        MEM_DeallocItem<ZclAttributePrivate>(d_ptr, &zclMemPriv->mem);
    }

    d_ptr = other.d_ptr;
    other.d_ptr = nullptr;
    Q_ASSERT(d_ptr);

    return *this;
}

ZclAttribute::~ZclAttribute()
{
    if (d_ptr)
    {
        MEM_DeallocItem<ZclAttributePrivate>(d_ptr, &zclMemPriv->mem);
        d_ptr = nullptr;
    }
}

uint16_t ZclAttribute::id() const
{
    Q_D(const ZclAttribute);
    return d->m_id;
}

ZclAttributeId_t ZclAttribute::id_t() const
{
    Q_D(const ZclAttribute);
    return ZclAttributeId_t(d->m_id);
}

const QString &ZclAttribute::description() const
{
    Q_D(const ZclAttribute);
    return d->m_description;
}

void ZclAttribute::setDescription(const QString &description)
{
    Q_D(ZclAttribute);
    d->m_description = description;
}

uint8_t ZclAttribute::dataType() const
{
    Q_D(const ZclAttribute);
    return d->m_dataType;
}

ZclDataTypeId_t ZclAttribute::dataType_t() const
{
    Q_D(const ZclAttribute);
    return ZclDataTypeId_t(d->m_dataType);
}

void ZclAttribute::setDataType(uint8_t type)
{
    Q_D(ZclAttribute);
    d->m_dataType = type;

    switch (type)
    {
    case ZclIeeeAddress:
    case Zcl8BitBitMap:
    case Zcl16BitBitMap:
    case Zcl24BitBitMap:
    case Zcl32BitBitMap:
    case Zcl40BitBitMap:
    case Zcl48BitBitMap:
    case Zcl56BitBitMap:
    case Zcl64BitBitMap:
        d->m_numericBase = 16;
        break;

    default:
        break;
    }
}

const QString &ZclAttribute::name() const
{
    Q_D(const ZclAttribute);
    return d->m_name;
}

uint8_t ZclAttribute::subType() const
{
    Q_D(const ZclAttribute);
    return d->m_subType;
}

void ZclAttribute::setSubType(uint8_t subType)
{
    Q_D(ZclAttribute);
    d->m_subType = subType;
}

const NumericUnion &ZclAttribute::numericValue() const
{
    Q_D(const ZclAttribute);
    return d->m_numericValue;
}

void ZclAttribute::setNumericValue(const NumericUnion &value)
{
    Q_D(ZclAttribute);
    d->m_numericValue = value;
}

QString ZclAttribute::valueNameAt(int bitOrEnum) const
{
    Q_D(const ZclAttribute);
    for (size_t i = 0; i < d->m_valuePos.size(); i++)
    {
        if (d->m_valuePos[i] == bitOrEnum)
        {
            if (d->m_valueNames.size() > static_cast<int>(i))
            {
                return d->m_valueNames.at(static_cast<int>(i));
            }
        }
    }

    return {};
}

QStringList ZclAttribute::valuesNames() const
{
    Q_D(const ZclAttribute);
    return d->m_valueNames;
}

const std::vector<int> &ZclAttribute::valueNamePositions() const
{
    Q_D(const ZclAttribute);
    return d->m_valuePos;
}

/*!
    Writes the attribute in the specifications format to the stream.

    \return true if the attribute was written
            false if the type is not supported or the attribute is not valid.
 */
bool ZclAttribute::writeToStream(QDataStream &stream) const
{
    bool ok = false;
    Q_D(const ZclAttribute);

    switch (dataType())
    {
    case Zcl8BitBitMap:
    case Zcl16BitBitMap:
    case Zcl24BitBitMap:
    case Zcl32BitBitMap:
    case Zcl40BitBitMap:
    case Zcl48BitBitMap:
    case Zcl56BitBitMap:
    case Zcl64BitBitMap:
    {
        quint64 tmp = bitmap();
        const ZclDataType &type = zclDataBase()->dataType(dataType());

        if ((type.length() <= 0) || (type.length() > 64))
        {
            return false;
        }

        for (int i = 0; i < type.length(); i++)
        {
            stream << (quint8)(tmp & 0xff);
            tmp >>= 8;
        }

    }
        return true;

    case Zcl24BitUint:
    case Zcl40BitUint:
    case Zcl48BitUint:
    case Zcl56BitUint:
    {
        quint64 tmp = d->m_numericValue.u64;
        const ZclDataType &type = zclDataBase()->dataType(dataType());

        if ((type.length() <= 0) || (type.length() > 64))
        {
            return false;
        }

        for (int i = 0; i < type.length(); i++)
        {
            stream << static_cast<quint8>(tmp & 0xff);
            tmp >>= 8;
        }
    }
        return true;

    case ZclBoolean:
    case Zcl8BitData:
    case Zcl8BitUint: { stream << d->m_numericValue.u8; ok = true; } break;
    case ZclAttributeId:
    case ZclClusterId:
    case Zcl16BitData:
    case Zcl16BitUint: { stream << d->m_numericValue.u16; ok = true; } break;
    case ZclBACNetOId:
    case ZclUtcTime:
    case Zcl32BitData:
    case Zcl32BitUint: { stream << d->m_numericValue.u32; ok = true; } break;
    case ZclIeeeAddress:
    case Zcl64BitData:
    case Zcl64BitUint: { stream << (quint64)d->m_numericValue.u64; ok = true; } break;

    case Zcl8BitEnum:
        if (d->m_numericValue.u32 <= 0xFFul)
        {
            stream << static_cast<quint8>(d->m_numericValue.u32);
            return true;
        }
        return false;

    case Zcl16BitEnum:
        if (d->m_numericValue.u32 <= 0xFFFFul)
        {
            stream << static_cast<quint16>(d->m_numericValue.u32);
            return true;
        }
        return false;

    case Zcl8BitInt:  { stream << d->m_numericValue.s8; ok = true; } break;
    case Zcl16BitInt: { stream << d->m_numericValue.s16; ok = true; } break;
    case Zcl32BitInt: { stream << d->m_numericValue.s32; ok = true; } break;
    case Zcl64BitInt: { stream << (qint64)d->m_numericValue.s64; ok = true; } break;

    case Zcl24BitInt:
    case Zcl40BitInt:
    case Zcl48BitInt:
    case Zcl56BitInt:
    {
        quint64 tmp = d->m_numericValue.s64 >= 0 ? d->m_numericValue.s64 : -d->m_numericValue.s64;
        const ZclDataType &type = zclDataBase()->dataType(dataType());

        if ((type.length() <= 0) || (type.length() > 64))
        {
            return false;
        }

        for (int i = 0; i < type.length(); i++)
        {
            quint8 b = static_cast<quint8>(tmp & 0xff);
            if (i == (type.length() - 1) && d->m_numericValue.s64 < 0)
            {
                b |= 0x80; // signed
            }
            stream << (b & 0xFF);
            tmp >>= 8;
        }
    }
        return true;

    case ZclSingleFloat:
    {
        stream << d->m_numericValue.u32;
        ok = true;
    }
        break;

    case ZclOctedString:
    {
        if (d->m_value.isValid() && d->m_value.userType() == QVariant::ByteArray)
        {
            const QByteArray data = d->m_value.toByteArray();

            if (data.size() > UINT8_MAX)
            {
                return false;
            }

            stream << static_cast<uint8_t>(data.size());
            if (data.size() > 0)
            {
                stream.writeRawData(data.constData(), data.size());
            }
            ok = true;
        }
        else
        {
            stream << static_cast<uint8_t>(0); // length = 0: empty
            ok = true;
        }
    }
        break;

    case ZclCharacterString:
    {
        const QString text = d->m_value.toString();

        if (text.size() > UINT8_MAX)
        {
            return false;
        }

        int len = text.size();
        stream << static_cast<uint8_t>(len);

        for (int i = 0; i < len; i++)
        {
            stream << static_cast<uint8_t>(text[i].toLatin1());
        }

        return true;
    }
        break;

    case Zcl128BitSecurityKey:
    {
        QByteArray key = d->m_value.toByteArray();
        if (key.size() == 16)
        {
            for (int i  = 0; i < 16; i++)
            {
                stream << (quint8)key[i];
            }
            ok = true;
        }
    }
    break;

    default:
        ok = false;
        break;
    }

    return ok;
}


/* Latin1 0x00-0xff is valid encoding
 * Check for printable characters, since these are expected in ZCL string attributes
 */
static int isLikelyLatin1String(const char *str, unsigned len)
{
    unsigned i;
    unsigned ok_chars = 0;

    for (i = 0; i < len; i++)
    {
        unsigned char ch = (unsigned char)str[i];

        // reject NUL, SOH, STX, EOT, ENQ, ACK, BEL, BS
        if      (ch == '\t')               { ok_chars++; }
        else if (ch == '\n')               { ok_chars++; }
        else if (ch == '\r')               { ok_chars++; }
        else if (ch == 0x7f)               { break; } // delete DEL
        else if (ch < 0x20)                { break; } // ASCII control chars
        else if (ch >= 0x80 && ch <= 0x9f) { break; } // Latin1 control chars
        else                               { ok_chars++; }
    }

    return ok_chars == len ? 1 : 0;
}

static int latin1ToUtf8Opinionated(const char *lat, unsigned len, unsigned char *out, unsigned outlen)
{
    unsigned i;
    unsigned w; // out write position

    w = 0;
    out[0] = '\0';

    for (i = 0; i < len; i++)
    {
        unsigned char ch = lat[i];

        if      (ch == 0xa0) ch = ' '; // non breakable space to ASCII space
        else if (ch == 0xad) ch = '-'; // soft hyphen to ASCII minus

        if (ch < 128 && (outlen - w) > 1)
        {
            out[w++] = ch;
            out[w] = '\0';
        }
        else if (ch > 128 && (outlen - w) > 2)
        {
            out[w++] = 0xc0 | ch >> 6;
            out[w++] = (0x80 | (ch & 0x3f));
            out[w] = '\0';
        }
        else
        {
            return 0; // unlikely
        }
    }

    return 1;
}

bool ZclAttribute::readFromStream(QDataStream &stream)
{
    if (stream.atEnd())
    {
        return false;
    }

    Q_D(ZclAttribute);
    quint8 u8;
    d->m_numericValue.u64 = 0;
    ZclDataType type = zclDataBase()->dataType(dataType());

    if (!zclDataBase()->knownDataType(dataType()))
    {
        DBG_Printf(DBG_ZCLDB, "ZCL Read Attributes Datatype 0x%02X %s"
               " not supported yet, abort\n",
               type.id(), qPrintable(type.name()));
        return false;
    }

    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    switch (dataType())
    {
    case ZclOctedString:
    {
        quint8 len;
        stream >> len;

        if ((len > 0) && stream.atEnd())
            return false;

        QByteArray data;

        if (len > 0)
        {
            data.resize(len);
            Q_ASSERT(data.capacity() >= len);

            int nbytes = stream.readRawData(data.data(), len);
            if (nbytes != int(len))
                return false;
        }

        d->m_value = data;
        d->m_numericValue.u64 = unsigned(len);

        return true;
    }
        break;

    case ZclCharacterString:
    {
        uint8_t len;
        stream >> len;

        if (stream.status() == QDataStream::ReadPastEnd)
            return false;

        if (len == 0)
        {
            d->m_value = QString();
            return true;
        }

        int ret;
        char buf[120 + 8] = {0}; // 7 bytes safe zone for utf8 parsing + '\0'

        if (unsigned(len) > sizeof(buf) - 8)
            return false;

        ret = stream.readRawData(buf, len);

        if (ret != len || stream.status() == QDataStream::ReadPastEnd)
        {
            return false;
        }

        buf[len] = '\0';

        // strip trailing zero terminators
        for (; len > 0;)
        {
            if (buf[len - 1] != '\0')
                break;

            len--;
        }

        if (len == 0)
        {
            d->m_value = QString();
            return true;
        }

        unsigned codepoint = U_INVALID_UNICODE_CODEPOINT;
        const char *p = &buf[0];
        const char *pnonprint = nullptr;

        while (p < &buf[len])
        {
            p = U_utf8_codepoint(p, &codepoint);

            if (codepoint == U_INVALID_UNICODE_CODEPOINT)
                break;

            if (!pnonprint && codepoint == 0)
                pnonprint = p - 1;
        }

        // in rare cases latin1 encoding has been seen, check this
        if (codepoint == U_INVALID_UNICODE_CODEPOINT && isLikelyLatin1String(buf, len))
        {
            unsigned char utf8buf[256];

            if (latin1ToUtf8Opinionated(buf, len, utf8buf, sizeof(utf8buf)))
            {
                d->m_value = QString::fromUtf8((const char*)&utf8buf[0]);
                return true;
            }
        }

        if (codepoint == U_INVALID_UNICODE_CODEPOINT)
        {
            // contains non utf8 characters
            d->m_value = QByteArray(&buf[0], len);
            d->m_formatHint = deCONZ::ZclAttribute::Prefix;
            d->m_numericValue.u64 = unsigned(len);
        }
        else if (pnonprint && pnonprint < &buf[len - 1])
        {
            d->m_value = QByteArray(&buf[0], len);
            d->m_numericValue.u64 = unsigned(len);
        }
        else if (p == &buf[len]) // note we also reach here if last codepoint is '\0'
        {
            for (;len > 0 && buf[len - 1] == '\0';)
                len--;

            d->m_value = QString::fromUtf8((const char*)&buf[0], len);
        }
    }
        break;

    case Zcl8BitData:
    case Zcl8BitUint:
    case Zcl8BitEnum:
    {
        stream >> d->m_numericValue.u8;
        d->m_value = quint64(d->m_numericValue.u8);
    }
        break;

    case Zcl16BitData:
    case Zcl16BitUint:
    case Zcl16BitEnum:
    case ZclAttributeId:
    case ZclClusterId:
        if (isList() && (listSize() > 0))
        {
            quint16 val;

            QVariantList ls;
            int i = listSize() - 1;

            while (i && !stream.atEnd())
            {
                stream >> val;
                ls.append(val);
                i--;
            }

            d->m_numericValue.u16 = (quint16)ls.first().toUInt();
            d->m_value = ls;
        }
        else
        {
            stream >> d->m_numericValue.u16;
            d->m_value = d->m_numericValue.u16;
        }
        break;

    case Zcl32BitData:
    case Zcl32BitUint:
    {
        stream >> d->m_numericValue.u32;
        d->m_value = d->m_numericValue.u32;
    }
        break;

    case Zcl8BitInt:
    {
        stream >> d->m_numericValue.s8;
        d->m_value = qint32(d->m_numericValue.s8);
    }
        break;

    case Zcl16BitInt:
    {
        stream >> d->m_numericValue.s16;
        d->m_value = qint32(d->m_numericValue.s16);
    }
        break;

    case Zcl32BitInt:
    {
        stream >> d->m_numericValue.s32;
        d->m_value = qint32(d->m_numericValue.s32);
    }
        break;

    case Zcl24BitInt:
    case Zcl40BitInt:
    case Zcl48BitInt:
    case Zcl56BitInt:
    {
        d->m_numericValue.s64 = 0;

        if (type.length() > 8)
        {
            return false;
        }

        // TODO the signed bit 0x80 needs to be extracted and processes.
        char bytes[8];
        stream.readRawData(bytes, type.length());
        memcpy(&d->m_numericValue.s64, bytes, type.length());
        d->m_value = qint64(d->m_numericValue.s64);
    }
        break;

    case Zcl64BitInt:
    {
        qint64 s64;
        stream >> s64;
        d->m_numericValue.s64 = s64;
        d->m_value = s64;
    }
        break;

    case ZclSingleFloat:
    {
        stream >> d->m_numericValue.u32;
        d->m_value = qreal(d->m_numericValue.real);
    }
        break;

    case ZclUtcTime:
    {
        stream >> d->m_numericValue.u32;
        d->m_value = d->m_numericValue.u32;
    }
        break;

    case Zcl128BitSecurityKey:
    {
        QByteArray key(16, (char)0x00);
        for (int i  = 0; i < 16; i++)
        {
            stream >> u8;
            key[i] = u8;
        }
        setValue(QVariant(key));
    }
    break;

    case Zcl8BitBitMap:
    case Zcl16BitBitMap:
    case Zcl24BitBitMap:
    case Zcl32BitBitMap:
    case Zcl40BitBitMap:
    case Zcl48BitBitMap:
    case Zcl56BitBitMap:
    case Zcl64BitBitMap:
    {
        d->m_numericValue.u64 = 0;

        if (type.length() > 8)
        {
            return false;
        }

        char bytes[8];
        stream.readRawData(bytes, type.length());
        memcpy(&d->m_numericValue.u64, bytes, type.length());
        setBitmap(d->m_numericValue.u64);
    }
    break;

    case Zcl24BitData:
    case Zcl40BitData:
    case Zcl48BitData:
    case Zcl56BitData:

    case Zcl24BitUint:
    case Zcl40BitUint:
    case Zcl48BitUint:
    case Zcl56BitUint:
    {
        d->m_numericValue.u64 = 0;

        if (type.length() > 8)
        {
            return false;
        }

        char bytes[8];
        stream.readRawData(bytes, type.length());
        memcpy(&d->m_numericValue.u64, bytes, type.length());
        d->m_value = quint64(d->m_numericValue.u64);
    }
    break;

    case ZclIeeeAddress:
    case Zcl64BitData:
    case Zcl64BitUint:
    {
        quint64 u64;
        stream >> u64;
        d->m_numericValue.u64 = u64;
        d->m_value = u64;
    }
    break;

    case ZclBoolean:
    {
        stream >> d->m_numericValue.u8;
        d->m_numericValue.u8 = (d->m_numericValue.u8 == 1) ? 1 : 0;
        d->m_value = d->m_numericValue.u8 == 1;
    }
        break;

    case ZclArray:
    {
        quint16 m;
        stream >> d->m_subType;
        stream >> m;
        d->m_numericValue.u64 = m;

        const ZclDataType &dt = deCONZ::zclDataBase()->dataType(d->m_subType);
        if (!dt.isValid())
        {
            return false;
        }

        if (m == 0 || m == 0xffff || m > 32)
        {
            break;
        }

        // int len = 1 + 2 + m * dt.length();
        char buf[256];

        buf[0] = d->m_subType;
        memcpy(buf + 1, &m, 2);
        // assume array is only read as single attribute
        int len = stream.readRawData(buf + 3, sizeof(buf) - 3);

        if (len != -1)
        {
            d->m_value.setValue(QByteArray(buf, len + 3));
            return true;
        }
    }
        break;


    default:
        return false;
    }

    if (stream.status() == QDataStream::ReadPastEnd)
    {
        return false;
    }

    return true;
}

/*!
    Writes the reportable change in the specifications format to the stream.

    \return true if the data was written
            false if the type is not supported or the data is not valid.
 */
bool ZclAttribute::writeReportableChangeToStream(QDataStream &stream) const
{
    bool ok = false;

    switch (dataType())
    {
    case ZclBoolean:
    case Zcl8BitUint:  stream << reportableChange().u8; return true;
    case Zcl16BitUint: stream << reportableChange().u16; return true;
    case Zcl32BitUint: stream << reportableChange().u32; return true;
    case Zcl64BitUint: stream << (quint64)reportableChange().u64; return true;

    case Zcl8BitInt:  stream << reportableChange().s8; return true;
    case Zcl16BitInt: stream << reportableChange().s16; return true;
    case Zcl32BitInt: stream << reportableChange().s32; return true;
    case Zcl64BitInt: stream << (qint64)reportableChange().s64; return true;

    case Zcl24BitUint:
    case Zcl40BitUint:
    case Zcl48BitUint:
    case Zcl56BitUint:
    {
        quint64 tmp = reportableChange().u64;
        const ZclDataType &type = zclDataBase()->dataType(dataType());

        if ((type.length() <= 0) || (type.length() > 64))
        {
            return false;
        }

        for (int i = 0; i < type.length(); i++)
        {
            stream << static_cast<quint8>(tmp & 0xff);
            tmp >>= 8;
        }
    }
        return true;

    default:
        ok = false;
        break;
    }

    return ok;
}

bool ZclAttribute::readReportableChangeFromStream(QDataStream &stream)
{
    if (stream.atEnd())
    {
        return false;
    }

    Q_D(ZclAttribute);
    ZclDataType type = zclDataBase()->dataType(dataType());

    if (!zclDataBase()->knownDataType(dataType()))
    {
        DBG_Printf(DBG_ZCLDB, "ZCL Read Attributes Datatype %02X %s"
               " not supported yet, abort\n",
               type.id(), qPrintable(type.name()));
        return false;
    }

    d->m_reportableChange.u64 = 0;
    qint64 s64;
    quint64 u64;

    switch (dataType())
    {
    case Zcl8BitUint:  stream >> d->m_reportableChange.u8; break;
    case Zcl16BitUint:  stream >> d->m_reportableChange.u16; break;
    case Zcl32BitUint:  stream >> d->m_reportableChange.u32; break;
    case Zcl64BitUint:  stream >> u64; d->m_reportableChange.u64 = u64; break;

    case Zcl24BitUint:
    case Zcl40BitUint:
    case Zcl48BitUint:
    case Zcl56BitUint:
    {
        d->m_reportableChange.u64 = 0;

        quint8 byte;
        for (int i = 0; i < type.length(); i++)
        {
            stream >> byte;
            d->m_reportableChange.u64 |= byte << 8 * i;
        }
    }
    break;

    case Zcl8BitInt:  stream >> d->m_reportableChange.s8; break;
    case Zcl16BitInt:  stream >> d->m_reportableChange.s16; break;
    case Zcl32BitInt:  stream >> d->m_reportableChange.s32; break;
    case Zcl64BitInt:  stream >> s64; d->m_reportableChange.s64 = s64; break;

    case ZclBoolean:   stream >> d->m_reportableChange.u8; break;

    default:
        return false;
    }

    return true;
}

void ZclAttribute::setFormatHint(ZclAttribute::FormatHint formatHint)
{
    Q_D(ZclAttribute);
    d->m_formatHint = formatHint;
}

ZclAttribute::FormatHint ZclAttribute::formatHint() const
{
    Q_D(const ZclAttribute);
    return d->m_formatHint;
}

int ZclAttribute::rangeMin() const
{
    Q_D(const ZclAttribute);
    return d->m_rangeMin;
}

void ZclAttribute::setRangeMin(int rangeMin)
{
    Q_D(ZclAttribute);
    d->m_rangeMin = rangeMin;
}

int ZclAttribute::rangeMax() const
{
    Q_D(const ZclAttribute);
    return d->m_rangeMax;
}

void ZclAttribute::setRangeMax(int rangeMax)
{
    Q_D(ZclAttribute);
    d->m_rangeMax = rangeMax;
}

quint16 ZclAttribute::manufacturerCode() const
{
    Q_D(const ZclAttribute);
    return d->m_manufacturerCode;
}

ManufacturerCode_t ZclAttribute::manufacturerCode_t() const
{
    Q_D(const ZclAttribute);
    return ManufacturerCode_t(d->m_manufacturerCode);
}

void ZclAttribute::setManufacturerCode(quint16 mfcode)
{
    Q_D(ZclAttribute);
    d->m_manufacturerCode = mfcode;
}

void ZclAttribute::setManufacturerCode(ManufacturerCode_t mfcode)
{
    Q_D(ZclAttribute);
    d->m_manufacturerCode = static_cast<quint16>(mfcode);
}

bool ZclAttribute::isManufacturerSpecific() const
{
    Q_D(const ZclAttribute);
    return d->m_manufacturerCode != 0;
}

void ZclAttribute::setAttributeSet(quint16 attrSetId, quint16 mfcode)
{
    Q_D(ZclAttribute);
    d->m_attrSetId = attrSetId;
    d->m_attrSetManufacturerCode = mfcode;
}

quint16 ZclAttribute::attributeSet() const
{
    Q_D(const ZclAttribute);
    return d->m_attrSetId;
}

quint16 ZclAttribute::attributeSetManufacturerCode() const
{
    Q_D(const ZclAttribute);
    return d->m_attrSetManufacturerCode;
}

void ZclAttribute::setValue(quint64 value)
{
    Q_D(ZclAttribute);
    d->m_numericValue.u64 = 0;

    switch (dataType())
    {
    case ZclBoolean:
    {
        d->m_numericValue.u8 = value > 0;
        d->m_value = value > 0;
    }
        break;

    case Zcl8BitData:
    case Zcl8BitUint:
    {
        d->m_numericValue.u8 = boundChecked(d->m_numericValue.u8, value);
        d->m_value = uint(d->m_numericValue.u8);
    }
        break;

    case ZclAttributeId:
    case ZclClusterId:
    case Zcl16BitData:
    case Zcl16BitUint:
    {
        d->m_numericValue.u16 = boundChecked(d->m_numericValue.u16, value);
        d->m_value = uint(d->m_numericValue.u16);
    }
        break;

    case Zcl24BitUint:
    case Zcl24BitData:
    case Zcl32BitData:
    case Zcl32BitUint:
    case ZclUtcTime:
    {
        d->m_numericValue.u32 = boundChecked(d->m_numericValue.u32, value);
        d->m_value = uint(d->m_numericValue.u32);
    }
        break;

    case Zcl40BitData:
    case Zcl40BitUint:
    case Zcl48BitData:
    case Zcl48BitUint:
    case Zcl56BitData:
    case Zcl56BitUint:
    case ZclIeeeAddress:
    case Zcl64BitData:
    case Zcl64BitUint:
    {
        d->m_numericValue.u64 = value;
        d->m_value = value;
    }
        break;

    case Zcl8BitEnum:
    case Zcl16BitEnum:
    {
        setEnumerator(value);
    }
        break;

    case Zcl8BitBitMap:
    case Zcl16BitBitMap:
    case Zcl24BitBitMap:
    case Zcl32BitBitMap:
    case Zcl40BitBitMap:
    case Zcl48BitBitMap:
    case Zcl56BitBitMap:
    case Zcl64BitBitMap:
    {
       setBitmap(value);
    }
        break;

    default:
        {
            DBG_Printf(DBG_ERROR, "ZclAttribute::setValue() unsupported datatype 0x%02X\n", dataType());
        }
        break;
    }
}

void ZclAttribute::setValue(bool value)
{
    Q_D(ZclAttribute);

    if (dataType() == ZclBoolean)
    {
        d->m_numericValue.u64 = 0;
        d->m_numericValue.u8 = (value == true) ? 1 : 0;
        d->m_value.setValue(value);
    }
}

void ZclAttribute::setValue(qint64 value)
{
    Q_D(ZclAttribute);

    d->m_numericValue.s64 = 0;

    switch (dataType())
    {
    case ZclBoolean:
    {
        d->m_numericValue.u8 = value > 0;
        d->m_value = value > 0;
    }
        break;

    case Zcl8BitInt:
    {
        d->m_numericValue.s8 = boundChecked(d->m_numericValue.s8, value);
        d->m_value = int(d->m_numericValue.s8);
    }
        break;

    case Zcl16BitInt:
    {
        d->m_numericValue.s16 = boundChecked(d->m_numericValue.s16, value);
        d->m_value = int(d->m_numericValue.s16);
    }
        break;

    case Zcl24BitInt:
    case Zcl32BitInt:
    {
        d->m_numericValue.s32 = boundChecked(d->m_numericValue.s32, value);
        d->m_value = int(d->m_numericValue.s32);
    }
        break;

    case Zcl40BitInt:
    case Zcl48BitInt:
    case Zcl56BitInt:
    case Zcl64BitInt:
    {
        d->m_numericValue.s64 = value;
        d->m_value = value;
    }
        break;

    case Zcl8BitEnum:
    case Zcl16BitEnum:
    {
        setEnumerator(uint(value));
    }
        break;

    case Zcl8BitBitMap:
    case Zcl16BitBitMap:
    case Zcl24BitBitMap:
    case Zcl32BitBitMap:
    case Zcl40BitBitMap:
    case Zcl48BitBitMap:
    case Zcl56BitBitMap:
    case Zcl64BitBitMap:
    {
        setBitmap(static_cast<quint64>(value));
    }
        break;

    default:
    {
        DBG_Printf(DBG_INFO, "ZclAttribute::setValue(qint64 value) for unsupported datatype 0x%02X\n", dataType());
    }
        break;
    }
}

void ZclAttribute::setValue(const QVariant &value)
{
    bool ok;
    Q_D(ZclAttribute);

    if (dataType() == ZclSingleFloat)
    {
        const float val = value.toFloat(&ok);
        if (!ok)
        {
            DBG_Printf(DBG_ZCLDB, "ZclAttribute 0x%04X can't set float\n", id());
        }
        else
        {
            d->m_numericValue.real = val;
            d->m_value = value;
        }
    }
    else if (dataType() >= Zcl8BitBitMap && (dataType() <= Zcl64BitBitMap))
    {
        qulonglong bmp = value.toULongLong(&ok);

        if (!ok)
        {
            DBG_Printf(DBG_ZCLDB, "ZclAttribute 0x%04X cant't set bitmap mask\n", id());
        }
        else
        {
            setBitmap(bmp);
        }
    }
    else if ((dataType() == Zcl8BitEnum) || (dataType() == Zcl16BitEnum))
    {
        const uint enumerator = value.toUInt(&ok);

        if (!ok)
        {
            DBG_Printf(DBG_ZCLDB, "ZclAttribute 0x%04X cant't set enumerator\n", id());
        }
        else
        {
            setEnumerator(enumerator);
        }
    }
    else if (dataType() >= Zcl8BitUint && (dataType() <= Zcl64BitUint))
    {
        const quint64 val = value.toULongLong(&ok);

        if (!ok)
        {
            DBG_Printf(DBG_ZCLDB, "ZclAttribute 0x%04X cant't set value\n", id());
        }
        else
        {
            setValue(val);
        }
    }
    else if (dataType() >= Zcl8BitInt && (dataType() <= Zcl64BitInt))
    {
        const qint64 val = value.toLongLong(&ok);

        if (!ok)
        {
            DBG_Printf(DBG_ZCLDB, "ZclAttribute 0x%04X cant't set value\n", id());
        }
        else
        {
            setValue(val);
        }
    }
    else if (dataType() >= Zcl8BitData && (dataType() <= Zcl64BitData))
    {
        const quint64 val = value.toULongLong(&ok);

        if (!ok)
        {
            DBG_Printf(DBG_ZCLDB, "ZclAttribute 0x%04X cant't set value\n", id());
        }
        else
        {
            setValue(val);
        }
    }
    else if (dataType() == ZclBoolean)
    {
        d->m_numericValue.u64 = 0;
        d->m_numericValue.u8 = value.toBool() ? 1 : 0;
        d->m_value = value;
    }
    else
    {
        d->m_value = value;
    }
}

void ZclAttribute::setLastRead(int64_t time)
{
    Q_D(ZclAttribute);
    d->m_lastRead = time;
}

uint16_t ZclAttribute::listSizeAttribute() const
{
    Q_D(const ZclAttribute);
    return d->m_listSizeAttr;
}

void ZclAttribute::setListSizeAttribute(uint16_t id)
{
    Q_D(ZclAttribute);
    d->m_listSizeAttr = id;
}

bool ZclAttribute::isList() const
{
    Q_D(const ZclAttribute);
    return d->m_listSizeAttr != 0xFFFF;
}

int ZclAttribute::listSize() const
{
    Q_D(const ZclAttribute);
    return d->m_listSize;
}

void ZclAttribute::setListSize(int listSize)
{
    Q_D(ZclAttribute);
    d->m_listSize = listSize;
}

int64_t ZclAttribute::lastRead() const
{
    Q_D(const ZclAttribute);
    return d->m_lastRead;
}

bool ZclAttribute::isReadonly() const
{
    Q_D(const ZclAttribute);
    return (d->m_access == ZclRead);
}

bool ZclAttribute::isMandatory() const
{
    Q_D(const ZclAttribute);
    return d->m_required;
}

bool ZclAttribute::isAvailable() const
{
    Q_D(const ZclAttribute);
    return d->m_avail;
}

void ZclAttribute::setAvailable(bool available)
{
    Q_D(ZclAttribute);
    d->m_avail = available;
}

uint8_t ZclAttribute::numericBase() const
{
    Q_D(const ZclAttribute);
    return d->m_numericBase;
}

void ZclAttribute::setNumericBase(uint8_t base)
{
    Q_D(ZclAttribute);
    d->m_numericBase = base;
}

uint ZclAttribute::enumerator() const
{
    Q_D(const ZclAttribute);
    return d->m_numericValue.u32;
}

void ZclAttribute::setEnumerator(uint value)
{
    Q_D(ZclAttribute);
    d->m_numericValue.u32 = value;
    d->m_value = value;
}

void ZclAttribute::setBit(uint bit, bool one)
{
    Q_D(ZclAttribute);

    if (bit < 64)
    {
        if (one) d->m_valueState.bitmap |= (1ULL << (uint64_t)bit);
        else     d->m_valueState.bitmap &= ~(1ULL << (uint64_t)bit);

        d->m_value = quint64(d->m_valueState.bitmap);
    }
}

bool ZclAttribute::bit(int bit) const
{
    Q_D(const ZclAttribute);
    if (bit >= 0 && bit < 64)
        return d->m_valueState.bitmap & (1ULL << (uint64_t)bit);
    return false;
}

int ZclAttribute::bitCount() const
{
    switch (dataType()) {
    case Zcl8BitBitMap:  return  8;
    case Zcl16BitBitMap: return 16;
    case Zcl24BitBitMap: return 24;
    case Zcl32BitBitMap: return 32;
    case Zcl40BitBitMap: return 40;
    case Zcl48BitBitMap: return 48;
    case Zcl56BitBitMap: return 56;
    case Zcl64BitBitMap: return 64;
    default:
        break;
    }

    return 0;
}

quint64 ZclAttribute::bitmap() const
{
    Q_D(const ZclAttribute);
    return d->m_valueState.bitmap;
}

void ZclAttribute::setBitmap(quint64 bmp)
{
    Q_D(ZclAttribute);
    d->m_valueState.bitmap = bmp;
    d->m_value = quint64(bmp);
}

int ZclAttribute::enumCount() const
{
    Q_D(const ZclAttribute);
    return int(d->m_valuePos.size());
}

quint8 ZclAttribute::enumerationId() const
{
    Q_D(const ZclAttribute);
    return d->m_enumerationId;
}

void ZclAttribute::setEnumerationId(quint8 id)
{
    Q_D(ZclAttribute);
    d->m_enumerationId = id;
}

QString ZclAttribute::toString(ZclAttribute::FormatHint formatHint) const
{
    const ZclDataType &dataType = zclDataBase()->dataType(this->dataType());
    return toString(dataType, formatHint);
}

QString ZclAttribute::toString(const ZclDataType &dataType, ZclAttribute::FormatHint formatHint) const
{
    QString str;
    Q_D(const ZclAttribute);

    int fieldWidth = 0; // auto
    if (numericBase() == 16)
    {
        fieldWidth = dataType.length() * 2;
    }

    switch (d->m_dataType)
    {
    case ZclBoolean:
        str = (d->m_numericValue.u8 == 1) ? QLatin1String("true") : QLatin1String("false");
        break;

    case ZclOctedString:
    {
        if (d->m_value.isValid())
        {
            const QByteArray arr = d->m_value.toByteArray();
            if (arr.size() > 0)
            {
                str = QLatin1String("0x") + arr.toHex();
            }
        }
    }
        break;
    case ZclCharacterString:
    {
        if (d->m_value.isValid())
        {
            if (d->m_formatHint == ZclAttribute::Prefix)
            {
                const QByteArray arr = d->m_value.toByteArray();
                if (arr.size() > 0)
                {
                    str = QLatin1String("0x") + arr.toHex();
                }
            }
            else
            {
                str = d->m_value.toString();
            }
        }
    }
        break;

    case Zcl8BitData:
    case Zcl8BitUint:
        str = QString("%1").arg((quint16)d->m_numericValue.u8, fieldWidth, (int)numericBase(), QChar('0'));
        break;
    case ZclAttributeId:
    case ZclClusterId:
    case Zcl16BitUint:
    case Zcl16BitData:
        str = QString("%1").arg(d->m_numericValue.u16, fieldWidth, (int)numericBase(), QChar('0'));
        break;
    case Zcl24BitUint:
    case Zcl32BitUint:
        str = QString("%1").arg(d->m_numericValue.u32, fieldWidth, (int)numericBase(), QChar('0'));
        break;

    case Zcl40BitData:
    case Zcl48BitData:
    case Zcl56BitData:
    case Zcl64BitData:

    case Zcl40BitUint:
    case Zcl48BitUint:
    case Zcl56BitUint:
    case Zcl64BitUint:
        str = QString("%1").arg(d->m_numericValue.u64, fieldWidth, (int)numericBase(), QChar('0'));
        break;

    case ZclIeeeAddress:
        str = QString("%1").arg(d->m_numericValue.u64, (int)16, (int)16, QChar('0'));
        break;

    case Zcl8BitInt:
        str = QString("%1").arg((qint16)d->m_numericValue.s8, fieldWidth, (int)numericBase(), QChar('0'));
        break;
    case Zcl16BitInt:
        str = QString("%1").arg(d->m_numericValue.s16, fieldWidth, (int)numericBase(), QChar('0'));
        break;
    case Zcl24BitInt:
    case Zcl32BitInt:
        str = QString("%1").arg(d->m_numericValue.s32, fieldWidth, (int)numericBase(), QChar('0'));
        break;

    case Zcl40BitInt:
    case Zcl48BitInt:
    case Zcl56BitInt:
    case Zcl64BitInt:
        str = QString("%1").arg(d->m_numericValue.s64, fieldWidth, (int)numericBase(), QChar('0'));
        break;

    case Zcl8BitBitMap:
    case Zcl16BitBitMap:
    case Zcl24BitBitMap:
    case Zcl32BitBitMap:
    case Zcl40BitBitMap:
    case Zcl48BitBitMap:
    case Zcl56BitBitMap:
    case Zcl64BitBitMap:
    {
        str = QString("%1").arg(bitmap(), (int)dataType.length() * 2, (int)16, QChar('0'));
    }
        break;

    case Zcl8BitEnum:
    case Zcl16BitEnum:
        str = valueNameAt(enumerator());
        if (str.isEmpty())
        {
            str = QString("%1").arg(enumerator(), (int)dataType.length() * 2, (int)16, QChar('0'));
        }
        break;

    case ZclSingleFloat:
        //DBG_Printf(DBG_INFO, "float: 0x%08X\n", d->m_numericValue.u32);
        str.setNum(d->m_numericValue.real, 'g', 6);
        break;

    case ZclUtcTime:
    {
        // UTCTime is an unsigned 32-bit value representing the number of seconds since 0 hours, 0 minutes,
        // 0 seconds, on the 1st of January, 2000 UTC (Universal Coordinated Time). The value that
        // represents an invalid value of this type is 0xffffffff.

        QDateTime utc = QDateTime::fromString(QLatin1String("2000-01-01T00:00:00Z"), Qt::ISODate);
        str = utc.addSecs(d->m_numericValue.u32).toString();
    }
        break;

    case Zcl128BitSecurityKey:
    {
        if (d->m_value.isValid())
        {
            str = d->m_value.toByteArray().toHex();
        }
    }
        break;

    case ZclArray:
    {
        if (d->m_value.isValid())
        {
            str = d->m_value.toByteArray().toHex();
        }
    }
        break;


    default:
    {
        DBG_Printf(DBG_ZCLDB, "ZclAttribute::toString()) no string support for data type %s\n", qPrintable(dataType.name()));
    }
        break;
    }

    switch (formatHint)
    {
    case Prefix:
        if (numericBase() == 16)
        {
            str.prepend("0x");
        }
        else if (numericBase() == 2)
        {
            str.prepend("0b");
        }
        break;

    default:
        break;
    }

    return str;
}

const QVariant &ZclAttribute::toVariant() const
{
    Q_D(const ZclAttribute);
    return d->m_value;
}

uint16_t ZclAttribute::minReportInterval() const
{
    Q_D(const ZclAttribute);
    return d->m_minReportInterval;
}

void ZclAttribute::setMinReportInterval(uint16_t interval)
{
    Q_D(ZclAttribute);
    d->m_minReportInterval = interval;
}

uint16_t ZclAttribute::maxReportInterval() const
{
    Q_D(const ZclAttribute);
    return d->m_maxReportInterval;
}

void ZclAttribute::setMaxReportInterval(uint16_t interval)
{
    Q_D(ZclAttribute);
    d->m_maxReportInterval = interval;
}

uint16_t ZclAttribute::reportTimeoutPeriod() const
{
    Q_D(const ZclAttribute);
    return d->m_reportTimeout;
}

void ZclAttribute::setReportTimeoutPeriod(uint16_t period)
{
    Q_D(ZclAttribute);
    d->m_reportTimeout = period;
}

const NumericUnion &ZclAttribute::reportableChange() const
{
    Q_D(const ZclAttribute);
    return d->m_reportableChange;
}

void ZclAttribute::setReportableChange(const NumericUnion &reportableChange)
{
    Q_D(ZclAttribute);
    d->m_reportableChange = reportableChange;
}

ZclAttributeSetPrivate::ZclAttributeSetPrivate() :
    id(0xFFFF),
    manufacturerCode(0)
{
}

ZclAttributeSet::ZclAttributeSet() :
    d_ptr(new ZclAttributeSetPrivate)
{
}

ZclAttributeSet::~ZclAttributeSet()
{
    delete d_ptr;
    d_ptr = 0;
}

ZclAttributeSet::ZclAttributeSet(const ZclAttributeSet &other) :
    d_ptr(new ZclAttributeSetPrivate(*other.d_ptr))
{
}

ZclAttributeSet &ZclAttributeSet::operator= (const ZclAttributeSet &other)
{
    // Self assignment?
    if (this == &other)
    {
        return *this;
    }

    DBG_Assert(other.d_ptr != 0);
    *this->d_ptr = *other.d_ptr;
    return *this;
}

ZclAttributeSet::ZclAttributeSet(uint16_t id, const QString &description) :
    d_ptr(new ZclAttributeSetPrivate)
{
    Q_D(ZclAttributeSet);
    d->id = id;
    d->description = description;

#ifdef ZCL_LOAD_DBG
    qDebug("Attribute Set: %04X -- %s", id, qPrintable(description));
#endif
}

uint16_t ZclAttributeSet::id() const
{
    Q_D(const ZclAttributeSet);
    return d->id;
}

const QString &ZclAttributeSet::description() const
{
    Q_D(const ZclAttributeSet);
    return d->description;
}

const std::vector<int> &ZclAttributeSet::attributes() const
{
    Q_D(const ZclAttributeSet);
    return d->attributeIndexes;
}

void ZclAttributeSet::addAttribute(int idx)
{
    Q_D(ZclAttributeSet);
    d->attributeIndexes.push_back(idx);
}

quint16 ZclAttributeSet::manufacturerCode() const
{
    Q_D(const ZclAttributeSet);
    return d->manufacturerCode;
}

void ZclAttributeSet::setManufacturerCode(quint16 mfcode)
{
    Q_D(ZclAttributeSet);
    d->manufacturerCode = mfcode;
}

ZclCommandPrivate::ZclCommandPrivate() :
    m_id (0xFF),
    m_manufacturerId(0),
    m_responseId(0xFF),
    m_required(false),
    m_recv(false),
    m_isProfileWide(false),
    m_disableDefaultResponse(false)
{
}

ZclCommand::ZclCommand() :
    d_ptr(new ZclCommandPrivate)
{

}

ZclCommand::ZclCommand(const ZclCommand &other) :
    d_ptr(new ZclCommandPrivate(*other.d_ptr))
{
}

ZclCommand::ZclCommand(ZclCommand &&other) noexcept :
    d_ptr(other.d_ptr)
{
    Q_ASSERT(d_ptr);
    Q_ASSERT(other.d_ptr);
    other.d_ptr = nullptr;
}

ZclCommand &ZclCommand::operator=(const ZclCommand &other)
{
    // Self assignment?
    if (this == &other)
    {
        return *this;
    }

    Q_ASSERT(d_ptr);
    Q_ASSERT(other.d_ptr);
    *d_ptr = *other.d_ptr;
    return *this;
}

ZclCommand &ZclCommand::operator=(ZclCommand &&other) noexcept
{
    // Self assignment?
    if (this == &other)
    {
        return *this;
    }

    Q_ASSERT(other.d_ptr);

    if (d_ptr)
    {
        delete d_ptr;
    }

    d_ptr = other.d_ptr;
    other.d_ptr = nullptr;
    Q_ASSERT(d_ptr);

    return *this;
}

ZclCommand::ZclCommand(uint8_t id, const QString &name, bool required, bool recv, const QString &description) :
    d_ptr(new ZclCommandPrivate)
{
    Q_D(ZclCommand);
    d->m_id = id;
    d->m_responseId = 0xFF;
    d->m_name = name;
    d->m_required = required;
    d->m_recv = recv;
    d->m_description = description;
    d->m_isProfileWide = false;
    d->m_disableDefaultResponse = false;
#ifdef ZCL_LOAD_DBG_COMMAND
    qDebug("Command: %s %02X", qPrintable(name), id);
#endif
}

ZclCommand::~ZclCommand()
{
    if (d_ptr)
    {
        delete d_ptr;
        d_ptr = nullptr;
    }
}

uint8_t ZclCommand::id() const
{
    Q_D(const ZclCommand);
    return d->m_id;
}

ZclCommandId_t ZclCommand::id_t() const
{
    Q_D(const ZclCommand);
    return ZclCommandId_t(d->m_id);
}

void ZclCommand::setId(uint8_t id)
{
    Q_D(ZclCommand);
    d->m_id = id;
}

bool ZclCommand::isValid() const
{
    Q_D(const ZclCommand);
    return (d->m_id != 0xFF);
}


uint16_t ZclCommand::manufacturerId() const
{
    Q_D(const ZclCommand);
    return d->m_manufacturerId;
}

void ZclCommand::setManufacturerId(uint16_t id)
{
    Q_D(ZclCommand);
    d->m_manufacturerId = id;
}

uint8_t ZclCommand::responseId() const
{
    Q_D(const ZclCommand);
    return d->m_responseId;
}

void ZclCommand::setResponseId(uint8_t id)
{
    Q_D(ZclCommand);
    d->m_responseId = id;
}

bool ZclCommand::hasResponse() const
{
    Q_D(const ZclCommand);
    return (d->m_responseId != 0xFF);
}

bool ZclCommand::directionReceived() const
{
    Q_D(const ZclCommand);
    return d->m_recv;
}

bool ZclCommand::directionSend() const
{
    Q_D(const ZclCommand);
    return !d->m_recv;
}

const QString &ZclCommand::name() const
{
    Q_D(const ZclCommand);
    return d->m_name;
}

const QString &ZclCommand::description() const
{
    Q_D(const ZclCommand);
    return d->m_description;
}

void ZclCommand::setDescription(const QString &description)
{
    Q_D(ZclCommand);
    d->m_description = description;
}

bool ZclCommand::isProfileWide() const
{
    Q_D(const ZclCommand);
    return d->m_isProfileWide;
}

void ZclCommand::setIsProfileWide(bool profileWide)
{
    Q_D(ZclCommand);
    d->m_isProfileWide = profileWide;
}

bool ZclCommand::disableDefaultResponse() const
{
    Q_D(const ZclCommand);
    return d->m_disableDefaultResponse;
}

void ZclCommand::setDisableDefaultResponse(bool disable)
{
    Q_D(ZclCommand);
    d->m_disableDefaultResponse = disable;
}

const std::vector<ZclAttribute> &ZclCommand::parameters() const
{
    Q_D(const ZclCommand);
    return d->m_payload;
}

std::vector<ZclAttribute> &ZclCommand::parameters()
{
    Q_D(ZclCommand);
    return d->m_payload;
}

bool ZclCommand::readFromStream(QDataStream &stream)
{
    bool ok = true;
    Q_D(ZclCommand);

    std::vector<ZclAttribute>::iterator i = d->m_payload.begin();
    std::vector<ZclAttribute>::iterator end = d->m_payload.end();

    for (; i != end; ++i)
    {
        if (ok && !i->readFromStream(stream))
        {
            ok = false;
        }

        // clear all following values
//        if (!ok)
//        {
//            i->setValue(QVariant());
//        }
    }

    return ok;
}

bool ZclCommand::writeToStream(QDataStream &stream) const
{
    bool ok = true;
    Q_D(const ZclCommand);
    std::vector<ZclAttribute>::const_iterator i = d->m_payload.begin();
    std::vector<ZclAttribute>::const_iterator end = d->m_payload.end();

    for (; i != end; ++i)
    {
        if (ok && !i->writeToStream(stream))
        {
            ok = false;
        }

//        // clear all following values
//        if (!ok)
//        {
//            i->setValue(QVariant());
//        }
    }

    return ok;
}

ZclClusterPrivate::ZclClusterPrivate() :
    id(0xFFFF),
    oppositeId(0xFFFF),
    manufacturerCode(0),
    isZcl(true),
    isServer(false)
{

}

ZclCluster::ZclCluster() :
    d_ptr(new ZclClusterPrivate)
{
}

ZclCluster::ZclCluster(const ZclCluster &other)
    : d_ptr(new ZclClusterPrivate(*other.d_ptr))
{
}

ZclCluster::ZclCluster(uint16_t id, const QString &name, const QString &description) :
    d_ptr(new ZclClusterPrivate)
{
    Q_D(ZclCluster);
    d->id = id;
    d->oppositeId = id;
    d->name = name;
    d->description = description;
    d->isZcl = true;
    d->isServer = false;

#ifdef ZCL_LOAD_DBG
           qDebug("Cluster: %s %04X", qPrintable(name), id);
#endif
}

ZclCluster &ZclCluster::operator= (const ZclCluster &other)
{
    // Self assignment?
    if (this == &other)
    {
        return *this;
    }

    DBG_Assert(other.d_ptr != 0);
    *this->d_ptr = *other.d_ptr;
    return *this;
}

ZclCluster::~ZclCluster()
{
    delete d_ptr;
    d_ptr = 0;
}

uint16_t ZclCluster::id() const
{
    return d_ptr->id;
}

ZclClusterId_t ZclCluster::id_t() const
{
    Q_D(const ZclCluster);
    return ZclClusterId_t(d->id);
}

void ZclCluster::setId(uint16_t id)
{
    d_ptr->id = id;
}

uint16_t ZclCluster::oppositeId() const
{
    return d_ptr->oppositeId;
}

void ZclCluster::setOppositeId(uint16_t id)
{
    d_ptr->oppositeId = id;
}

uint16_t ZclCluster::manufacturerCode() const
{
    return d_ptr->manufacturerCode;
}

ManufacturerCode_t ZclCluster::manufacturerCode_t() const
{
    return ManufacturerCode_t(d_ptr->manufacturerCode);
}

void ZclCluster::setManufacturerCode(uint16_t manufacturerCode)
{
    d_ptr->manufacturerCode = manufacturerCode;
}

void ZclCluster::setManufacturerCode(ManufacturerCode_t manufacturerCode)
{
    d_ptr->manufacturerCode = static_cast<quint16>(manufacturerCode);
}

const QString &ZclCluster::name() const
{
    return d_ptr->name;
}

const QString &ZclCluster::description() const
{
    return d_ptr->description;
}

void ZclCluster::setDescription(const QString &description)
{
    d_ptr->description = description;
}

bool ZclCluster::isValid() const
{
    return !d_ptr->name.isEmpty() && d_ptr->name != QLatin1String("unknown");
}

bool ZclCluster::isZcl() const
{
    return d_ptr->isZcl;
}

void ZclCluster::setIsZcl(bool isZcl)
{
    d_ptr->isZcl = isZcl;
}

bool ZclCluster::isServer() const
{
    return d_ptr->isServer;
}

bool ZclCluster::isClient() const
{
    return !isServer();
}

void ZclCluster::setIsServer(bool isServer)
{
    d_ptr->isServer = isServer;
}

bool ZclCluster::readCommand(const deCONZ::ApsDataIndication &ind)
{
    if (isZcl())
    {
        return false;
    }

    if (commands().size() != 1)
    {
        DBG_Printf(DBG_ZCLDB, "%s just one non ZCL command supported per cluster\n", Q_FUNC_INFO);
        return false;
    }

    QDataStream stream(ind.asdu());
    stream.setByteOrder(QDataStream::LittleEndian);

    return commands().front().readFromStream(stream);
}

std::vector<ZclAttribute> &ZclCluster::attributes()
{
    return d_ptr->attributes;
}

const std::vector<ZclAttribute> &ZclCluster::attributes() const
{
    return d_ptr->attributes;
}

std::vector<ZclAttributeSet> &ZclCluster::attributeSets()
{
    return d_ptr->attributeSets;
}

const std::vector<ZclAttributeSet> &ZclCluster::attributeSets() const
{
    return d_ptr->attributeSets;
}

std::vector<ZclCommand> &ZclCluster::commands()
{
    return d_ptr->commands;
}

const std::vector<ZclCommand> &ZclCluster::commands() const
{
    return d_ptr->commands;
}

bool ZclCluster::readCommand(const ZclFrame &zclFrame)
{
    if (!isZcl())
    {
        return false;
    }

    std::vector<ZclCommand>::iterator i = commands().begin();
    std::vector<ZclCommand>::iterator end = commands().end();
    QDataStream stream(zclFrame.payload());
    stream.setByteOrder(QDataStream::LittleEndian);

    for (; i != end; ++i)
    {
        if (i->id() != zclFrame.commandId())
        {
            continue;
        }

        if (isServer())
        {
            if (i->directionReceived() && ((zclFrame.frameControl() & ZclFCDirectionServerToClient) == 0))
            {
                return i->readFromStream(stream);
            }
            else if (i->directionSend() && (zclFrame.frameControl() & ZclFCDirectionServerToClient))
            {
                return i->readFromStream(stream);
            }
        }
        else if (isClient())
        {
            if (i->directionReceived() && (zclFrame.frameControl() & ZclFCDirectionServerToClient))
            {
                return i->readFromStream(stream);
            }
            else if (i->directionSend() && ((zclFrame.frameControl() & ZclFCDirectionServerToClient) == 0))
            {
                return i->readFromStream(stream);
            }
        }
    }

    return false;
}

ZclFrame::ZclFrame() :
    d_ptr(MEM_AllocItem<ZclFramePrivate>(&zclMemPriv->mem))
{
    if (d_ptr->payload.size() > 0)
    {
        d_ptr->payload.reserve(d_ptr->payload.size()); // keep alloc
        d_ptr->payload.resize(0);
        Q_ASSERT(d_ptr->payload.capacity() > 0);
    }

    d_ptr->valid = 0;
    d_ptr->frameControl = 0;
    d_ptr->manufacturerCode = 0xFFFF;
    d_ptr->seqNumber = 0;
    d_ptr->commandId = 0;
}

ZclFrame::ZclFrame(const ZclFrame &other) :
    d_ptr(MEM_AllocItem<ZclFramePrivate>(&zclMemPriv->mem))
{
    *d_ptr = *other.d_ptr;
}

ZclFrame &ZclFrame::operator=(const ZclFrame &other)
{
    // Self assignment?
    if (this == &other)
    {
        return *this;
    }

    DBG_Assert(other.d_ptr != 0);
    *d_ptr = *other.d_ptr;
    return *this;
}

ZclFrame::~ZclFrame()
{
    MEM_DeallocItem<ZclFramePrivate>(d_ptr, &zclMemPriv->mem);
    d_ptr = nullptr;
}

uint8_t ZclFrame::frameControl() const
{
    Q_D(const ZclFrame);
    return d->frameControl;
}

void ZclFrame::setFrameControl(uint8_t frameControl)
{
    Q_D(ZclFrame);
    d->frameControl = frameControl;
}

uint16_t ZclFrame::manufacturerCode() const
{
    Q_D(const ZclFrame);
    return d->manufacturerCode;
}

ManufacturerCode_t ZclFrame::manufacturerCode_t() const
{
    Q_D(const ZclFrame);
    return ManufacturerCode_t(d->manufacturerCode);
}

void ZclFrame::setManufacturerCode(uint16_t code)
{
    Q_D(ZclFrame);
    d->manufacturerCode = code;
}

void ZclFrame::setManufacturerCode(ManufacturerCode_t code)
{
    Q_D(ZclFrame);
    d->manufacturerCode = static_cast<quint16>(code);
}

uint8_t ZclFrame::sequenceNumber() const
{
    Q_D(const ZclFrame);
    return d->seqNumber;
}

void ZclFrame::setSequenceNumber(uint8_t seqNumber)
{
    Q_D(ZclFrame);
    d->seqNumber = seqNumber;
}

uint8_t ZclFrame::commandId() const
{
    Q_D(const ZclFrame);
    return d->commandId;
}

ZclCommandId_t ZclFrame::commandId_t() const
{
    Q_D(const ZclFrame);
    return ZclCommandId_t(d->commandId);
}

void ZclFrame::setCommandId(uint8_t commandId)
{
    Q_D(ZclFrame);
    d->commandId = commandId;
}

void ZclFrame::setCommandId(ZclCommandId_t commandId)
{
    Q_D(ZclFrame);
    d->commandId = static_cast<quint8>(commandId);
}

unsigned char ZclFrame::payloadAt(int index) const
{
    Q_D(const ZclFrame);
    if (index >= 0 && index < d->payload.size())
        return static_cast<unsigned char>(d->payload.at(index));
    return 0;
}

const QByteArray &ZclFrame::payload() const
{
    Q_D(const ZclFrame);
    return d->payload;
}

QByteArray &ZclFrame::payload()
{
    Q_D(ZclFrame);
    return d->payload;
}

void ZclFrame::setPayload(const QByteArray &payload)
{
    Q_D(ZclFrame);
    d->payload = payload;
}

void ZclFrame::writeToStream(QDataStream &stream)
{
    stream << (quint8)frameControl();
    if (frameControl() & ZclFCManufacturerSpecific)
    {
        stream << manufacturerCode();
    }
    stream << sequenceNumber();
    stream << commandId();

    for (int i = 0; i < payload().size(); i++)
    {
        stream << (quint8)payload()[i];
    }
}

void ZclFrame::readFromStream(QDataStream &stream)
{
    quint8 u8;
    Q_D(ZclFrame);

    d->valid = 0;

    stream >> d->frameControl;
    if (d->frameControl & 0x04)
    {
        stream >> d->manufacturerCode;
    }
    else
    {
        d->manufacturerCode = 0x0000;
    }

    stream >> d->seqNumber;
    stream >> d->commandId;

    if (stream.status() != QDataStream::ReadPastEnd)
    {
        d->valid = 1;
    }

    d->payload.clear();
    while (!stream.atEnd())
    {
        stream >> u8;
        d->payload.append(u8);
    }
}

bool ZclFrame::isClusterCommand() const
{
    Q_D(const ZclFrame);
    return (d->frameControl & ZclFCClusterCommand);
}

bool ZclFrame::isProfileWideCommand() const
{
    return !isClusterCommand();
}

bool ZclFrame::isDefaultResponse() const
{
    return isProfileWideCommand() && (commandId() == ZclDefaultResponseId);
}

bool ZclFrame::isValid() const
{
    Q_D(const ZclFrame);
    return d->valid == 1;
}

uint8_t ZclFrame::defaultResponseCommandId() const
{
    Q_D(const ZclFrame);
    return d->payload.size() == 2 ? d->payload[0] : 0xFF;
}

ZclCommandId_t ZclFrame::defaultResponseCommandId_t() const
{
    Q_D(const ZclFrame);
    return ZclCommandId_t(static_cast<quint8>(d->payload.size() == 2 ? d->payload[0] : 0xFF));
}

ZclStatus ZclFrame::defaultResponseStatus() const
{
    Q_D(const ZclFrame);
    return d->payload.size() == 2 ? (ZclStatus)d->payload.at(1) : ZclFailureStatus;
}

void ZclFrame::reset()
{
    Q_D(ZclFrame);
    d->valid = 0;
    d->frameControl = 0;
    d->manufacturerCode = 0xFFFF;
    d->seqNumber = 0;
    d->commandId = 0;
    if (d->payload.size() > 0)
    {
        d->payload.reserve(d->payload.size()); // keep alloc
    }
    d->payload.resize(0);
}

QDataStream &operator<<(QDataStream &ds, ZclDataTypeId_t id)
{
    ds << static_cast<quint8>(id);
    return ds;
}

QDataStream &operator>>(QDataStream &ds, ZclDataTypeId_t &id)
{
    quint8 x;
    ds >> x;

    if (ds.status() != QDataStream::ReadPastEnd)
    {
        id = ZclDataTypeId_t(x);
    }

    return ds;
}

ZclDataTypePrivate::ZclDataTypePrivate() :
    m_id(0x00),
    m_length(0),
    m_analogDiscrete(UnknownData)
{
}

ZclDataType::ZclDataType() :
    d_ptr(new ZclDataTypePrivate)
{
}

ZclDataType::ZclDataType(const ZclDataType &other) :
    d_ptr(new ZclDataTypePrivate(*other.d_ptr))
{
}

ZclDataType &ZclDataType::operator=(ZclDataType &other)
{
    // Self assignment?
    if (this == &other)
    {
        return *this;
    }

    *this->d_ptr = *other.d_ptr;
    return *this;
}

ZclDataType::~ZclDataType()
{
    delete d_ptr;
    d_ptr = 0;
}

ZclDataType::ZclDataType(uint8_t id, const QString &name, const QString &shortname, int length, char analogDiscrete) :
    d_ptr(new ZclDataTypePrivate)
{
    Q_D(ZclDataType);
    d->m_id = id;
    d->m_name = name;
    d->m_shortname = shortname;
    d->m_length = length;

    switch (analogDiscrete)
    {
    case 'a':
    case 'A':
        d->m_analogDiscrete = ZclDataTypePrivate::AnalogData;
        break;

    case 'd':
    case 'D':
        d->m_analogDiscrete = ZclDataTypePrivate::DiscreteData;
        break;

    case '-':
    default:
        d->m_analogDiscrete = ZclDataTypePrivate::UnknownData;
        break;
    }

#ifdef ZCL_LOAD_DBG
    qDebug("DataType: %02X %s %d", id, qPrintable(name), length);
#endif
}

uint8_t ZclDataType::id() const
{
    Q_D(const ZclDataType);
    return d->m_id;
}

ZclDataTypeId_t ZclDataType::id_t() const
{
    Q_D(const ZclDataType);
    return ZclDataTypeId_t(d->m_id);
}

const QString &ZclDataType::name() const
{
    Q_D(const ZclDataType);
    return d->m_name;
}

const QString &ZclDataType::shortname() const
{
    Q_D(const ZclDataType);
    return d->m_shortname;
}

int ZclDataType::length() const
{
    Q_D(const ZclDataType);
    return d->m_length;
}

bool ZclDataType::isValid() const
{
    Q_D(const ZclDataType);
    return (d->m_id != 0x00);
}

bool ZclDataType::isAnalog() const
{
    Q_D(const ZclDataType);
    return (d->m_analogDiscrete == ZclDataTypePrivate::AnalogData);
}

bool ZclDataType::isDiscrete() const
{
    Q_D(const ZclDataType);
    return (d->m_analogDiscrete == ZclDataTypePrivate::DiscreteData);
}

ZclDataBase::ZclDataBase() :
    m_unknownCluster(0xFFFF, "unknown", "unkown cluster"),
    m_unknownDataType(0x00, "No Data", "-", 0, '-')
{
#ifdef Q_OS_WIN
    m_iconPath = QString("icons") + QDir::separator();
#else
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("share/deCONZ/icons");
    m_iconPath = dir.absolutePath() + "/";
#endif
    //m_iconPath(QString("../share/deCONZ/icons") + QDir::separator())
    // singleton
    DBG_Assert(_zclDB == 0);
}

ZclDataBase::~ZclDataBase()
{
    _zclDB = 0;
}

static_assert (sizeof deCONZ::ZclNoData == 1, "Assumed enum sizeof deCONZ::ZclNoData is 1");
static_assert (sizeof deCONZ::ZclReadAttributesId == 1, "Assumed enum sizeof deCONZ::ZclReadAttributesId is 1");
static_assert (sizeof deCONZ::ZclFCProfileCommand == 1, "Assumed enum sizeof deCONZ::ZclFCProfileCommand is 1");
static_assert (sizeof deCONZ::ZclDataTypeId_t(quint8(1)) == 1, "Assumed enum sizeof ZclDataTypeId_t(quint8(1)) is 1");
static_assert (sizeof deCONZ::ZclAttributeId_t(quint16(1)) == 2, "Assumed enum sizeof ZclAttributeId_t(quint16(1)) is 2");
static_assert (sizeof deCONZ::ZclClusterId_t(quint16(1)) == 2, "Assumed enum sizeof ZclClusterId_t(quint16(1)) is 2");
static_assert (sizeof deCONZ::ManufacturerCode_t(quint16(1)) == 2, "Assumed enum sizeof ManufacturerCode_t(quint16(1)) is 2");
static_assert (sizeof deCONZ::ZclCommandId_t(quint8(1)) == 1, "Assumed enum sizeof ZclCommandId_t(quint8(1)) is 1");

void ZclDataBase::load(const QString &dbfile)
{
    QString name;
    QString descr;
    uint8_t u8id{};
    uint16_t u16id;
    QStack<ParseSection> curSection;
    curSection.push(InNone);

    Enumeration enumeration;
    ZclProfile profile;
    ZclDomain domain;
    ZclDevice device;
    ZclCluster cluster;
    ZclAttributeSet attrSet;
    ZclAttribute attr;
    ZclCommand command;
    QStringList attrValueNames;
    std::vector<int> attrValuePos;

    QFile file(dbfile);

    if (!file.open(QIODevice::ReadOnly)) {
        DBG_Printf(DBG_ZCLDB, "%s can't read %s\n", Q_FUNC_INFO, qPrintable(dbfile));
        return;
    }

    DBG_Printf(DBG_ZCLDB, "%s reading file %s\n", Q_FUNC_INFO, qPrintable(dbfile));

    QXmlStreamReader xml(&file);

    while (!xml.atEnd())
    {
            xml.readNext();
            const auto xmlName = xml.name();

            if (xml.error() != QXmlStreamReader::NoError)
            {
                DBG_Printf(DBG_ZCLDB, "ZCL XML error: %s, line: %d, column: %d\n", qPrintable(xml.errorString()), (int)xml.lineNumber(), (int)xml.columnNumber());
                break;
            }

            if (xml.isStartElement())
            {
                const auto xmlAttributes = xml.attributes();
                // check domain
                if (xmlName == QLatin1String("domain"))
                {
                    bool ok = true;

                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("name"))) ok = false;
                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("description"))) ok = false;

                    if (ok)
                    {
                        name = xmlAttributes.value(QLatin1String("name")).toString();
                        descr = xmlAttributes.value(QLatin1String("description")).toString();

                        // create or update a domain
                        domain = this->domain(name);

                        domain.setName(name);
                        domain.setDescription(descr);

                        // check for properitary clusters
                        if (xmlAttributes.hasAttribute(QLatin1String("useZcl")))
                        {
                            if (xmlAttributes.value(QLatin1String("useZcl")) == QLatin1String("false"))
                            {
                                domain.setUseZcl(false);
                            }
                            else
                            {
                                domain.setUseZcl(true);
                            }
                        }
                        curSection.push(InDomain);
                    }
                    else
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, invalid domain element\n", (int)xml.lineNumber());
                        xml.skipCurrentElement();
                    }

                    continue;
                }

                // check domain-ref
                if (curSection.top() == InProfile && xmlName == QLatin1String("domain-ref"))
                {
                    bool ok = true;

                    if (xmlAttributes.size() < 1) ok = false;
                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("name"))) ok = false;

                    if (ok)
                    {
                        name = xmlAttributes.value(QLatin1String("name")).toString().toLower();
                        ZclDomain dom = this->domain(name);

                        if (dom.isValid())
                        {
                            profile.addDomain(dom);
                        }
                        else
                        {
                            DBG_Printf(DBG_ZCLDB, "ZCL line: %d, domain-ref: %s for profile: %s not found\n", (int)xml.lineNumber(), qPrintable(name), qPrintable(profile.name()));
                        }
                    }
                    else
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, invalid domain-ref element\n", (int)xml.lineNumber());
                        xml.skipCurrentElement();
                    }

                    continue;
                }

                // check profile
                if (xmlName == QLatin1String("profile"))
                {
                    bool ok = true;
                    if (!xmlAttributes.hasAttribute(QLatin1String("id"))) ok = false;
                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("name"))) ok = false;
                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("description"))) ok = false;
                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("icon"))) ok = false;

                    if (ok)
                    {
                        u16id = xmlAttributes.value(QLatin1String("id")).toUShort(0, 16);
                        name = xmlAttributes.value(QLatin1String("name")).toString();
                        descr = xmlAttributes.value(QLatin1String("description")).toString();
                        QString icon = xmlAttributes.value(QLatin1String("icon")).toString();

                        curSection.push(InProfile);

                        // update or create a profile
                        profile = this->profile(u16id);

                        profile.setId(u16id);
                        profile.setName(name);
                        profile.setDescription(descr);
                        if (!icon.startsWith(QDir::separator()))
                        {
                            icon.prepend(m_iconPath);
                        }

                        if (QFile::exists(icon))
                        {
                            profile.setIcon(QIcon(icon));
                        }
                    }
                    else
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, invalid profile element\n", (int)xml.lineNumber());
                        xml.skipCurrentElement();
                    }

                    continue;
                }

                // check enumeration
                if (xmlName == QLatin1String("enumeration"))
                {
                    bool ok = true;
                    if (!xmlAttributes.hasAttribute(QLatin1String("id"))) ok = false;
                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("name"))) ok = false;

                    if (ok)
                    {
                        u16id = xmlAttributes.value(QLatin1String("id")).toUShort(&ok, 16);
                    }

                    if (ok)
                    {
                        name = xmlAttributes.value(QLatin1String("name")).toString();
                        enumeration = Enumeration(u16id, name);
                        curSection.push(InEnumeration);
                    }
                    else
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, invalid enumeration element\n", (int)xml.lineNumber());
                        xml.skipCurrentElement();
                    }

                    continue;
                }

                // check device
                if (xmlName == QLatin1String("device"))
                {
                    bool ok = true;
                    if (!xmlAttributes.hasAttribute(QLatin1String("id"))) ok = false;
                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("name"))) ok = false;
//                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("description"))) ok = false;
//                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("icon"))) ok = false;

                    if (ok)
                    {
                        u16id = xmlAttributes.value(QLatin1String("id")).toUShort(0, 16);
                        name = xmlAttributes.value(QLatin1String("name")).toString();
//                        descr = xmlAttributes.value(QLatin1String("description")).toString();
                        QString icon("dev-unknown.svg");
                        if (xmlAttributes.hasAttribute(QLatin1String("icon")))
                        {
                            QString iconDev = xmlAttributes.value(QLatin1String("icon")).toString();
                            if (!iconDev.isEmpty())
                                icon = iconDev;
                        }

                        descr.clear();
                        if (xmlAttributes.hasAttribute(QLatin1String("description")))
                        {
                            descr = xmlAttributes.value(QLatin1String("description")).toString();
                        }

                        curSection.push(InDevice);
                        if (!icon.startsWith(QDir::separator()))
                        {
                            icon.prepend(m_iconPath);
                        }
                        QIcon devIcon;
                        if (QFile::exists(icon))
                        {
                            devIcon = QIcon(icon);
                        }
                        device = ZclDevice(u16id, name, descr, devIcon);
                    }
                    else
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, invalid device element\n", (int)xml.lineNumber());
                        xml.skipCurrentElement();
                    }
                }

                // check cluster
                else if (curSection.top() == InDomain && xmlName == QLatin1String("cluster"))
                {
                    bool ok = true;

                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("id"))) ok = false;
                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("name"))) ok = false;

                    if (ok)
                    {
                        u16id = xmlAttributes.value(QLatin1String("id")).toUShort(0, 16);
                        name = xmlAttributes.value(QLatin1String("name")).toString();
                        cluster = ZclCluster(u16id, name);
                        cluster.setIsZcl(domain.useZcl());
                        // check if opposite cluster id differs
                        if (xmlAttributes.hasAttribute(QLatin1String("oppositeId")))
                        {
                            u16id = xmlAttributes.value(QLatin1String("oppositeId")).toUShort(0, 16);
                            cluster.setOppositeId(u16id);
                        }
                        if (xmlAttributes.hasAttribute(QLatin1String("mfcode")))
                        {
                            quint16 mfcode = xmlAttributes.value(QLatin1String("mfcode")).toUShort(0, 16);
                            cluster.setManufacturerCode(mfcode);
                        }
                        curSection.push(InCluster);
                    }
                    else
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, invalid cluster element\n", (int)xml.lineNumber());
                        xml.skipCurrentElement();
                    }

                    continue;
                }

                // check command
                else if ((curSection.top() == InClusterServer || curSection.top() == InClusterClient) && xmlName == QLatin1String("command"))
                {
                    bool ok = true;

                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("id"))) ok = false;
                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("name"))) ok = false;
                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("dir"))) ok = false;
                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("required"))) ok = false;

                    if (ok)
                    {
                        bool recv = false;
                        bool required = false;

                        u8id = (uint8_t)xmlAttributes.value(QLatin1String("id")).toUShort(0, 16);
                        name = xmlAttributes.value(QLatin1String("name")).toString();

                        if (xmlAttributes.value(QLatin1String("dir")) == QLatin1String("recv"))
                        {
                            recv = true;
                        }

                        if (xmlAttributes.value(QLatin1String("required")) == QLatin1String("m"))
                        {
                            required = true;
                        }

                        command = ZclCommand(u8id, name, required, recv);

                        if (xmlAttributes.hasAttribute(QLatin1String("response")))
                        {

                            u8id = (uint8_t)xmlAttributes.value(QLatin1String("response")).toUShort(&ok, 16);
                            if (ok)
                            {
                                command.setResponseId(u8id);
                                command.setDisableDefaultResponse(true);
                            }
                            else
                            {
                                DBG_Printf(DBG_ZCLDB, "ZCL line: %d, invalid command response id\n", (int)xml.lineNumber());
                            }
                        }

                        if (xmlAttributes.hasAttribute(QLatin1String("vendor")))
                        {

                            uint16_t vendor = xmlAttributes.value(QLatin1String("vendor")).toUShort(&ok, 16);
                            if (ok)
                            {
                                command.setManufacturerId(vendor);
                            }
                            else
                            {
                                DBG_Printf(DBG_ZCLDB, "ZCL line: %d, invalid command vendor id\n", (int)xml.lineNumber());
                            }
                        }

                        curSection.push(InCommand);

//                        // if we are in a attribute set
//                        if (attrSet.id() != 0xFFFF)
//                        {
//                            // store the index in clusters attribute list
//                            attrSet.addAttribute(cluster.attributes().size());
//                        }

//                        // start new name lists
//                        attrValueNames.clear();
//                        attrValuePos.clear();
                    }
                    else
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, invalid command element\n", (int)xml.lineNumber());
                        xml.skipCurrentElement();
                    }
                }

                // check command payload
                else if (curSection.top() == InCommand)
                {
                    if (xmlName == QLatin1String("payload"))
                    {
                        curSection.push(InCommandPayload);
                    }
                }

                // check attribute set
                else if ((curSection.top() == InClusterServer || curSection.top() == InClusterClient) && xmlName == QLatin1String("attribute-set"))
                {
                    bool ok = true;

                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("id"))) ok = false;
                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("description")))ok = false;

                    if (ok)
                    {
                        const auto textId = xmlAttributes.value(QLatin1String("id"));
                        Q_ASSERT_X(textId.size() == 6, "parse ZCL attribute id", "invalid id, expected hex uint16");

                        u16id = textId.toUShort(&ok, 16);
                        Q_ASSERT_X(ok, "parse ZCL attribute id", "invalid id, expected hex uint16");
                        descr = xmlAttributes.value(QLatin1String("description")).toString();
                        attrSet = ZclAttributeSet(u16id, descr);

                        if (xmlAttributes.hasAttribute(QLatin1String("mfcode")))
                        {
                            quint16 mfcode = xmlAttributes.value(QLatin1String("mfcode")).toUShort(&ok, 16);
                            if (ok && mfcode > 0)
                            {
                                attrSet.setManufacturerCode(mfcode);
                            }
                        }

                        curSection.push(InAttributeSet);
                    }
                    else
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, invalid attribute-set element\n", (int)xml.lineNumber());
                        xml.skipCurrentElement();
                    }
                }

                // check attribute
                else if ((curSection.top() == InClusterServer) ||
                         (curSection.top() == InClusterClient) ||
                         (curSection.top() == InAttributeSet)  ||
                         (curSection.top() == InCommandPayload))
                {
                    if (xmlName == QLatin1String("attribute"))
                    {
                        bool ok = true;

                        if (ok && !xmlAttributes.hasAttribute(QLatin1String("id"))) ok = false;
                        if (ok && !xmlAttributes.hasAttribute(QLatin1String("name"))) ok = false;
                        if (ok && !xmlAttributes.hasAttribute(QLatin1String("type"))) ok = false;
                        if ((curSection.top() != InCommandPayload) && // access always rw
                            ok && !xmlAttributes.hasAttribute(QLatin1String("access"))) ok = false;
                        if (ok && !xmlAttributes.hasAttribute(QLatin1String("required"))) ok = false;

                        if (ok)
                        {
                            attr = {};
                            uint16_t type = 0;

                            u16id = xmlAttributes.value(QLatin1String("id")).toUShort(0, 16);
                            name = xmlAttributes.value(QLatin1String("name")).toString();
                            const auto typeStr = xmlAttributes.value(QLatin1String("type"));
                            if (!typeStr.isEmpty())
                            {
                                if (typeStr.at(0).isDigit())
                                {
                                    type = typeStr.toUShort(&ok, 16);
                                    Q_ASSERT(type <= UINT8_MAX);
                                }
                                else
                                {
                                    for (const auto &t : m_dataTypes)
                                    {
                                        if (t.shortname() == typeStr)
                                        {
                                            type = t.id();
                                            break;
                                        }
                                    }
                                }
                            }

                            if (type == 0)
                            {
                                DBG_Printf(DBG_ZCLDB, "ZCL line: %d, unknown data type\n", (int)xml.lineNumber());
                            }

                            ZclAccess access = ZclRead;
                            if (curSection.top() == InCommandPayload || (xmlAttributes.value(QLatin1String("access")) == QLatin1String("rw")))
                            {
                                access = ZclReadWrite;
                            }
                            else if (xmlAttributes.value(QLatin1String("access")) == QLatin1String("w"))
                            {
                                access = ZclWrite;
                            }

                            bool required = false;
                            if (xmlAttributes.value(QLatin1String("required")) == QLatin1String("m"))
                            {
                                required = true;
                            }

                            attr = ZclAttribute(ZclAttributeId_t(u16id), ZclDataTypeId_t(quint8(type)), name, access, required);

                            if (xmlAttributes.hasAttribute(QLatin1String("description")))
                            {
                                attr.setDescription(xmlAttributes.value(QLatin1String("description")).toString());
                            }

                            if (xmlAttributes.hasAttribute(QLatin1String("mfcode")))
                            {
                                quint16 mfcode = xmlAttributes.value(QLatin1String("mfcode")).toUShort(&ok, 16);
                                if (ok && mfcode > 0)
                                {
                                    attr.setManufacturerCode(mfcode);
                                }
                            }

                            if (xmlAttributes.hasAttribute(QLatin1String("enumeration")))
                            {
                                const auto enumName = xmlAttributes.value(QLatin1String("enumeration"));
                                ok = false;
                                for (const auto &e : m_enums)
                                {
                                    if (e.name() == enumName && e.id() <= UINT8_MAX)
                                    {
                                        attr.setEnumerationId(static_cast<uint8_t>(e.id()));
                                        ok = true;
                                        break;
                                    }
                                }

                                if (!ok)
                                {
                                    DBG_Printf(DBG_ZCLDB, "ZCL line: %d, enumeration not found\n", (int)xml.lineNumber());
                                }
                            }

                            if (xmlAttributes.hasAttribute(QLatin1String("default")))
                            {
                                const auto valRef = xmlAttributes.value(QLatin1String("default"));

                                if (type >= Zcl8BitUint && type <= Zcl64BitUint)
                                {
                                    quint64 val = valRef.toULongLong(&ok, 0);
                                    if (ok) { attr.setValue(val); }
                                }
                                else if (type >= Zcl8BitInt && type <= Zcl64BitInt)
                                {
                                    struct NumericTrait
                                    {
                                        uint8_t dataType;
                                        int64_t min;
                                        int64_t max;
                                        const char *invalid;
                                    };

                                    const std::array<NumericTrait, 8> numTraits = { {
                                        { Zcl8BitInt, 0-(0x7FLL), INT8_MAX, "0x80" },
                                        { Zcl16BitInt, INT16_MIN, INT16_MAX, "0x8000" },
                                        { Zcl24BitInt, 0-(0x7FFFFF), 0x7FFFFF, "0x800000" },
                                        { Zcl32BitInt, INT32_MIN, INT32_MAX, "0x80000000" },
                                        { Zcl40BitInt, 0LL-(0x7FFFFFFFFFLL), 0x7FFFFFFFFFLL, "0x8000000000" },
                                        { Zcl48BitInt, 0LL-(0x7FFFFFFFFFFFLL), 0x7FFFFFFFFFFF, "0x800000000000" },
                                        { Zcl56BitInt, 0LL-(0x7FFFFFFFFFFFFFLL), 0x7FFFFFFFFFFFFF, "0x80000000000000" },
                                        { Zcl64BitInt, 0LL-(0x7FFFFFFFFFFFFFFFLL), 0x7FFFFFFFFFFFFFFF, "0x8000000000000000" },
                                    } };

                                    auto nt = std::find_if(numTraits.cbegin(), numTraits.cend(), [type](const auto &t)
                                                           { return t.dataType == type; });

                                    if (nt != numTraits.cend())
                                    {
                                        qint64 val = valRef.toLongLong(&ok, 0);
                                        if (valRef == QLatin1String(nt->invalid))
                                        {

                                        }
                                        else if (ok && val >= nt->min && val <= nt->max)
                                        {
                                            attr.setValue(val);
                                        }
                                        else
                                        {
                                            DBG_Assert(ok);
                                            DBG_Assert(val >= nt->min);
                                            DBG_Assert(val <= nt->max);
                                        }
                                    }
                                    else
                                    {
                                        DBG_Printf(DBG_ZCLDB, "ZCL no type trait found for data type 0x%02X\n", uint8_t(type));
                                    }
                                }
                                else if (type == Zcl8BitEnum || type == Zcl16BitEnum)
                                {
                                    uint enumerator = valRef.toUInt(&ok, 0);
                                    if (ok)
                                    {
                                        attr.setEnumerator(enumerator);
                                    }
                                }
                            }

                            if (xmlAttributes.hasAttribute(QLatin1String("showas")))
                            {
                                const auto showas = xmlAttributes.value(QLatin1String("showas"));

                                if (showas == QLatin1String("hex"))
                                {
                                    attr.setNumericBase(16);
                                }
                                else if (showas == QLatin1String("bin"))
                                {
                                    attr.setNumericBase(2);
                                }
                                else if (showas == QLatin1String("dec"))
                                {
                                    attr.setNumericBase(10);
                                }
                                else if (showas == QLatin1String("slider"))
                                {
                                    attr.setFormatHint(ZclAttribute::SliderFormat);
                                }
                                else
                                {
                                    DBG_Printf(DBG_ZCLDB, "ZCL line: %d, unknown showas attribute\n", (int)xml.lineNumber());
                                }
                            }

                            if (xmlAttributes.hasAttribute(QLatin1String("range")))
                            {
                                const auto range = xmlAttributes.value(QLatin1String("range")).toString().split(',');

                                ok = false;
                                if (range.size() == 2)
                                {
                                    auto r0 = range.at(0);
                                    auto r1 = range.at(1);

                                    if (r0.size() && r1.size())
                                    {
                                        int rangeMin = r0.toInt(&ok, 0);
                                        int rangeMax = ok ? r1.toInt(&ok, 0) : 0;

                                        if (ok)
                                        {
                                            attr.setRangeMin(rangeMin);
                                            attr.setRangeMax(rangeMax);
                                        }
                                    }
                                }

                                if (!ok)
                                {
                                    switch (attr.dataType())
                                    {
                                    case Zcl8BitUint: attr.setRangeMin(0); attr.setRangeMax(UCHAR_MAX); break;
                                    case Zcl16BitUint: attr.setRangeMin(0); attr.setRangeMax(USHRT_MAX); break;
                                    case Zcl24BitUint: attr.setRangeMin(0); attr.setRangeMax(0xFFFFFF); break;
                                    // the following is due a restriction in QSlider min,max as 32bit signed int
                                    case Zcl32BitUint: attr.setRangeMin(0); attr.setRangeMax(INT_MAX); break;
                                    case Zcl40BitUint: attr.setRangeMin(0); attr.setRangeMax(INT_MAX); break;
                                    case Zcl48BitUint: attr.setRangeMin(0); attr.setRangeMax(INT_MAX); break;
                                    case Zcl56BitUint: attr.setRangeMin(0); attr.setRangeMax(INT_MAX); break;
                                    case Zcl64BitUint: attr.setRangeMin(0); attr.setRangeMax(INT_MAX); break;
                                    case Zcl8BitBitMap: break;
                                    case Zcl16BitBitMap: break;
                                    case Zcl24BitBitMap: break;
                                    case Zcl32BitBitMap: break;
                                    case Zcl40BitBitMap: break;
                                    case Zcl48BitBitMap: break;
                                    case Zcl56BitBitMap: break;
                                    case Zcl64BitBitMap: break;
                                    default:
                                    {
                                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, invalid range attribute\n", (int)xml.lineNumber());
                                    }
                                        break;
                                    }
                                }
                            }

                            if (xmlAttributes.hasAttribute(QLatin1String("listSize")))
                            {
                                const auto listSize = xmlAttributes.value(QLatin1String("listSize"));
                                uint16_t attrId = listSize.toUShort(&ok, 16);

                                if (ok)
                                {
                                    attr.setListSizeAttribute(attrId);
                                }
                            }


                            // if we are in a attribute set
                            //if (attrSet.id() != 0xFFFF)
                            if (curSection.top() == InAttributeSet)
                            {
                                // store the index in clusters attribute list
                                attrSet.addAttribute(int(cluster.attributes().size()));
                                attr.setAttributeSet(attrSet.id(), attrSet.manufacturerCode());
                            }

                            curSection.push(InAttribute);

                            // start new name lists
                            attrValueNames.clear();
                            attrValuePos.clear();
                        }
                        else
                        {
                            DBG_Printf(DBG_ZCLDB, "ZCL line: %d, invalid attribute element\n", (int)xml.lineNumber());
                            xml.skipCurrentElement();
                        }

                        continue;
                    }
                }

                // check attribute value
                else if (curSection.top() == InAttribute)
                {
                    if (xmlName == QLatin1String("value"))
                    {
                        bool ok = true;

                        if (ok && !xmlAttributes.hasAttribute(QLatin1String("name"))) ok = false;
                        if (ok && !xmlAttributes.hasAttribute(QLatin1String("value"))) ok = false;

                        if (ok)
                        {
                            const QString name = xmlAttributes.value(QLatin1String("name")).toString();
                            uint pos = xmlAttributes.value(QLatin1String("value")).toUInt(&ok, 0);

                            if (!ok)
                            {
                                DBG_Printf(DBG_ZCLDB, "ZCL line: %d, TODO attribute value element range\n", (int)xml.lineNumber());
                                pos = 0;
                            }

                            if (!name.isEmpty())
                            {
                                attrValueNames.append(name);
                                attrValuePos.push_back((int)pos);
                            }
                            else
                            {
                                DBG_Printf(DBG_ZCLDB, "ZCL line: %d, invalid attribute value element\n", (int)xml.lineNumber());
                                xml.skipCurrentElement();
                            }
                        }
                    }
                }

                // check enumeration value
                else if (curSection.top() == InEnumeration)
                {
                    if (xmlName == QLatin1String("value"))
                    {
                        bool ok = true;

                        if (ok && !xmlAttributes.hasAttribute(QLatin1String("name"))) ok = false;
                        if (ok && !xmlAttributes.hasAttribute(QLatin1String("value"))) ok = false;

                        if (ok)
                        {
                            QString name = xmlAttributes.value(QLatin1String("name")).toString();
                            QString value = xmlAttributes.value(QLatin1String("value")).toString();

                            uint pos = value.toUInt(&ok, 0);

                            if (ok && !name.isEmpty())
                            {
                                enumeration.setValue(pos, name);
                            }
                            else
                            {
                                DBG_Printf(DBG_ZCLDB, "ZCL line: %d, invalid enum value\n", (int)xml.lineNumber());
                                xml.skipCurrentElement();
                            }
                        }
                    }
                }

                if (curSection.top() == InCluster)
                {
                    if (xmlName == QLatin1String("server"))
                    {
                        cluster.attributes().clear();
                        cluster.attributeSets().clear();
                        cluster.commands().clear();
                        command = ZclCommand();
                        curSection.push(InClusterServer);
                        continue;
                    }
                    else if (xmlName == QLatin1String("client"))
                    {
                        cluster.attributes().clear();
                        cluster.attributeSets().clear();
                        cluster.commands().clear();
                        command = ZclCommand();
                        curSection.push(InClusterClient);
                        continue;
                    }
                }

                if (xmlName == QLatin1String("description"))
                {
                    switch (curSection.top())
                    {
                    case InCluster:
                        cluster.setDescription(xml.readElementText());
                        break;

                    case InAttribute:
                        attr.setDescription(xml.readElementText());
                        break;

                    case InCommand:
                        command.setDescription(xml.readElementText());
                        break;

                    case InDomain:
                        domain.setDescription(xml.readElementText());
                        break;

                    case InProfile:
                        profile.setDescription(xml.readElementText());
                        break;

                    case InDevice:
                        break;

                    default:
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, description for unknown section\n", (int)xml.lineNumber());
                        break;
                    }
                    continue;
                }

                if (xmlName == QLatin1String("datatype"))
                {
                    bool ok = true;

                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("id"))) ok = false;
                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("name"))) ok = false;
                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("shortname"))) ok = false;
                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("length"))) ok = false;
                    if (ok && !xmlAttributes.hasAttribute(QLatin1String("ad"))) ok = false;

                    if (ok)
                    {
                        bool ok;
                        int length;
                        char analogDiscrete = '-';

                        ushort id = xmlAttributes.value(QLatin1String("id")).toUShort(&ok, 16);
                        DBG_Assert(ok && id <= UINT8_MAX);
                        if (!ok || id > UINT8_MAX)
                        {
                            continue;
                        }

                        if (xmlAttributes.value(QLatin1String("ad")).size() == 1)
                        {
                            analogDiscrete = xmlAttributes.value(QLatin1String("ad")).at(0).toUpper().toLatin1();
                        }
                        else
                        {
                            DBG_Printf(DBG_ZCLDB, "ZCL line: %d, invalid analog/discrete attribute\n", (int)xml.lineNumber());
                        }

                        length = xmlAttributes.value(QLatin1String("length")).toInt(&ok, 10);

                        if (ok)
                        {
                            if (length > 0xFF) length = 0xFF;
                            if (length < 0) length = 0;
                        }
                        else
                        {
                            length = 0;
                        }

                        ZclDataType datatype(uint8_t(id), xmlAttributes.value(QLatin1String("name")).toString(), xmlAttributes.value(QLatin1String("shortname")).toString(), length, analogDiscrete);

                        // create or update
                        bool found = false;
                        for (ZclDataType &dt: m_dataTypes)
                        {
                            if (dt.id() == id)
                            {
                                dt = datatype;
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                        {
                            m_dataTypes.push_back(datatype);
                        }
                    }
                    else
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, invalid data type element\n", (int)xml.lineNumber());
                    }
                }
            }
            else if (xml.isEndElement())
            {
                // check domain
                if (xmlName == QLatin1String("domain"))
                {
                    if (curSection.top() != InDomain)
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, </domain> while not InDomain\n", (int)xml.lineNumber());
                    }
                    else
                    {
                        addDomain(domain);
                    }
                    curSection.pop();
                }
                // check profile
                else if (xmlName == QLatin1String("profile"))
                {
                    if (curSection.top() != InProfile)
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, </profile> while not InProfile\n", (int)xml.lineNumber());
                    }
                    else
                    {
                        addProfile(profile);
                    }
                    curSection.pop();
                }
                // check device
                else if (xmlName == QLatin1String("device"))
                {
                    if (curSection.top() != InDevice)
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, </device> while not InDevice\n", (int)xml.lineNumber());
                        curSection.pop();
                    }
                    else
                    {
                        curSection.pop();

                        // check if the device is known already
                        bool known = false;

                        for (auto &dev : m_devices)
                        {
                            if (dev.id() == device.id() && dev.name() == device.name())
                            {
                                dev = device; // update
                                known = true;
                                break;
                            }
                        }

                        if (!known)
                        {
                            // check private profiles
                            if (curSection.top() == InProfile)
                            {
                                device.setProfileId(profile.id());
                                m_devices.insert(m_devices.begin(), device);
                            }
                            else
                            {
                                m_devices.push_back(device);
                            }
                        }
                    }
                }
                // check enumeration
                else if (xmlName == QLatin1String("enumeration"))
                {
                    if (curSection.top() != InEnumeration)
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, </enumeration> while not InEnumeration \n", (int)xml.lineNumber());
                    }
                    else
                    {
                        m_enums.push_back(enumeration);
                    }
                    curSection.pop();
                }
                else if (xmlName == QLatin1String("cluster"))
                {
                    if (curSection.top() != InCluster)
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, </cluster> while not InCluster\n", (int)xml.lineNumber());
                    }
                    curSection.pop();
                }
                else if (xmlName == QLatin1String("server"))
                {
                    if (curSection.top() == InClusterServer)
                    {
                        cluster.setIsServer(true);
                        quint32 hash = 0;
                        if (cluster.id() >= 0xfc00) {
                            hash |= cluster.manufacturerCode();
                            hash <<= 16;
                        }
                        hash |= cluster.id();

                        domain.m_inClusters.insert(hash, cluster);
                        curSection.pop();

                        if (curSection.top() != InCluster)
                        {
                            DBG_Printf(DBG_ZCLDB, "ZCL line: %d, </server> while not InCluster\n", (int)xml.lineNumber());
                        }
                    }
                    else
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, unknown 'server' end element\n", (int)xml.lineNumber());
                    }
                }
                else if (xmlName == QLatin1String("client"))
                {
                    if (curSection.top() == InClusterClient)
                    {
                        cluster.setIsServer(false);
                        quint32 hash = 0;
                        if (cluster.id() >= 0xfc00) {
                            hash |= cluster.manufacturerCode();
                            hash <<= 16;
                        }
                        hash |= cluster.id();
                        domain.m_outClusters.insert(hash, cluster);
                        curSection.pop();
                        if (curSection.top() != InCluster)
                        {
                            DBG_Printf(DBG_ZCLDB, "ZCL line: %d, </client> while not InCluster\n", (int)xml.lineNumber());
                        }
                    }
                    else
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, unknown 'cluster' end element\n", (int)xml.lineNumber());
                    }
                }
                else if (xmlName == QLatin1String("attribute-set"))
                {
                    if (curSection.top() != InAttributeSet)
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, </attribute-set> while not InAttributeSet\n", (int)xml.lineNumber());
                    }
                    else
                    {
                        cluster.attributeSets().push_back(attrSet);
                    }
                    attrSet = ZclAttributeSet(); // reset to invalid
                    curSection.pop();
                }
                else if (xmlName == QLatin1String("attribute"))
                {
                    if (curSection.top() != InAttribute)
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, </attribute> while not InAttribute\n", (int)xml.lineNumber());
                        curSection.pop();
                    }
                    else
                    {
                        if (!attrValueNames.isEmpty())
                        {
                            attr.d_ptr->m_valueNames = attrValueNames;
                            attr.d_ptr->m_valuePos = attrValuePos;
                        }

                        curSection.pop();

                        if (curSection.top() == InCommandPayload)
                        {
                            command.parameters().push_back(std::move(attr));
                        }
                        else
                        {
                            cluster.attributes().push_back(std::move(attr));
                        }
                    }
                }
                else if (xmlName == QLatin1String("command"))
                {
                    if (curSection.top() != InCommand)
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, </command> while not InCommand\n", (int)xml.lineNumber());
                    }
                    else
                    {
//                        if (!attrValueNames.isEmpty())
//                        {
//                            attr.m_value.setValue(attrValueNames);
//                            attr.m_valuePos = attrValuePos;
//                        }
                        cluster.commands().push_back(command);
                    }
                    curSection.pop();
                }
                else if (xmlName == QLatin1String("payload"))
                {
                    if (curSection.top() != InCommandPayload)
                    {
                        DBG_Printf(DBG_ZCLDB, "ZCL line: %d, </payload> while not InCommandPayload\n", (int)xml.lineNumber());
                    }
                    else
                    {
//                        if (!attrValueNames.isEmpty())
//                        {
//                            attr.m_value.setValue(attrValueNames);
//                            attr.m_valuePos = attrValuePos;
//                        }
                    }
                    curSection.pop();
                }
            }
    }
}

void ZclDataBase::initDbFile(const QString &zclFile)
{
    int found = 0;
    QFile file(zclFile);

    DBG_Printf(DBG_INFO, "ZCLDB init file %s\n", qPrintable(zclFile));

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);

        while (!stream.atEnd())
        {
            const QString path = stream.readLine(1024).trimmed();
            if (path.endsWith(".xml"))
            {
                if (QFile::exists(path) && path.contains("general.xml"))
                    found++;
            }
        }

        file.close();
    }

    if (QFile::exists(zclFile) && found == 0)
    {
        QFile::remove(zclFile); // could happen when installation moves
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text) || (file.size() <= 0))
    {
        if (file.isOpen())
        {
            file.close();
        }

        // create the file
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&file);
#ifdef Q_OS_LINUX

            char path[128];
            ssize_t sz = readlink("/proc/self/exe", path, sizeof(path) - 1);

            if ((sz > 0) && ((size_t)sz < sizeof(path)))
            {
                path[sz] = '\0';
                // go 2 dirs up
                int nslahes = 2; // /usr/bin/deCONZ --> /usr aka ../..
                while (sz > 0)
                {
                    if (path[sz] == '/')
                    {
                        path[sz] = '\0';
                        nslahes--;
                        if (nslahes == 0)
                        {
                            break;
                        }
                    }
                    sz--;
                }

                // append default.xml if not present
                QString gen = QString("%1/share/deCONZ/zcl/general.xml").arg(path);
                if (QFile::exists(gen))
                {
                    stream << gen;
                    stream << "\r\n";
                }
                else
                {
                    DBG_Printf(DBG_INFO, "ZCLDB File %s not found\n", qPrintable(gen));
                }
            }
#endif // Linux
#ifdef Q_OS_WIN
            QDir dir(qApp->applicationDirPath());
            dir.cdUp();
            dir.cd("zcl");

            QString gen = QString("%1/general.xml").arg(dir.path());
            if (QFile::exists(gen))
            {
                DBG_Printf(DBG_INFO, "ZCLDB add file %s \n", qPrintable(gen));
                stream << gen;
                stream << "\n";
            }
#endif // OSX
#ifdef __APPLE__
            QDir dir(qApp->applicationDirPath());
            dir.cdUp();

            QString gen = dir.absolutePath() + "/Resources/zcl/general.xml";

            if (QFile::exists(gen))
            {
                DBG_Printf(DBG_INFO, "ZCLDB add file %s \n", qPrintable(gen));
                stream << gen;
                stream << "\n";
            }
#endif // __APPLE__
        }
        else
        {
            DBG_Printf(DBG_ERROR, "ZCLDB failed to create %s: %s\n", qPrintable(zclFile), qPrintable(file.errorString()));
        }
    }
}

void ZclDataBase::reloadAll(const QString &zclFile)
{
    QFile file(zclFile);

    clear();

#ifdef Q_OS_UNIX
    QString generalXml("/usr/share/deCONZ/zcl/general.xml");
    if (QFile::exists(generalXml))
    {
        zclDataBase()->load(generalXml);
    }
#else
    QString generalXml("");
#endif // Q_OS_UNIX

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);

        while (!stream.atEnd())
        {
            QString line = stream.readLine(1024).trimmed();
            if (line.endsWith(".xml") && line != generalXml)
            {
                zclDataBase()->load(line);
            }
        }
    }
    else
    {
        DBG_Printf(DBG_ERROR, "ZCLDB failed to open %s:%s\n", qPrintable(zclFile), qPrintable(file.errorString()));
    }
}

void ZclDataBase::clear()
{
    m_enums.clear();
    m_dataTypes.clear();
    m_domains.clear();
    m_profiles.clear();
    m_devices.clear();
}

ZclCluster ZclDataBase::inCluster(uint16_t profileId, uint16_t clusterId, quint16 mfcode)
{
    if (m_profiles.contains(profileId))
    {
        const ManufacturerCode_t _mfcode(mfcode);
        const ZclProfile &profile = m_profiles[profileId];

        for (auto i = profile.domains().cbegin(); i != profile.domains().cend(); ++i)
        {
            quint32 hash = 0;
            if (clusterId >= 0xfc00) {
                hash |= static_cast<quint16>(_mfcode);
                hash <<= 16;
            }
            hash |= clusterId;

            if (i->inClusters().contains(hash))
            {
                auto cl = i->inClusters().value(hash);

                std::vector<ZclAttribute> attributes;

                std::copy_if(cl.attributes().begin(), cl.attributes().end(), std::back_inserter(attributes), [_mfcode](const ZclAttribute &a) {
                    return a.manufacturerCode_t() == 0x0000_mfcode
                           || a.manufacturerCode_t() == _mfcode
                           || (a.manufacturerCode_t() == 0x115f_mfcode && _mfcode == 0x1037_mfcode) /* Xiaomi used both on the same device */;
                });

                if (cl.attributes().size() != attributes.size()) // filtered
                {
                    cl.attributes() = std::move(attributes);
                }

                std::vector<ZclCommand> commands;

                std::copy_if(cl.commands().begin(), cl.commands().end(), std::back_inserter(commands), [mfcode](const ZclCommand &a) {
                    return a.manufacturerId() == 0 || a.manufacturerId() == mfcode;
                });

                if (cl.commands().size() != commands.size()) // filtered
                {
                    cl.commands() = std::move(commands);
                }

                return cl;
            }
        }
    }

    return ZclCluster(clusterId, QLatin1String("Unknown"));
}

ZclCluster ZCL_InCluster(uint16_t profileId, uint16_t clusterId, uint16_t mfcode)
{
    if (_zclDB)
    {
        return _zclDB->inCluster(profileId, clusterId, mfcode);
    }

    return {};
}

ZclCluster ZCL_OutCluster(uint16_t profileId, uint16_t clusterId, uint16_t mfcode)
{
    if (_zclDB)
    {
        return _zclDB->outCluster(profileId, clusterId, mfcode);
    }

    return {};
}

ZclDataType ZCL_DataType(uint8_t id)
{
    if (_zclDB)
    {
        return _zclDB->dataType(id);
    }

    return {};
}

ZclDataType ZCL_DataType(const QString &name)
{
    if (_zclDB)
    {
        return _zclDB->dataType(name);
    }

    return {};
}

ZclCluster ZclDataBase::outCluster(uint16_t profileId, uint16_t clusterId, quint16 mfcode)
{
    if (m_profiles.contains(profileId))
    {
        const ZclProfile &profile = m_profiles[profileId];

        for (auto i = profile.domains().cbegin(); i != profile.domains().cend(); ++i)
        {
            quint32 hash = 0;
            if (clusterId >= 0xfc00) {
                hash |= mfcode;
                hash <<= 16;
            }
            hash |= clusterId;

            if (i->outClusters().contains(hash))
            {
                auto cl = i->outClusters().value(hash);

                std::vector<ZclAttribute> attributes;

                std::copy_if(cl.attributes().begin(), cl.attributes().end(), std::back_inserter(attributes), [mfcode](const ZclAttribute &a) {
                    return a.manufacturerCode() == 0 || a.manufacturerCode() == mfcode;
                });

                if (cl.attributes().size() != attributes.size()) // filtered
                {
                    cl.attributes() = std::move(attributes);
                }

                return cl;
            }
        }
    }

    return ZclCluster(clusterId, QLatin1String("Unknown"));
}

const ZclDataType &ZclDataBase::dataType(uint8_t id) const
{
    for (const ZclDataType &dt : m_dataTypes)
    {
        if (dt.id() == id)
        {
            return dt;
        }
    }

    return m_unknownDataType;
}

const ZclDataType &ZclDataBase::dataType(const QString &shortName) const
{
    for (const ZclDataType &dt : m_dataTypes)
    {
        if (dt.shortname() == shortName)
        {
            return dt;
        }
    }

    return m_unknownDataType;
}

ZclProfile ZclDataBase::profile(uint16_t id)
{
    const auto &profiles = m_profiles;
    for (const ZclProfile &profile : profiles)
    {
        if (profile.id() == id)
        {
            return profile;
        }
    }

    ZclProfile pro;
    pro.m_id = id;
    pro.m_name = QString("%1").arg(id, (int)4, (int)16, QChar('0'));

    return pro;
}

ZclDomain ZclDataBase::domain(const QString &name)
{
    for (auto i = m_domains.begin(); i != m_domains.end(); ++i)
    {
        if (i->name().toLower() == name.toLower())
        {
            return *i;
        }
    }

    return ZclDomain();
}

/*!
    Add or update a domain.
 */
void ZclDataBase::addDomain(const ZclDomain &domain)
{
    for (auto i = m_domains.begin(); i != m_domains.end(); ++i)
    {
        if (i->name().toLower() == domain.name().toLower())
        {
            *i = domain;
            return;
        }
    }

    m_domains.push_back(domain);
}

/*!
    Add or update a profile.
 */
void ZclDataBase::addProfile(const ZclProfile &profile)
{
    if (m_profiles.contains(profile.id()))
    {
        m_profiles[profile.id()] = profile;
        return;
    }

    m_profiles.insert(profile.id(), profile);
}

ZclDevice ZclDataBase::device(uint16_t profileId, uint16_t deviceId)
{
    // search first for most specific device
    auto i = m_devices.cbegin();
    auto end = m_devices.cend();

    for (; i != end; ++i)
    {
        if ((i->id() == deviceId) &&
            (i->profileId() == profileId))
        {
            return *i;
        }
    }


    // search for generic devices
    i = m_devices.cbegin();
    end = m_devices.cend();

    for (; i != end; ++i)
    {
        if ((i->id() == deviceId) &&
            (i->profileId() == 0xFFFF))
        {
            ZclDevice dev = *i;
            dev.setProfileId(profileId);
            return dev;
        }
    }

    ZclDevice dev(deviceId, QString("%1").arg(deviceId, (int)4, (int)16, QChar('0')), QString(), QIcon());
    dev.setProfileId(profileId);

    return dev;
}

/*!
    Known data type are supported in the application
    by at least parse them or and at best interpret the
    value data.
 */
bool ZclDataBase::knownDataType(uint8_t id)
{
    switch (id) {
    case Zcl8BitData:
    case Zcl16BitData:
    case Zcl24BitData:
    case Zcl32BitData:
    case Zcl40BitData:
    case Zcl48BitData:
    case Zcl56BitData:
    case Zcl64BitData:
    case ZclBoolean:
    case Zcl8BitBitMap:
    case Zcl16BitBitMap:
    case Zcl24BitBitMap:
    case Zcl32BitBitMap:
    case Zcl40BitBitMap:
    case Zcl48BitBitMap:
    case Zcl56BitBitMap:
    case Zcl64BitBitMap:
    case Zcl8BitUint:
    case Zcl16BitUint:
    case Zcl24BitUint:
    case Zcl32BitUint:
    case Zcl40BitUint:
    case Zcl48BitUint:
    case Zcl56BitUint:
    case Zcl64BitUint:
    case Zcl8BitInt:
    case Zcl16BitInt:
    case Zcl24BitInt:
    case Zcl32BitInt:
    case Zcl40BitInt:
    case Zcl48BitInt:
    case Zcl56BitInt:
    case Zcl64BitInt:
    case Zcl8BitEnum:
    case Zcl16BitEnum:
    case ZclSingleFloat:
    case ZclCharacterString:
    case ZclOctedString:
    case ZclUtcTime:
    case ZclAttributeId:
    case ZclClusterId:
    case ZclIeeeAddress:
    case Zcl128BitSecurityKey:
    case ZclArray:
        return true;

    default:
        break;
    }

    return false;
}

void ZclProfile::addDomain(const ZclDomain &domain)
{
    for (auto i = m_domains.begin(); i != m_domains.end(); ++i)
    {
        if (i->name() == domain.name())
        {

            DBG_Printf(DBG_ZCLDB, "ZCL: domain in profile already known - update\n");
            *i = domain;
            return;
        }
    }

    m_domains.push_back(domain);
}

ZclDataBase *zclDataBase()
{
    if (!_zclDB)
    {
        _zclDB = new ZclDataBase;
    }

    return _zclDB;
}

} // namespace deCONZ
