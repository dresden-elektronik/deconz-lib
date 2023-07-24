/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <array>
#include <string>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <chrono>
#include "deconz/dbg_trace.h"
#ifdef __WIN32__
  #include <windows.h>
#endif
#include <time.h>
#include <sys/timeb.h>

#define MAX_DBG_LINE 8192
#define MAX_BUFFERS 3
#define FLUSH_EVERY_N_LINES 10

/******************************************************************************
                    Local variables
******************************************************************************/
struct DbgContext
{
    DbgContext()
    {
        buf.reserve(MAX_DBG_LINE * MAX_BUFFERS);
    }
    uint64_t lastFlush = 0;
    std::string buf;
};

struct DbgItem
{
    int item = 0;
    std::string name;
};

static FILE *logFP = nullptr;
static char lookup[] = { '0', '1', '2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
static int dbgEnable = 0;
static char dbgLine[MAX_DBG_LINE];
static char dbgLine2[MAX_DBG_LINE];
static DbgContext *_dbg = nullptr;

static void (*dbgCallback)(int, const char*) = nullptr;

#define DBG_ITEM(x) #x

static const std::array<DbgItem, 24> levelStrings = {
    DbgItem{DBG_INFO, "INFO"},
    DbgItem{DBG_ERROR, "ERROR"},
    DbgItem{DBG_PROT, "PROT"},
    DbgItem{DBG_AIR, "AIR"},
    DbgItem{DBG_WIRE, "WIRE"},
    DbgItem{DBG_PROTBUF, "PROTBUF"},
    DbgItem{DBG_ZDP, "ZDP"},
    DbgItem{DBG_ZCL, "ZCL"},
    DbgItem{DBG_APS, "APS"},
    DbgItem{DBG_PROT_L2, "PROT_L2"},
    DbgItem{DBG_ZCLDB, "ZCLDB"},
    DbgItem{DBG_INFO_L2, "INFO_L2"},
    DbgItem{DBG_HTTP, "HTTP"},
    DbgItem{DBG_TLINK, "TLINK"},
    DbgItem{DBG_ERROR_L2, "ERROR_L2"},
    DbgItem{DBG_OTA, "OTA"},
    DbgItem{DBG_APS_L2, "APS_L2"},
    DbgItem{DBG_MEASURE, "MEASURE"},
    DbgItem{DBG_ROUTING, "ROUTING"},
    DbgItem{DBG_ZGP, "ZGP"},
    DbgItem{DBG_IAS, "IAS"},
    DbgItem{DBG_DDF, "DDF"},
    DbgItem{DBG_DEV, "DEV"},
    DbgItem{DBG_JS, "JS"}
};

/******************************************************************************
                    Implementation
******************************************************************************/

static uint64_t msSinceEpoch()
{
    const auto now = std::chrono::system_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return static_cast<uint64_t>(ms.count());
}

void DBG_WriteString(int level, const char *str)
{
    if (dbgEnable & level && _dbg)
    {
        {
            time_t rawtime;
            struct tm * timeinfo;

            time(&rawtime);
            timeinfo = localtime(&rawtime);
            if (strftime(dbgLine2, sizeof(dbgLine2), "%H:%M:%S", timeinfo) != 8)
            {
                return;
            }

            const uint64_t msCount = msSinceEpoch();

            if (snprintf(dbgLine2 + 8, sizeof(dbgLine2) - 16, ":%03ld ", static_cast<long int>(msCount % 1000)) != 5)
            {
                return;
            }
        }

        // 13: length of timestamp incl. space
        const size_t len = strlen(str);

        if (len + 13 >= sizeof(dbgLine2))
        {
            return;
        }

        memcpy(dbgLine2 + 13, str, len);
        dbgLine2[len + 13] = '\0';

        _dbg->buf.append(dbgLine2);

        if (_dbg->buf.capacity() < MAX_DBG_LINE)
        {
            DBG_FlushLazy();
        }

        if (dbgCallback)
        {
            dbgCallback(level, dbgLine2);
        }
    }
}

int DBG_Printf1(int level, const char *format, ...)
{
    if (dbgEnable & level)
    {
        {
            va_list args;
            va_start (args, format);
            vsnprintf(dbgLine, sizeof(dbgLine) - 1, format, args);
            va_end (args);
        }

        DBG_WriteString(level, dbgLine);
        return 0;
    }
    assert(level == DBG_ERROR);
    return -1;
}

void DBG_Init(FILE *logfile)
{
    assert(_dbg == nullptr);
    _dbg = new DbgContext;

    logFP = logfile;
    dbgEnable = 0;
    dbgLine2[sizeof(dbgLine2) - 4] = '.';
    dbgLine2[sizeof(dbgLine2) - 3] = '.';
    dbgLine2[sizeof(dbgLine2) - 2] = '.';
    dbgLine2[sizeof(dbgLine2) - 1] = '\0';
}

void DBG_Destroy()
{
    dbgEnable = 0;
    delete _dbg;
    _dbg = nullptr;
}

void DBG_Flush()
{
    if (!_dbg || _dbg->buf.empty())
    {
        return;
    }

#ifdef __WIN32__
    OutputDebugStringA(_dbg->buf.c_str());
#endif
#if defined(__linux__) || defined(__APPLE__)
    fputs(_dbg->buf.c_str(), logFP);
#if !defined(__arm__) // don't wait blocking
    fflush(logFP);
#endif
#endif // linux, apple
    _dbg->buf.clear();
    assert(_dbg->buf.capacity() > MAX_DBG_LINE);
}

void DBG_FlushLazy()
{
    if (!_dbg || _dbg->buf.empty())
    {
        return;
    }

    const uint64_t msCount = msSinceEpoch();

    if (msCount < _dbg->lastFlush || // invalid
       ((msCount - _dbg->lastFlush) > 100) || _dbg->buf.capacity() < MAX_DBG_LINE)
    {
        _dbg->lastFlush = msCount;
        DBG_Flush();
    }
}

void DBG_Enable(int item)
{
    dbgEnable |= item;
}

void DBG_Disable(int item)
{
    dbgEnable &= ~item;
}

/*! Returns the item number of the string represention \p item.
    \param item – the string representing an item, e.g. "DBG_INFO", etc.
    \returns  >0 - on sucess
               0 - if not found
 */
int DBG_ItemFromString(const char *item)
{
    for (const auto &i : levelStrings)
    {
        if (i.name == item)
        {
            return i.item;
        }
    }

    return 0;
}

/*! Copies the string representation of \p item into \p buf.
    \param buflen – should be at least \c 16.
    \returns  0 - on sucess
             -1 - on error
 */
int DBG_StringFromItem(const int item, char *buf, size_t buflen)
{
    for (const auto &i : levelStrings)
    {
        if (i.item == item)
        {
            assert(i.name.size() + 1 < buflen);
            if (i.name.size() + 1 > buflen)
            {
                break;
            }

            memcpy(buf, i.name.c_str(), i.name.size());
            buf[i.name.size()] = '\0';
            return 0;
        }
    }

    return -1;
}

void DBG_RegisterCallback(void (*cb)(int, const char*))
{
    dbgCallback = cb;
}

int DBG_IsEnabled(int item)
{
    if ((dbgEnable & item) == item)
    {
        return 1;
    }

    return 0;
}

/*!
    Converts data into hex-ascii string.

    The memory area \p ascii must have at least the
    size \p 2 * length + 1 bytes.

    A terminating zero will be appended at the end.

    \returns a pointer to the last byte (zero).
 */
uint8_t *DBG_HexToAscii(const uint8_t *hex, uint8_t length, uint8_t *ascii)
{
    uint8_t i;

    for (i = 0; i < length; i++)
    {
        *ascii++ = lookup[(hex[i] & 0xf0) >> 4];
        *ascii++ = lookup[hex[i] & 0xf];
    }

    *ascii = '\0';

    return ascii;
}
