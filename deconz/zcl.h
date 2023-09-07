#ifndef DECONZ_ZCL_H
#define DECONZ_ZCL_H

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <vector>
#include <QDataStream>
#include <QVariant>
#include <QStringList>
#include <deconz/types.h>
#include "deconz/declspec.h"

//#define ZCL_LOAD_DBG
//#define ZCL_LOAD_DBG_COMMAND

#define ZCL_ENUM 0x00

/*!
    \defgroup zcl ZCL
    \section zcl_intro ZigBee Cluster Library (ZCL)
    \brief Services to handle ZCL data transported via \ref aps_intro.

    Classes in this module help to deal with ZCL payloads, attributes and data types.
*/

/*!
    \defgroup cpp_literals C++11 Literals
    \brief C++11 literals to create strong typed Zigbee basic types like cluster and attribute identifiers.
    \see strong_types
*/

/*!
    \defgroup strong_types Zigbee Strong Types
    \brief Strong typed Zigbee types like cluster and attribute identifiers.
    \see cpp_literals

    These types should be used instead plain C++ types like \c quint8, \c quint16, \c quint32, etc.
    They prevent a common class of errors and provide safety at compile time.

    ### Problems with using basic C++ types for function arguments

    Consider the following function:
    \code {cpp}

    void example1(quint16 clusterId, quint16 attrId, quint8 dataType, quint16 mfcode)
    {
    }

    \endcode

    The function can be called in various ways without the compiler complaining if something is wrong.
    \code {cpp}

    example1(0x0006, 0x0000, 0x10, 0x0000); // ok, but can you tell which parameter is what?

    example1(0x0000, 0x0006, 0x0000, 0x01); // compiles fine, but now the parameter order is wrong


    \endcode

    #### Better solution with strong types

    Here is the same function but using strong types arguments.

    \code {cpp}

    void example1(deCONZ::ZclClusterId_t clusterId, deCONZ::ZclAttributeId_t attrId, deCONZ::ZclDataTypeId_t dataType, deCONZ::ManufacturerCode_t mfcode)
    {
    }

    example1(0x0006, 0x0000, 0x10, 0x0000); // compile time error, this style doesn't work anymore

    example1(0x0006_clid, 0x0000_atid, 0x10_dtid, 0x0000_mfcode); // ok, using C++11 literals

    example1(0x0000_atid, 0x0006_clid, 0x10_dtid, 0x0000_mfcode); // compile time error, wrong order of arguments 1 and 2

    \endcode

    Using strong typed values improves the readability and guards against common bugs at compile time.

    ### Performance

    Albeit the declaration of these classes looks rather complex, the compiled code is equal to the weakly typed version.
    Therefore there is zero runtime overhead by using these types.
*/

namespace deCONZ
{
class ApsDataIndication;
}

namespace deCONZ
{
    /*! \brief Various data types as defined in ZigBee ZCL specification.
        \ingroup zcl

        To handle data type in payloads use deCONZ::ZclDataTypeId_t
    */
    enum ZclDataTypeId : unsigned char
    {
        ZclNoData    = 0x00,
        Zcl8BitData  = 0x08,
        Zcl16BitData = 0x09,
        Zcl24BitData = 0x0a,
        Zcl32BitData = 0x0b,
        Zcl40BitData = 0x0c,
        Zcl48BitData = 0x0d,
        Zcl56BitData = 0x0e,
        Zcl64BitData = 0x0f,
        ZclBoolean   = 0x10,
        Zcl8BitBitMap   = 0x18,
        Zcl16BitBitMap  = 0x19,
        Zcl24BitBitMap  = 0x1a,
        Zcl32BitBitMap  = 0x1b,
        Zcl40BitBitMap  = 0x1c,
        Zcl48BitBitMap  = 0x1d,
        Zcl56BitBitMap  = 0x1e,
        Zcl64BitBitMap  = 0x1f,
        Zcl8BitUint  = 0x20,
        Zcl16BitUint = 0x21,
        Zcl24BitUint = 0x22,
        Zcl32BitUint = 0x23,
        Zcl40BitUint = 0x24,
        Zcl48BitUint = 0x25,
        Zcl56BitUint = 0x26,
        Zcl64BitUint = 0x27,
        Zcl8BitInt  = 0x28,
        Zcl16BitInt = 0x29,
        Zcl24BitInt = 0x2a,
        Zcl32BitInt = 0x2b,
        Zcl40BitInt = 0x2c,
        Zcl48BitInt = 0x2d,
        Zcl56BitInt = 0x2e,
        Zcl64BitInt = 0x2f,
        Zcl8BitEnum  = 0x30,
        Zcl16BitEnum = 0x31,
        ZclSemiFloat = 0x38,
        ZclSingleFloat = 0x39,
        ZclDoubleFloat = 0x3a,
        ZclOctedString = 0x41,
        ZclCharacterString     = 0x42,
        ZclLongOctedString     = 0x43,
        ZclLongCharacterString = 0x44,
        ZclArray = 0x48,
        ZclStruct = 0x4c,
        ZclTimeOfDay  = 0xe0,
        ZclDate       = 0xe1,
        ZclUtcTime    = 0xe2,
        ZclClusterId         = 0xe8,
        ZclAttributeId       = 0xe9,
        ZclBACNetOId         = 0xea,
        ZclIeeeAddress       = 0xf0,
        Zcl128BitSecurityKey = 0xf1
    };

    /*! General ZCL command ids every cluster shall support. */
    enum ZclGeneralCommandId : unsigned char
    {
        ZclReadAttributesId               = 0x00,
        ZclReadAttributesResponseId       = 0x01,
        ZclWriteAttributesId              = 0x02,
        ZclWriteAttributesUndividedId     = 0x03,
        ZclWriteAttributesResponseId      = 0x04,
        ZclWriteAttributesNoResponseId    = 0x05,
        ZclConfigureReportingId           = 0x06,
        ZclConfigureReportingResponseId   = 0x07,
        ZclReadReportingConfigId          = 0x08,
        ZclReadReportingConfigResponseId  = 0x09,
        ZclReportAttributesId             = 0x0a,
        ZclDefaultResponseId              = 0x0b,
        ZclDiscoverAttributesId           = 0x0c,
        ZclDiscoverAttributesResponseId   = 0x0d,
        ZclReadAttributesStructuredId     = 0x0e,
        ZclWriteAttributesStructuredId    = 0x0f,
        ZclWriteAttributesStructuredResponseId  = 0x10
    };

    /*!
        General ZCL Frame Format

        Bits
        ------ Header ----------------
        8         Frame control
        0/16      Manufacturer code
        8         Transaction sequence
        8         Command Id
        ------ ZCL payload -------------
        Variable  Frame payload
        ------------------------------
     */

    /*! Frame control flags used in ZCL frame header. */
    enum ZclFrameControl : unsigned char
    {
        ZclFCProfileCommand          = 0x00,
        ZclFCClusterCommand          = 0x01,
        ZclFCManufacturerSpecific    = 0x04,
        ZclFCDirectionServerToClient = 0x08,
        ZclFCDirectionClientToServer = 0x00,
        ZclFCEnableDefaultResponse   = 0x00,
        ZclFCDisableDefaultResponse  = 0x10
    };

    /*! \ingroup strong_types
        \class ZclDataTypeId_t
        \brief A strong typed ZCL data type identifier.
        \see strong_types
        \see deCONZ::ZclDataTypeId
        \since v2.6.1

        This wrapper around quint8 data type as ZCL data type identifier for code correctness.
        It helps to prevent common hard to catch programming errors. The class is very strict
        and enforces a type safe programming style.
     */
    class DECONZ_DLLSPEC ZclDataTypeId_t
    {
        ZclDataTypeId m_id = ZclNoData;

    public:
        /*! Default constructor with value = \c ZclNoData. */
        constexpr ZclDataTypeId_t() {}
        /*! Explicit constructor to init by value. */
        constexpr explicit ZclDataTypeId_t(quint8 id) : m_id(static_cast<ZclDataTypeId>(id)) { }
        /*! Constructor to init by enum value (preferred). */
        constexpr ZclDataTypeId_t(ZclDataTypeId id) : m_id(id) { }

        /*! Don't allow construction via implicit integer conversion. */
        template <class T>
        ZclDataTypeId_t(T) = delete;

        /*! Returns true for known data types. */
        constexpr bool isValid() const { return m_id > ZclNoData && m_id <= Zcl128BitSecurityKey; }

        /*! Explicit \c quint8 conversion operator, requires `static_cast<quint8>()`. */
        explicit operator quint8() const { return m_id; }
        /*! Explicit \c ZclDataTypeId conversion operator, requires `static_cast<ZclDataTypeId>()`. */
        explicit operator ZclDataTypeId() const { return m_id; }

        /*! Equal operator, requires strong type. */
        constexpr bool operator==(ZclDataTypeId_t other) const { return other.m_id == m_id; }
        /*! Not equal operator, requires strong type. */
        constexpr bool operator!=(ZclDataTypeId_t other) const { return other.m_id != m_id; }
        /*! Equal operator, requires ZclDataTypeId enum. */
        constexpr bool operator==(ZclDataTypeId other) const { return other == m_id; }
        /*! Not equal operator, requires ZclDataTypeId enum. */
        constexpr bool operator!=(ZclDataTypeId other) const { return other != m_id; }
    };

    /*! Writes ZclDataTypeId_t as quint8 to the QDataStream. */
    DECONZ_DLLSPEC QDataStream & operator<<(QDataStream &ds, ZclDataTypeId_t id);
    /*! Reads ZclDataTypeId_t as quint8 from the QDataStream. */
    DECONZ_DLLSPEC QDataStream & operator>>(QDataStream &ds, ZclDataTypeId_t &id);

/*! \ingroup cpp_literals
    \brief Literal to create ZclDataTypeId_t.
    \since v2.6.1

Shorthand for creating type safe ZCL data type identifiers from numbers but without casting.

#### Example usage for streams

\code {cpp}

    QDataStream stream;

    stream << 0x01_dtid;

    // same as

    stream << ZclDataTypeId_t(quint8(0x01));

\endcode

#### Example usage in ZclDataTypeId_t constructors and assignment

\code {cpp}

    ZclDataTypeId_t dt0(0x01_dtid);  // constructor
    ZclDataTypeId_t dt1 = 0x02_dtid; // assignment

    if (dt0 == dt1) { }

    if (dt0 == 0x03_dtid) {}

\endcode
 */
    inline namespace literals {
        constexpr ZclDataTypeId_t operator ""_dtid(unsigned long long id)
        {
            return ZclDataTypeId_t(static_cast<quint8>(id));
        }
    }

    class ZclDataTypePrivate;

    /*!
        \ingroup zcl
        \class ZclDataType
        \brief Represents the data type of a ZigBee cluster attribute.
     */
    class DECONZ_DLLSPEC ZclDataType
    {
    public:
        /*! Default constructor with value = ZclNoData. */
        ZclDataType();
        /*! Copy constructor. */
        ZclDataType(const ZclDataType &other);
        /*! Copy assignment constructor */
        ZclDataType &operator=(ZclDataType &other);
        /*! Constructor used by ZCLDB parser. */
        ZclDataType(uint8_t id, const QString &name, const QString &shortname, int length, char analogDiscrete);
        /*! Destructor. */
        ~ZclDataType();
        /*! Returns the data type identifier. */
        uint8_t id() const;
        /*! Returns the strong typed data type identifier.
            \since v2.6.1
         */
        ZclDataTypeId_t id_t() const;
        /*! Returns the data type name. */
        const QString &name() const;
        /*! Returns the data type short name (bmp8, uint8, enum16, ...). */
        const QString &shortname() const;
        /*! Returns the length of this data type in bytes (uint8 -> 1, uint32 = 4, ...). */
        int length() const;
        /*! Returns true if this data type object has valid data. */
        bool isValid() const;
        /*! Returns true if this data type represents analog data. */
        bool isAnalog() const;
        /*! Returns true if this data type represents discrete data. */
        bool isDiscrete() const;

    private:
        ZclDataTypePrivate *d_ptr;
        Q_DECLARE_PRIVATE(ZclDataType)
    };

/*! \ingroup strong_types
    \class ZclCommandId_t
    \brief A strong typed ZCL command identifier.
    \see strong_types
    \since v2.6.1

    This wrapper around quint8 data type as ZCL command identifier for code correctness.
    It helps to prevent common hard to catch programming errors. The class is very strict
    and enforces a type safe programming style.

 */
    class DECONZ_DLLSPEC ZclCommandId_t
    {
        quint8 m_id = 0;

    public:
        /*! Default constructor, with value = 0. */
        constexpr ZclCommandId_t() {}
        /*! Explicit constructor to init by value. */
        constexpr explicit ZclCommandId_t(quint8 id) : m_id(id) { }
        /*! Explicit \c quint8 conversion operator, needs `static_cast<quint8>()`. */
        constexpr explicit operator quint8() const { return m_id; }

        /*! Don't allow construction via implicit integer conversion. */
        template <class T>
        ZclCommandId_t(T) = delete;

        /*! Equal operator, requires strong type. */
        constexpr bool operator==(ZclCommandId_t other) const { return other.m_id == m_id; }
        /*! Not equal operator, requires strong type. */
        constexpr bool operator!=(ZclCommandId_t other) const { return other.m_id != m_id; }
    };

    /*! Writes ZclCommandId_t as quint8 to the QDataStream.
        \relates ZclCommandId_t
     */
    DECONZ_DLLSPEC QDataStream & operator<<(QDataStream &ds, ZclCommandId_t id);
    /*! Reades ZclCommandId_t as quint8 from the QDataStream.
        \relates ZclCommandId_t
    */
    DECONZ_DLLSPEC QDataStream & operator>>(QDataStream &ds, ZclCommandId_t &id);

/*! \ingroup cpp_literals
    \brief Literal to create ZclCommandId_t.
    \since v2.6.1
    \relates ZclCommandId_t

Shorthand for creating type safe ZCL command identifiers from numbers but without casting.

#### Example usage for streams

\code {cpp}

    QDataStream stream;

    stream << 0x05_cmdid;

    // same as

    stream << ZclCommandId_t(static_cast<quint8>(0x05)));

\endcode
 */
    inline namespace literals {
        constexpr ZclCommandId_t operator ""_cmdid(unsigned long long id)
        {
            return ZclCommandId_t(static_cast<quint8>(id));
        }
    }

    /*! Allowed access method of a ZclAttribute. */
    enum ZclAccess
    {
        ZclRead = 0x1,
        ZclWrite = 0x2,
        ZclReadWrite = 0x3
    };

/*! \ingroup strong_types
    \class ZclAttributeId_t
    \brief A strong typed ZCL attribute identifier.
    \see strong_types
    \since v2.6.1

This wrapper around \c quint16 data type as ZCL attribute identifier for code correctness.
It helps to prevent common hard to catch programming errors. The class is very strict
and enforces a type safe programming style.

#### Problems
Following issues where observed in the past and are hard to spot and debug.

\code {cpp}
    quint8  at1 = 0x8001;     // does compile, at best with a warning
    quint32 at2 = 0x0002;     // does compile, no warning

    stream << at1;            // 0x01 instead of 0x8001
    stream << at2;            // 0x00000001 instead of 0x0001
    stream << (quint8)0x0001; // 0x01 instead of 0x0001
    stream << (quint16)at1;   // at1 is already wrong type, won't be catched here either
\endcode


The strong typed ZclAttributeId_t catches all those errors at compile time.

#### Example: Construction

\code {cpp}
    ZclAttributeId_t at0(ONOFF_CLUSTER_ATTRID_ONOFF);    // ok, preferred, no magic numbers
    ZclAttributeId_t at1(0x0001_atid);                   // ok, using literal for ZclAttributeId_t
    ZclAttributeId_t at2(quint16(0x0000));               // ok
    ZclAttributeId_t at3(static_cast<quint16>(0x0001));  // ok
    ZclAttributeId_t at4(0x0002);                        // compile error, no conversion from int

    quint16          good1 = 0x0000;
    ZclAttributeId_t good2 = 0x0001_atid;
    quint8           bad1   = 0x0003;
    quint32          bad2   = 0x0004;

    ZclAttributeId_t at5(good1);  // ok
    ZclAttributeId_t at6(good2);  // ok
    ZclAttributeId_t at7(bad1);   // compile error, no conversion from quint8
    ZclAttributeId_t at8(bad2);   // compile error, no conversion from quint32
\endcode

#### Example: Assignment

\code {cpp}
    ZclAttributeId_t at1(ONOFF_CLUSTER_ATTRID_ONOFF);
    ZclAttributeId_t at2(0x0001_atid);

    at1 = ZclAttributeId_t(quint16(0x0005)); // ok
    at1 = at2;             // ok
    at1 = 0x0005_atid;     // ok
    at1 = 0x0005;          // compile error
    at1 = quint16(0x0005); // compile error due explicit constructor
\endcode

#### Example: Write attribute to stream

\code {cpp}
    //#define ONOFF_CLUSTER_ATTRID_ONOFF quint16(0x0000)  // ok, strong typed define
    #define ONOFF_CLUSTER_ATTRID_ONOFF 0x0000_atid        // ok, and better, using literal for ZclAttributeId_t

    QDataStream stream;

    ZclAttributeId_t at(ONOFF_CLUSTER_ATTRID_ONOFF);

    stream << (quint8) at;              // programmer mistake, won't be detected by compiler (don't use C style cast!)
    stream << static_cast<quint8>(at);  // compile error no valid conversion from ZclAttributeId_t to quint8

    stream << at;                                           // ok, preferred, because operator<< for QDataStream
    stream << ZclAttributeId_t(ONOFF_CLUSTER_ATTRID_ONOFF); // ok, because operator<< for QDataStream
    stream << static_cast<quint16>(at);                     // ok, static_cast<quint16> allowed
\endcode

\b Important: Always use C++ \c static_cast<> instead C style cast to catch invalid conversions at compile time.
Note when using ZclAttributeId_t properly, casts aren't needed at all.

#### Why use verbose ugly code?
In the following example (1) and (2) do the same and (1) seems easier to reason about at first.

\code {cpp}
    quint16 at1 = 0x0005;                  // (1)

    ZclAttributeId_t at2(quint16(0x0005)); // (2)
\endcode

\c at2 looks verbose and ugly and it should be! When using numbers directly, it's a code smell for magic values.
Searching for \c 0x0005 when looking for a certain attribute is not going well.

\b Better: Use strong typed defines or C++11 enum classes with type quint16 or the C++ literal `<number>_atid`.

\code {cpp}
    //#define ONOFF_CLUSTER_ATTRID_ONOFF quint16(0x0000)  // ok, strong typed define
    #define ONOFF_CLUSTER_ATTRID_ONOFF 0x0000_atid        // ok, and better, using literal for ZclAttributeId_t

    ZclAttributeId_t at1(ONOFF_CLUSTER_ATTRID_ONOFF);  // ok
    ZclAttributeId_t at1(0x0000_atid);                 // ok, but prefer defines or enums
\endcode

#### Summary

  - No data type errors possible.
  - No casts needed.
  - No magic numbers, ZclAttributeId_t as well as attribute defines are searchable.
*/
    class DECONZ_DLLSPEC ZclAttributeId_t
    {
        quint16 m_id = 0;

    public:
        /*! Default constructor, with value = 0. */
        constexpr ZclAttributeId_t() {}
        /*! Explicit constructor to init by value. */
        constexpr explicit ZclAttributeId_t(quint16 id) : m_id(id) { }
        /*! Explicit \c quint16 conversion operator, needs `static_cast<quint16>()`. */
        constexpr explicit operator quint16() const { return m_id; }

        /*! Don't allow construction via implicit integer conversion. */
        template <class T>
        ZclAttributeId_t(T) = delete;

        /*! Equal operator, requires strong type. */
        constexpr bool operator==(ZclAttributeId_t other) const { return other.m_id == m_id; }
        /*! Not equal operator, requires strong type. */
        constexpr bool operator!=(ZclAttributeId_t other) const { return other.m_id != m_id; }
    };

    /*! Writes ZclAttributeId_t as quint16 to the QDataStream.
        \relates ZclAttributeId_t
     */
    DECONZ_DLLSPEC QDataStream &operator<<(QDataStream &ds, ZclAttributeId_t id);
    /*! Reades ZclAttributeId_t as quint16 from the QDataStream.
        \relates ZclAttributeId_t
     */
    DECONZ_DLLSPEC QDataStream & operator>>(QDataStream &ds, ZclAttributeId_t &id);

/*! \ingroup cpp_literals
    \brief Literal to create ZclAttributeId_t.
    \since 2.6.1
    \relates ZclAttributeId_t

Shorthand for creating type safe ZCL attribute identifiers from numbers but without casting.

#### Example usage for streams

\code {cpp}

    QDataStream stream;

    stream << 0x0001_atid;

    // same as

    stream << ZclAttributeId_t(quint16(0x0001));

\endcode

#### Example usage in ZclAttributeId_t constructors and assignment

\code {cpp}

    ZclAttributeId_t at0(0x0001_atid);  // constructor
    ZclAttributeId_t at1 = 0x0002_atid; // assignment

    if (at0 == at1) { }

    if (at0 == 0x0003_atid) {}

\endcode
 */
    inline namespace literals {
        constexpr ZclAttributeId_t operator ""_atid(unsigned long long id)
        {
            return ZclAttributeId_t(static_cast<quint16>(id));
        }
    }

/*! \ingroup strong_types
    \class ManufacturerCode_t
    \brief A strong typed Zigbee manufacturer code.
    \see strong_types
    \since v2.6.1

    This wrapper around quint16 data type as Zigbee manufacturer codes for code correctness.
    It helps to prevent common hard to catch programming errors. The class is very strict
    and enforces a type safe programming style.
 */
    class DECONZ_DLLSPEC ManufacturerCode_t
    {
        quint16 m_id = 0;

    public:
        /*! Default constructor, with value = 0. */
        constexpr ManufacturerCode_t() {}
        /*! Explicit constructor to init by value. */
        constexpr explicit ManufacturerCode_t(quint16 id) : m_id(id) { }
        /*! Explicit \c quint16 conversion operator, needs `static_cast<quint16>()`. */
        constexpr explicit operator quint16() const { return m_id; }

        /*! Don't allow construction via implicit integer conversion. */
        template <class T>
        ManufacturerCode_t(T) = delete;

        /*! Equal operator, requires strong type. */
        constexpr bool operator==(ManufacturerCode_t other) const { return other.m_id == m_id; }
        /*! Not equal operator, requires strong type. */
        constexpr bool operator!=(ManufacturerCode_t other) const { return other.m_id != m_id; }
    };

    /*! Writes ManufacturerCode_t as quint16 to the QDataStream.
        \relates ManufacturerCode_t
     */
    DECONZ_DLLSPEC QDataStream & operator<<(QDataStream &ds, ManufacturerCode_t mfcode);
    /*! Reades ManufacturerCode_t as quint16 from the QDataStream.
        \relates ManufacturerCode_t
    */
    DECONZ_DLLSPEC QDataStream & operator>>(QDataStream &ds, ManufacturerCode_t &mfcode);

/*! \ingroup cpp_literals
    \brief Literal to create ManufacturerCode_t.
    \since 2.6.1
    \relates ManufacturerCode_t

Shorthand for creating type safe manufacturer codes from numbers but without casting.

#### Example usage for streams

\code {cpp}

    QDataStream stream;

    stream << 0x1135_mfcode;

    // same as

    stream << ManufacturerCode_t(static_cast<quint16>(0x1135)));

\endcode
 */
    inline namespace literals {
        constexpr ManufacturerCode_t operator ""_mfcode(unsigned long long mfcode)
        {
            return ManufacturerCode_t(static_cast<quint16>(mfcode));
        }
    }

    class ZclAttributePrivate;

    /*!
        \ingroup zcl
        \class ZclAttribute
        \brief Represents a ZigBee cluster attribute.
     */
    class DECONZ_DLLSPEC ZclAttribute
    {
    public:
        /*! Used to change the display format in the GUI. */
        enum FormatHint
        {
            DefaultFormat = 0, //!< Use the default format for a value
            Prefix, //!< Prepend prefix like 0x or 0b
            SliderFormat //!< Present a numeric value as a slider.
        };
        /*! Constructor */
        ZclAttribute();
        /*! Constructor used by ZCLDB parser. */
        ZclAttribute(uint16_t id, uint8_t type, const QString &name, ZclAccess access, bool required);
        /*! Strong typed constructor (preferred).
            \since v2.6.1
         */
        ZclAttribute(ZclAttributeId_t id, ZclDataTypeId_t type, const QString &name, ZclAccess access, bool required);
        /*! Copy constructor */
        ZclAttribute(const ZclAttribute &other);
        /*! Move constructor */
        ZclAttribute(ZclAttribute &&other) noexcept;
        /*! Copy assignment onstructor */
        ZclAttribute &operator=(const ZclAttribute &other);
        /*! Move assignment onstructor */
        ZclAttribute &operator=(ZclAttribute &&other) noexcept;
        /*! Deconstructor */
        ~ZclAttribute();
        /*! Returns the attribute identifier. */
        uint16_t id() const;
        /*! Returns the strong typed attribute identifier.
            \since v2.6.1
         */
        ZclAttributeId_t id_t() const;
        /*! Returns the attribute description. */
        const QString &description() const;
        /*! Sets the attribute description. */
        void setDescription(const QString &description);
        /*! Returns the attribute data type which is a ZclDataTypeId value. */
        uint8_t dataType() const;
        /*! Returns the strong type attribute data type.
            \since v2.6.1
         */
        ZclDataTypeId_t dataType_t() const;
        /*! Sets the attribute data type which is a ZclDataTypeId value. */
        void setDataType(uint8_t type);
        /*! Returns the attribute name. */
        const QString &name() const;
        /* \cond INTERNAL_SYMBOLS */
        uint8_t subType() const;
        void setSubType(uint8_t subType);
        /* \endcond */
        /*! Returns the attribute numberic value. */
        const NumericUnion &numericValue() const;
        /*! Sets the attribute numberic value. */
        void setNumericValue(const NumericUnion &value);

        /*! Returns the name of a enumerator or bitmap bit.

            There:
                    0 < bitOrEnum < bitCount()
                    or bitOrEnum is a known enumerator

            If bitOrEnum is unknown QString() will be returned.
         */
        QString valueNameAt(int bitOrEnum) const;
        /*! Returns all enumerator or bitmap bit names. */
        QStringList valuesNames() const;
        /*! Returns a list of all known bits or enumerators. */
        const std::vector<int> &valueNamePositions() const;
        /*! Sets the attribute bool value. */
        void setValue(bool value);
        /*! Sets the attribute unsigned value (8..64-bit). */
        void setValue(quint64 value);
        /*! Sets the attribute signed value (8..64-bit). */
        void setValue(qint64 value);
        /*! Sets the attribute value (various formats). */
        void setValue(const QVariant &value);
        /*! Sets the \p time where the attribute was last read. */
        /* \cond INTERNAL_SYMBOLS */
        void setLastRead(int64_t time);
        uint16_t listSizeAttribute() const;
        void setListSizeAttribute(uint16_t id);
        bool isList() const;
        int listSize() const;
        void setListSize(int listSize);
        /* \endcond */
        /*! Returns the last read time. */
        int64_t lastRead() const;
        /*! Returns true if the attribute is read only. */
        bool isReadonly() const;
        /*! Returns true if the attribute is mandatory. */
        bool isMandatory() const;
        /*! Returns true if the attribute is available on the device. */
        bool isAvailable() const;
        /*! Sets the is available flag of the attribute. */
        void setAvailable(bool available);
        /*! Returns the numeric base (for GUI display). */
        uint8_t numericBase() const;
        /*! Sets the numeric \p base (for GUI display). */
        void setNumericBase(uint8_t base);
        /*! Returns the current enumerator. */
        uint enumerator() const;
        /*! Sets the current enumerator. */
        void setEnumerator(uint value);
        /*! Set/unset a bit in the bitmap. */
        void setBit(uint bit, bool one);
        /*! Returns true if a bit is set where 0 < bit < bitCount() */
        bool bit(int bit) const;
        /*! Returns the number of bits in the bitmap. */
        int bitCount() const;
        /*! Returns the bitmap (8..64-bit) */
        quint64 bitmap() const;
        /*! Sets the bitmap to \p bmp (8..64-bit) */
        void setBitmap(quint64 bmp);
        /*! Returns the number of enum values. */
        int enumCount() const;
        /*! Returns the id of the enumeration. */
        quint8 enumerationId() const;
        /*! Sets the enumeration id. */
        void setEnumerationId(quint8 id);
        /*! Writes the attribute to \p stream. */
        bool writeToStream(QDataStream &stream) const;
        /*! Reads the attribute from \p stream. */
        bool readFromStream(QDataStream &stream);
        /*! Returns the attribute as string representation. */
        QString toString(FormatHint formatHint = DefaultFormat) const;
        /*! Returns the attribute as string representation for given data type. */
        QString toString(const deCONZ::ZclDataType &dataType, FormatHint formatHint = DefaultFormat) const;
        /*! Returns the attribute as variant. */
        const QVariant &toVariant() const;
        /*! Returns the minimum report interval. */
        uint16_t minReportInterval() const;
        /*! Sets the minimum report \p interval. */
        void setMinReportInterval(uint16_t interval);
        /*! Returns the maximum report interval. */
        uint16_t maxReportInterval() const;
        /*! Sets the maximum report \p interval. */
        void setMaxReportInterval(uint16_t interval);
        /*! Returns the report timeout period. */
        uint16_t reportTimeoutPeriod() const;
        /*! Sets the report timeout \p period. */
        void setReportTimeoutPeriod(uint16_t period);
        /*! Returns the reportable change as numeric value. */
        const NumericUnion &reportableChange() const;
        /*! Sets the reportable change. */
        void setReportableChange(const NumericUnion &reportableChange);
        /*! Writes the reportable change to \p stream TODO: describe. */
        bool writeReportableChangeToStream(QDataStream &stream) const;
        /*! Reads the reportable change from \p stream TODO: describe. */
        bool readReportableChangeFromStream(QDataStream &stream);
        /*! Sets the format hint for GUI display. */
        void setFormatHint(FormatHint formatHint);
        /*! Returns the format hint for GUI display. */
        FormatHint formatHint() const;
        /*! Return the minimum range. */
        int rangeMin() const;
        /*! Sets the minimum range. */
        void setRangeMin(int rangeMin);
        /*! Return the maximum range. */
        int rangeMax() const;
        /*! Sets the maximum range. */
        void setRangeMax(int rangeMax);
        /*! Returns the manufacturer code (default: 0x0000 = not manufacturer specific). */
        quint16 manufacturerCode() const;
        /*! Returns the strong typed manufacturer code (preferred).
            \since v2.6.1
         */
        ManufacturerCode_t manufacturerCode_t() const;
        /*! Sets the manufacturer code. */
        void setManufacturerCode(quint16 mfcode);
        /*! Sets the strong typed manufacturer code (preferred).
            \since v2.6.1
         */
        void setManufacturerCode(ManufacturerCode_t mfcode);
        /*! Returns true if attribute is manufacturer specific. */
        bool isManufacturerSpecific() const;

        /* \cond INTERNAL_SYMBOLS */
        void setAttributeSet(quint16 attrSetId, quint16 mfcode);
        quint16 attributeSet() const;
        quint16 attributeSetManufacturerCode() const;
        /* \endcond */

    private:
        friend class ZclDataBase;
        ZclAttributePrivate *d_ptr = nullptr;
        Q_DECLARE_PRIVATE(ZclAttribute)
    };

    class ZclAttributeSetPrivate;

    /*!
        \ingroup zcl
        \class ZclAttributeSet
        \brief Represents a named group of ZclAttribute.
     */
    class DECONZ_DLLSPEC ZclAttributeSet
    {
    public:
        /*! Constructor. */
        ZclAttributeSet();
        /*! Copy constructor. */
        ZclAttributeSet(const ZclAttributeSet &other);
        /*! Copy assignment constructor. */
        ZclAttributeSet &operator= (const ZclAttributeSet &other);
        /*! Constructor used by ZCLDB. */
        ZclAttributeSet(uint16_t id, const QString &description);
        /*! Destructor. */
        ~ZclAttributeSet();
        /*! Returns the attribute set identifier. */
        uint16_t id() const;
        /*! Returns the attribute set description. */
        const QString &description() const;
        /*! Returns a list of indexes into ZclCluster::attributes() list. */
        const std::vector<int> &attributes() const;
        /*! Adds a attribute to the set.
            \param idx index into ZclCluster::attributes() list
         */
        void addAttribute(int idx);
        /*! Return the manufacturer code (default: 0x0000 = not manufacturer specific). */
        quint16 manufacturerCode() const;
        /*! Sets the manufacturer code. */
        void setManufacturerCode(quint16 mfcode);

    private:
        ZclAttributeSetPrivate *d_ptr = nullptr;
        Q_DECLARE_PRIVATE(ZclAttributeSet)
    };

    /*! ZCL cluster side where client is a out cluster and server is a in cluster. */
    enum ZclClusterSide
    {
        ClientCluster = 0,
        ServerCluster = 1
    };

    class ZclFramePrivate;

    /*!
        \ingroup zcl
        \class ZclFrame
        \brief Helper to build ZCL based payloads to be transported via ApsDataRequest.
     */
    class DECONZ_DLLSPEC ZclFrame
    {
    public:
        /*! Constructor. */
        ZclFrame();
        /*! Copy constructor. */
        ZclFrame(const ZclFrame &other);
        /*! Copy assignment constructor. */
        ZclFrame &operator=(const ZclFrame &other);
        /*! Deconstructor. */
        virtual ~ZclFrame();
        /*! Returns ZCL header frame control field.
            \returns An OR combinded value of deCONZ::ZclFrameControl flags
         */
        uint8_t frameControl() const;
        /*! Sets the ZCL header frame control field.
            \param frameControl is an OR combinded value of deCONZ::ZclFrameControl flags
         */
        void setFrameControl(uint8_t frameControl);
        /*! Returns the vendor specific manufacturer code. */
        uint16_t manufacturerCode() const;
        /*! Returns the strong typed vendor specific manufacturer code.
            \since v2.6.1
         */
        ManufacturerCode_t manufacturerCode_t() const;
        /*! Sets the vendor specific manufacturer code. */
        void setManufacturerCode(uint16_t code);
        /*! Sets the strong typed vendor specific manufacturer code.
            \since v2.6.1
         */
        void setManufacturerCode(ManufacturerCode_t code);
        /*! Returns the sequence number associated with a request. */
        uint8_t sequenceNumber() const;
        /*! Sets the sequence number of a request. */
        void setSequenceNumber(uint8_t seqNumber);
        /*! Returns the ZCL command identifier. */
        uint8_t commandId() const;
        /*! Returns the strong typed ZCL command identifier.
            \since v2.6.1
         */
        ZclCommandId_t commandId_t() const;
        /*! Sets the ZCL command identifier. */
        void setCommandId(uint8_t commandId);
        /*! Sets the strong typed ZCL command identifier.
            \since v2.6.1
         */
        void setCommandId(ZclCommandId_t commandId);
        /*! Returns the ZCL payload byte at given \p index.
            If the index is out of bounds 0 is returned.
            \since v2.18.0
        */
        unsigned char payloadAt(int index) const;
        /*! Returns the const ZCL payload. */
        const QByteArray &payload() const;
        /*! Returns the modifiable ZCL payload. */
        QByteArray &payload();
        /*! Sets the ZCL payload. */
        void setPayload(const QByteArray &payload);
        /*! Writes the ZCL frame in ZigBee standard conform format to the \p stream.
            \param stream shall write to a ApsDatarequest::asdu() buffer
         */
        void writeToStream(QDataStream &stream);
        /*! Reads a ZCL frame in ZigBee standard conform format from the \p stream.
            \param stream shall read from a ApsDatarequest::asdu() buffer
         */
        void readFromStream(QDataStream &stream);
        /*! Returns true if the command() is related to a cluster (for example OnOff cluster). */
        bool isClusterCommand() const;
        /*! Returns true if the command() is profile wide (any ZclGeneralCommandId). */
        bool isProfileWideCommand() const;
        /*! Returns true if the ZCL frame is a default response. */
        bool isDefaultResponse() const;
        /*! Returns true if readFromStream() succeeded.
            \since v2.11.5
         */
        bool isValid() const;
        /*! Returns the command identifier wich belongs to a default response. */
        uint8_t defaultResponseCommandId() const;
        /*! Returns the strong typed command identifier wich belongs to a default response.
            \since v2.6.1
         */
        ZclCommandId_t defaultResponseCommandId_t() const;
        /*! Returns the ZCL status wich belongs to a default response. */
        ZclStatus defaultResponseStatus() const;
        /*! Resets the object to it's initial state, allowing reusing it without allocating a new one.
            \since v2.12.0
         */
        void reset();

    private:
        ZclFramePrivate *d_ptr = nullptr;
        Q_DECLARE_PRIVATE(ZclFrame)
    };

    class ZclCommandPrivate;

    /*!
        \ingroup zcl
        \class ZclCommand
        \brief Represents a ZigBee cluster command (ZCL and non-ZCL).
     */
    class DECONZ_DLLSPEC ZclCommand
    {
    public:
        /*! Constructor */
        ZclCommand();
        /*! Copy constructor */
        ZclCommand(const ZclCommand &other);
        /*! Move constructor */
        ZclCommand(ZclCommand &&other) noexcept;
        /*! Copy assignment constructor */
        ZclCommand&operator=(const ZclCommand &other);
        /*! Move assignment constructor */
        ZclCommand&operator=(ZclCommand &&other) noexcept;
        /*! Constructor used by ZCLDB parser. */
        ZclCommand(uint8_t id, const QString &name, bool required, bool recv, const QString &description = QString());
        /*! Deconstructor */
        ~ZclCommand();
        /*! Returns the command identifier. */
        uint8_t id() const;
        /*! Returns the strong typed command identifier.
            \since v2.6.1
         */
        ZclCommandId_t id_t() const;
        /*! Sets the command identifier. */
        void setId(uint8_t id);
        /*! Returns the manufacturer identifier. */
        uint16_t manufacturerId() const;
        /*! Sets the manufacturer identifier. */
        void setManufacturerId(uint16_t id);
        /*! Returns true if this command has valid data. */
        bool isValid() const;
        /*! Returns the response command identifier. */
        uint8_t responseId() const;
        /*! Sets the response command identifier. */
        void setResponseId(uint8_t id);
        /*! Returns true if this command expects a response. */
        bool hasResponse() const;
        /*! Returns true if the command will be received. */
        bool directionReceived() const;
        /*! Returns false if the command will be send. */
        bool directionSend() const;
        /*! Returns the name of the command. */
        const QString &name() const;
        /*! Returns the description of the command. */
        const QString &description() const;
        /*! Sets the description of the command. */
        void setDescription(const QString &description);
        /*! Returns true if the command is profile wide (not cluster specific). */
        bool isProfileWide() const;
        /*! Sets the is profile wide flag. */
        void setIsProfileWide(bool profileWide);
        /*! Returns true if the disable default response flag is set. */
        bool disableDefaultResponse() const;
        /*! Sets the disable default response flag. */
        void setDisableDefaultResponse(bool disable);
        /*! Returns the const list of command parameters. */
        const std::vector<ZclAttribute> &parameters() const;
        /*! Returns the modifiable list of command parameters. */
        std::vector<ZclAttribute> &parameters();
        /*! Reads the command parameters from \p stream. */
        bool readFromStream(QDataStream &stream);
        /*! Writes the command parameters to \p stream. */
        bool writeToStream(QDataStream &stream) const;

    private:
        ZclCommandPrivate *d_ptr = nullptr;
        Q_DECLARE_PRIVATE(ZclCommand)
    };

/*! \ingroup strong_types
    \class ZclClusterId_t
    \brief A strong typed ZCL cluster identifier.
    \see strong_types
    \since v2.6.1

    This wrapper around quint16 data type as ZCL cluster identifier for code correctness.
    It helps to prevent common hard to catch programming errors. The class is very strict
    and enforces a type safe programming style.

 */
    class DECONZ_DLLSPEC ZclClusterId_t
    {
        quint16 m_id = 0;

    public:
        /*! Default constructor, with value = 0. */
        constexpr ZclClusterId_t() {}
        /*! Explicit constructor to init by value. */
        constexpr explicit ZclClusterId_t(quint16 id) : m_id(id) { }
        /*! Explicit \c quint16 conversion operator, needs `static_cast<quint16>()`. */
        constexpr explicit operator quint16() const { return m_id; }

        /*! Don't allow construction via implicit integer conversion. */
        template <class T>
        ZclClusterId_t(T) = delete;

        /*! Equal operator, requires strong type. */
        constexpr bool operator==(ZclClusterId_t other) const { return other.m_id == m_id; }
        /*! Not equal operator, requires strong type. */
        constexpr bool operator!=(ZclClusterId_t other) const { return other.m_id != m_id; }
    };

    /*! Writes ZclClusterId_t as quint16 to the QDataStream.
        \relates ZclClusterId_t
     */
    DECONZ_DLLSPEC QDataStream &operator<<(QDataStream &ds, ZclClusterId_t id);
    /*! Reades ZclClusterId_t as quint16 from the QDataStream.
        \relates ZclClusterId_t
    */
    DECONZ_DLLSPEC QDataStream &operator>>(QDataStream &ds, ZclClusterId_t &id);

/*! \ingroup cpp_literals
    \brief Literal to create ZclClusterId_t.
    \since v2.6.1
    \relates ZclClusterId_t

Shorthand for creating type safe cluster identifiers from numbers but without casting.

#### Example usage for streams

\code {cpp}

    QDataStream stream;

    stream << 0x0500_clid;

    // same as

    stream << ZclClusterId_t(static_cast<quint16>(0x0500)));

\endcode
 */
    inline namespace literals {
        constexpr ZclClusterId_t operator ""_clid(unsigned long long id)
        {
            return ZclClusterId_t(static_cast<quint16>(id));
        }
    }

    class ZclClusterPrivate;

    /*!
        \ingroup zcl
        \class ZclCluster
        \brief Represents a ZigBee cluster (ZCL and non-ZCL).
        \see SimpleDescriptor::inClusters(), SimpleDescriptor::outClusters()
     */
    class DECONZ_DLLSPEC ZclCluster
    {
    public:
        /*! Constructor. */
        ZclCluster();
        /*! Copy constructor. */
        ZclCluster(const ZclCluster &other);
        /*! Constructor used by ZCLDB parser. */
        ZclCluster(uint16_t id, const QString &name, const QString &description = QString());
        /*! Copy assignment constructor. */
        ZclCluster &operator=(const ZclCluster &other);
        /*! Deconstructor. */
        ~ZclCluster();
        /*! Returns the cluster identifier. */
        uint16_t id() const;
        /*! Returns strong typed cluster identifier.
            \since v2.6.1
         */
        ZclClusterId_t id_t() const;
        /*! Sets the cluster identifier. */
        void setId(uint16_t id);
        /*! Returns the opposite cluster identifier (server/client) which is the same for ZCL based clusters. */
        uint16_t oppositeId() const;
        /*! Sets the opposite cluster identifier. */
        void setOppositeId(uint16_t id);
        /*! Returns the manufacturer code, default = 0. */
        uint16_t manufacturerCode() const;
        /*! Returns the strong typed manufacturer code (preferred), default = 0.
            \since v2.6.1
         */
        ManufacturerCode_t manufacturerCode_t() const;
        /*! Sets the manufacturer code. */
        void setManufacturerCode(uint16_t manufacturerCode);
        /*! Sets the strong typed manufacturer code (preferred).
            \since v2.6.1
         */
        void setManufacturerCode(ManufacturerCode_t manufacturerCode);
        /*! Returns the cluster name. */
        const QString &name() const;
        /*! Returns the cluster description. */
        const QString &description() const;
        /*! Sets the cluster description. */
        void setDescription(const QString &description);
        /*! Returns true if this cluster has valid data. */
        bool isValid() const;
        /*! Returns true if this cluster is ZCL based. */
        bool isZcl() const;
        /*! Sets the cluster is ZCL based flag. */
        void setIsZcl(bool isZcl);
        /*! Returns true if this is a server (in) cluster. */
        bool isServer() const;
        /*! Returns true if this is a client (out) cluster. */
        bool isClient() const;
        /*! Sets the is a server (in) cluster flag, this also effects the isClient() method. */
        void setIsServer(bool isServer);
        /*! Returns the modifiable attributes list. */
        std::vector<ZclAttribute> &attributes();
        /*! Returns the modifiable attributes list. */
        const std::vector<ZclAttribute> &attributes() const;
        /*! Returns the const attribute sets list. */
        std::vector<ZclAttributeSet> &attributeSets();
        /*! Returns the const attribute sets list. */
        const std::vector<ZclAttributeSet> &attributeSets() const;
        /*! Reads a ZCL based command from a ZCL frame. */
        bool readCommand(const ZclFrame &zclFrame);
        /*! Reads a non-ZCL based single command (like ZDP) from APSDE-DATA.indication. */
        bool readCommand(const ApsDataIndication &ind);
        /*! Returns the modifiable list of commands. */
        std::vector<ZclCommand> &commands();
        /*! Returns the const list of commands. */
        const std::vector<ZclCommand> &commands() const;

    private:
        ZclClusterPrivate *d_ptr = nullptr;
        Q_DECLARE_PRIVATE(ZclCluster)
    };


    class ZclMemoryPrivate;

    class DECONZ_DLLSPEC ZclMemory
    {
    public:
        ZclMemory();
        ~ZclMemory();

        /* \cond INTERNAL_SYMBOLS */
        ZclMemoryPrivate *d = nullptr; // opaque pointer only used internally
        /* \endcond */
    };

    DECONZ_DLLSPEC ZclCluster ZCL_InCluster(uint16_t profileId, uint16_t clusterId, uint16_t mfcode);
    DECONZ_DLLSPEC ZclCluster ZCL_OutCluster(uint16_t profileId, uint16_t clusterId, uint16_t mfcode);
    DECONZ_DLLSPEC ZclDataType ZCL_DataType(uint8_t id);
    DECONZ_DLLSPEC ZclDataType ZCL_DataType(const QString &name);

} // namespace deCONZ

#endif // DECONZ_ZCL_H
