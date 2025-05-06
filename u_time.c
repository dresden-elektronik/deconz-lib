/*
 * Copyright (c) 2012-2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <time.h>
#include <errno.h>

#include "deconz/u_assert.h"
#include "deconz/u_time.h"
#include "deconz/u_sstream.h"
#include "deconz/u_memory.h"

/* ISO 8601 extended format

    Supported formats:

    YYYY-MM-DDTHH:MM:SS.sssZ
    YYYY-MM-DDTHH:MM:SSZ
    YYYY-MM-DDTHH:MMZ
    YYYY-MM-DDTHHZ
    YYYY-MM-DD

    2022-07-16T12:39:33.164Z
    2022-07-16T12:39:33.164
    2022-07-16T12:39:33Z
    2022-07-16T12:39Z
    2022-07-16T12Z

    Timezones:
      Z
      ±hh:mm
      ±hhmm
      ±hh
*/
U_Time U_TimeFromISO8601(const char *str, unsigned len)
{
    U_Time result;
    U_SStream ss;
    struct tm time_;
    int millisec;

    U_memset(&time_, 0, sizeof(time_));
    millisec = 0;
    time_.tm_isdst = -1;
    U_sstream_init(&ss, (void*)str, len);

    time_.tm_year = U_sstream_get_long(&ss);
    if (ss.pos != 4 || U_sstream_peek_char(&ss) != '-' || time_.tm_year < 1900)
        goto err;
    time_.tm_year -= 1900;
    U_sstream_seek(&ss, ss.pos + 1);

    time_.tm_mon = U_sstream_get_long(&ss);
    if (ss.pos != 7 || U_sstream_peek_char(&ss) != '-' || time_.tm_mon < 1 || time_.tm_mon > 12)
        goto err;
    time_.tm_mon -= 1;
    U_sstream_seek(&ss, ss.pos + 1);

    time_.tm_mday = U_sstream_get_long(&ss);
    if (ss.pos != 10 || time_.tm_mday < 1 || time_.tm_mday > 31) /* 32 (?) */
        goto err;

    if (U_sstream_peek_char(&ss) == 'T')
    {
        U_sstream_seek(&ss, ss.pos + 1);

        time_.tm_hour = U_sstream_get_long(&ss);
        if (ss.pos != 13 || time_.tm_hour < 0 || time_.tm_hour > 23)
            goto err;

        if (U_sstream_peek_char(&ss) == ':') /* optional minutes */
        {
            U_sstream_seek(&ss, ss.pos + 1);

            time_.tm_min = U_sstream_get_long(&ss);
            if (ss.pos != 16 || time_.tm_min < 0 || time_.tm_min > 59)
                goto err;

            if (U_sstream_peek_char(&ss) == ':') /* optional seconds */
            {
                U_sstream_seek(&ss, ss.pos + 1);

                time_.tm_sec = U_sstream_get_long(&ss);
                if (ss.pos != 19 || time_.tm_sec < 0 || time_.tm_sec > 60)
                    goto err;

                if (time_.tm_sec == 60)
                {
                    time_.tm_sec -= 1;
                    /* todo leap second */
                }

                if (U_sstream_peek_char(&ss) == '.' || U_sstream_peek_char(&ss) == ',')
                {
                    U_sstream_seek(&ss, ss.pos + 1);
                    millisec = U_sstream_get_long(&ss);
                }
            }
        }
    }

    errno = 0;
    if (U_sstream_peek_char(&ss) == 'Z')
    {
#ifdef _WIN32
        result = _mkgmtime(&time_);
#else
        result = timegm(&time_);
#endif
        result *= 1000;
    }
    else if (U_sstream_peek_char(&ss) == '+' || U_sstream_peek_char(&ss) == '-')
    {
        result = 0;
        U_ASSERT(0 && "timezone not supported yet");
        /* timezone offset
            +-hh:mm
            +-hh
        */
    }
    else
    {
#ifdef _WIN32
        result = mktime(&time_);
#else
        result = mktime(&time_);
#endif
        result *= 1000;
    }

    if (errno)
    {
        result = 0;
    }
    else
    {
        result += millisec;
    }

    return result;

err:
    result = 0;

    return result;
}
