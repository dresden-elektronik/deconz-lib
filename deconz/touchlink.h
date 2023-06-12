#ifndef DECONZ_TOUCHLINK_H
#define DECONZ_TOUCHLINK_H

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
#include <deconz/aps.h>

namespace deCONZ {

/*! Status codes used by the touchlink module. */
enum TouchlinkStatus
{
    TouchlinkSuccess  = 0x00, //!< Success
    TouchlinkFailed   = 0x01, //!< Failed
    TouchlinkBusy     = 0x02  //!< Busy
};

class TouchlinkRequestPrivate;

/*!
    \ingroup touchlink
    \class TouchlinkRequest
    \brief Base class for all touchlink requests.
 */
class DECONZ_DLLSPEC TouchlinkRequest
{
public:
    /*! Constructor. */
    TouchlinkRequest();
    /*! Copy constructor. */
    TouchlinkRequest(const TouchlinkRequest &other);
    /*! Copy assignment operator. */
    TouchlinkRequest& operator=(const TouchlinkRequest &other);
    /*! Deconstructor. */
    ~TouchlinkRequest();
    /*! Returns the modifiable destination address. */
    Address &dstAddress();
    /*! Returns the const destination address. */
    const Address &dstAddress() const;
    /*! Returns the destination address mode. */
    ApsAddressMode dstAddressMode() const;
    /*! Sets the destination address mode.
        \param mode the address mode
     */
    void setDstAddressMode(ApsAddressMode mode);
    /*! Returns the logical channel. */
    uint8_t channel() const;
    /*! Sets the logical channel.
        \param channel channel number 11-26
     */
    void setChannel(uint8_t channel);
    /*! Returns the PANID. */
    uint16_t panId() const;
    /*! Sets the PANID.
        \param panId the PANID
     */
    void setPanId(uint16_t panId);
    /*! Returns the profile identifier. */
    uint16_t profileId() const;
    /*! Sets the profile identifier.
        \param profileId the profile identifier
     */
    void setProfileId(uint16_t profileId);
    /*! Returns the cluster identifier. */
    uint16_t clusterId() const;
    /*! Sets the cluster identifier.
        \param clusterId the cluster identifier
     */
    void setClusterId(uint16_t clusterId);
    /*! Writes the request to the stream.
        \code{.cpp}
          deCONZ::TouchlinkRequest req;
          // ... setup request
          QByteArray arr; // buffer into which the plain request will be written
          QDataStream stream(&arr, QIODevice::WriteOnly);
          stream.setByteOrder(QDataStream::LittleEndian); // everything in ZigBee is little endian

          // serialize request to buffer
          req.writeToStream(stream);
        \endcode
     */
    bool writeToStream(QDataStream &stream) const;
    /*! Returns the const ASDU payload. */
    const QByteArray &asdu() const;
    /*! Returns the writeable ASDU payload. */
    QByteArray &asdu();
    /*! Sets the ASDU payload. */
    void setAsdu(const QByteArray &asdu);
    /* \cond INTERNAL_SYMBOLS */
    /*! Returns the interpan transaction identifier. */
    uint32_t transactionId() const;
    /*! Sets the interpan transaction identifier for the request.
        \param transactionId the interpan transaction identifier
     */
    void setTransactionId(uint32_t transactionId);
    /* \endcond */
private:
    TouchlinkRequestPrivate *d_ptr;
    Q_DECLARE_PRIVATE(TouchlinkRequest)
};

} // namespace deCONZ

Q_DECLARE_METATYPE(deCONZ::TouchlinkRequest)

#endif // DECONZ_TOUCHLINK_H
