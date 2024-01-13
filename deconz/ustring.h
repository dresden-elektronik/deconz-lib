/*
 * Copyright (c) 2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef U_STRING_H
#define U_STRING_H

#include "deconz/atom.h"
#include "deconz/declspec.h"

#ifndef USTRING_NO_QSTRING
#include <QString>
#endif

typedef unsigned long long US_WordType;


/*!
    \ingroup utils
    \class UString
    \brief String class to replace QString (work in progress).

    Beside the usual string functions, a UString can be constructed from an atom.
    This makes the strings very small, since only the atom index is stored, and
    fast for comparisons and copies. \see atoms
 */
class DECONZ_DLLSPEC UString
{
public:
    /*! Constructor. */
    UString();
    /*! C string constructor. */
    UString(const char *str);
    /*! Copy constructor. */
    UString(const UString &other);
    /*! Move constructor. */
    UString(UString &&other) noexcept;
    /*! Copy assignment operator. */
    UString& operator=(const UString &other);
    /*! Move assignment operator. */
    UString& operator=(UString &&other) noexcept;
    /*! Destructor. */
    ~UString();

    bool operator==(const UString &) const;
    bool operator!=(const UString &rhs) const { return !(*this == rhs); }

    bool operator==(const char*) const;
    bool operator!=(const char *rhs) const { return !(*this == rhs); }

    /*! Returns the zero terminated C-String.

        This is always safe to use also for empty strings.
     */
    const char *c_str() const;
    /*! Returns the byte size of the string.

        Note for multibyte UTF-8 characters the size is larger than the
        codepoint count.
     */
    unsigned size() const;
    /*! Returns true if the string is empty. */
    bool empty() const;

    static UString number(int, int base=10);
    static UString number(unsigned int, int base=10);
    static UString number(long, int base=10);
    static UString number(unsigned long, int base=10);
    static UString number(long long, int base=10);
    static UString number(unsigned long long, int base=10);
    static UString number(double, char f='g', int prec=0);

    /*! Constructs a UString from an atom.

        No dynamic heap allocations are done as only
        the index referencing the atom is stored.
     */
    static UString fromAtom(AT_Atom);
    /*! Constructs a UString from an atom index.

        No dynamic heap allocations are done as only
        the index referencing the atom is stored.
     */
    static UString fromAtom(AT_AtomIndex);

private:
    /*! Internally the `d_word` is just a NaN boxed value which
        can be null, a UString heap allocated string or a atom pointer.
     */
    US_WordType d_word = 0;
};

#ifndef USTRING_NO_QSTRING
/*! Makes a QLatin1String from a UString. */
inline QLatin1String toQLatin1String(UString str)
{
    return QLatin1String(str.c_str(), int(str.size()));
}

/*! Makes a QString from a UString. */
inline QString toQString(UString str)
{
    return QString::fromUtf8(str.c_str(), int(str.size()));
}
#endif

#endif /* U_STRING_H */
