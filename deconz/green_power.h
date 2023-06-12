#ifndef DECONZ_GREEN_POWER_H
#define DECONZ_GREEN_POWER_H

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include "deconz/types.h"
#include "deconz/declspec.h"

class QDataStream;

namespace deCONZ
{

enum GpDeviceIds
{
    // GP Generic
    GpDeviceIdOnOffSwitch = 0x02
};

enum GpCommandIds
{
    GpCommandIdIdentify = 0x00,
    // 0x01 - 0x0f reserved
    GpCommandIdScene0             = 0x10,
    GpCommandIdScene1             = 0x11,
    GpCommandIdScene2             = 0x12,
    GpCommandIdScene3             = 0x13,
    GpCommandIdScene4             = 0x14,
    GpCommandIdScene5             = 0x15,
    GpCommandIdScene6             = 0x16,
    GpCommandIdScene7             = 0x17,
    GpCommandIdScene8             = 0x18,
    GpCommandIdScene9             = 0x19,
    GpCommandIdScene10            = 0x1a,
    GpCommandIdScene11            = 0x1b,
    GpCommandIdScene12            = 0x1c,
    GpCommandIdScene13            = 0x1d,
    GpCommandIdScene14            = 0x1e,
    GpCommandIdScene15            = 0x1f,
    GpCommandIdOff                = 0x20,
    GpCommandIdOn                 = 0x21,
    GpCommandIdToggle             = 0x22,
    GpCommandIdRelease            = 0x23,
    // 0x24 - 0x2f reserved
    GpCommandIdMoveUp             = 0x30,
    GpCommandIdMoveDown           = 0x31,
    GpCommandIdStepUp             = 0x32,
    GpCommandIdStepDown           = 0x33,
    GpCommandIdLevelControlStop   = 0x34,
    GpCommandIdMoveUpWithOnOff    = 0x35,
    GpCommandIdMoveDownWithOnOff  = 0x36,
    GpCommandIdStepUpWithOnOff    = 0x37,
    GpCommandIdStepDownWithOnOff  = 0x38,

    GpCommandIdMoveHueUp          = 0x41,
    GpCommandIdMoveHueDown        = 0x42,
    GpCommandIdStepHueUp          = 0x43,
    GpCommandIdStepHueDown        = 0x44,
    GpCommandIdMoveSaturationUp   = 0x46,
    GpCommandIdMoveSaturationDown = 0x47,
    GpCommandIdStepSaturationUp   = 0x48,
    GpCommandIdStepSaturationDown = 0x49,
    GpCommandIdMoveColor          = 0x4a,
    GpCommandIdStepColor          = 0x4b,

    GpCommandIdPress1Of1          = 0x60,
    GpCommandIdRelease1Of1        = 0x61,
    GpCommandIdPress1Of2          = 0x62,
    GpCommandIdRelease1Of2        = 0x63,
    GpCommandIdPress2Of2          = 0x64,
    GpCommandIdRelease2Of2        = 0x65,
    GpCommandIdShortPress1Of1     = 0x66,
    GpCommandIdShortPress1Of2     = 0x67,
    GpCommandIdShortPress2Of2     = 0x68,
    // 0x69 - 0x6f reserved
    // 0x70 - 0x9f reserved
    GpCommandIdAttributeReporting                = 0xa0,
    GpCommandIdManufacturerAttributeReporting    = 0xa1,
    GpCommandIdMultiClusterReporting             = 0xa2,
    GpCommandIdManufacturerMultiClusterReporting = 0xa3,
    GpCommandIdRequestAttributes                 = 0xa4,
    GpCommandIdReadAttributesResponse            = 0xa5,
    // 0xa6 - 0xae reserved
    GpCommandIdAnyGPGSensorCommand               = 0xaf, // (0xa0 - 0xa3)
    // 0xb0 - 0xdf reserved
    GpCommandIdCommissioning                     = 0xe0,
    GpCommandIdDecommissioning                   = 0xe1,
    GpCommandIdSuccess                           = 0xe2,
    GpCommandIdChannelRequest                    = 0xe3,
    // 0xe4 - 0xef reserved
    GpCommandIdCommissioningReply                = 0xf0,
    GpCommandIdWriteAttributes                   = 0xf1,
    GpCommandIdReadAttributes                    = 0xf2,
    GpCommandIdChannelConfiguration              = 0xf3,
    // 0xf4 - 0xff reserved for other commands sent to the GPD
};

enum GppCommandIds
{
    GppCommandIdNotification = 0x00,
    GppCommandIdCommissioningNotification = 0x04
};

enum GppGpdLqi
{
    GppGpdLqiPoor      = 0x00,
    GppGpdLqiModerate  = 0x01,
    GppGpdLqiHigh      = 0x02,
    GppGpdLqiExcellent = 0x03
};

union GPCommissioningOptions
{
    struct Bits
    {
        unsigned int macSequenceNumerCapability : 1;
        unsigned int rxOnCapability : 1;
        unsigned int reserved : 2;
        unsigned int panIdRequest : 1;
        unsigned int gpSecurityKeyRequest : 1;
        unsigned int fixedLocation : 1;
        unsigned int extOptionsField : 1;
    } bits;
    quint8 byte;
};

union GpExtCommissioningOptions
{
    struct Bits
    {
        unsigned int securityLevelCapabilities : 2;
        unsigned int keyType : 3;
        unsigned int gpdKeyPresent : 1;
        unsigned int gpdKeyEncryption : 1;
        unsigned int gpdOutgoingCounterPresent : 1;
    } bits;
    quint8 byte;
};

class GpDataIndicationPrivate;

/*!
    \ingroup gp
    \class GpDataIndication
    \brief GPDE-DATA.indication primitive.
 */
class DECONZ_DLLSPEC GpDataIndication
{
public:
    /*! Constructor. */
    GpDataIndication();
    /*! Copy constructor. */
    GpDataIndication(const GpDataIndication &other);
    /*! Copy constructor. */
    GpDataIndication &operator=(const GpDataIndication &other);
    /*! Deconstructor. */
    ~GpDataIndication();

    /*! Stream starts at msdu payload after MAC header. */
    bool readFromStream(QDataStream &stream);

    /*! Stream starts at msdu payload after MAC header. */
    bool readFromStreamGpNotification(QDataStream &stream);

    /*! Returns the GPD SrcID. */
    quint32 gpdSrcId() const;
    /*! Returns the GPD CommandID. */
    quint8 gpdCommandId() const;
    /*! Returns the security frame counter. */
    quint32 frameCounter() const;
    /*! Returns the GPD payload. */
    const QByteArray &payload() const;

    /*! Proxy (GPP) short address.
        \returns The GPP short address, or 0x0000 if this isn't a proxy relayed command.
        \since v2.8.0
     */
     quint16 gppShortAddress() const;

     /*! GPP-GPD link info as seen by the GPP.
        \returns The link info, or 0x00 if this isn't a proxy relayed command.
        \since v2.8.0
     */
     quint8 gppGpdLink() const;
     /*! GPP-GPD RSSI value.
        \returns The capped RSSI value [+8, -109] dBM.
        \since v2.8.0
     */
     qint8 gppRssi() const;
     /*! GPP-GPD Link Quality Indicator (LQI).
        \returns The coarse LQI value 0-3.
        \since v2.8.0
     */
     GppGpdLqi gppLqi() const;

private:
    GpDataIndicationPrivate *d_ptr;
    Q_DECLARE_PRIVATE(GpDataIndication)
};

} // namespace deCONZ

Q_DECLARE_METATYPE(deCONZ::GpDataIndication)

#endif // DECONZ_GREEN_POWER_H
