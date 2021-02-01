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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include "wiced_memory.h"
#include "wiced_mesh_client.h"
#include "hci_control_api.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_mesh_model_defs.h"
#include "wiced_bt_mesh_models.h"
#include "wiced_bt_mesh_provision.h"

//#define MIBLE
#ifdef MIBLE
#define mesh_provision_process_event mesh_bsa_provision_process_event
#endif

#define MESH_APP_PAYLOAD_OP_LONG              0x80
#define MESH_APP_PAYLOAD_OP_MANUF_SPECIFIC    0x40

#ifndef UINT40_TO_STREAM
#define UINT40_TO_STREAM(p, u40) {*(p)++ = (UINT8)(u40); *(p)++ = (UINT8)((u40) >> 8); *(p)++ = (UINT8)((u40) >> 16); *(p)++ = (UINT8)((u40) >> 24); \
                                  *(p)++ = (UINT8)((u40) >> 32);}
#endif

extern uint8_t wiced_hci_send(uint16_t opcode, uint8_t *p_buffer, uint16_t length);
extern void Log(char *fmt, ...);
uint8_t *wiced_bt_mesh_hci_header_from_event(wiced_bt_mesh_event_t *p_event, uint8_t *p_buffer, uint16_t len);
uint8_t *wiced_bt_mesh_format_hci_header(uint16_t dst, uint16_t app_key_idx, uint8_t element_idx, uint8_t reliable, uint8_t send_segmented, uint8_t ttl, uint8_t retransmit_count, uint8_t retransmit_interval, uint8_t reply_timeout, uint8_t *p_buffer, uint16_t len);
wiced_bt_mesh_event_t *wiced_bt_mesh_event_from_hci_header(uint8_t **p_buffer, uint16_t *len);
void wiced_bt_free_buffer(void *buf);
void *wiced_bt_get_buffer(uint16_t size);

static void process_node_reset_status(uint8_t *p_buffer, uint16_t len);
static void process_core_seq_changed(uint8_t *p_buffer, uint16_t len);
static void process_core_raw_model_data(uint8_t *p_buffer, uint16_t len);
static void process_provision_scan_capabilities_status(uint8_t *p_buffer, uint16_t len);
static void process_provision_scan_status(uint8_t *p_buffer, uint16_t len);
static void process_provision_scan_report(uint8_t *p_buffer, uint16_t len);
static void process_provision_scan_extended_report(uint8_t *p_buffer, uint16_t len);
static void process_provision_command_status(uint8_t *p_buffer, uint16_t len);
static void process_tx_complete(uint8_t *p_buffer, uint16_t len);
static void process_provision_end(uint8_t *p_buffer, uint16_t len);
static void process_proxy_connection_status(uint8_t *p_buffer, uint16_t len);
static void process_provision_link_report(uint8_t *p_buffer, uint16_t len);
static void process_provision_device_capabilities(uint8_t *p_buffer, uint16_t len);
static void process_provision_oob_data(uint8_t *p_buffer, uint16_t len);
static void process_composition_data_status(uint8_t *p_buffer, uint16_t len);
static void process_net_key_status(uint8_t *p_buffer, uint16_t len);
static void process_app_key_status(uint8_t *p_buffer, uint16_t len);
static void process_key_refresh_phase_status(uint8_t *p_buffer, uint16_t len);
static void process_model_app_bind_status(uint8_t *p_buffer, uint16_t len);
static void process_model_sub_status(uint8_t *p_buffer, uint16_t len);
static void process_model_pub_status(uint8_t *p_buffer, uint16_t len);
static void process_net_transmit_status(uint8_t *p_buffer, uint16_t len);
static void process_default_ttl_status(uint8_t *p_buffer, uint16_t len);
static void process_relay_status(uint8_t *p_buffer, uint16_t len);
static void process_friend_status(uint8_t *p_buffer, uint16_t len);
static void process_gatt_proxy_status(uint8_t *p_buffer, uint16_t len);
static void process_beacon_status(uint8_t *p_buffer, uint16_t len);
static void process_node_identity_status(uint8_t *p_buffer, uint16_t len);
static void process_proxy_filter_status(uint8_t *p_buffer, uint16_t len);
static void process_def_trans_time_status(uint8_t *p_buffer, uint16_t len);
static void process_on_off_status(uint8_t *p_buffer, uint16_t len);
static void process_level_status(uint8_t *p_buffer, uint16_t len);
static void process_lightness_status(uint8_t *p_buffer, uint16_t len);
static void process_hsl_status(uint8_t *p_buffer, uint16_t len);
static void process_ctl_status(uint8_t *p_buffer, uint16_t len);
static void process_sensor_descriptor_status(uint8_t *p_buffer, uint16_t len);
static void process_sensor_settings_status(uint8_t *p_buffer, uint16_t len);
static void process_sensor_cadence_status(uint8_t *p_buffer, uint16_t len);
static void process_sensor_data_status(uint8_t *p_buffer, uint16_t len);
static void process_properties_status(uint8_t *p_buffer, uint16_t len);
static void process_property_status(uint8_t *p_buffer, uint16_t len);
static void process_vendor_data(uint8_t *p_buffer, uint16_t len);

extern void mesh_provision_process_event(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
extern void mesh_sensor_process_event(uint16_t addr, uint16_t event, void* p_data);

void wiced_hci_process_data(uint16_t opcode, uint8_t *p_buffer, uint16_t len)
{
#ifdef MIBLE
    extern void mible_wiced_set_event(void);
    if ((opcode != HCI_CONTROL_MESH_EVENT_COMMAND_STATUS) && (opcode != HCI_CONTROL_MESH_EVENT_TX_COMPLETE))
        mible_wiced_set_event();
#endif

    switch (opcode)
    {
    case HCI_CONTROL_MESH_EVENT_COMMAND_STATUS:
        process_provision_command_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_TX_COMPLETE:
        process_tx_complete(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_CORE_SEQ_CHANGED:
        process_core_seq_changed(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_RAW_MODEL_DATA:
        process_core_raw_model_data(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROVISION_SCAN_CAPABILITIES_STATUS:
        process_provision_scan_capabilities_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROVISION_SCAN_STATUS:
        process_provision_scan_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROVISION_SCAN_REPORT:
        process_provision_scan_report(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROVISION_SCAN_EXTENDED_REPORT:
        process_provision_scan_extended_report(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_NODE_RESET_STATUS:
        process_node_reset_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROVISION_END:
        process_provision_end(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROXY_CONNECTION_STATUS:
        process_proxy_connection_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROVISION_LINK_REPORT:
        process_provision_link_report(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROVISION_DEVICE_CAPABITIES:
        process_provision_device_capabilities(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROVISION_OOB_DATA:
        process_provision_oob_data(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_COMPOSITION_DATA_STATUS:
        process_composition_data_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_NETKEY_STATUS:
        process_net_key_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_APPKEY_STATUS:
        process_app_key_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_KEY_REFRESH_PHASE_STATUS:
        process_key_refresh_phase_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_MODEL_APP_BIND_STATUS:
        process_model_app_bind_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_MODEL_SUBSCRIPTION_STATUS:
        process_model_sub_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_MODEL_PUBLICATION_STATUS:
        process_model_pub_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_NETWORK_TRANSMIT_PARAMS_STATUS:
        process_net_transmit_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_DEFAULT_TTL_STATUS:
        process_default_ttl_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_RELAY_STATUS:
        process_relay_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_FRIEND_STATUS:
        process_friend_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_GATT_PROXY_STATUS:
        process_gatt_proxy_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_BEACON_STATUS:
        process_beacon_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_NODE_IDENTITY_STATUS:
        process_node_identity_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROXY_FILTER_STATUS:
        process_proxy_filter_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_DEF_TRANS_TIME_STATUS:
        process_def_trans_time_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_ONOFF_STATUS:
        process_on_off_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LEVEL_STATUS:
        process_level_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_LIGHTNESS_STATUS:
        process_lightness_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_HSL_STATUS:
        process_hsl_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_LIGHT_CTL_STATUS:
        process_ctl_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_SENSOR_DESCRIPTOR_STATUS:
        process_sensor_descriptor_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_SENSOR_SETTINGS_STATUS:
        process_sensor_settings_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_SENSOR_CADENCE_STATUS:
        process_sensor_cadence_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_SENSOR_STATUS:
        process_sensor_data_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROPERTIES_STATUS:
        process_properties_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_PROPERTY_STATUS:
        process_property_status(p_buffer, len);
        break;
    case HCI_CONTROL_MESH_EVENT_VENDOR_DATA:
        process_vendor_data(p_buffer, len);
        break;
    default:
        Log("Rcvd Unknown Op Code: 0x%04x", opcode);
        break;
    }
}

void process_provision_command_status(uint8_t *p_buffer, uint16_t len)
{
    mesh_provision_process_event(WICED_BT_MESH_COMMAND_STATUS, NULL, NULL);
}

void process_core_seq_changed(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_core_state_seq_t data;
    data.addr = p_buffer[0] + ((uint16_t)p_buffer[1] << 8);
    data.seq = p_buffer[2] + ((uint32_t)p_buffer[3] << 8) + ((uint32_t)p_buffer[4] << 16) + ((uint32_t)p_buffer[5] << 24);
    data.previous_iv_idx = p_buffer[6];
    data.rpl_entry_idx = p_buffer[7] + ((uint16_t)p_buffer[8] << 8);
    mesh_provision_process_event(WICED_BT_MESH_SEQ_CHANGED, NULL, &data);
}

void process_core_raw_model_data(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    if ((p_buffer[0] & (MESH_APP_PAYLOAD_OP_LONG | MESH_APP_PAYLOAD_OP_MANUF_SPECIFIC)) == (MESH_APP_PAYLOAD_OP_LONG | MESH_APP_PAYLOAD_OP_MANUF_SPECIFIC))
    {
        p_event->opcode = p_buffer[0] & ~(MESH_APP_PAYLOAD_OP_LONG | MESH_APP_PAYLOAD_OP_MANUF_SPECIFIC);
        p_event->company_id = (p_buffer[1] << 8) + p_buffer[2];
        p_buffer += 3;
        len -= 3;
    }
    else
    {
        p_event->company_id = MESH_COMPANY_ID_BT_SIG;
        p_event->opcode = (p_buffer[0] << 8) + p_buffer[1];
        p_buffer += 2;
        len -= 2;
    }
    p_event->data_len = len;

    mesh_provision_process_event(WICED_BT_MESH_RAW_MODEL_DATA, p_event, p_buffer);
}

void process_provision_scan_capabilities_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_provision_scan_capabilities_status_data_t data;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.max_scanned_items = p_buffer[0];
    mesh_provision_process_event(WICED_BT_MESH_PROVISION_SCAN_CAPABILITIES_STATUS, p_event, &data);
}

void process_provision_scan_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_provision_scan_status_data_t data;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.status = p_buffer[0];
    data.state = p_buffer[1];
    data.scanned_items_limit = p_buffer[2];
    data.timeout = p_buffer[3];
    mesh_provision_process_event(WICED_BT_MESH_PROVISION_SCAN_STATUS, p_event, &data);
}

void process_provision_scan_report(uint8_t *p_buffer, uint16_t len)
{
#ifdef PROVISION_SCAN_REPORT_INCLUDE_BDADDR
    int i;
#endif
    wiced_bt_mesh_provision_scan_report_data_t data;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.rssi = p_buffer[0];
    memcpy(data.uuid, &p_buffer[1], 16);
    data.oob = p_buffer[17] + (p_buffer[18] << 8);
    data.uri_hash = p_buffer[19] + ((uint32_t)p_buffer[20] << 8) + ((uint32_t)p_buffer[21] << 16) + ((uint32_t)p_buffer[22] << 24);
#ifdef PROVISION_SCAN_REPORT_INCLUDE_BDADDR
    for (i = 0; i < BD_ADDR_LEN; i++)
        data.bdaddr[5 - i] = p_buffer[23 + i];
#endif
    mesh_provision_process_event(WICED_BT_MESH_PROVISION_SCAN_REPORT, p_event, &data);
}

void process_provision_scan_extended_report(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_provision_scan_extended_report_data_t data;
    uint8_t *p_adv_data, *p;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    memset(&data, 0, sizeof(data));
    data.status = p_buffer[0];
    memcpy(data.uuid, &p_buffer[1], 16);
    if (len >= 19)
    {
        data.oob = p_buffer[17] + (p_buffer[18] << 8);
        len -= 19;
        p = data.adv_data;
        p_adv_data = &p_buffer[19];
        while (len > 0)
        {
            if (*p_adv_data == 0)
                break;

            if ((*p_adv_data + 1 > len) || ((p + *p_adv_data + 1) > (data.adv_data + sizeof(data.adv_data))))
            {
                Log("bad adv data:");
                break;
            }
            memcpy(p, p_adv_data, *p_adv_data + 1);
            p += *p_adv_data + 1;
            len -= (*p_adv_data + 1);
            p_adv_data += *p_adv_data + 1;
        }
    }
    mesh_provision_process_event(WICED_BT_MESH_PROVISION_SCAN_EXTENDED_REPORT, p_event, &data);
}

void process_provision_end(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_provision_status_data_t data;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.provisioner_addr = p_buffer[0] + ((uint16_t)p_buffer[1] << 8);
    data.addr             = p_buffer[2] + ((uint16_t)p_buffer[3] << 8);
    data.net_key_idx      = p_buffer[4] + ((uint16_t)p_buffer[5] << 8);
    data.result           = p_buffer[6];
    memcpy(data.dev_key, &p_buffer[7], 16);
    mesh_provision_process_event(WICED_BT_MESH_PROVISION_END, p_event, &data);
}

void process_proxy_connection_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_connect_status_data_t data;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.provisioner_addr = p_buffer[0] + ((uint16_t)p_buffer[1] << 8);
    data.addr             = p_buffer[2] + ((uint16_t)p_buffer[3] << 8);
    data.connected        = p_buffer[4];
    data.over_gatt        = p_buffer[5];
    mesh_provision_process_event(WICED_BT_MESH_PROXY_CONNECTION_STATUS, p_event, &data);
}

void process_provision_link_report(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_provision_link_report_data_t data;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.link_status = p_buffer[0];
    data.rpr_state = p_buffer[1];
    data.reason = p_buffer[2];
    data.over_gatt = p_buffer[3];
    mesh_provision_process_event(WICED_BT_MESH_PROVISION_LINK_REPORT, p_event, &data);
}

void process_provision_device_capabilities(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_provision_device_capabilities_data_t data;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.provisioner_addr  = p_buffer[0] + ((uint16_t)p_buffer[1] << 8);
    data.elements_num      = p_buffer[2];
    data.algorithms        = p_buffer[3] + ((uint16_t)p_buffer[4] << 8);
    data.pub_key_type      = p_buffer[5];
    data.static_oob_type   = p_buffer[6];
    data.output_oob_size   = p_buffer[7];
    data.output_oob_action = p_buffer[8] + ((uint16_t)p_buffer[9] << 8);
    data.input_oob_size    = p_buffer[10];
    data.input_oob_action  = p_buffer[11] + ((uint16_t)p_buffer[12] << 8);
    mesh_provision_process_event(WICED_BT_MESH_PROVISION_DEVICE_CAPABILITIES, p_event, &data);
}

void process_provision_oob_data(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_provision_device_oob_request_data_t data;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.provisioner_addr = p_buffer[0] + ((uint16_t)p_buffer[1] << 8);
    data.type = p_buffer[2];
    data.size = p_buffer[3];
    data.action = p_buffer[4];
    mesh_provision_process_event(WICED_BT_MESH_PROVISION_GET_OOB_DATA, p_event, &data);
}

void process_tx_complete(uint8_t *p_buffer, uint16_t len)
{
    uint16_t addr;
    uint16_t hci_opcode;
    uint8_t  tx_status;

    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    hci_opcode = p_buffer[0] + (p_buffer[1] << 8);
    tx_status = p_buffer[2];
    addr = p_buffer[3] + (p_buffer[4] << 8);
    if (tx_status == TX_STATUS_FAILED)
    {
        Log("Node unreachable: opcode:%x addr:%x", hci_opcode, addr);
    }
    else
    {
        // Log("Mesh Tx Complete: opcode:%x addr:%x", hci_opcode, addr);
    }
    wiced_bt_mesh_release_event(p_event);
}

void process_node_reset_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event)
    {
        mesh_provision_process_event(WICED_BT_MESH_CONFIG_NODE_RESET_STATUS, p_event, NULL);
    }
}

void process_composition_data_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    wiced_bt_mesh_config_composition_data_status_data_t *p_comp_data = (wiced_bt_mesh_config_composition_data_status_data_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_config_composition_data_status_data_t) + len - 1);
    if (p_comp_data == NULL)
    {
        wiced_bt_mesh_release_event(p_event);
        return;
    }
    p_comp_data->page_number = p_buffer[0];
    p_comp_data->data_len    = len - 1;
    memcpy(p_comp_data->data, &p_buffer[1], len - 1);

    mesh_provision_process_event(WICED_BT_MESH_CONFIG_COMPOSITION_DATA_STATUS, p_event, p_comp_data);
}

void process_net_key_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    wiced_bt_mesh_config_netkey_status_data_t data;

    data.status = p_buffer[0];
    data.net_key_idx = p_buffer[1] + ((uint16_t)p_buffer[2] << 8);

    mesh_provision_process_event(WICED_BT_MESH_CONFIG_NETKEY_STATUS, p_event, &data);
}

void process_key_refresh_phase_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    wiced_bt_mesh_config_key_refresh_phase_status_data_t data;

    data.status = p_buffer[0];
    data.net_key_idx = p_buffer[1] + ((uint16_t)p_buffer[2] << 8);
    data.phase = p_buffer[3];

    mesh_provision_process_event(WICED_BT_MESH_CONFIG_KEY_REFRESH_PHASE_STATUS, p_event, &data);
}

void process_app_key_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    wiced_bt_mesh_config_appkey_status_data_t data;

    data.status = p_buffer[0];
    data.net_key_idx = p_buffer[1] + ((uint16_t)p_buffer[2] << 8);
    data.app_key_idx = p_buffer[3] + ((uint16_t)p_buffer[4] << 8);

    mesh_provision_process_event(WICED_BT_MESH_CONFIG_APPKEY_STATUS, p_event, &data);
}

void process_model_app_bind_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    wiced_bt_mesh_config_model_app_bind_status_data_t data;

    data.status         = p_buffer[0];
    data.element_addr   = p_buffer[1] + ((uint16_t)p_buffer[2] << 8);
    data.company_id     = p_buffer[3] + ((uint32_t)p_buffer[4] << 8);
    data.model_id       = p_buffer[5] + ((uint32_t)p_buffer[6] << 8);
    data.app_key_idx    = p_buffer[7] + ((uint16_t)p_buffer[8] << 8);

    mesh_provision_process_event(WICED_BT_MESH_CONFIG_MODEL_APP_BIND_STATUS, p_event, &data);
}

void process_model_sub_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    wiced_bt_mesh_config_model_subscription_status_data_t data;

    data.status = p_buffer[0];
    data.element_addr = p_buffer[1] + ((uint16_t)p_buffer[2] << 8);
    data.company_id = p_buffer[3] + ((uint32_t)p_buffer[4] << 8);
    data.model_id = p_buffer[5] + ((uint32_t)p_buffer[6] << 8);
    data.addr = p_buffer[7] + ((uint16_t)p_buffer[8] << 8);

    mesh_provision_process_event(WICED_BT_MESH_CONFIG_MODEL_SUBSCRIPTION_STATUS, p_event, &data);
}

void process_model_pub_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    wiced_bt_mesh_config_model_publication_status_data_t data;

    data.status = p_buffer[0];
    data.element_addr = p_buffer[1] + ((uint16_t)p_buffer[2] << 8);
    data.company_id = p_buffer[3] + ((uint32_t)p_buffer[4] << 8);
    data.model_id = p_buffer[5] + ((uint32_t)p_buffer[6] << 8);
    data.publish_addr = p_buffer[7] + ((uint16_t)p_buffer[8] << 8);
    data.app_key_idx = p_buffer[9] + ((uint16_t)p_buffer[10] << 8);
    data.credential_flag = p_buffer[11];
    data.publish_ttl = p_buffer[12];
    data.publish_period = p_buffer[13] + ((uint32_t)p_buffer[14] << 8) + ((uint32_t)p_buffer[15] << 16) + ((uint32_t)p_buffer[16] << 24);
    data.publish_retransmit_count = p_buffer[17];
    data.publish_retransmit_interval = p_buffer[18] + ((uint16_t)p_buffer[19] << 8);

    mesh_provision_process_event(WICED_BT_MESH_CONFIG_MODEL_PUBLICATION_STATUS, p_event, &data);
}

void process_net_transmit_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    wiced_bt_mesh_config_network_transmit_status_data_t data;

    data.count = p_buffer[0];
    data.interval = p_buffer[1] + ((uint16_t)p_buffer[2] << 8);

    mesh_provision_process_event(WICED_BT_MESH_CONFIG_NETWORK_TRANSMIT_STATUS, p_event, &data);
}

void process_default_ttl_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    wiced_bt_mesh_config_default_ttl_status_data_t data;

    data.ttl = p_buffer[0];
    data.received_ttl = p_buffer[1];

    mesh_provision_process_event(WICED_BT_MESH_CONFIG_DEFAULT_TTL_STATUS, p_event, &data);
}

void process_relay_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    wiced_bt_mesh_config_relay_status_data_t data;

    data.state = p_buffer[0];
    data.retransmit_count = p_buffer[1];
    data.retransmit_interval = p_buffer[2] + ((uint16_t)p_buffer[3] << 8);

    mesh_provision_process_event(WICED_BT_MESH_CONFIG_RELAY_STATUS, p_event, &data);
}

void process_friend_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    wiced_bt_mesh_config_friend_status_data_t data;

    data.state = p_buffer[0];

    mesh_provision_process_event(WICED_BT_MESH_CONFIG_FRIEND_STATUS, p_event, &data);
}

void process_gatt_proxy_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_config_gatt_proxy_status_data_t data;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.state = p_buffer[0];

    mesh_provision_process_event(WICED_BT_MESH_CONFIG_GATT_PROXY_STATUS, p_event, &data);
}

void process_beacon_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_config_beacon_status_data_t data;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.state = p_buffer[0];

    mesh_provision_process_event(WICED_BT_MESH_CONFIG_BEACON_STATUS, p_event, &data);
}

void process_node_identity_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_config_node_identity_status_data_t data;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.status = p_buffer[0];
    data.net_key_idx = p_buffer[1] + (p_buffer[2] << 8);
    data.identity = p_buffer[3];

    mesh_provision_process_event(WICED_BT_MESH_CONFIG_NODE_IDENTITY_STATUS, p_event, &data);
}

void process_proxy_filter_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_proxy_filter_status_data_t data;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.type = p_buffer[0];
    data.list_size = p_buffer[1] + (p_buffer[2] << 8);

    mesh_provision_process_event(WICED_BT_MESH_PROXY_FILTER_STATUS, p_event, &data);
}

void process_def_trans_time_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_default_transition_time_data_t data;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event != NULL)
    {
        p_event->app_key_idx = p_buffer[0] + ((uint16_t)p_buffer[1] << 8);
        p_event->element_idx = p_buffer[2];
        data.time = p_buffer[3] + ((uint32_t)p_buffer[4] << 8) + ((uint32_t)p_buffer[5] << 16) + ((uint32_t)p_buffer[6] << 24);

        mesh_provision_process_event(WICED_BT_MESH_DEFAULT_TRANSITION_TIME_STATUS, p_event, &data);
    }
}

void process_on_off_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_onoff_status_data_t data;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.present_onoff = p_buffer[0];
    data.target_onoff = p_buffer[1];
    data.remaining_time = p_buffer[2] + ((uint32_t)p_buffer[3] << 8) + ((uint32_t)p_buffer[4] << 16) + ((uint32_t)p_buffer[5] << 24);

    mesh_provision_process_event(WICED_BT_MESH_ONOFF_STATUS, p_event, &data);
}

void process_level_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_level_status_data_t data;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.present_level = p_buffer[0] + ((uint16_t)p_buffer[1] << 8);;
    data.target_level = p_buffer[2] + ((uint16_t)p_buffer[3] << 8);
    data.remaining_time = p_buffer[4] + ((uint32_t)p_buffer[5] << 8) + ((uint32_t)p_buffer[6] << 16) + ((uint32_t)p_buffer[7] << 24);

    mesh_provision_process_event(WICED_BT_MESH_LEVEL_STATUS, p_event, &data);
}

void process_lightness_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_light_lightness_status_data_t data;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.present = p_buffer[0] + ((uint16_t)p_buffer[1] << 8);;
    data.target = p_buffer[2] + ((uint16_t)p_buffer[3] << 8);
    data.remaining_time = p_buffer[4] + ((uint32_t)p_buffer[5] << 8) + ((uint32_t)p_buffer[6] << 16) + ((uint32_t)p_buffer[7] << 24);

    mesh_provision_process_event(WICED_BT_MESH_LIGHT_LIGHTNESS_STATUS, p_event, &data);
}

void process_hsl_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_light_hsl_status_data_t data;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.present.lightness = p_buffer[0] + ((uint16_t)p_buffer[1] << 8);;
    data.present.hue = p_buffer[2] + ((uint16_t)p_buffer[3] << 8);
    data.present.saturation = p_buffer[4] + ((uint16_t)p_buffer[5] << 8);
    data.remaining_time = p_buffer[6] + ((uint32_t)p_buffer[7] << 8) + ((uint32_t)p_buffer[8] << 16) + ((uint32_t)p_buffer[9] << 24);

    mesh_provision_process_event(WICED_BT_MESH_LIGHT_HSL_STATUS, p_event, &data);
}

void process_ctl_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_light_ctl_status_data_t data;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.present.lightness = p_buffer[0] + ((uint16_t)p_buffer[1] << 8);;
    data.present.temperature = p_buffer[2] + ((uint16_t)p_buffer[3] << 8);
    data.target.lightness = p_buffer[4] + ((uint16_t)p_buffer[5] << 8);
    data.target.temperature = p_buffer[6] + ((uint16_t)p_buffer[7] << 8);
    data.remaining_time = p_buffer[8] + ((uint32_t)p_buffer[9] << 8) + ((uint32_t)p_buffer[10] << 16) + ((uint32_t)p_buffer[11] << 24);

    mesh_provision_process_event(WICED_BT_MESH_LIGHT_CTL_STATUS, p_event, &data);
}

void process_sensor_descriptor_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_sensor_descriptor_status_data_t data = { 0 };
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    while (len >= 9)
    {
        data.descriptor_list[data.num_descriptors].property_id = p_buffer[0] + ((uint16_t)p_buffer[1] << 8);;
        data.descriptor_list[data.num_descriptors].positive_tolerance = p_buffer[2] + ((uint16_t)p_buffer[3] << 8);
        data.descriptor_list[data.num_descriptors].negative_tolerance = p_buffer[4] + ((uint16_t)p_buffer[5] << 8);
        data.descriptor_list[data.num_descriptors].sampling_function = p_buffer[6];
        data.descriptor_list[data.num_descriptors].measurement_period = p_buffer[7];
        data.descriptor_list[data.num_descriptors].update_interval = p_buffer[8];
        data.num_descriptors++;
        len -= 9;
    }
    mesh_sensor_process_event(p_event->src, WICED_BT_MESH_SENSOR_DESCRIPTOR_STATUS, &data);
    wiced_bt_mesh_release_event(p_event);
}

void process_sensor_settings_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_sensor_settings_status_data_t data = { 0 };
    int i = 0;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.property_id = p_buffer[0] + ((uint16_t)p_buffer[1] << 8);
    while (len >= 4)
    {
        len -= 2;
        p_buffer += 2;
        data.setting_property_id_list[i++] = p_buffer[0] + ((uint16_t)p_buffer[1] << 8);
    }
    mesh_sensor_process_event(p_event->src, WICED_BT_MESH_SENSOR_SETTINGS_STATUS, &data);
    wiced_bt_mesh_release_event(p_event);
}

void process_sensor_cadence_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_sensor_cadence_status_data_t data = { 0 };
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.property_id = p_buffer[0] + ((uint16_t)p_buffer[1] << 8);
    if (len > 2)
    {
        data.cadence_data.fast_cadence_period_divisor = p_buffer[2] + ((uint16_t)p_buffer[3] << 8);
        data.cadence_data.trigger_type       = p_buffer[4];
        data.cadence_data.trigger_delta_down = p_buffer[5]  + ((uint32_t)p_buffer[6] << 8)  + ((uint32_t)p_buffer[7] << 16)  + ((uint32_t)p_buffer[8] << 24);
        data.cadence_data.trigger_delta_up   = p_buffer[9] + ((uint32_t)p_buffer[10] << 8) + ((uint32_t)p_buffer[11] << 16) + ((uint32_t)p_buffer[12] << 24);
        data.cadence_data.min_interval       = p_buffer[13] + ((uint32_t)p_buffer[14] << 8) + ((uint32_t)p_buffer[15] << 16) + ((uint32_t)p_buffer[16] << 24);
        data.cadence_data.fast_cadence_low   = p_buffer[17] + ((uint32_t)p_buffer[18] << 8) + ((uint32_t)p_buffer[19] << 16) + ((uint32_t)p_buffer[20] << 24);
        data.cadence_data.fast_cadence_high  = p_buffer[21] + ((uint32_t)p_buffer[22] << 8) + ((uint32_t)p_buffer[23] << 16) + ((uint32_t)p_buffer[24] << 24);
    }
    mesh_sensor_process_event(p_event->src, WICED_BT_MESH_SENSOR_CADENCE_STATUS, &data);
    wiced_bt_mesh_release_event(p_event);
}

void process_sensor_data_status(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_sensor_status_data_t data = { 0 };
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    data.property_id = p_buffer[0] + ((uint16_t)p_buffer[1] << 8);
    data.prop_value_len = p_buffer[2];
    memcpy(data.raw_value, p_buffer + 3, data.prop_value_len);

    mesh_sensor_process_event(p_event->src, WICED_BT_MESH_SENSOR_STATUS, &data);
    wiced_bt_mesh_release_event(p_event);
}

void process_properties_status(uint8_t *p_buffer, uint16_t len)
{
    uint16_t event = 0;
    int i = 0;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    if (len < 1)
    {
        wiced_bt_mesh_release_event(p_event);
        return;
    }
    uint16_t num_props = (len - 1) / 2;
    wiced_bt_mesh_properties_status_data_t *p_data = (wiced_bt_mesh_properties_status_data_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_properties_status_data_t) + len - 2);
    if (p_data == NULL)
    {
        wiced_bt_mesh_release_event(p_event);
        return;
    }
    p_data->type = p_buffer[0];
    p_data->properties_num = (len - 1) / 2;
    p_buffer++;
    for (i = 0; i < p_data->properties_num; i++)
    {
        p_data->id[i] = p_buffer[0] + (p_buffer[1] << 8);
        p_buffer += 2;
    }
    if (p_data->type == WICED_BT_MESH_PROPERTY_TYPE_ADMIN)
        event = WICED_BT_MESH_ADMIN_PROPERTIES_STATUS;
    else if (p_data->type == WICED_BT_MESH_PROPERTY_TYPE_CLIENT)
        event = WICED_BT_MESH_CLIENT_PROPERTIES_STATUS;
    else if (p_data->type == WICED_BT_MESH_PROPERTY_TYPE_MANUFACTURER)
        event = WICED_BT_MESH_MANUF_PROPERTIES_STATUS;
    else
        event = WICED_BT_MESH_USER_PROPERTIES_STATUS;
    mesh_provision_process_event(WICED_BT_MESH_MANUF_PROPERTIES_STATUS, p_event, p_data);
    wiced_bt_free_buffer(p_data);
}

void process_property_status(uint8_t *p_buffer, uint16_t len)
{
    uint16_t event = 0;
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    if (len < 4)
    {
        wiced_bt_mesh_release_event(p_event);
        return;
    }

    wiced_bt_mesh_property_status_data_t* p_data = (wiced_bt_mesh_property_status_data_t*)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_property_status_data_t) + len - 5);
    if (p_data == NULL)
    {
        wiced_bt_mesh_release_event(p_event);
        return;
    }
    p_data->type = p_buffer[0];
    p_data->access = p_buffer[1];
    p_data->id = p_buffer[2] + (p_buffer[3] << 8);
    memcpy(p_data->value, &p_buffer[4], len - 4);
    if (p_data->type == WICED_BT_MESH_PROPERTY_TYPE_ADMIN)
        event = WICED_BT_MESH_ADMIN_PROPERTY_STATUS;
    else if (p_data->type == WICED_BT_MESH_PROPERTY_TYPE_MANUFACTURER)
        event = WICED_BT_MESH_MANUF_PROPERTY_STATUS;
    else
        event = WICED_BT_MESH_USER_PROPERTY_STATUS;
    mesh_provision_process_event(event, p_event, p_data);
    wiced_bt_free_buffer(p_data);
}

void process_vendor_data(uint8_t *p_buffer, uint16_t len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_buffer, &len);
    if (p_event == NULL)
        return;

    p_event->data_len = len;
    mesh_provision_process_event(WICED_BT_MESH_VENDOR_DATA, p_event, p_buffer);
}

wiced_bool_t wiced_bt_mesh_provision_connect(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_connect_data_t *p_data, uint8_t use_gatt)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));
    int i;

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_FALSE;

    for (i = 0; i < sizeof(p_data->uuid); i++)
        *p++ = p_data->uuid[i];
    *p++ = p_data->identify_duration;
    *p++ = p_data->procedure;
    *p++ = use_gatt;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_PROVISION_CONNECT, buffer, (uint16_t)(p - buffer));
}

wiced_bool_t wiced_bt_mesh_provision_start(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_start_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_FALSE;

    *p++ = (uint8_t)p_data->addr;
    *p++ = (uint8_t)(p_data->addr >> 8);
    *p++ = (uint8_t)p_data->net_key_idx;
    *p++ = (uint8_t)(p_data->net_key_idx >> 8);
    *p++ = p_data->algorithm;
    *p++ = p_data->public_key_type;
    *p++ = p_data->auth_method;
    *p++ = p_data->auth_action;
    *p++ = p_data->auth_size;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_PROVISION_START, buffer, (uint16_t)(p - buffer));
}

wiced_bool_t wiced_bt_mesh_provision_client_set_oob(wiced_bt_mesh_event_t *p_event, uint8_t* p_oob, uint32_t len)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_FALSE;

    memcpy(p, p_oob, len);
    p += len;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_PROVISION_OOB_VALUE, buffer, (uint16_t)(p - buffer));
}

wiced_bool_t wiced_bt_mesh_provision_disconnect(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_FALSE;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_PROVISION_DISCONNECT, buffer, (uint16_t)(p - buffer));
}

wiced_bool_t wiced_bt_mesh_client_proxy_connect(wiced_bt_mesh_proxy_connect_data_t *p_connect)
{
    uint8_t buffer[128];
    uint8_t *p = buffer;
    int i;
    if (p_connect->connect_type == CONNECT_TYPE_NODE_ID)
    {
        *p++ = p_connect->node_id & 0xff;
        *p++ = (p_connect->node_id >> 8) & 0xff;
    }
    else if (p_connect->connect_type == CONNECT_TYPE_BDADDR)
    {
        for (i = 0; i < 6; i++)
            *p++ = p_connect->bd_addr[5 - i];
        *p++ = p_connect->bd_addr_type;
    }
    *p++ = p_connect->scan_duration;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_PROXY_CONNECT, buffer, (uint16_t)(p - buffer));
}

wiced_bool_t wiced_bt_mesh_client_proxy_disconnect(void)
{
    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_PROXY_DISCONNECT, NULL, 0);
}

wiced_result_t wiced_bt_mesh_core_send(wiced_bt_mesh_event_t *p_event, const uint8_t* params, uint16_t params_len, wiced_bt_mesh_core_send_complete_callback_t complete_callback)
{
    uint8_t buffer[512];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    if (p == NULL)
    {
        wiced_bt_mesh_release_event(p_event);
        return WICED_FALSE;
    }
#define MESH_APP_PAYLOAD_OP_LONG              0x80
#define MESH_APP_PAYLOAD_OP_MANUF_SPECIFIC    0x40
    if (p_event->company_id != MESH_COMPANY_ID_BT_SIG)
    {
        *p++ = (p_event->opcode | MESH_APP_PAYLOAD_OP_LONG | MESH_APP_PAYLOAD_OP_MANUF_SPECIFIC) & 0xff;
        *p++ = (p_event->company_id >> 8) & 0xff;
        *p++ = p_event->company_id & 0xff;
    }
    else
    {
        *p++ = (p_event->opcode >> 8) & 0xff;
        *p++ = p_event->opcode & 0xff;
    }
    memcpy(p, params, params_len);
    p += params_len;

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_RAW_MODEL_DATA, buffer, (uint16_t)(p - buffer));
    wiced_bt_mesh_release_event(p_event);
    return WICED_BT_SUCCESS;
}

void wiced_bt_mesh_provision_set_dev_key(wiced_bt_mesh_set_dev_key_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = buffer;
    *p++ = p_data->dst & 0xff;
    *p++ = (p_data->dst >> 8) & 0xff;
    memcpy(p, p_data->dev_key, 16);
    p += 16;
    *p++ = p_data->net_key_idx & 0xff;
    *p++ = (p_data->net_key_idx >> 8) & 0xff;

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_SET_DEVICE_KEY, buffer, (uint16_t)(p - buffer));
}

void wiced_bt_mesh_adv_tx_power_set(uint8_t adv_tx_power)
{
    uint8_t buffer[260] = {0};
    uint8_t *p = buffer;
    p[0] = adv_tx_power;
    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_SET_ADV_TX_POWER, buffer, 1);
}

void wiced_bt_mesh_add_vendor_model(wiced_bt_mesh_add_vendor_model_data_t *p_data)
{
    uint8_t buffer[260] = {0};
    uint8_t *p = buffer;
    *p++ = p_data->company_id & 0xff;
    *p++ = (p_data->company_id >> 8) & 0xff;
    *p++ = p_data->model_id & 0xff;
    *p++ = (p_data->model_id >> 8) & 0xff;
    if (p_data->num_opcodes > WICED_BT_MESH_MAX_VENDOR_MODEL_OPCODES)
        p_data->num_opcodes = WICED_BT_MESH_MAX_VENDOR_MODEL_OPCODES;
    *p++ = p_data->num_opcodes;
    memcpy(p, p_data->opcode, p_data->num_opcodes * 3); // vendor model opcodes are 3-bytes long
    p += p_data->num_opcodes * 3;

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_ADD, buffer, (uint16_t)(p - buffer));
}

wiced_bool_t wiced_bt_mesh_config_composition_data_get(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_composition_data_get_data_t *p_get)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    if (p == NULL)
    {
        wiced_bt_mesh_release_event(p_event);
        return WICED_FALSE;
    }

    *p++ = p_get->page_number;

    Log("Composition Data Get addr:0x%04x page_number:%x", p_event->dst, p_get->page_number);

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_CONFIG_COMPOSITION_DATA_GET, buffer, (uint16_t)(p - buffer));
    wiced_bt_mesh_release_event(p_event);
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_config_netkey_change(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_netkey_change_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    if (p == NULL)
        return WICED_FALSE;

    *p++ = p_data->net_key_idx & 0xff;
    *p++ = (p_data->net_key_idx >> 8) & 0xff;

    if (p_data->operation == OPERATION_DELETE)
    {
        wiced_hci_send(HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_DELETE, buffer, (uint16_t)(p - buffer));
    }
    else
    {
        memcpy(p, p_data->net_key, 16);
        p += 16;
        wiced_hci_send(p_data->operation == OPERATION_ADD ? HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_ADD : HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_UPDATE, buffer, (uint16_t)(p - buffer));
    }
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_config_key_refresh_phase_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_key_refresh_phase_set_data_t *p_set)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    if (p == NULL)
    {
        wiced_bt_mesh_release_event(p_event);
        return WICED_FALSE;
    }
    *p++ = p_set->net_key_idx & 0xff;
    *p++ = (p_set->net_key_idx >> 8) & 0xff;
    *p++ = p_set->transition;

    Log("Key refresh addr:%04x net_key_idx:%x phase:%x", p_event->dst, p_set->net_key_idx, p_set->transition);
    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_CONFIG_KEY_REFRESH_PHASE_SET, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_config_appkey_change(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_appkey_change_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    if (p == NULL)
    {
        wiced_bt_mesh_release_event(p_event);
        return WICED_FALSE;
    }
    *p++ = p_data->net_key_idx & 0xff;
    *p++ = (p_data->net_key_idx >> 8) & 0xff;
    *p++ = p_data->app_key_idx & 0xff;
    *p++ = (p_data->app_key_idx >> 8) & 0xff;

    if (p_data->operation == OPERATION_DELETE)
    {
        Log("AppKey Delete addr:%04x net_key_idx:%x app_key_idx:%x", p_event->dst, p_data->net_key_idx, p_data->app_key_idx);
        wiced_hci_send(HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_DELETE, buffer, (uint16_t)(p - buffer));
    }
    else
    {
        memcpy(p, p_data->app_key, 16);
        p += 16;
        Log("AppKey %s addr:%04x net_key_idx:%x app_key_idx:%x", p_data->operation == OPERATION_ADD ? "Add" : "Update", p_event->dst, p_data->net_key_idx, p_data->app_key_idx);
        wiced_hci_send(p_data->operation == OPERATION_ADD ? HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_ADD : HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_UPDATE, buffer, (uint16_t)(p - buffer));
    }
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_config_model_app_bind(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_model_app_bind_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    if (p == NULL)
        return WICED_FALSE;

    *p++ = p_data->element_addr & 0xff;
    *p++ = (p_data->element_addr >> 8) & 0xff;
    *p++ = p_data->company_id & 0xff;
    *p++ = (p_data->company_id >> 8) & 0xff;
    *p++ = p_data->model_id & 0xff;
    *p++ = (p_data->model_id >> 8) & 0xff;
    *p++ = p_data->app_key_idx & 0xff;
    *p++ = (p_data->app_key_idx >> 8) & 0xff;

    wiced_hci_send(p_data->operation == OPERATION_BIND ? HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_APP_BIND : HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_APP_UNBIND, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_provision_local_device_set(wiced_bt_mesh_local_device_set_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = buffer;
    *p++ = p_data->addr & 0xff;
    *p++ = (p_data->addr >> 8) & 0xff;
    memcpy(p, p_data->dev_key, 16);
    p += 16;
    memcpy(p, p_data->network_key, 16);
    p += 16;
    *p++ = p_data->net_key_idx & 0xff;
    *p++ = (p_data->net_key_idx >> 8) & 0xff;
    *p++ = p_data->iv_idx & 0xff;
    *p++ = (p_data->iv_idx >> 8) & 0xff;
    *p++ = (p_data->iv_idx >> 16) & 0xff;
    *p++ = (p_data->iv_idx >> 24) & 0xff;
    *p++ = p_data->key_refresh;
    *p++ = p_data->iv_update;
    *p++ = p_data->model_level_access;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_SET_LOCAL_DEVICE, buffer, (uint16_t)(p - buffer));
}

wiced_bool_t wiced_bt_mesh_config_model_publication_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_model_publication_set_data_t *p_set)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    if (p == NULL)
        return WICED_FALSE;

    *p++ = p_set->element_addr & 0xff;
    *p++ = (p_set->element_addr >> 8) & 0xff;
    *p++ = p_set->company_id & 0xff;
    *p++ = (p_set->company_id >> 8) & 0xff;
    *p++ = p_set->model_id & 0xff;
    *p++ = (p_set->model_id >> 8) & 0xff;
    memcpy(p, p_set->publish_addr, 16);
    p += 16;
    *p++ = p_set->app_key_idx & 0xff;
    *p++ = (p_set->app_key_idx >> 8) & 0xff;
    *p++ = p_set->credential_flag;
    *p++ = p_set->publish_ttl;
    *p++ = p_set->publish_period & 0xff;
    *p++ = (p_set->publish_period >> 8) & 0xff;
    *p++ = (p_set->publish_period >> 16) & 0xff;
    *p++ = (p_set->publish_period >> 24) & 0xff;
    *p++ = p_set->publish_retransmit_count;
    *p++ = p_set->publish_retransmit_interval & 0xff;
    *p++ = (p_set->publish_retransmit_interval >> 8) & 0xff;

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_PUBLICATION_SET, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_config_model_subscription_change(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_model_subscription_change_data_t *p_data)
{
    uint16_t hci_opcode = 0;
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    if (p == NULL)
    {
        wiced_bt_mesh_release_event(p_event);
        return WICED_FALSE;
    }
    *p++ = p_data->element_addr & 0xff;
    *p++ = (p_data->element_addr >> 8) & 0xff;
    *p++ = p_data->company_id & 0xff;
    *p++ = (p_data->company_id >> 8) & 0xff;
    *p++ = p_data->model_id & 0xff;
    *p++ = (p_data->model_id >> 8) & 0xff;
    memcpy(p, p_data->addr, 16);
    switch (p_data->operation)
    {
    case OPERATION_ADD:
        Log("Model Sub Add addr:%04x elem:%x model:%x addr:%04x", p_event->dst, p_data->element_addr, p_data->model_id, p_data->addr[0] + (p_data->addr[1] << 8));
        hci_opcode = HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_ADD;
        p += 16;
        break;
    case OPERATION_OVERWRITE:
        Log("Model Sub Overwrite addr:%04x elem:%x model:%x addr:%04x", p_event->dst, p_data->element_addr, p_data->model_id, p_data->addr[0] + (p_data->addr[1] << 8));
        hci_opcode = HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_OVERWRITE;
        p += 16;
        break;
    case OPERATION_DELETE:
        Log("Model Sub Delete addr:%04x elem:%x model:%x addr:%04x", p_event->dst, p_data->element_addr, p_data->model_id, p_data->addr[0] + (p_data->addr[1] << 8));
        hci_opcode = HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_DELETE;
        p += 16;
        break;
    case OPERATION_DELETE_ALL:
        Log("Model Sub DeleteAll addr:%04x elem:%x model:%x", p_event->dst, p_data->element_addr, p_data->model_id);
        hci_opcode = HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_DELETE_ALL;
        break;
    }
    wiced_hci_send(hci_opcode, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_config_network_transmit_params_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_network_transmit_set_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    if (p == NULL)
        return WICED_FALSE;

    *p++ = p_data->count;
    *p++ = p_data->interval & 0xff;
    *p++ = (p_data->interval >> 8) & 0xff;

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_CONFIG_NETWORK_TRANSMIT_SET, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_config_default_ttl_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_default_ttl_set_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    if (p == NULL)
        return WICED_FALSE;

    *p++ = p_data->ttl;

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_CONFIG_DEFAULT_TTL_SET, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_config_relay_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_relay_set_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));


    if (p == NULL)
        return WICED_FALSE;

    *p++ = p_data->state;
    *p++ = p_data->retransmit_count;
    *p++ = p_data->retransmit_interval & 0xff;
    *p++ = (p_data->retransmit_interval >> 8) & 0xff;

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_CONFIG_RELAY_SET, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_config_friend_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_friend_set_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    if (p == NULL)
        return WICED_FALSE;

    *p++ = p_data->state;

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_CONFIG_FRIEND_SET, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_config_gatt_proxy_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_gatt_proxy_set_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    if (p == NULL)
        return WICED_FALSE;

    *p++ = p_data->state;

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_CONFIG_GATT_PROXY_SET, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_config_beacon_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_beacon_set_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    if (p == NULL)
        return WICED_FALSE;

    *p++ = p_data->state;

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_CONFIG_BEACON_SET, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_config_node_reset(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    if (p == NULL)
        return WICED_FALSE;

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_CONFIG_NODE_RESET, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_health_attention_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_health_attention_set_data_t *p_set)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_FALSE;

    *p++ = p_set->timer;

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_HEALTH_ATTENTION_SET, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_config_node_identity_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_config_node_identity_set_data_t *p_set)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_FALSE;

    *p++ = p_set->net_key_idx & 0xff;
    *p++ = (p_set->net_key_idx >> 8) & 0xff;
    *p++ = p_set->identity;

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_CONFIG_NODE_IDENTITY_SET, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_proxy_filter_change_addr(wiced_bt_mesh_event_t *p_event, wiced_bool_t is_add, wiced_bt_mesh_proxy_filter_change_addr_data_t *p_addr)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));
    int i;

    if ((p == NULL) || (p_addr->addr_num > sizeof(buffer) / 2))
        return WICED_FALSE;

    for (i = 0; i < p_addr->addr_num; i++)
    {
        *p++ = p_addr->addr[i] & 0xff;
        *p++ = (p_addr->addr[i] >> 8) & 0xff;
    }
    wiced_hci_send(is_add ? HCI_CONTROL_MESH_COMMAND_PROXY_FILTER_ADDRESSES_ADD : HCI_CONTROL_MESH_COMMAND_PROXY_FILTER_ADDRESSES_DELETE, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_provision_scan_capabilities_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_FALSE;

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_PROVISION_SCAN_CAPABILITIES_GET, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_provision_scan_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_FALSE;

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_PROVISION_SCAN_GET, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

/*
* Send Remote Provisioning Scan Start to Remote Provisioning Server
*/
wiced_bool_t wiced_bt_mesh_provision_scan_start(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_scan_start_data_t *p_data)
{
    uint8_t  buffer[35];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_FALSE;

    *p++ = p_data->scanned_items_limit;
    *p++ = p_data->timeout;
    if (p_data->scan_single_uuid)
    {
        memcpy(p, p_data->uuid, MESH_DEVICE_UUID_LEN);
        p += MESH_DEVICE_UUID_LEN;
    }
    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_PROVISION_SCAN_START, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

/*
* Send Remote Provisioning Scan Stop to Remote Provisioning Server
*/
wiced_bool_t wiced_bt_mesh_provision_scan_stop(wiced_bt_mesh_event_t *p_event)
{
    uint8_t  buffer[30];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_FALSE;

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_PROVISION_SCAN_STOP, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

/*
 * Send Remote Provisioning Scan Extended Scan Start to Remote Provisioning Server
 */
wiced_bool_t wiced_bt_mesh_provision_scan_extended_start(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_provision_scan_extended_start_t *p_data)
{
    int i;
    uint8_t  buffer[50];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_FALSE;

    UINT8_TO_STREAM(p, p_data->num_ad_filters);
    for (i = 0; i < p_data->num_ad_filters && i < WICED_BT_MESH_AD_FILTER_TYPES_MAX; i++)
    {
        UINT8_TO_STREAM(p, p_data->ad_filter_types[i]);
    }
    if (p_data->uuid_present)
    {
        memcpy(p, p_data->uuid, sizeof(p_data->uuid));
        p += sizeof(p_data->uuid);
        UINT8_TO_STREAM(p, p_data->timeout);
    }
    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_PROVISION_SCAN_EXTENDED_START, buffer, (uint16_t)(p - buffer));
    return WICED_TRUE;
}

#ifdef MESH_DFU_ENABLED
/*
 * The application can call this function to get the state of the current firmware distribution process.
 * The function may register a callback which will be executed when reply from the distributor is received.
 */
wiced_bool_t wiced_bt_mesh_fw_provider_get_status(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_fw_provider_callback_t *p_callback)
{
    return WICED_FALSE;
}

/*
 * The application can call this function to start firmware distribution procedure.
 */
wiced_bool_t wiced_bt_mesh_fw_provider_start(wiced_bt_mesh_event_t* p_event, wiced_bt_mesh_fw_distribution_start_data_t* p_data, wiced_bt_mesh_fw_provider_callback_t* p_callback)
{
    return WICED_FALSE;
}

/*
 * The application can call this function to terminate firmware distribution procedure.
 */
wiced_bool_t wiced_bt_mesh_fw_provider_stop(wiced_bt_mesh_event_t *p_event)
{
    return WICED_FALSE;
}

/*
 * Process finish event received from OTA client
 */
void wiced_bt_mesh_fw_provider_ota_finish(uint8_t status)
{
}
#endif

/*
 * Encrypt or decrypts and authenticates data
 */
uint16_t wiced_bt_mesh_core_crypt(wiced_bool_t encrypt, const uint8_t *p_in_data, uint16_t in_data_len, uint8_t *p_out_buf, uint16_t out_buf_len)
{
    return out_buf_len;
}


wiced_result_t wiced_bt_mesh_model_default_transition_time_client_send_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_default_transition_time_data_t* p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->time & 0xff;
    *p++ = (p_data->time >> 8) & 0xff;
    *p++ = (p_data->time >> 16) & 0xff;
    *p++ = (p_data->time >> 24) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_DEF_TRANS_TIME_SET, buffer, (uint16_t)(p - buffer)) ? WICED_BT_SUCCESS : WICED_BT_ERROR;
}

wiced_result_t wiced_bt_mesh_model_onoff_client_send_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_ONOFF_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_onoff_client_send_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_onoff_set_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->onoff;
    *p++ = p_data->transition_time & 0xff;
    *p++ = (p_data->transition_time >> 8) & 0xff;
    *p++ = (p_data->transition_time >> 16) & 0xff;
    *p++ = (p_data->transition_time >> 24) & 0xff;
    *p++ = p_data->delay & 0xff;
    *p++ = (p_data->delay >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_ONOFF_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_battery_client_send_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_BATTERY_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_property_client_send_properties_get(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_properties_get_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->type;
    *p++ = p_data->starting_id & 0xff;
    *p++ = (p_data->starting_id >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_PROPERTIES_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_property_client_send_property_get(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_property_get_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->type;
    *p++ = p_data->id & 0xff;
    *p++ = (p_data->id >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_PROPERTY_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_property_client_send_property_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_property_set_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->type;
    *p++ = p_data->id & 0xff;
    *p++ = (p_data->id >> 8) & 0xff;
    *p++ = p_data->access;
    *p++ = p_data->len & 0xff;
    *p++ = (p_data->len >> 8) & 0xff;

    memcpy(p, p_data->value, p_data->len);
    p += p_data->len;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_PROPERTY_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_lc_client_send_mode_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_LC_MODE_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_lc_client_send_mode_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_light_lc_mode_set_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->mode;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_LC_MODE_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_lc_client_send_occupancy_mode_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_LC_OCCUPANCY_MODE_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_lc_client_send_occupancy_mode_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_light_lc_occupancy_mode_set_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->mode;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_LC_OCCUPANCY_MODE_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_lc_client_send_light_onoff_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_LC_ONOFF_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_lc_client_send_light_onoff_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_light_lc_light_onoff_set_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->light_onoff;
    *p++ = p_data->transition_time & 0xff;
    *p++ = (p_data->transition_time >> 8) & 0xff;
    *p++ = (p_data->transition_time >> 16) & 0xff;
    *p++ = (p_data->transition_time >> 24) & 0xff;
    *p++ = p_data->delay & 0xff;
    *p++ = (p_data->delay >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_LC_ONOFF_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_lc_client_send_property_get(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_light_lc_property_get_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->id & 0xff;
    *p++ = (p_data->id >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_LC_PROPERTY_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_lc_client_send_property_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_light_lc_property_set_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->id & 0xff;
    *p++ = (p_data->id >> 8) & 0xff;
    *p++ = p_data->len & 0xff;
    *p++ = (p_data->len >> 8) & 0xff;
    memcpy(p, p_data->value, p_data->len);
    p += p_data->len;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_LC_PROPERTY_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_xyl_client_send_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_xyl_client_send_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_light_xyl_set_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->target.lightness & 0xff;
    *p++ = (p_data->target.lightness >> 8) & 0xff;
    *p++ = p_data->target.x & 0xff;
    *p++ = (p_data->target.x >> 8) & 0xff;
    *p++ = p_data->target.y & 0xff;
    *p++ = (p_data->target.y >> 8) & 0xff;
    *p++ = p_data->transition_time & 0xff;
    *p++ = (p_data->transition_time >> 8) & 0xff;
    *p++ = (p_data->transition_time >> 16) & 0xff;
    *p++ = (p_data->transition_time >> 24) & 0xff;
    *p++ = p_data->delay & 0xff;
    *p++ = (p_data->delay >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_xyl_client_send_target_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_TARGET_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_xyl_client_send_range_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_RANGE_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_xyl_client_send_range_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_light_xyl_range_set_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->x_min & 0xff;
    *p++ = (p_data->x_min >> 8) & 0xff;
    *p++ = p_data->x_max & 0xff;
    *p++ = (p_data->x_max >> 8) & 0xff;
    *p++ = p_data->y_min & 0xff;
    *p++ = (p_data->y_min >> 8) & 0xff;
    *p++ = p_data->y_max & 0xff;
    *p++ = (p_data->y_max >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_RANGE_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_xyl_client_send_default_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_DEFAULT_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_xyl_client_send_default_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_light_xyl_default_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->default_status.lightness & 0xff;
    *p++ = (p_data->default_status.lightness >> 8) & 0xff;
    *p++ = p_data->default_status.x & 0xff;
    *p++ = (p_data->default_status.x >> 8) & 0xff;
    *p++ = p_data->default_status.y & 0xff;
    *p++ = (p_data->default_status.y >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_DEFAULT_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_location_client_send_global_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LOCATION_GLOBAL_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_location_client_send_local_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LOCATION_LOCAL_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_location_client_send_global_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_location_global_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->global_latitude & 0xff;
    *p++ = (p_data->global_latitude >> 8) & 0xff;
    *p++ = (p_data->global_latitude >> 16) & 0xff;
    *p++ = (p_data->global_latitude >> 24) & 0xff;
    *p++ = p_data->global_longitude & 0xff;
    *p++ = (p_data->global_longitude >> 8) & 0xff;
    *p++ = (p_data->global_longitude >> 16) & 0xff;
    *p++ = (p_data->global_longitude >> 24) & 0xff;
    *p++ = p_data->global_altitude & 0xff;
    *p++ = (p_data->global_altitude >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LOCATION_GLOBAL_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_location_client_send_local_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_location_local_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->local_north & 0xff;
    *p++ = (p_data->local_north >> 8) & 0xff;
    *p++ = p_data->local_east & 0xff;
    *p++ = (p_data->local_east >> 8) & 0xff;
    *p++ = p_data->local_altitude & 0xff;
    *p++ = (p_data->local_altitude >> 8) & 0xff;
    *p++ = p_data->floor_number;
    *p++ = p_data->is_mobile;
    *p++ = p_data->update_time;
    *p++ = p_data->precision;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LOCATION_LOCAL_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_power_level_client_send_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_power_level_client_send_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_power_level_set_level_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->level & 0xff;
    *p++ = (p_data->level >> 8) & 0xff;
    *p++ = p_data->transition_time & 0xff;
    *p++ = (p_data->transition_time >> 8) & 0xff;
    *p++ = (p_data->transition_time >> 16) & 0xff;
    *p++ = (p_data->transition_time >> 24) & 0xff;
    *p++ = p_data->delay & 0xff;
    *p++ = (p_data->delay >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_power_level_client_send_last_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_LAST_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_power_level_client_send_default_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_DEFAULT_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_power_level_client_send_default_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_power_default_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->power & 0xff;
    *p++ = (p_data->power >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_DEFAULT_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_power_level_client_send_range_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_RANGE_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_power_level_client_send_range_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_power_level_range_set_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->power_min & 0xff;
    *p++ = (p_data->power_min >> 8) & 0xff;
    *p++ = p_data->power_max & 0xff;
    *p++ = (p_data->power_max >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_RANGE_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t  wiced_bt_mesh_model_power_onoff_client_send_onpowerup_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_ONPOWERUP_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_power_onoff_client_send_onpowerup_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_power_onoff_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->on_power_up;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_ONPOWERUP_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_scheduler_client_send_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_SCHEDULER_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_scheduler_client_send_action_get(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_scheduler_action_get_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->action_number & 0xff;
    *p++ = (p_data->action_number >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_SCHEDULER_ACTION_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_scheduler_client_send_action_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_scheduler_action_data_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->action_number;
    *p++ = p_data->year;
    *p++ = p_data->month & 0xff;
    *p++ = (p_data->month >> 8) & 0xff;
    *p++ = p_data->day;
    *p++ = p_data->hour;
    *p++ = p_data->minute;
    *p++ = p_data->second;
    *p++ = p_data->day_of_week;
    *p++ = p_data->action;
    *p++ = p_data->transition_time & 0xff;
    *p++ = (p_data->transition_time >> 8) & 0xff;
    *p++ = (p_data->transition_time >> 16) & 0xff;
    *p++ = (p_data->transition_time >> 24) & 0xff;
    *p++ = p_data->scene_number & 0xff;
    *p++ = (p_data->scene_number >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_SCHEDULER_ACTION_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_time_client_time_get_send(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_TIME_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_time_client_time_set_send(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_time_state_msg_t *p_data)
{
    uint8_t buffer[128];
    uint16_t auth_delta;
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    UINT40_TO_STREAM(p, p_data->tai_seconds);
    UINT8_TO_STREAM(p, p_data->subsecond);
    UINT8_TO_STREAM(p, p_data->uncertainty);
    auth_delta = p_data->tai_utc_delta_current << 1;
    auth_delta = auth_delta | (uint16_t)(p_data->time_authority ? 1 : 0 );
    UINT16_TO_STREAM(p, auth_delta);
    UINT8_TO_STREAM(p, p_data->time_zone_offset_current);

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_TIME_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_time_client_time_zone_get_send(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_TIME_ZONE_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_time_client_time_zone_set_send(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_time_zone_set_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    UINT8_TO_STREAM(p, p_data->time_zone_offset_new);
    UINT40_TO_STREAM(p, p_data->tai_of_zone_change);

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_TIME_ZONE_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_time_client_tai_utc_delta_get_send(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_TIME_TAI_UTC_DELTA_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_time_client_tai_utc_delta_set_send(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_time_tai_utc_delta_set_t *p_data)
{
    uint8_t buffer[128];
    uint16_t delta_new;

    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    delta_new = p_data->tai_utc_delta_new & (~(1 << 15));
    UINT16_TO_STREAM(p, delta_new);
    UINT40_TO_STREAM(p, p_data->tai_of_delta_change);

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_TIME_TAI_UTC_DELTA_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_time_client_time_role_get_send(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_TIME_ROLE_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_time_client_time_role_set_send(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_time_role_msg_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    UINT8_TO_STREAM(p, p_data->role);

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_TIME_ROLE_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_level_client_send_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LEVEL_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_level_client_send_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_level_set_level_t *p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->level & 0xff;
    *p++ = (p_data->level >> 8) & 0xff;
    *p++ = p_data->transition_time & 0xff;
    *p++ = (p_data->transition_time >> 8) & 0xff;
    *p++ = (p_data->transition_time >> 16) & 0xff;
    *p++ = (p_data->transition_time >> 24) & 0xff;
    *p++ = p_data->delay & 0xff;
    *p++ = (p_data->delay >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LEVEL_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_level_client_send_delta_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_level_set_delta_t* p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->delta & 0xff;
    *p++ = (p_data->delta >> 8) & 0xff;
    *p++ = (p_data->delta >> 16) & 0xff;
    *p++ = (p_data->delta >> 24) & 0xff;
    *p++ = p_data->continuation;
    *p++ = p_data->transition_time & 0xff;
    *p++ = (p_data->transition_time >> 8) & 0xff;
    *p++ = (p_data->transition_time >> 16) & 0xff;
    *p++ = (p_data->transition_time >> 24) & 0xff;
    *p++ = p_data->delay & 0xff;
    *p++ = (p_data->delay >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LEVEL_DELTA_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_level_client_send_move_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_level_set_move_t* p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->delta & 0xff;
    *p++ = (p_data->delta >> 8) & 0xff;
    *p++ = p_data->continuation;
    *p++ = p_data->transition_time & 0xff;
    *p++ = (p_data->transition_time >> 8) & 0xff;
    *p++ = (p_data->transition_time >> 16) & 0xff;
    *p++ = (p_data->transition_time >> 24) & 0xff;
    *p++ = p_data->delay & 0xff;
    *p++ = (p_data->delay >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LEVEL_MOVE_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_lightness_client_send_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_lightness_client_send_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_light_lightness_actual_set_t* p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->lightness_actual & 0xff;
    *p++ = (p_data->lightness_actual >> 8) & 0xff;
    *p++ = p_data->transition_time & 0xff;
    *p++ = (p_data->transition_time >> 8) & 0xff;
    *p++ = (p_data->transition_time >> 16) & 0xff;
    *p++ = (p_data->transition_time >> 24) & 0xff;
    *p++ = p_data->delay & 0xff;
    *p++ = (p_data->delay >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_hsl_client_send_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_hsl_client_send_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_light_hsl_set_t* p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->target.lightness & 0xff;
    *p++ = (p_data->target.lightness >> 8) & 0xff;
    *p++ = p_data->target.hue & 0xff;
    *p++ = (p_data->target.hue >> 8) & 0xff;
    *p++ = p_data->target.saturation & 0xff;
    *p++ = (p_data->target.saturation >> 8) & 0xff;
    *p++ = p_data->transition_time & 0xff;
    *p++ = (p_data->transition_time >> 8) & 0xff;
    *p++ = (p_data->transition_time >> 16) & 0xff;
    *p++ = (p_data->transition_time >> 24) & 0xff;
    *p++ = p_data->delay & 0xff;
    *p++ = (p_data->delay >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_hsl_client_send_hue_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_HUE_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_hsl_client_send_hue_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_light_hsl_hue_set_t* p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->level & 0xff;
    *p++ = (p_data->level >> 8) & 0xff;
    *p++ = p_data->transition_time & 0xff;
    *p++ = (p_data->transition_time >> 8) & 0xff;
    *p++ = (p_data->transition_time >> 16) & 0xff;
    *p++ = (p_data->transition_time >> 24) & 0xff;
    *p++ = p_data->delay & 0xff;
    *p++ = (p_data->delay >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_HUE_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_hsl_client_send_saturation_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_SATURATION_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_hsl_client_send_saturation_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_light_hsl_saturation_set_t* p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->level & 0xff;
    *p++ = (p_data->level >> 8) & 0xff;
    *p++ = p_data->transition_time & 0xff;
    *p++ = (p_data->transition_time >> 8) & 0xff;
    *p++ = (p_data->transition_time >> 16) & 0xff;
    *p++ = (p_data->transition_time >> 24) & 0xff;
    *p++ = p_data->delay & 0xff;
    *p++ = (p_data->delay >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_SATURATION_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_hsl_client_send_target_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_TARGET_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_hsl_client_send_default_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_DEFAULT_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_hsl_client_send_default_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_light_hsl_default_data_t* p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->default_status.lightness & 0xff;
    *p++ = (p_data->default_status.lightness >> 8) & 0xff;
    *p++ = p_data->default_status.hue & 0xff;
    *p++ = (p_data->default_status.hue >> 8) & 0xff;
    *p++ = p_data->default_status.saturation & 0xff;
    *p++ = (p_data->default_status.saturation >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_DEFAULT_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_hsl_client_send_range_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_RANGE_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_hsl_client_send_range_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_light_hsl_range_set_data_t* p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->hue_min & 0xff;
    *p++ = (p_data->hue_min >> 8) & 0xff;
    *p++ = p_data->hue_max & 0xff;
    *p++ = (p_data->hue_max >> 8) & 0xff;
    *p++ = p_data->saturation_min & 0xff;
    *p++ = (p_data->saturation_min >> 8) & 0xff;
    *p++ = p_data->saturation_max & 0xff;
    *p++ = (p_data->saturation_min >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_RANGE_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_ctl_client_send_get(wiced_bt_mesh_event_t *p_event)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_CTL_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_light_ctl_client_send_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_light_ctl_set_t* p_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = p_data->target.lightness & 0xff;
    *p++ = (p_data->target.lightness >> 8) & 0xff;
    *p++ = p_data->target.temperature & 0xff;
    *p++ = (p_data->target.temperature >> 8) & 0xff;
    *p++ = p_data->target.delta_uv & 0xff;
    *p++ = (p_data->target.delta_uv >> 8) & 0xff;
    *p++ = p_data->transition_time & 0xff;
    *p++ = (p_data->transition_time >> 8) & 0xff;
    *p++ = (p_data->transition_time >> 16) & 0xff;
    *p++ = (p_data->transition_time >> 24) & 0xff;
    *p++ = p_data->delay & 0xff;
    *p++ = (p_data->delay >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_LIGHT_CTL_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_sensor_client_descriptor_send_get(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_sensor_get_t *desc_get_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    if (desc_get_data->property_id != 0)
    {
        *p++ = desc_get_data->property_id & 0xff;
        *p++ = (desc_get_data->property_id >> 8) & 0xff;
    }
    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_SENSOR_DESCRIPTOR_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_sensor_client_sensor_send_get(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_sensor_get_t *sensor_get)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    if ((sensor_get != NULL) && (sensor_get->property_id != 0))
    {
        *p++ = sensor_get->property_id & 0xff;
        *p++ = (sensor_get->property_id >> 8) & 0xff;
    }
    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_SENSOR_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_sensor_client_sensor_column_send_get(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_sensor_column_get_data_t *column_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = column_data->property_id & 0xff;
    *p++ = (column_data->property_id >> 8) & 0xff;
    memcpy(p, column_data->raw_valuex, column_data->prop_value_len);
    p += column_data->prop_value_len;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_SENSOR_COLUMN_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_sensor_client_sensor_series_send_get(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_sensor_series_get_data_t *series_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));
    uint8_t len = series_data->prop_value_len;

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = series_data->property_id & 0xff;
    *p++ = (series_data->property_id >> 8) & 0xff;

    if ((series_data->start_index != 0) || (series_data->end_index != 0xFF))
    {
        memcpy(p, series_data->raw_valuex1, len);
        p += len;
        memcpy(p, series_data->raw_valuex2, len);
        p += len;
    }
    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_SENSOR_SERIES_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_sensor_client_sensor_setting_send_get(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_sensor_setting_get_data_t *setting_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = setting_data->property_id & 0xff;
    *p++ = (setting_data->property_id >> 8) & 0xff;
    *p++ = setting_data->setting_property_id & 0xff;
    *p++ = (setting_data->setting_property_id >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_SENSOR_SETTING_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_sensor_client_sensor_setting_send_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_sensor_setting_set_data_t *setting_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = setting_data->property_id & 0xff;
    *p++ = (setting_data->property_id >> 8) & 0xff;
    *p++ = setting_data->setting_property_id & 0xff;
    *p++ = (setting_data->setting_property_id >> 8) & 0xff;
    memcpy(p, setting_data->setting_raw_val, setting_data->prop_value_len);
    p += setting_data->prop_value_len;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_SENSOR_SETTING_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_sensor_client_sensor_settings_send_get(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_sensor_get_t *settings_data)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = settings_data->property_id & 0xff;
    *p++ = (settings_data->property_id >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_SENSOR_SETTINGS_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_sensor_client_sensor_cadence_send_get(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_sensor_get_t *cadence_get)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = cadence_get->property_id & 0xff;
    *p++ = (cadence_get->property_id >> 8) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_SENSOR_CADENCE_GET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_model_sensor_client_sensor_cadence_send_set(wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_sensor_cadence_set_data_t *cadence_set)
{
    uint8_t buffer[128];
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    wiced_bt_mesh_release_event(p_event);

    if (p == NULL)
        return WICED_BT_BADARG;

    *p++ = cadence_set->property_id & 0xff;
    *p++ = (cadence_set->property_id >> 8) & 0xff;
    *p++ = cadence_set->prop_value_len & 0xff;
    *p++ = cadence_set->cadence_data.fast_cadence_period_divisor & 0xff;
    *p++ = (cadence_set->cadence_data.fast_cadence_period_divisor >> 8) & 0xff;
    *p++ = cadence_set->cadence_data.trigger_type;

    //fill status trigger delta up and down values
    *p++ = cadence_set->cadence_data.trigger_delta_down & 0xff;
    *p++ = (cadence_set->cadence_data.trigger_delta_down >> 8) & 0xff;
    *p++ = (cadence_set->cadence_data.trigger_delta_down >> 16) & 0xff;
    *p++ = (cadence_set->cadence_data.trigger_delta_down >> 24) & 0xff;

    *p++ = cadence_set->cadence_data.trigger_delta_up & 0xff;
    *p++ = (cadence_set->cadence_data.trigger_delta_up >> 8) & 0xff;
    *p++ = (cadence_set->cadence_data.trigger_delta_up >> 16) & 0xff;
    *p++ = (cadence_set->cadence_data.trigger_delta_up >> 24) & 0xff;

    *p++ = cadence_set->cadence_data.min_interval & 0xff;
    *p++ = (cadence_set->cadence_data.min_interval >> 8) & 0xff;
    *p++ = (cadence_set->cadence_data.min_interval >> 16) & 0xff;
    *p++ = (cadence_set->cadence_data.min_interval >> 24) & 0xff;

    //fill status fast cadence high and low value
    *p++ = cadence_set->cadence_data.fast_cadence_low & 0xff;
    *p++ = (cadence_set->cadence_data.fast_cadence_low >> 8) & 0xff;
    *p++ = (cadence_set->cadence_data.fast_cadence_low >> 16) & 0xff;
    *p++ = (cadence_set->cadence_data.fast_cadence_low >> 24) & 0xff;

    *p++ = cadence_set->cadence_data.fast_cadence_high & 0xff;
    *p++ = (cadence_set->cadence_data.fast_cadence_high >> 8) & 0xff;
    *p++ = (cadence_set->cadence_data.fast_cadence_high >> 16) & 0xff;
    *p++ = (cadence_set->cadence_data.fast_cadence_high >> 24) & 0xff;

    return wiced_hci_send(HCI_CONTROL_MESH_COMMAND_SENSOR_CADENCE_SET, buffer, (uint16_t)(p - buffer));
}

wiced_result_t wiced_bt_mesh_client_vendor_data(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint16_t data_len)
{
    uint8_t buffer[400];
    wiced_result_t res;
    uint8_t *p = wiced_bt_mesh_hci_header_from_event(p_event, buffer, sizeof(buffer));

    if (p == NULL)
    {
        wiced_bt_mesh_release_event(p_event);
        return WICED_BT_BADARG;
    }
    if (data_len > sizeof(buffer) - (uint16_t)(p - buffer))
    {
        data_len = sizeof(buffer) - (uint16_t)(p - buffer);
    }
    *p++ = p_event->company_id & 0xff;
    *p++ = (p_event->company_id >> 8) & 0xff;
    *p++ = p_event->model_id & 0xff;
    *p++ = (p_event->model_id >> 8) & 0xff;
    *p++ = p_event->opcode & 0xff;

    memcpy(p, p_data, data_len);
    p += data_len;

    res = wiced_hci_send(HCI_CONTROL_MESH_COMMAND_VENDOR_DATA, buffer, (uint16_t)(p - buffer));
    wiced_bt_mesh_release_event(p_event);
    return res;
}

wiced_bool_t wiced_bt_mesh_core_set_seq(uint16_t addr, uint32_t seq, wiced_bool_t prev_iv_idx)
{
    uint8_t buffer[128];
    uint8_t *p = buffer;

    // Same as mesh_client_seq_t
    *p++ = (uint8_t)prev_iv_idx;
    *p++ = (uint8_t)seq;
    *p++ = (uint8_t)(seq >> 8);
    *p++ = (uint8_t)(seq >> 16);
    *p++ = addr & 0xff;
    *p++ = (addr >> 8) & 0xff;

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_CORE_SET_SEQ, buffer, (uint16_t)(p - buffer));
    return 1;
}

wiced_bool_t wiced_bt_mesh_core_del_seq(uint16_t addr)
{
    uint8_t buffer[128];
    uint8_t *p = buffer;
    *p++ = addr & 0xff;
    *p++ = (addr >> 8) & 0xff;

    wiced_hci_send(HCI_CONTROL_MESH_COMMAND_CORE_DEL_SEQ, buffer, (uint16_t)(p - buffer));
    return 1;
}

uint8_t *wiced_bt_mesh_format_hci_header(uint16_t dst, uint16_t app_key_idx, uint8_t element_idx, uint8_t reliable, uint8_t send_segmented, uint8_t ttl, uint8_t retransmit_count, uint8_t retransmit_interval, uint8_t reply_timeout, uint8_t *p_buffer, uint16_t len)
{
    uint8_t *p = p_buffer;

    if (len < 11)
        return NULL;

    *p++ = dst & 0xff;
    *p++ = (dst >> 8) & 0xff;
    *p++ = app_key_idx & 0xff;
    *p++ = (app_key_idx >> 8) & 0xff;
    *p++ = element_idx;
    *p++ = reliable;
    *p++ = send_segmented;
    *p++ = ttl;
    *p++ = retransmit_count;
    *p++ = retransmit_interval;
    *p++ = reply_timeout;
    return p;
}

uint8_t *wiced_bt_mesh_hci_header_from_event(wiced_bt_mesh_event_t *p_event, uint8_t *p_buffer, uint16_t len)
{
    return wiced_bt_mesh_format_hci_header(p_event->dst, p_event->app_key_idx, p_event->element_idx, p_event->reply, p_event->send_segmented, p_event->ttl, p_event->retrans_cnt, p_event->retrans_time, p_event->reply_timeout, p_buffer, len);
}

wiced_bt_mesh_event_t *wiced_bt_mesh_event_from_hci_header(uint8_t **p_buffer, uint16_t *len)
{
    uint8_t *p = *p_buffer;
    wiced_bt_mesh_event_t *p_event = (wiced_bt_mesh_event_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_event_t));
    if (p_event != NULL)
    {
        memset(p_event, 0, sizeof(wiced_bt_mesh_event_t));
        p_event->src         = p[0] + ((uint16_t)p[1] << 8);
        p_event->dst         = p[2] + ((uint16_t)p[3] << 8);
        p_event->app_key_idx = p[4] + ((uint16_t)p[5] << 8);
        p_event->element_idx = p[6];
        p_event->rssi        = p[7];
        p_event->ttl         = p[8];
        p_event->company_id  = p[9] + ((uint16_t)p[10] << 8);
        p_event->opcode      = p[11] + ((uint16_t)p[12] << 8);
        *p_buffer = &p[13];
        *len -= 13;
    }
    return p_event;
}


/*
* Create mesh event for unsolicited message
*/
wiced_bt_mesh_event_t *wiced_bt_mesh_create_event(uint8_t element_idx, uint16_t company_id, uint16_t model_id, uint16_t dst, uint16_t app_key_idx)
{
    wiced_bt_mesh_event_t *p_event = (wiced_bt_mesh_event_t*)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_event_t));
    if (p_event == NULL)
    {
        Log("create_unsolicited_event: wiced_bt_get_buffer failed\n");
        return NULL;
    }
    memset(p_event, 0, sizeof(wiced_bt_mesh_event_t));

    p_event->opcode = WICED_BT_MESH_OPCODE_UNKNOWN;
    p_event->element_idx = element_idx;
    p_event->company_id = company_id;
    p_event->model_id = model_id;
    // if it is special case with 0xffff company_id then just create mesh event with default ttl and 0x41 retransmit
    if (company_id == MESH_COMPANY_ID_UNUSED)
    {
        p_event->ttl = USE_CONFIGURED_DEFAULT_TTL;
        p_event->retrans_cnt = DEFAULT_RETRANSMISSION;
    }
    else if (dst != MESH_NODE_ID_INVALID)
    {
#if 0
        else if (!foundation_find_appkey_(app_key_idx, &p_event->app_key_idx, NULL, NULL))
        {
            Log("create_unsolicited_event: no app_key_idx:%x\n", app_key_idx);
            wiced_bt_free_buffer(p_event);
            return NULL;
        }
#endif
        p_event->dst = dst;
        p_event->ttl = USE_CONFIGURED_DEFAULT_TTL;
        p_event->credential_flag = 0;
        p_event->retrans_time = 0;
        p_event->retrans_cnt = 0;
    }
    else
    {
#if 0
        if (wiced_bt_mesh_core_get_publication(p_event))
#endif
        {
            wiced_bt_free_buffer(p_event);
            return NULL;
        }
    }
    p_event->src = dst;
    p_event->app_key_idx = app_key_idx;
    p_event->reply = 0;
    return p_event;
}

/*
* Release mesh event
*/
void wiced_bt_mesh_release_event(wiced_bt_mesh_event_t *p_event)
{
    wiced_bt_free_buffer(p_event);
}

wiced_bt_mesh_event_t *mesh_configure_create_event(uint16_t dst, wiced_bool_t retransmit)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_create_event(0, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT, dst, 0xFFFF);
    if (p_event != NULL)
    {
        // retransmit only if sending not to local device and not to GATT proxy
        if (retransmit)
        {
            p_event->retrans_cnt = 4;       // Try 5 times (this is in addition to network layer retransmit)
            p_event->retrans_time = 20;     // Every 1 sec
            p_event->reply_timeout = 100;   // wait for the reply 5 seconds
        }
    }
    return p_event;
}

void *wiced_bt_get_buffer(uint16_t size)
{
    return malloc(size);
}

void wiced_bt_free_buffer(void *buf)
{
    free(buf);
}

wiced_bool_t wiced_bt_mesh_remote_provisioning_scan_rsp(wiced_bt_ble_scan_results_t* p_adv_report, uint8_t* p_adv_data) { return WICED_TRUE; }
wiced_bool_t wiced_bt_mesh_remote_provisioning_connectable_adv_packet(wiced_bt_ble_scan_results_t* p_adv_report, uint8_t* p_adv_data) { return WICED_TRUE; }
wiced_bool_t wiced_bt_mesh_remote_provisioning_nonconnectable_adv_packet(wiced_bt_ble_scan_results_t* p_adv_report, uint8_t* p_adv_data) { return WICED_TRUE; }
void wiced_bt_mesh_gatt_client_process_connectable_adv(uint8_t *remote_bd_addr, uint8_t ble_addr_type, int8_t rssi, uint8_t *p_adv_data) {}
void wiced_bt_ble_set_scan_mode(uint8_t is_active) {}
wiced_result_t wiced_bt_mesh_core_init(wiced_bt_mesh_core_init_t *p_init) { return WICED_SUCCESS; }
void mesh_application_init(void) {}
void mesh_application_deinit(void) {}
void wiced_bt_mesh_config_client_connection_state_change(wiced_bool_t is_connected) {}
void wiced_bt_mesh_core_connection_status(uint32_t conn_id, wiced_bool_t connected_to_proxy, uint32_t ref_data, uint16_t mtu) {}
