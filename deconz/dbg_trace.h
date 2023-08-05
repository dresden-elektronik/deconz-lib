#ifndef DECONZ_DBG_TRACE_H
#define DECONZ_DBG_TRACE_H

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */
#include "deconz/declspec.h"

#define DBG_INFO     0x00000001
#define DBG_ERROR    0x00000002
#define DBG_PROT     0x00000004
#define DBG_AIR      0x00000008
#define DBG_WIRE     0x00000010
#define DBG_PROTBUF  0x00000020
#define DBG_ZDP      0x00000040
#define DBG_ZCL      0x00000080
#define DBG_APS      0x00000100
#define DBG_PROT_L2  0x00000200
#define DBG_ZCLDB    0x00000400
#define DBG_INFO_L2  0x00000800
#define DBG_HTTP     0x00001000
#define DBG_TLINK    0x00002000
#define DBG_ERROR_L2 0x00004000
#define DBG_OTA      0x00008000
#define DBG_APS_L2   0x00010000
#define DBG_MEASURE  0x00020000
#define DBG_ROUTING  0x00040000
#define DBG_ZGP      0x00080000
#define DBG_IAS      0x00100000
#define DBG_DDF      0x00200000
#define DBG_DEV      0x00400000
#define DBG_JS       0x00800000

#define DBG_END      0x01000000

#define CAST_LLD(x) ((long long)(x))
#define CAST_LLU(x) ((unsigned long long)(x))

#define DBG_Assert(e) ((e) ? true : (DBG_Printf1(DBG_ERROR, "%s,%d: assertion '%s' failed\n", Q_FUNC_INFO, __LINE__, #e), false))

#define FMT_MAC_CAST(mac) ((unsigned long long)(mac))
#define FMT_MAC "0x%016llX"
#define FMT_CLUSTER "0x%04" PRIX16
#define FMT_ATTR "0x%04" PRIX16


#define DBG_MEASURE_START(MEAS_ID) \
    QElapsedTimer MEAS_ID##measTimer; \
    const char *MEAS_ID##fn = __FUNCTION__; \
    const int MEAS_ID##line = __LINE__; \
    MEAS_ID##measTimer.start()

#define DBG_MEASURE_END(MEAS_ID) \
    if (DBG_IsEnabled(DBG_MEASURE)) { \
        DBG_Printf1(DBG_MEASURE, "MS " #MEAS_ID " (%s:%d) took %lld ms\n", MEAS_ID##fn, MEAS_ID##line, MEAS_ID##measTimer.elapsed()); \
    }

#define DBG_Printf(level, ...) \
    do { \
        if (DBG_IsEnabled(level)) { DBG_Printf1(level, __VA_ARGS__); } \
    } while(0)

#ifdef __cplusplus

class DebugLog
{

};

extern "C" {
#endif

DECONZ_DLLSPEC void DBG_Init(void *logfile);
DECONZ_DLLSPEC void DBG_Destroy();
DECONZ_DLLSPEC void DBG_Flush();
DECONZ_DLLSPEC void DBG_FlushLazy();
DECONZ_DLLSPEC void DBG_WriteString(int level, const char *str);
DECONZ_DLLSPEC int DBG_Printf1(int level, const char *format, ...)
#ifndef _WIN32
#ifdef DECONZ_DEBUG_BUILD
#ifdef __GNUC__
          __attribute__ (( format( printf, 2, 3 ) ))
#endif
#endif
#endif
;
DECONZ_DLLSPEC void DBG_Enable(int item);
DECONZ_DLLSPEC void DBG_Disable(int item);
DECONZ_DLLSPEC int DBG_IsEnabled(int item);
DECONZ_DLLSPEC unsigned char * DBG_HexToAscii(const void *hex, unsigned length, void *ascii);
DECONZ_DLLSPEC void DBG_RegisterCallback(void (*cb)(int, const char*));
DECONZ_DLLSPEC int DBG_ItemFromString(const char *item);
DECONZ_DLLSPEC int DBG_StringFromItem(const int item, char *buf, unsigned long buflen);

#ifdef __cplusplus
}
#endif

#endif // DECONZ_DBG_TRACE_H
