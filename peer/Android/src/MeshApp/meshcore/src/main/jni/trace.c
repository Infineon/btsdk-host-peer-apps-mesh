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
#include <android/log.h>
#include <platform.h>
#include <stdio.h>

#ifndef MESHAPP_TRACE_H
#define MESHAPP_TRACE_H

#define  Log(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define LOG_TAG "MeshJni"
#endif //MESHAPP_TRACE_H

void LOG0(int debug, const char *p_str)
{
    //Log(0,p_str);
}

void LOG1(int debug, const char *fmt_str, uint32_t p1)
{
    //Log(fmt_str, p1);
}

void LOG2(int debug, const char *fmt_str, uint32_t p1, uint32_t p2)
{
   // Log(fmt_str, p1, p2);
}

void LOG3(int debug, const char *fmt_str, uint32_t p1, uint32_t p2, uint32_t p3)
{
    //Log(fmt_str, p1, p2, p3);
}

void LOG4(int debug, const char *fmt_str, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4)
{
    //Log(fmt_str, p1, p2, p3, p4);
}

void LOG5(int debug, const char *fmt_str, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4, uint32_t p5)
{
   // Log(fmt_str, p1, p2, p3, p4, p5);
}

void LOG6(int debug, const char *fmt_str, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4, uint32_t p5,
          uint32_t p6)
{
   // Log(fmt_str, p1, p2, p3, p4, p5, p6);
}

void LOGN(int debug, char *p_str, uint32_t len)
{
//    int i;
//    for( i=0; i<len; i++){
//        Log("%x",*p_str);
//        p_str++;
//    }
}

#if 0
int WICED_BT_TRACE(char * buffer, int len, char * fmt_str, ...)
{
    char buf[2048];
    va_list va;
    va_start(va, fmt_str);
    LOG(buf, sizeof(buf), fmt_str, va);
    va_end(va);
    return 0;
}
#endif
