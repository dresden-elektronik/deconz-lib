/*
 * Copyright (c) 2023-2026 Manuel Pietschmann.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include "u_sstream.h"

#ifndef U_ASSERT
  #define U_ASSERT(c) ((void)0)
#endif

void U_sstream_init(U_SStream *ss, void *str, unsigned size)
{
    ss->str = (char*)str;
    ss->pos = 0;
    ss->len = size;

    if (str && size)
    {
        ss->status = U_SSTREAM_OK;
    }
    else
    {
        ss->status = U_SSTREAM_ERR_INVALID;
    }
}

const char *U_sstream_str(const U_SStream *ss)
{
    U_ASSERT(ss->pos < ss->len);
    return &ss->str[ss->pos];
}


unsigned U_sstream_pos(const U_SStream *ss)
{
    return ss->pos;
}

unsigned U_sstream_remaining(const U_SStream *ss)
{
    U_ASSERT(ss->pos <= ss->len);
    if (ss->pos <= ss->len)
        return ss->len - ss->pos;

    return 0;
}

int U_sstream_at_end(const U_SStream *ss)
{
    U_ASSERT(ss->pos <= ss->len);
    if (ss->pos <= ss->len)
        return (ss->len - ss->pos) == 0;

    return 1;
}

long U_sstream_get_long(U_SStream *ss)
{
    int err;
    long r;
    char *nptr;
    unsigned out_len;
    const char *endptr;

    r = 0;

    if (ss->pos < ss->len)
    {
        nptr = &ss->str[ss->pos];
        endptr = 0;

        r = U_strtol(nptr, ss->len - ss->pos, &endptr, &err);
        if (err)
        {
            if      (err & 0x1)         { ss->status = U_SSTREAM_ERR_INVALID; }
            else if (err & (0x2 | 0x4)) { ss->status = U_SSTREAM_ERR_RANGE; }
            r = 0;
        }

        if (endptr)
        {
            out_len = (unsigned)(endptr - nptr);
            if (ss->pos + out_len <= ss->len)
                ss->pos += out_len;
        }
    }

    return r;
}

double U_sstream_get_double(U_SStream *ss)
{
    int err;
    double r;
    char *nptr;
    unsigned out_len;
    const char *endptr;

    r = 0.0;

    if (ss->pos < ss->len)
    {
        nptr = &ss->str[ss->pos];
        endptr = 0;

        r = U_strtod(nptr, ss->len - ss->pos, &endptr, &err);
        if (err)
        {
            ss->status = U_SSTREAM_ERR_INVALID;
            r = 0.0;
        }

        if (endptr)
        {
            out_len = (unsigned)(endptr - nptr);
            if (ss->pos + out_len <= ss->len)
                ss->pos += out_len;
        }
    }

    return r;
}

char U_sstream_peek_char(const U_SStream *ss)
{
    if (ss->pos < ss->len)
        return ss->str[ss->pos];
    return '\0';
}

void U_sstream_skip_whitespace(U_SStream *ss)
{
    char ch;
    while (ss->pos < ss->len)
    {
        ch = ss->str[ss->pos];
        switch (ch)
        {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                ss->pos++;
                break;
            default:
                return;
        }
    }
}

int U_sstream_starts_with(const U_SStream *ss, const char *str)
{
    unsigned i;
    unsigned len;

    if (str == 0)
        return 0;

    for (len = 0; str[len]; len++)
        ;

    if (len == 0)
        return 0;

    if ((ss->len - ss->pos) >= len)
    {
        for (i = 0; i < len; i++)
        {
            if (ss->str[ss->pos + i] != str[i])
                return 0;
        }

        return 1;
    }

    return 0;
}

int U_sstream_find(U_SStream *ss, const char *str)
{
    unsigned i;
    unsigned len;
    unsigned pos;
    unsigned match;

    if (str == 0)
        return 0;

    for (len = 0; str[len]; len++)
        ;

    if (len == 0)
        return 0;

    pos = ss->pos;

    for (; pos < ss->len && (ss->len - pos) >= len; )
    {
        match = 0;
        for (i = 0; i < len; i++)
        {
            if (ss->str[pos + i] != str[i])
                break;

            match++;
        }

        if (match == len)
        {
            ss->pos = pos;
            return 1;
        }

        pos++;
    }

    return 0;
}

int U_sstream_compare(const U_SStream *ss, const char *str)
{
    unsigned i;
    unsigned char c1;
    unsigned char c2;

    if (ss->status != U_SSTREAM_OK)
        return 0;

    if (str == 0)
        return 0;

    for (i = 0; i < ss->pos; i++)
    {
        c1 = (unsigned char)ss->str[i];
        c2 = (unsigned char)str[i];

        if (c2 == 0)
            return (int)c1;

        if (c1 != c2)
            return (int)c1 - (int)c2;
    }

    if (str[i] == 0)
        return 0;

    return -((unsigned char)str[i]);
}

void U_sstream_put_str(U_SStream *ss, const char *str)
{
    unsigned len;
    unsigned pos;

    if (ss->status != U_SSTREAM_OK)
        return;

    if (str == 0)
        return;

    for (len = 0; str[len]; len++)
        ;

    /* Check for potential overflow and buffer bounds */
    if (ss->pos >= ss->len)
    {
        ss->status = U_SSTREAM_ERR_NO_SPACE;
        return;
    }

    /* Check for integer overflow and buffer bounds properly */
    if (len >= ss->len - ss->pos)
    {
        ss->status = U_SSTREAM_ERR_NO_SPACE;
        return;
    }

    /* Safe to copy */
    pos = ss->pos;
    while (*str && pos < ss->len - 1)
    {
        ss->str[pos++] = *str++;
    }
    
    ss->str[pos] = '\0';
    ss->pos = pos;
}

/*  Outputs the signed 32/64-bit integer 'num' as ASCII string.

    The range is different on 32-bit systems and Windows
    and 64-bit systems.

    -2147483648 .. 2147483647
    -9223372036854775807 .. 9223372036854775807

    \param num signed number
*/
void U_sstream_put_long(U_SStream *ss, long num)
{
    int i;
    int pos;
    long remainder;
    long n;
    unsigned char buf[24];

    if (ss->status != U_SSTREAM_OK)
        return;

    /* sign + max digits + NUL := 21 bytes on 64-bit */
    n = num;

    pos = 0;
    do
    {
        remainder = n % 10;
        remainder = remainder < 0 ? -remainder : remainder;
        n = n / 10;
        buf[pos++] = '0' + (unsigned char)remainder;
    }
    while (n);

    if (ss->len - ss->pos < (unsigned)pos + 1 + (num < 0 ? 1 : 0)) /* not enough space */
    {
        ss->status = U_SSTREAM_ERR_NO_SPACE;
        return;
    }

    if (num < 0)
        ss->str[ss->pos++] = '-';

    for (i = pos; i > 0; i--) /* reverse copy */
    {
        ss->str[ss->pos++] = (char)buf[i - 1];
    }

    ss->str[ss->pos] = '\0';
}

/*  Outputs the signed 64-bit integer 'num' as ASCII string.

    -9223372036854775807 .. 9223372036854775807

    \param num signed number
*/
void U_sstream_put_longlong(U_SStream *ss, long long num)
{
    int i;
    int pos;
    long remainder;
    long long n;
    char buf[24];

    if (ss->status != U_SSTREAM_OK)
        return;

    /* sign + max digits + NUL := 21 bytes on 64-bit */

    n = num;

    pos = 0;
    do
    {
        remainder = n % 10;
        remainder = remainder < 0 ? -remainder : remainder;
        n = n / 10;
        buf[pos++] = (char)('0' + remainder);
    }
    while (n);

    if (ss->len - ss->pos < (unsigned)pos + 1 + (num < 0 ? 1 : 0)) /* not enough space */
    {
        ss->status = U_SSTREAM_ERR_NO_SPACE;
        return;
    }

    if (num < 0)
        ss->str[ss->pos++] = '-';

    for (i = pos; i > 0; i--) /* reverse copy */
    {
        ss->str[ss->pos++] = buf[i - 1];
    }

    ss->str[ss->pos] = '\0';
}

/*  Outputs the unsigned 64-bit integer 'num' as ASCII string.

    0 .. 18446744073709551615

    \param num unsigned number
*/
void U_sstream_put_ulonglong(U_SStream *ss, unsigned long long num)
{
    int i;
    int pos;
    unsigned long long remainder;
    char buf[24];

    if (ss->status != U_SSTREAM_OK)
        return;

    /* max digits + NUL := 21 bytes on 64-bit */


    pos = 0;
    do
    {
        remainder = num % 10;
        num = num / 10;
        buf[pos++] = (char)('0' + remainder);
    }
    while (num);

    if (ss->len - ss->pos < (unsigned)pos + 1) /* not enough space */
    {
        ss->status = U_SSTREAM_ERR_NO_SPACE;
        return;
    }

    for (i = pos; i > 0; i--) /* reverse copy */
    {
        ss->str[ss->pos++] = buf[i - 1];
    }

    ss->str[ss->pos] = '\0';
}

union u64f
{
    double f;
    unsigned long long i;
};

static int uss_is_nan(double x)
{
    int e;
    union u64f u;

    u.i = 0;
    u.f = x;

    e = (int)(u.i >> 52 & 0x7ff);
    e ^= 0x7ff;
    u.i <<= 12; /* mantissa */

    return (e == 0 && u.i) ? 1 : 0;
}

static int uss_is_infinity(double x)
{
    int e;
    union u64f u;

    u.i = 0;
    u.f = x;

    e = (int)(u.i >> 52 & 0x7ff);
    e ^= 0x7ff;
    u.i <<= 12; /* mantissa */

    if (e == 0 && u.i == 0)
    {
        u.f = x;
        e = (int)(u.i >> 63); /* sign */
        return e ? 1 : 2;
    }

    return 0;
}

/*
 * Code adapted from musl libc modf implementation.
 * https://steve.hollasch.net/cgindex/coding/ieeefloat.html
 *
 * Sign    Exponent      Mantissa
 * 1 [63]  11 [62-52]    52 [51-0]
 * S       EEEEEEE EEEE  FFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF
 */
static double uss_modf(double x, double *iptr)
{
    int e;
    union u64f u;
    unsigned long long mask;

    u.i = 0; /* if unsigned long long becomes > 64-bit some day */
    u.f = x;
    /*
     * get 11-bit exponent, cut off sign bit
     */
    e = (int)(u.i >> 52 & 0x7ff);
    e -= 1023; /* substract bias */

    /* no fractional part */
    /*
     * exponent range 2..2^52
     */
    if (e >= 52)
    {
        *iptr = x;
        /*
         * NaN when exponent = 1024 and mantissa != 0
         */
        if (e == 1024 && u.i << 12 != 0) /* nan */
            return x;
        u.i &= 1ULL << 63; /* keep the sign */
        return u.f;
    }

    /* no integral part*/
    if (e < 0)
    {
        u.i &= 1ULL << 63;
        *iptr = u.f;
        return x;
    }

    mask = 0xFFFFFFFFFFFFFFFFULL >> 12 >> e;
    if ((u.i & mask) == 0)
    {
        *iptr = x;
        u.i &= 1ULL << 63;
        return u.f;
    }
    u.i &= ~mask;
    *iptr = u.f;
    return x - u.f;
}

void U_sstream_put_double(U_SStream *ss, double num, int precision)
{
    unsigned i;
    double ipart;
    double frac;
    double prec;
    long long b;
    char buf[32];

    if (uss_is_nan(num))
    {
        U_sstream_put_str(ss, "null");
        return;
    }

    b = (unsigned)uss_is_infinity(num);
    if (b != 0)
    {
        if (b == 1)
            U_sstream_put_str(ss, "-");

        U_sstream_put_str(ss, "1e99999");
        return;
    }

    if      (precision < 1) precision = 1;
    else if (precision > 18) precision = 18;

    frac = uss_modf(num, &ipart);

    /*
     * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number/MIN_SAFE_INTEGER
     */
    if (ipart > 9007199254740991.0 || ipart < -9007199254740991.0)
    {
        /* error outside of max safe range 2^53-1 */
        ss->status = U_SSTREAM_ERR_RANGE;
        return;
    }

    if (ipart < 0)
    {
        ipart = -ipart;
        frac = -frac;
        U_sstream_put_str(ss, "-");
    }

    prec = 10;

    b = (long long)ipart;
    U_sstream_put_longlong(ss, b);

    i = 0;
    buf[i++] = '.';

    for (;precision && i < sizeof(buf) - 1; precision--)
    {
        b = (long long)(frac * prec);
        b = b % 10;
        prec *= 10;
        buf[i++] = (char)(b + '0');
    }
    buf[i] = '\0';

    /* strip trailing zero and dot */
    for (; i && (buf[i-1] == '0' || buf[i-1] == '.'); i--)
        buf[i-1] = '\0';

    if (buf[0])
        U_sstream_put_str(ss, &buf[0]);
}

static const char ss_hex_table[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

void U_sstream_put_hex(U_SStream *ss, const void *data, unsigned size)
{
    unsigned i;
    unsigned char nib;
    const unsigned char *buf;
    unsigned available;

    if (ss->status != U_SSTREAM_OK)
        return;

    available = ss->len - ss->pos;
    if (available < 1 || size > (available - 1) / 2)
    {
        ss->status = U_SSTREAM_ERR_NO_SPACE;
        return;
    }

    buf = (const unsigned char*)data;

    for (i = 0; i < size; i++)
    {
        nib = buf[i];
        ss->str[ss->pos] = ss_hex_table[(nib & 0xF0) >> 4];
        ss->pos++;
        ss->str[ss->pos] = ss_hex_table[(nib & 0x0F)];
        ss->pos++;
    }

    ss->str[ss->pos] = '\0';
}

void U_sstream_seek(U_SStream *ss, unsigned pos)
{
    if (pos <= ss->len)
        ss->pos = pos;
}

/** Converts a base 10 number string to signed long.
 *
 * Depending on sizeof(long) 4/8 the valid numeric range is:
 *
 *   32-bit: -2147483648 ... 2147483647
 *   64-bit: -9223372036854775808 ... 9223372036854775807
 *
 * The err variable is a bitmap:
 *
 *   0x01 invalid input
 *   0x02 range overflow
 *   0x04 range underflow
 *
 * \param s pointer to string, doesn't have to be '\0' terminated.
 * \param len length of s ala strlen(s).
 * \param endp pointer which will be set to first non 0-9 character (must NOT be NULL).
 * \param err pointer to error variable (must NOT be NULL).
 *
 * \return If the conversion is successful the number is returned and err set to 0.
 *         On failure err has a non zero value.
 */
long U_strtol(const char *s, unsigned len, const char **endp, int *err)
{
    int sign = 1;
    int digit;
    unsigned long result = 0;
    unsigned i = 0;
    
    /* Determine max values based on system architecture */
#if __SIZEOF_LONG__ == 8
    const unsigned long max = 9223372036854775807UL;
    const unsigned long max_neg = 9223372036854775808UL;  /* |LONG_MIN| */
#else
    const unsigned long max = 2147483647UL;
    const unsigned long max_neg = 2147483648UL;  /* |LONG_MIN| on 32-bit */
#endif

    if (len == 0)
    {
        *err = 1;
        *endp = s;
        return 0;
    }

    /* skip whitespace */
    while (i < len && (*s == ' ' || *s == '\t'))
    {
        s++;
        i++;
    }

    if (i >= len)
    {
        *err = 1;
        *endp = s;
        return 0;
    }

    /* Check for sign */
    if (*s == '-')
    {
        sign = -1;
        s++;
        i++;
    }
    else if (*s == '+')
    {
        s++;
        i++;
    }

    if (i >= len)
    {
        *err = 1;
        *endp = s;
        return 0;
    }

    /* Check that first character is a digit */
    if (*s < '0' || *s > '9')
    {
        *err = 1;
        *endp = s;
        return 0;
    }

    /* Handle the core conversion with overflow checking */
    for (; i < len && *s >= '0' && *s <= '9'; i++, s++)
    {
        digit = *s - '0';
        
        /* Check for overflow before multiplying */
        if (sign == 1)
        {
            /* Positive number - check against max */
            if (result > (max - digit) / 10)
            {
                *err = 2; /* overflow */
                *endp = s;
                return 0;
            }
        }
        else
        {
            /* Negative number - check against max_neg (|LONG_MIN|) */
            if (result > (max_neg - digit) / 10)
            {
                *err = 2; /* overflow */
                *endp = s;
                return 0;
            }
        }
        
        result = result * 10 + digit;
    }

    *endp = s;
    *err = 0;
    
    /* Apply sign at the end */
    if (sign == -1)
        return -(long)result;
    
    return (long)result;
}

/* custom pow() */
static double pow_helper(double base, int exponent)
{
    int i;
    int count;
    double result;

    count = exponent < 0 ? -exponent : exponent;

    result = 1.0;

    if (exponent < 0)
        base = 1.0 / base;

    for (i = 0; i < count; i++)
        result *= base;

    return result;
}

/** Converts a floating point number string to double.
 *
 * The err variable is a bitmap:
 *
 *   0x01 invalid input
 *   0x02 overflow
 *
 * \param str pointer to string, doesn't have to be '\0' terminated.
 * \param len length of s ala strlen(s).
 * \param endp pointer which will be set to first non 0-9 character (must NOT be NULL).
 * \param err pointer to error variable (must NOT be NULL).
 *
 * \return If the conversion is successful the number is returned and err set to 0.
 *         On failure err has a non zero value.
 */
double U_strtod(const char *str, unsigned len, const char **endp, int *err)
{
    int sign;
    int exponent;
    int exp_sign;
    int exp_num;
    int decimal_places;
    double num;
    int required;

    sign = 1;
    exponent = 0;
    exp_sign = 1;
    exp_num = 0;
    decimal_places = 0;
    num = 0.0;
    required = 0;

    /* skip whitespace */
    while (len && (*str == ' ' || *str == '\t'))
    {
        str++;
        len--;
    }

    if (len)
    {
        if (*str == '-')
        {
            sign = -1;
            str++;
            len--;
        }
        else if (*str == '+')
        {
            str++;
            len--;
        }
    }

    /* integer part */
    while (len && *str >= '0' && *str <= '9')
    {
        required = 1;
        num = num * 10 + (*str - '0');
        if (uss_is_infinity(num))
        {
            *endp = str;
            *err = 0x02;
            return 0.0;
        }
        str++;
        len--;
    }

    /* decimal part */
    if (len && *str == '.')
    {
        str++;
        len--;
        while (len && *str >= '0' && *str <= '9')
        {
            required = 1;
            num = num * 10 + (*str - '0');
            if (uss_is_infinity(num))
            {
                *endp = str;
                *err = 0x02;
                return 0.0;
            }
            decimal_places++;
            str++;
            len--;
        }
    }

    /* handle exponent */
    if (len && (*str == 'e' || *str == 'E'))
    {
        str++;
        len--;
        if (len)
        {
            if (*str == '-')
            {
                exp_sign = -1;
                str++;
                len--;
            }
            else if (*str == '+')
            {
                str++;
                len--;
            }
        }

        while (len && *str >= '0' && *str <= '9')
        {
            exp_num = exp_num * 10 + (*str - '0');
            str++;
            len--;
        }
        exponent = exp_sign * exp_num;
    }

    /* calculate final result */
    num *= pow_helper(10.0, exponent);
    num /= pow_helper(10.0, decimal_places);

    *endp = str;
    *err = required == 0 ? 1 : 0;

    return sign * num;
}
