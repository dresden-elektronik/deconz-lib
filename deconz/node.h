#ifndef DECONZ_NODE_H
#define DECONZ_NODE_H

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
#include <deconz/binding_table.h>
#include <deconz/zdp_descriptors.h>

namespace deCONZ
{

class DECONZ_DLLSPEC NodeNeighbor
{
public:
    NodeNeighbor() : m_lqi(0) {}
    NodeNeighbor(const Address &addr, quint8 lqi): m_address(addr), m_lqi(lqi) { }
    const Address &address() const { return m_address; }
    quint8 lqi() const { return m_lqi; }

private:
    Address m_address;
    quint8 m_lqi;
};

inline uint SR_HashUuid(const QString &uuid)
{
    return qHash(uuid);
}

class DECONZ_DLLSPEC SourceRoute
{
public:
    enum Constants { MaxHops = 9 };
    enum State { StateIdle, StateWorking, StateSleep };

    SourceRoute(const QString &uuid, const int order, const std::vector<Address> &hops) :
        m_uuid(uuid),
        m_order(order),
        m_hops(hops)
       {
           m_srHash = qHash(uuid);
       }

    bool operator==(const SourceRoute &other) const
    {
        bool eqLqi = true;
        for (size_t i = 0; i < MaxHops && i < m_hops.size(); i++)
        {
            if (m_hopLqi[i] != other.m_hopLqi[i])
            {
                eqLqi = false;
                break;
            }
        }
        return m_srHash == other.uuidHash() &&
                m_txOk == other.txOk() &&
                m_errors == other.errors() &&
                m_hops == other.hops() &&
                eqLqi;
    }

    bool operator!=(const SourceRoute &other) const
    {
        return !(*this == other);
    }

    bool isValid() const { return m_srHash != 0 && !m_hops.empty(); }
    bool isOperational() const;
    const QString &uuid() const { return m_uuid; }
    uint uuidHash() const { return m_srHash; }
    //! The last hop in the list is the destination node.
    const std::vector<Address> &hops() const { return m_hops; }
    /*! A lower order means a higher priority for the source route. */
    int order() const { return m_order; }
    void addHop(const deCONZ::Address &hop, quint8 lqi);
    bool hasHop(const deCONZ::Address &hop) const;
    void updateHopAddress(const deCONZ::Address &hop);
    size_t errors() const { return m_errors; }
    void incrementErrors();
    size_t txOk() const { return m_txOk; }
    void incrementTxOk();
    State state() const { return m_state; }
    void setState(State state) { m_state = state; }
    bool needSave() const { return m_needSave; }
    void saved() { m_needSave = false; }

    quint8 m_hopLqi[MaxHops] = { };

private:
    bool m_needSave = false;
    State m_state = StateIdle;
    QString m_uuid;
    int m_order = 0;
    size_t m_txOk = 0;
    size_t m_errors = 0;
    uint m_srHash = 0;
    std::vector<Address> m_hops;
};

class NodePrivate;

/*!
    \ingroup aps
    \class Node
    \brief Represents a ZigBee node with all its descriptors and clusters.
 */
class DECONZ_DLLSPEC Node
{
public:
    /*! Constructor. */
    Node();
    /*! Copy constructor. */
    Node(const Node &other);
    /*! Assignment operator. */
    Node& operator=(const Node &other);
    /*! Deconstructor. */
    virtual ~Node();
    /*! Returns the current node state. */
    virtual CommonState state() const = 0;
    /*! Returns the modifyable node address. */
    deCONZ::Address &address();
    /*! Returns the const node address. */
    const deCONZ::Address &address() const;
    /*! Returns true if the node is a coordinator. */
    bool isCoordinator() const;
    /*! Returns true if the node is a router. */
    bool isRouter() const;
    /*! Returns true if the node is a end-device. */
    bool isEndDevice() const;
    /*! Returns true if the node not reachable. */
    bool isZombie() const;
    /*! Sets the reachable state of the node. */
    void setIsZombie(bool isZombie);
    /*! Returns the user descriptor (usually the nodes name). */
    const QString &userDescriptor() const;
    /*! Sets the user descriptor.
        \param userDescriptor a string with a max length of 16
     */
    void setUserDescriptor(const QString &userDescriptor);
    /*! Returns the device type as string which might be Coordinator, Router or End device. */
    QString deviceTypeString();
    /*! Returns all known active endpoint numbers. */
    const std::vector<uint8_t> &endpoints() const;
    /*! Sets the active endpoint numbers. */
    void setActiveEndpoints(const std::vector<uint8_t> &ep);
    /*! Returns a simple descriptor for a \p endpoint.
        \param endpoint shall be between 1-254
        \returns a pointer to the internal SimpleDescriptor if found
        \retval 0 if no simple descriptor is available for \p endpoint
     */
    SimpleDescriptor *getSimpleDescriptor(uint8_t endpoint);
    /*! Adds or updates a simple descriptor.
        \param descr a simple descriptor with endpoint between 1-254
        \return true on updated added, false if nothing changed
     */
    bool setSimpleDescriptor(const SimpleDescriptor &descr);
    /*! Returns the modifyable list of a simple descriptors. */
    std::vector<SimpleDescriptor> &simpleDescriptors();
    /*! Returns the const list of a simple descriptors. */
    const std::vector<SimpleDescriptor> &simpleDescriptors() const;
    /*! Copy the simple descriptor specified by \p endpoint from internal storage to \p descr.
        \param endpoint shall be between 1-254
        \param descr pointer to a user simple descriptor
        \retval 0 on success
        \retval -1 if not found
        \retval -1 descr is 0
     */
    int copySimpleDescriptor(uint8_t endpoint, SimpleDescriptor *descr) const;
    /*! Returns the node descriptor. */
    const NodeDescriptor & nodeDescriptor() const;
    /*! Sets the node descriptor.
        \param descr a node descriptor
     */
    void setNodeDescriptor(const NodeDescriptor &descr);
    /*! Returns the power descriptor. */
    const PowerDescriptor &powerDescriptor() const;
    /*! Sets the power descriptor.
        \param descr a power descriptor
     */
    void setPowerDescriptor(const PowerDescriptor &descr);
    /*! Returns the mac capabilities. */
    const MacCapabilities &macCapabilities() const;
    /*! Sets the mac capabilities.
        \param cap or combined value of deCONZ::MacCapability
     */
    void setMacCapabilities(MacCapabilities cap);
    /*! Resets internal fields and state. */
    void resetAll();
    /*! Returns all neighbors of the node */
    virtual const std::vector<NodeNeighbor> &neighbors() const = 0;
    /*! Returns all source routes of the node */
    const std::vector<SourceRoute> &sourceRoutes() const;
    /*! Add or update a source route.
        If a source route with the same order or uuid exists, it will be replaced.
        \param sourceRoute
        \retval 0 on success, a new source route was added
        \retval 1 on success, existing source route was updated
        \retval -1 if the source route isn't valid
     */
    int addSourceRoute(const SourceRoute &sourceRoute);
    /*! Update a source route.
        If a source route with the same order or uuid exists, it will be replaced.
        \param sourceRoute
        \retval true when existing source route was updated
        \retval false if the source route wasn't found
     */
    bool updateSourceRoute(const SourceRoute &sourceRoute);
    /*! Removes a source route specified by \p srHash.
        \retval 0 if the source route was removed
        \retval -1 if there was no source route with the given uuid
    */
    int removeSourceRoute(uint srHash);
    /*! Returns the binding table of a node. */
    virtual const BindingTable &bindingTable() const = 0;

    void pushEdScan(int ed);
    int edScanValue() const;
    bool needRedraw() const;
    void setNeedRedraw(bool redraw);

protected:
    NodePrivate *d_ptr = nullptr;
    Q_DECLARE_PRIVATE(Node)
};

}

#endif // DECONZ_NODE_H
