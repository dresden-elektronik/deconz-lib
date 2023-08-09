#include <stdint.h>
#include "deconz/ustring.h"

int main(void)
{
    uint64_t num[] = {
        0,
        9237498,
        18446744073709551615ULL
    };

    for (unsigned i = 0; i < sizeof(num) / sizeof(num[0]); i++)
    {
        UString ustr = UString::number(num[i]);
        QString qstr = QString::number(num[i]);

        if (qstr != ustr.c_str())
            return 1;
    }

    return 0;
}
