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
 * Mesh libraries trace functions.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#if defined(__ANDROID__)
#  include <android/Log.h>
#  include <linux/time.h>
#elif defined(__APPLE__)
#  include <stdarg.h>
#  include <limits.h>
#  include <unistd.h>
#  include <time.h>
#  include <sys/time.h>
#  ifndef HAVE_CLOCK_GETTIME    // aimed to suppot platforms: iOS *.* < iOS 10.0
#    include <mach/clock.h>
#    include <mach/mach.h>
#  endif
#endif
#include "trace.h"

#define LOG_TAG "MeshJni"

// Max 20M for each single log file. It's big enought to record any signle meash operation.
#define LOG_FILE_MAX_SIZE (20 * 1024 * 1024)
static char* log_file_path = NULL;
static char log_file_name[] = MESH_CORE_LOG_FILE_NAME;
static FILE *log_fp = NULL;
static int is_debug_enabled = TRUE;

void mesh_trace_log_init(int is_console_enabled)
{
    is_debug_enabled = is_console_enabled;
}

void set_log_file_path(char* path)
{
    if (log_file_path) {
        free(log_file_path);
        log_file_path = NULL;
    }
    if (path == NULL) {
        // user current working path as log path.
        char buffer[PATH_MAX];
        getcwd(buffer, PATH_MAX);
        path = buffer;
    }
    size_t len = strlen(path);
    if (path[len - 1] == '/') {
        path[len - 1] = '\0';
    }
    len = strlen(path) + strlen(log_file_name) + 2;
    log_file_path = malloc(len);
    snprintf(log_file_path, len, "%s/%s", path, log_file_name);
    Log("setting log_file_path: %s", log_file_path);
}

void open_log_file()
{
    if (log_file_path == NULL) {
        Log("error: log_file_path not initialized yet, ignore\n");
        return;
    }
    if (log_fp) {
        fclose(log_fp);
    }
    log_fp = fopen(log_file_path, "a");
    if (log_fp == NULL) {
        Log("error: failed to open log_file_path: %s\n", log_file_path);
    }
    Log("opened log_file_path: %s", log_file_path);
}

void close_log_file()
{
    if (log_fp != NULL) {
        Log("closed log_file_path: %s", log_file_path);
        fclose(log_fp);
        log_fp = NULL;
    } else {
        Log("closed log_file_path: NULL");
    }
}

void print_current_time()
{
    struct timeval tv;
    char log_file_bak_name[PATH_MAX];

    if (log_fp && (ftell(log_fp) > LOG_FILE_MAX_SIZE)) {
        // remove previous backup log file and rename current log file as backup log file.
        fflush(log_fp);
        fclose(log_fp);
        snprintf(log_file_bak_name, PATH_MAX, "%s.bak", log_file_path);
        remove(log_file_bak_name);
        rename(log_file_path, log_file_bak_name);

        // re-create the log file.
        open_log_file();
        if (log_fp == NULL) {
            // try again if failed to re-create the log file.
            fflush(NULL);
            open_log_file();
        }
    }

    if (gettimeofday(&tv, NULL) == 0)
    {
        if (is_debug_enabled) printf("%ld.%06d ", tv.tv_sec, tv.tv_usec);
        if (log_fp) fprintf(log_fp, "%ld.%06d ", tv.tv_sec, tv.tv_usec);
    }
    else
    {
        if (is_debug_enabled) printf("%ld.%06d ", 0L, 0);
        if (log_fp) fprintf(log_fp, "%ld.%06d ", 0L, 0);
    }
}

#ifdef __ANDROID__
#define  LOGY(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#else
#define  LOGY(...)  if (is_debug_enabled) { printf(__VA_ARGS__); printf("\n"); } if (log_fp) { fprintf(log_fp, __VA_ARGS__); fprintf(log_fp, "\n"); }
#endif
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

void Log(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
#ifdef __ANDROID__
    __android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, fmt, ap);
#else
    va_list aplist;
    va_start(aplist, fmt);
    print_current_time();
    if (is_debug_enabled) vprintf(fmt, ap);
    if (log_fp) vfprintf(log_fp, fmt, aplist);
    if (fmt && fmt[strlen(fmt) - 1] != '\n')
    {
        if (is_debug_enabled) printf("\n");
        if (log_fp) fprintf(log_fp, "\n");
    }
    va_end(aplist);
#endif
    va_end(ap);
}

void ods(char * fmt_str, ...)
{
    va_list ap;
    va_start(ap, fmt_str);
#ifdef __ANDROID__
    __android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, fmt_str, ap);
#else
    va_list aplist;
    va_start(aplist, fmt_str);
    print_current_time();
    if (is_debug_enabled) vprintf(fmt_str, ap);
    if (log_fp) vfprintf(log_fp, fmt_str, aplist);
    if (fmt_str && fmt_str[strlen(fmt_str) - 1] != '\n')
    {
        if (is_debug_enabled) printf("\n");
        if (log_fp) fprintf(log_fp, "\n");
    }
    va_end(aplist);
#endif
    va_end(ap);
}

int wiced_printf(char * buffer, int len, char *fmt_str, ...)
{
#ifdef __ANDROID__
    unsigned int i;
    for( i=0; i<len; i++){
        Log("%x",*fmt_str);
        fmt_str++;
    }
#else
    print_current_time();
    va_list ap;
    va_start(ap, fmt_str);
    va_list aplist;
    va_start(aplist, fmt_str);
    if (buffer && len)
        vsnprintf(buffer, len, fmt_str, ap);
    else
        if (is_debug_enabled) vprintf(fmt_str, ap);
    if (log_fp)
        vfprintf(log_fp, fmt_str, aplist);
    va_end(aplist);
    va_end(ap);
#endif
    return 0;
}

void wiced_trace_array(const uint8_t *p_array, uint16_t len)
{
    Logn((uint8_t *)p_array,len);
}

/**
* mesh trace functions.
* These are just wrapper function for WICED trace function call. We use these
* wrapper functions to make the mesh code easier to port on different platforms.
*/
void ble_trace0(const char *p_str)
{
    Log((char *)p_str,1);
}

void ble_trace1(const char *fmt_str, UINT32 p1)
{
    Log((char *)fmt_str, p1);
}

void ble_trace2(const char *fmt_str, UINT32 p1, UINT32 p2)
{
    Log((char *)fmt_str, p1, p2);
}

void ble_trace3(const char *fmt_str, UINT32 p1, UINT32 p2, UINT32 p3)
{
    Log((char *)fmt_str, p1, p2, p3);
}

void ble_trace4(const char *fmt_str, UINT32 p1, UINT32 p2, UINT32 p3, UINT32 p4)
{
    Log((char *)fmt_str, p1, p2, p3, p4);
}

void ble_tracen(const char *p_str, UINT32 len)
{
    Logn((uint8_t *)p_str,len);
}

void LOG0(int debug, const char *p_str)
{
    if (debug)
        Log((char *)p_str);
}

void LOG1(int debug, const char *fmt_str, uint32_t p1)
{
    if (debug)
        Log((char *)fmt_str, p1);
}

void LOG2(int debug, const char *fmt_str, uint32_t p1, uint32_t p2)
{
    if (debug)
        Log((char *)fmt_str, p1, p2);
}

void LOG3(int debug, const char *fmt_str, uint32_t p1, uint32_t p2, uint32_t p3)
{
    if (debug)
        Log((char *)fmt_str, p1, p2, p3);
}

void LOG4(int debug, const char *fmt_str, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4)
{
    if (debug)
        Log((char *)fmt_str, p1, p2, p3, p4);
}

void LOG5(int debug, const char *fmt_str, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4, uint32_t p5)
{
    if (debug)
        Log((char *)fmt_str, p1, p2, p3, p4, p5);
}

void LOG6(int debug, const char *fmt_str, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4, uint32_t p5,
          uint32_t p6)
{
    if (debug)
        Log((char *)fmt_str, p1, p2, p3, p4, p5, p6);
}

void LOGN(int debug, char *p_str, uint32_t len)
{
    if (debug)
        Logn((uint8_t *)p_str, (int)len);
}
