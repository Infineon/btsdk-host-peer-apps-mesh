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

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

// if it is FW build then define MESH_FW
#if 0
#define MESH_FW
#endif
#ifndef MESH_OVER_GATT_ONLY
#include "wiced_bt_ble.h"
#ifdef MESH_FW
#define wiced_memory_permanent_allocate wiced_bt_get_buffer
#define MULTI_ADV_MAX_NUM_INSTANCES LE_MULTI_ADV_MAX_NUM_INSTANCES
enum
{
    WICED_BT_ADV_NOTIFICATION_READY,
    WICED_BT_ADV_NOTIFICATION_DONE
};
#define wiced_timer_callback_t wiced_timer_callback_legacy_t
#define p_256_init_curve(x) InitCurve(x)
#define MULTI_ADV_TX_POWER_MAX MULTI_ADV_TX_POWER_MAX_INDEX
#define TIMER_PARAM_TYPE WICED_TIMER_PARAM_TYPE
#endif

#ifdef WICEDX_LINUX
#define smp_aes_encrypt aes_encrypt
#define smp_aes_set_key aes_set_key
#else
#define aes_set_key smp_aes_set_key
#define aes_encrypt smp_aes_encrypt
#endif

#else

#if !defined __ANDROID__ && !defined __APPLE__
void* memcpy(void* _Dst, void const* _Src, unsigned int _Size);
#endif

#if defined __ANDROID__ || defined __APPLE__
#include "stdint.h"
#endif


#ifndef UINT32
typedef unsigned int UINT32;
#endif
#if defined(__APPLE__) || defined(__ANDROID__)
typedef unsigned int DWORD;
#else
typedef unsigned long DWORD;
#endif
typedef unsigned char UINT8;
typedef unsigned char BYTE;
typedef unsigned short UINT16;
#ifndef __APPLE__
#ifndef BOOL
typedef int BOOL;
#endif
#endif  // __APPLE__

#define TRUE 1
#define FALSE 0
typedef int INT32;
#define BT_MEMSET memset
#define BT_MEMCPY memcpy
typedef unsigned long long UINT64;
void* wiced_memory_permanent_allocate(UINT32 length);
void wiced_memory_free(void *memoryBlock);
#define smp_aes_set_key aes_set_key
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;

#if !defined __ANDROID__ && !defined __APPLE__
typedef unsigned long long uint64_t;
typedef signed long long int64_t;
#endif

typedef unsigned int   wiced_bool_t;
#define WICED_TRUE 1
#define WICED_FALSE 0
#define BTM_INQ_RES_IGNORE_RSSI     0x7f    /**< RSSI value not supplied (ignore it) */
#ifndef NULL
    #ifdef __cplusplus
        #define NULL 0
    #else
        #define NULL ((void *)0)
    #endif
#endif
void *wiced_bt_get_buffer(uint16_t len);
void wiced_bt_free_buffer(void* buffer);
uint64_t clock_SystemTimeMicroseconds64(void);

#ifndef BD_ADDR
typedef uint8_t BD_ADDR[6];
#endif

#define wiced_bt_device_address_t BD_ADDR

#ifndef wiced_bt_ble_address_type_t
typedef uint8_t wiced_bt_ble_address_type_t;
#endif

#define smp_aes_encrypt aes_encrypt

#define MAX_UUID_SIZE              16


#endif

#endif /* __PLATFORM_H__ */
