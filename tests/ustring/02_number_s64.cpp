#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include "deconz/ustring.h"

int main(void)
{
    int64_t num[] = {
        LONG_LONG_MIN,
        9237498,
        LONG_LONG_MAX
    };

    for (unsigned i = 0; i < sizeof(num) / sizeof(num[0]); i++)
    {
        UString ustr = UString::number(num[i]);
        QString qstr = QString::number(num[i]);

        if (qstr != ustr.c_str())
        {
            printf("ustr: %s, qstr: %s\n", ustr.c_str(), qPrintable(qstr));
            return 1;
        }
    }

    return 0;
}
