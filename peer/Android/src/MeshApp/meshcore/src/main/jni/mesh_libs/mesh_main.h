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

/** @file
*
* Mesh Main definitions
*
*/
#ifndef __MESH_MAIN_H__
#define __MESH_MAIN_H__

#include <android/log.h>
#include <wiced.h>

#define WICED_BT_MESH_TRACE_ENABLE 1
#define WICED_BT_TRACE_ENABLE

#ifdef __cplusplus
extern "C"
{
#endif
#define BT_MEMSET memset
#define BT_MEMCPY memcpy
void wiced_timer_handle(uint64_t i);

void mesh_set_adv_params(uint8_t delay, uint8_t repeat_times, uint16_t repeat_interval, uint8_t onetime_instance, uint16_t seg_dst, uint32_t seg_mask);
void wiced_hal_wdog_reset_system(void);
uint8_t wiced_bt_gatt_disconnect(uint16_t conn_id);
typedef void(*mesh_adv_scan_cb_t)(uint8_t *bda, uint8_t *guid, uint32_t data_len, uint8_t *data);
extern wiced_bool_t mesh_adv_scan_start(void);
extern wiced_bool_t mesh_set_scan_type(uint8_t is_active);
void wiced_bt_ble_set_scan_mode(uint8_t is_active);
extern void mesh_adv_scan_stop(void);
void Logn(uint8_t* data, int len);

extern wiced_result_t mesh_transport_send_data( uint16_t opcode, uint8_t* p_data, uint16_t length );

//extern void wiced_hci_process_data( uint16_t opcode, uint8_t* p_data, uint16_t length );

#define LOG_TAG "MeshLibrary"
//#define  WICED_BT_TRACE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
void Log(const char *fmt, ...);
#define WICED_BT_TRACE Log
#define WICED_BT_TRACE_CRIT(...) wiced_printf(NULL, 0, __VA_ARGS__)
#define WICED_BT_TRACE_ARRAY(ptr, len, ...) wiced_printf(NULL, 0, __VA_ARGS__); wiced_trace_array(ptr, len);

#ifdef __cplusplus
}
#endif

#endif //__MESH_MAIN_H__
