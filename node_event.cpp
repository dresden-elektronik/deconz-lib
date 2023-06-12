/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include "deconz/node_event.h"

namespace deCONZ {

class NodeEventPrivate
{
public:
    NodeEventPrivate();
    const Node *node;
    NodeEvent::Event event;
    uint8_t endpoint;
    uint16_t profileId;
    uint16_t clusterId;
    std::vector<uint16_t> attributeIds;
};

NodeEventPrivate::NodeEventPrivate() :
    node(0),
    event(NodeEvent::NodeAdded),
    endpoint(0),
    profileId(0),
    clusterId(0)
{

}

NodeEvent::NodeEvent() :
    d_ptr(new NodeEventPrivate)
{
}

NodeEvent::NodeEvent(NodeEvent::Event event, const Node *node, uint8_t endpoint, uint16_t profileId, uint16_t clusterId) :
    d_ptr(new NodeEventPrivate)
{
    Q_D(NodeEvent);
    d->event = event;
    d->node = node;
    d->endpoint = endpoint;
    d->profileId = profileId;
    d->clusterId = clusterId;
}

NodeEvent::NodeEvent(NodeEvent::Event event, const Node *node, const ApsDataIndication &ind) :
       d_ptr(new NodeEventPrivate)
{
    Q_D(NodeEvent);
    d->event = event;
    d->node = node;
    d->endpoint = ind.srcEndpoint();
    d->profileId = ind.profileId();
    d->clusterId = ind.clusterId();
}

NodeEvent::NodeEvent(const NodeEvent &other) :
    d_ptr(new NodeEventPrivate(*other.d_ptr))
{
}

NodeEvent &NodeEvent::operator= (const NodeEvent &other)
{
    // Self assignment?
    if (this == &other)
    {
        return *this;
    }

    *this->d_ptr = *other.d_ptr;
    return *this;
}

NodeEvent::~NodeEvent()
{
    delete d_ptr;
    d_ptr = 0;
}

const Node *NodeEvent::node() const
{
    Q_D(const NodeEvent);
    return d->node;
}

NodeEvent::Event NodeEvent::event() const
{
    Q_D(const NodeEvent);
    return d->event;
}

uint8_t NodeEvent::endpoint() const
{
    Q_D(const NodeEvent);
    return d->endpoint;
}

uint16_t NodeEvent::profileId() const
{
    Q_D(const NodeEvent);
    return d->profileId;
}

uint16_t NodeEvent::clusterId() const
{
    Q_D(const NodeEvent);
    return d->clusterId;
}

const std::vector<uint16_t> &NodeEvent::attributeIds() const
{
    Q_D(const NodeEvent);
    return d->attributeIds;
}

void NodeEvent::addAttributeId(uint16_t id)
{
    Q_D(NodeEvent);
    return d->attributeIds.push_back(id);
}

}
