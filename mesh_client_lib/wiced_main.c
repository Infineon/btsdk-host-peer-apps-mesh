/*
* Copyright 2022-2023, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*/

/** @file
*
* WICED functions implementation for host/peer applications
*/

#include <windows.h>

#include "wiced_timer.h"

typedef void (TIMER_CBACK)(void *p_tle);
extern void execute_timer_callback(TIMER_CBACK *p_callback, TIMER_PARAM_TYPE arg);

#define TIMER_ACTIVE          0x0001

typedef struct _tle
{
    struct _tle  *p_next;
    TIMER_CBACK  *p_cback;
    UINT16        flags;                /* Flags for timer*/
    UINT16        type;
    UINT32        interval;             /* Periodical time out inteval, in 1 us unit */
    TIMER_PARAM_TYPE arg;               /* parameter for expiration function */
    UINT64        target_time;          /* Target time for timer expire, in us */
} TIMER_LIST_ENT;

static TIMER_LIST_ENT *pOsTimerHead = NULL;
static UINT64       absTimeHi = 0;
static void timerThread(void *arg);
static HANDLE   sleepHandle;

extern void ods(char * fmt_str, ...);

extern CRITICAL_SECTION cs;

wiced_result_t wiced_init_timer(wiced_timer_t* p_timer, wiced_timer_callback_t TimerCb, TIMER_PARAM_TYPE cBackparam, wiced_timer_type_t type)
{
    TIMER_LIST_ENT *p = (TIMER_LIST_ENT *)p_timer;

    ods("wiced_init_timer p_timer:%x\n", p_timer);

    memset(p_timer, 0, sizeof(TIMER_LIST_ENT));
    p->p_cback  = TimerCb;
    p->arg      = cBackparam;
    p->type     = type;
    p->interval = 0;

    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_deinit_timer(wiced_timer_t* p)
{
    ods("wiced_deinit_timer:%x\n", p);
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_start_timer(wiced_timer_t* wt, uint32_t timeout)
{
    TIMER_LIST_ENT *p_timer = (TIMER_LIST_ENT *)wt;
    TIMER_LIST_ENT  *p_cur, *p_prev;
    UINT32          cur_tc, expire_tc;

    // This could be done more elegantly in a final product...
    static BOOL threadStarted = FALSE;
    if (!threadStarted)
    {
        DWORD   thread_address;
        threadStarted = TRUE;

        sleepHandle = CreateEvent (NULL, FALSE, FALSE, NULL);

        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)timerThread, (LPVOID)1, 0, &thread_address);
    }

    if (!p_timer->p_cback)
    {
        ods("wiced_start_timer timer not initialized\n");
        return WICED_BT_ERROR;
    }

    EnterCriticalSection(&cs);

    // ods("wiced_start_timer:%x timeout:%d\n", p_timer, timeout);

    // Make sure that we are not starting the same timer twice.
    wiced_stop_timer(wt);

    p_timer->interval = timeout;

    if (p_timer->type == WICED_SECONDS_TIMER || p_timer->type == WICED_SECONDS_PERIODIC_TIMER)
        timeout *= 1000;

    cur_tc = GetTickCount();
    expire_tc = cur_tc + timeout;   // interval is in milliseconds, convert to ms

    p_timer->target_time = absTimeHi + expire_tc;

    // Check for rollover of the 32-bit tick count
    if (expire_tc < cur_tc)
        p_timer->target_time += 0x100000000;

    // Put the timer in the appropriate place
    if ((pOsTimerHead == NULL) || (pOsTimerHead->target_time > p_timer->target_time))
    {
        // New timer goes at start of list
        p_timer->p_next = pOsTimerHead;
        pOsTimerHead = p_timer;
    }
    else
    {
        p_prev = pOsTimerHead;
        p_cur = p_prev->p_next;

        for (; ; )
        {
            if (p_cur == NULL)
            {
                // New timer goes at end of list
                p_prev->p_next = p_timer;
                p_timer->p_next = NULL;
                break;
            }
            if (p_cur->target_time > p_timer->target_time)
            {
                // New timer goes in the middle of the list
                p_prev->p_next  = p_timer;
                p_timer->p_next = p_cur;
                break;
            }
            p_prev = p_cur;
            p_cur = p_prev->p_next;
        }
    }

    p_timer->flags |= TIMER_ACTIVE;

    LeaveCriticalSection(&cs);

    SetEvent (sleepHandle);

    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_stop_timer(wiced_timer_t* wt)
{
    TIMER_LIST_ENT *p_timer = (TIMER_LIST_ENT *)wt;
    TIMER_LIST_ENT *pt;

    // ods("wiced_stop_timer:%x\n", p_timer);

    EnterCriticalSection(&cs);

    p_timer->flags &= ~TIMER_ACTIVE;

    if (p_timer == pOsTimerHead)
    {
        pOsTimerHead = pOsTimerHead->p_next;
        LeaveCriticalSection(&cs);
        SetEvent (sleepHandle);
        return WICED_BT_SUCCESS;
    }

    /* Find timer in the queue */
    for (pt = pOsTimerHead; pt != NULL; pt = pt->p_next)
        if (pt->p_next == p_timer)
            break;

    if (pt != NULL)
        pt->p_next = p_timer->p_next;

    LeaveCriticalSection(&cs);

    SetEvent (sleepHandle);

    return WICED_BT_SUCCESS;
}

wiced_bool_t wiced_is_timer_in_use(wiced_timer_t *p)
{
    TIMER_LIST_ENT *p_timer = (TIMER_LIST_ENT *)p;
    if (p_timer->flags & TIMER_ACTIVE)
        return WICED_TRUE;
    else
        return WICED_FALSE;
}

static void timerThread(void *arg)
{
    UINT32          last_tc, cur_tc, sleep_time;
    UINT64          cur_absTime;
    TIMER_LIST_ENT  *pTimer;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    for ( ; ; )
    {
        last_tc = GetTickCount();
        cur_absTime = absTimeHi + last_tc;

        if (pOsTimerHead != NULL)
        {
            if (pOsTimerHead->target_time < cur_absTime)
                sleep_time = 0;
            else
                sleep_time = (UINT32)(pOsTimerHead->target_time - cur_absTime);
        }
        else
            sleep_time = 60000;        // 1 minute is small enough to detect rollovers

        if (sleep_time != 0)
            WaitForSingleObject (sleepHandle, sleep_time);

        EnterCriticalSection(&cs);

        cur_tc = GetTickCount();

        // Check for rollover - this assumes no timer is more than 49 days
        if (cur_tc < last_tc)
            absTimeHi += 0x100000000;

        cur_absTime = absTimeHi + cur_tc;

        // Check if the first timer on the list has expired
        if ((pOsTimerHead != NULL) && (pOsTimerHead->target_time <= cur_absTime))
        {
            pTimer = pOsTimerHead;

            // Check for periodic timer
            if ((pTimer->type == WICED_SECONDS_PERIODIC_TIMER) || (pTimer->type == WICED_MILLI_SECONDS_PERIODIC_TIMER))
                wiced_start_timer ((wiced_timer_t *)pTimer, pTimer->interval);
            else
                wiced_stop_timer ((wiced_timer_t *)pTimer);

            LeaveCriticalSection(&cs);

            execute_timer_callback(pTimer->p_cback, pTimer->arg);
        }
        else
        {
            LeaveCriticalSection(&cs);
        }
    }
}
