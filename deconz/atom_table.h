/*
 * Copyright (c) 2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef ATOM_TABLE_H
#define ATOM_TABLE_H

#include "deconz/atom.h"
#include "deconz/declspec.h"

/*! @addtogroup utils

    @{
 */

/*! @addtogroup atoms

    @{
 */

/*! Maximum size of an atom */
#define AT_MAX_ATOM_SIZE 384

#ifdef __cplusplus
extern "C" {
#endif

/*! Initialize atom table, called by core. */
DECONZ_DLLSPEC void AT_Init(unsigned max_atoms);

/*! Destroyes atom table, called by core. */
DECONZ_DLLSPEC void AT_Destroy(void);

/*! Adds an atom from zero terminated C-String.

    \param data a zero terminated C-string
    \returns 1 on success
 */
DECONZ_DLLSPEC int AT_AddAtomString(const void *data);

/*! Adds an atom with data and size.

    This function can be called multiple times, the atom will only be added once.

    \param data the data of the atom (doesn't need to be zero terminated)
    \param size size of the data
    \param ati pointer to atom index which will be set on success
    \returns 1 on success
 */
DECONZ_DLLSPEC int AT_AddAtom(const void *data, unsigned size, AT_AtomIndex *ati);

/*! Returns the atom index for given data.

    \param data the data of the atom (doesn't need to be zero terminated)
    \param size size of the data
    \param ati pointer to atom index which will be set on success
    \returns 1 on success
 */
DECONZ_DLLSPEC int AT_GetAtomIndex(const void *data, unsigned size, AT_AtomIndex *ati);

/*! Returns the atom for given index.

    \param ati a atom index
    \returns a valid atom if found, or zero length atom if not found
 */
DECONZ_DLLSPEC AT_Atom AT_GetAtomByIndex(AT_AtomIndex ati);

#ifdef __cplusplus
}
#endif

/*! @} end of atoms */

/*! @} end of utils */

#endif /* ATOM_TABLE_H */
