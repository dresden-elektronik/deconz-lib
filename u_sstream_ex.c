#include "deconz/u_sstream_ex.h"

static const char _hex_table_lower[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

/*! \brief Convert utf-8 to unicode code point.

    Returns pointer to remainder of text. 'codepoint' is a valid codepoint
    or set to 0xFFFD for invalid utf8.
 */
static const char * U_sstream_utf8_codepoint(const char *text, unsigned *codepoint)
{
    unsigned cp;

    cp = (unsigned)*text & 0xFF;
    text++;

    if ((cp & 0x80) == 0)
    {
        // 1-byte ASCII
    }
    else if ((cp & 0xE0) == 0xC0 && text[0] != 0) /*  110 prefix 2-byte char */
    {
        /* 110x xxxx 10xx xxxx */
        cp &= 0x1F;
        cp <<= 6;
        cp |= (unsigned)*text & 0x3F;
        text++;
    }
    else if ((cp & 0xF0) == 0xE0 && text[0] != 0 && text[1] != 0) /*  1110 prefix 3-byte char */
    {
        /* 1110xxxx 10xxxxxx 10xxxxxx */
        cp &= 0x0F;
        cp <<= 6;
        cp |= (unsigned)*text & 0x3F;
        text++;
        cp <<= 6;
        cp |= (unsigned)*text & 0x3F;
        text++;
    }
    else if ((cp & 0xF8) == 0xF0 && text[0] != 0 && text[1] != 0 && text[2] != 0) /*  11110 prefix 4-byte char */
    {
        /* 1110xxxx 10xxxxxx 10xxxxxx */
        cp &= 0x07;
        cp <<= 6;
        cp |= (unsigned)*text & 0x3F;
        text++;
        cp <<= 6;
        cp |= (unsigned)*text & 0x3F;
        text++;
        cp <<= 6;
        cp |= (unsigned)*text & 0x3F;
        text++;
    }
    else
    {
        cp = 0xFFFD;
    }

    *codepoint = cp;

    return text;
}

void U_sstream_put_mac_address(U_SStream *ss, unsigned long long mac)
{
    unsigned i;
    unsigned char nib;

    /* 00:11:22:33:44:55:66:77 */
    if ((ss->len - ss->pos) < 23 + 1)
    {
        ss->status = U_SSTREAM_ERR_NO_SPACE;
        return;
    }

    for (i = 0; i < 8; i++)
    {
        nib = (mac >> 56) & 0xFF;
        mac <<= 8;
        ss->str[ss->pos] = _hex_table_lower[(nib & 0xF0) >> 4];
        ss->pos++;
        ss->str[ss->pos] = _hex_table_lower[(nib & 0x0F)];
        ss->pos++;

        if (i < 7)
        {
            ss->str[ss->pos] = ':';
            ss->pos++;
        }
    }

    ss->str[ss->pos] = '\0';
}

unsigned long long U_sstream_get_mac_address(U_SStream *ss)
{
    char ch;
    unsigned i;
    unsigned long long result;
    unsigned colon_mask = 0x124924;

    result = 0;

    if (ss->status != U_SSTREAM_OK)
        return result;

    /* 00:11:22:33:44:55:66:77 */
    if ((ss->len - ss->pos) < 23)
    {
        ss->status = U_SSTREAM_ERR_RANGE;
        return result;
    }

    for (i = 0; i < 23; i++)
    {
        ch = ss->str[ss->pos];
        ss->pos++;
        if      (ch >= '0' && ch <= '9') ch = ch - '0';
        else if (ch >= 'a' && ch <= 'f') ch = (ch - 'a') + 10;
        else if (ch >= 'A' && ch <= 'F') ch = (ch - 'A') + 10;
        else if (ch == ':' && colon_mask & (1 << i))
        {
            continue;
        }
        else
        {
            ss->status = U_SSTREAM_ERR_INVALID;
            result = 0;
            break;
        }
        result <<= 4;
        result += (ch & 0xF);
    }

    return result;
}

unsigned char U_sstream_get_hex_byte(U_SStream *ss)
{
    unsigned i;
    unsigned char ch;
    unsigned char result;

    result = 0;

    if (ss->status != U_SSTREAM_OK)
        return result;

    for (i = 0; i < 2 && ss->pos < ss->len; i++)
    {
        ch = (unsigned char)ss->str[ss->pos];

        if      (ch >= '0' && ch <= '9') ch = ch - '0';
        else if (ch >= 'a' && ch <= 'f') ch = (ch - 'a') + 10;
        else if (ch >= 'A' && ch <= 'F') ch = (ch - 'A') + 10;
        else
        {
            break;
        }

        result <<= 4;
        result |= ch;
        ss->pos += 1;
    }

    return result;
}

int U_sstream_is_valid_utf8(U_SStream *ss)
{
    const char *str;
    unsigned codepoint;

    if (ss->status != U_SSTREAM_OK)
        return 0;

    if (ss->len == 0)
        return 0;

    str = ss->str;
    codepoint = 0;

    for (;str < &ss->str[ss->len];)
    {
        str = U_sstream_utf8_codepoint(str, &codepoint);
        if (codepoint == 0xFFFD)
            return 0;

        if (codepoint == 0)
            break;
    }

    return 1;
}
