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
 * Header file for Mesh libraries trace functions.
 */

#ifndef MESHAPP_TRACE_H
#define MESHAPP_TRACE_H

#include <stdarg.h>
#include <string.h>
#include "platform.h"

// debug trace control, defined in build tool preprocessor macros.
#ifdef WICED_BT_MESH_TRACE_ENABLE
#define WICED_BT_TRACE_ENABLE 1
#endif

#define MESH_CORE_LOG_FILE_NAME "meshcore.log"
void mesh_trace_log_init(int is_console_enabled);
void set_log_file_path(char* path);
void open_log_file(void);
void close_log_file(void);

void ods(char *fmt, ...);
void Log(char *fmt, ...);
void Logn(uint8_t* data, int len);
int wiced_printf(char * buffer, int len, char *fmt_str, ...);
void wiced_trace_array(const uint8_t *p_array, uint16_t len);
#ifdef WICED_BT_TRACE_ENABLE
#define WICED_BT_TRACE(...)                     wiced_printf(NULL, 0, __VA_ARGS__)
#define WICED_BT_TRACE_CRIT(...)                wiced_printf(NULL, 0, __VA_ARGS__)
#define WICED_BT_TRACE_ARRAY(ptr, len, ...)     wiced_printf(NULL, 0, __VA_ARGS__); wiced_trace_array(ptr, len);
#else
#define WICED_BT_TRACE(...)
#define WICED_BT_TRACE_CRIT(...)
#define WICED_BT_TRACE_ARRAY(ptr, len, ...)
#endif  // #ifdef WICED_BT_TRACE_ENABLE

/**
 * Smart mesh trace functions to increase the code portability
 */
void ble_trace0(const char *p_str);
void ble_trace1(const char *fmt_str, UINT32 p1);
void ble_trace2(const char *fmt_str, UINT32 p1, UINT32 p2);
void ble_trace3(const char *fmt_str, UINT32 p1, UINT32 p2, UINT32 p3);
void ble_trace4(const char *fmt_str, UINT32 p1, UINT32 p2, UINT32 p3, UINT32 p4);
void ble_tracen(const char *p_str, UINT32 len);
void LOG0(int debug, const char *p_str);
void LOG1(int debug, const char *fmt_str, uint32_t p1);
void LOG2(int debug, const char *fmt_str, uint32_t p1, uint32_t p2);
void LOG3(int debug, const char *fmt_str, uint32_t p1, uint32_t p2, uint32_t p3);
void LOG4(int debug, const char *fmt_str, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);
void LOG5(int debug, const char *fmt_str, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4, uint32_t p5);
void LOG6(int debug, const char *fmt_str, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4, uint32_t p5, uint32_t p6);
void LOGN(int debug, char *p_str, uint32_t len);

#endif  // MESHAPP_TRACE_H
