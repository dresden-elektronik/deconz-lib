#ifndef MEM_POOL_H
#define MEM_POOL_H

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <algorithm>
#include <array>
#include <cassert>
#include <tuple>

template <typename T, typename Tuple>
auto &MEM_GetAllocContainer(Tuple &t)
{
    return std::get<std::array<T*, T::PoolSize>>(t);
}

template <typename T, typename MemTuple>
T *MEM_AllocItem(MemTuple *m)
{
    assert(m);
    auto &cont = MEM_GetAllocContainer<T>(*m);

    auto i = std::find_if(std::begin(cont), std::end(cont), [](const auto &i) { return i != nullptr; });

    if (i != cont.end())
    {
        auto *p = *i;
        *i = nullptr;
        return p;
    }

    return new T;
}

template <typename T, typename MemTuple>
void MEM_DeallocItem(T *priv, MemTuple *m)
{
    assert(m);
    auto &cont = MEM_GetAllocContainer<T>(*m);
    auto i = std::find(std::begin(cont), std::end(cont), nullptr);

    if (i != cont.end())
    {
        *i = priv;
        return;
    }

    delete priv;
}

#endif // MEM_POOL_H
