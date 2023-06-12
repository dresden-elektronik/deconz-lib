/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef ZCL_PRIVATE_H
#define ZCL_PRIVATE_H

#include <QString>
#include <cinttypes>
#include "deconz/declspec.h"

namespace deCONZ
{

class ZclAttributePrivate
{
public:
    ZclAttributePrivate();

    static constexpr int PoolSize = 64; // for ZclMemory

    uint16_t m_id = 0xFFFF;
    uint8_t m_dataType = 0xFF;
    uint8_t m_subType = 0xFF;
    QString m_name;
    QString m_description;
    ZclAccess m_access = ZclRead;
    uint8_t m_enumerationId = 0xFF; //!<  Id of a enumeration repository.
    uint8_t m_numericBase = 10; //!< For numeric data types.
    bool m_required = false; //!< True if mandatory.
    bool m_avail = true; //!< Attribute is available in the cluster.
    union {
        int idx; //!< index into a list
        uint64_t bitmap; //!< bitmap bits set
    } m_valueState;

    /*!
        May hold a value or the static content like enumeration or
        bitmap values. If so the m_valueInfo.(idx|bitmap) is used
        to optain the real value.
     */
    QVariant m_value;
    NumericUnion m_numericValue{}; //!< Numeric data.
    std::vector<int> m_valuePos;
    QStringList m_valueNames;
    time_t m_lastRead = (time_t)-1;

    /*!
         If this is a list here is the attribute id wich holds the list size.
         if set to 0xFFFF this is no list.
     */
    uint16_t m_listSizeAttr = 0xFFFF;
    int m_listSize = 0; //!< Current size of the list.

    // reporting
    uint16_t m_minReportInterval = 0; //!< Minimum reporting interval.
    uint16_t m_maxReportInterval = 0xFFFF; //!< Maximum reporting interval.
    uint16_t m_reportTimeout = 0; //!< Report timeout period.
    NumericUnion m_reportableChange{}; //!< Reportable change.

    ZclAttribute::FormatHint m_formatHint = ZclAttribute::DefaultFormat;
    int m_rangeMin = 0;
    int m_rangeMax = 0;

    quint16 m_manufacturerCode = 0;

    quint16 m_attrSetId = 0xFFFF;
    quint16 m_attrSetManufacturerCode = 0;
};

class ZclAttributeSetPrivate
{
public:
    ZclAttributeSetPrivate();
    uint16_t id;
    QString description;
    quint16 manufacturerCode;
    std::vector<int> attributeIndexes;
};

class ZclClusterPrivate
{
public:
    ZclClusterPrivate();
    uint16_t id;
    uint16_t oppositeId;
    uint16_t manufacturerCode;
    QString name;
    QString description;
    bool isZcl;
    bool isServer;
    std::vector<ZclAttribute> attributes;
    std::vector<ZclAttributeSet> attributeSets;
    std::vector<ZclCommand> commands;
};

class ZclFramePrivate
{
public:
    static constexpr int PoolSize = 16; // for ZclMemory
    uint8_t valid = 0;
    uint8_t frameControl = 0;
    uint16_t manufacturerCode = 0xFFFF;
    uint8_t seqNumber = 0;
    uint8_t commandId = 0;
    QByteArray payload;
};

class ZclDataTypePrivate
{
public:
    /*! Data kind of the data type. */
    enum DataKind
    {
        UnknownData,
        AnalogData,
        DiscreteData
    };
    ZclDataTypePrivate();
    uint8_t m_id;
    QString m_name;
    QString m_shortname;
    int m_length;
    DataKind m_analogDiscrete;
};

class ZclCommandPrivate
{
public:
    ZclCommandPrivate();
    uint8_t m_id;
    uint16_t m_manufacturerId;
    uint8_t m_responseId;
    QString m_name;
    bool m_required;
    bool m_recv;
    QString m_description;
    bool m_isProfileWide;
    bool m_disableDefaultResponse;
    std::vector<ZclAttribute> m_payload;
};

// TODO: place in private header and hide in public release
class DECONZ_DLLSPEC ZclDomain
{
public:
    ZclDomain() : m_useZcl(true) { }
    ZclDomain(const QString &name, const QString &description, const QIcon &icon) :
        m_useZcl(true),
        m_name(name),
        m_description(description),
        m_icon(icon)
    {
#ifdef ZCL_LOAD_DBG
        qDebug("Domain: %s %04X-%04X -- %s", qPrintable(name),  qPrintable(description));
#endif
    }
    bool useZcl() const { return m_useZcl; }
    void setUseZcl(bool useZcl) { m_useZcl = useZcl; }
    const QString &name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }
    const QString &description() const { return m_description; }
    void setDescription(const QString &description) { m_description = description; }
    const QIcon &icon() const { return m_icon; }
    void setIcon(const QIcon &icon) { m_icon = icon; }
    const QHash<uint32_t, ZclCluster> &inClusters() const { return m_inClusters; }
    const QHash<uint32_t, ZclCluster> &outClusters() const { return m_outClusters; }
    bool isValid() const { return !m_name.isEmpty(); }

private:
    friend class ZclDataBase;
    bool m_useZcl;
    QString m_name;
    QString m_description;
    QIcon m_icon;
    QHash<uint32_t, ZclCluster> m_inClusters;
    QHash<uint32_t, ZclCluster> m_outClusters;
};

class DECONZ_DLLSPEC ZclDevice
{
public:
    ZclDevice() : m_deviceId(0xFFFF), m_profileId(0xFFFF) {}
    ZclDevice(uint16_t id, const QString &name, const QString &description, const QIcon &icon) :
        m_deviceId(id),
        m_profileId(0xFFFF),
        m_name(name),
        m_description(description),
        m_icon(icon)
    {
#ifdef ZCL_LOAD_DBG
        qDebug("ZclDevice: %04X %s -- %s", id, qPrintable(name),  qPrintable(description));
#endif
    }

    uint16_t id() const { return m_deviceId; }
    void setId(uint16_t id) { m_deviceId = id; }
    uint16_t profileId() const { return m_profileId; }
    void setProfileId(uint16_t id) { m_profileId = id; }
    const QString &name() const { return m_name; }
    const QString &description() const { return m_description; }
    const QIcon &icon() const { return m_icon; }

private:
    uint16_t m_deviceId;
    uint16_t m_profileId;
    QString m_name;
    QString m_description;
    QIcon m_icon;
};

// TODO: place in private header and hide in public release
class DECONZ_DLLSPEC ZclProfile
{
public:
    ZclProfile() : m_id(0xFFFF) {}
    ZclProfile(uint16_t id, const QString &name, const QString &description, const QIcon &icon) :
        m_id(id),
        m_name(name),
        m_description(description),
        m_icon(icon)
    {
#ifdef ZCL_LOAD_DBG
        qDebug("Profile: %04X %s -- %s", id, qPrintable(name),  qPrintable(description));
#endif
    }

    uint16_t id() const { return m_id; }
    void setId(uint16_t id) { m_id = id; }
    const QString &name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }
    const QString &description() const { return m_description; }
    void setDescription(const QString &description) { m_description = description; }
    const QIcon &icon() const { return m_icon; }
    void setIcon(const QIcon &icon) { m_icon = icon; }
    const std::vector<ZclDomain> &domains() const { return m_domains; }
    void addDomain(const ZclDomain &domain);
    bool isValid() const { return (m_id != 0xFFFF); }

private:
    friend class ZclDataBase;

    uint16_t m_id;
    QString m_name;
    QString m_description;
    QIcon m_icon;
    std::vector<ZclDomain> m_domains;
};

// TODO: place in private header and hide in public release
class DECONZ_DLLSPEC ZclDataBase
{
public:
    ZclDataBase();
    ~ZclDataBase();
    ZclCluster inCluster(uint16_t profileId, uint16_t clusterId, quint16 mfcode);
    ZclCluster outCluster(uint16_t profileId, uint16_t clusterId, quint16 mfcode);
    const ZclDataType &dataType(uint8_t id) const;
    const ZclDataType &dataType(const QString &shortName) const;
    ZclProfile profile(uint16_t id);
    ZclDomain domain(const QString &name);
    void addDomain(const ZclDomain &domain);
    void addProfile(const ZclProfile &profile);
    ZclDevice device(uint16_t profileId, uint16_t deviceId);
    bool getEnumeration(uint id, deCONZ::Enumeration &out)
    {
        for (const auto &e : m_enums)
        {
            if (e.id() == id)
            {
                out = e;
                return true;
            }
        }

        return false;
    }
    void load(const QString &dbfile);
    void initDbFile(const QString &zclFile);
    void reloadAll(const QString &zclFile);
    void clear();
    bool knownDataType(uint8_t id);

private:
    std::vector<deCONZ::Enumeration> m_enums;
    ZclCluster m_unknownCluster;
    ZclDataType m_unknownDataType;
    std::vector<ZclDataType> m_dataTypes;
    std::vector<ZclDomain> m_domains;
    QHash<uint16_t, ZclProfile> m_profiles;
    /*!
        The device list is ordered. Private profile
        devices comes first. (device.profileId() != 0xFFFF).
     */
    std::vector<ZclDevice> m_devices;
    QString m_iconPath;
};

DECONZ_DLLSPEC ZclDataBase * zclDataBase();

} //namespace deCONZ

#endif // ZCL_PRIVATE_H
