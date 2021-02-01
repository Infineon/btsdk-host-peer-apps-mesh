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
#include "mesh_main.h"
#include <wiced_bt_ble.h>
#include <wiced_timer.h>
#include <android/Log.h>
#include <wiced_bt_mesh_core.h>
#include <linux/time.h>
#include "wiced_bt_gatt.h"
#include "wiced_bt_mesh_provision.h"
#include <pthread.h>
#include <android/log.h>


typedef void (TIMER_CBACK)(void *p_tle);
#define TIMER_PARAM_TYPE    void *
#define WICED_BT_MESH_TRACE_ENABLE 1

#if !defined(WICED_BT_TRACE_ENABLE)
#define WICED_BT_TRACE_ENABLE
#endif

#define WICED_BT_MESH_MODEL_TRACE(...) WICED_BT_TRACE(__VA_ARGS__)

#define  LOGY(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

//extern uint32_t SetTimer(uint32_t timer, uint32_t timeout);
extern uint32_t start_timer(int32_t timeout,  uint16_t type);
extern void stop_timer(uint32_t timer_id);

typedef struct _tle
{
    struct _tle  *p_next;
    struct _tle  *p_prev;
    TIMER_CBACK  *p_cback;
    uint32_t     idTimer;
    uint32_t      timeout;
    uint64_t      end_time;
    TIMER_PARAM_TYPE   param;
    uint16_t        type;
    uint8_t         in_use;
} TIMER_LIST_ENT;

typedef struct
{
    TIMER_LIST_ENT   *p_first;
    TIMER_LIST_ENT   *p_last;
    int             last_ticks;
} TIMER_LIST_Q;

typedef unsigned int        UINT32;
typedef signed   int        INT32;
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
uint32_t SetTimer(uint32_t timer, uint32_t timeout);
extern void mesh_provision_process_event(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);

void EnterCriticalSection();
void LeaveCriticalSection();
wiced_bool_t initTimer();

void Log(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    __android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, fmt, ap);
    va_end(ap);
}

int wiced_printf(char * buffer, int len, char * fmt_str, ...)
{
//    unsigned int i;
//    for( i=0; i<len; i++){
//        Log("%x",*fmt_str);
//        fmt_str++;
//    }

    va_list ap;
    va_start(ap, fmt_str);
    __android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, fmt_str, ap);
    va_end(ap);

    return 0;
}

void ods(char * fmt_str, ...) {
    va_list ap;
    va_start(ap, fmt_str);
    __android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, fmt_str, ap);
    va_end(ap);
}
/**
* mesh trace functions.
* These are just wrapper function for WICED trace function call. We use these
* wrapper functions to make the mesh code easier to port on different platforms.
*/
void ble_trace0(const char *p_str)
{
    Log(p_str,1);
}

void ble_trace1(const char *fmt_str, UINT32 p1)
{
    Log(fmt_str, p1);
}

void ble_trace2(const char *fmt_str, UINT32 p1, UINT32 p2)
{
    Log(fmt_str, p1, p2);
}

void ble_trace3(const char *fmt_str, UINT32 p1, UINT32 p2, UINT32 p3)
{
    Log((char *)fmt_str, p1, p2, p3);
}

void ble_trace4(const char *fmt_str, UINT32 p1, UINT32 p2, UINT32 p3, UINT32 p4)
{
    Log((char *)fmt_str, p1, p2, p3, p4);
}
//void Logn(uint8_t* data, int len)
void ble_tracen(const char *p_str, UINT32 len)
{
//    char buf[100];
//    memset(buf, 0, sizeof(buf));
//    unsigned int i;
//    for( i=0; i<len; i++) {
//        //Log("%x", *p_str);
//        LOGY("%x", *p_str);
//        p_str++;
//    }
    Logn(p_str,len);
}

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

static double TimeSpecToSeconds(struct timespec* ts)
{
    return (double)ts->tv_sec + (double)ts->tv_nsec / 1000000000.0;
//    return  (1000.0 *  (double)ts->tv_sec + (double) ts->tv_nsec / 1e6);
}

uint64_t GetTickCount64()
{

//    pthread_mutex_lock(&cs);
    uint64_t time;
    clock_gettime(CLOCK_MONOTONIC,&res);
    time = ((uint64_t)1000 * res.tv_sec) +  (res.tv_nsec/1000000);

//    pthread_mutex_unlock(&cs);
    return time;
}

/*
uint64_t GetTickCount64()
{

    uint64_t time;
    clock_gettime(CLOCK_MONOTONIC, &res);
    time =  1000.0 * res.tv_sec + (double) res.tv_nsec / 1000000;
    Log("wiced_bt_mesh_core_get_tick_count %d",res.tv_sec);
    Log("%d seconds and %ld nanoseconds\n", res.tv_sec, res.tv_nsec);
    return time;

    return 0;
}
*/
uint64_t clock_SystemTimeMicroseconds64(void)
{
    uint64_t res =  1000 * GetTickCount64();
    return res;
}

static wiced_timer_t *wiced_timer_first = NULL;

void wiced_timer_handle(uint64_t timerid)
{
    /*
    wiced_timer_t   *p_timer;
    uint64_t        curr_time = time;

    clock_gettime(CLOCK_MONOTONIC, &end);
    uint64_t elapsedSeconds = TimeSpecToSeconds(&end) - TimeSpecToSeconds(&start);

    Log("CLOCK_MONOTONIC end :%ld,  start:%ld",TimeSpecToSeconds(&end) ,TimeSpecToSeconds(&start));
    Log("%d seconds and %ld nanoseconds\n", end.tv_sec, end.tv_nsec);
    time = ((double) 1000.0 * end.tv_sec) + ((double) end.tv_nsec / 1000000);
    long accum = ( end.tv_nsec - start.tv_nsec )
                 + ( end.tv_sec - start.tv_sec ) * BILLION;
    Log("CLOCK_MONOTONIC accum time :%ld",time);
    // got through each initialized timer
    for (p_timer = wiced_timer_first; p_timer != NULL; p_timer = (wiced_timer_t*)p_timer->next)
    {
        // if it is not started or not expired then ignore it
        if (p_timer->end_time == 0 || p_timer->end_time > curr_time)
            continue;
        // update end_time
        switch (p_timer->type)
        {
        case WICED_SECONDS_PERIODIC_TIMER:
            p_timer->end_time += (uint64_t)p_timer->timeout * 1000;
            break;
        case WICED_MILLI_SECONDS_PERIODIC_TIMER:
            p_timer->end_time += p_timer->timeout;
            break;
        default:
            p_timer->end_time = 0;
            break;
        }
        //call callback
//        if (p_timer->cback)
//            p_timer->cback(p_timer->cback_param);
        //exit. Oter expired timers will be handled next time
        break;
    }
     */
}

TIMER_LIST_ENT *wiced_find_timer(uint32_t idTimer)
{
    //ods("wiced_find_timer finding ...timerid:%x \n", idTimer);
    TIMER_LIST_ENT  *p_timer;
    // got through each initialized timer
    for (p_timer = (TIMER_LIST_ENT *)wiced_timer_first; p_timer != NULL; p_timer = (TIMER_LIST_ENT*)p_timer->p_next)
    {
        ods("wiced_find_timer:%x p_timer:%x\n", p_timer->idTimer, p_timer);
        if (p_timer->idTimer == idTimer)
            return p_timer;
    }
    return NULL;
}

wiced_result_t wiced_init_timer( wiced_timer_t* p_timer, wiced_timer_callback_t TimerCb,
                                 TIMER_PARAM_TYPE cBackparam, wiced_timer_type_t type)
{
    if(!initTimer())
        return 1;

    pthread_mutex_lock(&cs);
    TIMER_LIST_ENT *p_cur;
    Log("wiced_init_timer , type = %d " , type);
    TIMER_LIST_ENT *p = (TIMER_LIST_ENT *)p_timer;
    ods("wiced_init_timer p_timer:%x\n", p_timer);

    memset(p_timer, 0, sizeof(TIMER_LIST_ENT));
    p->p_cback = TimerCb;
    p->param = cBackparam;
    p->type = type;
    p->timeout = 0;
    pthread_mutex_unlock(&cs);
    return WICED_BT_SUCCESS;

}
wiced_result_t wiced_deinit_timer(wiced_timer_t* p)
{
    ods("wiced_deinit_timer cback:%x\n", p);
    return WICED_BT_SUCCESS;

}

wiced_result_t wiced_start_timer(wiced_timer_t* wt, uint32_t timeout)
{
    ods("wiced_start_timer with timeout:%d \n",timeout);
    EnterCriticalSection();

    // Make sure that we are not starting the same timer twice.
    if (wiced_is_timer_in_use(wt))
    wiced_stop_timer(wt);

    TIMER_LIST_ENT *p_timer = (TIMER_LIST_ENT *)wt;
    if (p_timer->type == WICED_SECONDS_TIMER || p_timer->type == WICED_SECONDS_PERIODIC_TIMER)
        timeout *= 1000;
    p_timer->timeout = timeout;
    p_timer->idTimer = start_timer(timeout, p_timer->type);
    p_timer->in_use = WICED_TRUE;
    //ods("wiced_start_timer %x duration:%d\n", p_timer->idTimer, timeout);
    //ods("MeshTimerFunc callback...assigned %x id:%x",p_timer->p_cback, p_timer->idTimer);

    p_timer->p_next = (TIMER_LIST_ENT *)wiced_timer_first;
    wiced_timer_first = (wiced_timer_t *)p_timer;
    LeaveCriticalSection();
    return WICED_BT_SUCCESS;

}

uint32_t SetTimer(uint32_t timer, uint32_t timeout) {
    return 0;
}

wiced_result_t wiced_stop_timer(wiced_timer_t* wt)
{

    TIMER_LIST_ENT *p_timer = (TIMER_LIST_ENT *)wt;
    TIMER_LIST_ENT *p_cur;
    ods("wiced_stop_timer:%x\n", p_timer->idTimer);
    EnterCriticalSection();
    stop_timer( p_timer->idTimer);
	p_timer->idTimer = 0;
    p_timer->in_use = WICED_FALSE;
// unlink the timer from the list
    if (wiced_timer_first == wt)
    {
        wiced_timer_first = (wiced_timer_t *)((TIMER_LIST_ENT *)wt)->p_next;
        p_timer->p_next = NULL;
        LeaveCriticalSection();
        return WICED_BT_SUCCESS;
    }

    // got through each initialized timer
    for (p_cur = (TIMER_LIST_ENT *)wiced_timer_first; p_cur != NULL; p_cur = (TIMER_LIST_ENT*)p_cur->p_next)
    {
        if (p_cur->p_next == p_timer)
        {
            p_cur->p_next = p_timer->p_next;
            p_timer->p_next = NULL;
            LeaveCriticalSection();
            return WICED_BT_SUCCESS;
        }
    }
    ods("!!! timer not found\n");
    LeaveCriticalSection();
    return WICED_BT_SUCCESS;
}

wiced_bool_t wiced_is_timer_in_use(wiced_timer_t *p)
{
    TIMER_LIST_ENT *p_timer = (TIMER_LIST_ENT *)p;
    if (p_timer->idTimer != 0)
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
  //  ods("MeshTimerFunc start... %x",timer_id);
    TIMER_LIST_ENT *p_timer = wiced_find_timer(timer_id);
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
           // ods("MeshTimerFunc calling callback... %x",p_timer->p_cback);
            p_timer->p_cback(p_timer->param);
        }
    }
    else
    {
  //  ods("!!! Timer expired not found:%x\n", timer_id);
    }
    LeaveCriticalSection();
    ods("MeshTimerFunc end... %x",timer_id);
}


void wiced_release_timer(wiced_bt_mesh_event_t *p_event)
{
    Log("release timer");
 #if 0
    TIMER_LIST_ENT *p_timer = (TIMER_LIST_ENT *)&p_event->timer;
    if (p_timer->idTimer != 0)
    {
        ods("!!!wiced_release_timer:%x\n", p_timer->idTimer);
        wiced_stop_timer(&p_event->timer);
    }
 #endif
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

//void Logn(uint8_t* data, int len)
//{
//    int i=0;
////
////    for(i=0;i<len; i++)
////        Log("%x",data[i]);
//}

//#define LOG_TAG "MeshSecurity"


void Logn(uint8_t* data, int len)
{
    int count = 0;
    int i;
    for (i = 0; i < len; i += count){
        count = len - i;
        if (count >= 16) {
            count = 16;
            LOGY("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                data[0],  data[1],  data[2],  data[3], data[4],  data[5],  data[6],  data[7],
                data[8],  data[9],  data[10], data[11],data[12], data[13], data[14], data[15]);
        }
        else if (count == 15) {
            LOGY("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                data[0],  data[1],  data[2],  data[3], data[4],  data[5],  data[6],  data[7],
                data[8],  data[9],  data[10], data[11],data[12], data[13], data[14]);
        }
        else if (count == 14) {
            LOGY("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                data[0],  data[1],  data[2],  data[3], data[4],  data[5],  data[6],  data[7],
                data[8],  data[9],  data[10], data[11],data[12], data[13]);
        }
        else if (count == 13) {
            LOGY("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                data[0],  data[1],  data[2],  data[3], data[4],  data[5],  data[6],  data[7],
                data[8],  data[9],  data[10], data[11],data[12]);
        }
        else if (count == 12) {
            LOGY("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                data[0],  data[1],  data[2],  data[3], data[4],  data[5],  data[6],  data[7],
                data[8],  data[9],  data[10], data[11]);
        }
        else if (count == 11) {
            LOGY("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                data[0],  data[1],  data[2],  data[3], data[4],  data[5],  data[6],  data[7],
                data[8],  data[9],  data[10]);
        }
        else if (count == 10) {
            LOGY("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                data[0],  data[1],  data[2],  data[3],
                data[4],  data[5],  data[6],  data[7], data[8],  data[9]);
        }
        else if (count == 9) {
            LOGY("%02x %02x %02x %02x %02x %02x %02x %02x %02x",
                data[0],  data[1],  data[2],  data[3],
                data[4],  data[5],  data[6],  data[7], data[8]);
        }
        else if (count == 8) {
            LOGY("%02x %02x %02x %02x %02x %02x %02x %02x",
                data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
        }
        else if (count == 7) {
            LOGY("%02x %02x %02x %02x %02x %02x %02x",
                data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
        }
        else if (count == 6) {
            LOGY("%02x %02x %02x %02x %02x %02x",
                data[0], data[1], data[2], data[3], data[4], data[5]);
        }
        else if (count == 5) {
            LOGY("%02x %02x %02x %02x %02x", data[0], data[1], data[2], data[3], data[4]);
        }
        else if (count == 4) {
            LOGY("%02x %02x %02x %02x", data[0], data[1], data[2], data[3]);
        }
        else if (count == 3) {
            LOGY("%02x %02x %02x", data[0], data[1], data[2]);
        }
        else if (count == 2) {
            LOGY("%02x %02x", data[0], data[1]);
        }
        else {
            LOGY("%02x", data[0]);
        }
        data += count;
    }
}

void mesh_core_state_changed(wiced_bt_mesh_core_state_type_t type, wiced_bt_mesh_core_state_t *p_state)
{
    if (type == WICED_BT_MESH_CORE_STATE_TYPE_SEQ)
        mesh_provision_process_event(WICED_BT_MESH_SEQ_CHANGED, NULL, &p_state->seq);
    else if(type == WICED_BT_MESH_CORE_STATE_IV)
        mesh_provision_process_event(WICED_BT_MESH_IV_CHANGED, NULL, &p_state->iv);
}



void EnterCriticalSection()
{
    pthread_mutex_lock(&cs);
}

void LeaveCriticalSection()
{
    pthread_mutex_unlock(&cs);
}

wiced_bool_t initTimer()
{
    if(!timer_initialized ) {
        timer_initialized = WICED_TRUE;
        pthread_mutexattr_t Attr;
        pthread_mutexattr_init(&Attr);
        pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_RECURSIVE);

        if (pthread_mutex_init(&cs, &Attr) != 0)
        {
            printf("\n mutex init has failed\n");
            return WICED_FALSE;
        }

    }
    return WICED_TRUE;
}

void wiced_bt_ble_set_scan_mode(uint8_t is_active)
{

}
