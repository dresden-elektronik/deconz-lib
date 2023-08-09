/*
 * Copyright (c) 2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef ATOM_H
#define ATOM_H

/*! @addtogroup utils

    @{
 */

/*! @addtogroup atoms Atoms

    Atoms refer to strings which are allocated once and are stored in the
    atom table for the lifetime of the application. They can be referenced
    by a unique and stable index.

    @{
 */

/*! \struct AT_Atom
    \brief Represents an atom which is uniquely stored in the atom table.
 */
typedef struct AT_Atom
{
    unsigned len; /*!< Length of the atom, excluding 0 terminator */
    unsigned char *data; /*!< Data pointer, always zero terminated, often a C-String */
} AT_Atom;

/*! \struct AT_AtomIndex
    \brief A unique index referencing a atom.

    This is a struct rather than just a plain integer to make functions using the
    index type save and prevent common errors.
 */
typedef struct AT_AtomIndex
{
    unsigned index;
} AT_AtomIndex;

/*! @} end of atoms */

/*! @} end of utils */
#endif // ATOM_H
