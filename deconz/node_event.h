#ifndef DECONZ_NODE_EVENT_H
#define DECONZ_NODE_EVENT_H

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

namespace deCONZ {

// forward declarations
class Node;
class NodeEventPrivate;

/*!
    \ingroup aps
    \class NodeEvent
    \brief Node events are triggered from the ApsController::nodeEvent() signal to notify about changes in the node cache.
 */
class DECONZ_DLLSPEC NodeEvent
{
public:
    /*! Events which might occur for a node. */
    enum Event
    {
        /*! The user seleced a node in the GUI. */
        NodeSelected,
        /*! A node in the GUI gots deselected. */
        NodeDeselected,
        /*! Context menu of a node in the GUI was requested. */
        NodeContextMenu,
        /*! A new node was added to the nodecache. */
        NodeAdded,
        /*! A node was removed from nodecache. */
        NodeRemoved,
        /*! Child end-device polled for data. */
        NodeMacDataRequest,
        /*! A node reachable state changed.
            \see isZombie()
         */
        NodeZombieChanged,
        /*! The node address was updated. */
        UpdatedNodeAddress,
        /*! The node descriptor was updated. */
        UpdatedNodeDescriptor,
        /*! The power descriptor was updated. */
        UpdatedPowerDescriptor,
        /*! The user descriptor (node name) was updated. */
        UpdatedUserDescriptor,
        /*! The simple descriptor was updated.
            \see endpoint()
         */
        UpdatedSimpleDescriptor,
        /*! Data in a cluster was updated.
            \see endpoint(), profileId(), clusterId()
         */
        UpdatedClusterData,
        /*! Data in a cluster was updated via ZCL read.
            \see endpoint(), profileId(), clusterId()
         */
        UpdatedClusterDataZclRead,
        /*! Data in a cluster was updated via ZCL report.
            \see endpoint(), profileId(), clusterId()
         */
        UpdatedClusterDataZclReport,
        /*! GUI request to edit the DDF file of a device. */
        EditDeviceDDF
    };

    /*! Constructor. */
    NodeEvent();
    /*! Constructor used by controller. */
    NodeEvent(Event event, const Node *node = 0, uint8_t endpoint = 0, uint16_t profileId = 0, uint16_t clusterId = 0);
    /*! Constructor used by controller. */
    NodeEvent(Event event, const Node *node, const ApsDataIndication &ind);
    /*! Copy Constructor. */
    NodeEvent(const NodeEvent &other);
    /*! Copy assignment constructor. */
    NodeEvent &operator= (const NodeEvent &other);
    /*! Deconstructor. */
    ~NodeEvent();
    /*! Returns the node which belongs to the event. */
    const Node *node() const;
    /*! Returns the event type. */
    Event event() const;
    /*! Returns the endpoint related to the event.

        Endpoint is available in following events:
           - UpdatedSimpleDescriptor
           - UpdatedClusterData
     */
    uint8_t endpoint() const;
    /*! Returns the profile identifier related to the event.
        Profile identifier is available in following events:
           - UpdatedClusterData
     */
    uint16_t profileId() const;
    /*! Returns the cluster identifier related to the event.
        Cluster identifier is available in following events:
           - UpdatedClusterData
     */
    uint16_t clusterId() const;

    /*! Returns the attribute identifier related to the event.
        Attribute identifier are available in following events:
           - UpdatedClusterData
     */
    const std::vector<uint16_t> &attributeIds() const;

    /* \cond INTERNAL_SYMBOLS */
    void addAttributeId(uint16_t id);
private:
    NodeEventPrivate *d_ptr;
    Q_DECLARE_PRIVATE(NodeEvent)
};

} // namespace deCONZ

Q_DECLARE_METATYPE(deCONZ::NodeEvent)

#endif // DECONZ_NODE_EVENT_H
