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
 * This file shows how to create a device which implements mesh provisioner client.
 * The main purpose of the app is to process messages coming from the MCU and call Mesh Core
 * Library to perform functionality.
 */


#include "wiced.h"
#include "mesh_main.h"
#include <wiced_bt_mesh_cfg.h>
#include <wiced_bt_mesh_models.h>
#include <wiced_bt_mesh_provision.h>
#include <hci_control_api.h>
#include "trace.h"
#include <wiced_bt_ble.h>
#include "wiced_bt_mesh_app.h"

#include <malloc.h>
#include <wiced_bt_mesh_core.h>

#ifdef MESH_DFU_ENABLED
#include "wiced_bt_mesh_dfu.h"
#include "wiced_mesh_client_dfu.h"
#endif

static void mesh_app_init(wiced_bool_t is_provisioned);
static void mesh_provision_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_config_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_control_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static void mesh_sensor_message_handler(uint8_t element_idx, uint16_t addr, uint16_t event, void *p_data);
static void mesh_core_state_changed(wiced_bt_mesh_core_state_type_t type, wiced_bt_mesh_core_state_t *p_state);
extern void mesh_provision_process_event(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
extern void mesh_sensor_process_event(uint16_t addr, uint16_t event, void *p_data);

extern void proxy_gatt_send_cb(uint32_t conn_id, uint32_t ref_data, const uint8_t *packet, uint32_t packet_len);
static uint32_t mesh_nvram_access(wiced_bool_t write, int inx, uint8_t* value, uint16_t len, wiced_result_t *p_result);
typedef wiced_bool_t (*wiced_model_message_handler_t)(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint16_t data_len);
static wiced_bt_mesh_core_received_msg_handler_t get_msg_handler_callback(uint16_t company_id, uint16_t opcode, uint16_t *p_model_id, wiced_bool_t *p_dont_save_rpl);
static void mesh_start_stop_scan_callback(wiced_bool_t start, wiced_bool_t is_active);
wiced_bool_t vendor_data_handler(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint16_t data_len);
extern void meshClientVendorSpecificDataStatus(const char *device_name, uint16_t company_id, uint16_t model_id, uint8_t opcode,uint8_t ttl,uint8_t *p_data, uint16_t data_len);
extern void mesh_native_lib_read_dfu_meta_data(uint8_t *p_fw_id, uint32_t *p_fw_id_len, uint8_t *p_validation_data, uint32_t *p_validation_data_len);


/******************************************************
 *          Constants
 ******************************************************/
// defines Friendship Mode
#define MESH_FRIENDSHIP_NONE  0
#define MESH_FRIENDSHIP_FRND  1
#define MESH_FRIENDSHIP_LPN   2

#define MESH_PID                0x3001          //change this from 0x3016 to 0x 3001
#define MESH_VID                0x0002
#define MESH_CACHE_REPLAY_SIZE  200
#define APPEARANCE_GENERIC_TAG  512
#define MESH_VENDOR_COMPANY_ID          0x131   // Cypress Company ID
#define MESH_VENDOR_MODEL_ID            1       // ToDo.  This need to be modified

#define SENSOR_PROPERTY_ID_SIZE                     (2)
#define PROPERTY_ID_AMBIENT_LUX_LEVEL_ON            (0x2B)
#define PROPERTY_ID_MOTION_SENSED                   (0x42)
#define PROPERTY_ID_TOTAL_LIGHT_EXPOSURE_TIME       (0x6F)
#define PROPERTY_ID_PRESENCE_DETECTED               (0x4D)
#define PROPERTY_ID_PRESENT_AMBIENT_LIGHT_LEVEL     (0x4E)
#define PROPERTY_ID_PRESENT_AMBIENT_TEMPERATURE     (0x4F)
#define SETTING_PROPERTY_ID                         (0x2001)
// This sample shows simple use of vendor get/set/status messages.  Vendor model
// can define any opcodes it wants.
#define MESH_VENDOR_OPCODE1          1       // Command to Get data
#define MESH_VENDOR_OPCODE2          2       // Command to Set data ack is required


wiced_result_t mesh_transport_send_data( uint16_t opcode, uint8_t* p_data, uint16_t length );
void mesh_provisioner_client_message_handler(uint16_t event, void *p_data);

wiced_result_t provision_gatt_send(uint16_t conn_id, const uint8_t *packet, uint32_t packet_len);

wiced_bt_mesh_core_config_model_t   mesh_element1_models[] =
{
    WICED_BT_MESH_DEVICE,
    WICED_BT_MESH_MODEL_CONFIG_CLIENT,
    WICED_BT_MESH_MODEL_HEALTH_CLIENT,
    WICED_BT_MESH_MODEL_PROPERTY_CLIENT,
    WICED_BT_MESH_MODEL_REMOTE_PROVISION_SERVER,
    WICED_BT_MESH_MODEL_REMOTE_PROVISION_CLIENT,
    WICED_BT_MESH_MODEL_DEFAULT_TRANSITION_TIME_CLIENT,
    WICED_BT_MESH_MODEL_ONOFF_CLIENT,
    WICED_BT_MESH_MODEL_LEVEL_CLIENT,
    WICED_BT_MESH_MODEL_POWER_ONOFF_CLIENT,
    WICED_BT_MESH_MODEL_LIGHT_LIGHTNESS_CLIENT,
    WICED_BT_MESH_MODEL_LIGHT_CTL_CLIENT,
    WICED_BT_MESH_MODEL_LIGHT_HSL_CLIENT,
    WICED_BT_MESH_MODEL_SENSOR_CLIENT,
    WICED_BT_MESH_MODEL_LIGHT_LC_CLIENT,
#ifdef MESH_DFU_ENABLED
    WICED_BT_MESH_MODEL_FW_DISTRIBUTION_CLIENT,
    WICED_BT_MESH_MODEL_BLOB_TRANSFER_CLIENT,
#endif
#ifdef MESH_VENDOR_MODEL_ID
    { MESH_VENDOR_COMPANY_ID, MESH_VENDOR_MODEL_ID, vendor_data_handler, NULL, NULL },
#endif
};
#define MESH_APP_NUM_MODELS  (sizeof(mesh_element1_models) / sizeof(wiced_bt_mesh_core_config_model_t))

#define MESH_PROVISIONER_CLIENT_ELEMENT_INDEX   0

wiced_bt_mesh_core_config_property_t mesh_element1_properties[] =
{
    { PROPERTY_ID_AMBIENT_LUX_LEVEL_ON,            0, 0, 3, NULL},
    { PROPERTY_ID_MOTION_SENSED,                   0, 0, 1, NULL},
    { PROPERTY_ID_PRESENCE_DETECTED,               0, 0, 1, NULL},
    { PROPERTY_ID_PRESENT_AMBIENT_LIGHT_LEVEL,     0, 0, 3, NULL},
    { WICED_BT_MESH_PROPERTY_TOTAL_DEVICE_RUNTIME, 0, 0, 3, NULL},
    { SETTING_PROPERTY_ID,                         0, 0, 2, NULL},
    { WICED_BT_MESH_PROPERTY_PRESENT_AMBIENT_TEMPERATURE,  0, 0, 1, NULL},
};

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
        .properties_num = 7,                                            // Number of properties in the array models
        .properties = mesh_element1_properties,                         // Array of properties in the element.
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
    .replay_cache_size  = MESH_CACHE_REPLAY_SIZE,                   // Number of replay protection entries, i.e. maximum number of mesh devices that can send application messages to this device.
    .features                  = 0,                                 //
    .friend_cfg         =                                           // Empty Configuration of the Friend Feature
    {
        .receive_window        = 0,                                 // Receive Window value in milliseconds supported by the Friend node.
        .cache_buf_len         = 0                                  // Length of the buffer for the cache
    },
    .low_power          =                                           // Configuration of the Low Power Feature
    {
        .rssi_factor           = 0,                                 // contribution of the RSSI measured by the Friend node used in Friend Offer Delay calculations.
        .receive_window_factor = 0,                                 // contribution of the supported Receive Window used in Friend Offer Delay calculations.
        .min_cache_size_log    = 0,                                 // minimum number of messages that the Friend node can store in its Friend Cache.
        .receive_delay         = 0,                                 // Receive delay in 1 ms units to be requested by the Low Power node.
        .poll_timeout          = 0                                  // Poll timeout in 100ms unite to bt requested by the Low Power node.
    },
    .gatt_client_only          = WICED_TRUE,                        // Can connect to mesh over GATT or ADV
    .elements_num  = (uint8_t)(sizeof(mesh_elements) / sizeof(mesh_elements[0])),   // number of elements on this device
    .elements      = mesh_elements                                  // Array of elements for this device
};

/******************************************************
 *          Structures
 ******************************************************/

/******************************************************
 *          Function Prototypes
 ******************************************************/
extern wiced_bool_t mesh_gatt_client_provision_connect(wiced_bt_mesh_provision_connect_data_t *p_connect, wiced_bool_t use_pb_gatt);
extern void mesh_gatt_client_scan_unprovisioned_devices(uint8_t start);
extern wiced_bool_t mesh_gatt_client_proxy_connect(wiced_bt_mesh_proxy_connect_data_t *p_connect);
//extern wiced_bool_t mesh_gatt_client_provision_disconnect(wiced_bt_mesh_provision_disconnect_data_t *p_disconnect);
extern void mesh_gatt_client_search_proxy(uint8_t start);
extern void mesh_core_state_changed(wiced_bt_mesh_core_state_type_t type, wiced_bt_mesh_core_state_t *p_state);

static void mesh_config_client_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);

extern wiced_result_t wiced_send_gatt_packet( uint16_t opcode, uint8_t* p_data, uint16_t length );
wiced_bool_t wiced_bt_mesh_config_client_message_handler(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint16_t data_len);


/******************************************************
 *               Function Definitions
 ******************************************************/
void mesh_application_init(void)
{
    static int core_initialized = 0;

    extern uint8_t mesh_model_trace_level;
    mesh_model_trace_level = TRACE_INFO;

    // Set the local device key buffer to a larger number for Android platform
    extern uint8_t wiced_bt_mesh_provision_dev_key_max_num;
    wiced_bt_mesh_provision_dev_key_max_num = 64;

    extern void wiced_bt_mesh_core_set_trace_level(uint32_t fids_mask, uint8_t level);
    wiced_bt_mesh_core_set_trace_level(0xffffffff, 4);      //(ALL, TRACE_DEBUG)
    // wiced_bt_mesh_core_set_trace_level(0xffffffff, 0);      //(ALL, TRACE_DEBUG)
    wiced_bt_mesh_core_set_trace_level(0x4, 3);             //(CORE_AES_CCM_C, TRACE_INFO)

    WICED_BT_TRACE("mesh_application_init enter");
    if(!core_initialized)
    {
        // activate IV Update test mode to remove the 96-hour limit
        //wiced_bt_core_iv_update_test_mode = WICED_TRUE;

        core_initialized = 1;
        wiced_bt_mesh_core_init_t   init = { 0 };
#ifdef MESH_SUPPORT_PB_GATT
        mesh_config.features |= WICED_BT_MESH_CORE_FEATURE_BIT_PB_GATT;
#endif
        mesh_config.directed_forward.wanted_rssi = -127;
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
wiced_bt_mesh_core_received_msg_handler_t get_msg_handler_callback(uint16_t company_id, uint16_t opcode, uint16_t *p_model_id, wiced_bool_t *p_dont_save_rpl)
{
    wiced_bt_mesh_core_received_msg_handler_t p_message_handler = NULL;
    uint8_t                                   idx_elem, idx_model;
    wiced_bt_mesh_event_t                     temp_event;
    uint16_t                                  model_id;

#if WICED_BT_MODELS_VERBOSE == 1
    WICED_BT_TRACE("company_id:%x opcode:%s\n", company_id, mesh_opcode_string(opcode));
#else
    WICED_BT_TRACE("company_id:%x opcode:%x\n", company_id, opcode);
#endif
    if (company_id == MESH_COMPANY_ID_UNUSED)
    {
        p_message_handler = wiced_bt_mesh_proxy_client_message_handler;
    }
    else
    {
        temp_event.company_id = company_id;
        temp_event.opcode = opcode;
        temp_event.model_id = 0xffff;   //it is sign of special mode for model to just return model_id without message handling
                                        // model changes it to any other value if it wants do disable RPL saving for its messages

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
                // Check if model wants to disable RPL saving for its messages
                if (temp_event.model_id != 0xffff && p_dont_save_rpl != NULL)
                    *p_dont_save_rpl = WICED_TRUE;
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
    wiced_bt_ble_observe(start, 0, NULL);
}

void mesh_app_init(wiced_bool_t is_provisioned)
{
    WICED_BT_TRACE("mesh_app_init\n");

    wiced_bt_mesh_provision_client_init(mesh_provision_message_handler, is_provisioned);
    wiced_bt_mesh_client_init(mesh_provision_message_handler, is_provisioned);

    wiced_bt_mesh_config_client_init(mesh_config_message_handler, is_provisioned);
    wiced_bt_mesh_health_client_init(mesh_config_message_handler, is_provisioned);
    wiced_bt_mesh_proxy_client_init(mesh_config_message_handler, is_provisioned);
#ifdef MESH_DFU_ENABLED
    wiced_bt_mesh_model_fw_provider_init();
#endif
    wiced_bt_mesh_model_property_client_init(0, mesh_control_message_handler, is_provisioned);
    wiced_bt_mesh_model_onoff_client_init(0, mesh_control_message_handler, is_provisioned);
    wiced_bt_mesh_model_level_client_init(0, mesh_control_message_handler, is_provisioned);
    wiced_bt_mesh_model_power_onoff_client_init(0, mesh_control_message_handler, is_provisioned);
    wiced_bt_mesh_model_light_lightness_client_init(0, mesh_control_message_handler, is_provisioned);
    wiced_bt_mesh_model_light_ctl_client_init(0, mesh_control_message_handler, is_provisioned);
    wiced_bt_mesh_model_light_hsl_client_init(0, mesh_control_message_handler, is_provisioned);
    wiced_bt_mesh_model_default_transition_time_client_init(0, mesh_control_message_handler, is_provisioned);
    wiced_bt_mesh_model_sensor_client_init(0, mesh_sensor_message_handler, is_provisioned);
    wiced_bt_mesh_model_light_lc_client_init(0, mesh_control_message_handler, is_provisioned);
}

void mesh_provision_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    WICED_BT_TRACE("provision message:%d\n", event);
    mesh_provision_process_event(event, p_event, p_data);
}

void mesh_config_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    Log("config message:%d\n", event);
    mesh_provision_process_event(event, p_event, p_data);
}

void mesh_control_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    Log("control message:%d\n", event);
    mesh_provision_process_event(event, p_event, p_data);
}

void mesh_sensor_message_handler(uint8_t element_idx, uint16_t addr, uint16_t event, void *p_data)
{
    Log("sensor message:%d\n", event);
    mesh_sensor_process_event(addr, event, p_data);
}

uint8_t mesh_provisioner_process_proxy_connected(uint8_t* p_data, uint16_t length) {
    int conn_id ;
    int mtu;
    STREAM_TO_UINT32(conn_id, p_data);
    STREAM_TO_UINT16(mtu, p_data);

    wiced_bt_mesh_core_connection_status(conn_id, TRUE,0,mtu);
    return 0;
}

#if 0
wiced_bool_t wiced_bt_mesh_proxy_disconnect(wiced_bt_mesh_provision_disconnect_data_t *p_disconnect)
{
//    return mesh_gatt_client_proxy_disconnect(&p_disconnect)  ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}
#endif

uint32_t mesh_app_on_received_provision_gatt_pkt(uint8_t *p_data, uint32_t length)
{
    // packet has arrived from JAVA
    Log("\n mesh_app_on_received_provision_gatt_pkt\n");
    Logn(p_data,length);
    //TODO send proper connection id from JAVA
    wiced_bt_mesh_core_provision_gatt_packet(WICED_TRUE, 1, p_data, length);
    return 0;
}

static uint16_t write_reg(char name[80], uint8_t *value, uint16_t len, uint32_t *pInt) {
    return len;
}

static uint16_t read_reg(char name[80], uint8_t *value, uint16_t len, uint32_t *pInt) {
    return  len;
}

static uint32_t mesh_nvram_access(wiced_bool_t write, int inx, uint8_t* value, uint16_t len, wiced_result_t *p_result)
{
    char        name[80];
    uint32_t    res;
    //Log(name, sizeof(name), "NVRAM_%d", NVRAM_ID_MESH_START + inx);
    Log(name, sizeof(name), "NVRAM_%d", inx);
    if (!write)
        len = (uint16_t)read_reg(name, value, len, &res);
    else
        len = (uint16_t)write_reg(name, value, len, &res);
    if (p_result)
        //*p_result = res == ERROR_SUCCESS ? WICED_BT_SUCCESS : WICED_BT_ERROR;
        *p_result = WICED_BT_SUCCESS;
    return len;
}

wiced_bool_t wiced_bt_get_upgrade_fw_info(uint16_t *company_id, uint8_t *fw_id_len, uint8_t *fw_id)
{
    return WICED_FALSE;
}

wiced_bool_t wiced_ota_fw_upgrade_get_new_fw_info(uint16_t *company_id, uint8_t *fw_id_len, uint8_t *fw_id)
{
    return WICED_FALSE;
}

uint32_t wiced_firmware_upgrade_init_nv_locations(void)
{
    return 1;
}

uint32_t wiced_firmware_upgrade_store_to_nv(uint32_t offset, uint8_t *data, uint32_t len)
{
    return len;
}

int32_t ota_fw_upgrade_calculate_checksum(int32_t offset, int32_t length)
{
    return 0;
}

wiced_bool_t vendor_data_handler(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint16_t data_len)
{
    char buf[100] = { 0 };

    // 0xffff model_id means request to check if that opcode belongs to that model
    if (p_event->model_id == 0xffff)
    {
        switch (p_event->opcode)
        {
            case MESH_VENDOR_OPCODE1:
            case MESH_VENDOR_OPCODE2:
                break;
            default:
                return WICED_FALSE;
        }
        return WICED_TRUE;
    }

    WICED_BT_TRACE("Vendor Data Opcode:%d\n", p_event->opcode);

    for (int i = 0; i < data_len && i < 33; i++)
        sprintf(&buf[strlen(buf)], "%02x", p_data[i]);

    WICED_BT_TRACE("%s\n", buf);

    meshClientVendorSpecificDataStatus(p_event->src, p_event->company_id, p_event->model_id, (uint8_t)p_event->opcode,(uint8_t)p_event->ttl, p_data, data_len);
    wiced_bt_mesh_release_event(p_event);

    return WICED_TRUE;
}

uint32_t const crc32_table[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32_t update_crc32(uint32_t crc, uint8_t *buf, uint32_t len)
{
    uint32_t c = crc;
    uint32_t n;

    for (n = 0; n < len; n++)
    {
        c = crc32_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }

    return c;
}

void wiced_bt_mesh_add_vendor_model(wiced_bt_mesh_add_vendor_model_data_t* p_data)
{

}

wiced_bool_t wiced_firmware_upgrade_erase_nv(uint32_t start, uint32_t size)
{
    return WICED_TRUE;
}
