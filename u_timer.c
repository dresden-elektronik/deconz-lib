/*
 * Copyright (c) 2025 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include "deconz/u_assert.h"
#include "deconz/u_timer.h"
#include "deconz/u_memory.h"
#include "deconz/dbg_trace.h"
#include <actor/plugin.h>

enum CommonMessageIds
{
   M_ID_LIST_DIR_REQ = AM_MESSAGE_ID_COMMON_REQUEST(1),
   M_ID_LIST_DIR_RSP = AM_MESSAGE_ID_COMMON_RESPONSE(1),
   M_ID_READ_ENTRY_REQ = AM_MESSAGE_ID_COMMON_REQUEST(2),
   M_ID_READ_ENTRY_RSP = AM_MESSAGE_ID_COMMON_RESPONSE(2)
};

enum SpecificMessageIds
{
   M_ID_START_TIMER_REQ = AM_MESSAGE_ID_SPECIFIC_REQUEST(1),
   M_ID_START_TIMER_RSP = AM_MESSAGE_ID_SPECIFIC_RESPONSE(1),
   M_ID_STOP_TIMER_REQ = AM_MESSAGE_ID_SPECIFIC_REQUEST(2),
   M_ID_STOP_TIMER_RSP = AM_MESSAGE_ID_SPECIFIC_RESPONSE(2),
   M_ID_TIMEOUT_NOTIFY = AM_MESSAGE_ID_SPECIFIC_NOTIFY(3)
};

typedef struct UTimer
{
    struct UTimer *next;
    unsigned long actor_id;
    unsigned long timer_id;
    unsigned long timeout;
    unsigned long repeat;
    long long trigger_at;
} UTimer;

static struct am_actor am_actor_u_timers;
static struct am_api_functions *am;
static UTimer *u_active_timers;
static UTimer *u_free_timers;
static long long t_total;

static void U_TimerInsertTimer(UTimer *tmr)
{
    UTimer **cur = &u_active_timers;
    UTimer **prev = 0;

    if (!u_active_timers)
    {
        tmr->next = 0;
        u_active_timers = tmr;
        return;
    }

    for (; *cur;)
    {
        if (tmr->trigger_at <= (*cur)->trigger_at)
        {
            tmr->next = (*cur);
            (*cur) = tmr;
            return;
        }
        prev = cur;
        cur = &(*cur)->next;
    }

    /* insert at end */
    U_ASSERT(*prev);
    tmr->next = 0;
    (*prev)->next = tmr;
}

static int U_TimerReleaseTimer(unsigned long actor_id, unsigned long timer_id)
{
    UTimer *tmr;
    UTimer **cur = &u_active_timers; /* search addr which points to tmr */

    for (; *cur;)
    {
        if ((*cur)->actor_id == actor_id && (*cur)->timer_id == timer_id)
            break;

        cur = &(*cur)->next;
    }

    if (*cur)
    {
        tmr = *cur;
        *cur = tmr->next;
        tmr->next = u_free_timers;
        u_free_timers = tmr;
        return 1;
    }

    return 0;
}

void U_TimerSendTimeoutMessage(UTimer *tmr)
{
    struct am_message *m;

    m = am->msg_alloc();

    if (!m)
        return;

    m->dst = tmr->actor_id;
    m->src = AM_ACTOR_ID_TIMERS;
    m->id = M_ID_TIMEOUT_NOTIFY;

    am->msg_put_u16(m, 0);  /* tag */
    am->msg_put_u32(m, tmr->timer_id);

    am->send_message(m);
}

static int U_TimerStartTimerRequest(struct am_message *msg)
{
    UTimer *tmr;

    unsigned tag;
    unsigned long actor_id;
    unsigned long timer_id;
    unsigned long timeout;
    unsigned long repeat;

    tag = am->msg_get_u16(msg);
    actor_id = am->msg_get_u32(msg);
    timer_id = am->msg_get_u32(msg);
    timeout = am->msg_get_u32(msg);
    repeat = am->msg_get_u32(msg);

    if (msg->status != AM_MSG_STATUS_OK)
        return AM_CB_STATUS_INVALID;

    if (u_free_timers)
    {
        tmr = u_free_timers;
        u_free_timers = tmr->next;
    }
    else
    {
        tmr = U_Alloc(sizeof(*tmr));
    }

    tmr->actor_id = actor_id;
    tmr->timer_id = timer_id;
    tmr->timeout = timeout;
    tmr->repeat = repeat;
    tmr->trigger_at = t_total + tmr->timeout;

    U_TimerInsertTimer(tmr);

    return AM_CB_STATUS_OK;
}

static int U_TimerStopTimerRequest(struct am_message *msg)
{
    unsigned tag;
    unsigned long actor_id;
    unsigned long timer_id;

    tag = am->msg_get_u16(msg);
    actor_id = am->msg_get_u32(msg);
    timer_id = am->msg_get_u32(msg);

    if (msg->status != AM_MSG_STATUS_OK)
        return AM_CB_STATUS_INVALID;

    if (U_TimerReleaseTimer(actor_id, timer_id))
        return AM_CB_STATUS_OK;

    return AM_CB_STATUS_INVALID;
}

static int U_Timers_MessageCallback(struct am_message *msg)
{
    if (msg->id == M_ID_START_TIMER_REQ)
        return U_TimerStartTimerRequest(msg);

    if (msg->id == M_ID_STOP_TIMER_REQ)
        return U_TimerStopTimerRequest(msg);

    return AM_CB_STATUS_UNSUPPORTED;
}


void U_TimerInit(void *am_api_fn)
{
    t_total = 0;
    am = am_api_fn;

    AM_INIT_ACTOR(&am_actor_u_timers, AM_ACTOR_ID_TIMERS, U_Timers_MessageCallback);

    am->register_actor(&am_actor_u_timers);

}

void U_TimerTick(long long diff)
{
    UTimer * tmr;
    tmr = u_active_timers;

    if (0 < diff)
    {
        t_total += diff;
    }

    if (tmr)
    {
        if (tmr->trigger_at <= t_total)
        {
            DBG_Printf(DBG_INFO, "timer %lu triggerd\n", tmr->timer_id);
            u_active_timers = tmr->next;
            tmr->next = 0;

            U_TimerSendTimeoutMessage(tmr);

            if (tmr->repeat)
            {
                if (tmr->repeat != U_TIMER_REPEAT_INFINITE)
                    tmr->repeat--;

                tmr->trigger_at = t_total + tmr->timeout;
                U_TimerInsertTimer(tmr);
                return;
            }

            tmr->next = u_free_timers;
            u_free_timers = tmr;
        }
    }
}

void U_TimerDestroy(void)
{
    am = 0;
    UTimer *tmr;

    for (;u_free_timers;)
    {
        tmr = u_free_timers;
        u_free_timers = tmr->next;
        U_Free(tmr);
    }

    for (;u_active_timers;)
    {
        tmr = u_active_timers;
        u_active_timers = tmr->next;
        U_Free(tmr);
    }
}

/* Public interface */

/*! Local interface to start a timer from any thread.

    The actual timer is created in the timer actor message handler.
 */
int U_TimerStart(unsigned long actor_id, unsigned long timer_id, unsigned long timeout, unsigned long repeat)
{
    struct am_message *m;

    if (am)
    {
        m = am->msg_alloc();
        if (m)
        {
            m->src = am_actor_u_timers.actor_id;
            m->dst = am_actor_u_timers.actor_id;
            m->id = M_ID_START_TIMER_REQ;
            am->msg_put_u16(m, 0);    /* tag */
            am->msg_put_u32(m, actor_id);
            am->msg_put_u32(m, timer_id);
            am->msg_put_u32(m, timeout);
            am->msg_put_u32(m, repeat);
            am->send_message(m);
            return 1;
        }
    }

    return 0;
}

int U_TimerStop(unsigned long actor_id, unsigned long timer_id)
{
    struct am_message *m;

    if (am)
    {
        m = am->msg_alloc();
        if (m)
        {
            m->src = am_actor_u_timers.actor_id;
            m->dst = am_actor_u_timers.actor_id;
            m->id = M_ID_STOP_TIMER_REQ;
            am->msg_put_u16(m, 0);    /* tag */
            am->msg_put_u32(m, actor_id);
            am->msg_put_u32(m, timer_id);
            am->send_message(m);
            return 1;
        }
    }

    return 0;
}



