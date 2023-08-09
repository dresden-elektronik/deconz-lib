/*
 * Copyright (c) 2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef NANBOX_H
#define NANBOX_H

#include "deconz/declspec.h"

/*! \addtogroup utils

    @{
 */

/*! \addtogroup nanbox NaN Boxing
    \brief 64-bit NaN box format to represent various types.

    NaN boxing allows to use a 64-bit value for multiple value types in
    a type safe manner. The 64-bit word may represent null, true, false, NaN (not a number)
    a UString pointer, Atoms, timestamps or simply normal double values.
    The actual type is encoded within 3 TTT bits.

    \par Format of the 64-bit value

    The 64-bit NaN boxing format assumes a 48-bit virtual address space. A IEEE-745 double value is NaN when
    all exponent bits are 1.

 <pre>
   | S | EEE EEEEEEEE | Q | TTT FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF |

   | 1 | 111 11111111 | 1 | 000 00000000 00000000 00000000 00000000 00000000 00000000 | NaN
   | 1 | 111 11111111 | 1 | 001 00000000 00000000 00000000 00000000 00000000 00000001 | null
   | 1 | 111 11111111 | 1 | 010 PPPPPPPP PPPPPPPP PPPPPPPP PPPPPPPP PPPPPPPP PPPPPP00 | 48-bit UString pointer
       ...                            ...                                  ...
   | 1 | 111 11111111 | 1 | 010 PPPPPPPP PPPPPPPP PPPPPPPP PPPPPPPP PPPPPPPP PPPPPP11 | other pointers
   | 1 | 111 11111111 | 1 | 011 CCCCCCCC LLLLLLLL LLLLLLLL IIIIIIII IIIIIIII IIIIIIII | Atom pointer
   | 1 | 111 11111111 | 1 | 100 FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF | UTC unix time milliseconds
   | 1 | 111 11111111 | 1 | 101 00000000 00000000 00000000 00000000 00000000 00000011 | true
   | 1 | 111 11111111 | 1 | 101 00000000 00000000 00000000 00000000 00000000 00000010 | false
   | 1 | 111 11111111 | 1 | 110 00000000 00000000 00000000 00000000 00000000 cccccc00 | cons cell
   | 1 | 111 11111111 | 1 | 110 NNNNNNNN NNNNNNNN RRRRRRRR RRRRRRRR RRRRRRRR RRRRRR01 | cons cell array
</pre>

   Integers and floating point values are valid IEEE-745 encoded doubles.
   Safe integer range withouth loss of precision is -2^53-1 to 2^53-1.

    @{
 */

/*! \enum NB_BoxType
    \brief 3-bit enumeration refering to TTT type bits.
 */
typedef enum NB_BoxType
{
    NB_TYPE_NAN         = 0, //!< NaN (not a number)
    NB_TYPE_NULL        = 1, //!< null
    NB_TYPE_USTRING_PTR = 2, //!< Used internally by UString for heap allocated strings
    NB_TYPE_ATOM_PTR    = 3, //!< Atom index and length
    NB_TYPE_TIMESTAMP   = 4, //!< 48-bit Unix timestamp with millisecond precision
    NB_TYPE_BOOL        = 5, //!< Bool type for true and false

    NB_TYPE_DOUBLE      = 7, //!< Not a NaN boxed value but a valid double

    NB_TYPE_UNKNOWN     = 8  //!< Type can't be determined
} NB_BoxType;

typedef unsigned long long nb_word;

#define NB_SHIFT_TYPE  48LLU
#define NB_SHIFT_Q     51LLU
#define NB_S_TO_Q_MASK 0x1fffLLU
#define NB_TYPE_MASK   7

#define NB_ATOM_SHIFT_LENGTH 24ULL
#define NB_ATOM_LENGTH_MASK 0xFFFF
#define NB_ATOM_PTR_MASK 0xFFFFFFUL

/*! 64-bit value representing NaN */
#define NB_VALUE_NAN   ((NB_S_TO_Q_MASK << NB_SHIFT_Q) | ((nb_word)NB_TYPE_NAN  << NB_SHIFT_TYPE))
/*! 64-bit value representing null */
#define NB_VALUE_NULL  ((NB_S_TO_Q_MASK << NB_SHIFT_Q) | ((nb_word)NB_TYPE_NULL << NB_SHIFT_TYPE))
/*! 64-bit value representing true */
#define NB_VALUE_TRUE  ((NB_S_TO_Q_MASK << NB_SHIFT_Q) | ((nb_word)NB_TYPE_BOOL << NB_SHIFT_TYPE) | 3)
/*! 64-bit value representing false */
#define NB_VALUE_FALSE ((NB_S_TO_Q_MASK << NB_SHIFT_Q) | ((nb_word)NB_TYPE_BOOL << NB_SHIFT_TYPE) | 2)

/*! creates a 64-bit value representing a unique atom pointer */
#define NB_VALUE_ATOM_PTR(idx, len)  ((NB_S_TO_Q_MASK << NB_SHIFT_Q) | ((nb_word)NB_TYPE_ATOM_PTR << NB_SHIFT_TYPE) | ((len) << NB_ATOM_SHIFT_LENGTH) | (idx))

/*! \struct NB_NaNBox
    \brief Holds a NaN boxed value.
 */
typedef struct NB_NanBox
{
    union
    {
        nb_word u64; /*!< unsigned value for bit operations */
        double f64;  /*!< double value for NB_TYPE_DOUBLE */
    };
} NB_NanBox;

#ifdef __cplusplus
extern "C" {
#endif

/*! Returns 1 if the value is NaN boxed, that is the expontent bits are 1 and TTT type is known. */
DECONZ_DLLSPEC int NB_IsBox(struct NB_NanBox);
/*! Returns the type of a NaN boxed value. */
DECONZ_DLLSPEC NB_BoxType NB_Type(struct NB_NanBox);

#ifdef __cplusplus
}
#endif

/*! @} end of nanbox */

/*! @} end of utils */

#endif /* NANBOX_H */
