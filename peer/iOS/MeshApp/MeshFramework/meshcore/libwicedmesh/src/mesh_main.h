/*
 * Copyright 2016-2020, Cypress Semiconductor Corporation or a subsidiary of
 * Cypress Semiconductor Corporation. All Rights Reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software"), is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
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
 * Mesh Main definitions
 */

#ifndef __MESH_MAIN_H__
#define __MESH_MAIN_H__

#ifdef __ANDROID__
#include <android/log.h>
#endif
#include "wiced.h"
#include "bt_types.h"
#include "trace.h"

#define LOG_TAG "MeshLibrary"

#ifdef __cplusplus
extern "C"
{
#endif
#define BT_MEMSET memset
#define BT_MEMCPY memcpy
void MeshTimerFunc(long timer_id);

void mesh_set_adv_params(uint8_t delay, uint8_t repeat_times, uint16_t repeat_interval, uint8_t onetime_instance, uint16_t seg_dst, uint32_t seg_mask);
void wiced_hal_wdog_reset_system(void);
uint8_t wiced_bt_gatt_disconnect(uint16_t conn_id);
typedef void(*mesh_adv_scan_cb_t)(uint8_t *bda, uint8_t *guid, uint32_t data_len, uint8_t *data);
extern wiced_bool_t mesh_adv_scan_start(void);
extern wiced_bool_t mesh_set_scan_type(uint8_t is_active);
void wiced_bt_ble_set_scan_mode(uint8_t is_active);
extern void mesh_adv_scan_stop(void);

extern wiced_result_t mesh_transport_send_data( uint16_t opcode, uint8_t* p_data, uint16_t length );

/* through all the meshcore library, must use these APIs to protocol data. Such as for timer threads, and APIs call in and call out */
wiced_bool_t initTimer(void);   // This API must be called firstly before using the EnterCriticalSection and LeaveCriticalSection APIs.
void EnterCriticalSection(void);
void LeaveCriticalSection(void);

void setDfuFilePath(char* dfuFilepath);
char *getDfuFilePath(void);

#ifdef __cplusplus
}
#endif

#endif //__MESH_MAIN_H__
