#ifndef DECONZ_APS_CONTROLLER_H
#define DECONZ_APS_CONTROLLER_H

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <QObject>
#include <deconz/types.h>
#include <deconz/aps.h>
#include <deconz/binding_table.h>

#define FW_ONLY_AVR_BOOTLOADER 1
#define FW_ONLY_R21_BOOTLOADER 2

namespace deCONZ
{

/*! Values for ParamFirmwareUpdateActive.
 */
enum FirmwareUpdateState
{
    FirmwareUpdateIdle,
    FirmwareUpdateReadyToStart,
    FirmwareUpdateRunning
};

/*! Parameters of type uint8_t.
 */
enum U8Parameter
{
    ParamCurrentChannel, //!< the current operation channel
    ParamDeviceType,     //!< deCONZ::Coordinator or deCONZ::Router
    ParamSecurityMode,   //!< one of the deCONZ::SecurityMode values
    ParamPermitJoin,     //!< permit join duration
    ParamOtauActive,     //!< otau server: 1 = activated, 0 = deactivated
    ParamAutoPollingActive,  //!< Automatic polling of ZDP / REST Plugin: 1 = activated, 0 = deactivated
    ParamNetworkUpdateId, //!< network update id
    ParamFirmwareUpdateActive, //!< values from FirmwareUpdateState enum
    ParamDeviceConnected, //!< device connected state: 1 = connected, 0 = disconnected
    ParamApsAck,           //!< aps ack enabled/disabled
    ParamPredefinedPanId,  //!< Predefined PanId enabled/disabled
    ParamCustomMacAddress,      //!< custom Mac Address enabled/disabled
    ParamStaticNwkAddress       //!< static NwkAddress enabled/disabled
};

/*! Parameters of type uint16_t.
 */
enum U16Parameter
{
    ParamPANID,    //!< the short PANID
    ParamNwkAddress, //!< the network address of the device
    ParamHttpPort,    //!< the HTTP server port
    ParamHttpsPort    //!< the HTTPS server port
};

/*! Parameters of type uint32_t.
 */
enum U32Parameter
{
    ParamChannelMask,      //!< the 32-bit bitmap with enabled channels set as bit
    ParamFirmwareVersion,  //!< the firmware version of the connected device, special value 1 means bootloader but no firmware
    ParamFrameCounter      //!< the current outgoing frame counter
};

/*! Parameters of type uint64_t.
 */
enum U64Parameter
{
    ParamApsUseExtendedPANID, //!< the extended PANID to use during join
    ParamExtendedPANID,       //!< the current extended PANID
    ParamMacAddress,          //!< the MAC address of the device
    ParamTrustCenterAddress   //!< the MAC address of the trust center
};

/*! Parameters of type QString.
 */
enum StringParameter
{
    ParamDeviceName, //!< name of the connetced device (RaspBee, ConBee, ...)
    ParamDevicePath, //!< /dev/ttyUSB0, /dev/ttyAMA0, COM49, ...
    ParamHttpRoot    //!< the http server root directory
};

/*! Parameters of type QByteArray.
 */
enum ArrayParameter
{
    ParamNetworkKey,          //!< the 128-bit network key
    ParamTrustCenterLinkKey,  //!< the 128-bit trust center link key
    ParamSecurityMaterial0    //!< the 256-bit security material set 0
};

/*! Parameters of type QVariantMap.
 */
enum VariantMapParameter
{
    ParamHAEndpoint,
    ParamLinkKey
};

class Node;
class NodeEvent;
class SourceRoute;

/*!
    \ingroup aps
    \class ApsController
    \brief Provides APSDE-DATA service and access to node cache.
 */
class DECONZ_DLLSPEC ApsController: public QObject
{
    Q_OBJECT

public:
    /*! Constructor. */
    ApsController(QObject *parent);
    /*! Deconstructor. */
    virtual ~ApsController();
    /*! Get the singleton instance of the ApsController. */
    static ApsController *instance();
    /*! Returns the current network state. */
    virtual State networkState() = 0;
    /*! Sets the current network state.

        \param state either NotInNetwork or InNetwork
        \retval Success the request is enqueued and will be processed
        \retval ErrorNotConnected if not connected to device
     */
    virtual int setNetworkState(State state) = 0;
    /*! Sets the permit join duration.

        \param duration 0..255 (in seconds)
        \retval Success the request is enqueued and will be processed
        \retval ErrorNotConnected if not connected to a network
     */
    virtual int setPermitJoin(uint8_t duration) = 0;
    /*! Returns number of enqueued APS requests. */
    virtual int apsQueueSize() = 0;
    /*!
        Send a APSDE-DATA.request to the network.

        Multible requests could be send in parallel.
        It is guaranteed that for each request a apsdeDataConfirm() signal is emitted
        after the request is processed.

        The confirmation might take from 5 milliseconds up to serval seconds depending on
        if a new route must be found or the destination is not available.

        To match the request to the confirmation compare their ids,
              ApsDataRequest::id() == ApsDataConfirm::id().

        \retval Success the request is enqueued and will be processed
        \retval ErrorNotConnected if not connected to a network
        \retval ErrorQueueIsFull request queue is full
        \retval ErrorNodeIsZombie destionation node is zombie node, only ZDP requests are allowed
     */
    virtual int apsdeDataRequest(const ApsDataRequest &req) = 0;
    /*! Fills in missing network or extended IEEE address information.
     *  \param addr at least nwk() or ext() must be set
     *  \retval Success on success
     *  \retval ErrorNotFound if the missing address part was not available
     */
    virtual int resolveAddress(Address &addr) = 0;
    /*! Iterates through the internal node cache.
        \code {cpp}
        int i = 0;
        const deCONZ::Node *node;
        deCONZ::ApsController *ctrl = deCONZ::ApsController::instance();

        while (ctrl->getNode(i, &node) == 0)
        {
          // do something with *node
          i++;
        }
        \endcode

        The node with the index 0 is always the own node.

        \param index a index >= 0
        \param node a pointer to a user provides deCONZ::Node pointer
        \retval 0 if \p node points to a valid deCONZ::Node
        \retval -1 if no node is available for \p index
     */
    virtual int getNode(int index, const Node **node) = 0;
    /*! TODO: return object instead of pointer ... */
    //virtual const Node &getNode(int index) = 0;
    /*! Updates the data of a node in the internal node cache.
        \param node a node which must have a extended IEEE address
     */
    virtual bool updateNode(const Node &node) = 0;
    /*! Returns a 8-bit parameter.
        \param parameter - the parameter identifier
        \return the parameter value
     */
    virtual uint8_t getParameter(U8Parameter parameter) = 0;
    /*! Sets a 8-bit parameter.
        \param parameter - the parameter identifier
        \param value - the parameter value
        \return true on success
     */
    virtual bool setParameter(U8Parameter parameter, uint8_t value) = 0;
    /*! Sets a 16-bit parameter.
        \param parameter - the parameter identifier
        \param value - the parameter value
        \return true on success
     */
    virtual bool setParameter(U16Parameter parameter, uint16_t value) = 0;
    /*! Sets a 32-bit parameter.
        \param parameter - the parameter identifier
        \param value - the parameter value
        \return true on success
     */
    virtual bool setParameter(U32Parameter parameter, uint32_t value) = 0;
    /*! Sets a 64-bit parameter.
        \param parameter - the parameter identifier
        \param value - the parameter value
        \return true on success
     */
    virtual bool setParameter(U64Parameter parameter, uint64_t value) = 0;
    /*! Sets a byte array parameter.
        \param parameter - the parameter identifier
        \param value - the parameter value
        \return true on success
     */
    virtual bool setParameter(ArrayParameter parameter, QByteArray value) = 0;
    /*! Sets a VariantMap parameter.
        \param parameter - the parameter identifier
        \param value - the parameter value
        \return true on success
     */
    virtual bool setParameter(VariantMapParameter parameter, QVariantMap value) = 0;
    /*! Sets a string parameter.
        \param parameter - the parameter identifier
        \param value - the parameter value
        \return true on success
     */
    virtual bool setParameter(StringParameter parameter, const QString &value) = 0;
    /*! Returns a 16-bit parameter.
        \param parameter - the parameter identifier
        \return the parameter value
     */
    virtual uint16_t getParameter(U16Parameter parameter) = 0;
    /*! Returns a 32-bit parameter.
        \param parameter - the parameter identifier
        \return the parameter value
     */
    virtual uint32_t getParameter(U32Parameter parameter) = 0;
    /*! Returns a 64-bit parameter.
        \param parameter - the parameter identifier
        \return the parameter value
     */
    virtual uint64_t getParameter(U64Parameter parameter) = 0;
    /*! Returns a string parameter.
        \param parameter - the parameter identifier
        \return the parameter value
     */
    virtual QString getParameter(StringParameter parameter) = 0;
    /*! Returns a byte array parameter.
        \param parameter - the parameter identifier
        \return the parameter value
     */
    virtual QByteArray getParameter(ArrayParameter parameter) = 0;
    /*! Returns a variant map parameter.
        \param parameter - the parameter identifier
        \param index (optional)
        \return the parameter value
     */
    virtual QVariantMap getParameter(VariantMapParameter parameter, int index) = 0;

    /*! Activate a source route, also called when restored from database.
        \param sourceRoute - the source route
        \since 2.05.81
     */
    virtual void activateSourceRoute(const SourceRoute &sourceRoute) = 0;

    /*! Adds a binding, if not already exists, to a nodes binding table.
        \param binding - the binding, the node is determinded by Binding::srcAddress()
        \since 2.05.86
     */
    virtual void addBinding(const deCONZ::Binding &binding) = 0;

    /*! Removes a binding to a nodes binding table.
        \param binding - the binding, the node is determinded by Binding::srcAddress()
        \since 2.05.86
     */
    virtual void removeBinding(const deCONZ::Binding &binding) = 0;

    /*! Alloctes a new APS request id.
        \since 2.19.0
        \return new non zero request id 1-255
     */
    virtual uint8_t nextRequestId() = 0;

Q_SIGNALS:
    /*! Is emitted on the reception of a APSDE-DATA.confirm primitive.

        \note The id of the confirmation equals the id from the former request.
        \sa  apsdeDataRequest()
     */
    void apsdeDataConfirm(const deCONZ::ApsDataConfirm &);

    /*! Is emitted on the reception of a APSDE-DATA.indication primitive.

        A indication might be received at any time and is not necessarily
        releated to a former request.

        No filtering is done by the application any indication which is
        received will be forwarded.
     */
    void apsdeDataIndication(const deCONZ::ApsDataIndication &);

    /*! Is emitted when a APSDE-DATA.request primitive was added to the queue.

        \since 2.05.79
     */
    void apsdeDataRequestEnqueued(const deCONZ::ApsDataRequest &);

    /*! Is emitted on changes in the nodecache or user interactions. */
    void nodeEvent(const deCONZ::NodeEvent&);

    /*! Is emitted when the configuration changed via deCONZ network settings or any other means.
        \since 2.05.44
     */
    void configurationChanged();

    /*! Is emitted when the user requested network state change. true = connect; false = disconnect.
        \since 2.05.70
     */
    void networkStateChangeRequest(bool);

    /*! Is emitted when a new source route is created.
        \since 2.05.81
     */
     void sourceRouteCreated(const deCONZ::SourceRoute&);

     /*! Is emitted when a source route was changed.
         \since 2.05.81
      */
     void sourceRouteChanged(const deCONZ::SourceRoute&);

     /*! Is emitted when a source route is deleted.
        \since 2.05.81
     */
     void sourceRouteDeleted(const QString &uuid);

    /*! Is emitted when the controller has restored all nodes from database.
        \since 2.05.81
     */
     void nodesRestored();
};

} // namespace deCONZ

#endif // DECONZ_APS_CONTROLLER_H
