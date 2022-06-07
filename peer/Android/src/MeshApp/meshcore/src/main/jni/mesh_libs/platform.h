/*
 * Copyright 2016-2022, Cypress Semiconductor Corporation (an Infineon company) or
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
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

// if it is FW build then define MESH_FW_20735
#if 0 //ASIC
#define MESH_FW_20735
#endif
#ifndef MESH_OVER_GATT_ONLY
#include "wiced_bt_ble.h"
#ifdef MESH_FW_20735
#include "wiced_bt_mesh_defs.h"
#endif
#else

#ifndef __ANDROID__
void* memcpy(void* _Dst, void const* _Src, unsigned int _Size);
#endif

#ifdef __ANDROID__
#include "stdint.h"
#endif

#ifndef UINT32
typedef unsigned int UINT32;
#endif
#if defined(__ANDROID__)
typedef unsigned int DWORD;
#else
typedef unsigned long DWORD;
#endif
typedef unsigned char UINT8;
typedef unsigned char BYTE;
typedef unsigned short UINT16;
#if 1
#ifndef BOOL
typedef int BOOL;
//typedef unsigned char BOOL;
#endif
#endif

#define TRUE 1
#define FALSE 0
typedef int INT32;
#define BT_MEMSET memset
#define BT_MEMCPY memcpy
typedef unsigned long long UINT64;
void* wiced_memory_allocate(UINT32 length);
void* wiced_memory_permanent_allocate(UINT32 length);
void wiced_memory_free(void *memoryBlock);
#define smp_aes_set_key aes_set_key
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
#ifndef __ANDROID__
typedef unsigned long long uint64_t;
typedef signed long long int64_t;
#endif
//typedef void(*wiced_timer_callback_fp)(uint32_t cBackparam);
//enum wiced_timer_type_e
//{
//    WICED_SECONDS_TIMER = 1,
//    WICED_MILLI_SECONDS_TIMER, /* The minimum resolution supported is 1 ms */
//    WICED_SECONDS_PERIODIC_TIMER,
//    WICED_MILLI_SECONDS_PERIODIC_TIMER /*The minimum resolution supported is 1 ms */
//};
//typedef UINT8 wiced_timer_type_t;/* (see #wiced_timer_type_e) */
//typedef struct
//{
//    wiced_timer_callback_fp cback;
//    uint32_t                cback_param;
//    wiced_timer_type_t      type;
//    uint32_t                timeout;
//    uint64_t                end_time;
//    void                    *next;  //wiced_timer_t*
//} wiced_timer_t;
//typedef int wiced_bool_t;
typedef unsigned int   wiced_bool_t;
#define WICED_TRUE 1
#define WICED_FALSE 0
//typedef uint16_t wiced_result_t;
//#define WICED_ERROR 4
//wiced_result_t wiced_init_timer(wiced_timer_t* p_timer, wiced_timer_callback_fp TimerCb, uint32_t cBackparam, wiced_timer_type_t type);
//wiced_result_t wiced_start_timer(wiced_timer_t* p_timer, uint32_t timeout);
//wiced_result_t wiced_stop_timer(wiced_timer_t* p_timer);
//UINT32 mesh_read_node_info(int inx, UINT8* node_info, UINT16 len, wiced_result_t *p_result);
//UINT32 mesh_write_node_info(int inx, const UINT8* node_info, UINT16 len, wiced_result_t *p_result);
#define BTM_INQ_RES_IGNORE_RSSI     0x7f    /**< RSSI value not supplied (ignore it) */
//#define WICED_BT_SUCCESS    0
//#define WICED_BT_ERROR      1
#ifndef NULL
    #ifdef __cplusplus
        #define NULL 0
    #else
        #define NULL ((void *)0)
    #endif
#endif
void *wiced_bt_get_buffer(uint32_t len);
void wiced_bt_free_buffer(void* buffer);

uint64_t clock_SystemTimeMicroseconds64(void);

#ifndef BD_ADDR
typedef uint8_t BD_ADDR[6];
#endif

#define wiced_bt_device_address_t BD_ADDR

#ifndef wiced_bt_ble_address_type_t
typedef uint8_t wiced_bt_ble_address_type_t;
#endif

#include "mesh_main.h"

//#define UINT8_TO_STREAM(p, u8)   {*(p)++ = (uint8_t)(u8);}
//#define STREAM_TO_UINT8(u8, p)   {u8 = (uint8_t)(*(p)); (p) += 1;}
//#define UINT16_TO_STREAM(p, u16) {*(p)++ = (uint8_t)(u16); *(p)++ = (uint8_t)((u16) >> 8);}
//#define UINT32_TO_STREAM(p, u32) {*(p)++ = (uint8_t)(u32); *(p)++ = (uint8_t)((u32) >> 8); *(p)++ = (uint8_t)((u32) >> 16); *(p)++ = (uint8_t)((u32) >> 24);}
//#define STREAM_TO_UINT16(u16, p) {u16 = ((uint16_t)(*(p)) + (((uint16_t)(*((p) + 1))) << 8)); (p) += 2;}
//#define STREAM_TO_UINT32(u32, p) {u32 = (((uint32_t)(*(p))) + ((((uint32_t)(*((p) + 1)))) << 8) + ((((uint32_t)(*((p) + 2)))) << 16) + ((((uint32_t)(*((p) + 3)))) << 24)); (p) += 4;}
//#define ARRAY_TO_STREAM(p, a, len) {register int ijk; for (ijk = 0; ijk < len;        ijk++) *(p)++ = (UINT8) a[ijk];}
//#define STREAM_TO_ARRAY(a, p, len) {register int ijk; for (ijk = 0; ijk < len; ijk++) ((UINT8 *) a)[ijk] = *p++;}
//#define STREAM_TO_UINT24(u32, p) {u32 = (((UINT32)(*(p))) + ((((UINT32)(*((p) + 1)))) << 8) + ((((UINT32)(*((p) + 2)))) << 16) ); (p) += 3;}
//#define UINT8_TO_BE_STREAM(p, u8)   {*(p)++ = (UINT8)(u8);}
//#define UINT16_TO_BE_STREAM(p, u16) {*(p)++ = (UINT8)((u16) >> 8); *(p)++ = (UINT8)(u16);}
//#define BE_STREAM_TO_UINT8(u8, p)   {u8 = (UINT8)(*(p)); (p) += 1;}
//#define BE_STREAM_TO_UINT16(u16, p) {u16 = (UINT16)(((UINT16)(*(p)) << 8) + (UINT16)(*((p) + 1))); (p) += 2;}

#define MAX_UUID_SIZE              16

//enum wiced_bt_ble_advert_mode_e
//{
//    BTM_BLE_ADVERT_OFF,                 /**< Stop advertising */
//    BTM_BLE_ADVERT_DIRECTED_HIGH,       /**< Directed advertisement (high duty cycle) */
//    BTM_BLE_ADVERT_DIRECTED_LOW,        /**< Directed advertisement (low duty cycle) */
//    BTM_BLE_ADVERT_UNDIRECTED_HIGH,     /**< Undirected advertisement (high duty cycle) */
//    BTM_BLE_ADVERT_UNDIRECTED_LOW,      /**< Undirected advertisement (low duty cycle) */
//    BTM_BLE_ADVERT_NONCONN_HIGH,        /**< Non-connectable advertisement (high duty cycle) */
//    BTM_BLE_ADVERT_NONCONN_LOW,         /**< Non-connectable advertisement (low duty cycle) */
//    BTM_BLE_ADVERT_DISCOVERABLE_HIGH,   /**< discoverable advertisement (high duty cycle) */
//    BTM_BLE_ADVERT_DISCOVERABLE_LOW     /**< discoverable advertisement (low duty cycle) */
//};
//
#endif

#endif /* __PLATFORM_H__ */
