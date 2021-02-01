/*
 * Copyright 2016-2021, Cypress Semiconductor Corporation (an Infineon company) or
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
 * Mesh Main implementation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#if defined(__ANDROID__)
#  include <android/Log.h>
#  include <linux/time.h>
#elif defined(__APPLE__)
#  include <stdarg.h>
#  include <unistd.h>
#  include <time.h>
#  include <sys/time.h>
#  ifndef HAVE_CLOCK_GETTIME    // aimed to suppot platforms: iOS *.* < iOS 10.0
#    include <mach/clock.h>
#    include <mach/mach.h>
#  endif
#endif
#include "mesh_main.h"
#include "wiced_bt_ble.h"
#include "wiced_timer.h"
#include "wiced_bt_mesh_core.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_mesh_provision.h"

typedef void (TIMER_CBACK)(void *p_tle);
#undef TIMER_PARAM_TYPE
#define TIMER_PARAM_TYPE    void *

extern uint32_t start_timer(int32_t timeout,  uint16_t type);
extern void stop_timer(uint32_t timer_id);
extern void mesh_provision_process_event(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);

#define IS_TIMER_ENABLED(p, f)    ((((p)->flags) & (f)) == (f))
#define TIMER_FLAG_ACTIVE       0x0001
typedef struct _tle
{
    struct _tle         *p_next;
    TIMER_CBACK         *p_cback;   /* Timer expiration callback function. */
    TIMER_PARAM_TYPE    param;      /* Parameter for the expiration callback function. */
    uint32_t            idTimer;    /* Uniquely identifies a timer in the system. */
    uint32_t            timeout;    /* Periodical timeout interval in 1ms unit. */
    uint16_t            type;       /* Timer type, see wiced_timer_type_t definitions. */
    uint16_t            flags;      /* Flags for timer. */
} TIMER_LIST_ENT;   // Must make sure the size of TIMER_LIST_ENT is equal or less than the size of wiced_timer_t.

#define BILLION  1000000000L;
pthread_mutex_t cs = PTHREAD_MUTEX_INITIALIZER;
static wiced_bool_t timer_initialized = WICED_FALSE;
extern wiced_bool_t meshClientTimerCallback(int start, int timeout);
extern void wiced_bt_ble_set_scan_mode(uint8_t is_active);
//TODO CHECK WITH VICTOR : currrently this is used by ivi_recovery_start in the same file
typedef void(*ivi_recovery_start_cb_t)(wiced_bool_t res);

extern wiced_bool_t mesh_adv_scan_start(void);
extern wiced_bool_t mesh_bt_gatt_le_connect(wiced_bt_device_address_t bd_addr, wiced_bt_ble_address_type_t bd_addr_type,
    wiced_bt_ble_conn_mode_t conn_mode, wiced_bool_t is_direct);
extern wiced_bool_t mesh_bt_gatt_le_disconnect(uint32_t conn_id);
extern uint32_t restart_timer(uint32_t timeout, uint32_t timer_id);

void* wiced_memory_allocate(UINT32 length)
{
    return malloc(length);
}
void* wiced_memory_permanent_allocate(UINT32 length)
{
    return malloc(length);
}
void wiced_memory_free(void *memoryBlock)
{
    free(memoryBlock);
}

void *wiced_bt_get_buffer(uint16_t len)
{
    return malloc(len);
}
void wiced_bt_free_buffer(void* buffer)
{
    free(buffer);
}

struct timespec res;
struct timespec start;
struct timespec end;

uint64_t GetTickCount64()
{
    uint64_t time;
#if defined(__APPLE__)
    if (__builtin_available(iOS 10.0, *)) {
        clock_gettime(CLOCK_MONOTONIC,&res);
    } else {
        // aimed to support platforms: iOS *.* < iOS 10.0
        // clock_gettime only support in iOS 10.0 and newer.
        // The support of the clock_gettime API can be checked based macro HAVE_CLOCK_GETTIME.
        struct timespec ts;
        clock_serv_t cclock;
        mach_timespec_t mts;
        host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
        clock_get_time(cclock, &mts);
        mach_port_deallocate(mach_task_self(), cclock);
        ts.tv_sec = mts.tv_sec;
        ts.tv_nsec = mts.tv_nsec;
    }
#else
    clock_gettime(CLOCK_MONOTONIC,&res);
#endif
    time = ((uint64_t)1000 * res.tv_sec) +  (res.tv_nsec/1000000);

    return time;
}

uint64_t clock_SystemTimeMicroseconds64(void)
{
    uint64_t res =  1000 * GetTickCount64();
    return res;
}

static wiced_timer_t *wiced_timer_first = NULL;

TIMER_LIST_ENT *wiced_find_timer(uint32_t idTimer)
{
    TIMER_LIST_ENT  *p_timer;
    // got through each initialized timer
    for (p_timer = (TIMER_LIST_ENT *)wiced_timer_first; p_timer != NULL; p_timer = (TIMER_LIST_ENT*)p_timer->p_next)
    {
        if (p_timer->idTimer == idTimer)
        {
            //ods("wiced_find_timer, found timer p_timer:0x%lx, idTimer=%u\n", p_timer, p_timer->idTimer);
            return p_timer;
        }
    }

    //ods("!!! wiced_find_timer, not found queued timer with idTimer=%u\n", idTimer);
    return NULL;
}

wiced_bool_t is_queued_timer(wiced_timer_t *wt)
{
    TIMER_LIST_ENT *p_timer;
    for (p_timer = (TIMER_LIST_ENT *)wiced_timer_first; p_timer != NULL; p_timer = p_timer->p_next)
    {
        if (p_timer == (TIMER_LIST_ENT *)wt)
        {
            return WICED_TRUE;
        }
    }
    return WICED_FALSE;
}

wiced_result_t wiced_init_timer(wiced_timer_t *wt, wiced_timer_callback_t TimerCb,
                                TIMER_PARAM_TYPE cBackparam, wiced_timer_type_t type)
{
    EnterCriticalSection();
    if(!initTimer())
    {
        LeaveCriticalSection();
        return WICED_NOT_AVAILABLE;
    }

    TIMER_LIST_ENT *p_timer = (TIMER_LIST_ENT *)wt;
    if (is_queued_timer(wt)) {
        // should not go to here.
        ods("!!! wiced_init_timer, init queued timer, p_timer:0x%lx, type=%d, idTimer=%u, timeout=%u, p_cback=0x%lx, flags=0x%04x, p_next=0x%lx\n", p_timer, p_timer->type, p_timer->idTimer, p_timer->timeout, p_timer->p_cback, p_timer->flags, p_timer->p_next);
        wiced_deinit_timer(wt);
    }

    //ods("wiced_init_timer, p_timer:0x%lx, type=%d, TimerCb=0x%lx, cBackparam=0x%lx\n", p_timer, type, TimerCb, cBackparam);
    memset(p_timer, 0, sizeof(TIMER_LIST_ENT));
    p_timer->p_cback = TimerCb;
    p_timer->param = cBackparam;
    p_timer->type = type;

    //ods("wiced_init_timer, p_timer:0x%lx\n", p_timer);
    LeaveCriticalSection();
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_deinit_timer(wiced_timer_t* wt)
{
    EnterCriticalSection();
    // Make sure that we are not running the timer and removed from the timer list.
    if (wiced_is_timer_in_use(wt))
    {
        ods("!!! wiced_deinit_timer, p_timer:0x%lx is in using, trying to stop it firstly\n", wt);
        wiced_stop_timer(wt);
    }
    //ods("wiced_deinit_timer, p_timer:0x%lx\n", wt);
    memset(wt, 0, sizeof(TIMER_LIST_ENT));
    LeaveCriticalSection();
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_start_timer(wiced_timer_t *wt, uint32_t timeout)
{
    EnterCriticalSection();

    TIMER_LIST_ENT *p_timer = (TIMER_LIST_ENT *)wt;
    if (p_timer->type == WICED_SECONDS_TIMER || p_timer->type == WICED_SECONDS_PERIODIC_TIMER)
    {
        timeout *= 1000;
    }

    // Make sure that we are not starting the same timer twice.
    // So, try to restart it with new timeout value if it was possible.
    if (p_timer->idTimer != 0)
    {
        if (is_queued_timer(wt))
        {
            ods("!!! wiced_start_timer, restart timer p_timer:0x%lx, type=%d, idTimer=%u, timeout=%u, p_cback=0x%lx, flags=0x%04x, p_next=0x%lx\n", p_timer, p_timer->type, p_timer->idTimer, timeout, p_timer->p_cback, p_timer->flags, p_timer->p_next);
            p_timer->idTimer = restart_timer(timeout, p_timer->idTimer);
            if (p_timer->idTimer != 0)
            {
                p_timer->timeout = timeout;
                p_timer->flags &= TIMER_FLAG_ACTIVE;

                LeaveCriticalSection();
                return WICED_SUCCESS;
            }
            else
            {
                ods("!!! wiced_start_timer, restart timer p_timer:0x%lx failed\n", p_timer);
            }
        }

        // try to stop the timer firstly when the timer has been running.
        wiced_stop_timer(wt);
    }

    p_timer->idTimer = start_timer(timeout, p_timer->type);
    if (p_timer->idTimer == 0)
    {
        ods("!!! wiced_start_timer, failed to do start_timer, p_timer=0x%lx\n", p_timer);
        return WICED_START_ERROR;
    }
    p_timer->timeout = timeout;
    p_timer->flags &= TIMER_FLAG_ACTIVE;
    p_timer->p_next = (TIMER_LIST_ENT *)wiced_timer_first;
    wiced_timer_first = wt;
    //ods("wiced_start_timer, p_timer:0x%lx, type=%d, idTimer=%u, timeout=%u, p_cback=0x%lx, flags=0x%04x, p_next=0x%lx\n", p_timer, p_timer->type, p_timer->idTimer, timeout, p_timer->p_cback, p_timer->flags, p_timer->p_next);

    LeaveCriticalSection();
    return WICED_SUCCESS;
}

wiced_result_t wiced_stop_timer(wiced_timer_t* wt)
{
    TIMER_LIST_ENT *p_timer = (TIMER_LIST_ENT *)wt;
    TIMER_LIST_ENT *p_cur;

    EnterCriticalSection();
    //ods("wiced_stop_timer, p_timer:0x%lx, type=%d, idTimer=%u, flags=0x%04x, p_next=0x%lx\n", p_timer, p_timer->type, p_timer->idTimer, p_timer->flags, p_timer->p_next);
    if (!is_queued_timer(wt))
    {
        if (p_timer->idTimer != 0)
        {
            stop_timer(p_timer->idTimer);
        }
        p_timer->flags &= ~TIMER_FLAG_ACTIVE;
        LeaveCriticalSection();
        return WICED_BT_SUCCESS;
    }

    stop_timer(p_timer->idTimer);
    p_timer->flags &= ~TIMER_FLAG_ACTIVE;
    //ods("wiced_stop_timer, p_timer:0x%lx, type=%d, idTimer=%u, flags=0x%04x, p_next=0x%lx\n", p_timer, p_timer->type, p_timer->idTimer, p_timer->flags, p_timer->p_next);

    // unlink the timer from the list
    if (wiced_timer_first == wt)
    {
        wiced_timer_first = (wiced_timer_t *)(p_timer->p_next);
        p_timer->p_next = NULL;
        LeaveCriticalSection();
        return WICED_BT_SUCCESS;
    }

    // got through each initialized timer
    for (p_cur = (TIMER_LIST_ENT *)wiced_timer_first; p_cur != NULL; p_cur = (TIMER_LIST_ENT *)p_cur->p_next)
    {
        if (p_cur->p_next == p_timer)
        {
            p_cur->p_next = p_timer->p_next;
            p_timer->p_next = NULL;
            LeaveCriticalSection();
            return WICED_BT_SUCCESS;
        }
    }

    ods("!!! wiced_stop_timer, not found in queued timer list, p_timer:0x%lx, type=%d, idTimer=%u, flags=0x%04x, p_next=0x%lx\n", p_timer, p_timer->type, p_timer->idTimer, p_timer->flags, p_timer->p_next);
    LeaveCriticalSection();
    return WICED_NO_INSTANCE;
}

wiced_bool_t wiced_is_timer_in_use(wiced_timer_t *wt)
{
    TIMER_LIST_ENT *p_timer = (TIMER_LIST_ENT *)wt;
    if (is_queued_timer(wt) || ((p_timer->idTimer != 0) && IS_TIMER_ENABLED(p_timer, TIMER_FLAG_ACTIVE)))
    {
        return WICED_TRUE;
    }
    else
    {
        return WICED_FALSE;
    }
}

void MeshTimerFunc(long timer_id)
{
    EnterCriticalSection();

    //ods("MeshTimerFunc, timer triggerred for timer_id=%ld", timer_id);
    TIMER_LIST_ENT *p_timer = wiced_find_timer((uint32_t)timer_id);
    if (p_timer != NULL)
    {
        if ((p_timer->type != WICED_SECONDS_PERIODIC_TIMER) && (p_timer->type != WICED_MILLI_SECONDS_PERIODIC_TIMER))
        {
            wiced_stop_timer((wiced_timer_t *)p_timer);
        } else {
            restart_timer(p_timer->timeout, p_timer->idTimer);
        }

        if (p_timer->p_cback)
        {
            //ods("MeshTimerFunc, invoking the timer callback p_timer=0x%lx, timer_id=%ld, p_cback=0x%lx param=0x%lx\n", p_timer, p_timer->idTimer, p_timer->p_cback, p_timer->param);
            p_timer->p_cback(p_timer->param);
        }
    }
    else
    {
        ods("!!! timer expired not found for timer_id=%ld\n", timer_id);
    }

    LeaveCriticalSection();
}

// empty functions not needed in MeshController

void mesh_discovery_start(void)
{
}

void mesh_update_beacon(void)
{
}

void wiced_hal_wdog_reset_system(void)
{
}

void mesh_discovery_stop(void)
{
}

void provision_gatt_send(uint16_t conn_id, const uint8_t *packet, uint32_t packet_len)
{
}

void mesh_provisioner_hci_event_scan_report_send(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_scan_report_data_t *p_scan_report_data)
{

}

/*
* Send Proxy Device Network Data event over transport
*/
void mesh_provisioner_hci_event_proxy_device_send(wiced_bt_mesh_proxy_device_network_data_t *p_data)
{

}

wiced_bt_dev_status_t wiced_bt_ble_observe(wiced_bool_t start, uint8_t duration, wiced_bt_ble_scan_result_cback_t *p_scan_result_cback)
{
    if (start)
        mesh_adv_scan_start();
    else
        mesh_adv_scan_stop();

    return WICED_SUCCESS;
}

wiced_bt_gatt_status_t wiced_bt_gatt_register(wiced_bt_gatt_cback_t *p_gatt_cback)
{
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_gatt_le_connect(wiced_bt_device_address_t bd_addr,
    wiced_bt_ble_address_type_t bd_addr_type,
    wiced_bt_ble_conn_mode_t conn_mode,
    wiced_bool_t is_direct)
{
    ods("wiced_bt_gatt_le_connect\n");
    return mesh_bt_gatt_le_connect(bd_addr, bd_addr_type, conn_mode, is_direct);
}

wiced_bool_t wiced_bt_gatt_cancel_connect(wiced_bt_device_address_t bd_addr, wiced_bool_t is_direct)
{
    return WICED_TRUE;
}


wiced_bt_gatt_status_t wiced_bt_gatt_disconnect(uint16_t conn_id)
{
    return mesh_bt_gatt_le_disconnect(conn_id);
}

wiced_bt_gatt_status_t wiced_bt_gatt_configure_mtu(uint16_t conn_id, uint16_t mtu)
{
    return WICED_BT_GATT_SUCCESS;
}

wiced_bt_gatt_status_t wiced_bt_util_send_gatt_discover(uint16_t conn_id, wiced_bt_gatt_discovery_type_t type, uint16_t uuid, uint16_t s_handle, uint16_t e_handle)
{
    return WICED_BT_GATT_SUCCESS;
}

wiced_bt_gatt_status_t wiced_bt_util_set_gatt_client_config_descriptor(uint16_t conn_id, uint16_t handle, uint16_t value)
{
    return WICED_BT_GATT_SUCCESS;
}

inline void EnterCriticalSection()
{
    pthread_mutex_lock(&cs);
}

inline void LeaveCriticalSection()
{
    pthread_mutex_unlock(&cs);
}

wiced_bool_t initTimer()
{
    if(!timer_initialized ) {
        if (sizeof(wiced_timer_t) < sizeof(TIMER_LIST_ENT)) {
            // used to check the data size is correctly, because the wiced_timer_t data size may be udpated and cause inconsistent issue.
            ods("\nerror: initTimer, invalid wiced_time_t size:%lu < TIMER_LIST_ENT size:%lu\n", sizeof(wiced_timer_t), sizeof(TIMER_LIST_ENT));
            ods("please check the WICED_TIMER_INSTANCE_SIZE_IN_WORDS value in wiced_timer.h or update the TIMER_LIST_ENT to fix the new size.\n");
            return WICED_FALSE;
        }

        pthread_mutexattr_t Attr;
        pthread_mutexattr_init(&Attr);
        pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_RECURSIVE);
        if (pthread_mutex_init(&cs, &Attr) != 0)
        {
            ods("\nerror: initTimer, failed to initialize recursive mutex lock\n");
            return WICED_FALSE;
        }
        timer_initialized = WICED_TRUE;
    }
    return WICED_TRUE;
}

void wiced_bt_ble_set_scan_mode(uint8_t is_active)
{

}
