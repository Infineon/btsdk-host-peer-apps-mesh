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
* meshdb.h : header file
*
*/

#pragma once

#include <stdint.h>

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "wiced_bt_mesh_db.h"

/*
 * Read Mesh Object from the file stream
 */
wiced_bt_mesh_db_mesh_t *mesh_json_read_file(FILE *fp);

/*
* Write Mesh Object to the file stream
*/
void mesh_json_write_file(FILE *fp, wiced_bt_mesh_db_mesh_t *p_mesh);

/*
 * Add net key to the node
 */
wiced_bool_t mesh_db_add_node_net_key(wiced_bt_mesh_db_node_t *node, wiced_bt_mesh_db_key_idx_phase *net_key_idx);

/*
 * Add application key to the node
 */
wiced_bool_t mesh_db_add_node_app_key(wiced_bt_mesh_db_node_t *node, wiced_bt_mesh_db_key_idx_phase *app_key_idx);

/*
 * Add model to application key binding
 */
wiced_bool_t mesh_db_add_model_app_bind(wiced_bt_mesh_db_model_t *model, uint16_t app_key_idx);

/*
 * Add model subscription address
 */
wiced_bool_t mesh_db_add_model_sub(wiced_bt_mesh_db_model_t *model, wiced_bt_mesh_db_address_t *addr);

/*
 * Delete address from model subscription
 */
wiced_bool_t mesh_db_delete_model_sub(wiced_bt_mesh_db_model_t *model, uint16_t addr);

/*
* Add scene address
*/
wiced_bool_t mesh_db_add_scene_address(wiced_bt_mesh_db_scene_t* scene, uint16_t addr);

/*
* Read mesh proprietary parameters from the file stream
*/
void mesh_extra_params_read_file(FILE *fp, wiced_bt_mesh_db_mesh_t *p_mesh);

/*
* Write mesh proprietary parameters to the file stream
*/
void mesh_extra_params_write_file(FILE *fp, wiced_bt_mesh_db_mesh_t *p_mesh);
