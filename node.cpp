/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include "deconz/node.h"
#include "deconz/dbg_trace.h"
#include "node_private.h"

namespace deCONZ {

Node::Node() :
    d_ptr(new NodePrivate)
{

}

Node::Node(const Node &other) :
    d_ptr(new NodePrivate(*other.d_ptr))
{
}

Node &Node::operator=(const Node &other)
{
    // Self assignment?
    if (this==&other)
    {
        return *this;
    }

    if (d_ptr && other.d_ptr)
    {
        *d_ptr = *other.d_ptr;
    }
    return *this;
}

Node::~Node()
{
    delete d_ptr;
    d_ptr = 0;
}

Address &Node::address()
{
    return d_ptr->address;
}

const Address &Node::address() const
{
    return d_ptr->address;
}

bool Node::isCoordinator() const
{
     if (address().hasNwk() && (address().nwk() == 0x0000) && (d_ptr->m_macCapa & MacDeviceIsFFD))
     {
         return true;
     }

     return false;
}

bool Node::isRouter() const
{
    if (address().hasNwk() && (address().nwk() != 0x0000) && (d_ptr->m_macCapa & MacDeviceIsFFD))
    {
        return true;
    }

    return false;
}

bool Node::isEndDevice() const
{
    return (!isCoordinator() && !isRouter());
}

bool Node::isZombie() const
{
    return d_ptr->isZombie;
}

void Node::setIsZombie(bool isZombie)
{
    if (d_ptr->isZombie != isZombie)
    {
        d_ptr->isZombie = isZombie;
        d_ptr->needRedraw = true;
    }
}

const QString &Node::userDescriptor() const
{
    return d_ptr->userDescr;
}

void Node::setUserDescriptor(const QString &userDescriptor)
{
    if (d_ptr->userDescr != userDescriptor)
    {
        d_ptr->userDescr = userDescriptor;
        d_ptr->needRedraw = true;
    }
}

QString Node::deviceTypeString()
{
    if (isRouter())
        return "Router";

    if (isCoordinator())
        return "Coordinator";

    if (isEndDevice())
        return "End device";

    return "Unknown";
}

/*!
    Returns a list of all active application endpoints
    on the node. The list might be empty if the ZDP active endpoint
    request is not done for the node yet.
 */
const std::vector<uint8_t> &Node::endpoints() const
{
    return d_ptr->m_endpoints;
}

void Node::setActiveEndpoints(const std::vector<uint8_t> &ep)
{
    d_ptr->m_endpoints = ep;
    d_ptr->m_fetchEndpoints.clear();

    for (quint8 e : ep)
    {
        bool found = false;
        for (const deCONZ::SimpleDescriptor &sd : simpleDescriptors())
        {
            if (sd.endpoint() == e)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            d_ptr->m_fetchEndpoints.push_back(e);
            d_ptr->needRedraw = true;
        }
    }
}

// TODO: replace this function by a safer one
SimpleDescriptor *Node::getSimpleDescriptor(uint8_t endpoint)
{
    for (auto &sd : d_ptr->m_simpleDescriptors)
    {
        if (sd.endpoint() == endpoint)
            return &sd;
    }

    return 0;
}

bool endpointLessThan(const SimpleDescriptor &a, const SimpleDescriptor &b)
{
    return a.endpoint() < b.endpoint();
}

bool Node::setSimpleDescriptor(const SimpleDescriptor &descr)
{
    for (auto &sd : d_ptr->m_simpleDescriptors)
    {
        if (&sd == &descr)
        {
            return true;
        }

        if (sd.endpoint() == descr.endpoint())
        {
            if (sd.inClusters().size() == descr.inClusters().size() &&
                sd.outClusters().size() == descr.outClusters().size())
            {
                // assume same
                return false;
            }
            // copy old data in new descriptor if clusters are present on both
            deCONZ::SimpleDescriptor sd2 = descr;
            for (deCONZ::ZclCluster &cl : sd2.inClusters())
            {
                deCONZ::ZclCluster *old = sd.cluster(cl.id(), deCONZ::ServerCluster);
                if (old)
                {
                    DBG_Printf(DBG_INFO_L2, "copy %s cluster data\n", qPrintable(cl.name()));
                    cl = *old;
                }
            }

            sd = sd2;
            d_ptr->needRedraw = true;
            return true;
        }
    }

    if (std::find(d_ptr->m_endpoints.begin(),
                  d_ptr->m_endpoints.end(),
                  descr.endpoint()) == d_ptr->m_endpoints.end())
    {
        d_ptr->m_endpoints.push_back(descr.endpoint());
    }

    d_ptr->m_simpleDescriptors.push_back(descr);
    std::sort(d_ptr->m_simpleDescriptors.begin(), d_ptr->m_simpleDescriptors.end(), endpointLessThan);
    d_ptr->needRedraw = true;
    return true;
}

std::vector<SimpleDescriptor> &Node::simpleDescriptors()
{
    return d_ptr->m_simpleDescriptors;
}

const std::vector<SimpleDescriptor> &Node::simpleDescriptors() const
{
    return d_ptr->m_simpleDescriptors;
}

int Node::copySimpleDescriptor(uint8_t endpoint, SimpleDescriptor *descr) const
{
    if (descr)
    {
        auto i = d_ptr->m_simpleDescriptors.cbegin();
        const auto end = d_ptr->m_simpleDescriptors.cend();

        for (; i != end; ++i)
        {
            if (i->endpoint() == endpoint)
            {
                *descr = *i;
                return 0;
            }
        }
    }

    return -1;
}

void Node::resetAll()
{
    d_ptr->isZombie = false;
    d_ptr->userDescr.clear();
    d_ptr->m_simpleDescriptors.clear();
}

const std::vector<SourceRoute> &Node::sourceRoutes() const
{
    return d_ptr->m_sourceRoutes;
}

int Node::addSourceRoute(const SourceRoute &sourceRoute)
{
    if (!sourceRoute.isValid())
    {
        return -1;
    }

    for (auto &sr : d_ptr->m_sourceRoutes)
    {
        if (sr.uuidHash() == sourceRoute.uuidHash())
        {
            if (sr != sourceRoute)
            {
                sr = sourceRoute;
                return 1; // updated
            }
            return 2; // nop
        }
    }

    d_ptr->m_sourceRoutes.push_back(sourceRoute);
    return 0; // new item added
}

bool Node::updateSourceRoute(const SourceRoute &sourceRoute)
{
    for (auto &sr : d_ptr->m_sourceRoutes)
    {
        if (sr.uuidHash() == sourceRoute.uuidHash())
        {
            sr = sourceRoute;
            return true; // updated
        }
    }

    return false;
}

int Node::removeSourceRoute(uint srHash)
{
    auto i = std::find_if(d_ptr->m_sourceRoutes.begin(), d_ptr->m_sourceRoutes.end(), [srHash](const SourceRoute &sr)
    {
        return sr.uuidHash() == srHash;
    });

    if (i != d_ptr->m_sourceRoutes.end())
    {
        d_ptr->m_sourceRoutes.erase(i);
        return 0;
    }

    return -1;
}

void Node::pushEdScan(int ed)
{
//   if (d_ptr->edIter >= d_ptr->edValues.size())
//   {
//       d_ptr->edIter = 0;
//   }
   d_ptr->edIter = 0;
   d_ptr->edValues[d_ptr->edIter] = ed;

   // d_ptr->edIter++;
}

int Node::edScanValue() const
{
//    unsigned result = 0;
//    unsigned count = 0;

//    for (int8_t ed : d_ptr->edValues)
//    {
//        if (ed)
//        {
//            count++;
//            result += ed;
//        }
//    }

//    if (count)
//    {
//        return result / count;
//    }

    return d_ptr->edValues[0];
}

bool Node::needRedraw() const
{
    return d_ptr->needRedraw;
}

void Node::setNeedRedraw(bool redraw)
{
    d_ptr->needRedraw = redraw;
}

const NodeDescriptor &Node::nodeDescriptor() const
{
    return d_ptr->m_nodeDescr;
}

void Node::setNodeDescriptor(const NodeDescriptor &descr)
{
    d_ptr->m_nodeDescr = descr;
    d_ptr->needRedraw = true;
}

const PowerDescriptor &Node::powerDescriptor() const
{
    return d_ptr->m_powerDescr;
}

void Node::setPowerDescriptor(const PowerDescriptor &descr)
{
    d_ptr->m_powerDescr = descr;
    d_ptr->needRedraw = true;
}

const MacCapabilities &Node::macCapabilities() const
{
    return d_ptr->m_macCapa;
}

void Node::setMacCapabilities(MacCapabilities cap)
{
    if (d_ptr->m_macCapa != cap)
    {
        d_ptr->m_macCapa = cap;
        d_ptr->needRedraw = true;
    }
}

bool SourceRoute::isOperational() const
{
    if (m_state == StateSleep)
    {
        return false;
    }

    for (size_t i = 0; i < m_hops.size(); i++)
    {
        if (m_hopLqi[i] == 0)
        {
            return false;
        }
    }

    return !m_hops.empty();
}

void SourceRoute::addHop(const Address &hop, quint8 lqi)
{
    if (!hasHop(hop) && m_hops.size() < MaxHops)
    {
        m_hopLqi[m_hops.size()] = lqi;
        m_hops.push_back(hop);
    }
}

bool SourceRoute::hasHop(const Address &hop) const
{
    auto i = std::find_if(m_hops.begin(), m_hops.end(), [&hop](const deCONZ::Address &addr)
    {
        return addr.ext() == hop.ext();
    });

    return i != m_hops.end();
}

void SourceRoute::updateHopAddress(const Address &hop)
{
    auto i = std::find_if(m_hops.begin(), m_hops.end(), [&hop](const deCONZ::Address &addr)
    {
        return addr.ext() == hop.ext();
    });

    if (i != m_hops.end())
    {
        i->setNwk(hop.nwk());
    }
}

void SourceRoute::incrementErrors()
{
    if (m_errors < UINT32_MAX)
    {
        m_errors++;
    }

    if (m_errors % 10 == 0 && m_txOk > 0)
    {
        m_txOk >>= 1;
    }

    if (m_txOk == 0 && m_errors > 10)
    {
        m_state = StateSleep;
        m_needSave = false;
    }
}

void SourceRoute::incrementTxOk()
{
    if (m_txOk < UINT32_MAX)
    {
        m_txOk++;
    }

    if (m_state != StateWorking)
    {
        m_state = StateWorking;
    }

    if (m_errors > 0 && m_txOk % 10 == 0)
    {
        m_errors--;
    }

    if (m_errors < (m_txOk / 3) && m_txOk % 50 == 0)
    {
        m_needSave = true;
    }
}


} // namespace deCONZ
