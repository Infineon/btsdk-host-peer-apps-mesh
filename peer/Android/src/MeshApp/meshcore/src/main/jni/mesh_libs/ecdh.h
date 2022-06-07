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
/**
* Generates ECDH public key from private key.
*
* Parameters:
*   priv_key:   ECDH private key(length WICED_BT_MESH_PROVISION_PRIV_KEY_LEN)
*   pub_key:    Buffer for generated ECDH public key (length WICED_BT_MESH_PROVISION_PUBLIC_KEY_LEN)
*
* Return:   None
*
*/
extern void ecdh_create_pub_key(const unsigned char* priv_key, unsigned char* pub_key);

/**
* Generates ECDH shared secret from peer public key and own private key.
*
* Parameters:
*   peer_pub_key:   ECDH public key of the peer device (length WICED_BT_MESH_PROVISION_PUBLIC_KEY_LEN).
*   priv_key:       own ECDH private key (length WICED_BT_MESH_PROVISION_PRIV_KEY_LEN)
*   ecdh_secret:    Buffer for generated ECDH shared secret(length WICED_BT_MESH_PROVISION_ECDH_SECRET_LEN)
*
* Return:   None
*
*/
extern void ecdh_calc_secret(const unsigned char* peer_pub_key, const unsigned char* priv_key, unsigned char* ecdh_secret);

/**
* Initializes of ECDH parameters.
*
* Parameters:   None
*
* Return:   None
*
*/
extern void ecdh_init_curve(void);

/**
* Validates public key:
*     - Verifies x and y are in range [0,p-1]
*     - Verifies that y^2 == x^3 + ax + b (mod p)
* Parameters:
*   pub_key:    Public key
*
* Return:   WICED_TRUE if public key is valid
*/
wiced_bool_t ecdh_validate_pub_key(const uint8_t* pub_key);
