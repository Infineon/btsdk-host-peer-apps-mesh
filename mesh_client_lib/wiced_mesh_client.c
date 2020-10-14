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
 * Client side interface implementation to access mesh libraries
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#ifdef _WIN32
#include <io.h>
#endif
#ifdef __APPLE__
#include <sys/uio.h>
#include <dirent.h>
#include <errno.h>
#endif
#if defined __ANDROID__ || defined WICEDX_LINUX
#include <dirent.h>
#include <errno.h>
#endif
#include "wiced_bt_ble.h"
#include "wiced_mesh_client.h"
#include "hci_control_api.h"
#include "wiced_bt_mesh_model_defs.h"
#include "wiced_bt_mesh_models.h"
#include "wiced_bt_mesh_provision.h"
#include "wiced_bt_mesh_client.h"
#include "wiced_bt_mesh_db.h"
#ifdef MESH_DFU_ENABLED
#include "wiced_bt_mesh_dfu.h"
#include "wiced_mesh_client_dfu.h"
#endif

//#ifndef CLIENTCONTROL
extern void ods(char * fmt_str, ...);
//#else
//#endif

#define USE_SETUP_APPKEY
#define USE_VENDOR_APPKEY
#define wiced_bt_get_buffer malloc
#define wiced_bt_free_buffer free

// can be local, or remote, or remote and rssi, or rssi.
#define PROVISION_BY_LOCAL_IF_POSSIBLE      0
#define PROVISION_BY_REMOTE_IF_POSSIBLE     1
#define PROVISION_BY_BEST_RSSI              1


#define LOCAL_DEVICE_TTL                    63
#define LOCAL_DEVICE_NET_TRANSMIT_COUNT     3
#define LOCAL_DEVICE_NET_TRANSMIT_INTERVAL  50
#define NUM_HOPS_UNKNOWN                    0xFF

#define NODE_RESET_TIMEOUT                  10

#define OPERATION_REMOTE                    0
#define OPERATION_LOCAL                     1

#define CONFIG_OPERATION_SET_DEV_KEY        1
#define CONFIG_OPERATION_NET_KEY_UPDATE     2
#define CONFIG_OPERATION_APP_KEY_UPDATE     3
#define CONFIG_OPERATION_MODEL_APP_BIND     4
#define CONFIG_OPERATION_MODEL_SUBSCRIBE    5
#define CONFIG_OPERATION_MODEL_PUBLISH      6
#define CONFIG_OPERATION_NET_TRANSMIT_SET   7
#define CONFIG_OPERATION_DEFAULT_TTL_SET    8
#define CONFIG_OPERATION_RELAY_SET          9
#define CONFIG_OPERATION_NET_BEACON_SET     10
#define CONFIG_OPERATION_PROXY_SET          11
#define CONFIG_OPERATION_FRIEND_SET         12
#define CONFIG_OPERATION_DEF_TRANS_TIME     13
#define CONFIG_OPERATION_FILTER_ADD         14
#define CONFIG_OPERATION_NODE_RESET         15
#define CONFIG_OPERATION_KR_PHASE_SET       16
#define CONFIG_OPERATION_SENSOR_DESC_GET    17
#define CONFIG_OPERATION_SENSOR_SETTINGS_GET 18
#define CONFIG_OPERATION_SENSOR_CADENCE_GET 19
#define CONFIG_OPERATION_FILTER_DEL         20

#define DEVICE_TYPE_LIGHT_HSL_HUE           13
#define DEVICE_TYPE_LIGHT_HSL_SATURATION    14
#define DEVICE_TYPE_LIGHT_CTL_TEMPERATURE   15
#define DEVICE_TYPE_LIGHT_CONTROLLER        16

#define FOUNDATION_FEATURE_BIT_RELAY        0x0001
#define FOUNDATION_FEATURE_BIT_PROXY        0x0002
#define FOUNDATION_FEATURE_BIT_FRIEND       0x0004
#define FOUNDATION_FEATURE_BIT_LOW_POWER    0x0008

#define SCAN_DURATION_DEFAULT               10  // seconds
#define SCAN_DURATION_MAX                   255 // seconds
#define SCAN_EXTENDED_DURATION              10

// define how long to wait for node identity beacons after provision complete and disconnection
#define NODE_IDENTITY_SCAN_DURATION         8
#define VENDOR_ID_LEN                       8

typedef struct t_pending_operation
{
    struct t_pending_operation *p_next;
    uint8_t operation;
    union
    {
        wiced_bt_mesh_set_dev_key_data_t set_dev_key;
        wiced_bt_mesh_config_netkey_change_data_t net_key_change;
        wiced_bt_mesh_config_appkey_change_data_t app_key_change;
        wiced_bt_mesh_config_model_app_bind_data_t app_key_bind;
        wiced_bt_mesh_config_model_subscription_change_data_t model_sub;
        wiced_bt_mesh_config_model_publication_set_data_t model_pub;
        wiced_bt_mesh_config_network_transmit_set_data_t net_transmit_set;
        wiced_bt_mesh_config_default_ttl_set_data_t default_ttl_set;
        wiced_bt_mesh_config_relay_set_data_t relay_set;
        wiced_bt_mesh_config_beacon_set_data_t beacon_set;
        wiced_bt_mesh_config_gatt_proxy_set_data_t proxy_set;
        wiced_bt_mesh_config_friend_set_data_t friend_set;
        wiced_bt_mesh_proxy_filter_change_addr_data_t filter_add;
        wiced_bt_mesh_default_transition_time_data_t default_trans_time;
        wiced_bt_mesh_config_key_refresh_phase_set_data_t kr_phase_set;
        wiced_bt_mesh_sensor_get_t sensor_get;
    } uu;
    wiced_bt_mesh_event_t *p_event;
} pending_operation_t;

extern void Log(char *fmt, ...);
extern void mesh_application_init(void);
extern void mesh_application_deinit(void);

const char *p_device_type_name[] =
{
    "Unknown",
    "Switch",
    "Dimmer",
    "OnOff Server",
    "Level Server",
    "Power OnOff",
    "Power Level",
    "Light",
    "Light",
    "Light",
    "Light",
    "Sensor",
    "Sensor Client"
    "Vendor Specific",
};

typedef struct
{
    uint8_t  is_setup;
    uint8_t  need_sub_pub;
    uint16_t company_id;
    uint16_t model_id;
} model_element_t;

// list of models that are configured for publication. These are all client models and Sensor Server.
model_element_t models_configured_for_pub[] =
{
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_CLNT },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_CLNT },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_DEFTT_CLNT },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_POWER_ONOFF_CLNT },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_POWER_LEVEL_CLNT },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_BATTERY_CLNT },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LOCATION_CLNT },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_PROPERTY_CLNT },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_TIME_CLNT },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SCENE_CLNT },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SCHEDULER_CLNT },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_CLNT },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_CLNT },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_CLNT },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_XYL_CLNT },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LC_CLNT },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_CLNT },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV },
};

// list of models that are configured for subscribtion when provisioned or placed to a group. These are all server models.
// We also configure Sensor Client. We would configure Sensor Server to publish data to a group, so it would make
// sense for a client by default to process the messages.
model_element_t models_configured_for_sub[] =
{
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_SRV },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_SRV },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_DEFTT_SRV },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_POWER_ONOFF_SRV },
    { 1, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_POWER_ONOFF_SETUP_SRV },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_POWER_LEVEL_SRV },
    { 1, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_POWER_LEVEL_SETUP_SRV },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_BATTERY_SRV },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LOCATION_SRV },
    { 1, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LOCATION_SETUP_SRV },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_SRV },
    { 1, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_SETUP_SRV },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SRV },
    { 1, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SETUP_SRV },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_SRV },
    { 1, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_SETUP_SRV },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_XYL_SRV },
    { 1, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_XYL_SETUP_SRV },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LC_SRV },
    { 1, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LC_SETUP_SRV },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SCENE_SRV },
    { 1, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SCENE_SETUP_SRV },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_TIME_SRV },
    { 1, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_TIME_SETUP_SRV },
    { 0, 0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SCHEDULER_SRV },
    { 1, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SCHEDULER_SETUP_SRV },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_HUE_SRV },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SATURATION_SRV },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_TEMPERATURE_SRV },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ADMIN_PROPERTY_SRV },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_MANUFACT_PROPERTY_SRV },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_USER_PROPERTY_SRV },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_CLIENT_PROPERTY_SRV },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV },
    { 1, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SETUP_SRV },
    { 0, 1, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_CLNT },
};

#define MAX_CONNECT_RETRIES     4

typedef struct unprovisioned_report__t
{
    struct unprovisioned_report__t *p_next;
    uint16_t provisioner_addr;
    uint8_t  uuid[16];
    int8_t   rssi;
    uint16_t oob;
#define EXTENDED_SCAN_STATE_NOT_STARTED 0
#define EXTENDED_SCAN_STATE_STARTED     1
#define EXTENDED_SCAN_STATE_COMPLETED   2
    uint8_t  extended_scan_state;

    wiced_timer_t scan_timer;
} unprovisioned_report_t;

typedef struct
{
#define PROVISION_STATE_IDLE                                0
#define PROVISION_STATE_CONNECTING                          1
#define PROVISION_STATE_PROVISIONING                        2
#define PROVISION_STATE_PROVISION_DISCONNECTING             3
#define PROVISION_STATE_NODE_CONNECTING                     4
#define PROVISION_STATE_GET_REMOTE_COMPOSITION_DATA         5
#define PROVISION_STATE_CONFIGURATION                       6
#define PROVISION_STATE_CONFIGURE_DISCONNECTING             7
#define PROVISION_STATE_NETWORK_CONNECT                     8
#define PROVISION_STATE_CONNECTING_NODE_WAIT_NODE_IDENTITY  9
#define PROVISION_STATE_CONNECTING_NODE_WAIT_DISCONNECT     10
#define PROVISION_STATE_CONNECTING_NODE_WAIT_CONNECT        11
#define PROVISION_STATE_RECONFIGURATION                     12
#define PROVISION_STATE_KEY_REFRESH_1                       13
#define PROVISION_STATE_KEY_REFRESH_2                       14
#define PROVISION_STATE_KEY_REFRESH_3                       15
    uint8_t     state;

    uint8_t     network_opened;
    uint8_t     retries;

#define WICED_BT_MESH_PROVISION_PROCEDURE_PROVISION                     0xFF
#define WICED_BT_MESH_PROVISION_PROCEDURE_DEV_KEY_REFRESH               0
#define WICED_BT_MESH_PROVISION_PROCEDURE_NODE_ADDRESS_REFRESH          1
#define WICED_BT_MESH_PROVISION_PROCEDURE_NODE_COMPOSITION_REFRESH      2
    uint8_t     provision_procedure;

    uint16_t    unicast_addr;       // local device unicast address
    uint16_t    company_id;         // Local device company ID
    uint8_t     dev_key[16];        // local device key
    unprovisioned_report_t *p_first_unprovisioned;// address of the first unprovisioned report
    uint8_t     uuid[16];           // device being provisioned
    uint8_t     oob_data[16];       // Static OOB data to be used during provisioning
    uint8_t     oob_data_len;       // Length of Static OOB Data, or 0 if provisioning with no static OOB data
    uint32_t    provision_conn_id;  // connection id used during provisioning
    uint32_t    proxy_conn_id;      // connection id of the connection to proxy
    uint8_t     provision_completed_sent;   // Set to false when starting procedure, true when failed/success sent to the client
    uint8_t     provision_last_status;      // Last status sent to the client
    uint8_t     over_gatt;          // connected over gatt
    uint16_t    addr;               // address of the device being provisioned
    uint8_t     num_elements;       // number of elements received in the device capabilities
    uint16_t    group_addr;         // address of the group to which device belongs to
    uint8_t     store_config;       // If TRUE save configuration to Provisioner DB on every status received.
    uint8_t     scan_duration;      // How long to wait for proxy device to show up.
    uint8_t     scan_one_uuid;      // Interested to find if a device with specific uuid is present
    uint16_t    provisioner_addr;   // address of the device who performs provisioning
    char *      provisioning_device_name; // name of the device being provisioned
    uint16_t    proxy_addr;         // Address of GATT Proxy
    uint16_t    device_key_addr;    // address of the device key configured in the local device
    uint8_t     db_changed;         // set to true if database changes during any operation
    uint8_t     identify_duration;
    uint8_t     use_gatt;
    uint8_t     is_gatt_proxy;
    uint8_t     is_friend;
    uint8_t     is_relay;
    uint8_t     relay_xmit_count;
    uint16_t    relay_xmit_interval;
    uint8_t     beacon;
    uint8_t     default_ttl;
    uint8_t     net_xmit_count;
    uint16_t    net_xmit_interval;
    uint8_t     publish_ttl;                   ///< Default TTL value for the outgoing messages
    uint8_t     publish_retransmit_count;      ///< Number of retransmissions for each published message
    uint16_t    publish_retransmit_interval;   ///< Interval in milliseconds between retransmissions
    uint8_t     publish_credential_flag;       ///< Value of the Friendship Credential Flag
    wiced_bt_mesh_config_composition_data_status_data_t *p_remote_composition_data;
    wiced_bt_mesh_config_composition_data_status_data_t *p_local_composition_data;
    mesh_client_network_opened_t p_opened_callback;
    mesh_client_component_info_status_t p_component_info_status_callback;
    mesh_client_unprovisioned_device_t p_unprovisioned_device;
    mesh_client_provision_status_t p_provision_status;
    mesh_client_connect_status_t p_connect_status;
    mesh_client_node_connect_status_t p_node_connect_status;
    mesh_client_node_reset_status_t p_reset_status;
    mesh_client_database_changed_t p_database_changed;
    mesh_client_on_off_status_t p_onoff_status;
    mesh_client_level_status_t p_level_status;
    mesh_client_lightness_status_t p_lightness_status;
    mesh_client_hsl_status_t p_hsl_status;
    mesh_client_ctl_status_t p_ctl_status;
    mesh_client_sensor_status_t p_sensor_status;
    mesh_client_light_lc_mode_status_t p_light_lc_mode_status;
    mesh_client_light_lc_occupancy_mode_status_t p_light_lc_occupancy_mode_status;
    mesh_client_light_lc_property_status_t p_light_lc_property_status;
    mesh_client_vendor_specific_data_t p_vendor_specific_data;
    pending_operation_t *p_first;
    wiced_timer_t op_timer;
} mesh_provision_cb_t;

mesh_provision_cb_t provision_cb = { 0 };

static void start_next_op(mesh_provision_cb_t *p_cb);
static void clean_pending_op_queue(uint16_t addr);
static uint8_t configure_local_device(uint16_t unicast_addr, uint8_t phase, uint16_t net_key_idx, uint8_t *p_net_key);
static void configure_queue_local_device_operations(mesh_provision_cb_t *p_cb);
static void configure_queue_remote_device_operations(mesh_provision_cb_t *p_cb);
static void configure_pending_operation_queue(mesh_provision_cb_t *p_cb, pending_operation_t *p_op);
static void app_key_add(mesh_provision_cb_t* p_cb, uint16_t addr, wiced_bt_mesh_db_net_key_t* net_key, wiced_bt_mesh_db_app_key_t* app_key);
static void model_app_bind(mesh_provision_cb_t* p_cb, wiced_bool_t is_local, uint16_t addr, uint16_t company_id, uint16_t model_id, uint16_t app_key_idx);
static pending_operation_t *configure_pending_operation_dequeue(mesh_provision_cb_t *p_cb);
static wiced_bool_t configure_event_in_pending_op(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event);
static void configure_execute_pending_operation(mesh_provision_cb_t *p_cb);
static void provision_status_notify(mesh_provision_cb_t* p_cb, uint8_t status);
static model_element_t* model_needs_default_pub(uint16_t company_id, uint16_t model_id);
static model_element_t* model_needs_default_sub(uint16_t company_id, uint16_t model_id);
static const char *get_component_name(uint16_t addr);
static void get_rpl_filename(char *filename);
static wiced_bool_t is_model_present(uint16_t element_addr, uint16_t company_id, uint16_t model_id);
void mesh_sensor_process_event(uint16_t addr, uint16_t event, void *p_data);

void mesh_provision_process_event(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_provision_state_idle(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_provision_state_connecting(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_provision_state_provisioning(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_provision_state_provision_disconnecting(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_configure_state_node_connecting(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_configure_state_getting_remote_composition_data(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_configure_state_configuration(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_configure_state_disconnecting(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_configure_state_network_connect(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_client_state_connecting_node_wait_node_identity(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_client_state_connecting_node_wait_disconnect(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_client_state_connecting_node_wait_connect(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_configure_state_key_refresh1(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_configure_state_key_refresh2(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_configure_state_key_refresh3(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);

static void mesh_client_provision_connect(mesh_provision_cb_t *p_cb);
static void mesh_provision_connecting_link_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_link_report_data_t *p_data);
static void mesh_provision_provisioning_link_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_link_report_data_t *p_data);
static void mesh_provision_disconnecting_link_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_link_report_data_t *p_data);
static void mesh_node_connecting_link_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_connect_status_data_t *p_data);
static void mesh_provision_process_device_caps(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_device_capabilities_data_t *p_data);
static void mesh_provision_process_provision_end(mesh_provision_cb_t *p_cb, wiced_bt_mesh_provision_status_data_t *p_data);
static void mesh_provision_process_get_oob_data(mesh_provision_cb_t* p_cb, wiced_bt_mesh_event_t* p_event, wiced_bt_mesh_provision_device_oob_request_data_t* p_data);

void mesh_configure_set_local_device_key(uint16_t addr);
static void mesh_configure_proxy_connection_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_connect_status_data_t *p_data);
static void mesh_configure_disconnecting_link_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_connect_status_data_t *p_data);
static void mesh_configure_composition_data_get(mesh_provision_cb_t *p_cb, uint8_t is_local);
static void mesh_configure_composition_data_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_composition_data_status_data_t *p_data);
static void mesh_configure_net_key_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_netkey_status_data_t *p_data);
static void mesh_configure_app_key_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_appkey_status_data_t *p_data);
static void mesh_configure_phase_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_key_refresh_phase_status_data_t *p_data);
static void mesh_configure_model_app_bind_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_model_app_bind_status_data_t *p_data);
static void mesh_configure_model_sub_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_model_subscription_status_data_t *p_data);
static void mesh_configure_model_pub_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_model_publication_status_data_t *p);
static void mesh_configure_net_transmit_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_network_transmit_status_data_t *p_data);
static void mesh_configure_default_ttl_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_default_ttl_status_data_t *p_data);
static void mesh_configure_relay_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_relay_status_data_t *p_data);
static void mesh_configure_friend_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_friend_status_data_t *p_data);
static void mesh_configure_gatt_proxy_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_gatt_proxy_status_data_t *p_data);
static void mesh_configure_beacon_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_beacon_status_data_t *p_data);
static void mesh_configure_filter_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_proxy_filter_status_data_t *p_data);

static uint8_t mesh_reset_node(mesh_provision_cb_t *p_cb, wiced_bt_mesh_db_node_t *node);
static void mesh_key_refresh_update_keys(mesh_provision_cb_t *p_cb, wiced_bt_mesh_db_net_key_t *net_key);
static void mesh_key_refresh_continue(mesh_provision_cb_t *p_cb, wiced_bt_mesh_db_net_key_t *net_key);
static void mesh_key_refresh_link_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_connect_status_data_t *p_data);
static void mesh_key_refresh_net_key_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_netkey_status_data_t *p_data);
static void mesh_key_refresh_app_key_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_appkey_status_data_t *p_data);
static void mesh_key_refresh_phase2_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_key_refresh_phase_status_data_t *p_data);
static void mesh_key_refresh_phase3_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_key_refresh_phase_status_data_t *p_data);
static int mesh_client_key_refresh_phase1_continue(mesh_provision_cb_t *p_cb, wiced_bt_mesh_db_net_key_t *net_key);

static void mesh_key_refresh_phase1_completed(mesh_provision_cb_t *p_cb, wiced_bt_mesh_db_net_key_t *net_key);
static void mesh_key_refresh_phase2_completed(mesh_provision_cb_t *p_cb, wiced_bt_mesh_db_net_key_t *net_key);
static void mesh_key_refresh_phase3_completed(mesh_provision_cb_t *p_cb, wiced_bt_mesh_db_net_key_t *net_key);
static int mesh_client_transition_next_key_refresh_phase(mesh_provision_cb_t *p_cb, wiced_bt_mesh_db_net_key_t *net_key, uint8_t transition);

static void mesh_process_properties_status(wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_process_property_status(wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_process_health_attention_status(wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_process_scan_capabilities_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_scan_capabilities_status_data_t *p_data);
static void mesh_process_scan_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_scan_status_data_t *p_data);
static void mesh_process_scan_report(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_scan_report_data_t *p_data);
static void mesh_process_scan_extended_report(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_scan_extended_report_data_t *p_data);
static void mesh_process_light_lc_mode_status(wiced_bt_mesh_event_t* p_event, void* p_data);
static void mesh_process_light_lc_occupancy_mode_status(wiced_bt_mesh_event_t* p_event, void* p);
static void mesh_process_light_lc_property_status(wiced_bt_mesh_event_t* p_event, void* p_data);

static void mesh_default_trans_time_status(wiced_bt_mesh_event_t *p_event, void *p);
static void mesh_process_on_off_status(wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_process_level_status(wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_process_lightness_status(wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_process_hsl_status(wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_process_ctl_status(wiced_bt_mesh_event_t *p_event, void *p_data);

static void mesh_process_sensor_descriptor_status(uint16_t addr, wiced_bt_mesh_sensor_descriptor_status_data_t *ptr);
static void mesh_process_sensor_setting_status(uint16_t addr, wiced_bt_mesh_sensor_setting_status_data_t *ptr);
static void mesh_process_sensor_settings_status(uint16_t addr, wiced_bt_mesh_sensor_settings_status_data_t *ptr);
static void mesh_process_sensor_status(uint16_t addr, wiced_bt_mesh_sensor_status_data_t *ptr);
static void mesh_process_sensor_cadence_status(uint16_t addr, wiced_bt_mesh_sensor_cadence_status_data_t *ptr);

static void mesh_process_tx_complete(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event);
static void mesh_client_start_extended_scan(mesh_provision_cb_t* p_cb, unprovisioned_report_t* p_report);
wiced_bt_mesh_db_node_t *mesh_find_node(wiced_bt_mesh_db_mesh_t *p_mesh, uint16_t unicast_address);
wiced_bt_mesh_db_node_t *mesh_find_node_by_uuid(wiced_bt_mesh_db_mesh_t *p_mesh, uint8_t *uuid);
wiced_bt_mesh_db_net_key_t *find_net_key(wiced_bt_mesh_db_mesh_t *p_mesh, uint16_t net_key_idx);
uint16_t *get_group_list(uint16_t addr);
wiced_bool_t is_core_model(uint16_t company_id, uint16_t model_id);
wiced_bool_t is_secondary_element(uint16_t element_idx);
wiced_bool_t is_provisioner(wiced_bt_mesh_db_node_t *p_node);
static void download_rpl_list(void);
static void rand128(uint8_t *p_array);
static uint16_t rand16();
static wiced_bool_t model_needs_sub(uint16_t model_id, wiced_bt_mesh_db_model_id_t *p_models_array);
wiced_bt_mesh_event_t *mesh_create_control_event(wiced_bt_mesh_db_mesh_t *p_mesh_db, uint16_t company_id, uint16_t model_id, uint16_t dst, uint16_t app_key_idx);
static void scan_timer_cb(TIMER_PARAM_TYPE arg);
static void provision_timer_cb(TIMER_PARAM_TYPE arg);
static wiced_bool_t add_filter(mesh_provision_cb_t *p_cb, uint16_t addr);
wiced_bool_t get_control_method(const char *method_name, uint16_t *company_id, uint16_t *model_id);
wiced_bool_t get_target_method(const char *method_name, uint16_t *company_id, uint16_t *model_id);
void download_iv(uint32_t *p_iv_idx, uint8_t *p_iv_update);
static uint16_t get_device_addr(const char *p_dev_name);
static uint16_t *mesh_get_group_list(uint16_t group_addr, uint16_t company_id, uint16_t model_id, uint16_t *num);
static uint16_t get_group_addr(const char *p_dev_name);
extern wiced_bt_mesh_event_t *mesh_configure_create_event(uint16_t dst, wiced_bool_t retransmit);
void wiced_bt_mesh_gatt_client_connection_state_changed(uint16_t conn_id, uint16_t mtu);

wiced_bt_mesh_db_mesh_t *p_mesh_db = NULL;

char *mesh_new_string(const char *name)
{
    char *p_string = (char *)wiced_bt_get_buffer(strlen(name) + 1);
    if (p_string != NULL)
    {
        strcpy(p_string, name);
    }
    return p_string;
}

int mesh_client_network_exists(char *mesh_name)
{
    return wiced_bt_mesh_db_network_exists(mesh_name);
}

int mesh_client_network_create(const char *provisioner_name, const char *provisioner_uuid, char *mesh_name)
{
    uint8_t uuid[16];
    uint8_t dev_key[16];
    wiced_bt_mesh_db_app_key_t  app_key = { 0 };
    wiced_bt_mesh_db_net_key_t  net_key = { 0 };
    int i, j;

    if (p_mesh_db != NULL)
    {
        wiced_bt_mesh_db_deinit(p_mesh_db);
    }
    if ((p_mesh_db = wiced_bt_mesh_db_init(mesh_name)) == NULL)
    {
        p_mesh_db = (wiced_bt_mesh_db_mesh_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_mesh_t));
        if (p_mesh_db == NULL)
            return MESH_CLIENT_ERR_NO_MEMORY;

        memset(p_mesh_db, 0, sizeof(wiced_bt_mesh_db_mesh_t));

        rand128(p_mesh_db->uuid);
        p_mesh_db->name = mesh_new_string(mesh_name);

        net_key.index = 0;
        rand128(net_key.key);
        net_key.name = mesh_new_string("Net Key");
        net_key.phase = 0;

        wiced_bt_mesh_db_net_key_add(p_mesh_db, &net_key);

        app_key.index = rand16() & 0xFFF;
        rand128(app_key.key);
        app_key.bound_net_key_index = 0;
        app_key.name = mesh_new_string("Generic");

        wiced_bt_mesh_db_app_key_add(p_mesh_db, &app_key);

#ifdef USE_SETUP_APPKEY
        app_key.index = rand16() & 0xFFF;
        rand128(app_key.key);
        app_key.bound_net_key_index = 0;
        app_key.name = mesh_new_string("Setup");

        wiced_bt_mesh_db_app_key_add(p_mesh_db, &app_key);
#endif

#ifdef USE_VENDOR_APPKEY
        app_key.index = rand16() & 0xFFF;
        rand128(app_key.key);
        app_key.bound_net_key_index = 0;
        app_key.name = mesh_new_string("Vendor");

        wiced_bt_mesh_db_app_key_add(p_mesh_db, &app_key);
#endif
        for (i = 0; i < 16; i++)
        {
            sscanf(&provisioner_uuid[i * 2], "%02x", &j);
            uuid[i] = (uint8_t)j;
        }
        rand128(dev_key);
        p_mesh_db->unicast_addr = wiced_bt_mesh_db_provisioner_add(p_mesh_db, provisioner_name, uuid, dev_key);

        wiced_bt_mesh_db_store(p_mesh_db);

        wiced_bt_mesh_db_deinit(p_mesh_db);
        p_mesh_db = NULL;
    }
    return MESH_CLIENT_SUCCESS;
}

#if !(defined(_WIN32)) && (defined(__ANDROID__) || defined(__APPLE__) || defined(WICEDX_LINUX) || defined(BSA))
static int parse_file_ext(const struct dirent *dir, char *ext)
{
    // the ext must start with '.' character.
    if (!dir || !ext || strlen(ext) < 2)
        return 0;

    if (dir->d_type == DT_REG)
    {
        const char *ext_start = strrchr(dir->d_name, '.');
        if (!ext_start || ext_start == dir->d_name)
        {
            return 0;
        }
        else
        {
            if (strcasecmp(ext_start, ext) == 0)
                return 1;
        }
    }
    return 0;
}

static int parse_json_ext(const struct dirent *dir)
{
    return parse_file_ext(dir, ".json");
}

static int parse_bin_ext(const struct dirent *dir)
{
    return parse_file_ext(dir, ".bin");
}
#endif  // #if !(defined(_WIN32)) && (defined(__ANDROID__) || defined(__APPLE__) || defined(WICEDX_LINUX) || defined(BSA))

int mesh_client_network_delete(const char *provisioner_name, const char *provisioner_uuid, char *mesh_name)
{
    // we might need to init mesh to get UUID to delete RPL file.
    if (p_mesh_db == NULL)
    {
        p_mesh_db = wiced_bt_mesh_db_init(mesh_name);
    }
    if (p_mesh_db != NULL)
    {
        // remove the RPL and UUID file only when this is the last mesh network to be deleted.
        int json_num = 0;
#if !(defined(_WIN32)) && (defined(__ANDROID__) || defined(__APPLE__) || defined(WICEDX_LINUX) || defined(BSA))
        struct dirent **namelist;
        int n;

        json_num = n = scandir(".", &namelist, parse_json_ext, alphasort);
        while (n--)
        {
            wiced_bt_free_buffer(namelist[n]);
        }
        wiced_bt_free_buffer(namelist);
#else
        #ifdef _WIN32
                intptr_t hFile;
        #else
                long hFile;
        #endif
        struct _finddata_t find_file;

        if ((hFile = _findfirst("*.json", &find_file)) != -1L)
        {
            do
            {
                json_num++;
            } while (_findnext(hFile, &find_file) == 0);
            _findclose(hFile);
        }
#endif // #if !(defined(_WIN32)) && (defined(__ANDROID__) || defined(__APPLE__) || defined(WICEDX_LINUX) || defined(BSA))

        Log("json_num: %d\n", json_num);
        if (json_num < 2)
        {
            int ret;
            char filename[50];
            get_rpl_filename(filename);
            remove(filename);

#if !(defined(_WIN32)) && (defined(__ANDROID__) || defined(__APPLE__) || defined(WICEDX_LINUX) || defined(BSA))
            n = scandir(".", &namelist, parse_bin_ext, alphasort);
            while (n--) {
                ret = remove(namelist[n]->d_name);
                Log("remove: %s, ret=%d, errno=%d\n", namelist[n]->d_name, ret, errno);
                wiced_bt_free_buffer(namelist[n]);
            }
            wiced_bt_free_buffer(namelist);
#else
            if ((hFile = _findfirst("*.bin", &find_file)) != -1L)
            {
                do
                {
                    ret = remove(find_file.name);
                    Log("remove: %s, ret=%d, errno=%d\n", find_file.name, ret, errno);
                } while (_findnext(hFile, &find_file) == 0);
                _findclose(hFile);
            }
#endif // #if !(defined(_WIN32)) && (defined(__ANDROID__) || defined(__APPLE__) || defined(WICEDX_LINUX) || defined(BSA))
        }

        wiced_bt_mesh_db_deinit(p_mesh_db);
        p_mesh_db = NULL;
    }
    return wiced_bt_mesh_db_network_delete(mesh_name) ? MESH_CLIENT_SUCCESS : MESH_CLIENT_ERR_NOT_FOUND;
}

int mesh_client_network_open(const char *provisioner_name, const char *provisioner_uuid, char *mesh_name, mesh_client_network_opened_t p_opened_callback)
{
    uint8_t uuid[16];
    uint8_t dev_key[16];
    wiced_bool_t save = WICED_FALSE;
    wiced_bt_mesh_db_provisioner_t *provisioner;
    wiced_bt_mesh_db_app_key_t  app_key = { 0 };
    wiced_bt_mesh_db_net_key_t  net_key = { 0 };
    wiced_bt_mesh_db_net_key_t *p_net_key;
    int i, j = 0;

    if (p_mesh_db != NULL)
    {
        mesh_client_network_close();
    }

    if ((p_mesh_db = wiced_bt_mesh_db_init(mesh_name)) == NULL)
        return MESH_CLIENT_ERR_NETWORK_CLOSED;

    provision_cb.p_opened_callback = p_opened_callback;
    provision_cb.network_opened    = WICED_FALSE;

    mesh_application_init();

    wiced_init_timer(&provision_cb.op_timer, provision_timer_cb, &provision_cb, WICED_MILLI_SECONDS_TIMER);

    p_net_key = wiced_bt_mesh_db_net_key_get(p_mesh_db, 0);
    if (p_net_key == NULL)
    {
        net_key.index = 0;
        rand128(net_key.key);
        net_key.name = mesh_new_string("Net Key");
        net_key.phase = 0;

        p_net_key = &net_key;

        wiced_bt_mesh_db_net_key_add(p_mesh_db, p_net_key);
        save = WICED_TRUE;
    }
    if (wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic") == NULL)
    {
        app_key.index = rand16() & 0xFFF;
        rand128(app_key.key);
        app_key.bound_net_key_index = 0;
        app_key.name = mesh_new_string("Generic");

        wiced_bt_mesh_db_app_key_add(p_mesh_db, &app_key);
        save = WICED_TRUE;
    }
#ifdef USE_SETUP_APPKEY
    if (wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Setup") == NULL)
    {
        app_key.index = rand16() & 0xFFF;
        rand128(app_key.key);
        app_key.bound_net_key_index = 0;
        app_key.name = mesh_new_string("Setup");

        wiced_bt_mesh_db_app_key_add(p_mesh_db, &app_key);
        save = WICED_TRUE;
    }
#endif
#ifdef USE_VENDOR_APPKEY
    if (wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Vendor") == NULL)
    {
        app_key.index = rand16() & 0xFFF;
        rand128(app_key.key);
        app_key.bound_net_key_index = 0;
        app_key.name = mesh_new_string("Vendor");

        wiced_bt_mesh_db_app_key_add(p_mesh_db, &app_key);
        save = WICED_TRUE;
    }
#endif
    for (i = 0; i < 16; i++)
    {
        sscanf(&provisioner_uuid[i * 2], "%02x", &j);
        uuid[i] = (uint8_t)j;
    }
    if ((provisioner = wiced_bt_mesh_db_provisioner_get_by_uuid(p_mesh_db, uuid)) == NULL)
    {
        rand128(dev_key);
        p_mesh_db->unicast_addr = wiced_bt_mesh_db_provisioner_add(p_mesh_db, provisioner_name, uuid, dev_key);
        save = WICED_TRUE;
    }
    else
    {
        wiced_bt_mesh_db_node_t *node = mesh_find_node_by_uuid(p_mesh_db, provisioner->uuid);
        if (node == NULL)
            // TBD create node
            return MESH_CLIENT_ERR_NO_MEMORY;
        p_mesh_db->unicast_addr = node->unicast_address;
    }
    /*
    for (int i = 0; i < sizeof(group_name) / sizeof(group_name[0]); i++)
    {
        if (!wiced_bt_mesh_db_find_group_by_name(p_mesh_db, group_name[i]))
        {
            save = WICED_TRUE;
            wiced_bt_mesh_db_group_add(p_mesh_db, mesh_provisioner.unicast_address, group_name[i]);
        }
    }
    */
    if (save)
    {
        wiced_bt_mesh_db_store(p_mesh_db);

        if (provision_cb.p_database_changed)
            provision_cb.p_database_changed(mesh_name);
    }
    return configure_local_device(p_mesh_db->unicast_addr, 0, p_net_key->index, p_net_key->phase == WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL ? p_net_key->key : p_net_key->old_key);
}

char *mesh_client_network_import(const char *provisioner_name, const char *provisioner_uuid, char *json_string, mesh_client_network_opened_t p_opened_callback)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    char *db_name = NULL;
    FILE *fp;

    if (p_cb->state != PROVISION_STATE_IDLE)
        return NULL;

    fp = fopen("temp.json", "w");
    if (fp != NULL)
    {
        fwrite(json_string, 1, strlen(json_string), fp);
        fclose(fp);

        if (mesh_client_network_open(provisioner_name, provisioner_uuid, "temp", p_opened_callback) == MESH_CLIENT_SUCCESS)
        {
            wiced_bt_mesh_db_store(p_mesh_db);
            db_name = p_mesh_db->name;
        }
        remove("temp.json");
    }
    return db_name;
}

char *mesh_client_network_export(char *mesh_name)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    char *json_string;
    char *p_filename;

    p_filename = (char *)wiced_bt_get_buffer(strlen(mesh_name) + 6);
    if (p_filename == NULL)
        return NULL;

    strcpy(p_filename, mesh_name);
    strcat(p_filename, ".json");

    FILE *fp = fopen(p_filename, "r");
    if (fp == NULL)
    {
        wiced_bt_free_buffer(p_filename);
        return NULL;
    }
    wiced_bt_free_buffer(p_filename);

    fseek(fp, 0, SEEK_END);
    size_t json_string_size = ftell(fp);
    rewind(fp);

    if ((json_string = (char *)wiced_bt_get_buffer(json_string_size + 1)) == NULL)
        return NULL;

    memset(json_string, 0, json_string_size + 1);
    fread(json_string, 1, json_string_size, fp);
    fclose(fp);

    return (json_string);
}

void mesh_client_network_close(void)
{
    mesh_provision_cb_t *p_cb = &provision_cb;

    provision_cb.network_opened = WICED_FALSE;

    if (p_mesh_db != NULL)
    {
        // after application deinit, we will not receive link status
        if ((p_cb->proxy_conn_id != 0) && p_cb->over_gatt)
        {
            if (p_cb->p_connect_status != NULL)
                p_cb->p_connect_status(0, p_cb->proxy_conn_id, 0, p_cb->over_gatt);

            mesh_client_disconnect_network();
        }
        /*
        if (p_cb->provision_conn_id != 0)
            wiced_bt_mesh_provision_disconnect(NULL);
            */
        mesh_application_deinit();

        wiced_stop_timer(&provision_cb.op_timer);
        wiced_deinit_timer(&provision_cb.op_timer);

        wiced_bt_mesh_core_init(NULL);

        wiced_bt_mesh_db_deinit(p_mesh_db);
        p_mesh_db = NULL;
    }
    clean_pending_op_queue(0);
}

int mesh_client_group_create(char *group_name, char *parent_group_name)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    char no_parent_name[1] = { 0 };
    uint16_t group_addr;

    if (p_mesh_db == NULL)
        return MESH_CLIENT_ERR_NETWORK_CLOSED;

    if ((group_name[0] == 0) || wiced_bt_mesh_db_node_get_by_element_name(p_mesh_db, group_name) != NULL)
    {
        Log("cannot create group with component name\n");
        return MESH_CLIENT_ERR_INVALID_ARGS;
    }
    if ((group_addr = wiced_bt_mesh_db_group_get_addr(p_mesh_db, group_name)) != 0)
    {
        Log("group %s already exists", group_name);
        return MESH_CLIENT_ERR_DUPLICATE_NAME;
    }
    if ((p_cb->p_first != NULL) && (p_cb->state != PROVISION_STATE_IDLE))
    {
        Log("Group create state:%d", p_cb->state);
        return MESH_CLIENT_ERR_INVALID_STATE;
    }
    if ((parent_group_name == NULL) ||
        (strcmp(p_mesh_db->name, parent_group_name) == 0))
    {
        parent_group_name = no_parent_name;
    }
    if ((group_addr = wiced_bt_mesh_db_group_add(p_mesh_db, p_mesh_db->unicast_addr, group_name, parent_group_name)) == 0)
    {
        Log("Failed to create group %s", group_name);
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    Log("Group %x allocated for %s", group_addr, group_name);
    wiced_bt_mesh_db_store(p_mesh_db);

#if SUBSCRIBE_LOCAL_MODELS_TO_ALL_GROUPS
    wiced_bt_mesh_db_node_t* p_node;
    pending_operation_t* p_op;
    int i;

    // All models of the local device should be subscribed to the messages published to the group address
    p_node = wiced_bt_mesh_db_node_get_by_element_addr(p_mesh_db, p_mesh_db->unicast_addr);
    for (i = 0; i < p_node->element[0].num_models; i++)
    {
        if ((p_node->element[0].model[i].model.company_id == MESH_COMPANY_ID_BT_SIG) &&
            ((p_node->element[0].model[i].model.id == WICED_BT_MESH_CORE_MODEL_ID_CONFIG_SRV) ||
             (p_node->element[0].model[i].model.id == WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT) ||
             (p_node->element[0].model[i].model.id == WICED_BT_MESH_CORE_MODEL_ID_HEALTH_SRV) ||
             (p_node->element[0].model[i].model.id == WICED_BT_MESH_CORE_MODEL_ID_HEALTH_CLNT) ||
             (p_node->element[0].model[i].model.id == WICED_BT_MESH_CORE_MODEL_ID_REMOTE_PROVISION_SRV) ||
             (p_node->element[0].model[i].model.id == WICED_BT_MESH_CORE_MODEL_ID_REMOTE_PROVISION_CLNT) ||
             (p_node->element[0].model[i].model.id == WICED_BT_MESH_CORE_MODEL_ID_DIRECTED_FORWARDING_SRV) ||
             (p_node->element[0].model[i].model.id == WICED_BT_MESH_CORE_MODEL_ID_DIRECTED_FORWARDING_CLNT)))
            continue;

        if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
        {
            memset(p_op, 0, sizeof(pending_operation_t));
            p_op->operation = CONFIG_OPERATION_MODEL_SUBSCRIBE;
            p_op->p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT, p_cb->unicast_addr, 0xFFFF);
            p_op->uu.model_sub.operation = OPERATION_ADD;
            p_op->uu.model_sub.element_addr = p_cb->unicast_addr;
            p_op->uu.model_sub.company_id = p_node->element[0].model[i].model.company_id;;
            p_op->uu.model_sub.model_id = p_node->element[0].model[i].model.id;
            p_op->uu.model_sub.addr[0] = group_addr & 0xff;
            p_op->uu.model_sub.addr[1] = (group_addr >> 8) & 0xff;
            configure_pending_operation_queue(p_cb, p_op);
        }
    }
    if (p_cb->p_first != NULL)
    {
        p_cb->db_changed = WICED_TRUE;
        configure_execute_pending_operation(p_cb);
    }
    else
#endif
    {
        if (provision_cb.p_database_changed)
            provision_cb.p_database_changed(p_mesh_db->name);
    }
    return MESH_CLIENT_SUCCESS;
}

int mesh_client_group_delete(char *p_group_name)
{
    uint16_t *p_elements_array, *p_element;
    wiced_bt_mesh_db_node_t *p_node;
    wiced_bt_mesh_db_model_id_t *p_models_array;
    uint32_t total_len = 0;
    uint16_t group_addr = wiced_bt_mesh_db_group_get_addr(p_mesh_db, p_group_name);
    pending_operation_t *p_op;
    mesh_provision_cb_t *p_cb = &provision_cb;
    wiced_bt_mesh_db_app_key_t *app_key;
    int j;

    if (p_mesh_db == NULL)
        return MESH_CLIENT_ERR_NETWORK_CLOSED;

    if ((p_elements_array = wiced_bt_mesh_db_get_all_elements(p_mesh_db)) == NULL)
        return MESH_CLIENT_ERR_NETWORK_DB;

    if ((p_cb->state != PROVISION_STATE_IDLE) || !mesh_client_is_proxy_connected())
    {
        // will only allow to delete empty group if not connected
        for (p_element = p_elements_array; *p_element != 0; p_element++)
        {
            p_node = wiced_bt_mesh_db_node_get_by_element_addr(p_mesh_db, *p_element);
            if ((p_node == NULL) || p_node->blocked)
                continue;

            if ((p_models_array = wiced_bt_mesh_db_get_all_models_of_element(p_mesh_db, *p_element, group_addr)) != NULL)
            {
                for (j = 0; p_models_array[j].company_id != MESH_COMPANY_ID_UNUSED; j++)
                {
                    if ((model_needs_default_sub(p_models_array[j].company_id, p_models_array[j].id) != NULL) ||
                        (model_needs_default_pub(p_models_array[j].company_id, p_models_array[j].id) != NULL))
                    {
                        Log("group not empty and not connected:%d or not idle:%d\n", mesh_client_is_proxy_connected(), p_cb->state);
                        wiced_bt_free_buffer(p_models_array);
                        return (p_cb->state != PROVISION_STATE_IDLE) ? MESH_CLIENT_ERR_INVALID_STATE : MESH_CLIENT_ERR_NOT_CONNECTED;
                    }
                }
                wiced_bt_free_buffer(p_models_array);
            }
        }
        wiced_bt_mesh_db_group_delete(p_mesh_db, p_mesh_db->unicast_addr, p_group_name);
        wiced_bt_mesh_db_store(p_mesh_db);

        if (provision_cb.p_database_changed)
            provision_cb.p_database_changed(p_mesh_db->name);

        return MESH_CLIENT_SUCCESS;
    }

    for (p_element = p_elements_array; *p_element != 0; p_element++)
    {
        if ((p_models_array = wiced_bt_mesh_db_get_all_models_of_element(p_mesh_db, *p_element, group_addr)) != NULL)
        {
            for (j = 0; p_models_array[j].company_id != MESH_COMPANY_ID_UNUSED; j++)
            {
                uint16_t dst = wiced_bt_mesh_db_get_node_addr(p_mesh_db, *p_element);

                if (model_needs_default_sub(p_models_array[j].company_id, p_models_array[j].id) != NULL)
                {
                    if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
                    {
                        memset(p_op, 0, sizeof(pending_operation_t));
                        p_op->operation = CONFIG_OPERATION_MODEL_SUBSCRIBE;
                        p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
                        p_op->uu.model_sub.operation = OPERATION_DELETE;
                        p_op->uu.model_sub.element_addr = *p_element;
                        p_op->uu.model_sub.company_id = p_models_array[j].company_id;
                        p_op->uu.model_sub.model_id = p_models_array[j].id;
                        p_op->uu.model_sub.addr[0] = group_addr & 0xff;
                        p_op->uu.model_sub.addr[1] = (group_addr >> 8) & 0xff;
                        configure_pending_operation_queue(p_cb, p_op);
                    }
                }
                else if (model_needs_default_pub(p_models_array[j].company_id, p_models_array[j].id) != NULL)
                {
                    if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
                    {
                        // Publish address 0 means delete publication
                        memset(p_op, 0, sizeof(pending_operation_t));
                        p_op->operation = CONFIG_OPERATION_MODEL_PUBLISH;
                        p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
                        p_op->uu.model_pub.element_addr = *p_element;
                        p_op->uu.model_pub.company_id = p_models_array[j].company_id;
                        p_op->uu.model_pub.model_id = p_models_array[j].id;
#ifdef USE_VENDOR_APPKEY
                        app_key = (p_models_array[j].company_id != MESH_COMPANY_ID_BT_SIG) ? wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Vendor") : wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
                        p_op->uu.model_pub.app_key_idx = app_key->index;
#else
                        app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
                        p_op->uu.model_pub.app_key_idx = app_key->index;
#endif
                        configure_pending_operation_queue(p_cb, p_op);
                    }
                }
            }
            wiced_bt_free_buffer(p_models_array);
        }
    }
    wiced_bt_free_buffer(p_elements_array);

    wiced_bt_mesh_db_group_delete(p_mesh_db, p_mesh_db->unicast_addr, p_group_name);
    wiced_bt_mesh_db_store(p_mesh_db);

    if (p_cb->p_first != NULL)
    {
        p_cb->db_changed = WICED_TRUE;
        p_cb->state = PROVISION_STATE_RECONFIGURATION;
        configure_execute_pending_operation(p_cb);
    }
    else
    {
        if (provision_cb.p_database_changed)
            provision_cb.p_database_changed(p_mesh_db->name);
    }
    return MESH_CLIENT_SUCCESS;
}

void mesh_client_init(mesh_client_init_t *p)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    memset(p_cb, 0, sizeof(mesh_provision_cb_t));
    p_cb->p_unprovisioned_device = p->unprovisioned_device_callback;
    p_cb->p_provision_status = p->provision_status_callback;
    p_cb->p_connect_status = p->connect_status_callback;
    p_cb->p_node_connect_status = p->node_connect_status_callback;
    p_cb->p_database_changed = p->database_changed_callback;
    p_cb->p_onoff_status = p->on_off_changed_callback;
    p_cb->p_level_status = p->level_changed_callback;
    p_cb->p_lightness_status = p->lightness_changed_callback;
    p_cb->p_hsl_status = p->hsl_changed_callback;
    p_cb->p_ctl_status = p->ctl_changed_callback;
    p_cb->p_sensor_status = p->sensor_changed_callback;
    p_cb->p_vendor_specific_data = p->vendor_specific_data_callback;
}

void get_rpl_filename(char *filename)
{
    int i;

    strcpy(filename, "rpl");
    for (i = 0; i < 16; i++)
        sprintf(&filename[3 + (2 * i)], "%02x", p_mesh_db->uuid[i]);

    strcpy(&filename[35], ".bin");
}

void download_iv(uint32_t *p_iv_idx, uint8_t *p_iv_update)
{
    FILE                *fp;
    mesh_client_iv_t    iv;
    char                filename[50];

    // default values for the case when we don't have RPL file
    *p_iv_idx = 0;
    *p_iv_update = WICED_FALSE;

    get_rpl_filename(filename);

    fp = fopen(filename, "rb");
    if (!fp)
        return;

    // just read IV record - first record in the file
    if (fread(&iv, 1, sizeof(iv), fp) == sizeof(iv))
    {
        *p_iv_idx = iv.iv_index;
        *p_iv_update = iv.iv_update != 0 ? WICED_TRUE : WICED_FALSE;
    }
    fclose(fp);
}

void download_rpl_list(void)
{
    FILE *            fp;
    mesh_client_seq_t entry;
    mesh_client_iv_t  iv;
    char              filename[50];
    uint32_t          seq;

    get_rpl_filename(filename);

    fp = fopen(filename, "rb");
    if (!fp)
        return;

    // skip IV record - first record in the file
    if (fread(&iv, 1, sizeof(iv), fp) == sizeof(iv))
    {
        // Read all SEQ records passing them to the mesh core
        while (fread(&entry, 1, sizeof(entry), fp) == sizeof(entry))
        {
            seq = entry.seq[0] + (((uint32_t)entry.seq[1]) << 8) + (((uint32_t)entry.seq[2]) << 16);
            wiced_bt_mesh_core_set_seq(entry.addr, seq, entry.previous_iv_idx != 0 ? WICED_TRUE : WICED_FALSE);
        }
    }
    fclose(fp);
}

void mesh_process_iv_changed(wiced_bt_mesh_core_state_iv_t *p_iv)
{
    FILE                *fp;
    mesh_client_iv_t    iv;
    char                filename[50];

    get_rpl_filename(filename);

    fp = fopen(filename, "rb+");
    if (!fp)
    {
        fp = fopen(filename, "wb+");
        if (!fp)
            return;
    }
    iv.iv_index = p_iv->index;
    iv.iv_update = p_iv->update_flag ? 1 : 0;
    fwrite(&iv, 1, sizeof(iv), fp);
    fclose(fp);
}

void mesh_process_seq_changed(wiced_bt_mesh_core_state_seq_t *p_seq_changed)
{
    FILE *fp;
    mesh_client_seq_t entry;
    mesh_client_iv_t    iv;
    char filename[50];
    uint32_t            seq;

    get_rpl_filename(filename);

    fp = fopen(filename, "rb+");
    if (!fp)
    {
        fp = fopen(filename, "wb+");
        if (!fp)
            return;
    }
    // skip IV record - first record in the file
    if (fread(&iv, 1, sizeof(iv), fp) != sizeof(iv))
    {
        // no IV record. Create it with 0 values
        iv.iv_index = 0;
        iv.iv_update = 0;
        fwrite(&iv, 1, sizeof(iv), fp);
    }
    else
    {
        while (fread(&entry, 1, sizeof(entry), fp) == sizeof(entry))
        {
            if (p_seq_changed->addr == entry.addr)
            {
                // entry with the same addr found, update the entry if it is changed and return
                seq = entry.seq[0] + (((uint32_t)entry.seq[1]) << 8) + (((uint32_t)entry.seq[2]) << 16);
                if ((seq != p_seq_changed->seq)
                    || ((p_seq_changed->previous_iv_idx == WICED_TRUE) != (entry.previous_iv_idx != 0)))
                {
                    entry.previous_iv_idx = p_seq_changed->previous_iv_idx ? 1 : 0;
                    entry.seq[0] = (uint8_t)p_seq_changed->seq;
                    entry.seq[1] = (uint8_t)(p_seq_changed->seq >> 8);
                    entry.seq[2] = (uint8_t)(p_seq_changed->seq >> 16);
                    fseek(fp, 0 - sizeof(entry), SEEK_CUR);
                    fwrite(&entry, 1, sizeof(entry), fp);
                }
                fclose(fp);
                return;
            }
        }
    }
    // not found entry with addr, need to create one and stick at the end of the file
    entry.addr = p_seq_changed->addr;
    entry.previous_iv_idx = p_seq_changed->previous_iv_idx ? 1 : 0;
    entry.seq[0] = (uint8_t)p_seq_changed->seq;
    entry.seq[1] = (uint8_t)(p_seq_changed->seq >> 8);
    entry.seq[2] = (uint8_t)(p_seq_changed->seq >> 16);

    fseek(fp, 0, SEEK_END);
    fwrite(&entry, 1, sizeof(entry), fp);
    fclose(fp);
}

void mesh_del_seq(uint16_t addr)
{
    FILE *fp;
    mesh_client_seq_t *p_entry;
    long file_size;
    char filename[50];
    uint8_t *p_buffer;
    get_rpl_filename(filename);

    fp = fopen(filename, "rb");
    if (!fp)
        return;

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    p_buffer = (uint8_t *)wiced_bt_get_buffer(file_size);
    if (p_buffer == NULL)
    {
        fclose(fp);
        return;
    }
    fseek(fp, 0, SEEK_SET);
    fread(p_buffer, 1, file_size, fp);
    fclose(fp);

    for (p_entry = (mesh_client_seq_t *)(p_buffer + sizeof(mesh_client_iv_t)); (uint8_t *)p_entry < p_buffer + file_size; p_entry++)
    {
        if (p_entry->addr == addr)
            break;
    }
    if ((uint8_t *)p_entry == p_buffer + file_size)
    {
        wiced_bt_free_buffer(p_buffer);
        return;
    }
    fp = fopen(filename, "wb");
    if (!fp)
    {
        wiced_bt_free_buffer(p_buffer);
        return;
    }
    memmove(p_entry, p_entry + 1, file_size - ((uint8_t *)p_entry - p_buffer) - sizeof(mesh_client_seq_t));
    file_size -= sizeof(mesh_client_seq_t);
    fwrite(p_buffer, 1, file_size, fp);
    fclose(fp);
    wiced_bt_free_buffer(p_buffer);
}

uint8_t configure_local_device(uint16_t unicast_addr, uint8_t phase, uint16_t net_key_idx, uint8_t *p_net_key)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_addr(p_mesh_db, unicast_addr);
    if (node == NULL)
        return MESH_CLIENT_ERR_NOT_FOUND;

    // save data in the control block in case we need to retry
    mesh_provision_cb_t *p_cb = &provision_cb;

    p_cb->state = PROVISION_STATE_IDLE;

    memcpy(p_cb->dev_key, node->device_key, sizeof(p_cb->dev_key));
    p_cb->unicast_addr = unicast_addr;

    wiced_bt_mesh_local_device_set_data_t set;
    memset(&set, 0, sizeof(set));

    set.addr = unicast_addr;
    memcpy(set.dev_key, node->device_key, 16);
    memcpy(set.network_key, p_net_key, 16);
    set.net_key_idx = net_key_idx;
    set.key_refresh = phase;

    download_iv(&set.iv_idx, &set.iv_update);

    Log("Set Local Device addr:0x%04x net_key_idx:%04x iv_idx:%d key_refresh:%d iv_updata:%d", set.addr, set.net_key_idx, set.iv_idx, set.key_refresh, set.iv_update);

    wiced_bt_mesh_provision_local_device_set(&set);

    download_rpl_list();

    mesh_configure_composition_data_get(p_cb, OPERATION_LOCAL);
    return MESH_CLIENT_SUCCESS;
}

char *mesh_client_get_all_networks(void)
{
    return wiced_bt_mesh_db_get_all_networks();
}

char *mesh_client_get_all_groups(char *in_group)
{
    if (p_mesh_db == NULL)
        return NULL;

    if (in_group == NULL)
        in_group = p_mesh_db->name;

    return wiced_bt_mesh_db_get_all_groups(p_mesh_db, in_group);
}

char *mesh_client_get_all_provisioners(void)
{
    if (p_mesh_db == NULL)
        return NULL;

    return wiced_bt_mesh_db_get_all_provisioners(p_mesh_db);
}

uint8_t model_component_type(wiced_bt_mesh_db_model_id_t *p_models_array)
{
    uint16_t model_id, company_id;
    uint16_t j;
    uint8_t component_type = DEVICE_TYPE_UNKNOWN;

    for (j = 0; ; j++)
    {
        company_id = p_models_array[j].company_id;
        if (company_id == MESH_COMPANY_ID_UNUSED)
            break;

        if (company_id == MESH_COMPANY_ID_BT_SIG)
        {
            model_id = p_models_array[j].id;
            switch (model_id)
            {
            case WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SETUP_SRV:
                component_type = DEVICE_TYPE_LIGHT_HSL;
                break;

            case WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_HUE_SRV:
                component_type = DEVICE_TYPE_LIGHT_HSL_HUE;
                break;

            case WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SATURATION_SRV:
                component_type = DEVICE_TYPE_LIGHT_HSL_SATURATION;
                break;

            case WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_SETUP_SRV:
                component_type = DEVICE_TYPE_LIGHT_CTL;
                break;

            case WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_TEMPERATURE_SRV:
                component_type = DEVICE_TYPE_LIGHT_CTL_TEMPERATURE;
                break;

            case WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LC_SETUP_SRV:
                component_type = DEVICE_TYPE_LIGHT_CONTROLLER;
                break;

            case WICED_BT_MESH_CORE_MODEL_ID_LIGHT_XYL_SETUP_SRV:
                if (component_type != DEVICE_TYPE_LIGHT_HSL)
                    component_type = DEVICE_TYPE_LIGHT_XYL;
                break;

            case WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_SETUP_SRV:
                if (component_type < DEVICE_TYPE_LIGHT_DIMMABLE)
                    component_type = DEVICE_TYPE_LIGHT_DIMMABLE;
                break;

            case WICED_BT_MESH_CORE_MODEL_ID_GENERIC_POWER_LEVEL_SETUP_SRV:
                if (component_type < DEVICE_TYPE_POWER_LEVEL_SERVER)
                    component_type = DEVICE_TYPE_POWER_LEVEL_SERVER;
                break;

            case WICED_BT_MESH_CORE_MODEL_ID_GENERIC_POWER_ONOFF_SETUP_SRV:
                if (component_type < DEVICE_TYPE_POWER_ON_OFF_SERVER)
                    component_type = DEVICE_TYPE_POWER_ON_OFF_SERVER;
                break;

            case WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_SRV:
                if (component_type < DEVICE_TYPE_GENERIC_LEVEL_SERVER)
                    component_type = DEVICE_TYPE_GENERIC_LEVEL_SERVER;
                break;

            case WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_SRV:
                if (component_type < DEVICE_TYPE_GENERIC_ON_OFF_SERVER)
                    component_type = DEVICE_TYPE_GENERIC_ON_OFF_SERVER;
                break;

            case WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_CLNT:
                component_type = DEVICE_TYPE_GENERIC_ON_OFF_CLIENT;
                break;

            case WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_CLNT:
                component_type = DEVICE_TYPE_GENERIC_LEVEL_CLIENT;
                break;

            case WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV:
                if (component_type < DEVICE_TYPE_SENSOR_SERVER)
                    component_type = DEVICE_TYPE_SENSOR_SERVER;
                break;

            case WICED_BT_MESH_CORE_MODEL_ID_SENSOR_CLNT:
                if (component_type < DEVICE_TYPE_SENSOR_CLIENT)
                    component_type = DEVICE_TYPE_SENSOR_CLIENT;
                break;

            case WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LOCATION_SRV:
                if (component_type < DEVICE_TYPE_LOCATION_SERVER)
                    component_type = DEVICE_TYPE_LOCATION_SERVER;
                break;

            case WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LOCATION_CLNT:
                if (component_type < DEVICE_TYPE_LOCATION_CLIENT)
                    component_type = DEVICE_TYPE_LOCATION_CLIENT;
                break;
            }
        }
    }
    return component_type;
}

uint8_t get_component_type(void *p_mesh_db, uint16_t element_addr)
{
    uint8_t component_type = DEVICE_TYPE_UNKNOWN;
    wiced_bt_mesh_db_model_id_t *p_models_array;

    if (p_mesh_db == NULL)
        return MESH_CLIENT_ERR_NETWORK_CLOSED;

    if ((p_models_array = wiced_bt_mesh_db_get_all_models_of_element(p_mesh_db, element_addr, 0)) != NULL)
    {
        component_type = model_component_type(p_models_array);
    }
    wiced_bt_free_buffer(p_models_array);
    return component_type;
}

/*
 * Light Controller can be on the next element, but not always. For example, there
 * can be Light Lightness, followed by color light temperature and then light controller.
 * Return the address of the Light Controller element, or 0 if not present.
 */
uint16_t get_light_lc_element_addr(uint16_t component_addr)
{
    uint16_t element_addr;

    for (element_addr = component_addr + 1; ; element_addr++)
    {
        if (!is_secondary_element(element_addr))
            return 0;

        if (is_model_present(element_addr, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LC_SRV))
            return element_addr;
    }
}

int mesh_client_get_component_type(char *component_name)
{
    uint16_t *p_elements_array, *p_element;
    uint8_t component_type;

    if (p_mesh_db == NULL)
        return MESH_CLIENT_ERR_NETWORK_CLOSED;

    if ((p_elements_array = wiced_bt_mesh_db_get_all_elements(p_mesh_db)) == NULL)
        return MESH_CLIENT_ERR_NETWORK_DB;

    for (p_element = p_elements_array; *p_element != 0; p_element++)
    {
        if (is_secondary_element(*p_element))
            continue;

        const char *p_element_name = wiced_bt_mesh_db_get_element_name(p_mesh_db, *p_element);
        component_type = get_component_type(p_mesh_db, *p_element);
        if ((p_element_name != NULL) && (strcmp(component_name, p_element_name) == 0))
        {
            wiced_bt_free_buffer(p_elements_array);
            return component_type;
        }
    }
    wiced_bt_free_buffer(p_elements_array);
    return DEVICE_TYPE_UNKNOWN;
}

int mesh_client_is_light_controller(char* component_name)
{
    uint16_t component_addr = get_device_addr(component_name);
    if (component_addr != 0)
        return (get_light_lc_element_addr(component_addr) != 0);
    return 0;
}

uint8_t mesh_client_get_component_info(char *component_name, mesh_client_component_info_status_t p_component_info_status_callback)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    uint16_t dst = get_device_addr(component_name);
    wiced_bt_mesh_db_app_key_t *app_key;
    char buf[200];

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (dst != 0)
    {
        if (!is_model_present(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_USER_PROPERTY_SRV))
        {
            wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_addr(p_mesh_db, dst);
            if (node == NULL)
                return MESH_CLIENT_ERR_NOT_FOUND;

            sprintf(buf, "CID:%05d PID:%05d VID:%05d VER:Not Available", node->cid, node->pid, node->vid);

            if (p_component_info_status_callback != NULL)
                p_component_info_status_callback(0, (char *)wiced_bt_mesh_db_get_element_name(p_mesh_db, dst), buf);

            return MESH_CLIENT_SUCCESS;
        }
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    wiced_bt_mesh_event_t *p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_USER_PROPERTY_SRV, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("property get no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    Log("Property Get addr:%04x app_key_idx:%04x", p_event->dst, p_event->app_key_idx);

    wiced_bt_mesh_property_get_data_t get_data;

    get_data.type = WICED_BT_MESH_PROPERTY_TYPE_USER;
    get_data.id   = WICED_BT_MESH_PROPERTY_DEVICE_FIRMWARE_REVISION;
    provision_cb.p_component_info_status_callback = p_component_info_status_callback;

    wiced_bt_mesh_model_property_client_send_property_get(p_event, &get_data);
    return MESH_CLIENT_SUCCESS;
}

int element_is_in_group(uint16_t element_addr, uint16_t group_addr)
{
    return (int)wiced_bt_mesh_db_element_is_in_group(p_mesh_db, element_addr, group_addr);
}

char *mesh_client_get_group_components(char *p_group_name)
{
    wiced_bt_mesh_db_node_t *p_node;
    uint16_t *p_elements_array, *p_element;
    char *p_components_array;
    uint8_t num_components = 0;
    uint32_t total_len = 0;

    if (p_mesh_db == NULL)
        return NULL;

    uint16_t group_addr = 0;
    if (strcmp(p_group_name, p_mesh_db->name) == 0)
        group_addr = 0xffff;
    else
        group_addr = wiced_bt_mesh_db_group_get_addr(p_mesh_db, p_group_name);

    if (group_addr == 0)
        return NULL;

    if ((p_elements_array = wiced_bt_mesh_db_get_all_elements(p_mesh_db)) == NULL)
        return NULL;

    for (p_element = p_elements_array; *p_element != 0; p_element++)
    {
        if (is_secondary_element(*p_element))
            continue;

        p_node = wiced_bt_mesh_db_node_get_by_element_addr(p_mesh_db, *p_element);
        if ((p_node == NULL) || p_node->blocked)
            continue;

        if (element_is_in_group(*p_element, group_addr))
            total_len += (strlen(get_component_name(*p_element)) + 1);
    }
    total_len++;

    if ((p_components_array = (char *)wiced_bt_get_buffer(total_len)) == NULL)
    {
        wiced_bt_free_buffer(p_elements_array);
        return NULL;
    }
    memset(p_components_array, 0, total_len);
    total_len = 0;
    for (p_element = p_elements_array; *p_element != 0; p_element++)
    {
        if (is_secondary_element(*p_element))
            continue;

        p_node = wiced_bt_mesh_db_node_get_by_element_addr(p_mesh_db, *p_element);
        if ((p_node == NULL) || p_node->blocked)
            continue;

        if (element_is_in_group(*p_element, group_addr))
        {
            const char *p_name = get_component_name(*p_element);
            strcpy(&p_components_array[total_len], p_name);
            total_len += (strlen(p_name) + 1);
        }
    }
    p_components_array[total_len] = 0;
    wiced_bt_free_buffer(p_elements_array);
    return p_components_array;
}

/*
 * Get Name of the Components.
 * The function returns the name of the components with specified address.
 */
const char *get_component_name(uint16_t addr)
{
    char buffer[30];

    wiced_bt_mesh_db_element_t *element = wiced_bt_mesh_db_get_element(p_mesh_db, addr);
    if (element == NULL)
        return NULL;

    const char *p_name = wiced_bt_mesh_db_get_element_name(p_mesh_db, addr);
    if (p_name != NULL)
        return p_name;

    strcpy(buffer, p_device_type_name[get_component_type(p_mesh_db, addr)]);
    sprintf(&buffer[strlen(buffer)], " (%04x)", addr);
    wiced_bt_mesh_db_set_element_name(p_mesh_db, addr, buffer);
    wiced_bt_mesh_db_store(p_mesh_db);
    provision_cb.db_changed = WICED_TRUE;
    return get_component_name(addr);
}

void mesh_client_set_component_name(uint16_t addr, const char *p_name)
{
    if (p_mesh_db == NULL)
        return;

    wiced_bt_mesh_db_set_element_name(p_mesh_db, addr, p_name);
}

int mesh_client_rename(char *old_name, char *new_name)
{
    uint16_t *p_elements_array, *p_element;
    uint16_t element_addr = 0;

    if (p_mesh_db == NULL)
        return MESH_CLIENT_ERR_NETWORK_CLOSED;

    // The name should not be empty string
    if (strlen(new_name) == 0)
        return MESH_CLIENT_ERR_INVALID_ARGS;

    // The name should be unique
    if ((wiced_bt_mesh_db_group_get_by_name(p_mesh_db, new_name) != NULL) ||
        (wiced_bt_mesh_db_node_get_by_element_name(p_mesh_db, new_name) != NULL))
        return MESH_CLIENT_ERR_DUPLICATE_NAME;

    // If this is a group, just change the name in the DB.
    if (wiced_bt_mesh_db_group_rename(p_mesh_db, old_name, new_name))
    {
        wiced_bt_mesh_db_store(p_mesh_db);

        if (provision_cb.p_database_changed)
            provision_cb.p_database_changed(p_mesh_db->name);

        return MESH_CLIENT_SUCCESS;
    }
    // check that new name does not exist
    if ((p_elements_array = wiced_bt_mesh_db_get_all_elements(p_mesh_db)) == NULL)
        return MESH_CLIENT_ERR_NETWORK_DB;

    // make sure that device with new name does not exist yet
    for (p_element = p_elements_array; *p_element != 0; p_element++)
    {
        if (is_secondary_element(*p_element))
            continue;

        const char *p_name = get_component_name(*p_element);

        if (strcmp(old_name, p_name) == 0)
            element_addr = *p_element;

        if (strcmp(new_name, p_name) == 0)
        {
            wiced_bt_free_buffer(p_elements_array);
            return MESH_CLIENT_ERR_NETWORK_DB;
        }
    }
    wiced_bt_free_buffer(p_elements_array);
    if (element_addr == 0)
        return MESH_CLIENT_ERR_NETWORK_DB;

    wiced_bt_mesh_db_set_element_name(p_mesh_db, element_addr, new_name);
    wiced_bt_mesh_db_store(p_mesh_db);

    if (provision_cb.p_database_changed)
        provision_cb.p_database_changed(p_mesh_db->name);

    return MESH_CLIENT_SUCCESS;
}

/*
 * Get Address of the Component.
 */
uint16_t get_device_addr(const char *p_dev_name)
{
    uint16_t *p_elements_array, *p_element;
    uint16_t element_addr = 0;
    wiced_bt_mesh_db_node_t *p_node;

    if (p_mesh_db == NULL)
        return 0;

    if ((p_elements_array = wiced_bt_mesh_db_get_all_elements(p_mesh_db)) == NULL)
        return 0;

    // find first element of the component with component_name
    for (p_element = p_elements_array; *p_element != 0; p_element++)
    {
        if (is_secondary_element(*p_element))
            continue;

        p_node = wiced_bt_mesh_db_node_get_by_element_addr(p_mesh_db, *p_element);
        if ((p_node == NULL) || p_node->blocked)
            continue;

        const char *p_name = get_component_name(*p_element);
        if (strcmp(p_name, p_dev_name) == 0)
        {
            element_addr = *p_element;
            break;
        }
    }
    wiced_bt_free_buffer(p_elements_array);
    return element_addr;
}

/*
 * Get Address of the Group.
 */
uint16_t get_group_addr(const char *p_dev_name)
{
    return wiced_bt_mesh_db_group_get_addr(p_mesh_db, p_dev_name);
}

char *mesh_client_get_device_components(uint8_t *p_uuid)
{
    uint16_t *p_elements_array, *p_element;
    char *p_component_names;
    uint32_t total_len = 0;

    if (p_mesh_db == NULL)
        return NULL;

    if ((p_elements_array = wiced_bt_mesh_db_get_device_elements(p_mesh_db, p_uuid)) == NULL)
        return NULL;

    for (p_element = p_elements_array; *p_element != 0; p_element++)
    {
        if (is_secondary_element(*p_element))
            continue;

        total_len += (strlen(get_component_name(*p_element)) + 1);
    }

    if ((p_component_names = (char *)wiced_bt_get_buffer(total_len + 1)) == NULL)
    {
        wiced_bt_free_buffer(p_elements_array);
        return NULL;
    }
    memset(p_component_names, 0, (total_len + 1));
    total_len = 0;
    for (p_element = p_elements_array; *p_element != 0; p_element++)
    {
        if (is_secondary_element(*p_element))
            continue;

        const char *p_name = get_component_name(*p_element);
        strcpy(&p_component_names[total_len], p_name);
        total_len += (strlen(p_name) + 1);
    }
    wiced_bt_free_buffer(p_elements_array);
    return p_component_names;
}

/*
 * Set Device Configuration.
 * The function sets up configuration for the new devices, or reconfigures existing device
 * If device_name parameter is NULL, the configuration parameters will apply to the devices that will be configured.
 */
int mesh_client_set_device_config(const char *device_name, int is_gatt_proxy, int is_friend, int is_relay, int beacon, int relay_xmit_count, int relay_xmit_interval, int default_ttl, int net_xmit_count, int net_xmit_interval)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    pending_operation_t *p_op;
    uint16_t dst;
    wiced_bt_mesh_db_node_t *node;
    uint8_t     state;
    uint8_t     ttl;
    uint16_t    count;
    uint32_t    interval;

    if (device_name == NULL)
    {
        p_cb->is_gatt_proxy = (uint8_t)is_gatt_proxy;
        p_cb->is_friend = (uint8_t)is_friend;
        p_cb->is_relay = (uint8_t)is_relay;
        p_cb->beacon = (uint8_t)beacon;
        p_cb->relay_xmit_count = (uint8_t)relay_xmit_count;
        p_cb->relay_xmit_interval = (uint16_t)relay_xmit_interval;
        p_cb->default_ttl = (uint8_t)default_ttl;
        p_cb->net_xmit_count = (uint8_t)net_xmit_count;
        p_cb->net_xmit_interval = (uint32_t)net_xmit_interval;
        return MESH_CLIENT_SUCCESS;
    }
    if (p_mesh_db == NULL)
        return MESH_CLIENT_ERR_NETWORK_CLOSED;

    dst = get_device_addr(device_name);
    if (dst == 0)
        return MESH_CLIENT_ERR_NOT_FOUND;

    node = wiced_bt_mesh_db_node_get_by_addr(p_mesh_db, dst);
    if (node == NULL)
        return MESH_CLIENT_ERR_NETWORK_DB;

    if ((p_cb->p_first != NULL) && (p_cb->state != PROVISION_STATE_IDLE))
    {
        Log("Device reconfigure state:%d", p_cb->state);
        return MESH_CLIENT_ERR_INVALID_STATE;
    }
    clean_pending_op_queue(0);

    if (!wiced_bt_mesh_db_net_transmit_get(p_mesh_db, dst, &count, &interval) ||
        (count != net_xmit_count) || (interval != net_xmit_interval))
    {
        if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
        {
            // wiced_bt_mesh_config_network_transmit_set_data_t
            p_op->operation = CONFIG_OPERATION_NET_TRANSMIT_SET;
            p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
            p_op->uu.net_transmit_set.count = net_xmit_count;
            p_op->uu.net_transmit_set.interval = net_xmit_interval;
            configure_pending_operation_queue(p_cb, p_op);
        }
    }
    if (!wiced_bt_mesh_db_default_ttl_get(p_mesh_db, dst, &ttl) ||
        (ttl != default_ttl))
    {
        if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
        {
            //wiced_bt_mesh_config_default_ttl_set_data_t
            p_op->operation = CONFIG_OPERATION_DEFAULT_TTL_SET;
            p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
            p_op->uu.default_ttl_set.ttl = default_ttl;
            configure_pending_operation_queue(p_cb, p_op);
        }
    }
    if (!wiced_bt_mesh_db_relay_get(p_mesh_db, dst, &state, &count, &interval) ||
        (state != is_relay) || (count != relay_xmit_count) || (interval != relay_xmit_interval))
    {
        if ((state != MESH_FEATURE_UNSUPPORTED) &&
            ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL))
        {
            p_op->operation = CONFIG_OPERATION_RELAY_SET;
            p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
            p_op->uu.relay_set.state = is_relay;
            p_op->uu.relay_set.retransmit_count = relay_xmit_count;
            p_op->uu.relay_set.retransmit_interval = relay_xmit_interval;
            configure_pending_operation_queue(p_cb, p_op);
        }
    }
    if (!wiced_bt_mesh_db_beacon_get(p_mesh_db, dst, &state) ||
        (state != beacon))
    {
        if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
        {
            p_op->operation = CONFIG_OPERATION_NET_BEACON_SET;
            p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
            p_op->uu.beacon_set.state = beacon;
            configure_pending_operation_queue(p_cb, p_op);
        }
    }
    if (!wiced_bt_mesh_db_gatt_proxy_get(p_mesh_db, dst, &state) ||
        (state != is_gatt_proxy))
    {
        if ((state != MESH_FEATURE_UNSUPPORTED) &&
            ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL))
        {
            //wiced_bt_mesh_config_gatt_proxy_set_data_t
            p_op->operation = CONFIG_OPERATION_PROXY_SET;
            p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
            p_op->uu.proxy_set.state = is_gatt_proxy;
            configure_pending_operation_queue(p_cb, p_op);
        }
    }
    if (!wiced_bt_mesh_db_friend_get(p_mesh_db, dst, &state) ||
        (state != is_friend))
    {
        if ((state != MESH_FEATURE_UNSUPPORTED) &&
            ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL))
        {
            //wiced_bt_mesh_config_friend_set_data_t
            p_op->operation = CONFIG_OPERATION_FRIEND_SET;
            p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
            p_op->uu.friend_set.state = is_friend;
            configure_pending_operation_queue(p_cb, p_op);
        }
    }
    if (p_cb->p_first != NULL)
    {
        p_cb->state = PROVISION_STATE_RECONFIGURATION;
        configure_execute_pending_operation(p_cb);
    }
    return MESH_CLIENT_SUCCESS;
}

/*
 * Set Publication Configuration.
 * The function sets up publication configuration for the new devices, or reconfigures existing device.
 * If the device_name parameter is NULL, the configuration parameters apply to the devices that a newly provisioned.  In this case publication
 * information is ued for the main function of the device.  For example if a dimmer is being provisioned, the publication will be configured
 * for the Generic Level Client model of the device.
 * If the device_name parameter is not NULL, the application can select which it applies to..
 */
int mesh_client_set_publication_config(int publish_credential_flag, int publish_retransmit_count, int publish_retransmit_interval, int publish_ttl)
{
    mesh_provision_cb_t *p_cb = &provision_cb;

    if (p_mesh_db == NULL)
        return MESH_CLIENT_ERR_NETWORK_CLOSED;

    if (((publish_credential_flag != 0) && (publish_credential_flag != 1)) ||
        (publish_retransmit_count > 7) ||
        (publish_ttl > 0x7f))
        return MESH_CLIENT_ERR_INVALID_ARGS;

    p_cb->publish_credential_flag = (uint8_t)publish_credential_flag;
    p_cb->publish_retransmit_count = (uint8_t)publish_retransmit_count;
    p_cb->publish_retransmit_interval = (uint16_t)publish_retransmit_interval;
    p_cb->publish_ttl = (uint8_t)publish_ttl;
    return MESH_CLIENT_SUCCESS;
}

int mesh_client_sensor_setting_set(const char *device_name, int property_id, int setting_property_id, uint8_t *val)
{
    wiced_bt_mesh_sensor_setting_set_data_t config_data;
    wiced_bt_mesh_db_setting_t curr_setting_data;
    wiced_bt_mesh_db_app_key_t *app_key;
    uint8_t  prop_value_len;
    uint16_t dst = get_device_addr(device_name);
    uint16_t num_nodes = 0;
    uint16_t *group_list = NULL;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }

    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Setup");

    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }

    if (dst != 0)
    {
        if (!is_model_present(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV))
        {
            Log("sensor model not present\n");
            return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
        }

        if (wiced_bt_mesh_db_node_model_setting_get(p_mesh_db, dst, (uint16_t)property_id, &curr_setting_data, (uint16_t)setting_property_id, &prop_value_len))
        {
            if (memcmp(val, curr_setting_data.val, prop_value_len) == 0)
            {
                Log("Setting value is same\n");
                return MESH_CLIENT_SUCCESS;
            }
        }
    }
    else
    {
        dst = get_group_addr(device_name);
        if (dst != 0)
        {
            group_list = mesh_get_group_list(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV, &num_nodes);
            if (group_list == NULL)
            {
                Log("sensor model not present in group\n");
                return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
            }
            if (num_nodes == 1)
                dst = group_list[0];

            wiced_bt_free_buffer(group_list);
        }
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }

    config_data.setting_property_id = (uint16_t)setting_property_id;
    config_data.property_id = (uint16_t)property_id;
    config_data.prop_value_len = wiced_bt_mesh_db_get_property_value_len(setting_property_id);
    memcpy(config_data.setting_raw_val, val, config_data.prop_value_len);
    wiced_bt_mesh_event_t *p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_CLNT, dst, app_key->index);

    if (p_event == NULL)
    {
        Log("setting get no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    p_event->reply = WICED_TRUE;

    return wiced_bt_mesh_model_sensor_client_sensor_setting_send_set(p_event, &config_data);
}

int *mesh_client_sensor_property_list_get(const char *device_name)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    uint16_t element_addr;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return NULL;
    }
    element_addr = get_device_addr(device_name);
    if (element_addr == 0)
        return NULL;

    return wiced_bt_mesh_db_node_sensor_get_property_ids(p_mesh_db, element_addr);
}

int mesh_client_sensor_get(const char *p_name, int property_id)
{
    uint16_t dst = get_device_addr(p_name);
    wiced_bt_mesh_sensor_get_t get_data;
    uint16_t num_nodes = 0;
    uint16_t *group_list = NULL;
    wiced_bt_mesh_db_app_key_t *app_key;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }

    if (dst != 0)
    {
        if (!is_model_present(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV))
        {
            Log("sensor model not present\n");
            return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
        }
    }
    else
    {
        dst = get_group_addr(p_name);
        if (dst != 0)
        {
            group_list = mesh_get_group_list(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV, &num_nodes);
            if (group_list == NULL)
            {
                Log("sensor model not present in group\n");
                return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
            }
            if (num_nodes == 1)
                dst = group_list[0];

            wiced_bt_free_buffer(group_list);
        }
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }

    get_data.property_id = (uint16_t)property_id;
    wiced_bt_mesh_event_t *p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_CLNT, dst, app_key->index);

    if (p_event == NULL)
    {
        Log("sensor get no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    Log("Sensor Get addr:%04x app_key_idx:%04x", p_event->dst, p_event->app_key_idx);
    wiced_bt_mesh_model_sensor_client_sensor_send_get(p_event, &get_data);

#if 0 // for test only
    Log("Sensor Setting Get addr:%04x app_key_idx:%04x id property_id:%04x setting id:%04x", p_event->dst, p_event->app_key_idx, property_id, 0x6e);

    wiced_bt_mesh_sensor_setting_get_data_t setting_data;
    setting_data.property_id = (uint16_t)property_id;
    setting_data.setting_property_id = (uint16_t)0x6e;
    wiced_bt_mesh_model_sensor_client_sensor_setting_send_get(p_event, &setting_data);
#endif
    return MESH_CLIENT_SUCCESS;
}

int* mesh_client_sensor_setting_property_ids_get(const char *device_name, int property_id)
{
    if (p_mesh_db == NULL)
        return NULL;

    uint16_t element_addr = get_device_addr(device_name);
    if (element_addr == 0)
        return NULL;

    if (!is_model_present(element_addr, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV))
    {
        Log("sensor model not present\n");
        return NULL;
    }
    return wiced_bt_mesh_db_node_model_get_setting_property_ids(p_mesh_db, element_addr, (uint16_t)property_id);
}

int mesh_client_sensor_cadence_set(const char *p_name, int property_id,
                                   uint16_t fast_cadence_period_divisor, wiced_bool_t trigger_type,
                                   uint32_t trigger_delta_down, uint32_t trigger_delta_up,
                                   uint32_t min_interval, uint32_t fast_cadence_low,
                                   uint32_t fast_cadence_high)
{
    uint16_t dst = get_device_addr(p_name);
    wiced_bt_mesh_sensor_cadence_set_data_t config_data;
    wiced_bt_mesh_db_cadence_t curr_cadence_data;
    uint16_t num_nodes = 0;
    uint16_t *group_list = NULL;
    wiced_bt_mesh_db_app_key_t *app_key;
    uint8_t prop_value_len;
    wiced_bool_t is_data_present;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }

    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Setup");

    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }

    if (dst != 0)
    {
        if (!is_model_present(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV))
        {
            Log("sensor model not present\n");
            return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
        }
        if (wiced_bt_mesh_db_node_model_cadence_get(p_mesh_db, &is_data_present, dst, (uint16_t)property_id, &curr_cadence_data, &prop_value_len))
        {
            if (is_data_present)
            {
                if (!((curr_cadence_data.fast_cadence_period_divisor != fast_cadence_period_divisor) ||
                      (curr_cadence_data.min_interval != min_interval) ||
                      (fast_cadence_low != curr_cadence_data.fast_cadence_low) ||
                      (fast_cadence_high != curr_cadence_data.fast_cadence_high) ||
                      (trigger_type != curr_cadence_data.trigger_type) ||
                      (trigger_delta_down != curr_cadence_data.trigger_delta_down) ||
                      (trigger_delta_up != curr_cadence_data.trigger_delta_up)))
                {
                    Log("All values are same as current values\n");
                    return MESH_CLIENT_SUCCESS;
                }
            }
        }
    }
    else
    {
        dst = get_group_addr(p_name);
        if (dst != 0)
        {
            group_list = mesh_get_group_list(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV, &num_nodes);
            if (group_list == NULL)
            {
                Log("sensor model not present in group\n");
                return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
            }
            if (num_nodes == 1)
                dst = group_list[0];

            wiced_bt_free_buffer(group_list);
        }
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }

    config_data.property_id = (uint16_t)property_id;
    config_data.prop_value_len = wiced_bt_mesh_db_get_property_value_len(property_id);
    config_data.cadence_data.trigger_type = trigger_type;
    config_data.cadence_data.min_interval = min_interval;
    config_data.cadence_data.fast_cadence_period_divisor = fast_cadence_period_divisor;

    config_data.cadence_data.trigger_delta_down = trigger_delta_down;
    config_data.cadence_data.trigger_delta_up = trigger_delta_up;

    config_data.cadence_data.fast_cadence_low = fast_cadence_low;
    config_data.cadence_data.fast_cadence_high = fast_cadence_high;

    wiced_bt_mesh_event_t *p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("cadence set no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }

    p_event->reply = WICED_TRUE;
    return wiced_bt_mesh_model_sensor_client_sensor_cadence_send_set(p_event, &config_data);
}

int mesh_client_sensor_cadence_get(const char *device_name, int property_id,
    uint16_t *fast_cadence_period_divisor, wiced_bool_t *trigger_type,
    uint32_t *trigger_delta_down, uint32_t *trigger_delta_up,
    uint32_t *min_interval, uint32_t *fast_cadence_low,
    uint32_t *fast_cadence_high)
{
    wiced_bool_t is_data_present;
    wiced_bt_mesh_db_cadence_t curr_cadence_data;
    uint8_t prop_value_len;

    if (p_mesh_db == NULL)
        return MESH_CLIENT_ERR_NETWORK_CLOSED;

    uint16_t dst = get_device_addr(device_name);
    if (dst == 0)
        return MESH_CLIENT_ERR_NOT_FOUND;

    if (!is_model_present(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV))
    {
        Log("sensor model not present\n");
        return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
    }
    if (!wiced_bt_mesh_db_node_model_cadence_get(p_mesh_db, &is_data_present, dst, (uint16_t)property_id, &curr_cadence_data, &prop_value_len))
        return MESH_CLIENT_ERR_NOT_FOUND;

    if (!is_data_present)
        return MESH_CLIENT_ERR_NOT_FOUND;

    *fast_cadence_period_divisor = curr_cadence_data.fast_cadence_period_divisor;
    *trigger_type = curr_cadence_data.trigger_type;
    *trigger_delta_down = curr_cadence_data.trigger_delta_down;
    *trigger_delta_up = curr_cadence_data.trigger_delta_up;
    *min_interval = curr_cadence_data.min_interval;
    *fast_cadence_low = curr_cadence_data.fast_cadence_low;
    *fast_cadence_high = curr_cadence_data.fast_cadence_high;
    return MESH_CLIENT_SUCCESS;
}

int mesh_client_light_lc_mode_get(const char* p_name, mesh_client_light_lc_mode_status_t p_lc_mode_status)
{
    uint16_t dst = get_device_addr(p_name);
    wiced_bt_mesh_db_app_key_t* app_key;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Setup");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst != 0)
    {
        dst = get_light_lc_element_addr(dst);
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    wiced_bt_mesh_event_t* p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LC_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("property get no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    provision_cb.p_light_lc_mode_status = p_lc_mode_status;

    Log("Light LC Mode Get addr:%04x app_key_idx:%04x", p_event->dst, p_event->app_key_idx);
    wiced_bt_mesh_model_light_lc_client_send_mode_get(p_event);
    return MESH_CLIENT_SUCCESS;
}

int mesh_client_light_lc_mode_set(const char* p_name, int mode, mesh_client_light_lc_mode_status_t p_lc_mode_status)
{
    wiced_bt_mesh_light_lc_mode_set_data_t data;
    uint16_t dst = get_device_addr(p_name);
    wiced_bt_mesh_db_app_key_t* app_key;

    if ((mode != 0 ) && (mode != 1))
    {
        Log("invalid arg\n");
        return MESH_CLIENT_ERR_INVALID_ARGS;
    }
    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Setup");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst != 0)
    {
        dst = get_light_lc_element_addr(dst);
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    wiced_bt_mesh_event_t* p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LC_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("property get no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    p_event->reply = WICED_TRUE;

    data.mode = mode;
    provision_cb.p_light_lc_mode_status = p_lc_mode_status;

    Log("Light LC Mode Set addr:%04x app_key_idx:%04x mode:%d", p_event->dst, p_event->app_key_idx, mode);
    wiced_bt_mesh_model_light_lc_client_send_mode_set(p_event, &data);
    return MESH_CLIENT_SUCCESS;
}

int mesh_client_light_lc_occupancy_mode_get(const char* p_name, mesh_client_light_lc_occupancy_mode_status_t p_lc_occupancy_mode_status)
{
    uint16_t dst = get_device_addr(p_name);
    wiced_bt_mesh_db_app_key_t* app_key;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Setup");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst != 0)
    {
        dst = get_light_lc_element_addr(dst);
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    wiced_bt_mesh_event_t* p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LC_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("property get no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    provision_cb.p_light_lc_occupancy_mode_status = p_lc_occupancy_mode_status;

    Log("Light LC Occupancy Mode Get addr:%04x app_key_idx:%04x", p_event->dst, p_event->app_key_idx);
    wiced_bt_mesh_model_light_lc_client_send_occupancy_mode_get(p_event);
    return MESH_CLIENT_SUCCESS;
}

int mesh_client_light_lc_occupancy_mode_set(const char* p_name, int mode, mesh_client_light_lc_occupancy_mode_status_t p_lc_occupancy_mode_status)
{
    wiced_bt_mesh_light_lc_occupancy_mode_set_data_t data;
    uint16_t dst = get_device_addr(p_name);
    wiced_bt_mesh_db_app_key_t* app_key;

    if ((mode != 0) && (mode != 1))
    {
        Log("invalid arg\n");
        return MESH_CLIENT_ERR_INVALID_ARGS;
    }
    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Setup");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst != 0)
    {
        dst = get_light_lc_element_addr(dst);
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    wiced_bt_mesh_event_t* p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LC_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("property get no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    p_event->reply = WICED_TRUE;

    data.mode = mode;
    provision_cb.p_light_lc_occupancy_mode_status = p_lc_occupancy_mode_status;

    Log("Light LC Occupancy Mode Set addr:%04x app_key_idx:%04x mode:%d", p_event->dst, p_event->app_key_idx, mode);
    wiced_bt_mesh_model_light_lc_client_send_occupancy_mode_set(p_event, &data);
    return MESH_CLIENT_SUCCESS;
}

/*
 * Property Get.
 * If operation is successful, the callback will be executed when reply from the peer is received.
 */
int mesh_client_light_lc_property_get(const char* p_name, int property_id, mesh_client_light_lc_property_status_t p_property_status_callback)
{
    uint16_t dst = get_device_addr(p_name);
    wiced_bt_mesh_light_lc_property_get_data_t get_data;
    wiced_bt_mesh_db_app_key_t* app_key;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Setup");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }

    if (dst != 0)
    {
        if ((dst = get_light_lc_element_addr(dst)) == 0)
        {
            Log("property model not present\n");
            return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
        }
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }

    get_data.id = (uint16_t)property_id;

    wiced_bt_mesh_event_t* p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LC_CLNT, dst, app_key->index);

    if (p_event == NULL)
    {
        Log("property get no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    provision_cb.p_light_lc_property_status = p_property_status_callback;

    Log("Light LC Property Get addr:%04x app_key_idx:%04x ID:%04x", p_event->dst, p_event->app_key_idx, property_id);
    wiced_bt_mesh_model_light_lc_client_send_property_get(p_event, &get_data);
    return MESH_CLIENT_SUCCESS;
}

/*
 * Property Set
 * If operation is successful, the callback will be executed when reply from the peer is received.
 */
int mesh_client_light_lc_property_set(const char* p_name, int property_id, int value, mesh_client_light_lc_property_status_t p_property_status_callback)
{
    uint16_t dst = get_device_addr(p_name);
    wiced_bt_mesh_light_lc_property_set_data_t set_data;
    uint16_t num_nodes = 0;
    wiced_bt_mesh_db_app_key_t* app_key;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Setup");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst != 0)
    {
        if ((dst = get_light_lc_element_addr(dst)) == 0)
        {
            Log("property model not present\n");
            return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
        }
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }

    set_data.id = (uint16_t)property_id;
    set_data.len = wiced_bt_mesh_db_get_property_value_len(property_id);
    if (set_data.len == 0)
    {
        Log("Invalid property id:%d\n", property_id);
        return MESH_CLIENT_ERR_INVALID_ARGS;
    }
    set_data.value[3] = (value >> 24) & 0xFF;
    set_data.value[2] = (value >> 16) & 0xFF;
    set_data.value[1] = (value >> 8) & 0xFF;
    set_data.value[0] = value & 0xFF;

    wiced_bt_mesh_event_t* p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LC_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("LC Property Set no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    p_event->reply = WICED_TRUE;

    provision_cb.p_light_lc_property_status = p_property_status_callback;

    Log("Light LC Property Set addr:%04x app_key_idx:%04x ID:%04x", p_event->dst, p_event->app_key_idx, property_id);
    wiced_bt_mesh_model_light_lc_client_send_property_set(p_event, &set_data);
    return MESH_CLIENT_SUCCESS;
}

/*
 * Set Light Controller On/Off state of a device
 */
int mesh_client_light_lc_on_off_set(const char* p_name, uint8_t on_off, wiced_bool_t reliable, uint32_t transition_time, uint16_t delay)
{
    wiced_bt_mesh_onoff_set_data_t set_data;
    uint16_t dst = get_device_addr(p_name);
    uint16_t num_nodes = 0;
    wiced_bt_mesh_db_app_key_t* app_key;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst != 0)
    {
        if ((dst = get_light_lc_element_addr(dst)) == 0)
        {
            Log("property model not present\n");
            return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
        }
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    wiced_bt_mesh_event_t* p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("onoff get no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    p_event->reply = reliable;
    set_data.onoff = on_off;
    set_data.transition_time = transition_time;
    set_data.delay = delay;

    Log("OnOff Set addr:%04x app_key_idx:%04x reply:%d onoff:%d transition_time:%d delay:%d", p_event->dst, p_event->app_key_idx, p_event->reply, set_data.onoff, set_data.transition_time, set_data.delay);

    wiced_bt_mesh_model_onoff_client_send_set(p_event, &set_data);
    return MESH_CLIENT_SUCCESS;
}

int mesh_client_scan_unprovisioned(int start, uint8_t *p_uuid)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    int i;
    wiced_bt_mesh_event_t *p_event;
#if 0
    void mesh_client_start_extended_scan(mesh_provision_cb_t * p_cb, unprovisioned_report_t * p_report);
    unprovisioned_report_t report;
    report.provisioner_addr = 1;
    mesh_client_start_extended_scan(p_cb, &report);
    return;
#endif
    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    // When starting new scan, clean up results of the previous scan
    if (start)
    {
        while (p_cb->p_first_unprovisioned != NULL)
        {
            unprovisioned_report_t *p_temp = p_cb->p_first_unprovisioned->p_next;
            wiced_stop_timer(&p_cb->p_first_unprovisioned->scan_timer);
            wiced_bt_free_buffer(p_cb->p_first_unprovisioned);
            p_cb->p_first_unprovisioned = p_temp;
        }
        if (p_uuid != NULL)
        {
            p_cb->scan_one_uuid = WICED_TRUE;
            memcpy(p_cb->uuid, p_uuid, MESH_DEVICE_UUID_LEN);
        }
        else
        {
            p_cb->scan_one_uuid = WICED_FALSE;
        }
    }

    // Go through all the nodes that support remote provisioning server and start scan process by getting scan capabilities
    for (i = 0; i < p_mesh_db->num_nodes; i++)
    {
        if (!is_model_present(p_mesh_db->node[i].unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_REMOTE_PROVISION_SRV))
            continue;

        // If not connected can only tell local device to do the scan
        if ((p_mesh_db->node[i].unicast_address != p_cb->unicast_addr) && !mesh_client_is_proxy_connected())
            continue;

        // We will not use local device to scan if we are connected to mesh but connected to proxy over IP
        if ((p_mesh_db->node[i].unicast_address == p_cb->unicast_addr) && (mesh_client_is_proxy_connected() && (p_cb->proxy_conn_id == 0xFFFF)))
            continue;

        if (start && (p_mesh_db->node[i].scanning_state == SCANNING_STATE_IDLE))
        {
            p_mesh_db->node[i].scanning_state = SCANNING_STATE_GETTING_INFO;

            Log("ScanInfoGet addr:%04x", p_mesh_db->node[i].unicast_address);

            mesh_configure_set_local_device_key(p_mesh_db->node[i].unicast_address);

            if ((p_event = mesh_configure_create_event(p_mesh_db->node[i].unicast_address, (p_mesh_db->node[i].unicast_address != p_cb->unicast_addr))) != NULL)
                wiced_bt_mesh_provision_scan_capabilities_get(p_event);
        }
        else if (!start &&
           ((p_mesh_db->node[i].scanning_state == SCANNING_STATE_STARTING) ||
            (p_mesh_db->node[i].scanning_state == SCANNING_STATE_SCANNING)))
        {
            p_mesh_db->node[i].scanning_state = SCANNING_STATE_IDLE;

            Log("ScanStop addr:%04x", p_mesh_db->node[i].unicast_address);

            if ((p_event = mesh_configure_create_event(p_mesh_db->node[i].unicast_address, (p_mesh_db->node[i].unicast_address != p_cb->unicast_addr))) != NULL)
                wiced_bt_mesh_provision_scan_stop(p_event);
        }
    }
    return MESH_CLIENT_SUCCESS;
}

void mesh_client_start_extended_scan(mesh_provision_cb_t *p_cb, unprovisioned_report_t *p_report)
{
    wiced_bt_mesh_provision_scan_extended_start_t data;
    wiced_bt_mesh_event_t *p_event;

    Log("Extended Scan Start addr:%04x", p_report->provisioner_addr);
    if ((p_event = mesh_configure_create_event(p_report->provisioner_addr, WICED_FALSE)) == NULL)
        return;

    mesh_configure_set_local_device_key(p_event->dst);

    memset(&data, 0, sizeof(data));
    data.timeout = SCAN_EXTENDED_DURATION;
    data.num_ad_filters = 1;
    data.ad_filter_types[0] = BTM_BLE_ADVERT_TYPE_NAME_COMPLETE;
    //data.ad_filter_types[1] = BTM_BLE_ADVERT_TYPE_APPEARANCE;
    data.uuid_present = WICED_TRUE;
    memcpy(data.uuid, p_report->uuid, sizeof(p_report->uuid));

    wiced_bt_mesh_provision_scan_extended_start(p_event, &data);
}

void scan_timer_cb(TIMER_PARAM_TYPE arg)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    unprovisioned_report_t *p_report = (unprovisioned_report_t *)arg;

    mesh_client_start_extended_scan(p_cb, p_report);
}

uint8_t mesh_client_dev_key_refresh(uint8_t *uuid)
{
    uint8_t db_changed = WICED_FALSE;
    int i = 0;
    mesh_provision_cb_t *p_cb = &provision_cb;
    wiced_bt_mesh_db_node_t* p_node;

    if ((p_cb->p_first != NULL) && (p_cb->state != PROVISION_STATE_IDLE))
    {
        Log("Client provision bad operation state:%d", p_cb->state);
        return MESH_CLIENT_ERR_INVALID_STATE;
    }
    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    if ((p_node = wiced_bt_mesh_db_node_get_by_uuid(p_mesh_db, uuid)) == NULL)
    {
        Log("UUID not found\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }

    clean_pending_op_queue(0);

    p_cb->provisioner_addr = p_node->unicast_address;
    mesh_configure_set_local_device_key(p_cb->provisioner_addr);

    memcpy(p_cb->uuid, uuid, sizeof(p_cb->uuid));

    p_cb->state = PROVISION_STATE_CONNECTING;
    p_cb->addr = p_node->unicast_address;
    p_cb->provision_procedure = WICED_BT_MESH_PROVISION_PROCEDURE_DEV_KEY_REFRESH;
    p_cb->provision_completed_sent = WICED_FALSE;
    mesh_client_provision_connect(p_cb);

    provision_status_notify(p_cb, MESH_CLIENT_PROVISION_STATUS_CONNECTING);
    return MESH_CLIENT_SUCCESS;
}

uint8_t mesh_client_provision_start(const char* device_name, const char* group_name, uint8_t* uuid, uint8_t identify_duration)
{
    uint8_t db_changed = WICED_FALSE;
    int i = 0;
    mesh_provision_cb_t* p_cb = &provision_cb;
    unprovisioned_report_t* p_report, * p_best_report = NULL;

    if ((p_cb->p_first != NULL) && (p_cb->state != PROVISION_STATE_IDLE))
    {
        Log("Client provision bad operation state:%d", p_cb->state);
        return MESH_CLIENT_ERR_INVALID_STATE;
    }
    clean_pending_op_queue(0);

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    if (p_cb->p_remote_composition_data != NULL)
    {
        wiced_bt_free_buffer(p_cb->p_remote_composition_data);
        p_cb->p_remote_composition_data = NULL;
    }
    if (p_cb->provisioning_device_name != NULL)
    {
        wiced_bt_free_buffer(p_cb->provisioning_device_name);
        p_cb->provisioning_device_name = NULL;
    }
    if (strcmp(device_name, "local") == 0)
    {
        memset(p_cb->uuid, 0, 16);
        p_cb->provisioner_addr = p_cb->unicast_addr;
        p_cb->provision_completed_sent = WICED_FALSE;
        mesh_client_provision_connect(p_cb);
        return MESH_CLIENT_SUCCESS;
    }

    // Find Provisioner with the highest RSSI
    int8_t highest_rssi = -127;
    for (p_report = p_cb->p_first_unprovisioned; p_report != NULL; p_report = p_report->p_next)
    {
        if (memcmp(p_report->uuid, uuid, sizeof(p_report->uuid)) == 0)
        {
            if (p_report->rssi > highest_rssi)
            {
                p_best_report = p_report;
                highest_rssi = p_report->rssi;
            }
        }
    }
    if (p_best_report == NULL)
    {
        // for test
        // mesh_client_dev_key_refresh(uuid);
        return MESH_CLIENT_ERR_NOT_FOUND;
    }

    p_cb->provisioner_addr = p_best_report->provisioner_addr;
    p_cb->provisioning_device_name = mesh_new_string(device_name);
    p_cb->use_gatt = (p_cb->provisioner_addr == p_cb->unicast_addr);    // use PB-GATT only if provisioning from iOS/Android/Windows, use PB-ADV if provisioner is some device on the network
    memcpy(p_cb->uuid, uuid, 16);
    p_cb->provision_procedure = WICED_BT_MESH_PROVISION_PROCEDURE_PROVISION;
    p_cb->group_addr = wiced_bt_mesh_db_group_get_addr(p_mesh_db, group_name);
    p_cb->identify_duration = identify_duration;
    p_cb->provision_completed_sent = WICED_FALSE;

    p_cb->state = PROVISION_STATE_CONNECTING;
    p_cb->scan_duration = SCAN_DURATION_DEFAULT;
    p_cb->over_gatt = 0;
    p_cb->retries = 0;

    // If we are currently connected to proxy, disconnect the proxy connection,
    // so that we can provision/configure new device over GATT even if the stack supports only one connection.
    // After provisioning/configuration, new node will become a GATT proxy.
    if (p_cb->use_gatt && (p_cb->proxy_conn_id != 0) && (p_cb->provisioner_addr == p_cb->unicast_addr))
        wiced_bt_mesh_client_proxy_disconnect();
    else
    {
        mesh_client_provision_connect(p_cb);
        provision_status_notify(p_cb, MESH_CLIENT_PROVISION_STATUS_CONNECTING);
    }
    return MESH_CLIENT_SUCCESS;
}

uint8_t mesh_client_provision_with_oob(const char* device_name, const char* group_name, uint8_t* uuid, uint8_t identify_duration, uint8_t* p_oob_data, uint8_t oob_data_len)
{
    mesh_provision_cb_t* p_cb = &provision_cb;

    if ((oob_data_len == 0) || (oob_data_len > sizeof(p_cb->oob_data)))
        return MESH_CLIENT_ERR_INVALID_ARGS;

    p_cb->oob_data_len = oob_data_len;
    memcpy(p_cb->oob_data, p_oob_data, oob_data_len);
    return mesh_client_provision_start(device_name, group_name, uuid, identify_duration);
}

uint8_t mesh_client_provision(const char* device_name, const char* group_name, uint8_t* uuid, uint8_t identify_duration)
{
    mesh_provision_cb_t* p_cb = &provision_cb;

    p_cb->oob_data_len = 0;
    return mesh_client_provision_start(device_name, group_name, uuid, identify_duration);
}

uint8_t mesh_client_connect_proxy(mesh_provision_cb_t *p_cb, uint8_t connect_type, uint8_t scan_duration)
{
    wiced_bt_mesh_proxy_connect_data_t data;

    data.connect_type = connect_type;
    data.scan_duration = scan_duration;

    if (connect_type == CONNECT_TYPE_NODE_ID)
        data.node_id = p_cb->addr;

    Log("Proxy Connect type:%d", connect_type);

    wiced_bt_mesh_client_proxy_connect(&data);
    return MESH_CLIENT_SUCCESS;
}

uint8_t mesh_client_connect_network(uint8_t use_proxy, uint8_t scan_duration)
{
    mesh_provision_cb_t *p_cb = &provision_cb;

    if ((p_cb->state != PROVISION_STATE_IDLE) || !p_cb->network_opened || (p_cb->proxy_conn_id != 0))
    {
        Log("Connect Network bad operation state:%d Network opened:%d or connected:%d", p_cb->state, p_cb->network_opened, (p_cb->proxy_conn_id != 0));
        return MESH_CLIENT_ERR_INVALID_STATE;
    }
    p_cb->state = PROVISION_STATE_NETWORK_CONNECT;
    p_cb->scan_duration = scan_duration;
    if (use_proxy)
        mesh_client_connect_proxy(p_cb, CONNECT_TYPE_NET_ID, scan_duration);
    return MESH_CLIENT_SUCCESS;
}

uint8_t mesh_client_disconnect_network(void)
{
    mesh_provision_cb_t *p_cb = &provision_cb;

    p_cb->state = PROVISION_STATE_IDLE;

    if (p_cb->proxy_conn_id != 0)
    {
        wiced_bt_mesh_client_proxy_disconnect();
    }
    if (p_cb->db_changed)
    {
        p_cb->db_changed = WICED_FALSE;

        if ((provision_cb.p_database_changed != NULL) && (p_mesh_db != NULL))
            provision_cb.p_database_changed(p_mesh_db->name);
    }
    return MESH_CLIENT_SUCCESS;
}

uint8_t mesh_client_connect_component(char *component_name, uint8_t use_proxy, uint8_t scan_duration)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    wiced_bt_mesh_db_node_t *node;
    p_cb->scan_duration = scan_duration;
    wiced_bt_mesh_config_node_identity_set_data_t set;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    if (p_cb->state != PROVISION_STATE_IDLE)
    {
        Log("invalid state:%d\n", p_cb->state);
        return MESH_CLIENT_ERR_INVALID_STATE;
    }
    if (p_cb->proxy_conn_id == 0)
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    node = wiced_bt_mesh_db_node_get_by_element_name(p_mesh_db, component_name);
    if (node == NULL)
    {
        Log("Device not found:%s\n", component_name);
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    // Check if we are already connected to the correct device.
    if (p_cb->proxy_addr == node->unicast_address)
    {
        if (p_cb->p_node_connect_status != NULL)
            p_cb->p_node_connect_status(MESH_CLIENT_NODE_CONNECTED, node->name);
        return MESH_CLIENT_SUCCESS;
    }
    p_cb->state = PROVISION_STATE_CONNECTING_NODE_WAIT_NODE_IDENTITY;
    p_cb->addr = node->unicast_address;

    // connected to a Proxy, but wrong one.  Send a command to correct node
    // to start advert with node identity.
    wiced_bt_mesh_event_t *p_event = mesh_configure_create_event(p_cb->addr, (p_cb->addr != p_cb->unicast_addr));
    if (p_event == NULL)
        return MESH_CLIENT_ERR_NO_MEMORY;

    set.identity = 1;
    set.net_key_idx = node->net_key[0].index;

    mesh_configure_set_local_device_key(node->unicast_address);
    wiced_bt_mesh_config_node_identity_set(p_event, &set);
    return MESH_CLIENT_SUCCESS;
}

void mesh_client_provision_connect(mesh_provision_cb_t *p_cb)
{
    wiced_bt_mesh_provision_connect_data_t data;
    wiced_bt_mesh_event_t *p_event;
    if ((p_event = mesh_configure_create_event(p_cb->provisioner_addr, (p_cb->provisioner_addr != p_cb->unicast_addr))) == NULL)
        return;

    mesh_configure_set_local_device_key(p_cb->provisioner_addr);

    memset(&data, 0, sizeof(wiced_bt_mesh_provision_connect_data_t));
    data.procedure = p_cb->provision_procedure;

    if (data.procedure == WICED_BT_MESH_PROVISION_PROCEDURE_PROVISION)
    {
        memcpy(data.uuid, p_cb->uuid, 16);
        data.identify_duration = p_cb->identify_duration;
    }
    Log("Provision Connect Provisioner:%x identify_duration:%x use_gatt:%x", p_cb->provisioner_addr, data.identify_duration, p_cb->use_gatt);

    wiced_bt_mesh_provision_connect(p_event, &data, p_cb->use_gatt);
}

mesh_provision_cb_t *mesh_provision_find_by_addr(uint16_t addr)
{
    return (provision_cb.addr == addr) ? &provision_cb : NULL;
}

void mesh_sensor_process_event(uint16_t addr, uint16_t event, void *p_data)
{
#ifndef CLIENTCONTROL
    ods("mesh_sensor_process_event event:%d\n", event);
#else
    Log("mesh_sensor_process_event event:%d\n", event);
#endif

    // client control can receive events before db is opened. ignore.
    if (p_mesh_db == NULL)
        return;

    switch (event)
    {
        case WICED_BT_MESH_SENSOR_DESCRIPTOR_STATUS:
            mesh_process_sensor_descriptor_status(addr, (wiced_bt_mesh_sensor_descriptor_status_data_t *)p_data);
            return;

        case WICED_BT_MESH_SENSOR_SETTING_STATUS:
            mesh_process_sensor_setting_status(addr, (wiced_bt_mesh_sensor_setting_status_data_t *)p_data);
            return;

        case WICED_BT_MESH_SENSOR_STATUS:
            mesh_process_sensor_status(addr, (wiced_bt_mesh_sensor_status_data_t *)p_data);
            return;

        case WICED_BT_MESH_SENSOR_SETTINGS_STATUS:
            mesh_process_sensor_settings_status(addr, (wiced_bt_mesh_sensor_settings_status_data_t *)p_data);
            return;

        case WICED_BT_MESH_SENSOR_CADENCE_STATUS:
            mesh_process_sensor_cadence_status(addr, (wiced_bt_mesh_sensor_cadence_status_data_t *)p_data);
            return;
    }
}

void mesh_vendor_specific_data(uint16_t src, uint16_t company_id, uint16_t model_id, uint8_t opcode, uint8_t ttl, void* p_data, uint16_t data_len)
{
    mesh_provision_cb_t *p_cb = &provision_cb;

    if (p_cb->p_vendor_specific_data == NULL)
        return;

    const char *p_name = get_component_name(src);
    p_cb->p_vendor_specific_data(p_name, company_id, model_id, opcode, ttl, p_data, data_len);
}

void mesh_provision_process_event(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    wiced_bt_mesh_connect_status_data_t *p_link_status;

    // client control can receive events before db is opened. ignore.
    if (p_mesh_db == NULL)
        return;

    if ((event != WICED_BT_MESH_SEQ_CHANGED) && (event != WICED_BT_MESH_COMMAND_STATUS))
//#ifndef CLIENTCONTROL
        ods("mesh_provision_process_event state:%d event:%d\n", p_cb->state, event);
//#else
//        Log("mesh_provision_process_event state:%d event:%d\n", p_cb->state, event);
//#endif

    // status information from devices can be processed in any state
    switch (event)
    {
    case WICED_BT_MESH_PROXY_CONNECTION_STATUS:
        p_link_status = (wiced_bt_mesh_connect_status_data_t *)p_data;

        p_cb->proxy_conn_id = p_link_status->connected ? 1 : 0;
        p_cb->over_gatt = p_link_status->connected ? 1 : 0;
        if (!p_link_status->connected)
            p_cb->proxy_addr = 0;

        wiced_bt_mesh_config_client_connection_state_change(p_link_status->connected);
        break;

    case WICED_BT_MESH_TX_COMPLETE:
        if ((p_event->status.tx_flag == TX_STATUS_COMPLETED) || (p_event->status.tx_flag == TX_STATUS_ACK_RECEIVED) ||
            ((p_cb->state != PROVISION_STATE_CONNECTING_NODE_WAIT_NODE_IDENTITY) &&
             (p_cb->state != PROVISION_STATE_GET_REMOTE_COMPOSITION_DATA) &&
             (p_cb->state != PROVISION_STATE_CONFIGURATION) &&
             (p_cb->state != PROVISION_STATE_RECONFIGURATION) &&
             (p_cb->state != PROVISION_STATE_KEY_REFRESH_1) &&
             (p_cb->state != PROVISION_STATE_KEY_REFRESH_2) &&
             (p_cb->state != PROVISION_STATE_KEY_REFRESH_3)))
        {
            mesh_process_tx_complete(p_cb, p_event);
            return;
        }
        break;

    case WICED_BT_MESH_PROXY_FILTER_STATUS:
        p_cb->proxy_addr = p_event->src;
        break;

    case WICED_BT_MESH_COMMAND_STATUS:
        return;

    case WICED_BT_MESH_SEQ_CHANGED:
        mesh_process_seq_changed((wiced_bt_mesh_core_state_seq_t *)p_data);
        return;

    case WICED_BT_MESH_IV_CHANGED:
        mesh_process_iv_changed((wiced_bt_mesh_core_state_iv_t *)p_data);
        return;

    case WICED_BT_MESH_PROVISION_SCAN_CAPABILITIES_STATUS:
        mesh_process_scan_capabilities_status(p_cb, p_event, (wiced_bt_mesh_provision_scan_capabilities_status_data_t *)p_data);
        return;

    case WICED_BT_MESH_PROVISION_SCAN_STATUS:
        mesh_process_scan_status(p_cb, p_event, (wiced_bt_mesh_provision_scan_status_data_t *)p_data);
        return;

    case WICED_BT_MESH_PROVISION_SCAN_REPORT:
        mesh_process_scan_report(p_cb, p_event, (wiced_bt_mesh_provision_scan_report_data_t *)p_data);
        return;

    case WICED_BT_MESH_PROVISION_SCAN_EXTENDED_REPORT:
        mesh_process_scan_extended_report(p_cb, p_event, (wiced_bt_mesh_provision_scan_extended_report_data_t *)p_data);
        return;

#ifdef MESH_DFU_ENABLED
    case WICED_BT_MESH_FW_DISTRIBUTION_STATUS:
        mesh_process_fw_distribution_status(p_event, p_data);
        return;
#endif

    case WICED_BT_MESH_HEALTH_ATTENTION_STATUS:
        mesh_process_health_attention_status(p_event, p_data);
        return;

    case WICED_BT_MESH_USER_PROPERTIES_STATUS:
    case WICED_BT_MESH_CLIENT_PROPERTIES_STATUS:
    case WICED_BT_MESH_MANUF_PROPERTIES_STATUS:
    case WICED_BT_MESH_ADMIN_PROPERTIES_STATUS:
        mesh_process_properties_status(p_event, p_data);
        return;

    case WICED_BT_MESH_USER_PROPERTY_STATUS:
    case WICED_BT_MESH_MANUF_PROPERTY_STATUS:
    case WICED_BT_MESH_ADMIN_PROPERTY_STATUS:
        mesh_process_property_status(p_event, p_data);
        return;

    case WICED_BT_MESH_LIGHT_LC_MODE_STATUS:
        mesh_process_light_lc_mode_status(p_event, p_data);
        return;

    case WICED_BT_MESH_LIGHT_LC_OCCUPANCY_MODE_STATUS:
        mesh_process_light_lc_occupancy_mode_status(p_event, p_data);
        return;

    case WICED_BT_MESH_LIGHT_LC_PROPERTY_STATUS:
        mesh_process_light_lc_property_status(p_event, p_data);
        return;

    case WICED_BT_MESH_ONOFF_STATUS:
        mesh_process_on_off_status(p_event, p_data);
        return;

    case WICED_BT_MESH_LEVEL_STATUS:
        mesh_process_level_status(p_event, p_data);
        return;

    case WICED_BT_MESH_LIGHT_LIGHTNESS_STATUS:
//    case WICED_BT_MESH_LIGHT_LIGHTNESS_LINEAR_STATUS:
        mesh_process_lightness_status(p_event, p_data);
        return;

    case WICED_BT_MESH_LIGHT_HSL_STATUS:
        mesh_process_hsl_status(p_event, p_data);
        return;

    case WICED_BT_MESH_LIGHT_CTL_STATUS:
        mesh_process_ctl_status(p_event, p_data);
        return;

    case WICED_BT_MESH_DEFAULT_TRANSITION_TIME_STATUS:
        mesh_default_trans_time_status(p_event, p_data);
        return;

    case WICED_BT_MESH_VENDOR_DATA:
        mesh_vendor_specific_data(p_event->src, p_event->company_id, p_event->model_id, (uint8_t)p_event->opcode, (uint8_t)p_event->ttl, p_data, p_event->data_len);
        wiced_bt_mesh_release_event(p_event);
        return;
    }

    switch (p_cb->state)
    {
    case PROVISION_STATE_IDLE:
        mesh_provision_state_idle(p_cb, event, p_event, p_data);
        break;

    case PROVISION_STATE_CONNECTING:
        mesh_provision_state_connecting(p_cb, event, p_event, p_data);
        break;

    case PROVISION_STATE_PROVISIONING:
        mesh_provision_state_provisioning(p_cb, event, p_event, p_data);
        break;

    case PROVISION_STATE_PROVISION_DISCONNECTING:
        mesh_provision_state_provision_disconnecting(p_cb, event, p_event, p_data);
        break;

    case PROVISION_STATE_NODE_CONNECTING:
        mesh_configure_state_node_connecting(p_cb, event, p_event, p_data);
        break;

    case PROVISION_STATE_GET_REMOTE_COMPOSITION_DATA:
        mesh_configure_state_getting_remote_composition_data(p_cb, event, p_event, p_data);
        break;

    case PROVISION_STATE_CONFIGURATION:
        mesh_configure_state_configuration(p_cb, event, p_event, p_data);
        break;

    case PROVISION_STATE_CONFIGURE_DISCONNECTING:
        mesh_configure_state_disconnecting(p_cb, event, p_event, p_data);
        break;

    case PROVISION_STATE_NETWORK_CONNECT:
        mesh_configure_state_network_connect(p_cb, event, p_event, p_data);
        break;

    case PROVISION_STATE_CONNECTING_NODE_WAIT_NODE_IDENTITY:
        mesh_client_state_connecting_node_wait_node_identity(p_cb, event, p_event, p_data);
        break;

    case PROVISION_STATE_CONNECTING_NODE_WAIT_DISCONNECT:
        mesh_client_state_connecting_node_wait_disconnect(p_cb, event, p_event, p_data);
        break;

    case PROVISION_STATE_CONNECTING_NODE_WAIT_CONNECT:
        mesh_client_state_connecting_node_wait_connect(p_cb, event, p_event, p_data);
        break;

    case PROVISION_STATE_RECONFIGURATION:
        mesh_configure_state_configuration(p_cb, event, p_event, p_data);
        break;

    case PROVISION_STATE_KEY_REFRESH_1:
        mesh_configure_state_key_refresh1(p_cb, event, p_event, p_data);
        break;

    case PROVISION_STATE_KEY_REFRESH_2:
        mesh_configure_state_key_refresh2(p_cb, event, p_event, p_data);
        break;

    case PROVISION_STATE_KEY_REFRESH_3:
        mesh_configure_state_key_refresh3(p_cb, event, p_event, p_data);
        break;
    }

    // If event is TX complete, it stays in the queue.
    if ((p_event != NULL) && (event != WICED_BT_MESH_TX_COMPLETE))
        wiced_bt_mesh_release_event(p_event);
}

void mesh_process_sensor_cadence_status(uint16_t addr, wiced_bt_mesh_sensor_cadence_status_data_t *ptr)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    Log("Sensor Cadence Status from:%x", addr);
    if (p_cb->store_config)
    {
        wiced_bt_mesh_db_sensor_cadence_add(p_mesh_db, addr, ptr);
        wiced_bt_mesh_db_store(p_mesh_db);
        p_cb->db_changed = WICED_TRUE;
    }
    if ((p_cb->p_first != NULL) && p_cb->p_first->operation == CONFIG_OPERATION_SENSOR_CADENCE_GET)
        start_next_op(p_cb);
}

void mesh_process_sensor_status(uint16_t addr, wiced_bt_mesh_sensor_status_data_t *ptr)
{
    wiced_bt_mesh_sensor_status_data_t *p_data = (wiced_bt_mesh_sensor_status_data_t *)ptr;

    Log("Sensor Status from:%x", addr);

    if (provision_cb.p_sensor_status != NULL)
    {
        provision_cb.p_sensor_status(wiced_bt_mesh_db_get_element_name(p_mesh_db, addr), p_data->property_id, p_data->prop_value_len, p_data->raw_value);
    }
}

void mesh_process_sensor_settings_status(uint16_t addr, wiced_bt_mesh_sensor_settings_status_data_t *ptr)
{
    mesh_provision_cb_t *p_cb = &provision_cb;

    Log("Sensor settings Status from:%x Property ID:%x", addr, ptr->property_id);
    if (p_cb->store_config)
    {
        wiced_bt_mesh_db_sensor_settings_add(p_mesh_db, addr, ptr);
        wiced_bt_mesh_db_store(p_mesh_db);
    }
    start_next_op(p_cb);
}

void mesh_process_sensor_setting_status(uint16_t addr, wiced_bt_mesh_sensor_setting_status_data_t *ptr)
{
    mesh_provision_cb_t *p_cb = &provision_cb;

    Log("Sensor setting Status from:%x Property ID:%x", addr, ptr->property_id);
    if (p_cb->store_config)
    {
        wiced_bt_mesh_db_sensor_setting_add(p_mesh_db, addr, ptr);
        wiced_bt_mesh_db_store(p_mesh_db);
        p_cb->db_changed = WICED_TRUE;
    }
}

void mesh_provision_state_idle(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    switch (event)
    {
    case WICED_BT_MESH_PROXY_CONNECTION_STATUS:
        if (p_cb->p_connect_status != NULL)
            p_cb->p_connect_status(((wiced_bt_mesh_connect_status_data_t *)p_data)->connected, ((wiced_bt_mesh_connect_status_data_t *)p_data)->provisioner_addr, 0, ((wiced_bt_mesh_connect_status_data_t *)p_data)->over_gatt);
        break;

    case WICED_BT_MESH_CONFIG_COMPOSITION_DATA_STATUS:
        mesh_configure_composition_data_status(p_cb, p_event, (wiced_bt_mesh_config_composition_data_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_NETKEY_STATUS:
        mesh_configure_net_key_status(p_cb, p_event, (wiced_bt_mesh_config_netkey_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_KEY_REFRESH_PHASE_STATUS:
        mesh_configure_phase_status(p_cb, p_event, (wiced_bt_mesh_config_key_refresh_phase_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_APPKEY_STATUS:
        mesh_configure_app_key_status(p_cb, p_event, (wiced_bt_mesh_config_appkey_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_MODEL_APP_BIND_STATUS:
        mesh_configure_model_app_bind_status(p_cb, p_event, (wiced_bt_mesh_config_model_app_bind_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_MODEL_SUBSCRIPTION_STATUS:
        mesh_configure_model_sub_status(p_cb, p_event, (wiced_bt_mesh_config_model_subscription_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_NETWORK_TRANSMIT_STATUS:
        mesh_configure_net_transmit_status(p_cb, p_event, (wiced_bt_mesh_config_network_transmit_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_DEFAULT_TTL_STATUS:
        mesh_configure_default_ttl_status(p_cb, p_event, (wiced_bt_mesh_config_default_ttl_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_RELAY_STATUS:
        mesh_configure_relay_status(p_cb, p_event, (wiced_bt_mesh_config_relay_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_FRIEND_STATUS:
        mesh_configure_friend_status(p_cb, p_event, (wiced_bt_mesh_config_friend_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_GATT_PROXY_STATUS:
        mesh_configure_gatt_proxy_status(p_cb, p_event, (wiced_bt_mesh_config_gatt_proxy_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_BEACON_STATUS:
        mesh_configure_beacon_status(p_cb, p_event, (wiced_bt_mesh_config_beacon_status_data_t *)p_data);
        break;

    default:
        Log("Event:%d ignored\n", event);
        break;
    }
}

void mesh_provision_state_connecting(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    switch (event)
    {
    case WICED_BT_MESH_PROXY_CONNECTION_STATUS:
        Log("connecting proxy status changed\n");
        mesh_client_provision_connect(p_cb);

        provision_status_notify(p_cb, MESH_CLIENT_PROVISION_STATUS_CONNECTING);
        break;

    case WICED_BT_MESH_PROVISION_LINK_REPORT:
        mesh_provision_connecting_link_status(p_cb, p_event, (wiced_bt_mesh_provision_link_report_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROVISION_END:
        Log("Provision: end from Server:%x addr:%04x result:%x",
            ((wiced_bt_mesh_provision_status_data_t *)p_data)->provisioner_addr,
            ((wiced_bt_mesh_provision_status_data_t *)p_data)->addr,
            ((wiced_bt_mesh_provision_status_data_t *)p_data)->result);
        mesh_provision_process_provision_end(p_cb, (wiced_bt_mesh_provision_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROVISION_DEVICE_CAPABILITIES:
        mesh_provision_process_device_caps(p_cb, p_event, (wiced_bt_mesh_provision_device_capabilities_data_t *)p_data);
        break;

    default:
        Log("Event:%d ignored\n", event);
        break;
    }
}

void mesh_provision_state_provisioning(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    char buf[160];
    int i;

    switch (event)
    {
    case WICED_BT_MESH_PROVISION_LINK_REPORT:
        mesh_provision_provisioning_link_status(p_cb, p_event, (wiced_bt_mesh_provision_link_report_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROVISION_END:
        sprintf(buf, "Provision: end from Server:%x addr:%04x result:%x dev_key:",
            ((wiced_bt_mesh_provision_status_data_t *)p_data)->provisioner_addr,
            ((wiced_bt_mesh_provision_status_data_t *)p_data)->addr,
            ((wiced_bt_mesh_provision_status_data_t *)p_data)->result);
        for (i = 0; i < 16; i++)
        {
            sprintf(&buf[strlen(buf)], "%02x ", ((wiced_bt_mesh_provision_status_data_t *)p_data)->dev_key[i]);
        }
        Log(buf);
        mesh_provision_process_provision_end(p_cb, (wiced_bt_mesh_provision_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROVISION_GET_OOB_DATA:
        Log("Provision: get OOB from Server:%x type:%d size:%d action:%d",
            ((wiced_bt_mesh_provision_device_oob_request_data_t *)p_data)->provisioner_addr,
            ((wiced_bt_mesh_provision_device_oob_request_data_t *)p_data)->type,
            ((wiced_bt_mesh_provision_device_oob_request_data_t *)p_data)->size,
            ((wiced_bt_mesh_provision_device_oob_request_data_t *)p_data)->action);
        mesh_provision_process_get_oob_data(p_cb, p_event, (wiced_bt_mesh_provision_device_oob_request_data_t*)p_data);
 //       provision_status_notify(p_cb, MESH_CLiENT_PROVISION_STATUS_GET_OOB);
        break;

    default:
        Log("Event:%d ignored\n", event);
        break;
    }
}

void mesh_provision_state_provision_disconnecting(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    switch (event)
    {
    case WICED_BT_MESH_PROVISION_LINK_REPORT:
        mesh_provision_disconnecting_link_status(p_cb, p_event, (wiced_bt_mesh_provision_link_report_data_t *)p_data);
        break;

    default:
        Log("Event:%d ignored\n", event);
        break;
    }
}

void mesh_configure_state_node_connecting(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    switch (event)
    {
    case WICED_BT_MESH_PROXY_CONNECTION_STATUS:
        mesh_node_connecting_link_status(p_cb, p_event, (wiced_bt_mesh_connect_status_data_t *)p_data);
        break;

    default:
        Log("Event:%d ignored\n", event);
        break;
    }
}

void mesh_configure_state_getting_remote_composition_data(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    switch (event)
    {
    case WICED_BT_MESH_TX_COMPLETE:
        Log("Node unreachable:%04x", p_event->dst);
        if ((p_cb->proxy_conn_id != 0) && p_cb->over_gatt && (p_cb->proxy_addr == p_event->dst))
        {
            wiced_bt_mesh_client_proxy_disconnect();
        }
        else
        {
            Log("Provisioning failed\n");
            p_cb->state = PROVISION_STATE_IDLE;
            clean_pending_op_queue(0);
            provision_status_notify(p_cb, MESH_CLIENT_PROVISION_STATUS_FAILED);
        }
        break;

    case WICED_BT_MESH_PROXY_CONNECTION_STATUS:
        p_cb->state = PROVISION_STATE_NODE_CONNECTING;
        mesh_node_connecting_link_status(p_cb, p_event, (wiced_bt_mesh_connect_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_COMPOSITION_DATA_STATUS:
        mesh_configure_composition_data_status(p_cb, p_event, (wiced_bt_mesh_config_composition_data_status_data_t *)p_data);
        break;

    default:
        Log("Event:%d ignored\n", event);
        break;
    }
}

void mesh_configure_state_configuration(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    switch (event)
    {
    case WICED_BT_MESH_TX_COMPLETE:
        Log("Node unreachable:%04x", p_event->dst);
        if (p_cb->state == PROVISION_STATE_CONFIGURATION)
        {
            if ((p_cb->proxy_conn_id != 0) && p_cb->over_gatt && (p_cb->proxy_addr == p_event->dst))
            {
                wiced_bt_mesh_client_proxy_disconnect();
            }
            else
            {
                Log("Provisioning failed\n");
                p_cb->state = PROVISION_STATE_IDLE;
                clean_pending_op_queue(0);
                provision_status_notify(p_cb, MESH_CLIENT_PROVISION_STATUS_FAILED);
            }
        }
        else // reconfiguration
        {
            if (p_cb->db_changed)
            {
                p_cb->db_changed = WICED_FALSE;

                if (provision_cb.p_database_changed != NULL)
                    provision_cb.p_database_changed(p_mesh_db->name);
            }
            clean_pending_op_queue(p_event->dst);
            if (p_cb->p_first != NULL)
                configure_execute_pending_operation(p_cb);
            else
                p_cb->state = PROVISION_STATE_IDLE;

            // ToDo, need to send complete to the app
        }
        return;

    case WICED_BT_MESH_PROXY_CONNECTION_STATUS:
        mesh_configure_proxy_connection_status(p_cb, p_event, (wiced_bt_mesh_connect_status_data_t *)p_data);
        return;

    case WICED_BT_MESH_CONFIG_NETKEY_STATUS:
        mesh_configure_net_key_status(p_cb, p_event, (wiced_bt_mesh_config_netkey_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_APPKEY_STATUS:
        mesh_configure_app_key_status(p_cb, p_event, (wiced_bt_mesh_config_appkey_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_MODEL_APP_BIND_STATUS:
        mesh_configure_model_app_bind_status(p_cb, p_event, (wiced_bt_mesh_config_model_app_bind_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_MODEL_SUBSCRIPTION_STATUS:
        mesh_configure_model_sub_status(p_cb, p_event, (wiced_bt_mesh_config_model_subscription_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_MODEL_PUBLICATION_STATUS:
        mesh_configure_model_pub_status(p_cb, p_event, (wiced_bt_mesh_config_model_publication_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_NETWORK_TRANSMIT_STATUS:
        mesh_configure_net_transmit_status(p_cb, p_event, (wiced_bt_mesh_config_network_transmit_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_DEFAULT_TTL_STATUS:
        mesh_configure_default_ttl_status(p_cb, p_event, (wiced_bt_mesh_config_default_ttl_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_RELAY_STATUS:
        mesh_configure_relay_status(p_cb, p_event, (wiced_bt_mesh_config_relay_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_FRIEND_STATUS:
        mesh_configure_friend_status(p_cb, p_event, (wiced_bt_mesh_config_friend_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_GATT_PROXY_STATUS:
        mesh_configure_gatt_proxy_status(p_cb, p_event, (wiced_bt_mesh_config_gatt_proxy_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_BEACON_STATUS:
        mesh_configure_beacon_status(p_cb, p_event, (wiced_bt_mesh_config_beacon_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROXY_FILTER_STATUS:
        mesh_configure_filter_status(p_cb, p_event, (wiced_bt_mesh_proxy_filter_status_data_t *)p_data);
        break;

    default:
        Log("Configuration Event:%d ignored\n", event);
        return;
    }
    // If we are here, we received some configuration reply from the device. Reset
    // the retry counter. We will keep trying until we can hear something from the device.
    if ((p_cb->state == PROVISION_STATE_CONFIGURATION) && (p_cb->proxy_conn_id != 0) && p_cb->over_gatt)
        p_cb->retries = 0;
}

void mesh_configure_state_disconnecting(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    switch (event)
    {
    case WICED_BT_MESH_PROXY_CONNECTION_STATUS:
        Log("Proxy Connection provisioner:%04x addr:%04x connected:%x over_gatt:%d",
            ((wiced_bt_mesh_connect_status_data_t *)p_data)->provisioner_addr,
            ((wiced_bt_mesh_connect_status_data_t *)p_data)->addr,
            ((wiced_bt_mesh_connect_status_data_t *)p_data)->connected,
            ((wiced_bt_mesh_connect_status_data_t *)p_data)->over_gatt);
        mesh_configure_disconnecting_link_status(p_cb, p_event, (wiced_bt_mesh_connect_status_data_t *)p_data);
        break;

    default:
        Log("Event:%d ignored\n", event);
        break;
    }
}

void mesh_configure_state_network_connect(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    wiced_bt_mesh_connect_status_data_t *p_link_status;
    int net_key_idx;
    wiced_bt_mesh_db_node_t *node;
    pending_operation_t *p_op;

    switch (event)
    {
    case WICED_BT_MESH_PROXY_CONNECTION_STATUS:
        p_link_status = (wiced_bt_mesh_connect_status_data_t *)p_data;

        Log("Net Connect Proxy Connection status provisioner:%x connected:%x over_gatt:%d", p_link_status->provisioner_addr, p_link_status->connected, p_link_status->over_gatt);

        if (p_link_status->connected)
        {
            add_filter(p_cb, 0xFFFF);
        }
        else
        {
            if (p_cb->p_connect_status != NULL)
                p_cb->p_connect_status(p_link_status->connected, p_link_status->provisioner_addr, 0, p_link_status->over_gatt);

            p_cb->state = PROVISION_STATE_IDLE;

            if (p_cb->db_changed)
            {
                p_cb->db_changed = WICED_FALSE;

                if (provision_cb.p_database_changed != NULL)
                    provision_cb.p_database_changed(p_mesh_db->name);
            }
        }
        break;

    case WICED_BT_MESH_PROXY_FILTER_STATUS:
        Log("Net Connect Filter Status from:%x type:%d list size:%d\n", p_event->src, ((wiced_bt_mesh_proxy_filter_status_data_t *)p_data)->type, ((wiced_bt_mesh_proxy_filter_status_data_t *)p_data)->list_size);
        p_cb->state = PROVISION_STATE_IDLE;

        node = wiced_bt_mesh_db_node_get_by_element_addr(p_mesh_db, p_event->src);
        if ((node != NULL) && node->blocked)
        {
            if (p_cb->p_connect_status != NULL)
                p_cb->p_connect_status(0, p_cb->proxy_conn_id, 0, 1);

            // start sending the Node Reset to the device.
            if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) == NULL)
                return;

            p_op->operation = CONFIG_OPERATION_NODE_RESET;
            p_op->p_event = mesh_configure_create_event(node->unicast_address, (node->unicast_address != p_cb->unicast_addr));
            configure_pending_operation_queue(p_cb, p_op);
        }
        else
        {
#ifndef CLIENTCONTROL
            ods("Net Connected\n");
#endif
            if (p_cb->p_connect_status != NULL)
                p_cb->p_connect_status(1, p_cb->proxy_conn_id, 0, 1);
        }
        // check if we need to continue key refresh
        for (net_key_idx = 0; net_key_idx < p_mesh_db->num_net_keys; net_key_idx++)
        {
            if (p_mesh_db->net_key[net_key_idx].phase != WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL)
                mesh_key_refresh_continue(p_cb, &p_mesh_db->net_key[net_key_idx]);
        }
        if (p_cb->p_first != NULL)
            configure_execute_pending_operation(p_cb);
        break;

    default:
        Log("Event:%d ignored\n", event);
        break;
    }
}

void mesh_client_state_connecting_node_wait_node_identity(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    wiced_bt_mesh_config_node_identity_status_data_t *p_status;
    wiced_bt_mesh_connect_status_data_t *p_link_status;
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_addr(p_mesh_db, p_cb->addr);

    switch (event)
    {
    case WICED_BT_MESH_TX_COMPLETE:
        Log("Node unreachable:%04x", p_event->dst);
        if ((node != NULL) && (node->unicast_address == p_event->dst) && (p_cb->p_node_connect_status != NULL))
            p_cb->p_node_connect_status(MESH_CLIENT_NODE_WARNING_UNREACHABLE, node->name);
        p_cb->state = PROVISION_STATE_IDLE;
        break;

    case WICED_BT_MESH_PROXY_CONNECTION_STATUS:
        p_link_status = (wiced_bt_mesh_connect_status_data_t *)p_data;

        Log("Connecting node proxy connection conn_id:%x connected:%x over_gatt:%d", p_link_status->addr, p_link_status->connected, p_link_status->over_gatt);

        if (p_link_status->connected)
        {
            add_filter(p_cb, p_cb->unicast_addr);
        }
        else
        {
            mesh_client_connect_proxy(p_cb, CONNECT_TYPE_NODE_ID, p_cb->scan_duration);
        }
        p_cb->state = PROVISION_STATE_IDLE;
        break;

    case WICED_BT_MESH_CONFIG_NODE_IDENTITY_STATUS:
        Log("Node Identity Status from:%04x", p_event->src);
        p_status = (wiced_bt_mesh_config_node_identity_status_data_t *)p_data;
        if (p_status->status != 0)
        {
            if ((node != NULL) && (p_cb->p_node_connect_status != NULL))
                p_cb->p_node_connect_status(MESH_CLIENT_NODE_WARNING_UNREACHABLE, node->name);
            p_cb->state = PROVISION_STATE_IDLE;
        }
        else
        {
            p_cb->state = PROVISION_STATE_CONNECTING_NODE_WAIT_DISCONNECT;
            wiced_bt_mesh_client_proxy_disconnect();
        }
        break;

    default:
        Log("Event:%d ignored\n", event);
        break;
    }
}

void mesh_client_state_connecting_node_wait_disconnect(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    if (event == WICED_BT_MESH_PROXY_CONNECTION_STATUS)
    {
        wiced_bt_mesh_connect_status_data_t *p_link_status = (wiced_bt_mesh_connect_status_data_t *)p_data;

        if (!p_link_status->connected)
        {
            p_cb->state = PROVISION_STATE_CONNECTING_NODE_WAIT_CONNECT;
            mesh_client_connect_proxy(p_cb, CONNECT_TYPE_NODE_ID, p_cb->scan_duration);
        }
    }
    else
    {
        Log("Event:%d ignored\n", event);
    }
}

void mesh_client_state_connecting_node_wait_connect(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_addr(p_mesh_db, p_cb->addr);
    wiced_bt_mesh_connect_status_data_t *p_link_status;

    switch (event)
    {
    case WICED_BT_MESH_PROXY_CONNECTION_STATUS:
        p_link_status = (wiced_bt_mesh_connect_status_data_t *)p_data;

        if (p_link_status->connected)
        {
            if (p_cb->proxy_addr == 0)
            {
                add_filter(p_cb, 0xFFFF);
            }
            else
            {
                p_cb->state = PROVISION_STATE_IDLE;

                if ((node != NULL) && (p_cb->p_node_connect_status != NULL))
                    p_cb->p_node_connect_status(MESH_CLIENT_NODE_CONNECTED, node->name);
            }
        }
        else
        {
            p_cb->state = PROVISION_STATE_IDLE;
            if ((node != NULL) && (p_cb->p_node_connect_status != NULL))
                p_cb->p_node_connect_status(MESH_CLIENT_NODE_WARNING_UNREACHABLE, node->name);

            if (p_cb->p_connect_status != NULL)
                p_cb->p_connect_status(0, p_cb->proxy_conn_id, 0, p_cb->over_gatt);
        }
        break;

    case WICED_BT_MESH_PROXY_FILTER_STATUS:
        Log("Node connect Filter Status from:%x type:%d list size:%d\n", p_event->src, ((wiced_bt_mesh_proxy_filter_status_data_t *)p_data)->type, ((wiced_bt_mesh_proxy_filter_status_data_t *)p_data)->list_size);
        p_cb->state = PROVISION_STATE_IDLE;

        if ((node != NULL) && (p_cb->p_node_connect_status != NULL))
            p_cb->p_node_connect_status(MESH_CLIENT_NODE_CONNECTED, node->name);
        break;

    default:
        Log("Event:%d ignored\n", event);
        break;
    }
}

/*
 * State machine sent net key or app key update and waiting for all scheduled operations to complete
 */
void mesh_configure_state_key_refresh1(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    int net_key_idx;

    switch (event)
    {
    case WICED_BT_MESH_TX_COMPLETE:
        Log("KR1 Node unreachable:%04x p_event:%x op_event:%x", p_event->dst, p_event, p_cb->p_first != NULL ? p_cb->p_first->p_event : 0);
        if ((p_cb->p_first == NULL) || (p_cb->p_first->p_event != p_event))
            break;

        clean_pending_op_queue(p_event->dst);
        if (p_cb->p_first != NULL)
        {
            configure_execute_pending_operation(p_cb);
        }
        else
        {
            //All operations are completed but at-least 1 node is unreachable. The phase1 completed function will send report to the client.
            p_cb->state = PROVISION_STATE_IDLE;
            for (net_key_idx = 0; net_key_idx < p_mesh_db->num_net_keys; net_key_idx++)
                mesh_key_refresh_phase1_completed(p_cb, &p_mesh_db->net_key[net_key_idx]);
        }
        break;

    case WICED_BT_MESH_CONFIG_NODE_RESET_STATUS:
        if ((p_cb->p_first->operation != CONFIG_OPERATION_NODE_RESET) || (p_cb->p_first->p_event->dst != p_event->src))
        {
            Log("Ignored KR1 Node Reset Status from:%04x", p_event->src);
            break;
        }
        Log("Node Reset Status from:%04x", p_event->src);
        start_next_op(p_cb);
        if (p_cb->p_first == NULL)
        {
            for (net_key_idx = 0; net_key_idx < p_mesh_db->num_net_keys; net_key_idx++)
                mesh_key_refresh_phase1_completed(p_cb, &p_mesh_db->net_key[net_key_idx]);
        }
        break;

    case WICED_BT_MESH_PROXY_CONNECTION_STATUS:
        mesh_key_refresh_link_status(p_cb, p_event, (wiced_bt_mesh_connect_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROXY_FILTER_STATUS:
        if (p_cb->p_first->operation == CONFIG_OPERATION_FILTER_ADD)
            start_next_op(p_cb);
        configure_execute_pending_operation(p_cb);
        break;

    case WICED_BT_MESH_CONFIG_APPKEY_STATUS:
        mesh_key_refresh_app_key_status(p_cb, p_event, (wiced_bt_mesh_config_appkey_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_NETKEY_STATUS:
        mesh_key_refresh_net_key_status(p_cb, p_event, (wiced_bt_mesh_config_netkey_status_data_t *)p_data);
        break;

    default:
        Log("Event:%d ignored\n", event);
        break;
    }
}

/*
 * State machine sent at least one command to transition to Key Refresh Phase 2 state and waiting for all scheduled operations to complete
 */
void mesh_configure_state_key_refresh2(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    int net_key_idx;

    switch (event)
    {
    case WICED_BT_MESH_TX_COMPLETE:
        Log("KR2 Node unreachable:%04x p_event:%x op_event:%x", p_event->dst, p_event, p_cb->p_first != NULL ? p_cb->p_first->p_event : 0);
        if ((p_cb->p_first == NULL) || (p_cb->p_first->p_event != p_event))
            break;

        start_next_op(p_cb);

        if (p_cb->p_first == NULL)
            for (net_key_idx = 0; net_key_idx < p_mesh_db->num_net_keys; net_key_idx++)
                mesh_key_refresh_phase2_completed(p_cb, &p_mesh_db->net_key[net_key_idx]);
        break;

    case WICED_BT_MESH_CONFIG_NODE_RESET_STATUS:
        if ((p_cb->p_first->operation != CONFIG_OPERATION_NODE_RESET) || (p_cb->p_first->p_event->dst != p_event->src))
        {
            Log("Ignored KR2 Node Reset Status from:%04x", p_event->src);
            break;
        }
        Log("KR2 Node Reset Status from:%04x", p_event->src);

        start_next_op(p_cb);

        if (p_cb->p_first == NULL)
        {
            for (net_key_idx = 0; net_key_idx < p_mesh_db->num_net_keys; net_key_idx++)
                mesh_key_refresh_phase2_completed(p_cb, &p_mesh_db->net_key[net_key_idx]);
        }
        break;

    case WICED_BT_MESH_PROXY_CONNECTION_STATUS:
        mesh_key_refresh_link_status(p_cb, p_event, (wiced_bt_mesh_connect_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROXY_FILTER_STATUS:
        if (p_cb->p_first->operation == CONFIG_OPERATION_FILTER_ADD)
            start_next_op(p_cb);
        configure_execute_pending_operation(p_cb);
        break;

    case WICED_BT_MESH_CONFIG_KEY_REFRESH_PHASE_STATUS:
        mesh_key_refresh_phase2_status(p_cb, p_event, (wiced_bt_mesh_config_key_refresh_phase_status_data_t *)p_data);
        break;

    default:
        Log("Event:%d ignored\n", event);
        break;
    }
}

/*
 * State machine sent at least one command to transition to Key Refresh Phase 3 state and waiting for all scheduled operations to complete
 */
void mesh_configure_state_key_refresh3(mesh_provision_cb_t *p_cb, uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    int net_key_idx;

    switch (event)
    {
    case WICED_BT_MESH_TX_COMPLETE:
        Log("KR3 Node unreachable:%04x p_event:%x op_event:%x", p_event->dst, p_event, p_cb->p_first != NULL ? p_cb->p_first->p_event : 0);
        if ((p_cb->p_first == NULL) || (p_cb->p_first->p_event != p_event))
            break;

        start_next_op(p_cb);

        if (p_cb->p_first == NULL)
            for (net_key_idx = 0; net_key_idx < p_mesh_db->num_net_keys; net_key_idx++)
                mesh_key_refresh_phase3_completed(p_cb, &p_mesh_db->net_key[net_key_idx]);
        break;

    case WICED_BT_MESH_CONFIG_NODE_RESET_STATUS:
        if ((p_cb->p_first->operation != CONFIG_OPERATION_NODE_RESET) || (p_cb->p_first->p_event->dst != p_event->src))
        {
            Log("Ignored KR3 Node Reset Status from:%04x", p_event->src);
            break;
        }
        Log("KR3 Node Reset Status from:%04x", p_event->src);

        start_next_op(p_cb);

        if (p_cb->p_first == NULL)
        {
            for (net_key_idx = 0; net_key_idx < p_mesh_db->num_net_keys; net_key_idx++)
                mesh_key_refresh_phase3_completed(p_cb, &p_mesh_db->net_key[net_key_idx]);
        }
        break;

    case WICED_BT_MESH_PROXY_CONNECTION_STATUS:
        mesh_key_refresh_link_status(p_cb, p_event, (wiced_bt_mesh_connect_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROXY_FILTER_STATUS:
        if (p_cb->p_first->operation == CONFIG_OPERATION_FILTER_ADD)
            start_next_op(p_cb);
        configure_execute_pending_operation(p_cb);
        break;

    case WICED_BT_MESH_CONFIG_KEY_REFRESH_PHASE_STATUS:
        mesh_key_refresh_phase3_status(p_cb, p_event, (wiced_bt_mesh_config_key_refresh_phase_status_data_t *)p_data);
        break;

    default:
        Log("Event:%d ignored\n", event);
        break;
    }
}

void mesh_provision_connecting_link_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_link_report_data_t *p_data)
{
    ods("provision connecting link status:%d rpr state:%d provisioner:%04x over gatt:%d\n", p_data->link_status, p_data->rpr_state, p_cb->provisioner_addr, p_data->over_gatt);
    if (p_data->link_status == WICED_BT_MESH_REMOTE_PROVISION_STATUS_SUCCESS) // || (p_data->link_status == WICED_BT_MESH_REMOTE_PROVISION_STATUS_LINK_OPENED))
    {
        p_cb->provision_conn_id = 1;
#ifndef CLIENTCONTROL
        p_cb->over_gatt = (p_cb->provisioner_addr == p_cb->unicast_addr);
#else
        p_cb->over_gatt = p_data->over_gatt;
#endif
        return;
    }
    p_cb->provision_conn_id = 0;

    if (p_cb->provisioner_addr == p_cb->unicast_addr)
        wiced_bt_mesh_config_client_connection_state_change(0);

    if (p_cb->retries++ < MAX_CONNECT_RETRIES)
    {
        mesh_client_provision_connect(p_cb);
    }
    else
    {
        Log("Provisioning failed\n");
        p_cb->state = PROVISION_STATE_IDLE;

        provision_status_notify(p_cb, MESH_CLIENT_PROVISION_STATUS_FAILED);
    }
}

void mesh_provision_process_device_caps(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_device_capabilities_data_t *p_data)
{
    int i;
    uint8_t db_changed;
    uint16_t src = (p_event != NULL) ? p_event->src : 0;

    Log("Provision: Device Capabilities from Server:%x Num elements:%x algo:%x pub key type:%d oob type:%d oob size:%d oob_action:%d input oob size:%d action:%d",
        src, p_data->elements_num, p_data->algorithms, p_data->pub_key_type, p_data->static_oob_type,
        p_data->output_oob_size, p_data->output_oob_action, p_data->input_oob_size, p_data->input_oob_action);

    p_cb->state = PROVISION_STATE_PROVISIONING;
    p_cb->num_elements = p_data->elements_num;
    p_cb->addr = wiced_bt_mesh_db_alloc_unicast_addr(p_mesh_db, p_mesh_db->unicast_addr, p_data->elements_num, &db_changed);

    // Make sure that RPL for the entry with this unicast address does not exist. Can happen after export/import.
    for (i = 0; i < p_data->elements_num; i++)
    {
        mesh_del_seq(p_cb->addr + i);
        wiced_bt_mesh_core_del_seq(p_cb->addr + i);
    }

    if (db_changed)
    {
        wiced_bt_mesh_db_store(p_mesh_db);
        p_cb->db_changed = WICED_TRUE;
    }

    provision_status_notify(p_cb, MESH_CLIENT_PROVISION_STATUS_PROVISIONING);

    wiced_bt_mesh_event_t *p_event1;
    if ((p_event1 = mesh_configure_create_event(p_cb->provisioner_addr, (p_cb->provisioner_addr != p_cb->unicast_addr))) == NULL)
        return;

    wiced_bt_mesh_provision_start_data_t start;
    start.addr = p_cb->addr;
    start.net_key_idx = 0;      //ToDo: select somehow desired netkey. For now use primary net key (0)
    start.algorithm = 0;
    start.public_key_type = 0; // Don't use OOB Pub Key p_data->pub_key_type;    // use value passed in the capabilities
    start.auth_method = ((p_cb->oob_data_len == 0) || (p_data->static_oob_type == 0)) ? 0 : WICED_BT_MESH_PROVISION_START_AUTH_METHOD_STATIC;
    start.auth_action = 0;
    start.auth_size = 0;

    Log("Provision Start Server:%x pub_key_type:%x auth_method:%x auth_action:%d auth_size:%d", p_cb->provisioner_addr, start.public_key_type, start.algorithm, start.auth_method, start.auth_action, start.auth_size);

    wiced_bt_mesh_provision_start(p_event1, &start);
}

void mesh_provision_provisioning_link_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_link_report_data_t *p_data)
{
    ods("provision provisioning link status:%d provisioner:%04x\n", p_data->link_status, p_cb->provisioner_addr);

    if (p_data->link_status == WICED_BT_MESH_REMOTE_PROVISION_STATUS_SUCCESS)// || (p_data->reason == WICED_BT_MESH_REMOTE_PROVISION_STATUS_LINK_OPENED))
    {
        Log("unexpected link up\n");
        return;
    }
    p_cb->provision_conn_id = 0;

    if (p_cb->provisioner_addr == p_cb->unicast_addr)
        wiced_bt_mesh_config_client_connection_state_change(0);

    if (p_cb->retries++ < MAX_CONNECT_RETRIES)
    {
        p_cb->state = PROVISION_STATE_CONNECTING;
        mesh_client_provision_connect(p_cb);
    }
    else
    {
        Log("Provisioning failed\n");
        p_cb->state = PROVISION_STATE_IDLE;

        provision_status_notify(p_cb, MESH_CLIENT_PROVISION_STATUS_FAILED);
    }
}

void mesh_provision_process_provision_end(mesh_provision_cb_t *p_cb, wiced_bt_mesh_provision_status_data_t *p_data)
{
    wiced_bt_mesh_db_node_t* p_node;

    if (p_data->result != 0)
    {
        provision_status_notify(p_cb, MESH_CLIENT_PROVISION_STATUS_FAILED);

        p_cb->state = PROVISION_STATE_IDLE;
        Log("Provision end failed\n");
    }
    else
    {
        if (p_cb->provision_procedure == WICED_BT_MESH_PROVISION_PROCEDURE_PROVISION)
        {
            // Do not allow to create a node with the same UUID if one already exists. This can
            // happen if a node was manually factory reset, and now it is being provisioned again.
            if ((p_node = wiced_bt_mesh_db_node_get_by_uuid(p_mesh_db, p_cb->uuid)) != NULL)
            {
                // Delete the old one.
                wiced_bt_mesh_db_node_remove(p_mesh_db, p_node->unicast_address);

                mesh_del_seq(p_node->unicast_address);
                wiced_bt_mesh_core_del_seq(p_node->unicast_address);
            }
            p_node = wiced_bt_mesh_db_node_create(p_mesh_db, p_cb->provisioning_device_name, p_cb->addr, p_cb->uuid, p_cb->num_elements, p_data->dev_key, p_data->net_key_idx);
        }
        else
        {
            p_node = wiced_bt_mesh_db_node_get_by_element_addr(p_mesh_db, p_data->provisioner_addr);
            if (p_node == NULL)
                return;

            if (p_cb->provision_procedure == WICED_BT_MESH_PROVISION_PROCEDURE_DEV_KEY_REFRESH)
            {
                p_cb->state = PROVISION_STATE_IDLE;
                memcpy(p_node->device_key, p_data->dev_key, sizeof(p_node->device_key));
                wiced_bt_mesh_db_store(p_mesh_db);
                return;
            }
            if (p_cb->provision_procedure == WICED_BT_MESH_PROVISION_PROCEDURE_NODE_ADDRESS_REFRESH)
            {
            }
        }

        p_node->num_hops = NUM_HOPS_UNKNOWN;

        wiced_bt_mesh_db_store(p_mesh_db);

        provision_status_notify(p_cb, MESH_CLIENT_PROVISION_STATUS_END);

        if ((p_cb->provision_conn_id != 0) && p_cb->over_gatt && (p_cb->provisioner_addr == p_cb->unicast_addr))
        {
            // wait for the provisioner to disconnect
            p_cb->state = PROVISION_STATE_PROVISION_DISCONNECTING;
        }
        else if (p_cb->over_gatt && (p_cb->provisioner_addr == p_cb->unicast_addr))
        {
            p_cb->state = PROVISION_STATE_NODE_CONNECTING;
            p_cb->retries = 0;
            mesh_client_connect_proxy(p_cb, CONNECT_TYPE_NODE_ID, NODE_IDENTITY_SCAN_DURATION);
        }
        else
        {
            p_cb->state = PROVISION_STATE_GET_REMOTE_COMPOSITION_DATA;

            mesh_configure_set_local_device_key(p_cb->addr);
            mesh_configure_composition_data_get(p_cb, OPERATION_REMOTE);
        }
    }
}

void mesh_provision_process_get_oob_data(mesh_provision_cb_t* p_cb, wiced_bt_mesh_event_t* p_event, wiced_bt_mesh_provision_device_oob_request_data_t* p_data)
{
    if ((p_cb->oob_data_len == 0) || (p_data->type != WICED_BT_MESH_PROVISION_GET_OOB_TYPE_ENTER_STATIC))
    {
        Log("OOB type not supported:%d\n", p_data->type);
        return;
    }
    p_event = mesh_configure_create_event(p_data->provisioner_addr, (p_data->provisioner_addr != p_cb->unicast_addr));
    if (p_event != NULL)
    {
        wiced_bt_mesh_provision_client_set_oob(p_event, p_cb->oob_data, p_cb->oob_data_len);
    }
}

void mesh_provision_disconnecting_link_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_link_report_data_t *p_data)
{
    Log("Provisioning disconecting Link Status:%04x provisioner:%x", p_data->link_status, p_event->src);

    p_cb->provision_conn_id = 0;
    p_cb->state = PROVISION_STATE_NODE_CONNECTING;
    p_cb->retries = 0;
    mesh_client_connect_proxy(p_cb, CONNECT_TYPE_NODE_ID, p_cb->scan_duration);
}

void mesh_node_connecting_link_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_connect_status_data_t *p_data)
{
    Log("Node Connecting Link Status provisioner:%x addr:%04x connected:%x over_gatt:%d", p_data->provisioner_addr, p_data->addr, p_data->connected, p_data->over_gatt);

    if (p_cb->p_connect_status != NULL)
        p_cb->p_connect_status(p_data->connected, p_data->provisioner_addr, 0, p_data->over_gatt);

    if (p_data->connected)
    {
        p_cb->state = PROVISION_STATE_GET_REMOTE_COMPOSITION_DATA;

        mesh_configure_set_local_device_key(p_cb->addr);
        mesh_configure_composition_data_get(p_cb, OPERATION_REMOTE);
    }
    else
    {
        // If we failed to connect to node identity, try it again once, then try to connect to any proxy.
        if (p_cb->retries == 0)
        {
            p_cb->retries++;
            mesh_client_connect_proxy(p_cb, CONNECT_TYPE_NODE_ID, p_cb->scan_duration);
        }
        else if (p_cb->retries < MAX_CONNECT_RETRIES)
        {
            p_cb->retries++;
            mesh_client_connect_proxy(p_cb, CONNECT_TYPE_NET_ID, p_cb->scan_duration);
        }
        else
        {
            Log("Provisioning failed\n");
            p_cb->retries = 0;
            p_cb->state = PROVISION_STATE_IDLE;

            provision_status_notify(p_cb, MESH_CLIENT_PROVISION_STATUS_FAILED);
        }
    }
}

void mesh_configure_set_local_device_key(uint16_t addr)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    wiced_bt_mesh_set_dev_key_data_t set;

    if (p_mesh_db == NULL)
        return;

    if (p_cb->device_key_addr != addr)
    {
        p_cb->device_key_addr = addr;

        set.dst = addr;

        wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_addr(p_mesh_db, addr);
        if (node != NULL)
        {
            memcpy(set.dev_key, node->device_key, 16);
            set.net_key_idx = node->net_key[0].index;

            char buf[160];
            int i;
            sprintf(buf, "Set Dev Key addr:%04x dev_key:", addr);
            for (i = 0; i < 16; i++)
            {
                sprintf(&buf[strlen(buf)], "%02x ", set.dev_key[i]);
            }
            Log(buf);
            wiced_bt_mesh_provision_set_dev_key(&set);
        }
    }
}

void mesh_configure_composition_data_get(mesh_provision_cb_t *p_cb, uint8_t is_local)
{
    uint16_t dst = is_local ? p_cb->unicast_addr : p_cb->addr;
    wiced_bt_mesh_event_t *p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
    if (p_event != NULL)
    {
        wiced_bt_mesh_config_composition_data_get_data_t data;
        // At provisioning/configuration a Configuration Client shall send a Config Composition Data Get message
        // with the Page field value set to 0 to get all models to be configured. Page 0x80 can't be used at that time
        // because it can contain elements/models which can't be configured.
        data.page_number = 0;
        wiced_bt_mesh_config_composition_data_get(p_event, &data);
    }
}

void mesh_process_tx_complete(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event)
{
    pending_operation_t *p_op = p_cb->p_first;
    if ((p_event->status.tx_flag == TX_STATUS_COMPLETED) || (p_event->status.tx_flag == TX_STATUS_ACK_RECEIVED))
        return;
    if (p_event->status.tx_flag == TX_STATUS_FAILED)
    {
        // p_event contains all information about the message that has not been acknowledged.
        // for example if it was onoff set, the p_event->model == 0x1001 and p_event->opcode = 0x8202 and
        // p_event->dst is the address of the device which failed to send the ack.
        const char *p_name = get_component_name(p_event->dst);

        if ((p_op == NULL) || (p_op->p_event != p_event))
            wiced_bt_mesh_release_event(p_event);

        if ((p_name != NULL) && (p_cb->p_node_connect_status != NULL))
            p_cb->p_node_connect_status(MESH_CLIENT_NODE_ERROR_UNREACHABLE, (char *)p_name);
        return;
    }
    if ((p_op != NULL) && (p_op->p_event == p_event))
        start_next_op(p_cb);
    else
        wiced_bt_mesh_release_event(p_event);
}

uint8_t composition_data_get_num_elements(uint8_t *p_data, uint16_t len)
{
    uint8_t elem_idx = 0;
    while (len)
    {
        if (len < 4)
            return elem_idx;

        uint16_t location = p_data[0] + (p_data[1] << 8);
        uint8_t num_models = p_data[2];
        uint8_t num_vs_models = p_data[3];

        p_data += 4;
        len -= 4;

        if (len < 2 * num_models + 4 * num_vs_models)
            return elem_idx;

        len -= (2 * num_models + 4 * num_vs_models);
        p_data += (2 * num_models + 4 * num_vs_models);

        elem_idx++;
    }
    return elem_idx;
}

void mesh_configure_composition_data_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_composition_data_status_data_t *p_data)
{
    Log("Composition Data: from:%x page:%x CID:%x PID:%x VID:%x CRPL:%x Features:%x", p_event->src,
        p_data->page_number, p_data->data[0] + (p_data->data[1] << 8),
        p_data->data[2] + (p_data->data[3] << 8), p_data->data[4] + (p_data->data[5] << 8),
        p_data->data[6] + (p_data->data[7] << 8), p_data->data[8] + (p_data->data[9] << 8));

    // Save received composition data
    if (p_event->src == p_cb->unicast_addr)
    {
        // received local composition data, queue operations to configure local device.
        if (p_cb->p_local_composition_data != NULL)
            wiced_bt_free_buffer(p_cb->p_local_composition_data);

        p_cb->p_local_composition_data = p_data;
        p_cb->company_id = p_data->data[0] + (p_data->data[1] << 8);

        // If this is exactly the same device that has been configured last time, which will be likely
        // the case every time except for the very first time DB is created, do not update the
        // Provisioner Database to avoid changing the time stamp
        p_cb->store_config = !wiced_bt_mesh_db_node_check_composition_data(p_mesh_db, p_event->src, p_data->data, p_data->data_len);

        if (p_cb->store_config)
        {
            wiced_bt_mesh_db_node_set_composition_data(p_mesh_db, p_event->src, p_data->data, p_data->data_len);
            wiced_bt_mesh_db_store(p_mesh_db);
        }
        configure_queue_local_device_operations(p_cb);
        configure_execute_pending_operation(p_cb);
        return;
    }

    if (p_cb->p_remote_composition_data != NULL)
        wiced_bt_free_buffer(p_cb->p_remote_composition_data);

    provision_status_notify(p_cb, MESH_CLIENT_PROVISION_STATUS_CONFIGURING);

    if (!wiced_bt_mesh_db_node_set_composition_data(p_mesh_db, p_event->src, p_data->data, p_data->data_len))
    {
        Log("comp failed\n");
        provision_status_notify(p_cb, MESH_CLIENT_PROVISION_STATUS_FAILED);
        return;
    }
    p_cb->p_remote_composition_data = p_data;

    p_cb->state = PROVISION_STATE_CONFIGURATION;

    wiced_bt_mesh_db_store(p_mesh_db);

    configure_queue_remote_device_operations(p_cb);
    configure_execute_pending_operation(p_cb);
}

void mesh_configure_proxy_connection_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_connect_status_data_t *p_data)
{
    if (p_cb->p_connect_status != NULL)
        p_cb->p_connect_status(p_data->connected, p_data->addr, 0, p_data->over_gatt);

    Log("Configuration Link Status provisioner:%x addr:%04x connected:%x over_gatt:%d", p_data->provisioner_addr, p_data->addr, p_data->connected, p_data->over_gatt);
    if (p_cb->scan_duration == 0)
        p_cb->scan_duration = SCAN_DURATION_DEFAULT;

    if (p_data->connected)
    {
        configure_execute_pending_operation(p_cb);
    }
    else if (p_cb->retries++ < MAX_CONNECT_RETRIES - 1)
    {
        mesh_client_connect_proxy(p_cb, CONNECT_TYPE_NODE_ID, p_cb->scan_duration);
    }
    else if (p_cb->retries++ == MAX_CONNECT_RETRIES)
    {
        mesh_client_connect_proxy(p_cb, CONNECT_TYPE_NET_ID, p_cb->scan_duration);
    }
    else
    {
        Log("Provisioning failed\n");
        p_cb->state = PROVISION_STATE_IDLE;
        clean_pending_op_queue(0);

        provision_status_notify(p_cb, MESH_CLIENT_PROVISION_STATUS_FAILED);
    }
}

void mesh_key_refresh_link_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_connect_status_data_t *p_data)
{
    if (p_cb->p_connect_status != NULL)
        p_cb->p_connect_status(p_data->connected, p_data->provisioner_addr, 0, p_data->over_gatt);

    Log("KeyRefresh Link Status provisioner:%x addr:%04x connected:%x over_gatt:%d", p_data->provisioner_addr, p_data->addr, p_data->connected, p_data->over_gatt);

    if (p_data->connected)
    {
        add_filter(p_cb, 0xFFFF);
    }
    else if (p_cb->retries++ < MAX_CONNECT_RETRIES - 1)
    {
        mesh_client_connect_proxy(p_cb, CONNECT_TYPE_NET_ID, p_cb->scan_duration);
    }
    else
    {
        p_cb->state = PROVISION_STATE_IDLE;
    }
}

void clean_pending_op_queue(uint16_t addr)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    pending_operation_t *p_next, *p_op, *p_prev = NULL;

    for (p_op = p_cb->p_first; p_op != NULL; p_op = p_next)
    {
        p_next = p_op->p_next;

        // delete pending operation if we are deleting them all, or if operation has specified DST
        if ((addr == 0) || (p_op->p_event == NULL) || (p_op->p_event->dst == addr))
        {
            if (p_op->p_event)
                wiced_bt_mesh_release_event(p_op->p_event);

            wiced_bt_free_buffer(p_op);

            if (p_cb->p_first == p_op)
                p_cb->p_first = p_next;
            else
                p_prev->p_next = p_next;
        }
        else
        {
            p_prev = p_op;
        }
    }
}

void start_next_op(mesh_provision_cb_t *p_cb)
{
    pending_operation_t *p_op = configure_pending_operation_dequeue(p_cb);

    if (p_op)
    {
        if (p_op->p_event)
            wiced_bt_mesh_release_event(p_op->p_event);
        wiced_bt_free_buffer(p_op);
    }

    configure_execute_pending_operation(p_cb);

    if (p_cb->p_first == NULL)
    {
        // If state is Idle, we are configuring local device
        if (p_cb->state == PROVISION_STATE_CONFIGURATION)
        {
            // We are provisioning/configuring over GATT.  If GATT proxy feature
            // is not enabled, we need to disconnect.
            wiced_bt_mesh_db_node_t *node = mesh_find_node(p_mesh_db, p_cb->addr);
            ods("configuration done node:%04x proxy_addr:%04x proxy_conn_id:%d\n", p_cb->addr, p_cb->proxy_addr, p_cb->proxy_conn_id);
            if ((p_cb->proxy_conn_id != 0) && p_cb->over_gatt && (node->feature.gatt_proxy != MESH_FEATURE_ENABLED))
            {
                p_cb->state = PROVISION_STATE_CONFIGURE_DISCONNECTING;
                wiced_bt_mesh_client_proxy_disconnect();
            }
            else
            {
                p_cb->state = PROVISION_STATE_IDLE;
                if (p_cb->over_gatt)
                    p_cb->proxy_addr = p_cb->addr;
            }
        }
        else if ((p_cb->state != PROVISION_STATE_KEY_REFRESH_1) && (p_cb->state != PROVISION_STATE_KEY_REFRESH_2))
        {
            p_cb->state = PROVISION_STATE_IDLE;
            if (p_cb->db_changed)
            {
                p_cb->db_changed = WICED_FALSE;

                if (provision_cb.p_database_changed != NULL)
                    provision_cb.p_database_changed(p_mesh_db->name);
            }
        }
    }
}

void mesh_configure_net_key_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_netkey_status_data_t *p_data)
{
    // Check that this is not reply to a retransmission
    pending_operation_t *p_op = p_cb->p_first;
    if ((p_op == NULL) || (p_op->p_event == NULL) || (p_op->p_event->dst != p_event->src) ||
        (p_op->operation != CONFIG_OPERATION_NET_KEY_UPDATE) ||
        (p_op->uu.net_key_change.net_key_idx != p_data->net_key_idx))
    {
        Log("Ignored NetKey Status from:0x%04x status:%d NetKey Index:%x", p_event->src, p_data->status, p_data->net_key_idx);
        return;
    }
    Log("NetKey Status from:0x%04x status:%d NetKey Index:%x", p_event->src, p_data->status, p_data->net_key_idx);

    if (p_data->status != 0)
    {
        // ToDo ??
    }
    if ((p_data->status == 0) && p_cb->store_config)
    {
        wiced_bt_mesh_db_node_net_key_add(p_mesh_db, p_event->src, p_data->net_key_idx);
        wiced_bt_mesh_db_store(p_mesh_db);
        p_cb->db_changed = WICED_TRUE;
    }
    start_next_op(p_cb);
}

void mesh_configure_app_key_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_appkey_status_data_t *p_data)
{
    wiced_bt_mesh_db_node_t* p_node;
    int i;

    // Check that this is not reply to a retransmission
    pending_operation_t *p_op = p_cb->p_first;
    if ((p_op == NULL) || (p_op->p_event == NULL) || (p_op->p_event->dst != p_event->src) ||
        (p_op->operation != CONFIG_OPERATION_APP_KEY_UPDATE) ||
        (p_op->uu.app_key_change.net_key_idx != p_data->net_key_idx) ||
        (p_op->uu.app_key_change.app_key_idx != p_data->app_key_idx))
    {
        Log("Ignored AppKey Status from:0x%04x status:%d NetKey Index:%x ApptKey Index:%x", p_event->src,
            p_data->status, p_data->net_key_idx, p_data->app_key_idx);
        return;
    }
    Log("AppKey Status from:0x%04x status:%d NetKey Index:%x ApptKey Index:%x", p_event->src,
        p_data->status, p_data->net_key_idx, p_data->app_key_idx);

    // If operation failed, this device likely not support several app keys as required by this mesh client.
    // Reset device.
    if ((p_data->status != 0) && (p_cb->addr == p_event->src) && ((p_node = mesh_find_node(p_mesh_db, p_event->src)) != NULL))
    {
        p_cb->state = PROVISION_STATE_IDLE;
        for (i = 0; i < p_node->num_elements; i++)
            clean_pending_op_queue(p_node->unicast_address + i);

        // Notify client that provision failed. This will reset the node.
        provision_status_notify(p_cb, MESH_CLIENT_PROVISION_STATUS_FAILED);
        return;
    }
    if ((p_data->status == 0) && p_cb->store_config)
    {
        wiced_bt_mesh_db_node_app_key_add(p_mesh_db, p_event->src, p_data->net_key_idx, p_data->app_key_idx);
        wiced_bt_mesh_db_store(p_mesh_db);
        p_cb->db_changed = WICED_TRUE;
    }
    start_next_op(p_cb);
}

void mesh_configure_phase_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_key_refresh_phase_status_data_t *p_data)
{
    // Check that this is not reply to a retransmission
    pending_operation_t *p_op = p_cb->p_first;
    if ((p_op == NULL) || (p_op->p_event == NULL) || (p_op->p_event->dst != p_event->src) ||
        (p_op->uu.kr_phase_set.net_key_idx != p_data->net_key_idx))
    {
        Log("Ognored Set Phase Status from:0x%04x status:%d NetKey Index:%x Phase:%d", p_event->src, p_data->status, p_data->net_key_idx, p_data->phase);
        return;
    }
    Log("Set Phase Status from:0x%04x status:%d NetKey Index:%x Phase:%d", p_event->src, p_data->status, p_data->net_key_idx, p_data->phase);
    if (p_data->status != 0)
    {
        // ToDo ??
    }
    if ((p_data->status == 0) && p_cb->store_config)
    {
        wiced_bt_mesh_db_node_net_key_update(p_mesh_db, p_event->src, p_data->net_key_idx, p_data->phase);
        wiced_bt_mesh_db_store(p_mesh_db);
        p_cb->db_changed = WICED_TRUE;
    }
    start_next_op(p_cb);
}

/*
 * All operations that should have executed for Phase1 of the Key Refresh have been "attempted".
 */
void mesh_key_refresh_phase1_completed(mesh_provision_cb_t *p_cb, wiced_bt_mesh_db_net_key_t *net_key)
{
    int i, node_idx;
    int node_net_key_idx;
    int app_key_idx, node_app_key_idx;
    uint16_t *node_list;
    int num_nodes_to_update = 0;

    Log("KR phase1 completed key phase:%d\n", net_key->phase);

    if ((node_list = (uint16_t *)wiced_bt_get_buffer(sizeof(uint16_t) * p_mesh_db->num_nodes)) == NULL)
        return;

    // If this key is not being refreshed. Go to the next one.
    if (net_key->phase != WICED_BT_MESH_KEY_REFRESH_PHASE_FIRST)
    {
        Log("phase1 completed phase:%d\n", net_key->phase);
        return;
    }
    // Check if all non-blocked nodes are updated with all keys
    // For every other node in the network, check if the same key is present.  If true, schedule the update.
    // We can also ignore provisioner nodes because provisioner may be offline most of the time, but they will get DB out of bound.
    for (node_idx = 0; node_idx < p_mesh_db->num_nodes; node_idx++)
    {
        if (p_mesh_db->node[node_idx].blocked || is_provisioner(&p_mesh_db->node[node_idx]))
            continue;

        for (node_net_key_idx = 0; node_net_key_idx < p_mesh_db->node[node_idx].num_net_keys; node_net_key_idx++)
        {
            if (net_key->index == p_mesh_db->node[node_idx].net_key[node_net_key_idx].index)
            {
                if (p_mesh_db->node[node_idx].net_key[node_net_key_idx].phase == WICED_BT_MESH_KEY_REFRESH_PHASE_FIRST)
                    continue;

                // this key on that node has not been updated
                for (i = 0; i < num_nodes_to_update; i++)
                {
                    if (p_mesh_db->node[node_idx].unicast_address == node_list[i])
                        break;
                }
                if (i == num_nodes_to_update)
                {
                    node_list[num_nodes_to_update++] = p_mesh_db->node[node_idx].unicast_address;
                    break;
                }
            }
        }
    }

    // Go through the application keys that are bound to the network key being updated
    for (app_key_idx = 0; app_key_idx < p_mesh_db->num_app_keys; app_key_idx++)
    {
        if (p_mesh_db->app_key[app_key_idx].bound_net_key_index != net_key->index)
            continue;

        // For every other node in the network, check if the same key is present.  If true, check if it has been updated.
        for (node_idx = 0; node_idx < p_mesh_db->num_nodes; node_idx++)
        {
            if (p_mesh_db->node[node_idx].blocked || is_provisioner(&p_mesh_db->node[node_idx]))
                continue;

            for (node_app_key_idx = 0; node_app_key_idx < p_mesh_db->node[node_idx].num_app_keys; node_app_key_idx++)
            {
                if (p_mesh_db->app_key[app_key_idx].index == p_mesh_db->node[node_idx].app_key[node_app_key_idx].index)
                {
                    if (p_mesh_db->node[node_idx].app_key[node_app_key_idx].phase == WICED_BT_MESH_KEY_REFRESH_PHASE_FIRST)
                        continue;

                    // this key on that node has not been updated
                    for (i = 0; i < num_nodes_to_update; i++)
                    {
                        if (p_mesh_db->node[node_idx].unicast_address == node_list[i])
                            break;
                    }
                    if (i == num_nodes_to_update)
                    {
                        node_list[num_nodes_to_update++] = p_mesh_db->node[node_idx].unicast_address;
                        break;
                    }
                }
            }
        }
    }

    if (num_nodes_to_update == 0)
    {
        // if all nodes have been updated, switch to key_refresh phase 2.
        wiced_bt_free_buffer(node_list);
        mesh_client_transition_next_key_refresh_phase(p_cb, net_key, WICED_BT_MESH_KEY_REFRESH_TRANSITION_PHASE2);

        if (p_cb->p_first != NULL)
            configure_execute_pending_operation(p_cb);
        return;
    }
    // issue node connect status for each node that we failed to deliver the keys
    for (node_idx = 0; node_idx < num_nodes_to_update; node_idx++)
    {
        const char *p_name = get_component_name(node_list[node_idx]);

        if ((p_name != NULL) && (p_cb->p_node_connect_status != NULL))
            p_cb->p_node_connect_status(MESH_CLIENT_NODE_ERROR_UNREACHABLE, (char *)p_name);
    }
    wiced_bt_free_buffer(node_list);
    p_cb->state = PROVISION_STATE_IDLE;
}

void mesh_key_refresh_phase2_completed(mesh_provision_cb_t *p_cb, wiced_bt_mesh_db_net_key_t *net_key)
{
    int i, node_idx;
    int node_net_key_idx;
    uint16_t *node_list;
    int num_nodes_to_update = 0;

    if (net_key == NULL)
        return;

    Log("KR phase2 completed key phase:%d\n", net_key->phase);

    if ((node_list = (uint16_t *)wiced_bt_get_buffer(sizeof(uint16_t) * p_mesh_db->num_nodes)) == NULL)
        return;

    // Should be in phase2.
    // if (net_key->phase != WICED_BT_MESH_KEY_REFRESH_PHASE_SECOND)
    //    return;

    // Check if all non-blocked nodes are updated with all keys
    for (node_idx = 0; node_idx < p_mesh_db->num_nodes; node_idx++)
    {
        if (p_mesh_db->node[node_idx].blocked || is_provisioner(&p_mesh_db->node[node_idx]))
            continue;

        for (node_net_key_idx = 0; node_net_key_idx < p_mesh_db->node[node_idx].num_net_keys; node_net_key_idx++)
        {
            if (net_key->index == p_mesh_db->node[node_idx].net_key[node_net_key_idx].index)
            {
                if (p_mesh_db->node[node_idx].net_key[node_net_key_idx].phase == WICED_BT_MESH_KEY_REFRESH_PHASE_SECOND)
                    continue;

                // This node is not in the phase2. Add it to the array.
                for (i = 0; i < num_nodes_to_update; i++)
                {
                    if (p_mesh_db->node[node_idx].unicast_address == node_list[i])
                        break;
                }
                if (i == num_nodes_to_update)
                {
                    node_list[num_nodes_to_update++] = p_mesh_db->node[node_idx].unicast_address;
                    break;
                }
            }
        }
    }
    if (num_nodes_to_update == 0)
    {
        // if all nodes have been updated, switch to key_refresh phase 3.
        mesh_client_transition_next_key_refresh_phase(p_cb, net_key, WICED_BT_MESH_KEY_REFRESH_TRANSITION_PHASE3);

        if (p_cb->p_first != NULL)
            configure_execute_pending_operation(p_cb);
    }
    else
    {
        // issue node connect status for each node that we failed to deliver the keys
        for (node_idx = 0; node_idx < num_nodes_to_update; node_idx++)
        {
            const char *p_name = get_component_name(node_list[node_idx]);

            if ((p_name != NULL) && (p_cb->p_node_connect_status != NULL))
                p_cb->p_node_connect_status(MESH_CLIENT_NODE_ERROR_UNREACHABLE, (char *)p_name);
        }
        p_cb->state = PROVISION_STATE_IDLE;
    }
    wiced_bt_free_buffer(node_list);
}

void mesh_key_refresh_phase3_completed(mesh_provision_cb_t *p_cb, wiced_bt_mesh_db_net_key_t *net_key)
{
    uint16_t *p_blocked_addr;
    uint16_t num_blocked = 0;
    int i, node_idx;
    int node_net_key_idx;
    int app_key_idx;
    uint16_t *node_list;
    int num_nodes_to_update = 0;
    wiced_bool_t db_updated = WICED_FALSE;
    uint16_t *net_key_list;
    int node_can_be_removed;
    wiced_bt_mesh_db_net_key_t *net_key1;
    int key_refresh_required = WICED_FALSE;

    p_cb->state = PROVISION_STATE_IDLE;

    if (net_key == NULL)
        return;

    Log("KR phase3 completed key phase:%d\n", net_key->phase);

    // Should be in phase3.
    if (net_key->phase != WICED_BT_MESH_KEY_REFRESH_PHASE_THIRD)
        return;

    if ((node_list = (uint16_t *)wiced_bt_get_buffer(sizeof(uint16_t) * p_mesh_db->num_nodes)) == NULL)
        return;

    // Check if all non-blocked nodes are in phase3.  If not schedule the Set Phase.
    for (node_idx = 0; node_idx < p_mesh_db->num_nodes; node_idx++)
    {
        if (p_mesh_db->node[node_idx].blocked || is_provisioner(&p_mesh_db->node[node_idx]))
            continue;

        for (node_net_key_idx = 0; node_net_key_idx < p_mesh_db->node[node_idx].num_net_keys; node_net_key_idx++)
        {
            if (net_key->index == p_mesh_db->node[node_idx].net_key[node_net_key_idx].index)
            {
                if (p_mesh_db->node[node_idx].net_key[node_net_key_idx].phase == WICED_BT_MESH_KEY_REFRESH_PHASE_THIRD)
                    continue;

                // this key on that node has not been updated, add it to the list of nodes to be updated.
                for (i = 0; i < num_nodes_to_update; i++)
                {
                    if (p_mesh_db->node[node_idx].unicast_address == node_list[i])
                        break;
                }
                if (i == num_nodes_to_update)
                {
                    node_list[num_nodes_to_update++] = p_mesh_db->node[node_idx].unicast_address;
                    break;
                }
            }
        }
    }
    if (num_nodes_to_update != 0)
    {
        for (node_idx = 0; node_idx < num_nodes_to_update; node_idx++)
        {
            const char *p_name = get_component_name(node_list[node_idx]);

            if ((p_name != NULL) && (p_cb->p_node_connect_status != NULL))
                p_cb->p_node_connect_status(MESH_CLIENT_NODE_ERROR_UNREACHABLE, (char *)p_name);
        }
        wiced_bt_free_buffer(node_list);
        return;
    }
    wiced_bt_free_buffer(node_list);

    // Update Net/App Keys to reflect that the key refresh has been completed.
    net_key->phase = WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL;

    for (node_idx = 0; node_idx < p_mesh_db->num_nodes; node_idx++)
    {
        if (p_mesh_db->node[node_idx].blocked)
            continue;

        for (node_net_key_idx = 0; node_net_key_idx < p_mesh_db->node[node_idx].num_net_keys; node_net_key_idx++)
        {
            if (net_key->index != p_mesh_db->node[node_idx].net_key[node_net_key_idx].index)
                continue;

            p_mesh_db->node[node_idx].net_key[node_net_key_idx].phase = WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL;
            for (app_key_idx = 0; app_key_idx < p_mesh_db->node[node_idx].num_app_keys; app_key_idx++)
            {
                wiced_bt_mesh_db_app_key_t *app_key = wiced_bt_mesh_db_app_key_get_by_key_index(p_mesh_db, p_mesh_db->node[node_idx].app_key[app_key_idx].index);
                wiced_bt_mesh_db_net_key_t *net_key = wiced_bt_mesh_db_find_bound_net_key(p_mesh_db, app_key);
                if (net_key->phase == WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL)
                    p_mesh_db->node[node_idx].app_key[app_key_idx].phase = WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL;
            }
        }
    }

    // now we can reset the nodes that have been blocked
    // note that wiced_bt_mesh_db_node_remove will update p_mesh_db->node array, so need to allocate a separate array.
    if ((p_blocked_addr = (uint16_t *)wiced_bt_get_buffer(sizeof(uint16_t) * p_mesh_db->num_nodes)) == NULL)
        return;

    if ((net_key_list = (uint16_t *)wiced_bt_get_buffer(sizeof(uint16_t) * p_mesh_db->num_net_keys)) == NULL)
        return;

    for (node_idx = 0; node_idx < p_mesh_db->num_nodes; node_idx++)
    {
        if (!p_mesh_db->node[node_idx].blocked)
            continue;

        // It is possible that while in the key refresh, some node got the new keys and later was blocked.
        // If this is the case, we need to start another key refresh procedure.
        // check that all keys are in normal.
        node_can_be_removed = WICED_TRUE;

        for (node_net_key_idx = 0; node_net_key_idx < p_mesh_db->node[node_idx].num_net_keys; node_net_key_idx++)
        {
            net_key1 = find_net_key(p_mesh_db, p_mesh_db->node[node_idx].net_key[node_net_key_idx].index);

            if ((p_mesh_db->node[node_idx].net_key[node_net_key_idx].phase != WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL) &&
                (net_key1 != NULL))
            {
                node_can_be_removed = WICED_FALSE;

                if (net_key1 == net_key)
                {
                    // we will start new key refresh and new keys will be generated.  Mark that this node does not have new keys.
                    key_refresh_required = WICED_TRUE;
                    p_mesh_db->node[node_idx].net_key[node_net_key_idx].phase = WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL;
                }
            }
        }
        if (node_can_be_removed)
        {
            // node can be deleted
            p_blocked_addr[num_blocked++] = p_mesh_db->node[node_idx].unicast_address;
        }
    }
    for (i = 0; i < num_blocked; i++)
    {
        wiced_bt_mesh_db_node_remove(p_mesh_db, p_blocked_addr[i]);

        mesh_del_seq(p_blocked_addr[i]);
        wiced_bt_mesh_core_del_seq(p_blocked_addr[i]);
    }
    if (key_refresh_required)
    {
        p_cb->state = PROVISION_STATE_KEY_REFRESH_1;

        // generate new keys and schedule key updates for all nodes.  At least local device needs to be updated.
        mesh_key_refresh_update_keys(p_cb, net_key);
        mesh_key_refresh_continue(p_cb, net_key);
    }
    wiced_bt_mesh_db_store(p_mesh_db);

    if (p_cb->p_first != NULL)
    {
        p_cb->db_changed = WICED_TRUE;
        configure_execute_pending_operation(p_cb);
    }
    else
    {
        if (p_cb->db_changed)
        {
            p_cb->db_changed = WICED_FALSE;

            if (provision_cb.p_database_changed != NULL)
                provision_cb.p_database_changed(p_mesh_db->name);
        }
    }
}

void mesh_key_refresh_app_key_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_appkey_status_data_t *p_data)
{
    // Check that this is not reply to a retransmission
    pending_operation_t *p_op = p_cb->p_first;
    if ((p_op == NULL) || (p_op->p_event == NULL) || (p_op->p_event->dst != p_event->src) ||
        (p_op->operation != CONFIG_OPERATION_APP_KEY_UPDATE) ||
        (p_op->uu.app_key_change.net_key_idx != p_data->net_key_idx) ||
        (p_op->uu.app_key_change.app_key_idx != p_data->app_key_idx))
    {
        Log("Ignored AppKey Status from:0x%04x status:%d NetKey Index:%x ApptKey Index:%x", p_event->src,
            p_data->status, p_data->net_key_idx, p_data->app_key_idx);
        return;
    }

    Log("AppKey Status from:0x%04x status:%d NetKey Index:%x ApptKey Index:%x", p_event->src,
        p_data->status, p_data->net_key_idx, p_data->app_key_idx);

    if (p_data->status != 0)
    {
        // ToDo ??
    }
    if ((p_data->status == 0) && p_cb->store_config)
    {
        wiced_bt_mesh_db_node_app_key_update(p_mesh_db, p_event->src, p_data->net_key_idx, p_data->app_key_idx);
        wiced_bt_mesh_db_store(p_mesh_db);
        p_cb->db_changed = WICED_TRUE;
    }
    start_next_op(p_cb);
    if (p_cb->p_first == NULL)
        mesh_key_refresh_phase1_completed(p_cb, find_net_key(p_mesh_db, p_data->net_key_idx));
}

void mesh_key_refresh_net_key_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_netkey_status_data_t *p_data)
{
    // Check that this is not reply to a retransmission
    pending_operation_t *p_op = p_cb->p_first;
    if ((p_op == NULL) || (p_op->p_event == NULL) || (p_op->p_event->dst != p_event->src) ||
        (p_op->operation != CONFIG_OPERATION_NET_KEY_UPDATE) ||
        (p_op->uu.net_key_change.net_key_idx != p_data->net_key_idx))
    {
        Log("Ignored NetKey Status from:0x%04x status:%d NetKey Index:%x", p_event->src, p_data->status, p_data->net_key_idx);
        return;
    }

    Log("NetKey Status from:0x%04x status:%d NetKey Index:%x", p_event->src, p_data->status, p_data->net_key_idx);
    if (p_data->status != 0)
    {
        // ToDo ??
    }
    if ((p_data->status == 0) && p_cb->store_config)
    {
        wiced_bt_mesh_db_node_net_key_update(p_mesh_db, p_event->src, p_data->net_key_idx, WICED_BT_MESH_KEY_REFRESH_PHASE_FIRST);
        wiced_bt_mesh_db_store(p_mesh_db);
        p_cb->db_changed = WICED_TRUE;
    }
    start_next_op(p_cb);
    if (p_cb->p_first == NULL)
        mesh_key_refresh_phase1_completed(p_cb, find_net_key(p_mesh_db, p_data->net_key_idx));
}

void mesh_configure_model_app_bind_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_model_app_bind_status_data_t *p_data)
{
    // Check that this is not reply to a retransmission
    pending_operation_t *p_op = p_cb->p_first;
    if ((p_op == NULL) || (p_op->p_event == NULL) || (p_op->p_event->dst != p_event->src) ||
        (p_op->operation != CONFIG_OPERATION_MODEL_APP_BIND) ||
        (p_op->uu.app_key_bind.element_addr != p_data->element_addr) ||
        (p_op->uu.app_key_bind.company_id != p_data->company_id) ||
        (p_op->uu.app_key_bind.model_id != p_data->model_id) ||
        (p_op->uu.app_key_bind.app_key_idx != p_data->app_key_idx))
    {
        Log("Ignored App Bind Status from:%x status:%d Element addr:%04x Model ID:%x ApptKey Index:%x", p_event->src,
            p_data->status, p_data->element_addr, p_data->model_id, p_data->app_key_idx);
        return;
    }

    Log("Model App Bind Status from:%x status:%d Element addr:%04x Model ID:%x ApptKey Index:%x", p_event->src,
        p_data->status, p_data->element_addr, p_data->model_id, p_data->app_key_idx);

    // If operation failed during initial configuration, bad device, reset.
    if ((p_data->status != 0) && (p_cb->state == PROVISION_STATE_CONFIGURATION))
    {
        wiced_bt_mesh_db_node_t* p_node;

        p_cb->state = PROVISION_STATE_IDLE;
        clean_pending_op_queue(p_event->src);
        if ((p_node = mesh_find_node(p_mesh_db, p_event->src)) != NULL)
            mesh_reset_node(p_cb, p_node);
        return;
    }
    if (p_cb->store_config)
    {
        wiced_bt_mesh_db_node_model_app_bind_add(p_mesh_db, p_event->src, p_data->element_addr, p_data->company_id, p_data->model_id, p_data->app_key_idx);
        wiced_bt_mesh_db_store(p_mesh_db);
        p_cb->db_changed = WICED_TRUE;
    }
    start_next_op(p_cb);
}

void mesh_configure_model_sub_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_model_subscription_status_data_t *p_data)
{
    // Check that this is not reply to a retransmission
    pending_operation_t *p_op = p_cb->p_first;
    if ((p_op == NULL) || (p_op->p_event == NULL) || (p_op->p_event->dst != p_event->src) ||
        (p_op->operation != CONFIG_OPERATION_MODEL_SUBSCRIBE) ||
        (p_op->uu.model_sub.element_addr != p_data->element_addr) ||
        (p_op->uu.model_sub.company_id != p_data->company_id) ||
        (p_op->uu.model_sub.model_id != p_data->model_id) ||
        (p_op->uu.model_sub.addr[0] != (p_data->addr & 0xFF)) ||
        (p_op->uu.model_sub.addr[1] != ((p_data->addr >> 8) & 0xFF)))
    {
        Log("Ignored Model Sub Status from:%x status:%d Element addr:%04x Model ID:%x addr:%04x", p_event->src, p_data->status, p_data->element_addr, p_data->model_id, p_data->addr);
        return;
    }
    Log("Model Sub Status from:%x status:%d Element addr:%04x Model ID:%x addr:%04x", p_event->src, p_data->status, p_data->element_addr, p_data->model_id, p_data->addr);
    if (p_data->status != 0)
    {
        Log("sub status:%d\n", p_data->status);
    }
    switch (p_op->uu.model_sub.operation)
    {
    case OPERATION_DELETE_ALL:
        if (p_cb->store_config)
        {
            wiced_bt_mesh_db_node_model_sub_delete_all(p_mesh_db, p_data->element_addr, p_data->company_id, p_data->model_id);
            wiced_bt_mesh_db_store(p_mesh_db);
            p_cb->db_changed = WICED_TRUE;
        }
        break;

    case OPERATION_DELETE:
        if (p_cb->store_config)
        {
            //if we are deleting subscription of provisioner no need to update database
            if (p_data->addr != p_mesh_db->unicast_addr)
            {
                wiced_bt_mesh_db_node_model_sub_delete(p_mesh_db, p_data->element_addr, p_data->company_id, p_data->model_id, p_data->addr);
                wiced_bt_mesh_db_store(p_mesh_db);
                p_cb->db_changed = WICED_TRUE;
            }
        }
        break;

    case OPERATION_ADD:
        if (p_cb->store_config)
        {
            //if we are subscribing provisioner to a address, no need to update database
            if (p_data->addr != p_mesh_db->unicast_addr)
            {
                wiced_bt_mesh_db_node_model_sub_add(p_mesh_db, p_data->element_addr, p_data->company_id, p_data->model_id, p_data->addr);
                wiced_bt_mesh_db_store(p_mesh_db);
                p_cb->db_changed = WICED_TRUE;
            }
        }
        break;

    default:
        Log("sub bad op %d\n", p_op->uu.model_sub.operation);
        break;
    }
    start_next_op(p_cb);
}

void mesh_configure_model_pub_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_model_publication_status_data_t *p)
{
    // Check that this is not reply to a retransmission
    pending_operation_t *p_op = p_cb->p_first;
    if ((p_op == NULL) || (p_op->p_event == NULL) || (p_op->p_event->dst != p_event->src) ||
        (p_op->operation != CONFIG_OPERATION_MODEL_PUBLISH) ||
        (p_op->uu.model_pub.element_addr != p->element_addr))
    {
        Log("Ignored Model Pub Status from:%x status:%d Element:%x Pub addr:%04x Model ID:%x AppKeyIdx:%x TTL:%d Period:%d Count:%d Interval:%d Cred:%d", p_event->src, p->status, p->element_addr, p->publish_addr, p->model_id, p->app_key_idx,
            p->publish_ttl, p->publish_period, p->publish_retransmit_count, p->publish_retransmit_interval, p->credential_flag);
        return;
    }
    Log("Model Pub Status from:%x status:%d Element:%x Pub addr:%04x Model ID:%x AppKeyIdx:%x TTL:%d Period:%d Count:%d Interval:%d Cred:%d", p_event->src, p->status, p->element_addr, p->publish_addr, p->model_id, p->app_key_idx,
        p->publish_ttl, p->publish_period, p->publish_retransmit_count, p->publish_retransmit_interval, p->credential_flag);

    if (p->status != 0)
    {
        // ToDo ??
    }
    else
    {
        if (p_cb->store_config)
        {
            if (p->publish_addr != 0)
                wiced_bt_mesh_db_node_model_pub_add(p_mesh_db, p->element_addr, p->company_id, p->model_id, p->publish_addr, p->app_key_idx, p->publish_ttl, p->publish_period, p->publish_retransmit_count, p->publish_retransmit_interval, p->credential_flag);
            else
                wiced_bt_mesh_db_node_model_pub_delete(p_mesh_db, p->element_addr, p->company_id, p->model_id);
            wiced_bt_mesh_db_store(p_mesh_db);
            p_cb->db_changed = WICED_TRUE;
        }
    }
    start_next_op(p_cb);
}

void mesh_configure_net_transmit_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_network_transmit_status_data_t *p_data)
{
    // Check that this is not reply to a retransmission
    Log("Net Transmit Status from:%x count:%d interval:%d", p_event->src, p_data->count, p_data->interval);
    pending_operation_t *p_op = p_cb->p_first;
    if ((p_op == NULL) || (p_op->p_event == NULL) || (p_op->p_event->dst != p_event->src) ||
        (p_op->operation != CONFIG_OPERATION_NET_TRANSMIT_SET))
    {
        Log("Ignored net transmit Status from:%x ", p_event->src);
        return;
    }
    if (p_cb->store_config)
    {
        wiced_bt_mesh_db_net_transmit_set(p_mesh_db, p_event->src, p_data->count, p_data->interval);
        wiced_bt_mesh_db_store(p_mesh_db);
        p_cb->db_changed = WICED_TRUE;
    }
    start_next_op(p_cb);
}

static void mesh_configure_default_ttl_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_default_ttl_status_data_t *p_data)
{
    // Check that this is not reply to a retransmission
    pending_operation_t *p_op = p_cb->p_first;
    if ((p_op == NULL) || (p_op->p_event == NULL) || (p_op->p_event->dst != p_event->src) ||
        (p_op->operation != CONFIG_OPERATION_DEFAULT_TTL_SET))
    {
        Log("Ignored Default TTL Status from:%x TTL:%d Received TTL:%d", p_event->src, p_data->ttl, p_data->received_ttl);
        return;
    }
    wiced_bt_mesh_db_node_t *node = mesh_find_node(p_mesh_db, p_event->src);

    Log("Default TTL Status from:%x TTL:%d Received TTL:%d", p_event->src, p_data->ttl, p_data->received_ttl);

    if (p_cb->store_config)
    {
        wiced_bt_mesh_db_default_ttl_set(p_mesh_db, p_event->src, p_data->ttl);
        wiced_bt_mesh_db_store(p_mesh_db);
        p_cb->db_changed = WICED_TRUE;
    }
    if (node != NULL)
        node->num_hops = p_data->ttl - p_data->received_ttl;

    start_next_op(p_cb);
}

void mesh_configure_relay_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_relay_status_data_t *p_data)
{
    // Check that this is not reply to a retransmission
    pending_operation_t *p_op = p_cb->p_first;
    if ((p_op == NULL) || (p_op->p_event == NULL) || (p_op->p_event->dst != p_event->src) ||
        (p_op->operation != CONFIG_OPERATION_RELAY_SET))
    {
        Log("Ignored Relay Status from:%x state:%d count:%d interval:%d", p_event->src, p_data->state, p_data->retransmit_count, p_data->retransmit_interval);
        return;
    }
    Log("Relay Status from:%x state:%d count:%d interval:%d", p_event->src, p_data->state, p_data->retransmit_count, p_data->retransmit_interval);

    if (p_cb->store_config)
    {
        wiced_bt_mesh_db_relay_set(p_mesh_db, p_event->src, p_data->state, p_data->retransmit_count, p_data->retransmit_interval);
        wiced_bt_mesh_db_store(p_mesh_db);
        p_cb->db_changed = WICED_TRUE;
    }
    start_next_op(p_cb);
}

void mesh_configure_friend_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_friend_status_data_t *p_data)
{
    // Check that this is not reply to a retransmission
    pending_operation_t *p_op = p_cb->p_first;
    if ((p_op == NULL) || (p_op->p_event == NULL) || (p_op->p_event->dst != p_event->src) ||
        (p_op->operation != CONFIG_OPERATION_FRIEND_SET))
    {
        Log("Ignored Friend Status from:%x state:%d count:%d interval:%d", p_event->src, p_data->state);
        return;
    }
    Log("Friend Status from:%x state:%d", p_event->src, p_data->state);

    if (p_cb->store_config)
    {
        wiced_bt_mesh_db_friend_set(p_mesh_db, p_event->src, p_data->state);
        wiced_bt_mesh_db_store(p_mesh_db);
        p_cb->db_changed = WICED_TRUE;
    }
    start_next_op(p_cb);
}

void mesh_configure_gatt_proxy_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_gatt_proxy_status_data_t *p_data)
{
    // Check that this is not reply to a retransmission
    pending_operation_t *p_op = p_cb->p_first;
    if ((p_op == NULL) || (p_op->p_event == NULL) || (p_op->p_event->dst != p_event->src) ||
        (p_op->operation != CONFIG_OPERATION_PROXY_SET))
    {
        Log("Ignored GATT Proxy Status from:%x state:%d", p_event->src, p_data->state);
        return;
    }
    Log("GATT Proxy Status from:%x state:%d", p_event->src, p_data->state);

    if (p_cb->store_config)
    {
        wiced_bt_mesh_db_gatt_proxy_set(p_mesh_db, p_event->src, p_data->state);
        wiced_bt_mesh_db_store(p_mesh_db);
        p_cb->db_changed = WICED_TRUE;
    }
    start_next_op(p_cb);
}

void mesh_configure_beacon_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_beacon_status_data_t *p_data)
{
    // Check that this is not reply to a retransmission
    pending_operation_t *p_op = p_cb->p_first;
    if ((p_op == NULL) || (p_op->p_event == NULL) || (p_op->p_event->dst != p_event->src) ||
        (p_op->operation != CONFIG_OPERATION_NET_BEACON_SET))
    {
        Log("Ignored Beacon Status from:%x state:%d", p_event->src, p_data->state);
        return;
    }
    Log("Beacon Status from:%x state:%d", p_event->src, p_data->state);

    if (p_cb->store_config)
    {
        wiced_bt_mesh_db_beacon_set(p_mesh_db, p_event->src, p_data->state);
        wiced_bt_mesh_db_store(p_mesh_db);
        p_cb->db_changed = WICED_TRUE;
    }
    start_next_op(p_cb);
}

void mesh_configure_filter_status(mesh_provision_cb_t* p_cb, wiced_bt_mesh_event_t* p_event, wiced_bt_mesh_proxy_filter_status_data_t* p_data)
{
    // Check that this is not reply to a retransmission
    pending_operation_t* p_op = p_cb->p_first;

    if ((p_op == NULL) || (p_op->p_event == NULL))
        return;

    Log("Filter Status from:%x", p_event->src);

    if ((p_op->operation == CONFIG_OPERATION_FILTER_ADD) || (p_op->operation == CONFIG_OPERATION_FILTER_DEL))
    {
        start_next_op(p_cb);

        // If we receive filter status during configuration, that means that we configured new device as a Proxy
        // need to notify the client that connection to the network is now up
        p_cb->proxy_addr = p_event->src;
        p_cb->proxy_conn_id = 1;
        p_cb->over_gatt = 1;

        if (p_cb->p_connect_status != NULL)
            p_cb->p_connect_status(1, p_cb->proxy_conn_id, p_cb->proxy_addr, p_cb->over_gatt);
    }
    else
    {
        Log("Ignored Filter Status from:%x", p_event->src);
    }
}

void mesh_default_trans_time_status(wiced_bt_mesh_event_t *p_event, void *p)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    wiced_bt_mesh_default_transition_time_data_t *p_data = (wiced_bt_mesh_default_transition_time_data_t *)p;

    Log("Def Trans Time Status from:%x AppKeyIdx:%x Idx:%d Time:%d", p_event->src, p_event->app_key_idx, p_event->element_idx, p_data->time);

    if (p_cb->state == PROVISION_STATE_CONFIGURATION)
    {
        pending_operation_t *p_op = p_cb->p_first;
        if ((p_op == NULL) || (p_op->p_event == NULL) || (p_op->p_event->dst != p_event->src) ||
            (p_op->operation != CONFIG_OPERATION_DEF_TRANS_TIME))
        {
            Log("Def Trans Time Status from:%x AppKeyIdx:%x idx:%d Time:%d", p_event->src, p_event->app_key_idx, p_event->element_idx, p_data->time);
            return;
        }
        start_next_op(p_cb);
    }
    else
    {
        wiced_bt_mesh_release_event(p_event);
    }
}


void mesh_process_sensor_descriptor_status(uint16_t addr, wiced_bt_mesh_sensor_descriptor_status_data_t *ptr)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    wiced_bt_mesh_db_app_key_t *app_setup_key;
    pending_operation_t *p_op = p_cb->p_first;
    int i;

    if ((p_op == NULL) || (p_op->p_event == NULL) || (p_op->p_event->dst != addr) ||
        (p_op->operation != CONFIG_OPERATION_SENSOR_DESC_GET))
    {
        Log("Ignored Sensor descriptor Status from:%x ", addr);
        return;
    }

    if (p_cb->store_config)
    {
        wiced_bt_mesh_db_sensor_descriptor_add(p_mesh_db, addr, ptr);
        wiced_bt_mesh_db_store(p_mesh_db);
        p_cb->db_changed = WICED_TRUE;
    }
    // Schedule reading of the all settings that exists on each sensor.
    for (i = 0; i < ptr->num_descriptors; i++)
    {
        if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
        {
            app_setup_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Setup");
            memset(p_op, 0, sizeof(pending_operation_t));
            p_op->operation = CONFIG_OPERATION_SENSOR_SETTINGS_GET;
            p_op->p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_CLNT, addr, app_setup_key->index);

            //get all settings of sensor
            p_op->uu.sensor_get.property_id = ptr->descriptor_list[i].property_id;
            configure_pending_operation_queue(p_cb, p_op);
        }
        if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
        {
            app_setup_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Setup");
            memset(p_op, 0, sizeof(pending_operation_t));
            p_op->operation = CONFIG_OPERATION_SENSOR_CADENCE_GET;
            p_op->p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_CLNT, addr, app_setup_key->index);

            //get all settings of sensor
            p_op->uu.sensor_get.property_id = ptr->descriptor_list[i].property_id;
            configure_pending_operation_queue(p_cb, p_op);
        }
    }
    start_next_op(p_cb);
}


void mesh_configure_disconnecting_link_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_connect_status_data_t *p_data)
{
    if (p_cb->p_connect_status != NULL)
        p_cb->p_connect_status(p_data->connected, p_data->provisioner_addr, 0, p_data->over_gatt);

    Log("Configure Disconnecting Link Status provisioner:%x addr:%04x connected:%x over_gatt:%d", p_data->provisioner_addr, p_data->addr, p_data->connected, p_data->over_gatt);

    p_cb->state = PROVISION_STATE_IDLE;
}

void mesh_key_refresh_phase2_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_key_refresh_phase_status_data_t *p_data)
{
    // Check that this is not reply to a retransmission
    pending_operation_t *p_op = p_cb->p_first;
    if ((p_op == NULL) || (p_op->p_event == NULL) || (p_op->p_event->dst != p_event->src) ||
        (p_op->operation != CONFIG_OPERATION_KR_PHASE_SET) ||
        (p_op->uu.kr_phase_set.net_key_idx != p_data->net_key_idx))
    {
        Log("Ignored Key Refresh Phase2 Status from:0x%04x status:%d NetKey Index:%x", p_event->src, p_data->status, p_data->net_key_idx);
        return;
    }
    Log("Key Refresh Phase2 Status from:0x%04x status:%d NetKey Index:%x", p_event->src, p_data->status, p_data->net_key_idx);

    if (p_data->status != 0)
    {
        // ToDo ??
    }
    if ((p_data->status == 0) && p_cb->store_config)
    {
        wiced_bt_mesh_db_node_net_key_update(p_mesh_db, p_event->src, p_data->net_key_idx, WICED_BT_MESH_KEY_REFRESH_PHASE_SECOND);
        wiced_bt_mesh_db_store(p_mesh_db);
        p_cb->db_changed = WICED_TRUE;
    }
    start_next_op(p_cb);

    if (p_cb->p_first == NULL)
        mesh_key_refresh_phase2_completed(p_cb, find_net_key(p_mesh_db, p_data->net_key_idx));
}

void mesh_key_refresh_phase3_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_key_refresh_phase_status_data_t *p_data)
{
    // Check that this is not reply to a retransmission
    pending_operation_t *p_op = p_cb->p_first;
    if ((p_op == NULL) || (p_op->p_event == NULL) || (p_op->p_event->dst != p_event->src) ||
        (p_op->operation != CONFIG_OPERATION_KR_PHASE_SET) ||
        (p_op->uu.kr_phase_set.net_key_idx != p_data->net_key_idx))
    {
        Log("Ignored Key Refresh Phase3 Status from:0x%04x status:%d NetKey Index:%x", p_event->src, p_data->status, p_data->net_key_idx);
        return;
    }
    Log("Key Refresh Phase3 Status from:0x%04x status:%d NetKey Index:%x", p_event->src, p_data->status, p_data->net_key_idx);

    if (p_data->status != 0)
    {
        // ToDo ??
    }
    if ((p_data->status == 0) && p_cb->store_config)
    {
        wiced_bt_mesh_db_node_net_key_update(p_mesh_db, p_event->src, p_data->net_key_idx, WICED_BT_MESH_KEY_REFRESH_PHASE_THIRD);
        wiced_bt_mesh_db_store(p_mesh_db);
        p_cb->db_changed = WICED_TRUE;
    }
    start_next_op(p_cb);

    if (p_cb->p_first == NULL)
    {
        mesh_key_refresh_phase3_completed(p_cb, find_net_key(p_mesh_db, p_data->net_key_idx));
    }
}

/*
 * Perform factory reset of the device.
 */
uint8_t mesh_client_reset_device(const char *component_name)
{
    wiced_bt_mesh_db_node_t *node;
    mesh_provision_cb_t *p_cb = &provision_cb;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    node = wiced_bt_mesh_db_node_get_by_element_name(p_mesh_db, component_name);
    if (node == NULL)
    {
        Log("Device not found:%s\n", component_name);
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (node->unicast_address == p_cb->unicast_addr)
    {
        Log("Cannot delete local device\n");
        return MESH_CLIENT_ERR_INVALID_ARGS;
    }
    if (node->blocked)
    {
        Log("Already removed\n");
        return MESH_CLIENT_SUCCESS;
    }
    if (p_cb->state != PROVISION_STATE_IDLE)
    {
        Log("invalid state:%d\n", p_cb->state);
        return MESH_CLIENT_ERR_INVALID_STATE;
    }
    return mesh_reset_node(p_cb, node);
}

uint8_t mesh_reset_node(mesh_provision_cb_t *p_cb, wiced_bt_mesh_db_node_t *node)
{
    pending_operation_t *p_op;
    int net_key_idx, node_net_key_idx;
    int i;

    for (i = 0; i < node->num_elements; i++)
        clean_pending_op_queue(node->unicast_address + i);

    p_cb->retries = 0;

    // even if device does not reply, we need to mark device as blocked, so that we do not give it new keys during key refresh.
    node->blocked = 1;
    wiced_bt_mesh_db_store(p_mesh_db);
    p_cb->db_changed = WICED_TRUE;

    // start sending the Node Reset to the device.
    if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) == NULL)
        return MESH_CLIENT_ERR_NO_MEMORY;

    p_op->operation = CONFIG_OPERATION_NODE_RESET;
    p_op->p_event = mesh_configure_create_event(node->unicast_address, (node->unicast_address != p_cb->unicast_addr));
    configure_pending_operation_queue(p_cb, p_op);

    // now we need to execute a key refresh if there are any other nodes which were using any keys
    // that are preseent on the device.
    // Go through the list of all network keys that are present on the node
    for (net_key_idx = 0; net_key_idx < p_mesh_db->num_net_keys; net_key_idx++)
    {
        for (node_net_key_idx = 0; node_net_key_idx < node->num_net_keys; node_net_key_idx++)
        {
            if (p_mesh_db->net_key[net_key_idx].index != node->net_key[node_net_key_idx].index)
                continue;

            if (p_mesh_db->net_key[net_key_idx].phase == WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL)
            {
                p_cb->state = PROVISION_STATE_KEY_REFRESH_1;

                // generate new keys and schedule key updates for all nodes.  At least local device needs to be updated.
                mesh_key_refresh_update_keys(p_cb, &p_mesh_db->net_key[net_key_idx]);
            }
            mesh_key_refresh_continue(p_cb, &p_mesh_db->net_key[net_key_idx]);
        }
    }
    if (p_cb->p_first != NULL)
        configure_execute_pending_operation(p_cb);
    else
    {
        p_cb->db_changed = WICED_FALSE;

        if (provision_cb.p_database_changed != NULL)
            provision_cb.p_database_changed(p_mesh_db->name);
    }
    return MESH_CLIENT_SUCCESS;
}

/*
 * start key refresh for the specified net key
 */
void mesh_key_refresh_update_keys(mesh_provision_cb_t *p_cb, wiced_bt_mesh_db_net_key_t *net_key)
{
    int app_key_idx;

    net_key->phase = WICED_BT_MESH_KEY_REFRESH_PHASE_FIRST;
    memcpy(net_key->old_key, net_key->key, WICED_MESH_DB_KEY_SIZE);
    rand128(net_key->key);

    // Go through the application keys that are bound to the network key being updated
    for (app_key_idx = 0; app_key_idx < p_mesh_db->num_app_keys; app_key_idx++)
    {
        if (p_mesh_db->app_key[app_key_idx].bound_net_key_index != net_key->index)
            continue;

        // the key p_mesh_db->app_key[app_key_idx] need to be updated
        memcpy(p_mesh_db->app_key[app_key_idx].old_key, p_mesh_db->app_key[app_key_idx].key, WICED_MESH_DB_KEY_SIZE);
        rand128(p_mesh_db->app_key[app_key_idx].key);
    }
    wiced_bt_mesh_db_store(p_mesh_db);
    p_cb->db_changed = WICED_TRUE;
}

void mesh_key_refresh_continue(mesh_provision_cb_t *p_cb, wiced_bt_mesh_db_net_key_t *net_key)
{
    int num_operations_scheduled = 0;

    if (net_key->phase == WICED_BT_MESH_KEY_REFRESH_PHASE_FIRST)
    {
        p_cb->state = PROVISION_STATE_KEY_REFRESH_1;
        // key refresh has probably started by removing another node
        // new keys have been already generated and some operations may have been already completed
        if ((num_operations_scheduled = mesh_client_key_refresh_phase1_continue(p_cb, net_key)) == 0)
        {
            // phase1 is completed. Transition to phase2
            net_key->phase = WICED_BT_MESH_KEY_REFRESH_PHASE_SECOND;
        }
    }
    if (net_key->phase == WICED_BT_MESH_KEY_REFRESH_PHASE_SECOND)
    {
        if ((num_operations_scheduled = mesh_client_transition_next_key_refresh_phase(p_cb, net_key, WICED_BT_MESH_KEY_REFRESH_PHASE_SECOND)) == 0)
        {
            // phase2 is completed. Transition to phase3
            net_key->phase = WICED_BT_MESH_KEY_REFRESH_PHASE_THIRD;
        }
    }
    if (net_key->phase == WICED_BT_MESH_KEY_REFRESH_PHASE_THIRD)
    {
        if ((num_operations_scheduled = mesh_client_transition_next_key_refresh_phase(p_cb, net_key, WICED_BT_MESH_KEY_REFRESH_PHASE_THIRD)) == 0)
        {
            // phase3 is completed. Done.
            mesh_key_refresh_phase3_completed(p_cb, net_key);
        }
    }
}

/*
 * This function should be called when network key is in phase 1 and there is a chance that we can schedule a phase1 operations.
 * Return number of operations scheduled. -1 if no memory.
 *
 */
int mesh_client_key_refresh_phase1_continue(mesh_provision_cb_t *p_cb, wiced_bt_mesh_db_net_key_t *net_key)
{
    int node_net_key_idx;
    int app_key_idx, node_app_key_idx;
    int node_idx;
    uint16_t dst;
    pending_operation_t *p_op;
    uint8_t num_operations_scheduled = 0;

    Log("KR phase1 continue key phase:%d\n", net_key->phase);

    // For every node in the network, check if the same key is present.  If true, schedule the update.
    for (node_idx = 0; node_idx < p_mesh_db->num_nodes; node_idx++)
    {
        if (p_mesh_db->node[node_idx].blocked)
            continue;

        for (node_net_key_idx = 0; node_net_key_idx < p_mesh_db->node[node_idx].num_net_keys; node_net_key_idx++)
        {
            if (net_key->index == p_mesh_db->node[node_idx].net_key[node_net_key_idx].index)
            {
                // This can be a key refresh restart and key could have been delivered already
                if ((net_key->phase == WICED_BT_MESH_KEY_REFRESH_PHASE_FIRST) &&
                    (p_mesh_db->node[node_idx].net_key[node_net_key_idx].phase == WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL))
                {
                    if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) == NULL)
                        return -1;
                    dst = p_mesh_db->node[node_idx].unicast_address;
                    p_op->operation = CONFIG_OPERATION_NET_KEY_UPDATE;
                    p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
                    p_op->uu.net_key_change.operation = OPERATION_UPDATE;
                    p_op->uu.net_key_change.net_key_idx = net_key->index;
                    memcpy(p_op->uu.net_key_change.net_key, net_key->key, sizeof(p_op->uu.net_key_change.net_key));
                    configure_pending_operation_queue(p_cb, p_op);
                    num_operations_scheduled++;
                }
            }
        }
    }

    // Go through the application keys that are bound to the network key being updated
    for (app_key_idx = 0; app_key_idx < p_mesh_db->num_app_keys; app_key_idx++)
    {
        if (p_mesh_db->app_key[app_key_idx].bound_net_key_index != net_key->index)
            continue;

        // For every node in the network, check if the key with this app_key index is present.  If true, schedule the update.
        for (node_idx = 0; node_idx < p_mesh_db->num_nodes; node_idx++)
        {
            if (p_mesh_db->node[node_idx].blocked)
                continue;

            for (node_app_key_idx = 0; node_app_key_idx < p_mesh_db->node[node_idx].num_app_keys; node_app_key_idx++)
            {
                if (p_mesh_db->app_key[app_key_idx].index == p_mesh_db->node[node_idx].app_key[node_app_key_idx].index)
                {
                    // This can be a key refresh restart and key could have been delivered already
                    if (p_mesh_db->node[node_idx].app_key[node_app_key_idx].phase == WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL)
                    {
                        if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) == NULL)
                            return -1;

                        dst = p_mesh_db->node[node_idx].unicast_address;
                        p_op->operation = CONFIG_OPERATION_APP_KEY_UPDATE;
                        p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
                        p_op->uu.app_key_change.operation = OPERATION_UPDATE;
                        p_op->uu.app_key_change.net_key_idx = net_key->index;
                        p_op->uu.app_key_change.app_key_idx = p_mesh_db->app_key[app_key_idx].index;
                        memcpy(p_op->uu.app_key_change.app_key, p_mesh_db->app_key[app_key_idx].key, sizeof(p_op->uu.app_key_change.app_key));
                        configure_pending_operation_queue(p_cb, p_op);
                        num_operations_scheduled++;
                    }
                    break;
                }
            }
        }
    }
    return num_operations_scheduled;
}

/*
 * Transition to key refresh phase 2 or 3.
 */
int mesh_client_transition_next_key_refresh_phase(mesh_provision_cb_t *p_cb, wiced_bt_mesh_db_net_key_t *net_key, uint8_t transition)
{
    pending_operation_t *p_op;
    uint16_t dst;
    int node_idx;
    int node_net_key_idx;
    int num_operations_scheduled = 0;

    Log("KR transition key_phase %d transition:%d\n", net_key->phase, transition);

    //// if this key is not being refreshed go to next one
    //if (net_key->phase != (transition - 1))
    //{
    //    Log("should only be called in phase %d\n");
    //    return - 1;
    //}
    net_key->phase = transition;
    wiced_bt_mesh_db_store(p_mesh_db);
    p_cb->db_changed = WICED_TRUE;

    p_cb->state = transition == WICED_BT_MESH_KEY_REFRESH_TRANSITION_PHASE2 ? PROVISION_STATE_KEY_REFRESH_2 : PROVISION_STATE_KEY_REFRESH_3;

    // For every other node in the network, check if the same key is present.  If true, schedule the update.
    for (node_idx = 0; node_idx < p_mesh_db->num_nodes; node_idx++)
    {
        if (p_mesh_db->node[node_idx].blocked)
            continue;

        for (node_net_key_idx = 0; node_net_key_idx < p_mesh_db->node[node_idx].num_net_keys; node_net_key_idx++)
        {
            if (net_key->index != p_mesh_db->node[node_idx].net_key[node_net_key_idx].index)
                continue;

            if (p_mesh_db->node[node_idx].net_key[node_net_key_idx].phase == transition)
                continue;

            if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) == NULL)
                return -1;

            dst = p_mesh_db->node[node_idx].unicast_address;
            p_op->operation = CONFIG_OPERATION_KR_PHASE_SET;
            p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
            p_op->uu.kr_phase_set.net_key_idx = net_key->index;
            p_op->uu.kr_phase_set.transition = transition;
            configure_pending_operation_queue(p_cb, p_op);
            num_operations_scheduled++;
        }
    }
    return num_operations_scheduled;
}

wiced_bt_mesh_db_node_t *mesh_find_node(wiced_bt_mesh_db_mesh_t *p_mesh, uint16_t unicast_address)
{
    int i;

    if (p_mesh->node == NULL)
        return NULL;

    for (i = 0; i < p_mesh->num_nodes; i++)
    {
        if (p_mesh->node[i].unicast_address == unicast_address)
            return &p_mesh->node[i];
    }
    return NULL;
}

wiced_bt_mesh_db_node_t *mesh_find_node_by_uuid(wiced_bt_mesh_db_mesh_t *p_mesh, uint8_t *uuid)
{
    int i;

    if (p_mesh->node == NULL)
        return NULL;

    for (i = 0; i < p_mesh->num_nodes; i++)
    {
        if (memcmp(p_mesh->node[i].uuid, uuid, sizeof(p_mesh->node[i].uuid)) == 0)
            return &p_mesh->node[i];
    }
    return NULL;
}

wiced_bt_mesh_db_net_key_t *find_net_key(wiced_bt_mesh_db_mesh_t *p_mesh, uint16_t net_key_idx)
{
    int i;

    if (p_mesh->net_key == NULL)
        return NULL;

    for (i = 0; i < p_mesh->num_net_keys; i++)
    {
        if (p_mesh->net_key[i].index == net_key_idx)
            return &p_mesh->net_key[i];
    }
    return NULL;
}

void configure_pending_operation_queue(mesh_provision_cb_t *p_cb, pending_operation_t *p_op)
{
    pending_operation_t *p_cur;

    p_op->p_next = NULL;
    if (p_cb->p_first == NULL)
    {
        p_cb->p_first = p_op;
    }
    else
    {
        for (p_cur = p_cb->p_first; p_cur->p_next != NULL; p_cur = p_cur->p_next)
            ;
        p_cur->p_next = p_op;
    }
}

pending_operation_t *configure_pending_operation_dequeue(mesh_provision_cb_t *p_cb)
{
    pending_operation_t *p_op;

    if (p_cb->p_first == NULL)
        return NULL;
    p_op = p_cb->p_first;
    p_cb->p_first = p_cb->p_first->p_next;
    return p_op;
}

/*
 * this function schedules all operations required to configure local device
 */
void configure_queue_local_device_operations(mesh_provision_cb_t *p_cb)
{
    uint32_t i, j;
    uint8_t element_idx = 0;
    uint8_t *p_comp_data;
    uint16_t features;
    uint16_t comp_data_len = p_cb->p_local_composition_data->data_len;
    pending_operation_t *p_op;

    // first net key was added during provisiong.  Need to refresh if the phase is not 0
    for (i = 0; i < wiced_bt_mesh_db_num_net_keys(p_mesh_db); i++)
    {
        wiced_bt_mesh_db_net_key_t *net_key = wiced_bt_mesh_db_net_key_get(p_mesh_db, i);
        if (net_key == NULL)
            continue;

        if (i != 0)
        {
            if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
            {
                p_op->operation = CONFIG_OPERATION_NET_KEY_UPDATE;
                p_op->p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT, p_cb->unicast_addr, 0xFFFF);
                p_op->uu.net_key_change.operation = OPERATION_ADD;
                p_op->uu.net_key_change.net_key_idx = net_key->index;
                memcpy(p_op->uu.net_key_change.net_key, net_key->phase == WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL ? net_key->key : net_key->old_key, sizeof(net_key->key));
                configure_pending_operation_queue(p_cb, p_op);
            }
        }
        if (net_key->phase != WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL)
        {
            if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
            {
                p_op->operation = CONFIG_OPERATION_NET_KEY_UPDATE;
                p_op->p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT, p_cb->unicast_addr, 0xFFFF);
                p_op->uu.net_key_change.operation = OPERATION_UPDATE;
                p_op->uu.net_key_change.net_key_idx = net_key->index;
                memcpy(p_op->uu.net_key_change.net_key, net_key->key, sizeof(net_key->key));
                configure_pending_operation_queue(p_cb, p_op);
            }
            if (net_key->phase != WICED_BT_MESH_KEY_REFRESH_PHASE_FIRST)
            {
                if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
                {
                    p_op->operation = CONFIG_OPERATION_KR_PHASE_SET;
                    p_op->p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT, p_cb->unicast_addr, 0xFFFF);
                    p_op->uu.kr_phase_set.net_key_idx = net_key->index;
                    p_op->uu.kr_phase_set.transition = net_key->phase;
                    configure_pending_operation_queue(p_cb, p_op);
                }
            }
        }
    }

    // add all app keys and bind all app keys to all models
    // currently we assume that we support all hsl_client models, although it would
    // be better to get models from the local_composition_data
    for (i = 0; i < wiced_bt_mesh_db_num_app_keys(p_mesh_db); i++)
    {
        wiced_bt_mesh_db_app_key_t *app_key = wiced_bt_mesh_db_app_key_get(p_mesh_db, i);
        wiced_bt_mesh_db_net_key_t *net_key = wiced_bt_mesh_db_find_bound_net_key(p_mesh_db, app_key);
        if ((app_key == NULL) || (net_key == NULL))
            continue;

        if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
        {
            p_op->operation = CONFIG_OPERATION_APP_KEY_UPDATE;
            p_op->p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT, p_cb->unicast_addr, 0xFFFF);
            p_op->uu.app_key_change.operation = OPERATION_ADD;
            p_op->uu.app_key_change.app_key_idx = app_key->index;
            p_op->uu.app_key_change.net_key_idx = app_key->bound_net_key_index;
            memcpy(p_op->uu.app_key_change.app_key, net_key->phase == WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL ? app_key->key : app_key->old_key, sizeof(app_key->key));
            configure_pending_operation_queue(p_cb, p_op);

            if (net_key->phase != WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL)
            {
                if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
                {
                    p_op->operation = CONFIG_OPERATION_APP_KEY_UPDATE;
                    p_op->p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT, p_cb->unicast_addr, 0xFFFF);
                    p_op->uu.app_key_change.operation = OPERATION_UPDATE;
                    p_op->uu.app_key_change.app_key_idx = app_key->index;
                    p_op->uu.app_key_change.net_key_idx = app_key->bound_net_key_index;
                    memcpy(p_op->uu.app_key_change.app_key, app_key->key, sizeof(app_key->key));
                    configure_pending_operation_queue(p_cb, p_op);
                }
            }
        }
        element_idx = 0;

        features = p_cb->p_local_composition_data->data[8] + (p_cb->p_local_composition_data->data[9] << 8);

        comp_data_len = p_cb->p_local_composition_data->data_len - 10;
        p_comp_data = p_cb->p_local_composition_data->data + 10;
        while (comp_data_len)
        {
            if (comp_data_len < 4)
                break;

            uint8_t num_models = p_comp_data[2];
            uint8_t num_vs_models = p_comp_data[3];

            p_comp_data += 4;
            comp_data_len -= 4;

            if (comp_data_len < 2 * num_models + 4 * num_vs_models)
                break;

            for (j = 0; j < num_models; j++)
            {
                uint16_t model_id = p_comp_data[0] + (p_comp_data[1] << 8);
                if ((model_id != WICED_BT_MESH_CORE_MODEL_ID_CONFIG_SRV) &&
                    (model_id != WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT) &&
                    (model_id != WICED_BT_MESH_CORE_MODEL_ID_REMOTE_PROVISION_SRV) &&
                    (model_id != WICED_BT_MESH_CORE_MODEL_ID_REMOTE_PROVISION_CLNT))
                {
                    model_app_bind(p_cb, WICED_TRUE, p_cb->unicast_addr + element_idx, MESH_COMPANY_ID_BT_SIG, model_id, app_key->index);

#if SUBSCRIBE_LOCAL_MODELS_TO_ALL_GROUPS
                    // subscribe all client models to receive all messages for all groups.  When a device is provisioned
                    // it is configured to publish status data to the group
                    for (int k = 0; k < p_mesh_db->num_groups; k++)
                    {
                        if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
                        {
                            memset(p_op, 0, sizeof(pending_operation_t));
                            p_op->operation = CONFIG_OPERATION_MODEL_SUBSCRIBE;
                            p_op->p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT, p_cb->unicast_addr, 0xFFFF);
                            p_op->uu.model_sub.operation = OPERATION_ADD;
                            p_op->uu.model_sub.element_addr = p_cb->unicast_addr + element_idx;
                            p_op->uu.model_sub.company_id = MESH_COMPANY_ID_BT_SIG;
                            p_op->uu.model_sub.model_id = model_id;
                            p_op->uu.model_sub.addr[0] = p_mesh_db->group[k].addr & 0xff;
                            p_op->uu.model_sub.addr[1] = (p_mesh_db->group[k].addr >> 8) & 0xff;
                            configure_pending_operation_queue(p_cb, p_op);
                        }
                    }
#endif
                }
                p_comp_data += 2;
                comp_data_len -= 2;
            }
            for (j = 0; j < num_vs_models; j++)
            {
                model_app_bind(p_cb, WICED_TRUE, p_cb->unicast_addr + element_idx, p_comp_data[0] + (p_comp_data[1] << 8), p_comp_data[2] + (p_comp_data[3] << 8), app_key->index);

                p_comp_data += 4;
                comp_data_len -= 4;
            }
            element_idx++;
        }
    }

    if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
    {
        // wiced_bt_mesh_config_network_transmit_set_data_t
        p_op->operation = CONFIG_OPERATION_NET_TRANSMIT_SET;
        p_op->p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT, p_cb->unicast_addr, 0xFFFF);
        p_op->uu.net_transmit_set.count = LOCAL_DEVICE_NET_TRANSMIT_COUNT;
        p_op->uu.net_transmit_set.interval = LOCAL_DEVICE_NET_TRANSMIT_INTERVAL;
        configure_pending_operation_queue(p_cb, p_op);
    }
    if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
    {
        //wiced_bt_mesh_config_default_ttl_set_data_t
        p_op->operation = CONFIG_OPERATION_DEFAULT_TTL_SET;
        p_op->p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT, p_cb->unicast_addr, 0xFFFF);
        p_op->uu.default_ttl_set.ttl = LOCAL_DEVICE_TTL;
        configure_pending_operation_queue(p_cb, p_op);
    }
    if (((features & FOUNDATION_FEATURE_BIT_RELAY) == FOUNDATION_FEATURE_BIT_RELAY) &&
        ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL))
    {
        p_op->operation = CONFIG_OPERATION_RELAY_SET;
        p_op->p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT, p_cb->unicast_addr, 0xFFFF);
        p_op->uu.relay_set.state = 0;
        p_op->uu.relay_set.retransmit_count = 0;
        p_op->uu.relay_set.retransmit_interval = 0;
        configure_pending_operation_queue(p_cb, p_op);
    }
    if (((features & FOUNDATION_FEATURE_BIT_PROXY) == FOUNDATION_FEATURE_BIT_PROXY) &&
        ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL))
    {
        //wiced_bt_mesh_config_gatt_proxy_set_data_t
        p_op->operation = CONFIG_OPERATION_PROXY_SET;
        p_op->p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT, p_cb->unicast_addr, 0xFFFF);
        p_op->uu.proxy_set.state = 0;
        configure_pending_operation_queue(p_cb, p_op);
    }
    if (((features & FOUNDATION_FEATURE_BIT_FRIEND) == FOUNDATION_FEATURE_BIT_FRIEND) &&
        ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL))
    {
        //wiced_bt_mesh_config_friend_set_data_t
        p_op->operation = CONFIG_OPERATION_FRIEND_SET;
        p_op->p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT, p_cb->unicast_addr, 0xFFFF);
        p_op->uu.friend_set.state = 0;
        configure_pending_operation_queue(p_cb, p_op);
    }
    if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
    {
        p_op->operation = CONFIG_OPERATION_NET_BEACON_SET;
        p_op->p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT, p_cb->unicast_addr, 0xFFFF);
        p_op->uu.beacon_set.state = 0;
        configure_pending_operation_queue(p_cb, p_op);
    }
}

void app_key_add(mesh_provision_cb_t *p_cb, uint16_t addr, wiced_bt_mesh_db_net_key_t *net_key, wiced_bt_mesh_db_app_key_t *app_key)
{
    pending_operation_t *p_op;

    if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) == NULL)
        return;

    p_op->operation = CONFIG_OPERATION_APP_KEY_UPDATE;
    p_op->p_event = mesh_configure_create_event(addr, (addr != p_cb->unicast_addr));
    p_op->uu.app_key_change.operation = OPERATION_ADD;
    p_op->uu.app_key_change.app_key_idx = app_key->index;
    p_op->uu.app_key_change.net_key_idx = app_key->bound_net_key_index;
    memcpy(p_op->uu.app_key_change.app_key, (net_key->phase == WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL) ? app_key->key : app_key->old_key, sizeof(app_key->key));
    configure_pending_operation_queue(p_cb, p_op);

    if (net_key->phase != WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL)
    {
        if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
        {
            p_op->operation = CONFIG_OPERATION_APP_KEY_UPDATE;
            p_op->p_event = mesh_configure_create_event(addr, (addr != p_cb->unicast_addr));
            p_op->uu.app_key_change.operation = OPERATION_UPDATE;
            p_op->uu.app_key_change.app_key_idx = app_key->index;
            p_op->uu.app_key_change.net_key_idx = app_key->bound_net_key_index;
            memcpy(p_op->uu.app_key_change.app_key, app_key->key, sizeof(app_key->key));
            configure_pending_operation_queue(p_cb, p_op);
        }
    }
}

void model_app_bind(mesh_provision_cb_t* p_cb, wiced_bool_t is_local,  uint16_t addr, uint16_t company_id, uint16_t model_id, uint16_t app_key_idx)
{
    pending_operation_t* p_op;

    if ((p_op = (pending_operation_t*)wiced_bt_get_buffer(sizeof(pending_operation_t))) == NULL)
        return;

    p_op->operation = CONFIG_OPERATION_MODEL_APP_BIND;
    p_op->uu.app_key_bind.operation = OPERATION_BIND;
    if (is_local)
        p_op->p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT, p_cb->unicast_addr, 0xFFFF);
    else
        p_op->p_event = mesh_configure_create_event(p_cb->addr, (p_cb->addr != p_cb->unicast_addr));
    p_op->uu.app_key_bind.element_addr = addr;
    p_op->uu.app_key_bind.company_id = company_id;
    p_op->uu.app_key_bind.model_id = model_id;
    p_op->uu.app_key_bind.app_key_idx = app_key_idx;
    configure_pending_operation_queue(p_cb, p_op);
}

/*
 * this function schedules all operations required to configure remote device
 */
void configure_queue_remote_device_operations(mesh_provision_cb_t *p_cb)
{
    uint32_t i = 0, j, k;
    uint8_t element_idx = 0;
    uint8_t *p_comp_data;
    uint16_t comp_data_len = p_cb->p_remote_composition_data->data_len;
    pending_operation_t *p_op;
    wiced_bt_mesh_db_net_key_t *net_key;
    wiced_bt_mesh_db_app_key_t *app_key = NULL, *app_key_setup, *p_app_key;
    wiced_bool_t default_trans_time_model_present;
    uint16_t features;
//    uint8_t component_type = get_component_type(p_mesh_db, p_cb->addr);

    uint16_t *p_group_list = get_group_list(p_cb->group_addr);
    wiced_bool_t app_key_added = WICED_FALSE, app_key_setup_added = WICED_FALSE;

    // start with network transmit parameters, so that if some device is manufactured with low ratransmission counts,
    // it will be fixed right away.
    if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
    {
        // wiced_bt_mesh_config_network_transmit_set_data_t
        p_op->operation = CONFIG_OPERATION_NET_TRANSMIT_SET;
        p_op->p_event = mesh_configure_create_event(p_cb->addr, (p_cb->addr != p_cb->unicast_addr));
        p_op->uu.net_transmit_set.count = p_cb->net_xmit_count;
        p_op->uu.net_transmit_set.interval = p_cb->net_xmit_interval;
        configure_pending_operation_queue(p_cb, p_op);
    }

    // If we are in key refresh phase 1 we provisioned with old key, need to give new device new key as well.
    net_key = wiced_bt_mesh_db_net_key_get(p_mesh_db, 0);
    if (net_key->phase == WICED_BT_MESH_KEY_REFRESH_PHASE_FIRST)
    {
        if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
        {
            p_op->operation = CONFIG_OPERATION_NET_KEY_UPDATE;
            p_op->p_event = mesh_configure_create_event(p_cb->addr, (p_cb->addr != p_cb->unicast_addr));
            p_op->uu.net_key_change.operation = OPERATION_UPDATE;
            p_op->uu.net_key_change.net_key_idx = net_key->index;
            memcpy(p_op->uu.net_key_change.net_key, net_key->key, sizeof(net_key->key));
            configure_pending_operation_queue(p_cb, p_op);
        }
    }

    features = p_cb->p_remote_composition_data->data[8] + (p_cb->p_remote_composition_data->data[9] << 8);

    comp_data_len = p_cb->p_remote_composition_data->data_len - 10;
    p_comp_data = p_cb->p_remote_composition_data->data + 10;
    while (comp_data_len)
    {
        if (comp_data_len < 4)
            break;

        uint8_t num_models = p_comp_data[2];
        uint8_t num_vs_models = p_comp_data[3];

        p_comp_data += 4;
        comp_data_len -= 4;

        if (comp_data_len < 2 * num_models + 4 * num_vs_models)
            break;

        wiced_bt_mesh_db_model_id_t *p_models_array = (wiced_bt_mesh_db_model_id_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_model_id_t) * (num_models + 1));
        if (p_models_array == NULL)
            return;

        for (j = 0; j < num_models; j++)
        {
            p_models_array[j].company_id = MESH_COMPANY_ID_BT_SIG;
            p_models_array[j].id = p_comp_data[j * 2] + (p_comp_data[(j * 2) + 1] << 8);
        }
        p_models_array[j].company_id = MESH_COMPANY_ID_UNUSED;
        p_models_array[j].id = 0xFFFF;

        default_trans_time_model_present = WICED_FALSE;

        for (j = 0; j < num_models; j++)
        {
            uint16_t model_id = p_comp_data[0] + (p_comp_data[1] << 8);
            model_element_t *p_model_elem;

            if (model_id == WICED_BT_MESH_CORE_MODEL_ID_GENERIC_DEFTT_SRV)
                default_trans_time_model_present = WICED_TRUE;

            if ((model_id == WICED_BT_MESH_CORE_MODEL_ID_HEALTH_SRV) ||
#ifdef MESH_DFU_ENABLED
                (model_id == WICED_BT_MESH_CORE_MODEL_ID_FW_UPDATE_SRV) ||
                (model_id == WICED_BT_MESH_CORE_MODEL_ID_FW_UPDATE_CLNT) ||
                (model_id == WICED_BT_MESH_CORE_MODEL_ID_FW_DISTRIBUTION_SRV) ||
                (model_id == WICED_BT_MESH_CORE_MODEL_ID_FW_DISTRIBUTION_CLNT) ||
                (model_id == WICED_BT_MESH_CORE_MODEL_ID_BLOB_TRANSFER_SRV) ||
                (model_id == WICED_BT_MESH_CORE_MODEL_ID_BLOB_TRANSFER_CLNT) ||
#endif
                (model_id == WICED_BT_MESH_CORE_MODEL_ID_HEALTH_CLNT))
            {
                if (!app_key_added)
                {
                    app_key_added = WICED_TRUE;
                    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
                    app_key_add(p_cb, p_cb->addr, net_key, app_key);
                }
                app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
                model_app_bind(p_cb, WICED_FALSE, p_cb->addr + element_idx, MESH_COMPANY_ID_BT_SIG, model_id, app_key->index);
            }
            if ((p_model_elem = model_needs_default_sub(MESH_COMPANY_ID_BT_SIG, model_id)) != NULL)
            {
#ifdef USE_SETUP_APPKEY
                for (i = 0; i < 2; i++)

#endif
                {
                    if ((i == 0) && !app_key_added)
                    {
                        app_key_added = WICED_TRUE;
                        app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
                        app_key_add(p_cb, p_cb->addr, net_key, app_key);
                    }
                    if ((i == 1) && !app_key_setup_added)
                    {
                        app_key_setup_added = WICED_TRUE;
                        app_key_setup = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Setup");
                        app_key_add(p_cb, p_cb->addr, net_key, app_key_setup);
                    }
                }
                // If it is a setup model it needs to be bound to setup app key.
                // For a normal model, it should be bound to generic and setup app key.
#ifdef USE_SETUP_APPKEY
                if (!p_model_elem->is_setup)
#endif
                {
                    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
                    model_app_bind(p_cb, WICED_FALSE, p_cb->addr + element_idx, MESH_COMPANY_ID_BT_SIG, model_id, app_key->index);
                }
#ifdef USE_SETUP_APPKEY
                {
                    app_key_setup = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Setup");
                    model_app_bind(p_cb, WICED_FALSE, p_cb->addr + element_idx, MESH_COMPANY_ID_BT_SIG, model_id, app_key_setup->index);
                }
#endif
                if ((p_group_list != NULL) && model_needs_sub(model_id, p_models_array))
                {
                    for (k = 0; p_group_list[k] != 0; k++)
                    {
                        if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
                        {
                            memset(p_op, 0, sizeof(pending_operation_t));
                            p_op->operation = CONFIG_OPERATION_MODEL_SUBSCRIBE;
                            p_op->p_event = mesh_configure_create_event(p_cb->addr, (p_cb->addr != p_cb->unicast_addr));
                            p_op->uu.model_sub.operation = OPERATION_ADD;
                            p_op->uu.model_sub.element_addr = p_cb->addr + element_idx;
                            p_op->uu.model_sub.company_id = MESH_COMPANY_ID_BT_SIG;
                            p_op->uu.model_sub.model_id = model_id;
                            p_op->uu.model_sub.addr[0] = p_group_list[k] & 0xff;
                            p_op->uu.model_sub.addr[1] = (p_group_list[k] >> 8) & 0xff;
                            configure_pending_operation_queue(p_cb, p_op);
                        }
                    }
                }
            }
            if ((p_model_elem = model_needs_default_pub(MESH_COMPANY_ID_BT_SIG, model_id)) != NULL)
            {
                if (!app_key_added)
                {
                    app_key_added = WICED_TRUE;
                    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
                    app_key_add(p_cb, p_cb->addr, net_key, app_key);
                }
                app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
                model_app_bind(p_cb, WICED_FALSE, p_cb->addr + element_idx, MESH_COMPANY_ID_BT_SIG, model_id, app_key->index);

                // if this a client model (for example a switch we need to configure publication.  We also
                // configure publication for top level servers of the device.  For example a color bulb will be configured
                // to send HSL status on a change.
                if (p_model_elem->need_sub_pub)
//                    ||
//                    ((component_type == DEVICE_TYPE_LIGHT_HSL) && (model_id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SRV)) ||
//                    ((component_type == DEVICE_TYPE_LIGHT_CTL) && (model_id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_SRV)) ||
//                    ((component_type == DEVICE_TYPE_LIGHT_XYL) && (model_id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_XYL_SRV)) ||
//                    ((component_type == DEVICE_TYPE_LIGHT_DIMMABLE) && (model_id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_SRV)) ||
//                    ((component_type == DEVICE_TYPE_POWER_ON_OFF_SERVER) && (model_id == WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_SRV)) ||
//                    ((component_type == DEVICE_TYPE_GENERIC_ON_OFF_SERVER) && (model_id == WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_SRV)) ||
//                    ((component_type == DEVICE_TYPE_POWER_LEVEL_SERVER) && (model_id == WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_SRV)) ||
//                    ((component_type == DEVICE_TYPE_GENERIC_LEVEL_SERVER) && (model_id == WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_SRV)))
                {
                    if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
                    {
                        app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");

                        memset(p_op, 0, sizeof(pending_operation_t));
                        p_op->operation = CONFIG_OPERATION_MODEL_PUBLISH;
                        p_op->p_event = mesh_configure_create_event(p_cb->addr, (p_cb->addr != p_cb->unicast_addr));
                        p_op->uu.model_pub.element_addr = p_cb->addr + element_idx;
                        p_op->uu.model_pub.company_id = MESH_COMPANY_ID_BT_SIG;
                        p_op->uu.model_pub.model_id = model_id;
                        // if group address is 0, the publication will be a network broadcast
                        p_op->uu.model_pub.publish_addr[0] = (p_cb->group_addr ? p_cb->group_addr : 0xFFFF) & 0xff;
                        p_op->uu.model_pub.publish_addr[1] = ((p_cb->group_addr ? p_cb->group_addr : 0xFFFF) >> 8) & 0xff;
                        p_op->uu.model_pub.app_key_idx = app_key->index;
                        p_op->uu.model_pub.publish_period = 0;              // periodic publication if required will need to be configured separately
                        p_op->uu.model_pub.publish_ttl = p_cb->publish_ttl; // tbd
                        p_op->uu.model_pub.publish_retransmit_count = p_cb->publish_retransmit_count;
                        p_op->uu.model_pub.publish_retransmit_interval = p_cb->publish_retransmit_interval;
                        p_op->uu.model_pub.credential_flag = p_cb->publish_credential_flag;
                        configure_pending_operation_queue(p_cb, p_op);
                    }
                }
            }
            //if sensor model is present get sensor information
            if (model_id == WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV)
            {
                if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
                {
                    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");

                    memset(p_op, 0, sizeof(pending_operation_t));
                    p_op->operation = CONFIG_OPERATION_SENSOR_DESC_GET;
                    p_op->p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_CLNT,  p_cb->addr + element_idx, app_key->index);
                    // get all descriptors
                    p_op->uu.sensor_get.property_id = 0;
                    configure_pending_operation_queue(p_cb, p_op);
                }
            }
            p_comp_data += 2;
            comp_data_len -= 2;
        }
        wiced_bt_free_buffer(p_models_array);

        if (num_vs_models != 0)
        {
#ifdef USE_VENDOR_APPKEY
            p_app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Vendor");
            app_key_add(p_cb, p_cb->addr, net_key, p_app_key);
#else
            if (!app_key_added)
            {
                app_key_added = TRUE;
                app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
                app_key_add(p_cb, p_cb->addr, net_key, app_key);
            }
            p_app_key = app_key;
#endif

            for (j = 0; j < num_vs_models; j++)
            {
                uint16_t company_id = p_comp_data[0] + (p_comp_data[1] << 8);
                uint16_t model_id = p_comp_data[2] + (p_comp_data[3] << 8);

                p_comp_data += 4;
                comp_data_len -= 4;

                // check if we know this company ID. We will configure Vendor Specific model if it matches Company ID of the local
                // device. For Controllers that own configuration (Windows Mesh Client, Android, iOS, the company ID is defined in
                // the mesh_config structure.  For the embedded controller, the mesh_provision_client app contains definition
                if (company_id != p_cb->company_id)
                    continue;

                model_app_bind(p_cb, WICED_FALSE, p_cb->addr + element_idx, company_id, model_id, p_app_key->index);
                if (p_group_list != NULL)
                {
                    for (k = 0; p_group_list[k] != 0; k++)
                    {
                        if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
                        {
                            memset(p_op, 0, sizeof(pending_operation_t));
                            p_op->operation = CONFIG_OPERATION_MODEL_SUBSCRIBE;
                            p_op->p_event = mesh_configure_create_event(p_cb->addr, (p_cb->addr != p_cb->unicast_addr));
                            p_op->uu.model_sub.operation = OPERATION_ADD;
                            p_op->uu.model_sub.element_addr = p_cb->addr + element_idx;
                            p_op->uu.model_sub.company_id = company_id;
                            p_op->uu.model_sub.model_id = model_id;
                            p_op->uu.model_sub.addr[0] = p_group_list[k] & 0xff;
                            p_op->uu.model_sub.addr[1] = (p_group_list[k] >> 8) & 0xff;
                            configure_pending_operation_queue(p_cb, p_op);
                        }
                    }
                }
                if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
                {
                    memset(p_op, 0, sizeof(pending_operation_t));
                    p_op->operation = CONFIG_OPERATION_MODEL_PUBLISH;
                    p_op->p_event = mesh_configure_create_event(p_cb->addr, (p_cb->addr != p_cb->unicast_addr));
                    p_op->uu.model_pub.element_addr = p_cb->addr + element_idx;
                    p_op->uu.model_pub.company_id = company_id;
                    p_op->uu.model_pub.model_id = model_id;
                    // if group address is 0, the publication will be a network broadcast
                    p_op->uu.model_pub.publish_addr[0] = (p_cb->group_addr ? p_cb->group_addr : 0xFFFF) & 0xff;
                    p_op->uu.model_pub.publish_addr[1] = ((p_cb->group_addr ? p_cb->group_addr : 0xFFFF) >> 8) & 0xff;
                    p_op->uu.model_pub.app_key_idx = p_app_key->index;
                    p_op->uu.model_pub.publish_period = 0;              // periodic publication if required will need to be configured separately
                    p_op->uu.model_pub.publish_ttl = p_cb->publish_ttl; // tbd
                    p_op->uu.model_pub.publish_retransmit_count = p_cb->publish_retransmit_count;
                    p_op->uu.model_pub.publish_retransmit_interval = p_cb->publish_retransmit_interval;
                    p_op->uu.model_pub.credential_flag = p_cb->publish_credential_flag;
                    configure_pending_operation_queue(p_cb, p_op);
                }
            }
        }
        if (default_trans_time_model_present)
        {
            if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
            {
                app_key_setup = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Setup");

                p_op->operation = CONFIG_OPERATION_DEF_TRANS_TIME;
                p_op->uu.default_trans_time.time = 0;
                p_op->p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_DEFTT_CLNT, p_cb->addr + element_idx, app_key_setup->index);
                if (p_op->p_event)
                {
                    p_op->p_event->retrans_cnt = 4;       // Try 5 times (this is in addition to network layer retransmit)
                    p_op->p_event->retrans_time = 10;     // Every 500 msec
                    p_op->p_event->reply_timeout = 80;    // wait for the reply 4 seconds
                }
                configure_pending_operation_queue(p_cb, p_op);
            }
        }
        element_idx++;
    }
    wiced_bt_free_buffer(p_group_list);
    if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
    {
        p_op->operation = CONFIG_OPERATION_DEFAULT_TTL_SET;
        p_op->p_event = mesh_configure_create_event(p_cb->addr, (p_cb->addr != p_cb->unicast_addr));
        p_op->uu.default_ttl_set.ttl = 63;
        configure_pending_operation_queue(p_cb, p_op);
    }
    // Tell node to set configured values for relay/proxy/beacon unless it is a low power node.
    if (((features & FOUNDATION_FEATURE_BIT_RELAY) == FOUNDATION_FEATURE_BIT_RELAY) &&
        ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL))
    {
        p_op->operation = CONFIG_OPERATION_RELAY_SET;
        p_op->p_event = mesh_configure_create_event(p_cb->addr, (p_cb->addr != p_cb->unicast_addr));
        p_op->uu.relay_set.state = ((features & FOUNDATION_FEATURE_BIT_LOW_POWER) == FOUNDATION_FEATURE_BIT_LOW_POWER) ? 0 : p_cb->is_relay;
        p_op->uu.relay_set.retransmit_count = p_cb->relay_xmit_count;
        p_op->uu.relay_set.retransmit_interval = p_cb->relay_xmit_interval;
        configure_pending_operation_queue(p_cb, p_op);
    }
    if (((features & FOUNDATION_FEATURE_BIT_PROXY) == FOUNDATION_FEATURE_BIT_PROXY) &&
        ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL))
    {
        p_op->operation = CONFIG_OPERATION_PROXY_SET;
        p_op->p_event = mesh_configure_create_event(p_cb->addr, (p_cb->addr != p_cb->unicast_addr));
        p_op->uu.proxy_set.state = ((features & FOUNDATION_FEATURE_BIT_LOW_POWER) == FOUNDATION_FEATURE_BIT_LOW_POWER) ? 0 : p_cb->is_gatt_proxy;
        configure_pending_operation_queue(p_cb, p_op);
    }
    if (((features & FOUNDATION_FEATURE_BIT_FRIEND) == FOUNDATION_FEATURE_BIT_FRIEND) &&
        ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL))
    {
        p_op->operation = CONFIG_OPERATION_FRIEND_SET;
        p_op->p_event = mesh_configure_create_event(p_cb->addr, (p_cb->addr != p_cb->unicast_addr));
        p_op->uu.friend_set.state = ((features & FOUNDATION_FEATURE_BIT_LOW_POWER) == FOUNDATION_FEATURE_BIT_LOW_POWER) ? 0 : p_cb->is_friend;
        configure_pending_operation_queue(p_cb, p_op);
    }
    if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
    {
        p_op->operation = CONFIG_OPERATION_NET_BEACON_SET;
        p_op->p_event = mesh_configure_create_event(p_cb->addr, (p_cb->addr != p_cb->unicast_addr));
        p_op->uu.beacon_set.state = ((features & FOUNDATION_FEATURE_BIT_LOW_POWER) == FOUNDATION_FEATURE_BIT_LOW_POWER) ? 0 : p_cb->beacon;
        configure_pending_operation_queue(p_cb, p_op);
    }
    if ((p_cb->proxy_addr != p_cb->addr) && p_cb->over_gatt && (p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
    {
        p_op->operation = CONFIG_OPERATION_FILTER_ADD;
        p_op->p_event = mesh_configure_create_event(0xFFFF, WICED_FALSE);
        p_op->uu.filter_add.addr_num = (p_mesh_db->num_groups == 0) ? 1 : p_mesh_db->num_groups;
        if (p_mesh_db->num_groups == 0)
        {
            p_op->uu.filter_add.addr_num = 1;
            p_op->uu.filter_add.addr[0] = p_cb->unicast_addr;
        }
        else
        {
            // We have a space for about 10 groups, will probably need to fix at some point
            p_op->uu.filter_add.addr_num = (uint8_t)p_mesh_db->num_groups;

            if ((p_mesh_db->num_groups - 1) * sizeof(uint16_t) + sizeof(p_op->uu.filter_add) > sizeof(p_op->uu))
                p_op->uu.filter_add.addr_num = (sizeof(p_op->uu) - sizeof(p_op->uu.filter_add)) / sizeof(uint16_t) + 1;

            for (i = 0; i < p_op->uu.filter_add.addr_num; i++)
            {
                p_op->uu.filter_add.addr[i] = p_mesh_db->group[i].addr;
            }
        }
        configure_pending_operation_queue(p_cb, p_op);
    }
}

wiced_bool_t configure_event_in_pending_op(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event)
{
    pending_operation_t *p_op;
    for (p_op = p_cb->p_first; p_op != NULL; p_op = p_op->p_next)
    {
        if (p_op->p_event == p_event)
            return WICED_TRUE;
    }
    return WICED_FALSE;
}

void configure_execute_pending_operation(mesh_provision_cb_t *p_cb)
{
    if (p_cb->p_first != NULL)
    {
        if (wiced_start_timer(&p_cb->op_timer, 1) != WICED_BT_SUCCESS)
            provision_timer_cb(p_cb);
    }
    else
    {
        if (p_cb->p_local_composition_data)
        {
            wiced_bt_free_buffer(p_cb->p_local_composition_data);
            p_cb->p_local_composition_data = NULL;

            if (p_cb->store_config)
            {
                wiced_bt_mesh_db_node_config_complete(p_mesh_db, p_cb->unicast_addr, WICED_TRUE);
                wiced_bt_mesh_db_store(p_mesh_db);

                if (p_cb->p_database_changed)
                    provision_cb.p_database_changed(p_mesh_db->name);
            }
            // Result of all other operations after local device configuration should be stored.
            p_cb->store_config = WICED_TRUE;

            if (p_cb->p_opened_callback != NULL)
                p_cb->p_opened_callback(MESH_CLIENT_SUCCESS);

            p_cb->network_opened = WICED_TRUE;

        }
        if (p_cb->p_remote_composition_data)
        {
            wiced_bt_free_buffer(p_cb->p_remote_composition_data);
            p_cb->p_remote_composition_data = NULL;

            if (p_cb->store_config)
            {
                wiced_bt_mesh_db_node_config_complete(p_mesh_db, p_cb->addr, WICED_TRUE);
                wiced_bt_mesh_db_store(p_mesh_db);
            }
            provision_status_notify(p_cb, MESH_CLIENT_PROVISION_STATUS_SUCCESS);

            if (p_cb->p_database_changed)
                provision_cb.p_database_changed(p_mesh_db->name);
        }
        Log("done\n");
    }
}

void provision_status_notify(mesh_provision_cb_t* p_cb, uint8_t status)
{
    if (!p_cb->provision_completed_sent && (p_cb->p_provision_status != NULL))
        p_cb->p_provision_status(status, p_cb->uuid);

    if (status == MESH_CLIENT_PROVISION_STATUS_FAILED)
    {
        if ((p_cb->provision_last_status == MESH_CLIENT_PROVISION_STATUS_END) || (p_cb->provision_last_status == MESH_CLIENT_PROVISION_STATUS_CONFIGURING))
        {
            wiced_bt_mesh_db_node_t* node = wiced_bt_mesh_db_node_get_by_addr(p_mesh_db, p_cb->addr);
            if (node != NULL)
                mesh_reset_node(p_cb, node);
        }
    }
    p_cb->provision_last_status = status;

    if ((status == MESH_CLIENT_PROVISION_STATUS_SUCCESS) || (status == MESH_CLIENT_PROVISION_STATUS_FAILED))
        p_cb->provision_completed_sent = WICED_TRUE;
}

void provision_timer_cb(TIMER_PARAM_TYPE arg)
{
    mesh_provision_cb_t *p_cb = (mesh_provision_cb_t *)arg;
    pending_operation_t *p_op = p_cb->p_first;
    char buf[160];
    int i;

    if (p_op == NULL)
        return;

    if (p_op->p_event == NULL)
    {
        Log("Failed op:%d", p_op->operation);
        start_next_op(p_cb);
        return;
    }
    // all state machine transitions require a reply
    p_op->p_event->reply = WICED_TRUE;

    mesh_configure_set_local_device_key(p_op->p_event->dst);

    switch (p_op->operation)
    {
    case CONFIG_OPERATION_SET_DEV_KEY:
        sprintf(buf, "Set Dev Key addr:%04x dev_key:", p_op->p_event->dst);
        for (i = 0; i < 16; i++)
        {
            sprintf(&buf[strlen(buf)], "%02x ", p_op->uu.set_dev_key.dev_key[i]);
        }
        Log(buf);
        wiced_bt_mesh_provision_set_dev_key(&p_op->uu.set_dev_key);
        break;
    case CONFIG_OPERATION_NET_KEY_UPDATE:
        Log("NetKey %s addr:%04x net_key_idx:%04x", p_op->uu.net_key_change.operation == OPERATION_ADD ? "Add" : "Update", p_op->p_event->dst, p_op->uu.net_key_change.net_key_idx);
        wiced_bt_mesh_config_netkey_change(p_op->p_event, &p_op->uu.net_key_change);
        break;
    case CONFIG_OPERATION_APP_KEY_UPDATE:
        Log("AppKey %s addr:%04x net_key_idx:%04x app_key_idx:%04x", p_op->uu.app_key_change.operation == OPERATION_ADD ? "Add" : "Update", p_op->p_event->dst, p_op->uu.app_key_change.net_key_idx, p_op->uu.app_key_change.app_key_idx);
        wiced_bt_mesh_config_appkey_change(p_op->p_event, &p_op->uu.app_key_change);
        break;
    case CONFIG_OPERATION_MODEL_APP_BIND:
        Log("Model App %s addr:0x%04x element_addr:%04x company_id:%04x model_id:%04x app_key_idx:%04x", p_op->uu.app_key_bind.operation == OPERATION_BIND ? "Bind" : "Unbind", p_op->p_event->dst, p_op->uu.app_key_bind.element_addr, p_op->uu.app_key_bind.company_id, p_op->uu.app_key_bind.model_id, p_op->uu.app_key_bind.app_key_idx);
        wiced_bt_mesh_config_model_app_bind(p_op->p_event, &p_op->uu.app_key_bind);
        break;
    case CONFIG_OPERATION_MODEL_SUBSCRIBE:
        Log("Model Sub Operation:%d addr:%04x elem:%x company_id:%04x model:%x addr:%04x", p_op->uu.model_sub.operation, p_op->p_event->dst, p_op->uu.model_sub.element_addr, p_op->uu.model_sub.company_id, p_op->uu.model_sub.model_id, p_op->uu.model_sub.addr[0] + (p_op->uu.model_sub.addr[1] << 8));
        wiced_bt_mesh_config_model_subscription_change(p_op->p_event, &p_op->uu.model_sub);
        break;
    case CONFIG_OPERATION_MODEL_PUBLISH:
        Log("Model Pub Set addr:%04x elem:%04x comp:%04x model:%04x addr:%04x TTL:%d Period:%d Count:%d Interval:%d Cred:%d", p_op->p_event->dst, p_op->uu.model_pub.element_addr, p_op->uu.model_pub.company_id, p_op->uu.model_pub.model_id, p_op->uu.model_pub.publish_addr[0] + (p_op->uu.model_pub.publish_addr[1] << 8),
            p_op->uu.model_pub.publish_ttl, p_op->uu.model_pub.publish_period, p_op->uu.model_pub.publish_retransmit_count, p_op->uu.model_pub.publish_retransmit_interval, p_op->uu.model_pub.credential_flag);
        wiced_bt_mesh_config_model_publication_set(p_op->p_event, &p_op->uu.model_pub);
        break;
    case CONFIG_OPERATION_NET_TRANSMIT_SET:
        Log("Net transmit set addr:%04x count:%d interval:%d", p_op->p_event->dst, p_op->uu.net_transmit_set.count, p_op->uu.net_transmit_set.interval);
        wiced_bt_mesh_config_network_transmit_params_set(p_op->p_event, &p_op->uu.net_transmit_set);
        break;
    case CONFIG_OPERATION_DEFAULT_TTL_SET:
        Log("Default TTL set addr:%04x TTL:%d", p_op->p_event->dst, p_op->uu.default_ttl_set.ttl);
        wiced_bt_mesh_config_default_ttl_set(p_op->p_event, &p_op->uu.default_ttl_set);
        break;
    case CONFIG_OPERATION_DEF_TRANS_TIME:
        Log("Def trans time addr:%04x time:%d", p_op->p_event->dst, p_op->uu.default_trans_time.time);
        wiced_bt_mesh_model_default_transition_time_client_send_set(p_op->p_event, &p_op->uu.default_trans_time);
        break;
    case CONFIG_OPERATION_RELAY_SET:
        Log("Relay set addr:%04x state:%d rxmit count:%d rxmit interval:%d", p_op->p_event->dst, p_op->uu.relay_set.state, p_op->uu.relay_set.retransmit_count, p_op->uu.relay_set.retransmit_interval);
        wiced_bt_mesh_config_relay_set(p_op->p_event, &p_op->uu.relay_set);
        break;
    case CONFIG_OPERATION_PROXY_SET:
        Log("GATT Proxy set addr:%04x state:%d", p_op->p_event->dst, p_op->uu.proxy_set.state);
        wiced_bt_mesh_config_gatt_proxy_set(p_op->p_event, &p_op->uu.proxy_set);
        break;
    case CONFIG_OPERATION_FRIEND_SET:
        Log("Friend set addr:%04x state:%d", p_op->p_event->dst, p_op->uu.friend_set.state);
        wiced_bt_mesh_config_friend_set(p_op->p_event, &p_op->uu.friend_set);
        break;
    case CONFIG_OPERATION_NET_BEACON_SET:
        Log("Beacon set addr:%04x state:%d", p_op->p_event->dst, p_op->uu.beacon_set.state);
        wiced_bt_mesh_config_beacon_set(p_op->p_event, &p_op->uu.beacon_set);
        break;
    case CONFIG_OPERATION_FILTER_ADD:
        Log("Filter add addr:%04x num:%d first addr:%04x", p_op->p_event->dst, p_op->uu.filter_add.addr_num, p_op->uu.filter_add.addr[0]);
        wiced_bt_mesh_proxy_filter_change_addr(p_op->p_event, 1, &p_op->uu.filter_add);
        break;
    case CONFIG_OPERATION_FILTER_DEL:
        Log("Filter remove addr:%04x num:%d first addr:%04x", p_op->p_event->dst, p_op->uu.filter_add.addr_num, p_op->uu.filter_add.addr[0]);
        wiced_bt_mesh_proxy_filter_change_addr(p_op->p_event, 0, &p_op->uu.filter_add);
        break;
    case CONFIG_OPERATION_NODE_RESET:
        Log("Node Reset addr:%04x", p_op->p_event->dst);
        wiced_bt_mesh_config_node_reset(p_op->p_event);
        break;
    case CONFIG_OPERATION_KR_PHASE_SET:
        Log("Key Refresh Phase Set:%04x net_key_idx:%04x transition:%d", p_op->p_event->dst, p_op->uu.kr_phase_set.net_key_idx, p_op->uu.kr_phase_set.transition);
        wiced_bt_mesh_config_key_refresh_phase_set(p_op->p_event, &p_op->uu.kr_phase_set);
        break;
    case CONFIG_OPERATION_SENSOR_DESC_GET:
        Log("Sensor Descriptor Get:%04x ", p_op->p_event->dst);
        wiced_bt_mesh_model_sensor_client_descriptor_send_get(p_op->p_event, &p_op->uu.sensor_get);
        break;
    case CONFIG_OPERATION_SENSOR_SETTINGS_GET:
        Log("Sensor settings Get:%04x ", p_op->p_event->dst);
        wiced_bt_mesh_model_sensor_client_sensor_settings_send_get(p_op->p_event, &p_op->uu.sensor_get);
        break;
    case CONFIG_OPERATION_SENSOR_CADENCE_GET:
        Log("Sensor cadence Get:%04x ", p_op->p_event->dst);
        wiced_bt_mesh_model_sensor_client_sensor_cadence_send_get(p_op->p_event, &p_op->uu.sensor_get);
        break;
    }
}

uint16_t *get_group_list(uint16_t group_addr)
{
    uint16_t num_groups = 1;
    wiced_bt_mesh_db_group_t *group = wiced_bt_mesh_db_group_get_by_addr(p_mesh_db, group_addr);
    uint16_t *group_list = NULL;
    int i = 0;

    if (group == NULL)
        return NULL;

    while (group->parent_addr != 0)
    {
        num_groups++;
        group = wiced_bt_mesh_db_group_get_by_addr(p_mesh_db, group->parent_addr);
        if (group == NULL)
            break;
    }
    group_list = (uint16_t *)wiced_bt_get_buffer((num_groups + 1) * sizeof(uint16_t));
    if (group_list == NULL)
        return NULL;

    memset(group_list, 0, (num_groups + 1) * sizeof(uint16_t));

    group = wiced_bt_mesh_db_group_get_by_addr(p_mesh_db, group_addr);
    group_list[i++] = group_addr;

    while (group->parent_addr != 0)
    {
        group = wiced_bt_mesh_db_group_get_by_addr(p_mesh_db, group->parent_addr);
        if (group == NULL)
            break;

        group_list[i++] = group->addr;
    }
    return group_list;
}

/*
 * Subscribe all the models in the component to the group address
 */
int mesh_client_add_component_to_group(const char *component_name, const char *group_name)
{
    wiced_bt_mesh_db_node_t* p_node = wiced_bt_mesh_db_node_get_by_element_name(p_mesh_db, component_name);
    wiced_bt_mesh_db_model_id_t *p_models_array;
    uint16_t i, j, k;
    pending_operation_t *p_op;
    uint16_t dst;
    mesh_provision_cb_t *p_cb = &provision_cb;
    uint16_t group_addr;
    uint16_t *p_group_list;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    if (p_cb->state != PROVISION_STATE_IDLE)
    {
        Log("invalid state:%d\n", p_cb->state);
        return MESH_CLIENT_ERR_INVALID_STATE;
    }
    if (p_cb->proxy_conn_id == 0)
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    if (group_name[0] == 0)
    {
        Log("invalid groupname\n");
        return MESH_CLIENT_ERR_INVALID_ARGS;
    }
    if (p_node == NULL)
    {
        Log("component %s not found", component_name);
        return MESH_CLIENT_ERR_INVALID_ARGS;
    }
    if ((group_addr = wiced_bt_mesh_db_group_get_addr(p_mesh_db, group_name)) == 0)
    {
        Log("group %s doesnot exist", group_name);
        return MESH_CLIENT_ERR_INVALID_ARGS;
    }

    p_group_list = get_group_list(group_addr);
    clean_pending_op_queue(0);

    dst = p_node->unicast_address;

    for (i = 0; i < p_node->num_elements; i++)
    {
        if ((p_models_array = wiced_bt_mesh_db_get_all_models_of_element(p_mesh_db, p_node->unicast_address + i, 0)) != NULL)
        {
            for (j = 0; p_models_array[j].company_id != MESH_COMPANY_ID_UNUSED; j++)
            {
                if (is_core_model(p_models_array[j].company_id, p_models_array[j].id))
                    continue;

                if ((p_group_list != NULL) && ((p_models_array[j].company_id != MESH_COMPANY_ID_BT_SIG) || model_needs_sub(p_models_array[j].id, p_models_array)))
                {
                    for (k = 0; p_group_list[k] != 0; k++)
                    {
                        if (!wiced_bt_mesh_db_is_model_subscribed_to_group(p_mesh_db, p_node->unicast_address + i, p_models_array[j].company_id, p_models_array[j].id, p_group_list[k]))
                        {
                            if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
                            {
                                memset(p_op, 0, sizeof(pending_operation_t));
                                p_op->operation = CONFIG_OPERATION_MODEL_SUBSCRIBE;
                                p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
                                p_op->uu.model_sub.operation = OPERATION_ADD;
                                p_op->uu.model_sub.element_addr = p_node->unicast_address + i;
                                p_op->uu.model_sub.company_id = p_models_array[j].company_id;
                                p_op->uu.model_sub.model_id = p_models_array[j].id;
                                p_op->uu.model_sub.addr[0] = p_group_list[k] & 0xff;
                                p_op->uu.model_sub.addr[1] = (p_group_list[k] >> 8) & 0xff;
                                configure_pending_operation_queue(p_cb, p_op);
                            }
                        }
                        else
                        {
                            Log("Model:%4x is subscribed to the group:%4x\n", p_models_array[j].id, p_group_list[k]);
                        }
                    }
                }

                // If this model was configured for publication to the group, modify the publication address keeping
                // all other parameters of the publication. For example, a sensor may be configure for periodic publications that we want to keep.
                uint16_t pub_addr;
                uint16_t app_key_idx;
                uint8_t  publish_ttl;
                uint32_t publish_period;
                uint16_t publish_retransmit_count;
                uint32_t publish_retransmit_interval;
                uint8_t  credentials;

                if (wiced_bt_mesh_db_node_model_pub_get(p_mesh_db, p_node->unicast_address + i, p_models_array[j].company_id, p_models_array[j].id, &pub_addr, &app_key_idx, &publish_ttl, &publish_period, &publish_retransmit_count, &publish_retransmit_interval, &credentials)
                 && ((pub_addr == 0xFFFF) || (pub_addr != group_addr)))
                {
                    Log("Model:%4x reconfigure publication to:%4x\n", p_models_array[j].id, group_addr);

                    if ((p_op = (pending_operation_t*)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
                    {
                        memset(p_op, 0, sizeof(pending_operation_t));
                        p_op->operation = CONFIG_OPERATION_MODEL_PUBLISH;
                        p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
                        p_op->uu.model_pub.element_addr = p_node->unicast_address + i;
                        p_op->uu.model_pub.company_id = p_models_array[j].company_id;
                        p_op->uu.model_pub.model_id = p_models_array[j].id;
                        p_op->uu.model_pub.publish_addr[0] = group_addr & 0xff;
                        p_op->uu.model_pub.publish_addr[1] = (group_addr >> 8) & 0xff;
                        p_op->uu.model_pub.app_key_idx = app_key_idx;

                        // changing the group, keep using configured pub parameters.
                        p_op->uu.model_pub.publish_period = publish_period;
                        p_op->uu.model_pub.publish_ttl = publish_ttl;
                        p_op->uu.model_pub.publish_retransmit_count = (uint8_t)publish_retransmit_count;
                        p_op->uu.model_pub.publish_retransmit_interval = (uint16_t)publish_retransmit_interval;
                        p_op->uu.model_pub.credential_flag = credentials;
                        configure_pending_operation_queue(p_cb, p_op);
                    }
                }
            }
            wiced_bt_free_buffer(p_models_array);
        }
    }
    wiced_bt_free_buffer(p_group_list);
    if (p_cb->p_first != NULL)
    {
        p_cb->state = PROVISION_STATE_RECONFIGURATION;
        configure_execute_pending_operation(p_cb);
        return MESH_CLIENT_SUCCESS;
    }
    return MESH_CLIENT_ERR_NOT_FOUND;
}

/*
 *  Removes the component with the specified name from the group.
 */
int mesh_client_remove_component_from_group(const char *component_name, const char *group_name)
{
    wiced_bt_mesh_db_node_t* p_node = wiced_bt_mesh_db_node_get_by_element_name(p_mesh_db, component_name);
    uint16_t element_idx, model_idx, sub_idx;
    pending_operation_t *p_op;
    uint16_t dst;
    mesh_provision_cb_t *p_cb = &provision_cb;
    wiced_bt_mesh_db_group_t* p_group;
    model_element_t* p_model_elem;
    uint16_t pub_addr;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    if (p_cb->state != PROVISION_STATE_IDLE)
    {
        Log("invalid state:%d\n", p_cb->state);
        return MESH_CLIENT_ERR_INVALID_STATE;
    }
    if (p_cb->proxy_conn_id == 0)
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    if ((p_node == NULL) || ((p_node != NULL) && p_node->blocked))
    {
        Log("component %s not found", component_name);
        return MESH_CLIENT_ERR_INVALID_ARGS;
    }
    if ((p_group = wiced_bt_mesh_db_group_get_by_name(p_mesh_db, group_name)) == 0)
    {
        Log("group %s doesnot exist", group_name);
        return MESH_CLIENT_ERR_INVALID_ARGS;
    }

    clean_pending_op_queue(0);

    dst = p_node->unicast_address;

    for (element_idx = 0; element_idx < p_node->num_elements; element_idx++)
    {
        for (model_idx = 0; model_idx < p_node->element[element_idx].num_models; model_idx++)
        {
            // If model was subscribed to the group address, delete the subscription
            for (sub_idx = 0; sub_idx < p_node->element[element_idx].model[model_idx].num_subs; sub_idx++)
            {
                if (p_node->element[element_idx].model[model_idx].sub[sub_idx] == p_group->addr)
                {
                    if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
                    {
                        memset(p_op, 0, sizeof(pending_operation_t));
                        p_op->operation = CONFIG_OPERATION_MODEL_SUBSCRIBE;
                        p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
                        p_op->uu.model_sub.operation = OPERATION_DELETE;
                        p_op->uu.model_sub.element_addr = p_node->unicast_address + element_idx;
                        p_op->uu.model_sub.company_id = p_node->element[element_idx].model[model_idx].model.company_id;
                        p_op->uu.model_sub.model_id = p_node->element[element_idx].model[model_idx].model.id;
                        p_op->uu.model_sub.addr[0] = p_group->addr & 0xff;
                        p_op->uu.model_sub.addr[1] = (p_group->addr >> 8) & 0xff;
                        configure_pending_operation_queue(p_cb, p_op);
                    }
                }
            }
            // If model was configured to publish to the group, change publication to publish to the parent group or to unassigned address to disable publications
            if (p_node->element[element_idx].model[model_idx].pub.address == p_group->addr)
            {
                pub_addr = p_group->parent_addr != 0 ? p_group->parent_addr : 0xFFFF;

                // We always configure Vendor models for publications
                // For BT_SIG models we only configure models that are set int the models_configured_for_pub array
                if (p_node->element[element_idx].model[model_idx].model.company_id == MESH_COMPANY_ID_BT_SIG)
                {
                    p_model_elem = model_needs_default_pub(p_node->element[element_idx].model[model_idx].model.company_id, p_node->element[element_idx].model[model_idx].model.id);

                    // setting pub address to MESH_UNASSIGNED_ADDR disables publications
                    if ((p_model_elem == NULL) || !p_model_elem->need_sub_pub)
                        pub_addr = 0; // MESH_UNASSIGNED_ADDR
                }

                if ((p_op = (pending_operation_t*)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
                {
                    memset(p_op, 0, sizeof(pending_operation_t));
                    p_op->operation = CONFIG_OPERATION_MODEL_PUBLISH;
                    p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
                    p_op->uu.model_pub.element_addr = p_node->unicast_address + element_idx;
                    p_op->uu.model_pub.company_id = p_node->element[element_idx].model[model_idx].model.company_id;
                    p_op->uu.model_pub.model_id = p_node->element[element_idx].model[model_idx].model.id;
                    p_op->uu.model_pub.publish_addr[0] = pub_addr & 0xff;
                    p_op->uu.model_pub.publish_addr[1] = (pub_addr >> 8) & 0xff;
                    p_op->uu.model_pub.app_key_idx = p_node->element[element_idx].model[model_idx].pub.index;
                    p_op->uu.model_pub.publish_period = p_node->element[element_idx].model[model_idx].pub.period;
                    p_op->uu.model_pub.publish_ttl = p_node->element[element_idx].model[model_idx].pub.ttl;
                    p_op->uu.model_pub.publish_retransmit_count = (uint8_t)p_node->element[element_idx].model[model_idx].pub.retransmit.count;
                    p_op->uu.model_pub.publish_retransmit_interval = p_node->element[element_idx].model[model_idx].pub.retransmit.interval;
                    p_op->uu.model_pub.credential_flag = p_node->element[element_idx].model[model_idx].pub.credentials;
                    configure_pending_operation_queue(p_cb, p_op);
                }
            }
        }
    }
    if (p_cb->p_first != NULL)
    {
        p_cb->state = PROVISION_STATE_RECONFIGURATION;
        configure_execute_pending_operation(p_cb);
    }
    return MESH_CLIENT_SUCCESS;
}


int mesh_client_move_component_to_group(const char *component_name, const char *from_group_name, const char *to_group_name)
{
    wiced_bt_mesh_db_model_id_t *p_models_array;
    uint16_t j;
    pending_operation_t *p_op;
    uint16_t dst;
    mesh_provision_cb_t *p_cb = &provision_cb;
    wiced_bt_mesh_db_group_t* p_group_to;
    wiced_bt_mesh_db_group_t* p_group_from;
    wiced_bt_mesh_db_node_t* p_node = wiced_bt_mesh_db_node_get_by_element_name(p_mesh_db, component_name);
    int i;
    wiced_bool_t in_component;
    uint16_t first_component_element;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    if (p_cb->state != PROVISION_STATE_IDLE)
    {
        Log("invalid state:%d\n", p_cb->state);
        return MESH_CLIENT_ERR_INVALID_STATE;
    }
    if (p_cb->proxy_conn_id == 0)
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    if (p_node == NULL)
    {
        Log("component %s not found", component_name);
        return MESH_CLIENT_ERR_INVALID_ARGS;
    }
    if ((p_group_to = wiced_bt_mesh_db_group_get_by_name(p_mesh_db, to_group_name)) == 0)
    {
        Log("group %s doesnot exist", to_group_name);
        return MESH_CLIENT_ERR_INVALID_ARGS;
    }
    if ((p_group_from = wiced_bt_mesh_db_group_get_by_name(p_mesh_db, from_group_name)) == 0)
    {
        Log("group %s doesnot exist", from_group_name);
        return MESH_CLIENT_ERR_INVALID_ARGS;
    }
    if (p_group_to->parent_addr != p_group_from->parent_addr)
    {
        Log("groups should be of the same parent");
        return MESH_CLIENT_ERR_INVALID_ARGS;
    }
    if (p_group_to->addr == p_group_from->addr)
    {
        Log("groups should be different");
        return MESH_CLIENT_ERR_INVALID_ARGS;
    }
    clean_pending_op_queue(0);

    dst = p_node->unicast_address;

    // need to move all reconfigure all models of the specified component. There may be
    // several components in the node and we only want to move one.
    in_component = WICED_FALSE;

    for (i = 0; i < p_node->num_elements; i++)
    {
        if (!in_component)
        {
            // skip all elements that do not belong to this component
            // find first element of the component with component_name
            const char* p_name = get_component_name(p_node->unicast_address + i);
            if (strcmp(p_name, component_name) != 0)
                continue;

            first_component_element = i;
            in_component = WICED_TRUE;
        }

        // If we are in the element that do not belong to the component, we are done
        if ((i != first_component_element) && !is_secondary_element(p_node->unicast_address + i))
            break;

        // If we are here, this is the primary or secondary element of the component
        if ((p_models_array = wiced_bt_mesh_db_get_all_models_of_element(p_mesh_db, p_node->unicast_address + i, 0)) != NULL)
        {
            for (j = 0; p_models_array[j].company_id != MESH_COMPANY_ID_UNUSED; j++)
            {
                if (is_core_model(p_models_array[j].company_id, p_models_array[j].id))
                    continue;

                if ((p_models_array[j].company_id != MESH_COMPANY_ID_BT_SIG) || model_needs_sub(p_models_array[j].id, p_models_array))
                {
                    if (!wiced_bt_mesh_db_is_model_subscribed_to_group(p_mesh_db, p_node->unicast_address + i, p_models_array[j].company_id, p_models_array[j].id, p_group_to->addr))
                    {
                        Log("Model:%4x is not subscribed to the group adding subscription:%4x\n", p_models_array[j].id, p_group_to->addr);
                        if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
                        {
                            memset(p_op, 0, sizeof(pending_operation_t));
                            p_op->operation = CONFIG_OPERATION_MODEL_SUBSCRIBE;
                            p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
                            p_op->uu.model_sub.operation = OPERATION_ADD;
                            p_op->uu.model_sub.element_addr = p_node->unicast_address + i;
                            p_op->uu.model_sub.company_id = p_models_array[j].company_id;
                            p_op->uu.model_sub.model_id = p_models_array[j].id;
                            p_op->uu.model_sub.addr[0] = p_group_to->addr & 0xff;
                            p_op->uu.model_sub.addr[1] = (p_group_to->addr >> 8) & 0xff;
                            configure_pending_operation_queue(p_cb, p_op);
                        }
                    }
                    else
                    {
                        Log("Model:%4x is subscribed to the group:%4x\n", p_models_array[j].id, p_group_to->addr);
                    }
                }

                // If this model was configured for publication to the group, modify the publication address keeping
                // all other parameters of the publication. For example, a sensor may be configure for periodic publications that we want to keep.
                uint16_t pub_addr;
                uint16_t app_key_idx;
                uint8_t  publish_ttl;
                uint32_t publish_period;
                uint16_t publish_retransmit_count;
                uint32_t publish_retransmit_interval;
                uint8_t  credentials;

                if (wiced_bt_mesh_db_node_model_pub_get(p_mesh_db, p_node->unicast_address + i, p_models_array[j].company_id, p_models_array[j].id, &pub_addr, &app_key_idx, &publish_ttl, &publish_period, &publish_retransmit_count, &publish_retransmit_interval, &credentials)
                 && ((pub_addr == 0xFFFF) || (pub_addr == p_group_from->addr)))
                {
                    Log("Model:%4x reconfigure publication to:%4x\n", p_models_array[j].id, p_group_to->addr);

                    if ((p_op = (pending_operation_t*)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
                    {
                        memset(p_op, 0, sizeof(pending_operation_t));
                        p_op->operation = CONFIG_OPERATION_MODEL_PUBLISH;
                        p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
                        p_op->uu.model_pub.element_addr = p_node->unicast_address + i;
                        p_op->uu.model_pub.company_id = p_models_array[j].company_id;
                        p_op->uu.model_pub.model_id = p_models_array[j].id;
                        p_op->uu.model_pub.publish_addr[0] = p_group_to->addr & 0xff;
                        p_op->uu.model_pub.publish_addr[1] = (p_group_to->addr >> 8) & 0xff;
                        p_op->uu.model_pub.app_key_idx = app_key_idx;

                        // if we are changing the group, use configured pub parameters, otherwise use defaults.
                        p_op->uu.model_pub.publish_period = (pub_addr == p_group_from->addr) ? (uint32_t)publish_period : 0;
                        p_op->uu.model_pub.publish_ttl = (pub_addr == p_group_from->addr) ? publish_ttl : p_cb->publish_ttl;
                        p_op->uu.model_pub.publish_retransmit_count = (pub_addr == p_group_from->addr) ? publish_retransmit_count : p_cb->publish_retransmit_count;
                        p_op->uu.model_pub.publish_retransmit_interval = (pub_addr == p_group_from->addr) ? publish_retransmit_interval : p_cb->publish_retransmit_interval;
                        p_op->uu.model_pub.credential_flag = (pub_addr == p_group_from->addr) ? credentials : p_cb->publish_credential_flag;
                        configure_pending_operation_queue(p_cb, p_op);
                    }
                }
            }
            wiced_bt_free_buffer(p_models_array);
        }
    }

    // now the models of the element are subscribed to both group_to and gropu_from. Remove subsciptions from the group_from.
    // Note that we do not need to do publications because publication address has been changed.

    // need to move all reconfigure all models of the specified component. There may be
    // several components in the node and we only want to move one.
    in_component = WICED_FALSE;

    for (i = 0; i < p_node->num_elements; i++)
    {
        if (!in_component)
        {
            // skip all elements that do not belong to this component
            // find first element of the component with component_name
            const char* p_name = get_component_name(p_node->unicast_address + i);
            if (strcmp(p_name, component_name) != 0)
                continue;

            first_component_element = i;
            in_component = WICED_TRUE;
        }

        // If we are in the element that do not belong to the component, we are done
        if ((i != first_component_element) && !is_secondary_element(p_node->unicast_address + i))
            break;

        // If we are here, this is the primary or secondary element of the component
        if ((p_models_array = wiced_bt_mesh_db_get_all_models_of_element(p_mesh_db, p_node->unicast_address + i, p_group_from->addr)) != NULL)
        {
            for (j = 0; p_models_array[j].company_id != MESH_COMPANY_ID_UNUSED; j++)
            {
                if (is_core_model(p_models_array[j].company_id, p_models_array[j].id))
                    continue;

                if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
                {
                    memset(p_op, 0, sizeof(pending_operation_t));
                    p_op->operation = CONFIG_OPERATION_MODEL_SUBSCRIBE;
                    p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
                    p_op->uu.model_sub.operation = OPERATION_DELETE;
                    p_op->uu.model_sub.element_addr = p_node->unicast_address + i;
                    p_op->uu.model_sub.company_id = p_models_array[j].company_id;
                    p_op->uu.model_sub.model_id = p_models_array[j].id;
                    p_op->uu.model_sub.addr[0] = p_group_from->addr & 0xff;
                    p_op->uu.model_sub.addr[1] = (p_group_from->addr >> 8) & 0xff;
                    configure_pending_operation_queue(p_cb, p_op);
                }
            }
            wiced_bt_free_buffer(p_models_array);
        }
    }
    if (p_cb->p_first != NULL)
    {
        p_cb->state = PROVISION_STATE_RECONFIGURATION;
        configure_execute_pending_operation(p_cb);
    }
    return MESH_CLIENT_SUCCESS;
}

char *mesh_client_get_component_group_list(char *p_component_name)
{
    uint16_t *p_elements_array, *p_element, *p_group, *p_group_array = NULL;
    char *p_groups_array = NULL;
    uint32_t total_len = 0;

    if (p_mesh_db == NULL)
        return NULL;

    if ((p_elements_array = wiced_bt_mesh_db_get_all_elements(p_mesh_db)) == NULL)
        return NULL;

    for (p_element = p_elements_array; *p_element != 0; p_element++)
    {
        if (is_secondary_element(*p_element))
            continue;

        const char *p_name = get_component_name(*p_element);

        if (strcmp(p_component_name, p_name) == 0)
        {
            if ((p_group_array = wiced_bt_mesh_db_get_element_group_list(p_mesh_db, *p_element)) == NULL)
            {
                wiced_bt_free_buffer(p_elements_array);
                return NULL;
            }
            break;
        }
    }

    wiced_bt_free_buffer(p_elements_array);

    if (p_group_array == NULL)
        return NULL;

    for (p_group = p_group_array; *p_group != 0; p_group++)
    {
        total_len += (strlen(wiced_bt_mesh_db_get_group_name(p_mesh_db, *p_group)) + 1);
    }

    if ((p_groups_array = (char *)wiced_bt_get_buffer(total_len + 1)) == NULL)

    {
        return NULL;
    }
    memset(p_groups_array, 0, (total_len + 1));
    total_len = 0;
    for (p_group = p_group_array; *p_group != 0; p_group++)
    {
        const char *p_name = wiced_bt_mesh_db_get_group_name(p_mesh_db, *p_group);
        strcpy(&p_groups_array[total_len], p_name);
        total_len += (strlen(p_name) + 1);
    }

    p_groups_array[total_len] = 0;
    return p_groups_array;
}

int mesh_client_configure_publication(const char *component_name, uint8_t is_client, const char *method, const char *target_name, int publish_period)
{
    uint16_t *p_elements_array, *p_element, *p_secondary_element = NULL;
    wiced_bt_mesh_db_model_id_t *p_models_array;
    uint16_t j;
    pending_operation_t *p_op;
    mesh_provision_cb_t *p_cb = &provision_cb;
    uint16_t component_addr = 0;
    uint16_t company_id;
    uint16_t model_id;
    uint16_t target_addr;
    wiced_bt_mesh_db_app_key_t *app_key;

    if ((component_name == NULL) || (target_name == NULL))
        return MESH_CLIENT_ERR_INVALID_ARGS;

    if (is_client)
    {
        if (!get_control_method(method, &company_id, &model_id))
            return MESH_CLIENT_ERR_INVALID_ARGS;
    }
    else
    {
        if (!get_target_method(method, &company_id, &model_id))
            return MESH_CLIENT_ERR_INVALID_ARGS;
    }
    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    if (p_cb->state != PROVISION_STATE_IDLE)
    {
        Log("invalid state:%d\n", p_cb->state);
        return MESH_CLIENT_ERR_INVALID_STATE;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    clean_pending_op_queue(0);

    if (strcmp(target_name, "none") == 0)
        target_addr = 0;
    else
    {
        if (strcmp(target_name, "all-nodes") == 0)
            target_addr = 0xffff;
        else if (strcmp(target_name, "all-proxies") == 0)
            target_addr = 0xfffc;
        else if (strcmp(target_name, "all_friends") == 0)
            target_addr = 0xfffd;
        else if (strcmp(target_name, "all-relays") == 0)
            target_addr = 0xfffe;
        else if (strcmp(target_name, "this-device") == 0)
            target_addr = p_cb->unicast_addr;
        else
            target_addr = wiced_bt_mesh_db_group_get_addr(p_mesh_db, target_name);
    }
    // go through elements to find out which device is going to be configured for publication
    // and if target is not a group or provisioner, it should be one of the elements.
    if ((p_elements_array = wiced_bt_mesh_db_get_all_elements(p_mesh_db)) == NULL)
        return MESH_CLIENT_ERR_NETWORK_DB;

    // find first element of the component with component_name
    for (p_element = p_elements_array; *p_element != 0; p_element++)
    {
        if (is_secondary_element(*p_element))
            continue;

        const char *p_name = get_component_name(*p_element);
        if (strcmp(p_name, component_name) == 0)
        {
            component_addr = *p_element;

            // special case when Light LC server is being configured to publish on/off status.  We
            // want to publish on/off from the primary and from the LC elements
            if ((company_id == MESH_COMPANY_ID_BT_SIG) && (model_id == WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_SRV))
            {
                for (p_secondary_element = p_element + 1; *p_secondary_element != 0; p_secondary_element++)
                {
                    if (!is_secondary_element(*p_secondary_element))
                    {
                        p_secondary_element = NULL;
                        break;
                    }
                    if (is_model_present(*p_secondary_element, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LC_SRV))
                        break;
                }
            }
        }
        if ((strcmp(target_name, "none") != 0) && (target_addr == 0))
        {
            if (strcmp(p_name, target_name) == 0)
            {
                target_addr = *p_element;
            }
        }
    }

    if (((strcmp(target_name, "none") != 0) && (target_addr == 0)) || (component_addr == 0))
    {
        wiced_bt_free_buffer(p_elements_array);
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    uint16_t dst = wiced_bt_mesh_db_get_node_addr(p_mesh_db, component_addr);

    for (int i = 0; i < 2; i++)
    {
        if ((i == 1) && (p_secondary_element == NULL))
            break;

        if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
        {
            memset(p_op, 0, sizeof(pending_operation_t));
            p_op->operation = CONFIG_OPERATION_MODEL_PUBLISH;
            p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
            p_op->uu.model_pub.element_addr = (i == 0) ? component_addr : *p_secondary_element;
            p_op->uu.model_pub.company_id = company_id;
            p_op->uu.model_pub.model_id = model_id;
            p_op->uu.model_pub.publish_addr[0] = target_addr & 0xff;
            p_op->uu.model_pub.publish_addr[1] = (target_addr >> 8) & 0xff;
#ifdef USE_VENDOR_APPKEY
            app_key = (company_id != MESH_COMPANY_ID_BT_SIG) ? wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Vendor") : wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
            p_op->uu.model_pub.app_key_idx = app_key->index;
#else
            app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
            p_op->uu.model_pub.app_key_idx = app_key->index;
#endif
            p_op->uu.model_pub.publish_period = (uint32_t)publish_period;
            p_op->uu.model_pub.publish_ttl = p_cb->publish_ttl;
            p_op->uu.model_pub.publish_retransmit_count = p_cb->publish_retransmit_count;
            p_op->uu.model_pub.publish_retransmit_interval = p_cb->publish_retransmit_interval;
            p_op->uu.model_pub.credential_flag = p_cb->publish_credential_flag;
            configure_pending_operation_queue(p_cb, p_op);

            p_cb->state = PROVISION_STATE_RECONFIGURATION;
            configure_execute_pending_operation(p_cb);
        }
    }
    wiced_bt_free_buffer(p_elements_array);
    return MESH_CLIENT_SUCCESS;

    if ((p_models_array = wiced_bt_mesh_db_get_all_models_of_element(p_mesh_db, component_addr, 0)) == NULL)
        return MESH_CLIENT_ERR_NETWORK_DB;

    for (j = 0; ; j++)
    {
        company_id = p_models_array[j].company_id;
        model_id = p_models_array[j].id;
        if (company_id == MESH_COMPANY_ID_UNUSED)
            break;

        model_element_t* p_model_elem = model_needs_default_pub(company_id, model_id);
        if ((company_id != MESH_COMPANY_ID_BT_SIG) || (p_model_elem && p_model_elem->need_sub_pub))
        {
            if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
            {
                memset(p_op, 0, sizeof(pending_operation_t));
                p_op->operation = CONFIG_OPERATION_MODEL_PUBLISH;
                p_op->p_event = mesh_configure_create_event(dst, (dst != p_cb->unicast_addr));
                p_op->uu.model_pub.element_addr = component_addr;
                p_op->uu.model_pub.company_id = company_id;
                p_op->uu.model_pub.model_id = model_id;
                // if group address is 0, the publication will be a network broadcast
                p_op->uu.model_pub.publish_addr[0] = target_addr & 0xff;
                p_op->uu.model_pub.publish_addr[1] = (target_addr >> 8) & 0xff;
#ifdef USE_VENDOR_APPKEY
                app_key = (company_id != MESH_COMPANY_ID_BT_SIG) ? wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Vendor") : wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
                p_op->uu.model_pub.app_key_idx = app_key->index;
#else
                app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
                p_op->uu.model_pub.app_key_idx = app_key->index;
#endif
                p_op->uu.model_pub.publish_period = 0;              // periodic publication if required will need to be configured separately
                p_op->uu.model_pub.publish_ttl = p_cb->publish_ttl;
                p_op->uu.model_pub.publish_retransmit_count = p_cb->publish_retransmit_count;
                p_op->uu.model_pub.publish_retransmit_interval = p_cb->publish_retransmit_interval;
                p_op->uu.model_pub.credential_flag = p_cb->publish_credential_flag;
                configure_pending_operation_queue(p_cb, p_op);
            }
        }
    }
    wiced_bt_free_buffer(p_models_array);
    if (p_cb->p_first != NULL)
    {
        p_cb->state = PROVISION_STATE_RECONFIGURATION;
        configure_execute_pending_operation(p_cb);
    }
    return MESH_CLIENT_SUCCESS;
}

const char *mesh_client_get_publication_target(const char *component_name, uint8_t is_client, const char *method)
{
    uint16_t *p_elements_array, *p_element;
    uint16_t component_addr = 0;
    uint16_t company_id;
    uint16_t model_id;

    if ((component_name == NULL) || (p_mesh_db == NULL))
        return NULL;

    // go through elements to find out which device is going to be configured for publication
    // and if target is not a group or provisioner, it should be one of the elements.
    if ((p_elements_array = wiced_bt_mesh_db_get_all_elements(p_mesh_db)) == NULL)
        return NULL;

    // find first element of the component with component_name
    for (p_element = p_elements_array; *p_element != 0; p_element++)
    {
        if (is_secondary_element(*p_element))
            continue;

        const char *p_name = get_component_name(*p_element);
        if (strcmp(p_name, component_name) == 0)
        {
            component_addr = *p_element;
        }
    }
    wiced_bt_free_buffer(p_elements_array);

    if (component_addr == 0)
        return NULL;

    uint16_t dst = wiced_bt_mesh_db_get_node_addr(p_mesh_db, component_addr);
    if (is_client)
    {
        if (!get_control_method(method, &company_id, &model_id))
            return NULL;
    }
    else
    {
        if (!get_target_method(method, &company_id, &model_id))
            return NULL;
    }
    uint16_t pub_addr, app_key_idx, publish_retransmit_count;
    uint8_t publish_ttl, credentials;
    uint32_t publish_period, publish_retransmit_interval;

    if (!wiced_bt_mesh_db_node_model_pub_get(p_mesh_db, component_addr, company_id, model_id, &pub_addr, &app_key_idx, &publish_ttl, &publish_period, &publish_retransmit_count, &publish_retransmit_interval, &credentials))
        return NULL;

    if (pub_addr == 0)
        return "none";

    if (!WICED_BT_MESH_IS_GROUP_ADDR(pub_addr))
        return get_component_name(pub_addr);

    switch (pub_addr)
    {
    case 0xffff:
        return "all-nodes";

    case 0xfffc:
        return "all-proxies";

    case 0xfffd:
        return "all_friends";

    case 0xfffe:
        return "all-relays";

    default:
        return wiced_bt_mesh_db_get_group_name(p_mesh_db, pub_addr);
    }
}

int mesh_client_get_publication_period(const char *component_name, uint8_t is_client, const char *method)
{
    uint16_t *p_elements_array, *p_element;
    uint16_t component_addr = 0;
    uint16_t company_id;
    uint16_t model_id;

    if ((component_name == NULL) || (p_mesh_db == NULL))
        return 0;

    // go through elements to find out which device is going to be configured for publication
    // and if target is not a group or provisioner, it should be one of the elements.
    if ((p_elements_array = wiced_bt_mesh_db_get_all_elements(p_mesh_db)) == NULL)
        return 0;

    // find first element of the component with component_name
    for (p_element = p_elements_array; *p_element != 0; p_element++)
    {
        if (is_secondary_element(*p_element))
            continue;

        const char *p_name = get_component_name(*p_element);
        if (strcmp(p_name, component_name) == 0)
        {
            component_addr = *p_element;
        }
    }
    wiced_bt_free_buffer(p_elements_array);

    if (component_addr == 0)
        return 0;

    uint16_t dst = wiced_bt_mesh_db_get_node_addr(p_mesh_db, component_addr);
    if (is_client)
    {
        if (!get_control_method(method, &company_id, &model_id))
            return 0;
    }
    else
    {
        if (!get_target_method(method, &company_id, &model_id))
            return 0;
    }
    uint16_t pub_addr, app_key_idx, publish_retransmit_count;
    uint8_t publish_ttl, credentials;
    uint32_t publish_period, publish_retransmit_interval;

    if (!wiced_bt_mesh_db_node_model_pub_get(p_mesh_db, component_addr, company_id, model_id, &pub_addr, &app_key_idx, &publish_ttl, &publish_period, &publish_retransmit_count, &publish_retransmit_interval, &credentials))
        return 0;

    return publish_period;
}

wiced_bool_t is_model_present_in_models_array(uint16_t company_id, uint16_t model_id, wiced_bt_mesh_db_model_id_t *p_models_array)
{
    wiced_bool_t model_present = WICED_FALSE;
    int j;

    for (j = 0; p_models_array[j].company_id != MESH_COMPANY_ID_UNUSED; j++)
    {
        if ((p_models_array[j].company_id == company_id) &&
            (p_models_array[j].id == model_id))
        {
            model_present = WICED_TRUE;
            break;
        }
    }
    return model_present;
}

wiced_bool_t is_model_present(uint16_t element_addr, uint16_t company_id, uint16_t model_id)
{
    wiced_bt_mesh_db_model_id_t *p_models_array;
    wiced_bool_t model_present = WICED_FALSE;

    if ((p_models_array = wiced_bt_mesh_db_get_all_models_of_element(p_mesh_db, element_addr, 0)) != NULL)
        model_present = is_model_present_in_models_array(company_id, model_id, p_models_array);

    wiced_bt_free_buffer(p_models_array);
    return model_present;
}

wiced_bool_t get_vs_model(uint16_t element_addr, uint16_t company_id, uint16_t model_id)
{
    wiced_bt_mesh_db_model_id_t *p_models_array;
    wiced_bool_t model_present = WICED_FALSE;
    int j;

    if ((p_models_array = wiced_bt_mesh_db_get_all_models_of_element(p_mesh_db, element_addr, 0)) != NULL)
    {
        for (j = 0; p_models_array[j].company_id != MESH_COMPANY_ID_UNUSED; j++)
        {
            if ((p_models_array[j].company_id == company_id) && (p_models_array[j].id == model_id))
            {
                model_present = WICED_TRUE;
                break;
            }
        }
    }
    wiced_bt_free_buffer(p_models_array);
    return model_present;
}

uint16_t *mesh_get_group_list(uint16_t group_addr, uint16_t company_id, uint16_t model_id, uint16_t *num)
{
    int node_idx, element_idx, model_idx, sub_idx;
    uint16_t num_nodes = 0;
    uint16_t *group_list = NULL;
    wiced_bool_t is_subscribed;

    // Go through all nodes in the network
    for (node_idx = 0; node_idx < p_mesh_db->num_nodes; node_idx++)
    {
        // Go through all elements of the node
        for (element_idx = 0; element_idx < p_mesh_db->node[node_idx].num_elements; element_idx++)
        {
            // check if model is present on the element
            for (model_idx = 0; model_idx < p_mesh_db->node[node_idx].element[element_idx].num_models; model_idx++)
            {
                if ((p_mesh_db->node[node_idx].element[element_idx].model[model_idx].model.company_id == company_id) &&
                    (p_mesh_db->node[node_idx].element[element_idx].model[model_idx].model.id == model_id))
                    break;
            }
            // if model not present on the element_idx of node_idx go to the next element
            if (model_idx == p_mesh_db->node[node_idx].element[element_idx].num_models)
                continue;

            // Only top level models require subs. That means that this particular model may not be subsribed, but
            // is still a part of the group. It might not be exactly right, but we will consider element to be
            // a part of the group, if any of the models of the element is subscribed to the group.
            if (group_addr == 0xFFFF)
                num_nodes++;
            else
            {
                is_subscribed = WICED_FALSE;
                for (model_idx = 0; !is_subscribed && (model_idx < p_mesh_db->node[node_idx].element[element_idx].num_models); model_idx++)
                {
                    for (sub_idx = 0; sub_idx < p_mesh_db->node[node_idx].element[element_idx].model[model_idx].num_subs; sub_idx++)
                    {
                        if (p_mesh_db->node[node_idx].element[element_idx].model[model_idx].sub[sub_idx] == group_addr)
                        {
                            num_nodes++;
                            is_subscribed = WICED_TRUE;
                            break;
                        }
                    }
                }
            }
        }
    }
    if (num_nodes == 0)
    {
        Log("no devices in group %x\n", group_addr);
        return NULL;
    }
    group_list = (uint16_t *)wiced_bt_get_buffer(sizeof(uint16_t) * num_nodes);
    if (group_list == NULL)
        return NULL;

    num_nodes = 0;
    // Go through all nodes in the network
    for (node_idx = 0; node_idx < p_mesh_db->num_nodes; node_idx++)
    {
        // Go through all elements of the node
        for (element_idx = 0; element_idx < p_mesh_db->node[node_idx].num_elements; element_idx++)
        {
            // check if model is present on the element
            for (model_idx = 0; model_idx < p_mesh_db->node[node_idx].element[element_idx].num_models; model_idx++)
            {
                if ((p_mesh_db->node[node_idx].element[element_idx].model[model_idx].model.company_id == company_id) &&
                    (p_mesh_db->node[node_idx].element[element_idx].model[model_idx].model.id == model_id))
                    break;
            }
            // if model not present on the element_idx of node_idx go to the next element
            if (model_idx == p_mesh_db->node[node_idx].element[element_idx].num_models)
                continue;

            // Only top level models require subs. That means that this particular model may not be subsribed, but
            // is still a part of the group. It might not be exactly right, but we will consider element to be
            // a part of the group, if any of the models of the element is subscribed to the group.
            if (group_addr == 0xFFFF)
                num_nodes++;
            else
            {
                is_subscribed = WICED_FALSE;
                for (model_idx = 0; !is_subscribed && (model_idx < p_mesh_db->node[node_idx].element[element_idx].num_models); model_idx++)
                {
                    for (sub_idx = 0; sub_idx < p_mesh_db->node[node_idx].element[element_idx].model[model_idx].num_subs; sub_idx++)
                    {
                        if (p_mesh_db->node[node_idx].element[element_idx].model[model_idx].sub[sub_idx] == group_addr)
                        {
                            group_list[num_nodes++] = p_mesh_db->node[node_idx].unicast_address + element_idx;
                            is_subscribed = WICED_TRUE;
                            break;
                        }
                    }
                }
            }
        }
    }
    *num = num_nodes;
    return group_list;
}

/*
 * Identify method sends a message to a device or a group of device to identify itself for a certain duration.
 */
int mesh_client_identify(const char *p_name, uint8_t duration)
{
    uint16_t dst = get_device_addr(p_name);
    uint16_t num_nodes = 0;
    uint16_t *group_list = NULL;
    wiced_bt_mesh_db_app_key_t *app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
    wiced_bt_mesh_health_attention_set_data_t set;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst == 0)
    {
        dst = get_group_addr(p_name);
        if (dst != 0)
        {
            group_list = mesh_get_group_list(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_HEALTH_SRV, &num_nodes);
            if (group_list == NULL)
            {
                Log("onoff model not present in group\n");
                return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
            }
        }
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    wiced_bt_mesh_event_t *p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_HEALTH_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        wiced_bt_free_buffer(group_list);
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    p_event->reply = (num_nodes == 0);
    set.timer = duration;

    Log("Attention Set addr:%04x app_key_idx:%04x duration:%d", p_event->dst, p_event->app_key_idx, duration);

    wiced_bt_mesh_health_attention_set(p_event, &set);
    return MESH_CLIENT_SUCCESS;
}

/*
 * This function can be called to encrypt OTA FW upgrade commands and data. Note that output buffer should be at least 17 bytes larger
 * than input data length.  Function returns the size of the data to be transfered over the air using OTA_FW_UPDATE_COMMAND or _DATA handle
 */
uint16_t mesh_client_ota_data_encrypt(const char *component_name, const uint8_t *p_in_data, uint16_t in_data_len, uint8_t *p_out_buf, uint16_t out_buf_len)
{
    return wiced_bt_mesh_core_crypt(WICED_TRUE, p_in_data, in_data_len, p_out_buf, out_buf_len);
}

/*
 * This function can be called to decrypt OTA FW upgrade event received from the peer device. Function returns the size of the data to be
 * processed (typically 1 byte).
 */
uint16_t mesh_client_ota_data_decrypt(const char *component_name, const uint8_t *p_in_data, uint16_t in_data_len, uint8_t *p_out_buf, uint16_t out_buf_len)
{
    return wiced_bt_mesh_core_crypt(WICED_FALSE, p_in_data, in_data_len, p_out_buf, out_buf_len);
}

/*
 * Get On/Off state of a device or all devices in a group
 */
int mesh_client_on_off_get(const char *p_name)
{
    uint16_t dst = get_device_addr(p_name);
    uint16_t num_nodes = 0;
    uint16_t *group_list = NULL;
    wiced_bt_mesh_db_app_key_t *app_key;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst != 0)
    {
        if (!is_model_present(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_SRV))
        {
            Log("onoff model not present\n");
            return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
        }
    }
    else
    {
        dst = get_group_addr(p_name);
        if (dst != 0)
        {
            group_list = mesh_get_group_list(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_SRV, &num_nodes);
            if (group_list == NULL)
            {
                Log("onoff model not present in group\n");
                return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
            }
        }
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        wiced_bt_free_buffer(group_list);
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    wiced_bt_mesh_event_t *p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("onoff get no mem\n");
        wiced_bt_free_buffer(group_list);
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    Log("OnOff Get addr:%04x app_key_idx:%04x", p_event->dst, p_event->app_key_idx);
    wiced_bt_mesh_model_onoff_client_send_get(p_event);
    return MESH_CLIENT_SUCCESS;
}

/*
 * Set On/Off state of a device or a group
 */
int mesh_client_on_off_set(const char *p_name, uint8_t on_off, wiced_bool_t reliable, uint32_t transition_time, uint16_t delay)
{
    wiced_bt_mesh_onoff_set_data_t set_data;
    uint16_t dst = get_device_addr(p_name);
    uint16_t num_nodes = 0;
    uint16_t *group_list = NULL;
    wiced_bt_mesh_db_app_key_t *app_key;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst != 0)
    {
        if (!is_model_present(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_SRV))
        {
            Log("onoff model not present\n");
            return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
        }
    }
    else
    {
        dst = get_group_addr(p_name);
        if (dst != 0)
        {
            group_list = mesh_get_group_list(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_SRV, &num_nodes);
            if (group_list == NULL)
            {
                Log("onoff model not present in group\n");
                return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
            }
            if (num_nodes == 1)
                dst = group_list[0];

            wiced_bt_free_buffer(group_list);
        }
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    wiced_bt_mesh_event_t *p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("onoff get no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    p_event->reply = reliable;
    set_data.onoff = on_off;
    set_data.transition_time = transition_time;
    set_data.delay = delay;

    Log("OnOff Set addr:%04x app_key_idx:%04x reply:%d onoff:%d transition_time:%d delay:%d", p_event->dst, p_event->app_key_idx, p_event->reply, set_data.onoff, set_data.transition_time, set_data.delay);

    wiced_bt_mesh_model_onoff_client_send_set(p_event, &set_data);
    return MESH_CLIENT_SUCCESS;
}

/*
 * Get current state of Dimmable light
 */
int mesh_client_level_get(const char *p_name)
{
    uint16_t dst = get_device_addr(p_name);
    uint16_t num_nodes = 0;
    uint16_t *group_list = NULL;
    wiced_bt_mesh_db_app_key_t *app_key;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst != 0)
    {
        if (!is_model_present(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_SRV))
        {
            Log("level model not present\n");
            return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
        }
    }
    else
    {
        dst = get_group_addr(p_name);
        if (dst != 0)
        {
            group_list = mesh_get_group_list(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_SRV, &num_nodes);
            if (group_list == NULL)
            {
                Log("level model not present in group\n");
                return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
            }
            if (num_nodes == 1)
                dst = group_list[0];

            wiced_bt_free_buffer(group_list);
        }
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    wiced_bt_mesh_event_t *p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("onoff get no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    Log("Level Get addr:%04x app_key_idx:%04x", p_event->dst, p_event->app_key_idx);

    wiced_bt_mesh_model_level_client_send_get(p_event);
    return MESH_CLIENT_SUCCESS;
}

/*
 * Set level
 */
int mesh_client_level_set(const char *p_name, int16_t level, wiced_bool_t reliable, uint32_t transition_time, uint16_t delay)
{
    wiced_bt_mesh_level_set_level_t set_data;
    uint16_t dst = get_device_addr(p_name);
    uint16_t num_nodes = 0;
    uint16_t *group_list = NULL;
    wiced_bt_mesh_db_app_key_t *app_key;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst != 0)
    {
        if (!is_model_present(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_SRV))
        {
            Log("level model not present\n");
            return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
        }
    }
    else
    {
        dst = get_group_addr(p_name);
        if (dst != 0)
        {
            group_list = mesh_get_group_list(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_SRV, &num_nodes);
            if (group_list == NULL)
            {
                Log("level model not present in group\n");
                return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
            }
            if (num_nodes == 1)
                dst = group_list[0];

            wiced_bt_free_buffer(group_list);
        }
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    wiced_bt_mesh_event_t *p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("onoff get no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    p_event->reply = reliable;
    set_data.level = level;
    set_data.transition_time = transition_time;
    set_data.delay = delay;

    Log("Level Set addr:%04x app_key_idx:%04x reply:%d level:%d transition_time:%d delay:%d", p_event->dst, p_event->app_key_idx, p_event->reply, set_data.level, set_data.transition_time, set_data.delay);

    wiced_bt_mesh_model_level_client_send_set(p_event, &set_data);
    return MESH_CLIENT_SUCCESS;
}

/*
 * Increase or decrease the level
 */
int mesh_client_level_delta_set(const char *p_name, int32_t delta, wiced_bool_t continuation, wiced_bool_t reliable, uint32_t transition_time, uint16_t delay)
{
    wiced_bt_mesh_level_set_delta_t data;
    uint16_t dst = get_device_addr(p_name);
    uint16_t num_nodes = 0;
    uint16_t *group_list = NULL;
    wiced_bt_mesh_db_app_key_t *app_key;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst != 0)
    {
        if (!is_model_present(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_SRV))
        {
            Log("level model not present\n");
            return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
        }
    }
    else
    {
        dst = get_group_addr(p_name);
        if (dst != 0)
        {
            group_list = mesh_get_group_list(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_SRV, &num_nodes);
            if (group_list == NULL)
            {
                Log("level model not present in group\n");
                return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
            }
            if (num_nodes == 1)
                dst = group_list[0];

            wiced_bt_free_buffer(group_list);
        }
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    wiced_bt_mesh_event_t *p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("onoff get no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    p_event->reply = reliable;
    data.delta = delta;
    data.transition_time = transition_time;
    data.delay = delay;
    data.continuation = (uint8_t)continuation;

    Log("Level Delta Set addr:%04x app_key_idx:%04x reply:%d delta:%d continuation:%d transition_time:%d delay:%d", p_event->dst, p_event->app_key_idx, p_event->reply, data.delta, data.continuation, data.transition_time, data.delay);

    wiced_bt_mesh_model_level_client_send_delta_set(p_event, &data);
    return MESH_CLIENT_SUCCESS;
}


/*
 * Get current state of light
 */
int mesh_client_lightness_get(const char *p_name)
{
    uint16_t dst = get_device_addr(p_name);
    uint16_t num_nodes = 0;
    uint16_t *group_list = NULL;
    wiced_bt_mesh_db_app_key_t *app_key;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst != 0)
    {
        if (!is_model_present(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_SRV))
        {
            Log("lightness model not present\n");
            return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
        }
    }
    else
    {
        dst = get_group_addr(p_name);
        if (dst != 0)
        {
            group_list = mesh_get_group_list(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_SRV, &num_nodes);
            if (group_list == NULL)
            {
                Log("lightness model not present in group\n");
                return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
            }
            if (num_nodes == 1)
                dst = group_list[0];

            wiced_bt_free_buffer(group_list);
        }
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    wiced_bt_mesh_event_t *p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("lightness get no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    p_event->reply = WICED_TRUE;

    Log("Lightness Get addr:%04x app_key_idx:%04x", p_event->dst, p_event->app_key_idx);

    wiced_bt_mesh_model_light_lightness_client_send_get(p_event);
    return MESH_CLIENT_SUCCESS;
}

/*
 * Set state of HSL light
 */
int mesh_client_lightness_set(const char *p_name, uint16_t lightness, wiced_bool_t reliable, uint32_t transition_time, uint16_t delay)
{
    wiced_bt_mesh_light_lightness_actual_set_t set_data;
    uint16_t dst = get_device_addr(p_name);
    uint16_t num_nodes = 0;
    uint16_t *group_list = NULL;
    wiced_bt_mesh_db_app_key_t *app_key;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst != 0)
    {
        if (!is_model_present(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_SRV))
        {
            Log("lightness model not present\n");
            return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
        }
    }
    else
    {
        dst = get_group_addr(p_name);
        if (dst != 0)
        {
            group_list = mesh_get_group_list(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_SRV, &num_nodes);
            if (group_list == NULL)
            {
                Log("lightness model not present in group\n");
                return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
            }
            if (num_nodes == 1)
                dst = group_list[0];

            wiced_bt_free_buffer(group_list);
        }
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    wiced_bt_mesh_event_t *p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("lightness get no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    p_event->reply = reliable;
    set_data.lightness_actual = lightness;
    set_data.transition_time = transition_time;
    set_data.delay = delay;

    Log("Lightness Set addr:%04x app_key_idx:%04x reply:%d lightness:%d transition_time:%d delay:%d", p_event->dst, p_event->app_key_idx, p_event->reply, set_data.lightness_actual, set_data.transition_time, set_data.delay);

//    wiced_bt_mesh_model_light_lightness_client_send_linear_set(p_event, (wiced_bt_mesh_light_lightness_linear_set_t *)&set_data);
    wiced_bt_mesh_model_light_lightness_client_send_set(p_event, &set_data);
    return MESH_CLIENT_SUCCESS;
}


/*
 * Get current state of HSL light
 */
int mesh_client_hsl_get(const char *p_name)
{
    uint16_t dst = get_device_addr(p_name);
    uint16_t num_nodes = 0;
    uint16_t *group_list = NULL;
    wiced_bt_mesh_db_app_key_t *app_key;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst != 0)
    {
        if (!is_model_present(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SRV))
        {
            Log("hsl model not present\n");
            return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
        }
    }
    else
    {
        dst = get_group_addr(p_name);
        if (dst != 0)
        {
            group_list = mesh_get_group_list(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SRV, &num_nodes);
            if (group_list == NULL)
            {
                Log("hsl model not present in group\n");
                return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
            }
            if (num_nodes == 1)
                dst = group_list[0];

            wiced_bt_free_buffer(group_list);
        }
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    wiced_bt_mesh_event_t *p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("hsl get no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    p_event->reply = WICED_TRUE;

    Log("HSL Get addr:%04x app_key_idx:%04x", p_event->dst, p_event->app_key_idx);

    wiced_bt_mesh_model_light_hsl_client_send_get(p_event);
    return MESH_CLIENT_SUCCESS;
}

/*
 * Set state of HSL light
 */
int mesh_client_hsl_set(const char *p_name, uint16_t lightness, uint16_t hue, uint16_t saturation, wiced_bool_t reliable, uint32_t transition_time, uint16_t delay)
{
    wiced_bt_mesh_light_hsl_set_t set_data;
    uint16_t dst = get_device_addr(p_name);
    uint16_t num_nodes = 0;
    uint16_t *group_list = NULL;
    wiced_bt_mesh_db_app_key_t *app_key;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst != 0)
    {
        if (!is_model_present(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SRV))
        {
            Log("hsl model not present\n");
            return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
        }
    }
    else
    {
        dst = get_group_addr(p_name);
        if (dst != 0)
        {
            group_list = mesh_get_group_list(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SRV, &num_nodes);
            if (group_list == NULL)
            {
                Log("hsl model not present in group\n");
                return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
            }
            if (num_nodes == 1)
                dst = group_list[0];

            wiced_bt_free_buffer(group_list);
        }
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    wiced_bt_mesh_event_t *p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("hsl get no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    p_event->reply = reliable;
    set_data.target.lightness = lightness;
    set_data.target.hue = hue;
    set_data.target.saturation = saturation;
    set_data.transition_time = transition_time;
    set_data.delay = delay;

    Log("HSL Set addr:%04x app_key_idx:%04x reply:%d lightness:%d hue:%d saturation:%d transition_time:%d delay:%d", p_event->dst, p_event->app_key_idx, p_event->reply, set_data.target.lightness, set_data.target.hue, set_data.target.saturation, set_data.transition_time, set_data.delay);

    wiced_bt_mesh_model_light_hsl_client_send_set(p_event, &set_data);
    return MESH_CLIENT_SUCCESS;
}

/*
 * Get current state of CTL light
 */
int mesh_client_ctl_get(const char *p_name)
{
    uint16_t dst = get_device_addr(p_name);
    uint16_t num_nodes = 0;
    uint16_t *group_list = NULL;
    wiced_bt_mesh_db_app_key_t *app_key;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst != 0)
    {
        if (!is_model_present(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_SRV))
        {
            Log("ctl model not present\n");
            return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
        }
    }
    else
    {
        dst = get_group_addr(p_name);
        if (dst != 0)
        {
            group_list = mesh_get_group_list(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_SRV, &num_nodes);
            if (group_list == NULL)
            {
                Log("ctl model not present in group\n");
                return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
            }
            if (num_nodes == 1)
                dst = group_list[0];

            wiced_bt_free_buffer(group_list);
        }
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    wiced_bt_mesh_event_t *p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("ctl get no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    p_event->reply = WICED_TRUE;

    Log("CTL Get addr:%04x app_key_idx:%04x", p_event->dst, p_event->app_key_idx);

    wiced_bt_mesh_model_light_ctl_client_send_get(p_event);
    return MESH_CLIENT_SUCCESS;
}

/*
 * Set state of CTL light
 */
int mesh_client_ctl_set(const char *p_name, uint16_t lightness, uint16_t temperature, uint16_t delta_uv, wiced_bool_t reliable, uint32_t transition_time, uint16_t delay)
{
    wiced_bt_mesh_light_ctl_set_t set_data;
    uint16_t dst = get_device_addr(p_name);
    uint16_t num_nodes = 0;
    uint16_t *group_list = NULL;
    wiced_bt_mesh_db_app_key_t *app_key;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst != 0)
    {
        if (!is_model_present(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_SRV))
        {
            Log("ctl model not present\n");
            return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
        }
    }
    else
    {
        dst = get_group_addr(p_name);
        if (dst != 0)
        {
            group_list = mesh_get_group_list(dst, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_SRV, &num_nodes);
            if (group_list == NULL)
            {
                Log("ctl model not present in group\n");
                return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
            }
            if (num_nodes == 1)
                dst = group_list[0];

            wiced_bt_free_buffer(group_list);
        }
    }
    if (dst == 0)
    {
        Log("device not found in DB\n");
        return MESH_CLIENT_ERR_NOT_FOUND;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }
    wiced_bt_mesh_event_t *p_event = mesh_create_control_event(p_mesh_db, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_CLNT, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("ctl set no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    p_event->reply = reliable;
    set_data.target.lightness = lightness;
    set_data.target.temperature = temperature;
    set_data.target.delta_uv = delta_uv;
    set_data.transition_time = transition_time;
    set_data.delay = delay;

    Log("CTL Set addr:%04x app_key_idx:%04x reply:%d lightness:%d temperature:%d delta_uv:%d transition_time:%d delay:%d", p_event->dst, p_event->app_key_idx, p_event->reply, set_data.target.lightness, set_data.target.temperature, set_data.target.delta_uv, set_data.transition_time, set_data.delay);

    wiced_bt_mesh_model_light_ctl_client_send_set(p_event, &set_data);
    return MESH_CLIENT_SUCCESS;
}

int mesh_client_core_adv_tx_power_set(uint8_t adv_tx_power)
{
    Log("mesh_client_core_adv_tx_power_set called. tx_power:%d\n", adv_tx_power);
#ifdef CLIENTCONTROL
    wiced_bt_mesh_adv_tx_power_set(adv_tx_power);
#endif
    return MESH_CLIENT_SUCCESS;
}

int mesh_client_add_vendor_model(uint16_t company_id, uint16_t model_id, uint8_t num_opcodes, uint8_t *buffer, uint16_t data_len)
{
    mesh_provision_cb_t* p_cb = &provision_cb;
    wiced_bt_mesh_add_vendor_model_data_t vendata;
    wiced_bt_mesh_db_app_key_t* app_key;

    Log("mesh_client_add_vendor_model called\n");

    memset(&vendata, 0, sizeof(wiced_bt_mesh_add_vendor_model_data_t));
    vendata.company_id = company_id;
    vendata.model_id = model_id;
    vendata.num_opcodes = num_opcodes;
    if (num_opcodes > WICED_BT_MESH_MAX_VENDOR_MODEL_OPCODES)
        return MESH_CLIENT_ERR_INVALID_ARGS;

    memcpy(vendata.opcode, buffer, 3 * num_opcodes);

    wiced_bt_mesh_add_vendor_model(&vendata);

#ifdef USE_VENDOR_APPKEY
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Vendor");
#else
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
#endif
    model_app_bind(p_cb, WICED_TRUE, p_cb->unicast_addr, company_id, model_id, app_key->index);
    configure_execute_pending_operation(p_cb);
    return MESH_CLIENT_SUCCESS;
}


int mesh_client_vendor_data_set(const char *device_name, uint16_t company_id, uint16_t model_id, uint8_t opcode, wiced_bool_t disable_ntwk_retransmit, uint8_t *buffer, uint16_t data_len)
{
    int i;
    uint16_t dst = get_device_addr(device_name);
    uint16_t len;
    wiced_bt_mesh_db_app_key_t *app_key;

    if (p_mesh_db == NULL)
    {
        Log("Network closed\n");
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }
#ifdef USE_VENDOR_APPKEY
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Vendor");
#else
    app_key = wiced_bt_mesh_db_app_key_get_by_name(p_mesh_db, "Generic");
#endif
    if (app_key == NULL)
    {
        Log("Key not configured\n");
        return MESH_CLIENT_ERR_NETWORK_DB;
    }
    if (dst == 0)
    {
        if ((dst = get_group_addr(device_name)) == 0)
        {
            Log("device not found in DB\n");
            return MESH_CLIENT_ERR_NOT_FOUND;
        }
    }
    if (!get_vs_model(dst, company_id, model_id))
    {
        Log("vendor model not present\n");
        return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
    }
    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }

    char buf[100];
    uint8_t *p = buffer;
    len = data_len;

    while (len != 0)
    {
        buf[0] = 0;
        for (i = 0; i < len && i < 32; i++)
            sprintf(&buf[strlen(buf)], "%02x ", p[i]);

        len -= i;
        if (len != 0)
            Log(buf);
    }
    Log(buf);

    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_create_event(0, company_id, model_id, dst, app_key->index);
    if (p_event == NULL)
    {
        Log("vendor set no mem\n");
        return MESH_CLIENT_ERR_NO_MEMORY;
    }
    p_event->opcode = opcode;

    // Flag 0x80 means that the there will be no network retransmissions.
    if(disable_ntwk_retransmit)
        p_event->retrans_cnt = 0x80;

    Log("Send VS Data addr:%04x app_key_idx:%04x company_id:%04x model_id:%04x opcode:%02x data_len:%d",
        p_event->dst, p_event->app_key_idx, company_id, model_id, opcode, data_len);

    wiced_bt_mesh_client_vendor_data(p_event, buffer, data_len);
    return MESH_CLIENT_SUCCESS;
}


model_element_t* model_needs_default_pub(uint16_t company_id, uint16_t model_id)
{
    int i;
    for (i = 0; i < sizeof(models_configured_for_pub) / sizeof(models_configured_for_pub[0]); i++)
    {
        if ((models_configured_for_pub[i].company_id == company_id) &&
            (models_configured_for_pub[i].model_id == model_id))
        {
            return &models_configured_for_pub[i];
        }
    }
    return NULL;
}

model_element_t* model_needs_default_sub(uint16_t company_id, uint16_t model_id)
{
    int i;
    for (i = 0; i < sizeof(models_configured_for_sub) / sizeof(models_configured_for_sub[0]); i++)
    {
        if ((models_configured_for_sub[i].company_id == company_id) &&
            (models_configured_for_sub[i].model_id == model_id))
        {
            return &models_configured_for_sub[i];
        }
    }
    return NULL;
}

void mesh_process_health_attention_status(wiced_bt_mesh_event_t *p_event, void *p)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    wiced_bt_mesh_health_attention_status_data_t *p_data = (wiced_bt_mesh_health_attention_status_data_t *)p;

    Log("Attention Status from:%x AppKeyIdx:%x Element:%x timer:%d\n", p_event->src, p_event->app_key_idx, p_event->element_idx, p_data->timer);
    wiced_bt_mesh_release_event(p_event);
}

void mesh_process_properties_status(wiced_bt_mesh_event_t *p_event, void *p)
{
    char buf[200];
    mesh_provision_cb_t *p_cb = &provision_cb;
    wiced_bt_mesh_properties_status_data_t *p_data = (wiced_bt_mesh_properties_status_data_t *)p;
    int i;

    sprintf(buf, "Properties List from:%x AppKeyIdx:%x Element:%x Type:%d Num Props:%d", p_event->src, p_event->app_key_idx, p_event->element_idx, p_data->type, p_data->properties_num);
    for (i = 0; p_data->properties_num; i++)
        sprintf(&buf[strlen(buf)], "%02x ", p_data->id[i]);

    Log(buf);
    wiced_bt_mesh_release_event(p_event);
}

void mesh_process_property_status(wiced_bt_mesh_event_t *p_event, void *p)
{
    char buf[200];
    mesh_provision_cb_t *p_cb = &provision_cb;
    wiced_bt_mesh_property_status_data_t *p_data = (wiced_bt_mesh_property_status_data_t *)p;
    int i;

    sprintf(buf, "Property Status from:%x AppKeyIdx:%x Element:%x Type:%d Prop ID:%d Len:%d ", p_event->src, p_event->app_key_idx, p_event->element_idx, p_data->type, p_data->id, p_data->len);
    for (i = 0; ((i < p_data->len) && (i < WICED_BT_MESH_PROPERTY_LEN_DEVICE_FIRMWARE_REVISION)); i++)
        sprintf(&buf[strlen(buf)], "%02x ", p_data->value[i]);

    Log(buf);

    if ((p_data->id == WICED_BT_MESH_PROPERTY_DEVICE_FIRMWARE_REVISION) && (provision_cb.p_component_info_status_callback != NULL))
    {
        wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_addr(p_mesh_db, p_event->src);
        if (node == NULL)
            return;

        sprintf(buf, "CID:%05d PID:%05d VID:%05d VER:", node->cid, node->pid, node->vid);
        for (i = 0; ((i < p_data->len) && (i < WICED_BT_MESH_PROPERTY_LEN_DEVICE_FIRMWARE_REVISION)); i++)
            buf[34 + i] = p_data->value[i];
        buf[34 + i] = 0;

        provision_cb.p_component_info_status_callback(0, (char *)wiced_bt_mesh_db_get_element_name(p_mesh_db, p_event->src), buf);
        provision_cb.p_component_info_status_callback = NULL;
    }
    wiced_bt_mesh_release_event(p_event);
}

void mesh_process_light_lc_mode_status(wiced_bt_mesh_event_t* p_event, void* p)
{
    char buf[200];
    mesh_provision_cb_t* p_cb = &provision_cb;
    wiced_bt_mesh_light_lc_mode_set_data_t* p_data = (wiced_bt_mesh_light_lc_mode_set_data_t*)p;

    sprintf(buf, "Light LC Mode Status from:%x AppKeyIdx:%x Element:%x Mode:%d", p_event->src, p_event->app_key_idx, p_event->element_idx, p_data->mode);
    Log(buf);

    if (p_cb->p_light_lc_mode_status != NULL)
    {
        p_cb->p_light_lc_mode_status((char*)wiced_bt_mesh_db_get_element_name(p_mesh_db, p_event->src), p_data->mode);
        p_cb->p_light_lc_mode_status = NULL;
    }
    wiced_bt_mesh_release_event(p_event);
}

void mesh_process_light_lc_occupancy_mode_status(wiced_bt_mesh_event_t* p_event, void* p)
{
    char buf[200];
    mesh_provision_cb_t* p_cb = &provision_cb;
    wiced_bt_mesh_light_lc_occupancy_mode_set_data_t* p_data = (wiced_bt_mesh_light_lc_occupancy_mode_set_data_t*)p;

    sprintf(buf, "Light LC Occupancy Status from:%x AppKeyIdx:%x Element:%x Mode:%d", p_event->src, p_event->app_key_idx, p_event->element_idx, p_data->mode);
    Log(buf);

    if (p_cb->p_light_lc_occupancy_mode_status != NULL)
    {
        p_cb->p_light_lc_occupancy_mode_status((char*)wiced_bt_mesh_db_get_element_name(p_mesh_db, p_event->src), p_data->mode);
        p_cb->p_light_lc_occupancy_mode_status = NULL;
    }
    wiced_bt_mesh_release_event(p_event);
}

void mesh_process_light_lc_property_status(wiced_bt_mesh_event_t* p_event, void* p)
{
    char buf[200];
    mesh_provision_cb_t* p_cb = &provision_cb;
    wiced_bt_mesh_light_lc_property_status_data_t* p_data = (wiced_bt_mesh_light_lc_property_status_data_t*)p;

    sprintf(buf, "Light LC Property Status from:%x AppKeyIdx:%x Element:%x Prop ID:%d Len:%d ", p_event->src, p_event->app_key_idx, p_event->element_idx, p_data->id, p_data->len);
    Log(buf);

    uint16_t property_len = wiced_bt_mesh_db_get_property_value_len(p_data->id);
    int property_value = 0;

    switch (property_len)
    {
    case 4:
        property_value = (p_data->value[3] << 24) + (p_data->value[2] << 16) + (p_data->value[1] << 8) + p_data->value[0];
        break;
    case 3:
        property_value = (p_data->value[2] << 16) + (p_data->value[1] << 8) + p_data->value[0];
        break;
    case 2:
        property_value = (p_data->value[1] << 8) + p_data->value[0];
        break;
    case 1:
        property_value = p_data->value[0];
        break;
    }
    if (p_cb->p_light_lc_property_status != NULL)
    {
        p_cb->p_light_lc_property_status((char*)wiced_bt_mesh_db_get_element_name(p_mesh_db, p_event->src), p_data->id, property_value);
        p_cb->p_light_lc_property_status = NULL;
    }
    wiced_bt_mesh_release_event(p_event);
}

void mesh_process_scan_capabilities_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_scan_capabilities_status_data_t *p_data)
{
    wiced_bt_mesh_provision_scan_start_data_t data;

    uint16_t provisioner_addr = p_event->src;
    Log("ScanInfo from:%x MaxList:%d\n", provisioner_addr, p_data->max_scanned_items);

    wiced_bt_mesh_release_event(p_event);

    memset(&data, 0, sizeof(data));
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_addr(p_mesh_db, provisioner_addr);
    if (node == NULL)
        return;

    node->active_scan_supported = p_data->active_scan_supported;

    if ((p_event = mesh_configure_create_event(provisioner_addr, (provisioner_addr != p_cb->unicast_addr))) == NULL)
        return;

    node->scanning_state = SCANNING_STATE_STARTING;

    Log("ScanStart addr:%04x", provisioner_addr);

    mesh_configure_set_local_device_key(provisioner_addr);

    memset(&data, 0, sizeof(data));

    if (p_cb->scan_one_uuid)
    {
        data.scanned_items_limit = 1;
        data.scan_single_uuid = 1;
        memcpy(data.uuid, p_cb->uuid, MESH_DEVICE_UUID_LEN);
    }
    else
        data.scanned_items_limit = 0; //  p_data->max_scanned_items;
    data.timeout = SCAN_DURATION_MAX;

    wiced_bt_mesh_provision_scan_start(p_event, &data);
}

void mesh_process_scan_status(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_scan_status_data_t *p_data)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_addr(p_mesh_db, p_event->src);
    if (node == NULL)
        return;

    Log("ScanStatus from:%x Status:%d State:%x Limit:%d Timeout:%d\n", p_event->src, p_data->status, p_data->state, p_data->scanned_items_limit, p_data->timeout);

    node->scanning_state = (p_data->state == WICED_BT_MESH_REMOTE_PROVISIONING_SERVER_NO_SCANNING) ? SCANNING_STATE_IDLE : SCANNING_STATE_SCANNING;

    wiced_bt_mesh_release_event(p_event);
}

void mesh_process_scan_report(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_scan_report_data_t *p_data)
{
    unprovisioned_report_t *p_report;
    unprovisioned_report_t *p_temp;

    Log("ScanReport from:%04x UUID %02x-%02x RSSI:%d\n", p_event->src, p_data->uuid[0], p_data->uuid[15], p_data->rssi);

    if (p_cb->p_first_unprovisioned == NULL)
    {
        p_cb->p_first_unprovisioned = (unprovisioned_report_t *)wiced_bt_get_buffer(sizeof(unprovisioned_report_t));
        if (p_cb->p_first_unprovisioned == NULL)
            return;
        memset(p_cb->p_first_unprovisioned, 0, sizeof(unprovisioned_report_t));
        p_report = p_cb->p_first_unprovisioned;
    }
    else
    {
        // if UUID has been reported from that server, ignore
        for (p_report = p_cb->p_first_unprovisioned; p_report != NULL; p_report = p_report->p_next)
        {
            if (memcmp(p_report->uuid, p_data->uuid, sizeof(p_data->uuid)) == 0)
            {
                if (p_report->provisioner_addr == p_event->src)
                {
                    Log("ScanReport from:%x already present\n", p_event->src);
                    wiced_bt_mesh_release_event(p_event);
                    return;
                }
            }
        }
        // If this report was received not from local device and now is received from local device, replace the entry
        // if this report comes with better RSSI, replace the entry
        for (p_report = p_cb->p_first_unprovisioned; p_report != NULL; p_report = p_report->p_next)
        {
            if (memcmp(p_report->uuid, p_data->uuid, sizeof(p_data->uuid)) == 0)
            {
#if PROVISION_BY_LOCAL_IF_POSSIBLE
                if (p_event->src == p_cb->unicast_addr)
#endif
#if PROVISION_BY_REMOTE_IF_POSSIBLE
                if (p_event->src != p_cb->unicast_addr)
#endif
#if PROVISION_BY_BEST_RSSI
                // use new provisioner if original report was from local device or the report was with worse RSSI
                if ((p_report->provisioner_addr == p_cb->unicast_addr) || (p_data->rssi > p_report->rssi))
#endif
                {
                    p_report->provisioner_addr = p_event->src;
                    p_report->oob = p_data->oob;
                    p_report->rssi = p_data->rssi;
                    Log("ScanReport from:%x rssi:%d\n", p_event->src, p_data->rssi);
                    wiced_bt_mesh_release_event(p_event);
                    return;
                }
                else
                    Log("ScanReport from:%x rssi:%d worse\n", p_event->src, p_data->rssi);

                wiced_bt_mesh_release_event(p_event);
                return;
            }
        }
        p_temp = p_cb->p_first_unprovisioned;
        p_cb->p_first_unprovisioned = (unprovisioned_report_t *)wiced_bt_get_buffer(sizeof(unprovisioned_report_t));
        if (p_cb->p_first_unprovisioned == NULL)
        {
            Log("ScanReport no mem\n");
            wiced_bt_mesh_release_event(p_event);
            return;
        }
        memset(p_cb->p_first_unprovisioned, 0, sizeof(unprovisioned_report_t));
        p_cb->p_first_unprovisioned->p_next = p_temp;
        p_report = p_cb->p_first_unprovisioned;
    }
    memcpy(p_report->uuid, p_data->uuid, sizeof(p_data->uuid));
    p_report->provisioner_addr = p_event->src;
    p_report->oob = p_data->oob;
    p_report->rssi = p_data->rssi;

    wiced_bt_mesh_release_event(p_event);

    // Wait for 3 more seconds to receive reports from other Remote Provisioning Servers
    // Note that Client Control does not use timers
    wiced_init_timer(&p_report->scan_timer, scan_timer_cb, p_report, WICED_SECONDS_TIMER);
    if (wiced_start_timer(&p_report->scan_timer, 2) != WICED_BT_SUCCESS)
        mesh_client_start_extended_scan(p_cb, p_report);
}

void mesh_process_scan_extended_report(mesh_provision_cb_t *p_cb, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_scan_extended_report_data_t *p_data)
{
    Log("ScanExtendedReport from:%x status:%d\n", p_event->src, p_data->status);
    unprovisioned_report_t *p_report;
    uint8_t *p_adv_data;
    uint8_t *p_name = NULL;
    uint8_t name_len = 0;

    if (p_data->status == 0)
    {
        p_adv_data = p_data->adv_data;
        while (p_adv_data[0] != 0)
        {
            if ((p_adv_data[1] == BTM_BLE_ADVERT_TYPE_NAME_SHORT) || (p_adv_data[1] == BTM_BLE_ADVERT_TYPE_NAME_COMPLETE))
            {
                p_name = &p_adv_data[2];
                name_len = p_adv_data[0] - 1;
                break;
            }
            p_adv_data += *p_adv_data + 1;
        }

        for (p_report = p_cb->p_first_unprovisioned; p_report != NULL; p_report = p_report->p_next)
        {
            if (memcmp(p_report->uuid, p_data->uuid, sizeof(p_data->uuid)) == 0)
            {
                if (p_cb->p_unprovisioned_device)
                    p_cb->p_unprovisioned_device(p_report->uuid, p_report->oob, p_name, name_len);
            }
        }
    }
    wiced_bt_mesh_release_event(p_event);
}

void mesh_process_on_off_status(wiced_bt_mesh_event_t *p_event, void *p)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    wiced_bt_mesh_onoff_status_data_t *p_data = (wiced_bt_mesh_onoff_status_data_t *)p;

    Log("OnOff Status from:%x AppKeyIdx:%x Element:%x Present:%d Target:%d RemainingTime:%d", p_event->src, p_event->app_key_idx, p_event->element_idx, p_data->present_onoff, p_data->target_onoff, p_data->remaining_time);

    if (provision_cb.p_onoff_status != NULL)
    {
        provision_cb.p_onoff_status(wiced_bt_mesh_db_get_element_name(p_mesh_db, p_event->src), p_data->present_onoff, p_data->target_onoff, p_data->remaining_time);
    }
    wiced_bt_mesh_release_event(p_event);
}

void mesh_process_level_status(wiced_bt_mesh_event_t *p_event, void *p)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    wiced_bt_mesh_level_status_data_t *p_data = (wiced_bt_mesh_level_status_data_t *)p;

    Log("Level Status from:%x AppKeyIdx:%x Element:%x Present:%d Target:%d RemainingTime:%d", p_event->src, p_event->app_key_idx, p_event->element_idx, p_data->present_level, p_data->target_level, p_data->remaining_time);

    if (provision_cb.p_level_status != NULL)
    {
        provision_cb.p_level_status(wiced_bt_mesh_db_get_element_name(p_mesh_db, p_event->src), p_data->present_level, p_data->target_level, p_data->remaining_time);
    }
    wiced_bt_mesh_release_event(p_event);
}

void mesh_process_lightness_status(wiced_bt_mesh_event_t *p_event, void *p)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    wiced_bt_mesh_light_lightness_status_data_t *p_data = (wiced_bt_mesh_light_lightness_status_data_t *)p;

    Log("Lightness Status from:%x AppKeyIdx:%x idx:%d Present:%d Target:%d RemainingTime:%d", p_event->src, p_event->app_key_idx, p_event->element_idx, p_data->present, p_data->target, p_data->remaining_time);

    if (provision_cb.p_lightness_status != NULL)
    {
        provision_cb.p_lightness_status(wiced_bt_mesh_db_get_element_name(p_mesh_db, p_event->src), p_data->present, p_data->target, p_data->remaining_time);
    }
    wiced_bt_mesh_release_event(p_event);
}

void mesh_process_hsl_status(wiced_bt_mesh_event_t *p_event, void *p)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    wiced_bt_mesh_light_hsl_status_data_t *p_data = (wiced_bt_mesh_light_hsl_status_data_t *)p;

    Log("HSL Status from:%x AppKeyIdx:%x idx:%d Present L:%d H:%d S:%d RemainingTime:%d", p_event->src, p_event->app_key_idx, p_event->element_idx, p_data->present.lightness, p_data->present.hue, p_data->present.saturation, p_data->remaining_time);

    if (provision_cb.p_hsl_status != NULL)
    {
        provision_cb.p_hsl_status(wiced_bt_mesh_db_get_element_name(p_mesh_db, p_event->src), p_data->present.lightness, p_data->present.hue, p_data->present.saturation, p_data->remaining_time);
    }
    wiced_bt_mesh_release_event(p_event);
}

void mesh_process_ctl_status(wiced_bt_mesh_event_t *p_event, void *p)
{
    mesh_provision_cb_t *p_cb = &provision_cb;
    wiced_bt_mesh_light_ctl_status_data_t *p_data = (wiced_bt_mesh_light_ctl_status_data_t *)p;

    Log("CTL Status from:%x AppKeyIdx:%x idx:%d Present L:%d T:%d Target L:%d T:%d RemainingTime:%d", p_event->src, p_event->app_key_idx, p_event->element_idx, p_data->present.lightness, p_data->present.temperature, p_data->target.lightness, p_data->target.temperature, p_data->remaining_time);

    if (provision_cb.p_ctl_status != NULL)
    {
        provision_cb.p_ctl_status(wiced_bt_mesh_db_get_element_name(p_mesh_db, p_event->src), p_data->present.lightness, p_data->present.temperature, p_data->target.lightness, p_data->target.temperature, p_data->remaining_time);
    }
    wiced_bt_mesh_release_event(p_event);
}

wiced_bool_t model_needs_sub(uint16_t model_id, wiced_bt_mesh_db_model_id_t *p_models_array)
{
    if (model_id == WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SETUP_SRV)
        return WICED_TRUE;

    if (model_id == WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV)
        return WICED_FALSE;

    if (model_id == WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LOCATION_SETUP_SRV)
        return WICED_TRUE;

    if (model_id == WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LOCATION_SRV)
        return WICED_FALSE;

    if (is_model_present_in_models_array(MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SETUP_SRV, p_models_array))
    {
        if (is_model_present_in_models_array(MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_SETUP_SRV, p_models_array))
            return ((model_id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SETUP_SRV) || (model_id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_SETUP_SRV));

        if (is_model_present_in_models_array(MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_XYL_SETUP_SRV, p_models_array))
            return ((model_id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SETUP_SRV) || (model_id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_XYL_SETUP_SRV));

        return (model_id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SETUP_SRV);
    }
    if (is_model_present_in_models_array(MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_SETUP_SRV, p_models_array))
        return (model_id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_SETUP_SRV);

    if (is_model_present_in_models_array(MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_XYL_SETUP_SRV, p_models_array))
        return (model_id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_XYL_SETUP_SRV);

    if (is_model_present_in_models_array(MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_HUE_SRV, p_models_array))
        return (model_id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_HUE_SRV);

    if (is_model_present_in_models_array(MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SATURATION_SRV, p_models_array))
        return (model_id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SATURATION_SRV);

    if (is_model_present_in_models_array(MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_TEMPERATURE_SRV, p_models_array))
        return (model_id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_TEMPERATURE_SRV);

    if (is_model_present_in_models_array(MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LC_SETUP_SRV, p_models_array))
        return (model_id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LC_SETUP_SRV);

    if (is_model_present_in_models_array(MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_SETUP_SRV, p_models_array))
        return (model_id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_SETUP_SRV);

    if (is_model_present_in_models_array(MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_POWER_LEVEL_SETUP_SRV, p_models_array))
        return (model_id == WICED_BT_MESH_CORE_MODEL_ID_GENERIC_POWER_LEVEL_SETUP_SRV);

    if (is_model_present_in_models_array(MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_SRV, p_models_array))
        return (model_id == WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_SRV);

    if (is_model_present_in_models_array(MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_SRV, p_models_array))
        return (model_id == WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_SRV);

    return WICED_FALSE;
}

wiced_bool_t is_core_model(uint16_t company_id, uint16_t model_id)
{
    if (company_id != MESH_COMPANY_ID_BT_SIG)
        return WICED_FALSE;

    switch (model_id)
    {
    case WICED_BT_MESH_CORE_MODEL_ID_CONFIG_SRV:
    case WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT:
    case WICED_BT_MESH_CORE_MODEL_ID_HEALTH_SRV:
    case WICED_BT_MESH_CORE_MODEL_ID_HEALTH_CLNT:
    case WICED_BT_MESH_CORE_MODEL_ID_REMOTE_PROVISION_SRV:
    case WICED_BT_MESH_CORE_MODEL_ID_REMOTE_PROVISION_CLNT:
    case WICED_BT_MESH_CORE_MODEL_ID_DIRECTED_FORWARDING_SRV:
    case WICED_BT_MESH_CORE_MODEL_ID_DIRECTED_FORWARDING_CLNT:
#ifdef MESH_DFU_ENABLED
    case WICED_BT_MESH_CORE_MODEL_ID_FW_UPDATE_SRV:
    case WICED_BT_MESH_CORE_MODEL_ID_FW_UPDATE_CLNT:
    case WICED_BT_MESH_CORE_MODEL_ID_FW_DISTRIBUTION_SRV:
    case WICED_BT_MESH_CORE_MODEL_ID_FW_DISTRIBUTION_CLNT:
    case WICED_BT_MESH_CORE_MODEL_ID_BLOB_TRANSFER_SRV:
    case WICED_BT_MESH_CORE_MODEL_ID_BLOB_TRANSFER_CLNT:
#endif
            return WICED_TRUE;
    default:
        return WICED_FALSE;
    }
}

wiced_bool_t is_secondary_element(uint16_t element_addr)
{
    int i;
    wiced_bt_mesh_db_model_id_t *p_models_array = wiced_bt_mesh_db_get_all_models_of_element(p_mesh_db, element_addr, 0);
    wiced_bool_t res = WICED_FALSE;

    if (p_models_array != NULL)
    {
        for (i = 0; p_models_array[i].company_id != MESH_COMPANY_ID_UNUSED; i++)
        {
            if (p_models_array[i].company_id == MESH_COMPANY_ID_BT_SIG)
            {
                if ((p_models_array[i].id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SRV) ||
                    (p_models_array[i].id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_SRV))
                {
                    wiced_bt_free_buffer(p_models_array);
                    return WICED_FALSE;
                }
            }
        }
        for (i = 0; p_models_array[i].company_id != MESH_COMPANY_ID_UNUSED; i++)
        {
            if (p_models_array[i].company_id == MESH_COMPANY_ID_BT_SIG)
            {
                if ((p_models_array[i].id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_HUE_SRV) ||
                    (p_models_array[i].id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SATURATION_SRV) ||
                    (p_models_array[i].id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_TEMPERATURE_SRV) ||
                    (p_models_array[i].id == WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LC_SRV))
                {
                    res = WICED_TRUE;
                    break;
                }
            }
        }
        wiced_bt_free_buffer(p_models_array);
    }
    return res;
}

wiced_bool_t is_provisioner(wiced_bt_mesh_db_node_t *p_node)
{
    int i;
    for (i = 0; i < p_mesh_db->num_provisioners; i++)
    {
        if (memcmp(p_mesh_db->provisioner[i].uuid, p_node->uuid, sizeof(p_node->uuid)) == 0)
            return WICED_TRUE;
    }
    return WICED_FALSE;
}

void rand128(uint8_t *p_array)
{
    int i;
    for (i = 0; i < 16; i++)
        p_array[i] = (uint8_t)rand();
}

uint16_t rand16()
{
    return (uint16_t)rand();
}

wiced_bt_mesh_event_t* mesh_create_control_event(wiced_bt_mesh_db_mesh_t* p_mesh_db, uint16_t company_id, uint16_t model_id, uint16_t dst, uint16_t app_key_idx)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_create_event(0, company_id, model_id, dst, app_key_idx);
    if (p_event == NULL)
        return NULL;

    wiced_bt_mesh_db_node_t* p_node = mesh_find_node(p_mesh_db, dst);

    // For low power node, we will use segmented message to deliver to the Friend. Retransmission is done by the transport.
    if ((p_node != NULL) && p_node->feature.low_power == MESH_FEATURE_ENABLED)
    {
        p_event->retrans_cnt = 0;
        p_event->send_segmented = WICED_TRUE;
    }
    else
    {
        if (WICED_BT_MESH_IS_GROUP_ADDR(dst))     // for group address, do not retry
        {
            p_event->retrans_cnt = 0;
        }
        else
        {
            p_event->retrans_cnt = 2;       // Try 3 times (this is in addition to network layer retransmit)
            p_event->retrans_time  = 6;     // Every 300 msec
            p_event->reply_timeout = 60;    // wait for the reply 3 seconds
        }
    }
    return p_event;
}

void mesh_client_advert_report(uint8_t *bd_addr, uint8_t addr_type, int8_t rssi, uint8_t *adv_data)
{
    wiced_bt_ble_scan_results_t scan_result;

    memcpy(scan_result.remote_bd_addr, bd_addr, 6);
    scan_result.ble_addr_type = addr_type;
    scan_result.ble_evt_type = BTM_BLE_EVT_CONNECTABLE_ADVERTISEMENT;
    scan_result.rssi = rssi;

    if (!wiced_bt_mesh_remote_provisioning_connectable_adv_packet(&scan_result, adv_data))
        wiced_bt_mesh_gatt_client_process_connectable_adv(bd_addr, addr_type, rssi, adv_data);
}

/*
 * When application manages the connection, it can notify GATT Client that connection has been established.
 * That includes GATT connection, client configuration descriptor setting and MTU exchange
 */
void mesh_client_connection_state_changed(uint16_t conn_id, uint16_t mtu)
{
    mesh_provision_cb_t *p_cb = &provision_cb;

    // If connection is with the device connected over UART or over internet.
    // Set the state to Network Connect so that we send Add Filter
    if (conn_id == 0xFFFF)
    {
        if ((p_cb->state != PROVISION_STATE_IDLE) || !p_cb->network_opened || (p_cb->proxy_conn_id != 0))
        {
            Log("Connect Network bad operation state:%d Network opened:%d or connected:%d", p_cb->state, p_cb->network_opened, (p_cb->proxy_conn_id != 0));
            return;
        }
        p_cb->state = PROVISION_STATE_NETWORK_CONNECT;
    }

    // call to gatt_client state change will result in the state machine item, notify RPR server first.
    wiced_bt_mesh_remote_provisioning_connection_state_changed(conn_id, 0);

    wiced_bt_mesh_gatt_client_connection_state_changed(conn_id, mtu);

    if ((conn_id == 0) && (p_cb->state == PROVISION_STATE_NETWORK_CONNECT))
    {
        if (p_cb->p_connect_status != NULL)
            p_cb->p_connect_status(0, 0, 0, 1);

        p_cb->state = PROVISION_STATE_IDLE;
    }
}

/*
 * Return TRUE if connecting to a provisiong service, FALSE if to proxy.
 */
uint8_t mesh_client_is_connecting_provisioning()
{
    return (provision_cb.state == PROVISION_STATE_CONNECTING);
}

/*
 * Check if GATT proxy connection is established
 */
uint8_t mesh_client_is_proxy_connected(void)
{
#ifdef CLIENTCONTROL
    return 1;
#endif
    return (provision_cb.proxy_addr != 0);
}

/*
 * This function should be called whenever IP connection is established with a proxy device
 */
void mesh_client_extern_proxy_connected(uint16_t addr)
{
    provision_cb.proxy_addr = addr;
    provision_cb.proxy_conn_id = addr != 0 ? 0xFFFF : 0;
    wiced_bt_mesh_core_connection_status(addr != 0 ? 0xFFFF : 0, WICED_TRUE, 0, 75);
}

wiced_bool_t add_filter(mesh_provision_cb_t *p_cb, uint16_t addr)
{
    uint16_t i = 0;
    uint8_t num_addr = 1;
    wiced_bt_mesh_proxy_filter_change_addr_data_t* p_filter;
    wiced_bool_t res;

    wiced_bt_mesh_event_t *p_event = mesh_configure_create_event(0xFFFF, WICED_FALSE);
    if (p_event == NULL)
        return WICED_FALSE;

    // if group list is empty just add unicast address. This is not needed, but we use
    // SRC in the Status message to find out ADDR of the Proxy device.
    num_addr = (p_mesh_db->num_groups > 1) ? p_mesh_db->num_groups : 1;

    p_filter = (wiced_bt_mesh_proxy_filter_change_addr_data_t*)wiced_bt_get_buffer((num_addr - 1) * sizeof(uint16_t) + sizeof(wiced_bt_mesh_proxy_filter_change_addr_data_t));
    if (p_filter == NULL)
    {
        wiced_bt_mesh_release_event(p_event);
        return WICED_FALSE;
    }

    p_filter->addr_num = num_addr;

    if (p_mesh_db->num_groups == 0)
    {
        p_filter->addr[0] = addr;
    }
    else
    {
        for (i = 0; i < p_mesh_db->num_groups; i++)
            p_filter->addr[i] = p_mesh_db->group[i].addr;
    }
    Log("Filter add addr:%04x num:%d first addr:%04x", p_event->dst, p_filter->addr_num, p_filter->addr[0]);
    res = wiced_bt_mesh_proxy_filter_change_addr(p_event, WICED_TRUE, p_filter);
    wiced_bt_free_buffer(p_filter);
    return res;
}

int mesh_client_listen_for_app_group_broadcasts(char *control_method, char *group_name, wiced_bool_t start_listen)
{
    uint16_t group_addr;
    uint16_t model_id;
    uint16_t company_id;
    size_t len;
    char* p_groups;
    char* p_control_method;
    char* control_methods;
    char* groups_array;
    int i;
    pending_operation_t *p_op;
    mesh_provision_cb_t *p_cb = &provision_cb;

    if (p_mesh_db == NULL)
        return MESH_CLIENT_ERR_NETWORK_CLOSED;

    if (!mesh_client_is_proxy_connected())
    {
        Log("not connected\n");
        return MESH_CLIENT_ERR_NOT_CONNECTED;
    }

    if (control_method == NULL)
    {
        //the control_method parameter is NULL, the library will register to receive messages for all types of messages.
        const char* name = get_component_name(p_mesh_db->unicast_addr);
        control_methods = mesh_client_get_control_methods(name);
    }
    else
    {
        //check if control method is valid
        if (!get_control_method(control_method, &company_id, &model_id))
            return MESH_CLIENT_ERR_INVALID_ARGS;

        len = strlen(control_method) + 1;
        control_methods = (char *)wiced_bt_get_buffer(len + 1);
        if (control_methods == NULL)
            return MESH_CLIENT_ERR_NO_MEMORY;

        memset(control_methods, 0, len + 1);
        memcpy(control_methods, control_method, len);
    }

    if (group_name == NULL)
    {
        //the group_name is NULL, the library will register to receive messages sent to all the groups.
        groups_array = wiced_bt_mesh_db_get_all_groups(p_mesh_db, p_mesh_db->name);
    }
    else
    {
        //check if group is valid
        if (!get_group_addr(group_name))
            return MESH_CLIENT_ERR_INVALID_ARGS;

        len = strlen(group_name) + 1;
        groups_array = (char *)wiced_bt_get_buffer(len + 1);
        if (groups_array == NULL)
            return MESH_CLIENT_ERR_NO_MEMORY;
        memset(groups_array, 0, len + 1);
        memcpy(groups_array, group_name, len);
    }

    for (p_groups = groups_array; *p_groups != 0; p_groups += (strlen(p_groups) + 1))
    {
        group_addr = get_group_addr(p_groups);
        for (p_control_method = control_methods; *p_control_method != 0; p_control_method += (strlen(p_control_method) + 1))
        {
            if (!get_control_method(p_control_method, &company_id, &model_id))
                return MESH_CLIENT_ERR_INVALID_ARGS;

            //subscribe/unsubscribe local device to the group
            if ((p_op = (pending_operation_t *)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
            {
                memset(p_op, 0, sizeof(pending_operation_t));
                p_op->operation = CONFIG_OPERATION_MODEL_SUBSCRIBE;
                p_op->p_event = mesh_configure_create_event(p_mesh_db->unicast_addr, WICED_FALSE);
                p_op->uu.model_sub.operation = (uint8_t) (start_listen ? OPERATION_ADD : OPERATION_DELETE);
                p_op->uu.model_sub.element_addr = p_mesh_db->unicast_addr;
                p_op->uu.model_sub.company_id = MESH_COMPANY_ID_BT_SIG;
                p_op->uu.model_sub.model_id = model_id;
                p_op->uu.model_sub.addr[0] = (uint8_t) (group_addr & 0xff);
                p_op->uu.model_sub.addr[1] = (uint8_t) ((group_addr >> 8) & 0xff);
                configure_pending_operation_queue(p_cb, p_op);
            }
        }
    }

    //add filter for group address
    if ((p_op = (pending_operation_t*)wiced_bt_get_buffer(sizeof(pending_operation_t))) != NULL)
    {
        p_op->p_event = mesh_configure_create_event(0xFFFF, WICED_FALSE);
        p_op->uu.filter_add.addr_num = (p_mesh_db->num_groups == 0) ? 1 : p_mesh_db->num_groups;
        if (p_mesh_db->num_groups == 0)
        {
            p_op->uu.filter_add.addr_num = 1;
            p_op->uu.filter_add.addr[0] = p_cb->unicast_addr;
        }
        else
        {
            // We have a space for about 10 groups, will probably need to fix at some point
            p_op->uu.filter_add.addr_num = (uint8_t)p_mesh_db->num_groups;

            if ((p_mesh_db->num_groups - 1) * sizeof(uint16_t) + sizeof(p_op->uu.filter_add) > sizeof(p_op->uu))
                p_op->uu.filter_add.addr_num = (sizeof(p_op->uu) - sizeof(p_op->uu.filter_add)) / sizeof(uint16_t) + 1;

            for (i = 0; i < p_op->uu.filter_add.addr_num; i++)
            {
                p_op->uu.filter_add.addr[i] = p_mesh_db->group[i].addr;
            }
        }
    }
    configure_pending_operation_queue(p_cb, p_op);

    if (p_cb->p_first != NULL)
    {
        p_cb->state = PROVISION_STATE_RECONFIGURATION;
        configure_execute_pending_operation(p_cb);
    }

    wiced_bt_free_buffer(groups_array);
    wiced_bt_free_buffer(control_methods);

    return MESH_CLIENT_SUCCESS;
}

/*
 * Return a concatenated list of methods that can be used to control the device
 */
char *mesh_client_get_target_methods(const char *component_name)
{
    wiced_bt_mesh_db_node_t *p_node = wiced_bt_mesh_db_node_get_by_element_name(p_mesh_db, component_name);
    int model_idx, element_idx;
    char *control_list;
    size_t control_list_size = 1;

    if (p_node == NULL)
        return NULL;

    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_SRV))
        control_list_size += strlen(MESH_CONTROL_METHOD_ONOFF) + 1;

    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_SRV))
        control_list_size += strlen(MESH_CONTROL_METHOD_LEVEL) + 1;

    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_SRV))
        control_list_size += strlen(MESH_CONTROL_METHOD_LIGHTNESS) + 1;

    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SRV))
        control_list_size += strlen(MESH_CONTROL_METHOD_HSL) + 1;

    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_SRV))
        control_list_size += strlen(MESH_CONTROL_METHOD_CTL) + 1;

    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_XYL_SRV))
        control_list_size += strlen(MESH_CONTROL_METHOD_XYL) + 1;

    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_POWER_LEVEL_SRV))
        control_list_size += strlen(MESH_CONTROL_METHOD_POWER) + 1;

    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LOCATION_SRV))
        control_list_size += strlen(MESH_CONTROL_METHOD_LOCATION) + 1;

    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV))
        control_list_size += strlen(MESH_CONTROL_METHOD_SENSOR) + 1;

    for (element_idx = 0; element_idx < p_node->num_elements; element_idx++)
    {
        for (model_idx = 0; model_idx < p_node->element[element_idx].num_models; model_idx++)
        {
            if (p_node->element[element_idx].model[model_idx].model.company_id != MESH_COMPANY_ID_BT_SIG)
            {
                control_list_size += strlen(MESH_CONTROL_METHOD_VENDOR) + VENDOR_ID_LEN + 1;
            }
        }
    }
    control_list = (char *)wiced_bt_get_buffer(control_list_size + 1);
    if (control_list == NULL)
        return NULL;
    memset(control_list, 0, control_list_size + 1);

    control_list_size = 0;
    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_SRV))
    {
        strcpy(&control_list[control_list_size], MESH_CONTROL_METHOD_ONOFF);
        control_list_size += strlen(MESH_CONTROL_METHOD_ONOFF) + 1;
    }
    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_SRV))
    {
        strcpy(&control_list[control_list_size], MESH_CONTROL_METHOD_LEVEL);
        control_list_size += strlen(MESH_CONTROL_METHOD_LEVEL) + 1;
    }
    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_SRV))
    {
        strcpy(&control_list[control_list_size], MESH_CONTROL_METHOD_LIGHTNESS);
        control_list_size += strlen(MESH_CONTROL_METHOD_LIGHTNESS) + 1;
    }
    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SRV))
    {
        strcpy(&control_list[control_list_size], MESH_CONTROL_METHOD_HSL);
        control_list_size += strlen(MESH_CONTROL_METHOD_HSL) + 1;
    }
    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_SRV))
    {
        strcpy(&control_list[control_list_size], MESH_CONTROL_METHOD_CTL);
        control_list_size += strlen(MESH_CONTROL_METHOD_CTL) + 1;
    }
    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_XYL_SRV))
    {
        strcpy(&control_list[control_list_size], MESH_CONTROL_METHOD_XYL);
        control_list_size += strlen(MESH_CONTROL_METHOD_XYL) + 1;
    }
    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_POWER_LEVEL_SRV))
    {
        strcpy(&control_list[control_list_size], MESH_CONTROL_METHOD_POWER);
        control_list_size += strlen(MESH_CONTROL_METHOD_POWER) + 1;
    }
    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LOCATION_SRV))
    {
        strcpy(&control_list[control_list_size], MESH_CONTROL_METHOD_LOCATION);
        control_list_size += strlen(MESH_CONTROL_METHOD_LOCATION) + 1;
    }
    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV))
    {
        strcpy(&control_list[control_list_size], MESH_CONTROL_METHOD_SENSOR);
        control_list_size += strlen(MESH_CONTROL_METHOD_SENSOR) + 1;
    }
    for (element_idx = 0; element_idx < p_node->num_elements; element_idx++)
    {
        for (model_idx = 0; model_idx < p_node->element[element_idx].num_models; model_idx++)
        {
            if (p_node->element[element_idx].model[model_idx].model.company_id != MESH_COMPANY_ID_BT_SIG)
            {
                sprintf(&control_list[control_list_size], "%s%04x%04x", MESH_CONTROL_METHOD_VENDOR, p_node->element[element_idx].model[model_idx].model.company_id, p_node->element[element_idx].model[model_idx].model.id);
                control_list_size += strlen(MESH_CONTROL_METHOD_VENDOR) + VENDOR_ID_LEN + 1;
            }
        }
    }
    control_list[control_list_size++] = 0;
    return control_list;
}

/*
 * Return a model ID of the target method
 */
wiced_bool_t get_target_method(const char *method_name, uint16_t *company_id, uint16_t *model_id)
{
    int i1, i2;

    if ((method_name == NULL) || (company_id == NULL) || (model_id == NULL))
        return WICED_FALSE;

    if (strncmp(method_name, MESH_CONTROL_METHOD_VENDOR, strlen(MESH_CONTROL_METHOD_VENDOR)) == 0)
    {
        sscanf(&method_name[strlen(MESH_CONTROL_METHOD_VENDOR)], "%04x%04x", &i1, &i2);
        *company_id = (uint16_t)i1;
        *model_id = (uint16_t)i2;
    }
    else
    {
        *company_id = MESH_COMPANY_ID_BT_SIG;
        *model_id = 0;
        if (strcmp(method_name, MESH_CONTROL_METHOD_ONOFF) == 0)
            *model_id = WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_SRV;
        else if (strcmp(method_name, MESH_CONTROL_METHOD_LEVEL) == 0)
            *model_id = WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_SRV;
        else if (strcmp(method_name, MESH_CONTROL_METHOD_LIGHTNESS) == 0)
            *model_id = WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_SRV;
        else if (strcmp(method_name, MESH_CONTROL_METHOD_HSL) == 0)
            *model_id = WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_SRV;
        else if (strcmp(method_name, MESH_CONTROL_METHOD_CTL) == 0)
            *model_id = WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_SRV;
        else if (strcmp(method_name, MESH_CONTROL_METHOD_XYL) == 0)
            *model_id = WICED_BT_MESH_CORE_MODEL_ID_LIGHT_XYL_SRV;
        else if (strcmp(method_name, MESH_CONTROL_METHOD_POWER) == 0)
            *model_id = WICED_BT_MESH_CORE_MODEL_ID_GENERIC_POWER_LEVEL_SRV;
        else if (strcmp(method_name, MESH_CONTROL_METHOD_SENSOR) == 0)
            *model_id = WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV;
        else
            return WICED_FALSE;
    }
    return WICED_TRUE;
}
/*
 * Return a concatenated list of methods that can be used the device can control
 */
char *mesh_client_get_control_methods(const char *component_name)
{
    wiced_bt_mesh_db_node_t *p_node = wiced_bt_mesh_db_node_get_by_element_name(p_mesh_db, component_name);
    char *control_list;
    size_t control_list_size = 1;

    if (p_node == NULL)
        return NULL;

    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_CLNT))
        control_list_size += strlen(MESH_CONTROL_METHOD_ONOFF) + 1;

    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_CLNT))
        control_list_size += strlen(MESH_CONTROL_METHOD_LEVEL) + 1;

    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_CLNT))
        control_list_size += strlen(MESH_CONTROL_METHOD_LIGHTNESS) + 1;

    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_CLNT))
        control_list_size += strlen(MESH_CONTROL_METHOD_HSL) + 1;

    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_CLNT))
        control_list_size += strlen(MESH_CONTROL_METHOD_CTL) + 1;

    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_XYL_CLNT))
        control_list_size += strlen(MESH_CONTROL_METHOD_XYL) + 1;

    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_POWER_LEVEL_CLNT))
        control_list_size += strlen(MESH_CONTROL_METHOD_POWER) + 1;

    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LOCATION_CLNT))
        control_list_size += strlen(MESH_CONTROL_METHOD_LOCATION) + 1;

    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_CLNT))
        control_list_size += strlen(MESH_CONTROL_METHOD_SENSOR) + 1;

    control_list = (char *)wiced_bt_get_buffer(control_list_size + 1);
    if (control_list == NULL)
        return NULL;
    memset(control_list, 0, control_list_size);

    control_list_size = 0;
    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_CLNT))
    {
        strcpy(&control_list[control_list_size], MESH_CONTROL_METHOD_ONOFF);
        control_list_size += strlen(MESH_CONTROL_METHOD_ONOFF) + 1;
    }
    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_CLNT))
    {
        strcpy(&control_list[control_list_size], MESH_CONTROL_METHOD_LEVEL);
        control_list_size += strlen(MESH_CONTROL_METHOD_LEVEL) + 1;
    }
    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_CLNT))
    {
        strcpy(&control_list[control_list_size], MESH_CONTROL_METHOD_LIGHTNESS);
        control_list_size += strlen(MESH_CONTROL_METHOD_LIGHTNESS) + 1;
    }
    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_CLNT))
    {
        strcpy(&control_list[control_list_size], MESH_CONTROL_METHOD_HSL);
        control_list_size += strlen(MESH_CONTROL_METHOD_HSL) + 1;
    }
    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_CLNT))
    {
        strcpy(&control_list[control_list_size], MESH_CONTROL_METHOD_CTL);
        control_list_size += strlen(MESH_CONTROL_METHOD_CTL) + 1;
    }
    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_LIGHT_XYL_CLNT))
    {
        strcpy(&control_list[control_list_size], MESH_CONTROL_METHOD_XYL);
        control_list_size += strlen(MESH_CONTROL_METHOD_XYL) + 1;
    }
    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_POWER_LEVEL_CLNT))
    {
        strcpy(&control_list[control_list_size], MESH_CONTROL_METHOD_POWER);
        control_list_size += strlen(MESH_CONTROL_METHOD_POWER) + 1;
    }
    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LOCATION_CLNT))
    {
        strcpy(&control_list[control_list_size], MESH_CONTROL_METHOD_LOCATION);
        control_list_size += strlen(MESH_CONTROL_METHOD_LOCATION) + 1;
    }
    if (is_model_present(p_node->unicast_address, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_CLNT))
    {
        strcpy(&control_list[control_list_size], MESH_CONTROL_METHOD_SENSOR);
        control_list_size += strlen(MESH_CONTROL_METHOD_SENSOR) + 1;
    }
    control_list[control_list_size++] = 0;
    return control_list;
}

/*
 * Return a model ID of the control method
 */
wiced_bool_t get_control_method(const char *method_name, uint16_t *company_id, uint16_t *model_id)
{
    int i1, i2;

    if ((method_name == NULL) || (company_id == NULL) || (model_id == NULL))
        return WICED_FALSE;

    if (strncmp(method_name, MESH_CONTROL_METHOD_VENDOR, strlen(MESH_CONTROL_METHOD_VENDOR)) == 0)
    {
        sscanf(&method_name[strlen(MESH_CONTROL_METHOD_VENDOR)], "%04x%04x", &i1, &i2);
        *company_id = (uint16_t)i1;
        *model_id = (uint16_t)i2;
    }
    else
    {
        *company_id = MESH_COMPANY_ID_BT_SIG;
        *model_id = 0;
        if (strcmp(method_name, MESH_CONTROL_METHOD_ONOFF) == 0)
            *model_id = WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_CLNT;
        else if (strcmp(method_name, MESH_CONTROL_METHOD_LEVEL) == 0)
            *model_id = WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_CLNT;
        else if (strcmp(method_name, MESH_CONTROL_METHOD_LIGHTNESS) == 0)
            *model_id = WICED_BT_MESH_CORE_MODEL_ID_LIGHT_LIGHTNESS_CLNT;
        else if (strcmp(method_name, MESH_CONTROL_METHOD_HSL) == 0)
            *model_id = WICED_BT_MESH_CORE_MODEL_ID_LIGHT_HSL_CLNT;
        else if (strcmp(method_name, MESH_CONTROL_METHOD_CTL) == 0)
            *model_id = WICED_BT_MESH_CORE_MODEL_ID_LIGHT_CTL_CLNT;
        else if (strcmp(method_name, MESH_CONTROL_METHOD_XYL) == 0)
            *model_id = WICED_BT_MESH_CORE_MODEL_ID_LIGHT_XYL_CLNT;
        else if (strcmp(method_name, MESH_CONTROL_METHOD_POWER) == 0)
            *model_id = WICED_BT_MESH_CORE_MODEL_ID_GENERIC_POWER_LEVEL_CLNT;
        else if (strcmp(method_name, MESH_CONTROL_METHOD_SENSOR) == 0)
            *model_id = WICED_BT_MESH_CORE_MODEL_ID_SENSOR_CLNT;
        else
            return WICED_FALSE;
    }
    return WICED_TRUE;
}

uint32_t wiced_hal_rand_gen_num(void)
{
    uint32_t r;
    int i;
    uint8_t *p = (uint8_t *)&r;
    for (i = 0; i < 4; i++)
        *p++ = rand() & 0xff;
    return r;
}

void wiced_hal_rand_gen_num_array(uint32_t* randNumberArrayPtr, uint32_t length)
{
    uint32_t i;
    uint8_t *p = (uint8_t *)randNumberArrayPtr;
    for (i = 0; i < length * 4; i++)
        *p++ = rand() & 0xff;
}

uint16_t mesh_client_fix_lightness(uint16_t dst, uint16_t lightness)
{
    wiced_bt_mesh_db_node_t *p_node = wiced_bt_mesh_db_node_get_by_element_addr(p_mesh_db, dst);
    if (p_node->cid == MESH_COMPANY_ID_CYPRESS && p_node->vid == 1)
    {
    }
    return lightness;
}

uint16_t mesh_client_get_unicast_addr()
{
    return provision_cb.unicast_addr;
}

uint16_t mesh_client_get_proxy_addr()
{
    return provision_cb.proxy_addr;
}
