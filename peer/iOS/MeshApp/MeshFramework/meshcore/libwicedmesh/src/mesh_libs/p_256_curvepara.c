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
 * Simple pairing algorithms implementation
 */

#include "bt_target.h"
#include "wiced_bt_app_common.h"

#if SMP_LE_SC_INCLUDED == WICED_TRUE

#include <string.h>
#include "p_256_ecc_pp.h"

void p_256_init_curve(uint32_t keyLength)
{
    EC *ec;

    if(keyLength == KEY_LENGTH_DWORDS_P256)
    {
        ec = &curve_p256;

        ec->p[7] = 0xFFFFFFFF;
        ec->p[6] = 0x00000001;
        ec->p[5] = 0x0;
        ec->p[4] = 0x0;
        ec->p[3] = 0x0;
        ec->p[2] = 0xFFFFFFFF;
        ec->p[1] = 0xFFFFFFFF;
        ec->p[0] = 0xFFFFFFFF;

        memset(ec->omega, 0, KEY_LENGTH_DWORDS_P256);  // omega is not used anyway
        memset(ec->a, 0, KEY_LENGTH_DWORDS_P256);

        ec->a_minus3 = WICED_TRUE;

        //b
        ec->b[7] =  0x5ac635d8;
        ec->b[6] =  0xaa3a93e7;
        ec->b[5] =  0xb3ebbd55;
        ec->b[4] =  0x769886bc;
        ec->b[3] =  0x651d06b0;
        ec->b[2] =  0xcc53b0f6;
        ec->b[1] =  0x3bce3c3e;
        ec->b[0] =  0x27d2604b;

        //base point
        ec->G.x[7] =  0x6b17d1f2;
        ec->G.x[6] =  0xe12c4247;
        ec->G.x[5] =  0xf8bce6e5;
        ec->G.x[4] =  0x63a440f2;
        ec->G.x[3] =  0x77037d81;
        ec->G.x[2] =  0x2deb33a0;
        ec->G.x[1] =  0xf4a13945;
        ec->G.x[0] =  0xd898c296;

        ec->G.y[7] =  0x4fe342e2;
        ec->G.y[6] =  0xfe1a7f9b;
        ec->G.y[5] =  0x8ee7eb4a;
        ec->G.y[4] =  0x7c0f9e16;
        ec->G.y[3] =  0x2bce3357;
        ec->G.y[2] =  0x6b315ece;
        ec->G.y[1] =  0xcbb64068;
        ec->G.y[0] =  0x37bf51f5;
    }
}

#endif
