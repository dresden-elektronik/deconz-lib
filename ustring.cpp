/*
 * Copyright (c) 2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <string.h> /* memcpy */
#include <stdlib.h> /* malloc, free */

#include "deconz/atom_table.h"
#include "deconz/nanbox.h"
#include "deconz/ustring.h"
#include "deconz/u_sstream.h"

#define US_DEBUG

#ifdef US_DEBUG
  #if __GNUC__
    #define US_ASSERT(x) \
      if ((x) == 0) { __builtin_trap(); }
  #else
    #define US_ASSERT(x) (void*)0
  #endif
#else
  #define US_ASSERT(x) (void*)0
#endif

#define US_PTR_MASK 0xFFFFFFFFFFFFLLU // assume 48-bit pointer size

#define US_FLAG_IS_UTF8 0x01

struct US_Header
{
    unsigned len;
    unsigned char flags;
    unsigned char _pad[3];
    /*
     * U8 _str makes sizeof(US_Header) to include '\0'.
     * It also is the address of the cstring.
     */
    unsigned char _str;
};

static const char _us_empty_cstring = 0;

static NB_BoxType US_BoxTypeFromWord(US_WordType word)
{
    NB_NanBox nb;
    nb.u64 = word;
    return NB_Type(nb);
}

static US_Header *US_GetHeaderPtr(US_WordType word)
{
    // here the type check has already be done!
    if (word & US_PTR_MASK)
    {
        US_ASSERT(US_BoxTypeFromWord(word) == NB_TYPE_USTRING_PTR);
        US_ASSERT((word & 0x3) == 0); // assume aligned

        word &= US_PTR_MASK;
        return reinterpret_cast<US_Header*>(word);
    }

    return nullptr;
}

static NB_NanBox NB_FromUStringPtr(US_Header *ptr)
{
    NB_NanBox nb;

    if (ptr)
    {
        US_ASSERT((US_WordType(ptr) & US_PTR_MASK) == US_WordType(ptr));
        nb.u64 = (NB_S_TO_Q_MASK << NB_SHIFT_Q) | ((nb_word)NB_TYPE_USTRING_PTR << NB_SHIFT_TYPE) | US_WordType(ptr);
        US_ASSERT((nb.u64 & 3) == 0); // assumption lower 2-bits are zero
    }
    else
    {
        nb.u64 = NB_VALUE_NULL;
    }
    return nb;
}

static US_WordType US_HeaderPtrToWord(US_Header *ptr)
{
    return NB_FromUStringPtr(ptr).u64;
}

static void *US_Alloc(unsigned size)
{
    void *ptr = nullptr;
    if (size)
    {
        ptr = malloc(size);
        US_ASSERT((US_WordType(ptr) & 0x3) == 0); // assume aligned
    }
    return ptr;
}

static void US_Free(US_WordType word)
{
    const NB_BoxType type = US_BoxTypeFromWord(word);

    if (type == NB_TYPE_USTRING_PTR)
    {
        word &= US_PTR_MASK;
        if (word)
            free(reinterpret_cast<void*>(word));
    }
}

static US_Header *US_AllocForStrlen(unsigned len)
{
    return (US_Header*)US_Alloc(sizeof(US_Header) + len);
}

UString::UString()
{
    static_assert(sizeof(NB_NanBox) == sizeof(d_word), "UString::d_word size is wrong");
    d_word = NB_VALUE_NULL;
}

UString::UString(const UString &other)
{
    const NB_BoxType type = US_BoxTypeFromWord(other.d_word);

    d_word = NB_VALUE_NULL;

    if (type == NB_TYPE_USTRING_PTR)
    {
        US_Header *other_hdr = US_GetHeaderPtr(other.d_word);

        if (other_hdr)
        {
            US_Header *hdr = US_AllocForStrlen(other_hdr->len);

            if (hdr)
            {
                memcpy(hdr, other_hdr, sizeof(US_Header) + other_hdr->len);
                d_word = US_HeaderPtrToWord(hdr);
            }
        }
    }
    else if (type == NB_TYPE_ATOM_PTR)
    {
        d_word = other.d_word;
    }
}

UString::UString(UString &&other) noexcept
{
    if (&other != this)
    {
        d_word = other.d_word;
        other.d_word = 0;
    }
}

UString &UString::operator=(const UString &other)
{
    // Self assignment?
    if (this == &other)
    {
        return *this;
    }

    US_Free(d_word);
    d_word = NB_TYPE_NULL;

    const NB_BoxType type = US_BoxTypeFromWord(other.d_word);

    if (type == NB_TYPE_USTRING_PTR)
    {
        US_Header *other_hdr = US_GetHeaderPtr(other.d_word);

        if (other_hdr)
        {
            US_Header *hdr = US_AllocForStrlen(other_hdr->len);

            if (hdr)
            {
                memcpy(hdr, other_hdr, sizeof(US_Header) + other_hdr->len);
                d_word = US_HeaderPtrToWord(hdr);
            }
        }
    }
    else if (type == NB_TYPE_ATOM_PTR)
    {
        d_word = other.d_word;
    }
    else
    {
        US_ASSERT(type == NB_TYPE_NULL);
    }

    return *this;
}

UString &UString::operator=(UString &&other) noexcept
{
    // Self assignment?
    if (this == &other)
    {
        return *this;
    }

    US_Free(d_word);
    d_word = other.d_word;
    other.d_word = NB_VALUE_NULL;

    return *this;
}

UString::~UString()
{
    US_Free(d_word);
    d_word = NB_VALUE_NULL;
}

bool UString::operator==(const UString &rhs) const
{
    if (US_BoxTypeFromWord(d_word) == NB_TYPE_ATOM_PTR &&
        US_BoxTypeFromWord(rhs.d_word) == NB_TYPE_ATOM_PTR)
        return d_word == rhs.d_word;

    unsigned sz = size();
    if (sz != rhs.size())
        return false;

    unsigned i;
    const char *a = c_str();
    const char *b = rhs.c_str();

    for (i = 0; i < sz; i++)
    {
        if (a[i] != b[i])
            return false;
    }

    return true;
}

bool UString::operator==(const char *b) const
{
    if (!b)
        return false;

    unsigned sz;

    for (sz = 0; b[sz]; sz++)
    {
    }

    if (sz != size())
        return false;

    unsigned i;
    const char *a = c_str();

    for (i = 0; i < sz; i++)
    {
        if (a[i] != b[i])
            return false;
    }

    return true;
}

UString::UString(const char *str)
{
    unsigned char flags = 0;
    unsigned long len = 0;

    if (str)
    {
        for (len = 0; str[len]; len++)
        {
            if (str[len] & 0x80)
                flags |= US_FLAG_IS_UTF8;

        }
    }

    US_Header *hdr = nullptr;
    if (len != 0)
        hdr = US_AllocForStrlen(len);

    if (hdr)
    {
        hdr->len = len;
        hdr->flags = flags;
        memcpy(&hdr->_str, str, len + 1); // copy incl. '\0' terminator
        d_word = US_HeaderPtrToWord(hdr);
    }
    else
    {
        d_word = NB_VALUE_NULL;
    }
}

const char *UString::c_str() const
{
    const NB_BoxType type = US_BoxTypeFromWord(d_word);

    if (type == NB_TYPE_USTRING_PTR)
    {
        US_Header *hdr = US_GetHeaderPtr(d_word);
        if (hdr)
            return reinterpret_cast<char*>(&hdr->_str);
    }
    else if (type == NB_TYPE_ATOM_PTR)
    {
        AT_Atom a;
        AT_AtomIndex ati;

        ati.index = unsigned(d_word & NB_ATOM_PTR_MASK);
        a = AT_GetAtomByIndex(ati);
        US_ASSERT(a.len);
        US_ASSERT(a.data);
        if (a.data)
            return (const char*)a.data;
    }
    else
    {
        US_ASSERT(type == NB_TYPE_NULL);
    }

    return &_us_empty_cstring;
}

unsigned UString::size() const
{
    const NB_BoxType type = US_BoxTypeFromWord(d_word);

    if (type == NB_TYPE_USTRING_PTR)
    {
        US_Header *hdr = US_GetHeaderPtr(d_word);
        US_ASSERT(hdr);
        if (hdr)
            return hdr->len;
    }
    else if (type == NB_TYPE_ATOM_PTR)
    {
        const unsigned len = unsigned(d_word >> NB_ATOM_SHIFT_LENGTH) & NB_ATOM_LENGTH_MASK;

        return len;
    }
    else
    {
        US_ASSERT(type == NB_TYPE_NULL);
    }

    return 0;
}

bool UString::empty() const
{
    return size() == 0;
}

UString UString::number(int num, int base)
{
    return UString::number((long long)num, base);
}

UString UString::number(unsigned int num, int base)
{
    return UString::number((unsigned long long)num, base);
}

UString UString::number(long num, int base)
{
    return UString::number((long long)num, base);
}

UString UString::number(unsigned long num, int base)
{
    return UString::number((unsigned long long)num, base);
}

UString UString::number(long long num, int base)
{
    char str[32];
    U_SStream ss;

    str[0] = '\0';
    U_sstream_init(&ss, &str[0], sizeof(str));

    if (base == 10)
    {
        U_sstream_put_longlong(&ss, num);
    }
    else if (base == 16)
    {
        U_sstream_put_hex(&ss, &num, 8); /* TODO is the order correct? compare to Qt */
    }
    else
    {
        US_ASSERT(0 && "binary string conversion not implemented");
    }

    return UString(&str[0]);
}

UString UString::number(unsigned long long num, int base)
{
    char str[32];
    U_SStream ss;

    str[0] = '\0';
    U_sstream_init(&ss, &str[0], sizeof(str));

    if (base == 10)
    {
        U_sstream_put_ulonglong(&ss, num);
    }
    else if (base == 16)
    {
        U_sstream_put_hex(&ss, &num, 8); /* TODO is the order correct? compare to Qt */
    }
    else
    {
        US_ASSERT(0 && "binary string conversion not implemented");
    }

    return UString(&str[0]);
}

UString UString::number(double num, char f, int prec)
{
    int n;
    char str[256];
    char fmt[8];
    U_SStream ss;

    if (f != 'f' && f != 'g' && f != 'e' && f != 'E')
        f = 'f';

    if (prec < 1 || prec > 9)
        prec = 6;

    if (f == 'f')
    {
        U_sstream_init(&ss, str, sizeof(str));
        U_sstream_put_double(&ss, num, prec);

        if (ss.status == U_SSTREAM_OK)
        {
            return UString(ss.str);
        }
    }

    fmt[0] = '%';
    fmt[1] = '.';
    fmt[2] = (char)prec + '0';
    fmt[3] = f;
    fmt[4] = '\0';

    n = snprintf(str, sizeof(str) - 1, fmt, num);
    n = (n < 0 || n > 63) ? 0 : n;
    str[n] = '\0';
    // TODO(mpi): remove trailing zeros and dot

    return UString(&str[0]);
}

UString UString::fromAtom(AT_Atom atom)
{
    UString s;
    AT_AtomIndex ati;

    if (atom.len && atom.len <= NB_ATOM_PTR_MASK)
    {
        if (AT_GetAtomIndex(atom.data, atom.len, &ati))
        {
            // require max 24-bit index
            US_ASSERT(ati.index <= NB_ATOM_PTR_MASK);
            s.d_word = NB_VALUE_ATOM_PTR(ati.index, atom.len);
            return s;
        }
    }
    else if (atom.len)
    {
        US_ASSERT(0 && "atom length greater than 24-bit");
    }

    return s;
}

UString UString::fromAtom(AT_AtomIndex ati)
{
    return UString::fromAtom(AT_GetAtomByIndex(ati));
}
