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

/** @file
* wiced_bt_mesh_db.h : Header file for mesh db operations
*
*/

#ifndef WICED_BT_MESH_DB__H
#define WICED_BT_MESH_DB__H

#include <wiced_bt_mesh_models.h>
#include "stdint.h"
#ifdef DIRECTED_FORWARDING_SERVER_SUPPORTED
#include "wiced_bt_mesh_mdf.h"
#endif

typedef unsigned int wiced_bool_t;
#define WICED_TRUE  1
#define WICED_FALSE 0

#define WICED_MESH_DB_UUID_SIZE     16
#define WICED_MESH_DB_KEY_SIZE      16

typedef struct
{
    uint32_t iv_index;          /**< IV Index */
    uint8_t  iv_update;         /**< IV-UPDATE flag. Can be 0 or 1 */
} mesh_client_iv_t;

typedef struct
{
    uint8_t  previous_iv_idx;       /**< 0 - it is SEQ for current IV INDEX. 1 - for previous. If addr is 0 then it is ignored. */
    uint8_t  seq[3];                /**< Little Endian Sequence Number (SEQ). */
    uint16_t addr;
} mesh_client_seq_t;

typedef struct
{
    char *name;
    uint16_t index;
    uint8_t  key[WICED_MESH_DB_KEY_SIZE];
    uint8_t  old_key[WICED_MESH_DB_KEY_SIZE];
    uint8_t  phase;
#define MIN_SECURITY_HIGH   0
#define MIN_SECURITY_LOW    1
    uint8_t  min_security;
} wiced_bt_mesh_db_net_key_t;

typedef struct
{
    char *name;
    uint16_t index;
    uint16_t bound_net_key_index;
    uint8_t  key[WICED_MESH_DB_KEY_SIZE];
    uint8_t  old_key[WICED_MESH_DB_KEY_SIZE];
} wiced_bt_mesh_db_app_key_t;

typedef struct
{
    char *name;
    uint16_t addr;
    uint16_t parent_addr;
} wiced_bt_mesh_db_group_t;

typedef struct
{
    uint16_t low_addr;
    uint16_t high_addr;
} wiced_bt_mesh_db_range_t;

typedef struct
{
    char *name;
    uint8_t uuid[16];
    uint16_t num_allocated_group_ranges;
    wiced_bt_mesh_db_range_t *p_allocated_group_range;
    uint16_t num_allocated_unicast_ranges;
    wiced_bt_mesh_db_range_t *p_allocated_unicast_range;
    uint16_t num_allocated_scene_ranges;
    wiced_bt_mesh_db_range_t *p_allocated_scene_range;
} wiced_bt_mesh_db_provisioner_t;

typedef struct
{
    uint16_t count;
    uint32_t interval;
} wiced_bt_mesh_db_transmit_t;

typedef struct
{
    uint16_t address;
    uint16_t index;
    uint8_t  ttl;
    uint32_t period;
    wiced_bt_mesh_db_transmit_t retransmit;
    uint8_t  credentials;
} wiced_bt_mesh_db_publication_t;

typedef struct
{
    uint16_t company_id;
    uint16_t id;
} wiced_bt_mesh_db_model_id_t;

#define MODEL_ID_TO_UIN32(x) (((uint32_t)(x).company_id << 16) + (x.id))

typedef struct
{
    uint8_t  positive_tolerance_percentage;
    uint8_t  negative_tolerance_percentage;
    uint8_t  sampling_function;
    uint8_t  measurement_period;
    uint8_t  update_interval;
} wiced_bt_mesh_db_descriptor_t;

typedef struct
{
    uint16_t      property_id;
    uint16_t      fast_cadence_period_divisor;
    uint8_t       trigger_type;
    uint32_t      trigger_delta_down;
    uint32_t      trigger_delta_up;
    uint32_t      min_interval;
    uint32_t      fast_cadence_low;
    uint32_t      fast_cadence_high;
} wiced_bt_mesh_db_cadence_t;

typedef struct
{
    uint16_t setting_property_id;
    uint8_t  access;
    uint8_t*  val;
} wiced_bt_mesh_db_setting_t;

typedef struct
{
    uint16_t property_id;
    uint8_t prop_value_len;
    uint8_t num_settings;
    wiced_bt_mesh_db_descriptor_t descriptor;
    wiced_bt_mesh_db_cadence_t cadence;
    wiced_bt_mesh_db_setting_t *settings;
} wiced_bt_mesh_db_sensor_t;

typedef struct
{
    wiced_bt_mesh_db_model_id_t model;
    uint8_t  num_subs;
    uint16_t *sub;
    uint8_t  num_bound_keys;
    uint16_t *bound_key;
    wiced_bt_mesh_db_publication_t pub;
    uint8_t num_sensors;
    wiced_bt_mesh_db_sensor_t *sensor;
} wiced_bt_mesh_db_model_t;

typedef struct
{
    char *name;
    uint8_t index;
    uint16_t location;
    uint8_t num_models;
    wiced_bt_mesh_db_model_t *model;
} wiced_bt_mesh_db_element_t;

#define MESH_FEATURE_DISABLED           0
#define MESH_FEATURE_ENABLED            1
#define MESH_FEATURE_UNSUPPORTED        2
#define MESH_FEATURE_SUPPORTED_UNKNOWN  255

typedef struct
{
    uint8_t relay;
    uint8_t gatt_proxy;
    uint8_t private_gatt_proxy;
    uint8_t low_power;
    uint8_t friend;
} wiced_bt_mesh_db_features_t;

typedef struct
{
    uint16_t index;
    uint8_t  phase;
} wiced_bt_mesh_db_key_idx_phase;

typedef struct
{
    char *name;
    uint16_t unicast_address;
    uint8_t  uuid[16];
    uint8_t  device_key[16];
    uint8_t  security;
    uint8_t  config_complete;
    uint8_t  blocked;
    uint8_t  num_hops;
    uint16_t cid;
    uint16_t pid;
    uint16_t vid;
    uint16_t crpl;
    wiced_bt_mesh_db_features_t feature;
    uint8_t  num_net_keys;
    wiced_bt_mesh_db_key_idx_phase *net_key;
    uint8_t  num_app_keys;
    wiced_bt_mesh_db_key_idx_phase *app_key;
    uint8_t  default_ttl;
    uint8_t  beacon;
    uint8_t  private_beacon;
    wiced_bt_mesh_db_transmit_t network_transmit;
    wiced_bt_mesh_db_transmit_t relay_rexmit;
#ifdef DIRECTED_FORWARDING_SERVER_SUPPORTED
    wiced_bt_mesh_df_state_control_t    df;
#endif
    uint8_t  num_elements;
    wiced_bt_mesh_db_element_t *element;

#define SCANNING_STATE_IDLE                                 0
#define SCANNING_STATE_GETTING_INFO                         1
#define SCANNING_STATE_STARTING                             2
#define SCANNING_STATE_SCANNING                             3
#define SCANNING_STATE_STOPPING                             4
    uint8_t     scanning_state;
    uint8_t     active_scan_supported;

    uint8_t  random_update_interval;
    uint8_t  on_demand_private_proxy;
} wiced_bt_mesh_db_node_t;

typedef struct
{
    char *name;
    uint16_t addr;
    uint16_t number;
} wiced_bt_mesh_db_scene_t;

typedef struct
{
    char *name;
    uint8_t uuid[WICED_MESH_DB_UUID_SIZE];
    uint32_t timestamp;
    uint32_t iv_index;
    uint8_t iv_update;
    uint8_t num_net_keys;
    wiced_bt_mesh_db_net_key_t *net_key;
    uint8_t num_app_keys;
    wiced_bt_mesh_db_app_key_t *app_key;
    uint8_t num_provisioners;
    wiced_bt_mesh_db_provisioner_t *provisioner;
    uint16_t unicast_addr;
    uint16_t num_nodes;
    wiced_bt_mesh_db_node_t *node;
    uint16_t num_groups;
    wiced_bt_mesh_db_group_t *group;
    uint16_t num_scenes;
    wiced_bt_mesh_db_scene_t *scene;
    uint32_t solicitation_seq_num;
} wiced_bt_mesh_db_mesh_t;

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Check if network wiht specified name already exists
 */
wiced_bool_t wiced_bt_mesh_db_network_exists(const char *mesh_name);

/*
 * Allocate memeory for the mesh object and read data from the database
 */
wiced_bt_mesh_db_mesh_t *wiced_bt_mesh_db_init(const char *p_filename);

/*
 * Release memeory associated with the current database
 */
void wiced_bt_mesh_db_deinit(wiced_bt_mesh_db_mesh_t *mesh_db);

/*
 * Store current database.  The name of the file is contsructed from the mesh_db-?name field
 */
void wiced_bt_mesh_db_store(wiced_bt_mesh_db_mesh_t *mesh_db);

/*
 * Delete mesh database with the specified name
 */
wiced_bool_t wiced_bt_mesh_db_network_delete(const char *mesh_name);

/*
 * Return number of network keys in the network
 */
uint8_t wiced_bt_mesh_db_num_net_keys(wiced_bt_mesh_db_mesh_t *p_mesh_db);

/*
 * Add network key
 */
wiced_bool_t wiced_bt_mesh_db_net_key_add(wiced_bt_mesh_db_mesh_t *p_mesh, wiced_bt_mesh_db_net_key_t *key);

/*
 * Find network key with specified index
 */
wiced_bt_mesh_db_net_key_t *wiced_bt_mesh_db_net_key_get(wiced_bt_mesh_db_mesh_t *mesh_db, uint8_t index);

/*
 * Return number of application keys in the network
 */
uint8_t wiced_bt_mesh_db_num_app_keys(wiced_bt_mesh_db_mesh_t *mesh_db);

/*
 * Find application key with specified name
 */
wiced_bt_mesh_db_app_key_t *wiced_bt_mesh_db_app_key_get_by_name(wiced_bt_mesh_db_mesh_t *mesh_db, const char *p_name);

/*
 * Find application key with specified app key index
 */
wiced_bt_mesh_db_app_key_t *wiced_bt_mesh_db_app_key_get_by_key_index(wiced_bt_mesh_db_mesh_t *p_db, uint16_t app_key_idx);

/*
 * Get application key with specified index
 */
wiced_bt_mesh_db_app_key_t *wiced_bt_mesh_db_app_key_get(wiced_bt_mesh_db_mesh_t *mesh_db, uint8_t index);

/*
 * Add application key
 */
wiced_bool_t wiced_bt_mesh_db_app_key_add(wiced_bt_mesh_db_mesh_t *mesh_db, wiced_bt_mesh_db_app_key_t *app_key);

/*
 * Return number of provisioners in the network
 */
uint8_t wiced_bt_mesh_db_num_provisioners(wiced_bt_mesh_db_mesh_t *mesh_db);

/*
 * Add a provisioner to the network.  Returns unicast address assigned to the provisioner.
 */
uint16_t wiced_bt_mesh_db_provisioner_add(wiced_bt_mesh_db_mesh_t *mesh_db, const char *name, uint8_t *uuid, uint8_t *dev_key);

/*
 * Find provisioner based on the provisioner UUID
 */
wiced_bt_mesh_db_provisioner_t *wiced_bt_mesh_db_provisioner_get_by_uuid(wiced_bt_mesh_db_mesh_t *mesh_db, const uint8_t *uuid);

/*
 * Find provisioner based on the provisioner address
 */
wiced_bt_mesh_db_provisioner_t *wiced_bt_mesh_db_provisioner_get_by_addr(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t addr);

uint16_t wiced_bt_mesh_db_alloc_unicast_addr(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t provisioner_addr, uint8_t num_elements, uint8_t *db_changed);

/*
 * Store in the database new net key that has been added to the device
 */
wiced_bool_t wiced_bt_mesh_db_node_net_key_add(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t unicast_addr, uint16_t net_key_idx);

/*
 * Store in the database new app key that has been added to the device
 */
wiced_bool_t wiced_bt_mesh_db_node_app_key_add(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t unicast_addr, uint16_t net_key_idx, uint16_t app_key_idx);

/*
 * Return pointer to the net key to which the app key is bound to
 */
wiced_bt_mesh_db_net_key_t *wiced_bt_mesh_db_find_bound_net_key(wiced_bt_mesh_db_mesh_t *mesh_db, wiced_bt_mesh_db_app_key_t *app_key);

/*
 * Store in the database mesh to application key binding
 */
wiced_bool_t wiced_bt_mesh_db_node_model_app_bind_add(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t unicast_addr, uint16_t element_addr, uint16_t company_id, uint16_t model_id, uint16_t app_key_idx);

/*
 * The net key on the device has been updated during the key refresh.
 */
wiced_bool_t wiced_bt_mesh_db_node_net_key_update(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t unicast_addr, uint16_t net_key_idx, uint8_t phase);

/*
 * The app key on the device has been updated during the key refresh.
 */
wiced_bool_t wiced_bt_mesh_db_node_app_key_update(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t unicast_addr, uint16_t net_key_idx, uint16_t app_key_idx);

/*
 * Mark the configuration database that configuration has been completed for the node with specified address or if reconfiguration is required
 */
wiced_bool_t wiced_bt_mesh_db_node_config_complete(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t addr, wiced_bool_t is_complete);

/*
 * Get Group by Name.
 * Returns group, or NULL if group is not found.
 */
wiced_bt_mesh_db_group_t *wiced_bt_mesh_db_group_get_by_name(wiced_bt_mesh_db_mesh_t *mesh_db, const char *group_name);

/*
 * Get Group Address.
 * Returns group address, or o if group is not found.
 */
uint16_t wiced_bt_mesh_db_group_get_addr(wiced_bt_mesh_db_mesh_t *mesh_db, const char *group_name);

/*
 * Set Group Name.
 * Returns group address, or o if group is not found.
 */
wiced_bool_t wiced_bt_mesh_db_group_rename(wiced_bt_mesh_db_mesh_t *mesh_db, const char *old_name, const char *new_name);

/*
 * Add Publication.
 * The function adds publication objtect to the mesh device.  A provisioner should call this function after successfully configuring a client device for publications.
 */
wiced_bool_t wiced_bt_mesh_db_node_model_pub_add(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t element_addr, uint16_t company_id, uint16_t model_id, uint16_t pub_addr, uint16_t app_key_idx, uint8_t publish_ttl, uint32_t publish_period, uint16_t publish_retransmit_count, uint32_t publish_retransmit_interval, uint8_t credential);

/*
 * Get Publication.
 * The function retrieves publication information for the mesh device.
 */
wiced_bool_t wiced_bt_mesh_db_node_model_pub_get(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t element_addr, uint16_t company_id, uint16_t model_id, uint16_t *pub_addr, uint16_t *app_key_idx, uint8_t *publish_ttl, uint32_t *publish_period, uint16_t *publish_retransmit_count, uint32_t *publish_retransmit_interval, uint8_t *credentials);

/*
 * Delete Publication.
 * The function deletes publication objtect from a model of the mesh device.  A provisioner should call this function after successfully configuring a client device for publications.
 */
wiced_bool_t wiced_bt_mesh_db_node_model_pub_delete(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t element_addr, uint16_t company_id, uint16_t model_id);

/*
 * Add Subscription.
 * The function adds subscription objtect to the mesh device.  A provisioner should call this function after successfully adding subscription on a server model device.
 */
wiced_bool_t wiced_bt_mesh_db_node_model_sub_add(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t element_addr, uint16_t company_id, uint16_t model_id, uint16_t addr);

/*
 * Delete Subscription.
 * The function deletes subscription objtect to the mesh device.  A provisioner should call this function after successfully deleting subscription on a server model device.
 */
wiced_bool_t wiced_bt_mesh_db_node_model_sub_delete(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t element_addr, uint16_t company_id, uint16_t model_id, uint16_t addr);

/*
 * Delete All Subscriptions.
 * The function deletes all subscription objtects for the model of the mesh device.  A provisioner should call this function after successfully executing delete all subscriptions operation on a server model device.
 */
wiced_bool_t wiced_bt_mesh_db_node_model_sub_delete_all(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t element_addr, uint16_t company_id, uint16_t model_id);

/*
 * Set Network Transmit Parameters.
 * The function sets network transmit paramaneters in the configuration of the mesh device.  A provisioner should call this function after successfully executing Set or Get network transmit parameters procedure.
 */
wiced_bool_t wiced_bt_mesh_db_net_transmit_set(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t unicast_addr, uint16_t count, uint32_t interval);

/*
 * Get Network Transmit Parameters.
 * The function gets network transmit paramaneters in the configuration of the mesh device.
 */
wiced_bool_t wiced_bt_mesh_db_net_transmit_get(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t unicast_addr, uint16_t *count, uint32_t *interval);

/*
 * Set Default TTL.
 * The function sets the Default TTL in the configuration of the mesh device.  A provisioner should call this function after successfully executing Set or Get Default TTL procedure.
 */
wiced_bool_t wiced_bt_mesh_db_default_ttl_set(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t unicast_addr, uint8_t ttl);

/*
 * Get Default TTL.
 * The function gets the Default TTL in the configuration of the mesh device.
 */
wiced_bool_t wiced_bt_mesh_db_default_ttl_get(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t unicast_addr, uint8_t *ttl);

/*
 * Set Relay State.
 * The function sets the Relay State in the configuration of the mesh device.  A provisioner should call this function after successfully executing Set or Get Relay State procedure.
 */
wiced_bool_t wiced_bt_mesh_db_relay_set(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t unicast_addr, uint8_t state, uint16_t retransmit_count, uint32_t retransmit_interval);

/*
 * Get Relay State.
 * The function sets the Relay State in the configuration of the mesh device.  A provisioner should call this function after successfully executing Set or Get Relay State procedure.
 */
wiced_bool_t wiced_bt_mesh_db_relay_get(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t unicast_addr, uint8_t *state, uint16_t *retransmit_count, uint32_t *retransmit_interval);

/*
 * Set Friend State.
 * The function sets the Friend State in the configuration of the mesh device.  A provisioner should call this function after successfully executing Set or Get Friend State procedure.
 */
wiced_bool_t wiced_bt_mesh_db_friend_set(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t unicast_addr, uint8_t state);

/*
 * Get Friend State.
 * The function gets the Friend State in the configuration of the mesh device.
 */
wiced_bool_t wiced_bt_mesh_db_friend_get(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t unicast_addr, uint8_t *state);

/*
 * Set GATT Proxy State.
 * The function sets the GATT Proxy State in the configuration of the mesh device.  A provisioner should call this function after successfully executing Set or Get GATT Proxy State procedure.
 */
wiced_bool_t wiced_bt_mesh_db_gatt_proxy_set(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t unicast_addr, uint8_t state);

/*
 * Get GATT Proxy State.
 * The function gets the GATT Proxy State in the configuration of the mesh device.
 */
wiced_bool_t wiced_bt_mesh_db_gatt_proxy_get(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t unicast_addr, uint8_t *state);

/*
 * Set Send Network Beacons State.
 * The function sets the Send Network Beacons State in the configuration of the mesh device.  A provisioner should call this function after successfully executing Set or Get Network Beacon State procedure.
 */
wiced_bool_t wiced_bt_mesh_db_beacon_set(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t unicast_addr, uint8_t state);

/*
 * Get Send Network Beacons State.
 * The function gets the Send Network Beacons State of the mesh device.
 */
wiced_bool_t wiced_bt_mesh_db_beacon_get(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t unicast_addr, uint8_t *state);

#ifdef PRIVATE_PROXY_SUPPORTED
/*
 * Set Private Beacons State.
 * The function sets the Private Beacons State in the configuration of the mesh device.  A provisioner should call this function after successfully executing Set or Get Private Beacon State procedure.
 */
wiced_bool_t wiced_bt_mesh_db_private_beacon_set(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint8_t state, uint8_t random_update_interval);

/*
 * Get Private Beacons State.
 * The function gets the Private Beacons State of the mesh device.
 */
wiced_bool_t wiced_bt_mesh_db_private_beacon_get(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint8_t *state, uint8_t *random_update_interval);

/*
* Set Private GATT Proxy State.
* The function sets Private GATT Proxy State in the configuration of the mesh device.  A provisioner should call this function after successfully executing Set or Get Private GATT Proxy State procedure.
*/
wiced_bool_t wiced_bt_mesh_db_private_gatt_proxy_set(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint8_t state);

/*
* Get Private GATT Proxy State.
* The function gets the Private GATT Proxy State in the configuration of the mesh device.
*/
wiced_bool_t wiced_bt_mesh_db_private_gatt_proxy_get(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint8_t *state);

/*
* Set On-Demand Private GATT Proxy State.
* The function sets On-Demand Private GATT Proxy State in the configuration of the mesh device.  A provisioner should call this function after successfully executing Set or Get On-Demand Private Proxy State procedure.
*/
wiced_bool_t wiced_bt_mesh_db_on_demand_private_proxy_set(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint8_t state);

/*
* Get On-Demand Private GATT Proxy State.
* The function gets the On-Demand Private GATT Proxy State in the configuration of the mesh device.
*/
wiced_bool_t wiced_bt_mesh_db_on_demand_private_proxy_get(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint8_t *state);
#endif

#ifdef DIRECTED_FORWARDING_SERVER_SUPPORTED
/*
 * Get Directed Forwarding Control State.
 * The function gets the Directed Forwarding Control State of the mesh device.
 */
wiced_bool_t wiced_bt_mesh_db_df_control_get(wiced_bt_mesh_db_mesh_t* mesh_db, uint16_t unicast_addr, wiced_bt_mesh_df_state_control_t* df_control);

wiced_bool_t wiced_bt_mesh_db_df_control_set(wiced_bt_mesh_db_mesh_t* mesh_db, uint16_t unicast_addr, wiced_bt_mesh_df_state_control_t* df_control);
#endif

/*
 * Get all networks.
 * The function allocates a buffer and returns a concatenation of all network names. Caller is responsible for freeing the buffer.
 */
char *wiced_bt_mesh_db_get_all_networks(void);

/*
 * Get all groups.
 * The function allocates a buffer and returns a concatenation of all group names in the current database. Caller is responsible for freeing the buffer.
 */
char *wiced_bt_mesh_db_get_all_groups(wiced_bt_mesh_db_mesh_t *mesh_db, char *in_group);

/*
 * Get all provisioners.
 * The function allocates a buffer and returns a concatenation of all provisioner names in the current database. Caller is responsible for freeing the buffer.
 */
char *wiced_bt_mesh_db_get_all_provisioners(wiced_bt_mesh_db_mesh_t *mesh_db);

/*
 * Get Provisioner address
 * Returns address of the provisioner with specified UUID
 */
uint16_t wiced_bt_mesh_db_get_provisioner_addr(wiced_bt_mesh_db_mesh_t *p_mesh_db, const uint8_t *provisioner_uuid);

/*
 * Get all elements.
 * The function allocates a buffer and returns a zero terminated array of all element addresses. Caller is responsible for freeing the buffer.
 */
uint16_t *wiced_bt_mesh_db_get_all_elements(wiced_bt_mesh_db_mesh_t *mesh_db);

/*
 * Get Node Address of the Element.
 * The function returns the unicast address of the first element in the node to which element_addr belongs.
 */
uint16_t wiced_bt_mesh_db_get_node_addr(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t element_addr);

/*
 * Get elements of the device.
 * The function allocates a buffer and returns a zero terminated array of element addresses of the device. Caller is responsible for freeing the buffer.
 */
uint16_t *wiced_bt_mesh_db_get_device_elements(wiced_bt_mesh_db_mesh_t *mesh_db, uint8_t *p_uuid);

/*
 * Get all models of the element which belong to a group.
 * The function allocates a buffer and returns a zero terminated array of all models for the specified element addresses. Caller is responsible for freeing the buffer.
 * If in_group parameter is 0, the function returns all elements in the group.  Otherwise the function returns only elements which are configured to publish to the group
 * or subscribed for the in_group address.
 */
wiced_bt_mesh_db_model_id_t *wiced_bt_mesh_db_get_all_models_of_element(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, uint16_t in_group);

/*
 * Checks if model is subscribed to the group.
 */
wiced_bool_t wiced_bt_mesh_db_is_model_subscribed_to_group(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, uint16_t company_id, uint16_t model_id, uint16_t in_group_addr);

/*
 * Get name of the element.
 * The function returns the pointer to the name of the element in the current database, or NULL if element with specified address is not found, or address is not set.
 */
const char *wiced_bt_mesh_db_get_element_name(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t addr);

/*
 * Get element by name.
 * The function returns the pointer to the element structure based on address in the current database, or NULL if element with specified address is not found.
 */
wiced_bt_mesh_db_element_t *wiced_bt_mesh_db_get_element(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t addr);

/*
 * Set name of the element.
 * The function sets the name of the element in the current database.
 */
void wiced_bt_mesh_db_set_element_name(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t addr, const char *p_element_name);

/*
 * Create node function can be called at the end of the provisioning.  At this time
 * provisioner knows only address and device key and can assign a name.
 */
wiced_bt_mesh_db_node_t *wiced_bt_mesh_db_node_create(wiced_bt_mesh_db_mesh_t *mesh_db, const char *name, uint16_t node_addr, uint8_t *p_uuid, uint8_t num_elements, uint8_t *dev_key, uint16_t net_key_index);

/*
 * Mark node as blocked
 */
wiced_bool_t wiced_bt_mesh_db_node_block(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t addr);

/*
 * Delete node from the database
 */
wiced_bool_t wiced_bt_mesh_db_node_remove(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t addr);

/*
 * Set composition data for the node.
 */
wiced_bool_t wiced_bt_mesh_db_node_set_composition_data(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t node_addr, uint8_t *comp_data, uint16_t comp_data_len);

/*
 * Check composition data of the node.
 * Verifies if database definitions fully matches retrieved composition data.
 */
wiced_bool_t wiced_bt_mesh_db_node_check_composition_data(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t node_addr, uint8_t *comp_data, uint16_t comp_data_len);

/*
 * Get node information for the node with specified unicast address.
 */
wiced_bt_mesh_db_node_t *wiced_bt_mesh_db_node_get_by_addr(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t node_addr);

/*
 * Get node information for the node with specified uuid.
 */
wiced_bt_mesh_db_node_t* wiced_bt_mesh_db_node_get_by_uuid(wiced_bt_mesh_db_mesh_t* mesh_db, uint8_t *p_uuid);

/*
 * Get node information for the node using the name of one of its elements.
 */
wiced_bt_mesh_db_node_t *wiced_bt_mesh_db_node_get_by_element_name(wiced_bt_mesh_db_mesh_t *mesh_db, const char *name);

/*
 * Get node information for the node using the address of one of its elements.
 */
wiced_bt_mesh_db_node_t *wiced_bt_mesh_db_node_get_by_element_addr(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t elem_addr);

/*
 * Get group name
 * Returns name of the group for a given group address
 */
const char *wiced_bt_mesh_db_get_group_name(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t addr);

/*
 * Get list of groups to which element belongs
 * The function allocates array and fills with all groups that the element is subscribed to.
 */
uint16_t *wiced_bt_mesh_db_get_element_group_list(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t element_addr);

/*
 * Check if an elements with specified address is in the group with specified address
 */
wiced_bool_t wiced_bt_mesh_db_element_is_in_group(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t element_addr, uint16_t group_addr);

/*
* Get group by group address
*/
wiced_bt_mesh_db_group_t *wiced_bt_mesh_db_group_get_by_addr(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t addr);

/*
 * Creates group and allocates address
 */
uint16_t wiced_bt_mesh_db_group_add(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t provisioner_address, const char *group_name, const char *parent_group_name);

/*
 * Deletes group and frees the group address
 */
wiced_bool_t wiced_bt_mesh_db_group_delete(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t provisioner_addr, const char *group_name);

/*
 * Returns TRUE if property with specified ID is present on the element
 */
wiced_bool_t wiced_bt_mesh_db_sensor_property_present(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, uint16_t property_id);

/*
 * Returns zero terminated list of property IDs on the element
 */
int *wiced_bt_mesh_db_node_sensor_get_property_ids(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr);

/*
 * Adds list of descriptors to sensor model
 */
wiced_bool_t wiced_bt_mesh_db_sensor_descriptor_add(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, wiced_bt_mesh_sensor_descriptor_status_data_t *data);

/*
 * Gets current Cadence data of the sensor
 */
wiced_bool_t wiced_bt_mesh_db_node_model_cadence_get(wiced_bt_mesh_db_mesh_t *mesh_db, wiced_bool_t *is_data_present, uint16_t element_addr, uint16_t property_id,
                                                     wiced_bt_mesh_db_cadence_t *cadence_data, uint8_t *prop_value_len);

/*
 * Adds cadence information to sensor model
 */
wiced_bool_t wiced_bt_mesh_db_sensor_cadence_add(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, wiced_bt_mesh_sensor_cadence_status_data_t *data);

/*
 * Adds settings information to sensor model
 */
wiced_bool_t wiced_bt_mesh_db_sensor_settings_add(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, wiced_bt_mesh_sensor_settings_status_data_t *data);

/*
 * Get setting property Ids
 */
int* wiced_bt_mesh_db_node_model_get_setting_property_ids(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, uint16_t property_id);

/*
 * Get setting information
 */
wiced_bool_t wiced_bt_mesh_db_node_model_setting_get(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, uint16_t property_id,
                                                     wiced_bt_mesh_db_setting_t *setting_data, uint16_t setting_prop_id, uint8_t *prop_value_len);

/**
 * Update Sensor setting values
 */
wiced_bool_t wiced_bt_mesh_db_sensor_setting_add(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, wiced_bt_mesh_sensor_setting_status_data_t *data);

/*
 * Gets the property value length of the sensor property id.
 */
uint8_t wiced_bt_mesh_db_get_property_value_len(uint16_t property_id);

#ifdef __cplusplus
}
#endif

#endif
