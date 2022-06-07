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
#ifndef WIN_DATA_TYPES_H
#define WIN_DATA_TYPES_H

#ifndef __windows__
typedef unsigned int DWORD;
typedef wchar_t WCHAR;
//typedef unsigned int UINT32;
typedef unsigned char UINT8;
typedef unsigned char BYTE;
typedef unsigned int    BOOL;
typedef unsigned char * LPBYTE;
typedef unsigned long LRESULT;
typedef unsigned long WPARAM;
typedef unsigned long LPARAM ;
typedef unsigned short USHORT;
typedef void * HANDLE;
typedef unsigned int wiced_bool_t;
typedef unsigned int UINT_PTR;
//typedef int INT32;
typedef unsigned short UINT16;
typedef unsigned int    BOOL32;

//#ifndef __APPLE__
#if !defined __APPLE__ && !defined __windows__
typedef unsigned long int uint64_t;
#endif
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#define FALSE   0
#define TRUE   1
#else
#include "windows.h"
#endif
#endif // WIN_DATA_TYPES_H
