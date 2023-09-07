#ifndef DECONZ_TIMEREF_H
#define DECONZ_TIMEREF_H

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <stdint.h>
#include "deconz/declspec.h"


namespace deCONZ {


struct SteadyTimeRef
{
    SteadyTimeRef() : ref(0) {}
    SteadyTimeRef(int64_t r_) : ref(r_) {}
    int64_t ref = 0;
};

struct SystemTimeRef
{
    SystemTimeRef() : ref() {}
    SystemTimeRef(int64_t r_) : ref(r_) {}
    int64_t ref = 0;
};

/*! Strong typed milliseconds. */
struct TimeMs
{
    int64_t val = 0;
};

/*! Strong typed seconds. */
struct TimeSeconds
{
    int64_t val = 0;
};

/*! Returnes milliseconds since Epoch */
int64_t DECONZ_DLLSPEC msecSinceEpoch();

/*! Returnes a time point reference in system time as milliseconds since Epoch.

    Note that on machines without an RTC this time might jump between calls
    until system time is synced via NTP.
 */
SystemTimeRef DECONZ_DLLSPEC systemTimeRef() noexcept;

/*! Returnes a time point reference in milliseconds which is monotonic increasing.

    This is best used for timeouts and measure relative times.
 */
SteadyTimeRef DECONZ_DLLSPEC steadyTimeRef() noexcept;

inline bool isValid(SteadyTimeRef t) { return t.ref != 0; }
inline bool isValid(SystemTimeRef t) { return t.ref != 0; }

inline bool operator<(SystemTimeRef a, SystemTimeRef b) { return a.ref < b.ref; }

inline bool operator==(SteadyTimeRef a, SteadyTimeRef b) { return a.ref == b.ref; }
inline bool operator!=(SteadyTimeRef a, SteadyTimeRef b) { return a.ref != b.ref; }
inline bool operator<(SteadyTimeRef a, SteadyTimeRef b) { return a.ref < b.ref; }
inline bool operator<=(SteadyTimeRef a, SteadyTimeRef b) { return a.ref <= b.ref; }

inline TimeMs operator-(SteadyTimeRef a, SteadyTimeRef b) { TimeMs res; res.val = a.ref - b.ref; return res; }
inline SteadyTimeRef operator+(SteadyTimeRef a, TimeMs t) { return SteadyTimeRef{a.ref + t.val}; }
inline SteadyTimeRef operator+(SteadyTimeRef a, TimeSeconds t) { return SteadyTimeRef{a.ref + t.val * 1000}; }

inline bool operator<(TimeMs a, TimeMs b) { return a.val < b.val; }
inline bool operator<(TimeSeconds a, TimeSeconds b) { return a.val < b.val; }
inline bool operator<(TimeSeconds a, TimeMs b) { return a.val * 1000 < b.val; }
inline bool operator<(TimeMs a, TimeSeconds b) { return a.val < b.val * 1000; }

inline TimeSeconds operator*(TimeSeconds a, int factor) { TimeSeconds res; res.val = a.val * factor; return res; }
inline TimeMs operator*(TimeMs a, int factor) { TimeMs res; res.val = a.val * factor; return res; }

} // namespace deCONZ

#endif // DECONZ_TIMEREF_H
