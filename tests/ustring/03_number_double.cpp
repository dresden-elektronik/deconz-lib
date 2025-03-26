#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include "deconz/ustring.h"

int main(void)
{
    double num[] = {
        -1.555,
        0,
        1,
        10e33+1
    };

    for (unsigned i = 0; i < sizeof(num) / sizeof(num[0]); i++)
    {
        UString ustr = UString::number(num[i]);
        QString qstr = QString::number(num[i]);

        if (qstr != ustr.c_str())
        {
            printf("%d ustr: %s, qstr: %s\n", __LINE__, ustr.c_str(), qPrintable(qstr));
            return 1;
        }
    }

    for (unsigned i = 0; i < sizeof(num) / sizeof(num[0]); i++)
    {
        UString ustr = UString::number(num[i], 'f', 4);
        QString qstr = QString::number(num[i], 'f', 4);

        if (qstr != ustr.c_str())
        {
            printf("%d ustr: %s, qstr: %s\n", __LINE__, ustr.c_str(), qPrintable(qstr));
            return 1;
        }
    }

    for (unsigned i = 0; i < sizeof(num) / sizeof(num[0]); i++)
    {
        UString ustr = UString::number(num[i], 'g', 8);
        QString qstr = QString::number(num[i], 'g', 8);

        if (qstr != ustr.c_str())
        {
            printf("%d ustr: %s, qstr: %s\n", __LINE__, ustr.c_str(), qPrintable(qstr));
            return 1;
        }
    }

    return 0;
}
