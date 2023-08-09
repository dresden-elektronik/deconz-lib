/*
 * Copyright (c) 2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include "deconz/nanbox.h"


/*
 * | S | EEE EEEEEEEE | Q | TTT FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF |
 *
 */
int NB_IsBox(struct NB_NanBox nb)
{
    return (((nb.u64 >> NB_SHIFT_Q) & NB_S_TO_Q_MASK) == NB_S_TO_Q_MASK);
}

NB_BoxType NB_Type(struct NB_NanBox nb)
{
    NB_BoxType result;

    if (NB_IsBox(nb))
    {
        result = (NB_BoxType)((nb.u64 >> NB_SHIFT_TYPE) & NB_TYPE_MASK);

        if (result > NB_TYPE_UNKNOWN)
            result = NB_TYPE_UNKNOWN;
    }
    else
    {
        return NB_TYPE_DOUBLE;
    }

    return result;
}
