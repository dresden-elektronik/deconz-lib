/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <QDebug>
#include <QDataStream>
#include "deconz/binding_table.h"

namespace deCONZ {

/*!
    Adds the \p binding into the table if it not already exists.
 */
bool BindingTable::add(const Binding &binding)
{
    if (binding.isValid() && !contains(binding))
    {
        m_table.push_back(binding);
        return true;
    }

    return false;
}

/*!
    Removes the \p binding from the table.
 */
bool BindingTable::remove(const Binding &binding)
{
    auto i = std::find(m_table.begin(), m_table.end(), binding);
    if (i != m_table.end())
    {
        m_table.erase(i);
        return true;
    }

    return false;
}

bool BindingTable::contains(const Binding &binding) const
{
    return std::find(m_table.begin(), m_table.end(), binding) != m_table.end();
}

BindingTable::const_iterator BindingTable::const_begin() const
{
    return m_table.begin();
}

BindingTable::const_iterator BindingTable::const_end() const
{
    return m_table.end();
}

BindingTable::const_iterator BindingTable::cbegin() const
{
    return m_table.begin();
}

BindingTable::const_iterator BindingTable::cend() const
{
    return m_table.end();
}

BindingTable::iterator BindingTable::begin()
{
    return m_table.begin();
}

BindingTable::iterator BindingTable::end()
{
    return m_table.end();
}

void BindingTable::clearOldBindings()
{
    int count = 0;

    for (;count < 128;)
    {
        auto i = std::find_if(m_table.begin(), m_table.end(), [this](const auto &bnd)
        {
            return bnd.confirmedTimeRef() < m_responseIndex0TimeRef;
        });

        if (i == m_table.end())
            break;

        m_table.erase(i);
        count++;
    }
}

Binding::Binding(const quint64 src, const quint64 dst, const quint16 clusterId, const quint8 srcEndpoint, const quint8 dstEndpoint) :
    m_srcAddr(src),
    m_dstAddrMode(deCONZ::ApsExtAddress),
    m_cluster(clusterId),
    m_srcEndpoint(srcEndpoint),
    m_dstEndpoint(dstEndpoint)
{
    m_dstAddr.setExt(dst);
}

Binding::Binding(const quint64 src, const quint16 dstGroup, const quint16 clusterId, const quint8 srcEndpoint) :
    m_srcAddr(src),
    m_dstAddrMode(deCONZ::ApsGroupAddress),
    m_cluster(clusterId),
    m_srcEndpoint(srcEndpoint),
    m_dstEndpoint(0) // not present
{
    m_dstAddr.setGroup(dstGroup);
}

bool Binding::readFromStream(QDataStream &stream)
{
    quint8 dstAddrMode;

    stream >> m_srcAddr;
    stream >> m_srcEndpoint;
    stream >> m_cluster;
    stream >> dstAddrMode;

    if (dstAddrMode == deCONZ::ApsGroupAddress)
    {
        quint16 shortAddr;
        stream >> shortAddr;
        m_dstAddr.setGroup(shortAddr);
        m_dstEndpoint = 0; // not present
        m_dstAddrMode = deCONZ::ApsGroupAddress;
        return stream.status() == QDataStream::Ok;
    }
    else if (dstAddrMode == deCONZ::ApsExtAddress)
    {
        quint64 extAddr;
        stream >> extAddr;
        m_dstAddr.setExt(extAddr);
        stream >> m_dstEndpoint;
        m_dstAddrMode = deCONZ::ApsExtAddress;
        return stream.status() == QDataStream::Ok;
    }

    return false;
}

bool Binding::isValid() const
{
    return m_srcAddr != 0 &&
            ((m_dstAddrMode == deCONZ::ApsExtAddress && m_dstAddr.hasExt()) ||
             (m_dstAddrMode == deCONZ::ApsGroupAddress && m_dstAddr.hasGroup())) &&
            m_srcEndpoint != 0xff && m_dstEndpoint != 0xff;
            // note don't test for clusterId != 0xffff on purpose
}

} // namespace deCONZ
