/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef NODE_PRIVATE_H
#define NODE_PRIVATE_H

namespace deCONZ {

class NodePrivate
{
public:
    UString extAddrStr;
    Address address;
    MacCapabilities m_macCapa;
    NodeDescriptor m_nodeDescr;
    PowerDescriptor m_powerDescr;
    bool isZombie = false;
    bool needRedraw = true;
    QString userDescr;
    std::vector<quint8> m_endpoints; //!< List of active endpoints.
    std::vector<quint8> m_fetchEndpoints; //!< List of active endpoints to fetch.
    std::vector<SimpleDescriptor> m_simpleDescriptors;
    std::vector<SourceRoute> m_sourceRoutes;
    size_t edIter = 0;
    std::array<int8_t, 5> edValues{};
};

} // namespace deCONZ

#endif // NODE_PRIVATE_H
