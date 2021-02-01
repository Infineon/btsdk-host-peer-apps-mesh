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
#include "platform.h"

/**
 * Smart mesh trace functions to increase the code portability
 */
extern void LOG0(int debug, const char *p_str);
extern void LOG1(int debug, const char *fmt_str, uint32_t p1);
extern void LOG2(int debug, const char *fmt_str, uint32_t p1, uint32_t p2);
extern void LOG3(int debug, const char *fmt_str, uint32_t p1, uint32_t p2, uint32_t p3);
extern void LOG4(int debug, const char *fmt_str, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);
extern void LOG5(int debug, const char *fmt_str, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4,
                 uint32_t p5);
extern void LOG6(int debug, const char *fmt_str, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4,
                 uint32_t p5, uint32_t p6);
extern void LOGN(int debug, char *p_str, uint32_t len);
