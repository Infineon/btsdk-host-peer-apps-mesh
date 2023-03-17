/*
* Copyright 2016-2023, Cypress Semiconductor Corporation (an Infineon company) or
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
* Provides additional defines for the application
*/

#ifndef __ADD_DEFINES_H__
#define __ADD_DEFINES_H__

//#define WICED_BT_SUCCESS    0
//#define WICED_BT_ERROR      1

#define LE2TOUINT12(p) (((uint32_t)(p)[0] + (((uint32_t)(p)[1])<<8)) & 0x0fff)
#define LE2TOUINT12_2(p) (((uint32_t)(p)[0] + (((uint32_t)(p)[1])<<8)) >> 4)
#define BE2TOUINT16(p) ((uint16_t)(p)[1] + (((uint16_t)(p)[0])<<8))
#define LE2TOUINT16(p) ((uint32_t)(p)[0] + (((uint32_t)(p)[1])<<8))
#define LE3TOUINT32(p) ((uint32_t)(p)[0] + (((uint32_t)(p)[1])<<8) + (((uint32_t)(p)[2])<<16))

#define BIT16_TO_8( val ) \
    (uint8_t)(  (val)        & 0xff),/* LSB */ \
    (uint8_t)(( (val) >> 8 ) & 0xff) /* MSB */

#endif /* __ADD_DEFINES_H__*/
