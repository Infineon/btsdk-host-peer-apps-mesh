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
* Mesh Main implementation.
*/

#define _CRT_RAND_S

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include "wiced_bt_mesh_provision.h"
#include "wiced_bt_mesh_cfg.h"
#include "wiced_bt_mesh_models.h"
#include "wiced_bt_mesh_core.h"
#include "wiced_bt_trace.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_gatt.h"
#ifdef MESH_DFU_ENABLED
#include "wiced_bt_mesh_dfu.h"
#endif
#ifdef DIRECTED_FORWARDING_SERVER_SUPPORTED
#include "wiced_bt_mesh_mdf.h"
#endif
#ifdef PRIVATE_PROXY_SUPPORTED
#include "wiced_bt_mesh_private_proxy.h"
#endif
#include "wiced_bt_cfg.h"
wiced_bt_cfg_settings_t wiced_bt_cfg_settings;

#define MESH_VENDOR_COMPANY_ID          0x131   // Cypress Company ID
#define MESH_VENDOR_MODEL_ID            1       // ToDo.  This need to be modified

// This sample shows simple use of vendor specific messages.  Vendor model
// can define any opcodes it wants.
#define MESH_VENDOR_OPCODE1             1       // Vendor message opcode 1
#define MESH_VENDOR_OPCODE2             2       // Vendor message with opcode 2

#ifdef MESH_DFU_ENABLED
const mesh_dfu_opcodes_t dfu_opcodes = {
    .blob_transfer_get = WICED_BT_MESH_OPCODE_BLOB_TRANSFER_GET,
    .blob_transfer_start = WICED_BT_MESH_OPCODE_BLOB_TRANSFER_START,
    .blob_transfer_cancel = WICED_BT_MESH_OPCODE_BLOB_TRANSFER_CANCEL,
    .blob_transfer_status = WICED_BT_MESH_OPCODE_BLOB_TRANSFER_STATUS,
    .blob_block_get = WICED_BT_MESH_OPCODE_BLOB_BLOCK_GET,
    .blob_block_start = WICED_BT_MESH_OPCODE_BLOB_BLOCK_START,
    .blob_partial_block_report = WICED_BT_MESH_OPCODE_BLOB_PARTIAL_BLOCK_REPORT,
    .blob_block_status = WICED_BT_MESH_OPCODE_BLOB_BLOCK_STATUS,
    .blob_chunk_transfer = WICED_BT_MESH_OPCODE_BLOB_CHUNK_TRANSFER,
    .blob_info_get = WICED_BT_MESH_OPCODE_BLOB_INFO_GET,
    .blob_info_status = WICED_BT_MESH_OPCODE_BLOB_INFO_STATUS,

    .fw_update_info_get = WICED_BT_MESH_OPCODE_FW_UPDATE_INFO_GET,
    .fw_update_info_status = WICED_BT_MESH_OPCODE_FW_UPDATE_INFO_STATUS,
    .fw_update_metadata_check = WICED_BT_MESH_OPCODE_FW_UPDATE_FW_METADATA_CHECK,
    .fw_update_metadata_status = WICED_BT_MESH_OPCODE_FW_UPDATE_FW_METADATA_STATUS,
    .fw_update_get = WICED_BT_MESH_OPCODE_FW_UPDATE_GET,
    .fw_update_start = WICED_BT_MESH_OPCODE_FW_UPDATE_START,
    .fw_update_cancel = WICED_BT_MESH_OPCODE_FW_UPDATE_CANCEL,
    .fw_update_apply = WICED_BT_MESH_OPCODE_FW_UPDATE_APPLY,
    .fw_update_status = WICED_BT_MESH_OPCODE_FW_UPDATE_STATUS,

    .fw_distr_get = WICED_BT_MESH_OPCODE_FW_DISTR_GET,
    .fw_distr_start = WICED_BT_MESH_OPCODE_FW_DISTR_START,
    .fw_distr_cancel = WICED_BT_MESH_OPCODE_FW_DISTR_CANCEL,
    .fw_distr_apply = WICED_BT_MESH_OPCODE_FW_DISTR_APPLY,
    .fw_distr_status = WICED_BT_MESH_OPCODE_FW_DISTR_STATUS,
    .fw_distr_nodes_get = WICED_BT_MESH_OPCODE_FW_DISTR_NODES_GET,
    .fw_distr_nodes_list = WICED_BT_MESH_OPCODE_FW_DISTR_NODES_LIST,
    .fw_distr_nodes_add = WICED_BT_MESH_OPCODE_FW_DISTR_NODES_ADD,
    .fw_distr_nodes_delete_all = WICED_BT_MESH_OPCODE_FW_DISTR_NODES_DELETE_ALL,
    .fw_distr_nodes_status = WICED_BT_MESH_OPCODE_FW_DISTR_NODES_STATUS,
    .fw_distr_capabilities_get = WICED_BT_MESH_OPCODE_FW_DISTR_CAPABILITIES_GET,
    .fw_distr_capabilities_status = WICED_BT_MESH_OPCODE_FW_DISTR_CAPABILITIES_STATUS,
    .fw_distr_upload_get = WICED_BT_MESH_OPCODE_FW_DISTR_UPLOAD_GET,
    .fw_distr_upload_start = WICED_BT_MESH_OPCODE_FW_DISTR_UPLOAD_START,
    .fw_distr_upload_oob_start = WICED_BT_MESH_OPCODE_FW_DISTR_UPLOAD_OOB_START,
    .fw_distr_upload_cancel = WICED_BT_MESH_OPCODE_FW_DISTR_UPLOAD_CANCEL,
    .fw_distr_upload_status = WICED_BT_MESH_OPCODE_FW_DISTR_UPLOAD_STATUS,
    .fw_distr_fw_get = WICED_BT_MESH_OPCODE_FW_DISTR_FW_GET,
    .fw_distr_fw_status = WICED_BT_MESH_OPCODE_FW_DISTR_FW_STATUS,
    .fw_distr_fw_get_by_index = WICED_BT_MESH_OPCODE_FW_DISTR_FW_GET_BY_INDEX,
    .fw_distr_fw_delete = WICED_BT_MESH_OPCODE_FW_DISTR_FW_DELETE,
    .fw_distr_fw_delete_all = WICED_BT_MESH_OPCODE_FW_DISTR_FW_DELETE_ALL,
};

mesh_dfu_blob_client_config_t blob_client_cfg = {
    .max_block_size_log                 = 12,                       // maximum block size 4096 bytes
    .chunk_size_unicast                 = 256,                      // chunk size when sending with unicast
    .chunk_size_multicast               = 32,                       // chunk size when sending with multicast
    .max_chunks_number                  = 512,                      // maximum number of chunks in a block
    .max_mtu_size                       = 1024,                     // MTU
};

mesh_dfu_distributor_config_t dfu_distributor_cfg = {
    .max_buffer_size                    = 1024,                     // largest buffer size defined in wiced_bt_cfg.c
    .max_fw_storage_space               = 262144,                   // maximum firmware storage space is 256KB
    .oob_supported                      = 0,                        // out-of-band upload not supported
};

mesh_dfu_config_t dfu_cfg = {
    .p_blob_client_cfg                  = &blob_client_cfg,         // BLOB transfer client configuration
    .p_blob_server_cfg                  = NULL,
    .p_distributor_cfg                  = &dfu_distributor_cfg,     // DFU distributor configuration
    .p_update_server_cfg                = NULL
};
#endif

void wiced_hal_rand_gen_num_array(uint32_t* randNumberArrayPtr, uint32_t length);
uint32_t wiced_hal_get_pseudo_rand_number(void);
uint32_t wiced_hal_rand_gen_num(void);
void wiced_hal_wdog_reset_system(void);
void wiced_hal_delete_nvram(uint16_t vs_id, wiced_result_t* p_status);
uint16_t wiced_hal_write_nvram(uint16_t vs_id, uint16_t data_length, uint8_t* p_data, wiced_result_t* p_status);
uint16_t wiced_hal_read_nvram(uint16_t vs_id, uint16_t data_length, uint8_t* p_data, wiced_result_t* p_status);

wiced_bt_mesh_core_hal_api_t mesh_app_hal_api =
{
    .rand_gen_num_array = wiced_hal_rand_gen_num_array,
    .get_pseudo_rand_number = wiced_hal_get_pseudo_rand_number,
    .rand_gen_num = wiced_hal_rand_gen_num,
    .wdog_reset_system = wiced_hal_wdog_reset_system,
    .delete_nvram = wiced_hal_delete_nvram,
    .write_nvram = wiced_hal_write_nvram,
    .read_nvram = wiced_hal_read_nvram
};

static void mesh_app_init(wiced_bool_t is_provisioned);
static void mesh_provision_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_config_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_control_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_sensor_client_callback(uint16_t event, wiced_bt_mesh_event_t* p_event, void* p_data);

static void mesh_core_state_changed(wiced_bt_mesh_core_state_type_t type, wiced_bt_mesh_core_state_t *p_state);
extern void mesh_provision_process_event(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
extern void mesh_vendor_specific_data(uint16_t src, uint16_t company_id, uint16_t model_id, uint8_t opcode, uint8_t ttl, void *p_data, uint16_t data_len);

const char* szRegKey = "Software\\Cypress\\Mesh\\MeshClient";
static UINT32 read_reg(const char* name, UINT8* value, UINT16 len, DWORD *p_res)
{
    HKEY    hKey;
    DWORD   dwLen = len;
    *p_res = RegCreateKeyExA(HKEY_CURRENT_USER, szRegKey, 0, NULL, 0, KEY_QUERY_VALUE, NULL, &hKey, NULL);
    if (*p_res == ERROR_SUCCESS)
    {
        *p_res = RegQueryValueExA(hKey, name, 0, NULL, value, &dwLen);
        RegCloseKey(hKey);
    }
    if (*p_res != ERROR_SUCCESS)
        dwLen = 0;
    return dwLen;
}

static UINT32 write_reg(const char* name, UINT8* value, UINT16 len, DWORD *p_res)
{
    HKEY    hKey;
    *p_res = RegCreateKeyExA(HKEY_CURRENT_USER, szRegKey, 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL);
    if (*p_res == ERROR_SUCCESS)
    {
        *p_res = RegSetValueExA(hKey, name, 0, REG_BINARY, value, len);
        RegCloseKey(hKey);
    }
    if (*p_res != ERROR_SUCCESS)
        len = 0;
    return len;
}

/*
* Application provided function to read/write information from/into NVRAM
*/
static uint32_t mesh_nvram_access(wiced_bool_t write, int inx, uint8_t* value, uint16_t len, wiced_result_t *p_result)
{
    *p_result = ERROR_SUCCESS;
    if (write)
        return len;
    else
        return 0;
}

typedef wiced_bool_t(*wiced_model_message_handler_t)(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint16_t data_len);

static wiced_bt_mesh_core_received_msg_handler_t get_msg_handler_callback(uint16_t company_id, uint16_t opcode, uint16_t *p_model_id, uint8_t* p_rpl_delay);
static void                     mesh_start_stop_scan_callback(wiced_bool_t start, wiced_bool_t is_active);
void                            proxy_gatt_send_cb(uint32_t conn_id, uint32_t ref_data, const uint8_t *packet, uint32_t packet_len);
static uint32_t                 mesh_nvram_access(wiced_bool_t write, int inx, uint8_t* node_info, uint16_t len, wiced_result_t *p_result);
static wiced_bool_t             vendor_data_handler(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint16_t data_len);

#define MESH_PID                0x3006
#define MESH_VID                0x0002

wiced_bt_mesh_core_config_model_t   mesh_element1_models[] =
{
    WICED_BT_MESH_DEVICE,
    WICED_BT_MESH_MODEL_CONFIG_CLIENT,
    WICED_BT_MESH_MODEL_HEALTH_CLIENT,
    WICED_BT_MESH_MODEL_PROPERTY_CLIENT,
    WICED_BT_MESH_MODEL_REMOTE_PROVISION_SERVER,
    WICED_BT_MESH_MODEL_REMOTE_PROVISION_CLIENT,
#ifdef PRIVATE_PROXY_SUPPORTED
    WICED_BT_MESH_MODEL_PRIVATE_PROXY_CLIENT,
#endif
    WICED_BT_MESH_MODEL_DEFAULT_TRANSITION_TIME_CLIENT,
    WICED_BT_MESH_MODEL_ONOFF_CLIENT,
    WICED_BT_MESH_MODEL_LEVEL_CLIENT,
    WICED_BT_MESH_MODEL_POWER_ONOFF_CLIENT,
    WICED_BT_MESH_MODEL_LIGHT_LIGHTNESS_CLIENT,
    WICED_BT_MESH_MODEL_LIGHT_CTL_CLIENT,
    WICED_BT_MESH_MODEL_LIGHT_HSL_CLIENT,
    WICED_BT_MESH_MODEL_SENSOR_CLIENT,
    WICED_BT_MESH_MODEL_LIGHT_LC_CLIENT,
    WICED_BT_MESH_MODEL_SCENE_CLIENT,
#ifdef DIRECTED_FORWARDING_SERVER_SUPPORTED
    WICED_BT_MESH_DIRECTED_FORWARDING_CLIENT,
#endif
#ifdef MESH_DFU_ENABLED
    WICED_BT_MESH_MODEL_FW_DISTRIBUTION_CLIENT,
    WICED_BT_MESH_MODEL_FW_DISTRIBUTOR,
#endif
#ifdef MESH_VENDOR_MODEL_ID
    { MESH_VENDOR_COMPANY_ID, MESH_VENDOR_MODEL_ID, vendor_data_handler, NULL, NULL },
#endif
};
#define MESH_APP_NUM_MODELS  (sizeof(mesh_element1_models) / sizeof(wiced_bt_mesh_core_config_model_t))

#define MESH_PROVISIONER_CLIENT_ELEMENT_INDEX   0

wiced_bt_mesh_core_config_element_t mesh_elements[] =
{
    {
        .location = MESH_ELEM_LOC_MAIN,                                 // location description as defined in the GATT Bluetooth Namespace Descriptors section of the Bluetooth SIG Assigned Numbers
        .default_transition_time = MESH_DEFAULT_TRANSITION_TIME_IN_MS,  // Default transition time for models of the element in milliseconds
        .onpowerup_state = WICED_BT_MESH_ON_POWER_UP_STATE_RESTORE,     // Default element behavior on power up
        .default_level = 0,                                             // Default value of the variable controlled on this element (for example power, lightness, temperature, hue...)
        .range_min = 1,                                                 // Minimum value of the variable controlled on this element (for example power, lightness, temperature, hue...)
        .range_max = 0xffff,                                            // Maximum value of the variable controlled on this element (for example power, lightness, temperature, hue...)
        .move_rollover = 0,                                             // If true when level gets to range_max during move operation, it switches to min, otherwise move stops.
        .properties_num = 0,                                            // Number of properties in the array models
        .properties = NULL,                                             // Array of properties in the element.
        .sensors_num = 0,                                               // Number of sensors in the sensor array
        .sensors = NULL,                                                // Array of sensors of that element
        .models_num = MESH_APP_NUM_MODELS,                              // Number of models in the array models
        .models = mesh_element1_models,                                 // Array of models located in that element. Model data is defined by structure wiced_bt_mesh_core_config_model_t
    },
};

wiced_bt_mesh_core_config_t  mesh_config =
{
    .company_id         = MESH_COMPANY_ID_CYPRESS,                  // Company identifier assigned by the Bluetooth SIG
    .product_id         = MESH_PID,                                 // Vendor-assigned product identifier
    .vendor_id          = MESH_VID,                                 // Vendor-assigned product version identifier
    .features           = WICED_BT_MESH_CORE_FEATURE_BIT_NO_ADV_BEARER,     // GATT client mode: advert scanning but no advert sending and receiving
    .friend_cfg         =                                           // Empty Configuration of the Friend Feature
    {
        .receive_window        = 0,                                 // Receive Window value in milliseconds supported by the Friend node.
        .cache_buf_len  = 0,                                        // Length of the buffer for the cache
        .max_lpn_num    = 0                                         // Max number of Low Power Nodes with established friendship. Must be > 0 if Friend feature is supported.
    },
    .low_power          =                                           // Configuration of the Low Power Feature
    {
        .rssi_factor           = 0,                                 // contribution of the RSSI measured by the Friend node used in Friend Offer Delay calculations.
        .receive_window_factor = 0,                                 // contribution of the supported Receive Window used in Friend Offer Delay calculations.
        .min_cache_size_log    = 0,                                 // minimum number of messages that the Friend node can store in its Friend Cache.
        .receive_delay         = 0,                                 // Receive delay in 1 ms units to be requested by the Low Power node.
        .poll_timeout          = 0                                  // Poll timeout in 100ms units to be requested by the Low Power node.
    },
    .gatt_client_only          = WICED_TRUE,                        // Can connect to mesh over GATT or ADV
    .elements_num  = (uint8_t)(sizeof(mesh_elements) / sizeof(mesh_elements[0])),   // number of elements on this device
    .elements      = mesh_elements                                  // Array of elements for this device
};


void mesh_application_init(void)
{
    static int core_initialized = 0;

    wiced_bt_mesh_core_set_hal_api(&mesh_app_hal_api);

    // Set Debug trace level for mesh_models_lib and mesh_provisioner_lib
    wiced_bt_mesh_models_set_trace_level(WICED_BT_MESH_CORE_TRACE_INFO);
    // Set Debug trace level for all modules but Info level for CORE_AES_CCM module
    wiced_bt_mesh_core_set_trace_level(WICED_BT_MESH_CORE_TRACE_FID_ALL, WICED_BT_MESH_CORE_TRACE_DEBUG);
    wiced_bt_mesh_core_set_trace_level(WICED_BT_MESH_CORE_TRACE_FID_CORE_AES_CCM, WICED_BT_MESH_CORE_TRACE_INFO);

    if (!core_initialized)
    {
        // activate IV Update test mode to remove the 96-hour limit
        //wiced_bt_core_iv_update_test_mode = WICED_TRUE;

        core_initialized = 1;
        wiced_bt_mesh_core_init_t   init = { 0 };
#ifdef MESH_SUPPORT_PB_GATT
        mesh_config.features |= WICED_BT_MESH_CORE_FEATURE_BIT_PB_GATT;
#endif
        wiced_bt_mesh_core_net_key_max_num = 4;
        wiced_bt_mesh_core_app_key_max_num = 8;

        init.p_config_data = &mesh_config;
        init.callback = get_msg_handler_callback;
        init.scan_callback = mesh_start_stop_scan_callback;
        init.proxy_send_callback = proxy_gatt_send_cb;
        init.nvram_access_callback = mesh_nvram_access;
        init.state_changed_cb = mesh_core_state_changed;
        wiced_bt_mesh_core_init(&init);
        wiced_bt_mesh_remote_provisioning_server_init();
        wiced_bt_mesh_core_start();
    }
    mesh_app_init(WICED_TRUE);
}

void mesh_application_deinit(void)
{
    mesh_app_init(WICED_FALSE);
}

/*
* Application implements that function to handle received messages. Call each library that this device needs to support.
*/
wiced_bt_mesh_core_received_msg_handler_t get_msg_handler_callback(uint16_t company_id, uint16_t opcode, uint16_t *p_model_id, uint8_t* p_rpl_delay)
{
    wiced_bt_mesh_core_received_msg_handler_t p_message_handler = NULL;
    uint8_t                                   idx_elem, idx_model;
    wiced_bt_mesh_event_t                     temp_event;
    uint16_t                                  model_id;

    WICED_BT_TRACE("company_id:%x opcode:%x\n", company_id, opcode);
    if (company_id == MESH_COMPANY_ID_UNUSED)
    {
        p_message_handler = wiced_bt_mesh_proxy_client_message_handler;
    }
    else
    {
        temp_event.company_id = company_id;
        temp_event.opcode = opcode;
        temp_event.model_id = 0xffff;   //it is sign of special mode for model to just return model_id without message handling
        temp_event.status.rpl_delay = 0; // model can change that to indicate different rule

        for (idx_elem = 0; idx_elem < mesh_config.elements_num; idx_elem++)
        {
            for (idx_model = 0; idx_model < mesh_config.elements[idx_elem].models_num; idx_model++)
            {
                if (company_id != mesh_config.elements[idx_elem].models[idx_model].company_id)
                    continue;
                p_message_handler = (wiced_bt_mesh_core_received_msg_handler_t)mesh_config.elements[idx_elem].models[idx_model].p_message_handler;
                if (p_message_handler == NULL)
                    continue;
                if (!p_message_handler(&temp_event, NULL, 0))
                    continue;
                model_id = mesh_config.elements[idx_elem].models[idx_model].model_id;
                if (p_model_id)
                    *p_model_id = model_id;
                if (p_rpl_delay)
                    *p_rpl_delay = temp_event.status.rpl_delay;
                break;
            }
            if (idx_model < mesh_config.elements[idx_elem].models_num)
                break;
        }
        if (idx_elem >= mesh_config.elements_num)
            p_message_handler = NULL;
    }
    if (!p_message_handler)
        WICED_BT_TRACE("ignored\n");
    return p_message_handler;
}

/*
 * Application implements that function to start/stop scanning as requested by the core
 */
void mesh_start_stop_scan_callback(wiced_bool_t start, wiced_bool_t is_scan_active)
{
    extern void wiced_bt_ble_set_scan_mode(uint8_t is_active);

    static wiced_bool_t started = WICED_FALSE;
    static wiced_bool_t active = WICED_FALSE;
    uint16_t local_addr = wiced_bt_mesh_core_get_local_addr();
    WICED_BT_TRACE("scan callback: start:%d active:%d\n", start, is_scan_active);

    if ((started && start && (active == is_scan_active)) ||
        (!started && !start))
        return;

    // check if the request is to stop the scan, or scan type is different
    if ((started && !start) || (start && (active != is_scan_active)))
    {
        WICED_BT_TRACE("scan callback stop active:%d\n", active);
        wiced_bt_ble_observe(0, 0, NULL);

        started = WICED_FALSE;
        if (!start)
            return;
    }

    started = WICED_TRUE;
    active = is_scan_active;
    WICED_BT_TRACE("scan callback start active:%d\n", active);
    wiced_bt_ble_set_scan_mode(is_scan_active);
    wiced_bt_ble_observe(start, 0, NULL);
}

void mesh_app_init(wiced_bool_t is_provisioned)
{
    // Set Debug trace level for mesh_models_lib and mesh_provisioner_lib
    wiced_bt_mesh_models_set_trace_level(WICED_BT_MESH_CORE_TRACE_INFO);

    wiced_bt_mesh_provision_client_init(mesh_provision_message_handler, is_provisioned);
    wiced_bt_mesh_client_init(mesh_provision_message_handler, is_provisioned);

    wiced_bt_mesh_config_client_init(mesh_config_message_handler, is_provisioned);
    wiced_bt_mesh_health_client_init(mesh_config_message_handler, is_provisioned);
    wiced_bt_mesh_proxy_client_init(mesh_config_message_handler, is_provisioned);
#ifdef MESH_DFU_ENABLED
    wiced_bt_mesh_model_fw_update_set_opcodes(&dfu_opcodes);
    wiced_bt_mesh_model_fw_update_init(&dfu_cfg, NULL);
    wiced_bt_mesh_model_fw_provider_init();
    wiced_bt_mesh_model_fw_distribution_server_init();
#endif

    wiced_bt_mesh_model_property_client_init(0, mesh_control_message_handler, is_provisioned);
    wiced_bt_mesh_model_onoff_client_init(0, mesh_control_message_handler, is_provisioned);
    wiced_bt_mesh_model_power_onoff_client_init(0, mesh_control_message_handler, is_provisioned);
    wiced_bt_mesh_model_level_client_init(0, mesh_control_message_handler, is_provisioned);
    wiced_bt_mesh_model_power_onoff_client_init(0, mesh_control_message_handler, is_provisioned);
    wiced_bt_mesh_model_light_lightness_client_init(0, mesh_control_message_handler, is_provisioned);
    wiced_bt_mesh_model_light_ctl_client_init(0, mesh_control_message_handler, is_provisioned);
    wiced_bt_mesh_model_light_hsl_client_init(0, mesh_control_message_handler, is_provisioned);
    wiced_bt_mesh_model_default_transition_time_client_init(0, mesh_control_message_handler, is_provisioned);
    wiced_bt_mesh_model_sensor_client_init(0, mesh_sensor_client_callback, is_provisioned);
    wiced_bt_mesh_model_light_lc_client_init(0, mesh_control_message_handler, is_provisioned);
    wiced_bt_mesh_model_scene_client_init(0, mesh_control_message_handler, is_provisioned);
}

void mesh_provision_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    WICED_BT_TRACE("provision message:%d\n", event);
    mesh_provision_process_event(event, p_event, p_data);
}

void mesh_config_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    WICED_BT_TRACE("config message:%d\n", event);
    mesh_provision_process_event(event, p_event, p_data);
}

void mesh_control_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    WICED_BT_TRACE("control message:%d\n", event);
    mesh_provision_process_event(event, p_event, p_data);
}

static void mesh_sensor_client_callback(uint16_t event, wiced_bt_mesh_event_t* p_event, void* p_data)
{
    WICED_BT_TRACE("sensor message:%d\n", event);
    mesh_provision_process_event(event, p_event, p_data);
}


void mesh_core_state_changed(wiced_bt_mesh_core_state_type_t type, wiced_bt_mesh_core_state_t *p_state)
{
    if (type == WICED_BT_MESH_CORE_STATE_TYPE_SEQ)
        mesh_provision_process_event(WICED_BT_MESH_SEQ_CHANGED, NULL, &p_state->seq);
    else if(type == WICED_BT_MESH_CORE_STATE_IV)
        mesh_provision_process_event(WICED_BT_MESH_IV_CHANGED, NULL, &p_state->iv);
}


wiced_bool_t vendor_data_handler(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint16_t data_len)
{
    char buf[100] = { 0 };

    // 0xffff model_id means request to check if that opcode belongs to that model
    if (p_event->model_id == 0xffff)
    {
        return (p_event->company_id == MESH_VENDOR_COMPANY_ID);
    }

    WICED_BT_TRACE("Vendor Data Opcode:%d\n", p_event->opcode);

    for (int i = 0; i < data_len && i < 33; i++)
        sprintf(&buf[strlen(buf)], "%02x", p_data[i]);

    WICED_BT_TRACE("%s\n", buf);
    mesh_vendor_specific_data(p_event->src, p_event->company_id, p_event->model_id, (uint8_t)p_event->opcode, (uint8_t)p_event->ttl, p_data, data_len);
    wiced_bt_mesh_release_event(p_event);

    return WICED_TRUE;
}

void wiced_bt_mesh_add_vendor_model(wiced_bt_mesh_add_vendor_model_data_t* p_data)
{

}

void wiced_bt_mesh_adv_tx_power_set(uint8_t adv_tx_power)
{

}
