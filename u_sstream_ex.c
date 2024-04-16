#include "deconz/u_sstream_ex.h"

static const char _hex_table_lower[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

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
