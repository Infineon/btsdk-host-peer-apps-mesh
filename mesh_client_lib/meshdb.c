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
* meshdb.cpp : implementation of routines for database operations
*
*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "wiced_memory.h"
#include "meshdb.h"
#include "wiced_bt_mesh_db.h"

#define MAX_TAG_NAME                                32
#define MAX_SCHEMA_LEN                              100

#define MESH_JSON_TAG_SCHEMA                        0x0001
#define MESH_JSON_TAG_MESH_UUID                     0x0002
#define MESH_JSON_TAG_NAME                          0x0004
#define MESH_JSON_TAG_TIMESTAMP                     0x0008
#define MESH_JSON_TAG_PROVISIONERS                  0x0010
#define MESH_JSON_TAG_NET_KEYS                      0x0020
#define MESH_JSON_TAG_APP_KEYS                      0x0040
#define MESH_JSON_TAG_NODES                         0x0080
#define MESH_JSON_TAG_GROUPS                        0x0100
#define MESH_JSON_TAG_SCENES                        0x0200
#define MESH_JSON_TAG_MANDATORY (MESH_JSON_TAG_MESH_UUID | MESH_JSON_TAG_NAME | MESH_JSON_TAG_PROVISIONERS | MESH_JSON_TAG_NET_KEYS | MESH_JSON_TAG_NODES)

#define MESH_JSON_KEY_TAG_NAME                      0x0001
#define MESH_JSON_KEY_TAG_KEY_INDEX                 0x0002
#define MESH_JSON_KEY_TAG_PHASE                     0x0004
#define MESH_JSON_KEY_TAG_KEY                       0x0008
#define MESH_JSON_KEY_TAG_MIN_SECURITY              0x0010
#define MESH_JSON_KEY_TAG_OLD_KEY                   0x0020
#define MESH_JSON_KEY_TAG_BOUND_NET_KEY             0x0040
#define MESH_JSON_TAG_NET_KEY_MANDATORY (MESH_JSON_KEY_TAG_NAME | MESH_JSON_KEY_TAG_KEY_INDEX | MESH_JSON_KEY_TAG_PHASE | MESH_JSON_KEY_TAG_KEY | MESH_JSON_KEY_TAG_MIN_SECURITY)
#define MESH_JSON_TAG_APP_KEY_MANDATORY (MESH_JSON_KEY_TAG_NAME | MESH_JSON_KEY_TAG_KEY_INDEX | MESH_JSON_KEY_TAG_BOUND_NET_KEY | MESH_JSON_KEY_TAG_KEY)

#define MESH_JSON_PROVISIONER_TAG_NAME              0x0001
#define MESH_JSON_PROVISIONER_TAG_UUID              0x0002
#define MESH_JSON_PROVISIONER_TAG_GROUP_RANGE       0x0004
#define MESH_JSON_PROVISIONER_TAG_UNICAST_RANGE     0x0008

#define MESH_JSON_TAG_PROVISIONER_MANDATORY (MESH_JSON_PROVISIONER_TAG_NAME | MESH_JSON_PROVISIONER_TAG_UUID | MESH_JSON_PROVISIONER_TAG_GROUP_RANGE | MESH_JSON_PROVISIONER_TAG_UNICAST_RANGE)

// Mandatory Tags of the Nodes object
#define MESH_JSON_NODE_TAG_UUID                     0x0001
#define MESH_JSON_NODE_TAG_UNICAST_ADDR             0x0002
#define MESH_JSON_NODE_TAG_DEVICE_KEY               0x0004
#define MESH_JSON_NODE_TAG_SECURITY                 0x0008
#define MESH_JSON_NODE_TAG_NET_KEYS                 0x0010
#define MESH_JSON_NODE_TAG_CONFIG_COMPLETE          0x0020

// Otional Tags of the Nodes object
#define MESH_JSON_NODE_TAG_NAME                     0x0000
#define MESH_JSON_NODE_TAG_CID                      0x0000
#define MESH_JSON_NODE_TAG_PID                      0x0000
#define MESH_JSON_NODE_TAG_VID                      0x0000
#define MESH_JSON_NODE_TAG_CRPL                     0x0000
#define MESH_JSON_NODE_TAG_FEATURES                 0x0000
#define MESH_JSON_NODE_TAG_BEACON                   0x0000
#define MESH_JSON_NODE_TAG_DEFAULT_TTL              0x0000
#define MESH_JSON_NODE_TAG_NET_TRANSMIT             0x0000
#define MESH_JSON_NODE_TAG_RELAY_REXMIT             0x0000
#define MESH_JSON_NODE_TAG_APP_KEYS                 0x0000
#define MESH_JSON_NODE_TAG_ELEMENTS                 0x0000
#define MESH_JSON_NODE_TAG_BLACKLISTED              0x0000
#define MESH_JSON_NODE_TAG_KEY_COMPOSITION          0x0000

#define MESH_JSON_TAG_NODE_MANDATORY (MESH_JSON_NODE_TAG_UUID | MESH_JSON_NODE_TAG_UNICAST_ADDR | MESH_JSON_NODE_TAG_DEVICE_KEY | MESH_JSON_NODE_TAG_SECURITY | MESH_JSON_NODE_TAG_NET_KEYS | MESH_JSON_NODE_TAG_CONFIG_COMPLETE)

#define MESH_JSON_ELEMENT_TAG_NAME                  0x0001
#define MESH_JSON_ELEMENT_TAG_INDEX                 0x0002
#define MESH_JSON_ELEMENT_TAG_LOCATION              0x0004
#define MESH_JSON_ELEMENT_TAG_MODELS                0x0008

#define MESH_JSON_TAG_ELEMENT_MANDATORY (MESH_JSON_ELEMENT_TAG_INDEX | MESH_JSON_ELEMENT_TAG_LOCATION | MESH_JSON_ELEMENT_TAG_MODELS)

#define MESH_JSON_MODEL_TAG_MODEL_ID                0x0001
#define MESH_JSON_MODEL_TAG_SUBSCRIBE               0x0002
#define MESH_JSON_MODEL_TAG_PUBLISH                 0x0004
#define MESH_JSON_MODEL_TAG_BIND                    0x0008
#define MESH_JSON_MODEL_TAG_SENSOR                  0x0010

#define MESH_JSON_TAG_MODEL_MANDATORY (MESH_JSON_MODEL_TAG_MODEL_ID)

#define MESH_JSON_CONFIG_KEY_TAG_KEY                0x0001

#define MESH_JSON_CONFIG_NET_XMIT_TAG_COUNT         0x0001
#define MESH_JSON_CONFIG_NET_XMIT_TAG_INTERVAL      0x0002

#define MESH_JSON_PUBLISH_TAG_ADDRESS               0x0001
#define MESH_JSON_PUBLISH_TAG_INDEX                 0x0002
#define MESH_JSON_PUBLISH_TAG_TTL                   0x0004
#define MESH_JSON_PUBLISH_TAG_PERIOD                0x0008
#define MESH_JSON_PUBLISH_TAG_REXMIT                0x0010
#define MESH_JSON_PUBLISH_TAG_CREDENTIALS           0x0020

#define MESH_JSON_TAG_PUBLISH_MANDATORY (MESH_JSON_PUBLISH_TAG_ADDRESS | MESH_JSON_PUBLISH_TAG_INDEX | MESH_JSON_PUBLISH_TAG_TTL | MESH_JSON_PUBLISH_TAG_PERIOD | MESH_JSON_PUBLISH_TAG_REXMIT | MESH_JSON_PUBLISH_TAG_CREDENTIALS)

#define MESH_JSON_PUBLISH_TAG_REXMIT_COUNT          0x0001
#define MESH_JSON_PUBLISH_TAG_REXMIT_INTERVAL       0x0002

#define MESH_JSON_SENSOR_TAG_DESCRIPTOR             0x0001
#define MESH_JSON_SENSOR_TAG_CADENCE                0x0000
#define MESH_JSON_SENSOR_TAG_SETTING                0x0000
#define MESH_JSON_SENSOR_TAG_PROP_ID                0x0002
#define MESH_JSON_SENSOR_TAG_PROP_VAL_LEN           0x0004

#define MESH_JSON_TAG_SENSOR_MANDATORY (MESH_JSON_SENSOR_TAG_PROP_ID | MESH_JSON_SENSOR_TAG_PROP_VAL_LEN | MESH_JSON_SENSOR_TAG_DESCRIPTOR)

#define MESH_JSON_SENSOR_TAG_DESC_POS_TOLERANCE             0x0001
#define MESH_JSON_SENSOR_TAG_DESC_NEG_TOLERANCE             0x0002
#define MESH_JSON_SENSOR_TAG_DESC_SAMPLING_FUNC             0x0004
#define MESH_JSON_SENSOR_TAG_DESC_MEASUREMENT_PERIOD        0x0008
#define MESH_JSON_SENSOR_TAG_DESC_UPDATE_INTERVAL           0x0010

#define MESH_JSON_TAG_SENSOR_DEC_MANDATORY (MESH_JSON_SENSOR_TAG_DESC_POS_TOLERANCE | MESH_JSON_SENSOR_TAG_DESC_NEG_TOLERANCE | MESH_JSON_SENSOR_TAG_DESC_SAMPLING_FUNC | MESH_JSON_SENSOR_TAG_DESC_MEASUREMENT_PERIOD | MESH_JSON_SENSOR_TAG_DESC_UPDATE_INTERVAL)

#define MESH_JSON_SENSOR_TAG_CADENCE_FAST_CAD_PERIOD_DEV     0x0001
#define MESH_JSON_SENSOR_TAG_CADENCE_TRIGGER_TYPE            0x0002
#define MESH_JSON_SENSOR_TAG_CADENCE_DELTA_DOWN              0x0004
#define MESH_JSON_SENSOR_TAG_CADENCE_DELTA_UP                0x0008
#define MESH_JSON_SENSOR_TAG_CADENCE_MIN_INTERVAL            0x0010
#define MESH_JSON_SENSOR_TAG_CADENCE_FST_CAD_LOW             0x0020
#define MESH_JSON_SENSOR_TAG_CADENCE_FST_CAD_HIGH            0x0040

#define MESH_JSON_TAG_SENSOR_CAD_MANDATORY (MESH_JSON_SENSOR_TAG_CADENCE_FAST_CAD_PERIOD_DEV | MESH_JSON_SENSOR_TAG_CADENCE_TRIGGER_TYPE | MESH_JSON_SENSOR_TAG_CADENCE_DELTA_DOWN | MESH_JSON_SENSOR_TAG_CADENCE_DELTA_UP | MESH_JSON_SENSOR_TAG_CADENCE_MIN_INTERVAL | MESH_JSON_SENSOR_TAG_CADENCE_FST_CAD_LOW | MESH_JSON_SENSOR_TAG_CADENCE_FST_CAD_HIGH)

#define MESH_JSON_SENSOR_TAG_SETTING_PROP_ID                 0x0001
#define MESH_JSON_SENSOR_TAG_SETTING_ACCESS                  0x0002
#define MESH_JSON_SENSOR_TAG_SETTING_VAL                     0x0004

#define MESH_JSON_TAG_SENSOR_SETTING_MANDATORY (MESH_JSON_SENSOR_TAG_SETTING_PROP_ID | MESH_JSON_SENSOR_TAG_SETTING_ACCESS)

#define MESH_JSON_GROUP_TAG_NAME                    0x0001
#define MESH_JSON_GROUP_TAG_ADDR                    0x0002
#define MESH_JSON_GROUP_TAG_PARENT                  0x0004

#define MESH_JSON_TAG_GROUP_MANDATORY (MESH_JSON_GROUP_TAG_NAME | MESH_JSON_GROUP_TAG_ADDR)

#define MESH_JSON_SCENE_TAG_NAME                    0x0001
#define MESH_JSON_SCENE_TAG_ADDR                    0x0002
#define MESH_JSON_SCENE_TAG_NUMBER                  0x0004

#define RANGE_TYPE_GROUP                            0
#define RANGE_TYPE_UNICAST                          1
#define RANGE_TYPE_SCENE                            2

#define FOUNDATION_FEATURE_BIT_RELAY                0x0001
#define FOUNDATION_FEATURE_BIT_PROXY                0x0002
#define FOUNDATION_FEATURE_BIT_FRIEND               0x0004
#define FOUNDATION_FEATURE_BIT_LOW_POWER            0x0008

#define wiced_bt_get_buffer malloc
#define wiced_bt_free_buffer free

wiced_bt_mesh_db_mesh_t *mesh_json_read_file(FILE *fp);
void mesh_json_write_file(FILE *fp, wiced_bt_mesh_db_mesh_t *p_mesh);
int mesh_json_read_tag_name(FILE *fp, char *tagname, int len);
int mesh_json_read_string(FILE *fp, char *p_string, int len);
int mesh_json_read_schema(FILE *fp, char prefix);
int mesh_json_read_hex128(FILE *fp, char prefix, uint8_t *uuid);
int mesh_json_read_hex_array(FILE *fp, char prefix, uint8_t *value);
int mesh_json_read_uint8(FILE *fp, char prefix, uint8_t *value);
int mesh_json_read_uint16(FILE *fp, char prefix, uint16_t *value);
int mesh_json_read_uint32(FILE *fp, char prefix, uint32_t *value);
int mesh_json_read_boolean(FILE *fp, char prefix, uint8_t *value);
int mesh_json_read_hex16(FILE *fp, char prefix, uint16_t *value);
int mesh_json_read_hex32(FILE *fp, char prefix, uint32_t *value);
int mesh_json_read_sub_addr(FILE *fp, char prefix, uint16_t *value);
int mesh_json_read_name(FILE *fp, char prefix, int max_len, char **p_name);
int mesh_json_read_security(FILE *fp, char prefix, uint8_t *min_security);
int mesh_json_read_net_keys(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh);
int mesh_json_read_app_keys(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh);
int mesh_json_read_provisioners(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh);
int mesh_json_read_addr_range(FILE *fp, char c1, wiced_bt_mesh_db_provisioner_t *provisioner, int is_group);
int mesh_json_read_nodes(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh);
int mesh_json_read_groups(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh);
int mesh_json_read_scenes(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh);
int mesh_json_read_elements(FILE *fp, char prefix, wiced_bt_mesh_db_node_t *node);
int mesh_json_read_node_net_keys(FILE *fp, char prefix, wiced_bt_mesh_db_node_t *node);
int mesh_json_read_node_app_keys(FILE *fp, char prefix, wiced_bt_mesh_db_node_t *node);
int mesh_json_read_net_xmit(FILE *fp, char prefix, wiced_bt_mesh_db_node_t *node);
int mesh_json_read_relay_rexmit(FILE *fp, char prefix, wiced_bt_mesh_db_transmit_t *configuration);
int mesh_json_read_features(FILE *fp, char prefix, wiced_bt_mesh_db_node_t *node);
int mesh_json_read_models(FILE *fp, char prefix, wiced_bt_mesh_db_element_t *element);
int mesh_json_read_model_id(FILE *fp, char prefix, wiced_bt_mesh_db_model_id_t *value);
int mesh_json_read_model_subscribe(FILE *fp, char prefix, wiced_bt_mesh_db_model_t *model);
int mesh_json_read_model_publish(FILE *fp, char prefix, wiced_bt_mesh_db_publication_t *model);
int mesh_json_read_model_pub_rexmit(FILE *fp, char prefix, wiced_bt_mesh_db_transmit_t *pub_rexmit);
int mesh_json_read_model_bind(FILE *fp, char prefix, wiced_bt_mesh_db_model_t *model);
int mesh_json_read_model_sensors(FILE *fp, char prefix, wiced_bt_mesh_db_model_t *model);
int mesh_json_parse_skip_value(FILE *fp, char prefix);
char skip_space(FILE *fp);
char skip_comma(FILE *fp);
uint8_t process_nibble(char n);
extern uint8_t wiced_bt_mesh_property_len[WICED_BT_MESH_MAX_PROPERTY_ID + 1];

#if 0
int main()
{
    wiced_bt_mesh_db_mesh_t *p_mesh = NULL;
    FILE *fp;

    fp = fopen("net.json", "r");
    if (fp == NULL)
        return 0;

    p_mesh = mesh_json_read_file(fp);
    if (p_mesh == NULL)
        return 0;

    fclose(fp);

    fp = fopen("net1.json", "w");
    if (fp == NULL)
        return 0;

    mesh_json_write_file(fp, p_mesh);
    fclose(fp);
}
#endif

wiced_bt_mesh_db_mesh_t *mesh_json_read_file(FILE *fp)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    uint32_t tags = 0;
    wiced_bt_mesh_db_mesh_t *p_mesh = NULL;
    wiced_bool_t failed = WICED_FALSE;

    p_mesh = (wiced_bt_mesh_db_mesh_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_mesh_t));
    if (!p_mesh)
        return NULL;

    memset(p_mesh, 0, sizeof(wiced_bt_mesh_db_mesh_t));

    c1 = skip_space(fp);
    if (c1 != '{')
        failed = WICED_TRUE;
    else
    {
        c1 = skip_space(fp);
        if (c1 != '\"')
            failed = WICED_TRUE;
    }
    tags = 0;

    while (!failed)
    {
        if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
            break;

        c1 = skip_space(fp);
        if (c1 != ':')
        {
            failed = WICED_TRUE;
            break;
        }

        c1 = skip_space(fp);

        if (strcmp(tagname, "$schema") == 0)
        {
            if (!mesh_json_read_schema(fp, c1))
            {
                failed = WICED_TRUE;
                break;
            }
            tags |= MESH_JSON_TAG_SCHEMA;
        }
        else if (strcmp(tagname, "meshUUID") == 0)
        {
            if (!mesh_json_read_hex128(fp, c1, p_mesh->uuid))
            {
                failed = WICED_TRUE;
                break;
            }
            tags |= MESH_JSON_TAG_MESH_UUID;
        }
        else if (strcmp(tagname, "meshName") == 0)
        {
            if (!mesh_json_read_name(fp, c1, 512, &p_mesh->name))
            {
                failed = WICED_TRUE;
                break;
            }
            tags |= MESH_JSON_TAG_NAME;
        }
        else if (strcmp(tagname, "timestamp") == 0)
        {
            if (!mesh_json_read_hex32(fp, c1, &p_mesh->timestamp))
            {
                failed = WICED_TRUE;
                break;
            }
            tags |= MESH_JSON_TAG_TIMESTAMP;
        }
        else if (strcmp(tagname, "provisioners") == 0)
        {
            if (!mesh_json_read_provisioners(fp, c1, p_mesh))
            {
                failed = WICED_TRUE;
                break;
            }
            tags |= MESH_JSON_TAG_PROVISIONERS;
        }
        else if (strcmp(tagname, "netKeys") == 0)
        {
            if (!mesh_json_read_net_keys(fp, c1, p_mesh))
            {
                failed = WICED_TRUE;
                break;
            }
            tags |= MESH_JSON_TAG_NET_KEYS;
        }
        else if (strcmp(tagname, "appKeys") == 0)
        {
            if (!mesh_json_read_app_keys(fp, c1, p_mesh))
            {
                failed = WICED_TRUE;
                break;
            }
            tags |= MESH_JSON_TAG_APP_KEYS;
        }
        else if (strcmp(tagname, "nodes") == 0)
        {
            if (!mesh_json_read_nodes(fp, c1, p_mesh))
            {
                failed = WICED_TRUE;
                break;
            }
            tags |= MESH_JSON_TAG_NODES;
        }
        else if (strcmp(tagname, "groups") == 0)
        {
            if (!mesh_json_read_groups(fp, c1, p_mesh))
            {
                failed = WICED_TRUE;
                break;
            }
            tags |= MESH_JSON_TAG_GROUPS;
        }
        else if (strcmp(tagname, "scenes") == 0)
        {
            if (!mesh_json_read_scenes(fp, c1, p_mesh))
            {
                failed = WICED_TRUE;
                break;
            }
            tags |= MESH_JSON_TAG_SCENES;
        }
        else
        {
            // unknown tag
            if (!mesh_json_parse_skip_value(fp, c1))
            {
                failed = WICED_TRUE;
                break;
            }
        }

        if (skip_space(fp) != ',')
            break;

        c1 = skip_space(fp);
    }
    // All the fields are retrieved. Verify that all mandatory fields are present
    if (failed || ((tags & MESH_JSON_TAG_MANDATORY) != MESH_JSON_TAG_MANDATORY))
    {
        wiced_bt_mesh_db_deinit(p_mesh);
        return NULL;
    }
    return p_mesh;
}

int mesh_json_read_tag_name(FILE *fp, char *tagname, int len)
{
    int i = 0;
    int c = 0;

    for (i = 0; i < MAX_TAG_NAME; i++)
    {
        if (fread(&c, 1, 1, fp) <= 0)
            return 0;

        if (c == '\"')
        {
            tagname[i] = 0;
            return (strlen(tagname));
        }
        // no escapes in the tag name
        if (c == '\\')
            return 0;

        tagname[i] = c;
    }
    return 0;
}

int mesh_json_read_string(FILE *fp, char *buffer, int len)
{
    int i = 0;
    int c = 0;

    for (i = 0; i < len; i++)
    {
        if (fread(&c, 1, 1, fp) <= 0)
            return 0;

        if (c == '\"')
        {
            if (buffer)
                buffer[i] = 0;
            return (i + 1);
        }
        if (c == '\\')
        {
            if (fread(&c, 1, 1, fp) <= 0)
                return 0;

            switch (c)
            {
            case '\"':
            case '\\':
            case '/':
                if (buffer)
                    buffer[i] = c;
                break;
            case 'b':
                if (buffer)
                    buffer[i] = 8;
                break;
            case 'f':
                if (buffer)
                    buffer[i] = 12;
                break;
            case 'n':
                if (buffer)
                    buffer[i] = 10;
                break;
            case 'r':
                if (buffer)
                    buffer[i] = 13;
                break;
            case 't':
                if (buffer)
                    buffer[i] = 9;
                break;
            case 'u':
                // TBD
                // 4 hex unicode
                return 0;
            default:
                return 0;
            }
        }
        else
        {
            if (buffer)
                buffer[i] = c;
        }
    }
    return 0;
}

int mesh_json_read_schema(FILE *fp, char prefix)
{
    char schema[MAX_SCHEMA_LEN];

    if (prefix != '\"')
        return 0;

    if (!mesh_json_read_string(fp, schema, sizeof(schema)))
        return 0;

    return 1;
}

int mesh_json_read_hex_array(FILE *fp, char prefix, uint8_t *value)
{
    size_t  i;
    char buffer[100];

    if (prefix != '\"')
        return 0;

    if (!mesh_json_read_string(fp, buffer, 100))
        return 0;

    for (i = 0; i < strlen(buffer); i += 2)
    {
        uint8_t n1 = process_nibble(buffer[i]);
        if (n1 == 0xff)
            return 0;

        uint8_t n2 = process_nibble(buffer[i + 1]);
        if (n2 == 0xff)
            return 0;

        *value++ = (n1 << 4) + n2;
    }

    return i;
}

int mesh_json_read_hex128(FILE *fp, char prefix, uint8_t *uuid)
{
    char buffer[33];
    size_t  i;

    if (prefix != '\"')
        return 0;

    if (!mesh_json_read_string(fp, buffer, 33))
        return 0;

    for (i = 0; i < strlen(buffer); i += 2)
    {
        uint8_t n1 = process_nibble(buffer[i]);
        if (n1 == 0xff)
            return 0;

        uint8_t n2 = process_nibble(buffer[i + 1]);
        if (n2 == 0xff)
            return 0;

        *uuid++ = (n1 << 4) + n2;
    }
    return i;
}

int mesh_json_read_hex16(FILE *fp, char prefix, uint16_t *value)
{
    char temp[5];
    int value_len;
    int i;
    uint8_t *p_value = (uint8_t *)value;

    if (prefix != '\"')
        return 0;

    value_len = mesh_json_read_string(fp, temp, 5);
    if (value_len != 5)
        return 0;

    for (i = 0; i < 4; i += 2)
    {
        uint8_t n1 = process_nibble(temp[i]);
        if (n1 == 0xff)
            return 0;

        uint8_t n2 = process_nibble(temp[i + 1]);
        if (n2 == 0xff)
            return 0;

        p_value[1 - (i / 2)] = (n1 << 4) + n2;
    }
    return i;
}

int mesh_json_read_sub_addr(FILE *fp, char prefix, uint16_t *value)
{
    char temp[5];
    int value_len;
    int i;
    uint8_t *p_value = (uint8_t *)value;

    if (prefix != '\"')
        return 0;

    *value = 0;

    value_len = mesh_json_read_string(fp, temp, 5);
    if (value_len != 5)
        return 0;

    for (i = 0; i < value_len - 1; i += 2)
    {
        uint8_t n1 = process_nibble(temp[i]);
        if (n1 == 0xff)
            return 0;

        uint8_t n2 = process_nibble(temp[i + 1]);
        if (n2 == 0xff)
            return 0;

        p_value[1 - (i / 2)] = (n1 << 4) + n2;
    }
    return i;
}

int mesh_json_read_hex32(FILE *fp, char prefix, uint32_t *value)
{
    char temp[9];
    int value_len;
    int i;
    uint8_t *p_value = (uint8_t *)value;

    if (prefix != '\"')
        return 0;

    value_len = mesh_json_read_string(fp, temp, 9);
    if (value_len != 9)
        return 0;

    for (i = 0; i < 8; i += 2)
    {
        uint8_t n1 = process_nibble(temp[i]);
        if (n1 == 0xff)
            return 0;

        uint8_t n2 = process_nibble(temp[i + 1]);
        if (n2 == 0xff)
            return 0;

        p_value[1 - (i / 2)] = (n1 << 4) + n2;
    }
    return i;
}

int mesh_json_read_model_id(FILE *fp, char prefix, wiced_bt_mesh_db_model_id_t *value)
{
    char temp[9];
    int value_len;
    int i, j = 0;
    uint8_t *p_value;

    if (prefix != '\"')
        return 0;

    value_len = mesh_json_read_string(fp, temp, 9);
    if ((value_len != 9) && (value_len != 5))
        return 0;

    if (value_len == 5)
    {
        value->company_id = MESH_COMPANY_ID_BT_SIG;
    }
    else
    {
        p_value = (uint8_t *)&value->company_id;
        for (i = 0; i < 4; i += 2)
        {
            uint8_t n1 = process_nibble(temp[i]);
            if (n1 == 0xff)
                return 0;

            uint8_t n2 = process_nibble(temp[i + 1]);
            if (n2 == 0xff)
                return 0;

            p_value[1 - (i / 2)] = (n1 << 4) + n2;
        }
        j = 4;
    }
    p_value = (uint8_t *)&value->id;
    for (i = 0; i < 4; i += 2)
    {
        uint8_t n1 = process_nibble(temp[i + j]);
        if (n1 == 0xff)
            return 0;

        uint8_t n2 = process_nibble(temp[i + j + 1]);
        if (n2 == 0xff)
            return 0;

        p_value[1 - (i / 2)] = (n1 << 4) + n2;
    }
    return i + j;
}

int mesh_json_read_uint8(FILE *fp, char prefix, uint8_t *value)
{
    uint32_t temp;
    int c1 = 0;

    if ((prefix < '0') || (prefix > '9'))
        return 0;

    temp = prefix - '0';

    while (fread(&c1, 1, 1, fp) > 0)
    {
        if ((c1 < '0') || (c1 > '9'))
        {
            fseek(fp, -1, SEEK_CUR);
            *value = (uint8_t)temp;
            return 1;
        }
        temp = temp * 10 + c1 - '0';
        if (temp > 0xFF)
            break;
    }
    return 0;
}

int mesh_json_read_uint16(FILE *fp, char prefix, uint16_t *value)
{
    uint32_t temp;
    int c1 = 0;

    if ((prefix < '0') || (prefix > '9'))
        return 0;

    temp = prefix - '0';

    while (fread(&c1, 1, 1, fp) > 0)
    {
        if ((c1 < '0') || (c1 > '9'))
        {
            fseek(fp, -1, SEEK_CUR);
            *value = (uint16_t)temp;
            return 1;
        }
        temp = temp * 10 + c1 - '0';
        if (temp > 0xFFFF)
            break;
    }
    return 0;
}

int mesh_json_read_uint32(FILE *fp, char prefix, uint32_t *value)
{
    uint64_t temp;
    int c1 = 0;

    if ((prefix < '0') || (prefix > '9'))
        return 0;

    temp = prefix - '0';

    while (fread(&c1, 1, 1, fp) > 0)
    {
        if ((c1 < '0') || (c1 > '9'))
        {
            fseek(fp, -1, SEEK_CUR);
            *value = (uint32_t)temp;
            return 1;
        }
        temp = temp * 10 + c1 - '0';
        if (temp > 0xFFFFFFFF)
            break;
    }
    return 0;
}

int mesh_json_read_boolean(FILE *fp, char prefix, uint8_t *value)
{
    int c1 = 0;
    static char *s_true = "true";
    static char *s_false = "false";
    char *p;

    if (prefix == 't')
    {
        p = s_true;
        *value = 1;
    }
    else if (prefix == 'f')
    {
        p = s_false;
        *value = 0;
    }
    else
    {
        return 0;
    }
    for (p = p + 1; *p != 0; p++)
    {
        if (fread(&c1, 1, 1, fp) <= 0)
            return 0;
        if (c1 != *p)
            return 0;
    }
    return 1;
}


int mesh_json_parse_skip_value(FILE *fp, char prefix)
{
    char c1;
    char terminating_char;
    int skip = 0;

    if (prefix == '\"')
        terminating_char = '\"';
    else if (prefix == '{')
        terminating_char = '}';
    else if (prefix == '[')
        terminating_char = ']';
    else
        terminating_char = 0;

    while (fread(&c1, 1, 1, fp) > 0)
    {
        if ((terminating_char == 0) && ((c1 < '0') || (c1 > '9')))
            return 1;

        else if (c1 == terminating_char)
        {
            if (skip == 0)
                return 1;
            skip--;
        }

        else if (c1 == prefix)
            skip++;
    }
    return 0;

}

int mesh_json_read_name(FILE *fp, char prefix, int max_len, char **p_name)
{
    int pos;
    int namelen;

    if (prefix != '\"')
        return 0;

    pos = ftell(fp);

    namelen = mesh_json_read_string(fp, NULL, 512);
    if (namelen == 0)
        return 0;

    *p_name = (char *)wiced_bt_get_buffer(namelen);
    if (*p_name == NULL)
        return 0;

    fseek(fp, -namelen, SEEK_CUR);
    return mesh_json_read_string(fp, *p_name, namelen);
}

int mesh_json_read_security(FILE *fp, char prefix, uint8_t *security)
{
    int pos;
    int namelen;
    char s[5];

    if (prefix != '\"')
        return 0;

    pos = ftell(fp);

    namelen = mesh_json_read_string(fp, NULL, 512);
    fseek(fp, -namelen, SEEK_CUR);

    if (namelen == 4)
    {
        mesh_json_read_string(fp, s, namelen);
        if (strcmp(s, "low") == 0)
        {
            *security = 0;
            return namelen;
        }
    }
    else if (namelen == 5)
    {
        mesh_json_read_string(fp, s, namelen);
        if (strcmp(s, "high") == 0)
        {
            *security = 1;
            return namelen;
        }
    }
    return 0;
}

int mesh_json_read_net_keys(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    wiced_bt_mesh_db_net_key_t key;
    uint32_t tags = 0;

    if (prefix != '[')
        return 0;

    while (1)
    {
        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != '{')
            return 0;

        c1 = skip_space(fp);
        if (c1 != '\"')
            return 0;

        memset(&key, 0, sizeof(wiced_bt_mesh_db_net_key_t));
        tags = 0;

        while (1)
        {
            if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
                break;

            c1 = skip_space(fp);
            if (c1 != ':')
                return 0;

            c1 = skip_space(fp);

            if (strcmp(tagname, "name") == 0)
            {
                if (!mesh_json_read_name(fp, c1, 512, &key.name))
                    return 0;
                tags |= MESH_JSON_KEY_TAG_NAME;
            }
            else if (strcmp(tagname, "index") == 0)
            {
                if (!mesh_json_read_uint16(fp, c1, &key.index))
                    return 0;
                tags |= MESH_JSON_KEY_TAG_KEY_INDEX;
            }
            else if (strcmp(tagname, "phase") == 0)
            {
                if (!mesh_json_read_uint8(fp, c1, &key.phase))
                    return 0;
                tags |= MESH_JSON_KEY_TAG_PHASE;
            }
            else if (strcmp(tagname, "key") == 0)
            {
                if (!mesh_json_read_hex128(fp, c1, key.key))
                    return 0;
                tags |= MESH_JSON_KEY_TAG_KEY;
            }
            else if (strcmp(tagname, "minSecurity") == 0)
            {
                if (!mesh_json_read_security(fp, c1, &key.min_security))
                    return 0;
                tags |= MESH_JSON_KEY_TAG_MIN_SECURITY;
            }
            else if (strcmp(tagname, "oldKey") == 0)
            {
                if (!mesh_json_read_hex128(fp, c1, key.old_key))
                    return 0;
                tags |= MESH_JSON_KEY_TAG_OLD_KEY;
            }
            else
            {
                // unknown tag
                if (!mesh_json_parse_skip_value(fp, c1))
                    return 0;
            }
            c1 = skip_space(fp);

            if (c1 != ',')
                break;

            c1 = skip_space(fp);
        }
        if (c1 != '}')
            return 0;

        // All the fields are retrieved. Verify that all mandatory fields are present.
        // if phase is not 0, the old key should be present.
        if (((tags & MESH_JSON_TAG_NET_KEY_MANDATORY) != MESH_JSON_TAG_NET_KEY_MANDATORY) ||
            (((tags & MESH_JSON_KEY_TAG_PHASE) != 0) && (key.phase != 0) && ((tags & MESH_JSON_KEY_TAG_OLD_KEY) == 0)))
            return 0;

        wiced_bt_mesh_db_net_key_add(p_mesh, &key);

        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != ',')
            return 0;
    }
    return 1;
}

int mesh_json_read_app_keys(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    wiced_bt_mesh_db_app_key_t key;
    uint32_t tags = 0;
    wiced_bt_mesh_db_app_key_t *p_temp;
    wiced_bool_t failed = WICED_FALSE;

    if (prefix != '[')
        return 0;

    while (1)
    {
        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != '{')
            return 0;

        c1 = skip_space(fp);
        if (c1 != '\"')
            return 0;

        memset(&key, 0, sizeof(wiced_bt_mesh_db_app_key_t));
        tags = 0;

        while (1)
        {
            if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
                break;

            c1 = skip_space(fp);
            if (c1 != ':')
                return 0;

            c1 = skip_space(fp);

            if (strcmp(tagname, "name") == 0)
            {
                if (!mesh_json_read_name(fp, c1, 512, &key.name))
                    return 0;
                tags |= MESH_JSON_KEY_TAG_NAME;
            }
            else if (strcmp(tagname, "index") == 0)
            {
                if (!mesh_json_read_uint16(fp, c1, &key.index))
                    return 0;
                tags |= MESH_JSON_KEY_TAG_KEY_INDEX;
            }
            else if (strcmp(tagname, "boundNetKey") == 0)
            {
                if (!mesh_json_read_uint16(fp, c1, &key.bound_net_key_index))
                    return 0;
                tags |= MESH_JSON_KEY_TAG_BOUND_NET_KEY;
            }
            else if (strcmp(tagname, "key") == 0)
            {
                if (!mesh_json_read_hex128(fp, c1, key.key))
                    return 0;
                tags |= MESH_JSON_KEY_TAG_KEY;
            }
            else if (strcmp(tagname, "oldKey") == 0)
            {
                if (!mesh_json_read_hex128(fp, c1, key.old_key))
                    return 0;
                tags |= MESH_JSON_KEY_TAG_OLD_KEY;
            }
            else
            {
                // unknown tag
                if (!mesh_json_parse_skip_value(fp, c1))
                    return 0;
            }
            c1 = skip_space(fp);

            if (c1 != ',')
                break;

            c1 = skip_space(fp);
        }
        if (c1 != '}')
            return 0;

        // All the fields are retrieved. Verify that all mandatory fields are present.
        // TBD if phase of the bound net key is not 0, the old key should be present.
        if ((tags & MESH_JSON_TAG_APP_KEY_MANDATORY) != MESH_JSON_TAG_APP_KEY_MANDATORY)
            return 0;

        p_temp = p_mesh->app_key;
        p_mesh->app_key = (wiced_bt_mesh_db_app_key_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_app_key_t) * (p_mesh->num_app_keys + 1));
        if (p_mesh->app_key == NULL)
            return 0;
        if (p_temp != 0)
        {
            memcpy(p_mesh->app_key, p_temp, sizeof(wiced_bt_mesh_db_app_key_t) * p_mesh->num_app_keys);
            wiced_bt_free_buffer(p_temp);
        }
        memcpy(&p_mesh->app_key[p_mesh->num_app_keys], &key, sizeof(wiced_bt_mesh_db_app_key_t));
        p_mesh->num_app_keys++;

        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != ',')
            return 0;
    }
    return 1;
}

int mesh_json_read_provisioners(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    wiced_bt_mesh_db_provisioner_t provisioner;
    uint32_t tags = 0;
    wiced_bt_mesh_db_provisioner_t *p_temp;

    if (prefix != '[')
        return 0;

    while (1)
    {
        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != '{')
            return 0;

        c1 = skip_space(fp);
        if (c1 != '\"')
            return 0;

        memset(&provisioner, 0, sizeof(wiced_bt_mesh_db_provisioner_t));
        tags = 0;

        while (1)
        {
            if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
                break;

            c1 = skip_space(fp);
            if (c1 != ':')
                return 0;

            c1 = skip_space(fp);

            if (strcmp(tagname, "provisionerName") == 0)
            {
                if (!mesh_json_read_name(fp, c1, 512, &provisioner.name))
                    return 0;
                tags |= MESH_JSON_PROVISIONER_TAG_NAME;
            }
            else if (strcmp(tagname, "UUID") == 0)
            {
                if (!mesh_json_read_hex128(fp, c1, provisioner.uuid))
                    return 0;
                tags |= MESH_JSON_PROVISIONER_TAG_UUID;
            }
            else if (strcmp(tagname, "allocatedGroupRange") == 0)
            {
                if (!mesh_json_read_addr_range(fp, c1, &provisioner, RANGE_TYPE_GROUP))
                    return 0;
                tags |= MESH_JSON_PROVISIONER_TAG_GROUP_RANGE;
            }
            else if (strcmp(tagname, "allocatedUnicastRange") == 0)
            {
                if (!mesh_json_read_addr_range(fp, c1, &provisioner, RANGE_TYPE_UNICAST))
                    return 0;
                tags |= MESH_JSON_PROVISIONER_TAG_UNICAST_RANGE;
            }
            else if (strcmp(tagname, "allocatedScenetRange") == 0)
            {
                if (!mesh_json_read_addr_range(fp, c1, &provisioner, RANGE_TYPE_SCENE))
                    return 0;
                tags |= MESH_JSON_PROVISIONER_TAG_UNICAST_RANGE;
            }
            else
            {
                // unknown tag
                if (!mesh_json_parse_skip_value(fp, c1))
                    return 0;
            }
            c1 = skip_space(fp);

            if (c1 != ',')
                break;

            c1 = skip_space(fp);
        }
        if (c1 != '}')
            return 0;

        // All the fields are retrieved. Verify that all mandatory fields are present.
        if ((tags & MESH_JSON_TAG_PROVISIONER_MANDATORY) != MESH_JSON_TAG_PROVISIONER_MANDATORY)
            return 0;

        p_temp = p_mesh->provisioner;
        p_mesh->provisioner = (wiced_bt_mesh_db_provisioner_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_provisioner_t) * (p_mesh->num_provisioners + 1));
        if (p_mesh->provisioner == NULL)
            return 0;
        if (p_temp != 0)
        {
            memcpy(p_mesh->provisioner, p_temp, sizeof(wiced_bt_mesh_db_provisioner_t) * p_mesh->num_provisioners);
            wiced_bt_free_buffer(p_temp);
        }
        memcpy(&p_mesh->provisioner[p_mesh->num_provisioners], &provisioner, sizeof(wiced_bt_mesh_db_provisioner_t));
        p_mesh->num_provisioners++;

        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != ',')
            return 0;
    }
    return 1;
}

int mesh_json_read_addr_range(FILE *fp, char c1, wiced_bt_mesh_db_provisioner_t *provisioner, int range_type)
{
    char tagname[MAX_TAG_NAME];
    uint32_t tags = 0;
    wiced_bt_mesh_db_range_t *p_temp;
    wiced_bt_mesh_db_range_t range;
    wiced_bt_mesh_db_range_t **p_prov_range;
    uint16_t *num_ranges;

    if (c1 != '[')
        return 0;

    while (1)
    {
        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != '{')
            return 0;

        c1 = skip_space(fp);
        if (c1 != '\"')
            return 0;

        memset(&range, 0, sizeof(wiced_bt_mesh_db_range_t));
        tags = 0;

        while (1)
        {
            if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
                break;

            c1 = skip_space(fp);
            if (c1 != ':')
                return 0;

            c1 = skip_space(fp);

            if (strcmp(tagname, "highAddress") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &range.high_addr))
                    return 0;
            }
            else if (strcmp(tagname, "lowAddress") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &range.low_addr))
                    return 0;
            }
            else
            {
                // unknown tag
                if (!mesh_json_parse_skip_value(fp, c1))
                    return 0;
            }
            c1 = skip_space(fp);

            if (c1 != ',')
                break;

            c1 = skip_space(fp);
        }
        if (c1 != '}')
            return 0;

        // All the fields are retrieved. Verify that all mandatory fields are present
        if ((range.high_addr == 0) || (range.low_addr == 0))
            return 0;

        if (range_type == RANGE_TYPE_GROUP)
        {
            p_prov_range = &provisioner->p_allocated_group_range;
            num_ranges = &provisioner->num_allocated_group_ranges;
        }
        else if (range_type == RANGE_TYPE_UNICAST)
        {
            p_prov_range = &provisioner->p_allocated_unicast_range;
            num_ranges = &provisioner->num_allocated_unicast_ranges;
        }
        else
        {
            p_prov_range = &provisioner->p_allocated_scene_range;
            num_ranges = &provisioner->num_allocated_scene_ranges;
        }

        p_temp = *p_prov_range;
        *p_prov_range = (wiced_bt_mesh_db_range_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_range_t) * ((*num_ranges) + 1));
        if (*p_prov_range == NULL)
            return 0;
        if (p_temp != 0)
        {
            memcpy(*p_prov_range, p_temp, sizeof(wiced_bt_mesh_db_range_t) * (*num_ranges));
            wiced_bt_free_buffer(p_temp);
        }
        memcpy(&(*p_prov_range)[*num_ranges], &range, sizeof(wiced_bt_mesh_db_range_t));
        (*num_ranges)++;

        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != ',')
            return 0;
    }
    return 1;
}

int mesh_json_read_nodes(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    wiced_bt_mesh_db_node_t node;
    uint32_t tags = 0;
    wiced_bt_mesh_db_node_t *p_temp;

    if (prefix != '[')
        return 0;

    p_mesh->num_nodes = 0;

    while (1)
    {
        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != '{')
            return 0;

        c1 = skip_space(fp);
        if (c1 != '\"')
            return 0;

        memset(&node, 0, sizeof(wiced_bt_mesh_db_node_t));
        node.feature.relay = MESH_FEATURE_SUPPORTED_UNKNOWN;
        node.feature.gatt_proxy = MESH_FEATURE_SUPPORTED_UNKNOWN;
        node.feature.low_power = MESH_FEATURE_SUPPORTED_UNKNOWN;
        node.feature.friend = MESH_FEATURE_SUPPORTED_UNKNOWN;
        node.beacon = MESH_FEATURE_SUPPORTED_UNKNOWN;
        node.default_ttl = MESH_FEATURE_SUPPORTED_UNKNOWN;
        node.network_transmit.count = MESH_FEATURE_SUPPORTED_UNKNOWN;
        node.relay_rexmit.count = MESH_FEATURE_SUPPORTED_UNKNOWN;

        tags = 0;

        while (1)
        {
            if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
                break;

            c1 = skip_space(fp);
            if (c1 != ':')
                return 0;

            c1 = skip_space(fp);

            if (strcmp(tagname, "UUID") == 0)
            {
                if (!mesh_json_read_hex128(fp, c1, node.uuid))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_UUID;
            }
            else if (strcmp(tagname, "unicastAddress") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &node.unicast_address))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_UNICAST_ADDR;
            }
            else if (strcmp(tagname, "deviceKey") == 0)
            {
                if (!mesh_json_read_hex128(fp, c1, node.device_key))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_DEVICE_KEY;
            }
            else if (strcmp(tagname, "security") == 0)
            {
                if (!mesh_json_read_security(fp, c1, &node.security))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_SECURITY;
            }
            else if (strcmp(tagname, "netKeys") == 0)
            {
                if (!mesh_json_read_node_net_keys(fp, c1, &node))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_NET_KEYS;
            }
            else if (strcmp(tagname, "configComplete") == 0)
            {
                if (!mesh_json_read_boolean(fp, c1, &node.config_complete))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_CONFIG_COMPLETE;
            }
            else if (strcmp(tagname, "name") == 0)
            {
                if (!mesh_json_read_name(fp, c1, 512, &node.name))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_NAME;
            }
            else if (strcmp(tagname, "cid") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &node.cid))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_CID;
            }
            else if (strcmp(tagname, "pid") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &node.pid))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_PID;
            }
            else if (strcmp(tagname, "vid") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &node.vid))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_VID;
            }
            else if (strcmp(tagname, "crpl") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &node.crpl))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_CRPL;
            }
            else if (strcmp(tagname, "features") == 0)
            {
                if (!mesh_json_read_features(fp, c1, &node))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_FEATURES;
            }
            else if (strcmp(tagname, "secureNetworkBeacon") == 0)
            {
                if (!mesh_json_read_boolean(fp, c1, &node.beacon))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_BEACON;
            }
            else if (strcmp(tagname, "defaultTTL") == 0)
            {
                if (!mesh_json_read_uint8(fp, c1, &node.default_ttl))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_DEFAULT_TTL;
            }
            else if (strcmp(tagname, "networkTransmit") == 0)
            {
                if (!mesh_json_read_net_xmit(fp, c1, &node))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_NET_TRANSMIT;
            }
            else if (strcmp(tagname, "relayRetransmit") == 0)
            {
                if (!mesh_json_read_relay_rexmit(fp, c1, &node.relay_rexmit))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_RELAY_REXMIT;
            }
            else if (strcmp(tagname, "appKeys") == 0)
            {
                if (!mesh_json_read_node_app_keys(fp, c1, &node))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_APP_KEYS;
            }
            else if (strcmp(tagname, "elements") == 0)
            {
                if (!mesh_json_read_elements(fp, c1, &node))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_ELEMENTS;
            }
            else if (strcmp(tagname, "blacklisted") == 0)
            {
                if (!mesh_json_read_boolean(fp, c1, &node.blocked))
                    return 0;
                tags |= MESH_JSON_NODE_TAG_BLACKLISTED;
            }
            else
            {
                // unknown tag
                if (!mesh_json_parse_skip_value(fp, c1))
                    return 0;
            }
            c1 = skip_space(fp);

            if (c1 != ',')
                break;

            c1 = skip_space(fp);
        }
        if (c1 != '}')
            return 0;

        // All the fields are retrieved. Verify that all mandatory fields are present
        if ((tags & MESH_JSON_TAG_NODE_MANDATORY) != MESH_JSON_TAG_NODE_MANDATORY)
            return 0;

        p_temp = p_mesh->node;
        p_mesh->node = (wiced_bt_mesh_db_node_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_node_t) * (p_mesh->num_nodes + 1));
        if (p_mesh->node == NULL)
            return 0;
        if (p_temp != 0)
        {
            memcpy(p_mesh->node, p_temp, sizeof(wiced_bt_mesh_db_node_t) * p_mesh->num_nodes);
            wiced_bt_free_buffer(p_temp);
        }
        memcpy(&p_mesh->node[p_mesh->num_nodes], &node, sizeof(wiced_bt_mesh_db_node_t));
        p_mesh->num_nodes++;

        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != ',')
            return 0;
    }
    return 1;
}

int mesh_json_read_groups(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    wiced_bt_mesh_db_group_t group;
    uint32_t tags = 0;
    wiced_bt_mesh_db_group_t *p_temp;

    if (prefix != '[')
        return 0;

    p_mesh->num_groups = 0;

    while (1)
    {
        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != '{')
            return 0;

        c1 = skip_space(fp);
        if (c1 != '\"')
            return 0;

        memset(&group, 0, sizeof(wiced_bt_mesh_db_group_t));
        tags = 0;

        while (1)
        {
            if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
                break;

            c1 = skip_space(fp);
            if (c1 != ':')
                return 0;

            c1 = skip_space(fp);

            if (strcmp(tagname, "name") == 0)
            {
                if (!mesh_json_read_name(fp, c1, 512, &group.name))
                    return 0;
                tags |= MESH_JSON_GROUP_TAG_NAME;
            }
            else if (strcmp(tagname, "address") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &group.addr))
                    return 0;
                tags |= MESH_JSON_GROUP_TAG_ADDR;
            }
            else if (strcmp(tagname, "parent") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &group.parent_addr))
                    return 0;
                tags |= MESH_JSON_GROUP_TAG_PARENT;
            }
            else
            {
                // unknown tag
                if (!mesh_json_parse_skip_value(fp, c1))
                    return 0;
            }
            c1 = skip_space(fp);

            if (c1 != ',')
                break;

            c1 = skip_space(fp);
        }
        if (c1 != '}')
            return 0;

        // All the fields are retrieved. Verify that all mandatory fields are present
        if ((tags & MESH_JSON_TAG_GROUP_MANDATORY) != MESH_JSON_TAG_GROUP_MANDATORY)
            return 0;

        p_temp = p_mesh->group;
        p_mesh->group = (wiced_bt_mesh_db_group_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_group_t) * (p_mesh->num_groups + 1));
        if (p_mesh->group == NULL)
            return 0;
        if (p_temp != 0)
        {
            memcpy(p_mesh->group, p_temp, sizeof(wiced_bt_mesh_db_group_t) * p_mesh->num_groups);
            wiced_bt_free_buffer(p_temp);
        }
        memcpy(&p_mesh->group[p_mesh->num_groups], &group, sizeof(wiced_bt_mesh_db_group_t));
        p_mesh->num_groups++;

        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != ',')
            return 0;
    }
    return 1;
}

int mesh_json_read_scenes(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    wiced_bt_mesh_db_scene_t scene;
    uint32_t tags = 0;
    wiced_bt_mesh_db_scene_t *p_temp;

    if (prefix != '[')
        return 0;

    p_mesh->num_scenes = 0;

    while (1)
    {
        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != '{')
            return 0;

        c1 = skip_space(fp);
        if (c1 != '\"')
            return 0;

        memset(&scene, 0, sizeof(wiced_bt_mesh_db_scene_t));
        tags = 0;

        while (1)
        {
            if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
                break;

            c1 = skip_space(fp);
            if (c1 != ':')
                return 0;

            c1 = skip_space(fp);

            if (strcmp(tagname, "name") == 0)
            {
                if (!mesh_json_read_name(fp, c1, 512, &scene.name))
                    return 0;
                tags |= MESH_JSON_SCENE_TAG_NAME;
            }
            else if (strcmp(tagname, "address") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &scene.addr))
                    return 0;
                tags |= MESH_JSON_SCENE_TAG_ADDR;
            }
            else if (strcmp(tagname, "number") == 0)
            {
                if (!mesh_json_read_uint16(fp, c1, &scene.number))
                    return 0;
                tags |= MESH_JSON_SCENE_TAG_NUMBER;
            }
            else
            {
                // unknown tag
                if (!mesh_json_parse_skip_value(fp, c1))
                    return 0;
            }
            c1 = skip_space(fp);

            if (c1 != ',')
                break;

            c1 = skip_space(fp);
        }
        if (c1 != '}')
            return 0;

        // All the fields are retrieved. Verify that all mandatory fields are present
        // TBD

        p_temp = p_mesh->scene;
        p_mesh->scene = (wiced_bt_mesh_db_scene_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_scene_t) * (p_mesh->num_scenes + 1));
        if (p_mesh->scene == NULL)
            return 0;
        if (p_temp != 0)
        {
            memcpy(p_mesh->scene, p_temp, sizeof(wiced_bt_mesh_db_scene_t) * p_mesh->num_scenes);
            wiced_bt_free_buffer(p_temp);
        }
        memcpy(&p_mesh->scene[p_mesh->num_scenes], &scene, sizeof(wiced_bt_mesh_db_scene_t));
        p_mesh->num_scenes++;

        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != ',')
            return 0;
    }
    return 1;
}

int mesh_json_read_elements(FILE *fp, char prefix, wiced_bt_mesh_db_node_t *node)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    wiced_bt_mesh_db_element_t element;
    uint32_t tags = 0;
    wiced_bt_mesh_db_element_t *p_temp;

    if (prefix != '[')
        return 0;

    while (1)
    {
        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != '{')
            return 0;

        c1 = skip_space(fp);
        if (c1 != '\"')
            return 0;

        memset(&element, 0, sizeof(wiced_bt_mesh_db_element_t));
        tags = 0;

        while (1)
        {
            if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
                break;

            c1 = skip_space(fp);
            if (c1 != ':')
                return 0;

            c1 = skip_space(fp);

            if (strcmp(tagname, "name") == 0)
            {
                if (!mesh_json_read_name(fp, c1, 512, &element.name))
                    return 0;
                tags |= MESH_JSON_ELEMENT_TAG_NAME;
            }
            else if (strcmp(tagname, "index") == 0)
            {
                if (!mesh_json_read_uint8(fp, c1, &element.index))
                    return 0;
                tags |= MESH_JSON_ELEMENT_TAG_INDEX;
            }
            else if (strcmp(tagname, "location") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &element.location))
                    return 0;
                tags |= MESH_JSON_ELEMENT_TAG_LOCATION;
            }
            else if (strcmp(tagname, "models") == 0)
            {
                if (!mesh_json_read_models(fp, c1, &element))
                    return 0;
                tags |= MESH_JSON_ELEMENT_TAG_MODELS;
            }
            else
            {
                // unknown tag
                if (!mesh_json_parse_skip_value(fp, c1))
                    return 0;
            }
            c1 = skip_space(fp);

            if (c1 != ',')
                break;

            c1 = skip_space(fp);
        }
        if (c1 != '}')
            return 0;

        // All the fields are retrieved. Verify that all mandatory fields are present
        // TBD

        p_temp = node->element;
        node->element = (wiced_bt_mesh_db_element_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_element_t) * (node->num_elements + 1));
        if (node->element == NULL)
            return 0;
        if (p_temp != 0)
        {
            memcpy(node->element, p_temp, sizeof(wiced_bt_mesh_db_element_t) * node->num_elements);
            wiced_bt_free_buffer(p_temp);
        }
        memcpy(&node->element[node->num_elements], &element, sizeof(wiced_bt_mesh_db_element_t));
        node->num_elements++;

        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != ',')
            return 0;
    }
    return 1;
}

int mesh_json_read_node_net_keys(FILE *fp, char prefix, wiced_bt_mesh_db_node_t *node)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    wiced_bt_mesh_db_key_idx_phase key = { 0 };
    uint32_t tags = 0;
    wiced_bt_mesh_db_key_idx_phase *p_temp;

    if (prefix != '[')
        return 0;

    while (1)
    {
        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != '{')
            return 0;

        c1 = skip_space(fp);
        if (c1 != '\"')
            return 0;

        tags = 0;

        while (1)
        {
            if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
                break;

            c1 = skip_space(fp);
            if (c1 != ':')
                return 0;

            c1 = skip_space(fp);

            if (strcmp(tagname, "key") == 0)
            {
                if (!mesh_json_read_uint16(fp, c1, &key.index))
                    return 0;
                tags |= MESH_JSON_CONFIG_KEY_TAG_KEY;
            }
            else if (strcmp(tagname, "phase") == 0)
            {
                if (!mesh_json_read_uint8(fp, c1, &key.phase))
                    return 0;
            }
            else
            {
                // unknown tag
                if (!mesh_json_parse_skip_value(fp, c1))
                    return 0;
            }
            c1 = skip_space(fp);

            if (c1 != ',')
                break;

            c1 = skip_space(fp);
        }
        if (c1 != '}')
            return 0;

        if ((tags & MESH_JSON_CONFIG_KEY_TAG_KEY) != MESH_JSON_CONFIG_KEY_TAG_KEY)
            return 0;

        p_temp = node->net_key;
        node->net_key = (wiced_bt_mesh_db_key_idx_phase *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_key_idx_phase) * (node->num_net_keys + 1));
        if (node->net_key == NULL)
            return 0;
        if (p_temp != 0)
        {
            memcpy(node->net_key, p_temp, sizeof(wiced_bt_mesh_db_key_idx_phase) * node->num_net_keys);
            wiced_bt_free_buffer(p_temp);
        }
        memcpy(&node->net_key[node->num_net_keys], &key, sizeof(wiced_bt_mesh_db_key_idx_phase));
        node->num_net_keys++;

        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != ',')
            return 0;
    }
    return 1;
}

int mesh_json_read_node_app_keys(FILE *fp, char prefix, wiced_bt_mesh_db_node_t *node)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    wiced_bt_mesh_db_key_idx_phase key = { 0 };
    uint32_t tags = 0;
    wiced_bt_mesh_db_key_idx_phase *p_temp;

    if (prefix != '[')
        return 0;

    while (1)
    {
        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != '{')
            return 0;

        c1 = skip_space(fp);
        if (c1 != '\"')
            return 0;

        tags = 0;

        while (1)
        {
            if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
                break;

            c1 = skip_space(fp);
            if (c1 != ':')
                return 0;

            c1 = skip_space(fp);

            if (strcmp(tagname, "key") == 0)
            {
                if (!mesh_json_read_uint16(fp, c1, &key.index))
                    return 0;
                tags |= MESH_JSON_CONFIG_KEY_TAG_KEY;
            }
            else if (strcmp(tagname, "phase") == 0)
            {
                if (!mesh_json_read_uint8(fp, c1, &key.phase))
                    return 0;
            }
            else
            {
                // unknown tag
                if (!mesh_json_parse_skip_value(fp, c1))
                    return 0;
            }
            c1 = skip_space(fp);

            if (c1 != ',')
                break;

            c1 = skip_space(fp);
        }
        if (c1 != '}')
            return 0;

        if ((tags & MESH_JSON_CONFIG_KEY_TAG_KEY) != MESH_JSON_CONFIG_KEY_TAG_KEY)
            return 0;

        p_temp = node->app_key;
        node->app_key = (wiced_bt_mesh_db_key_idx_phase *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_key_idx_phase) * (node->num_app_keys + 1));
        if (node->app_key == NULL)
            return 0;
        if (p_temp != 0)
        {
            memcpy(node->app_key, p_temp, sizeof(wiced_bt_mesh_db_key_idx_phase) * node->num_app_keys);
            wiced_bt_free_buffer(p_temp);
        }
        memcpy(&node->app_key[node->num_app_keys], &key, sizeof(wiced_bt_mesh_db_key_idx_phase));
        node->num_app_keys++;

        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != ',')
            return 0;
    }
    return 1;
}

int mesh_json_read_net_xmit(FILE *fp, char prefix, wiced_bt_mesh_db_node_t *node)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    uint32_t tags = 0;

    if (prefix != '{')
        return 0;

    c1 = skip_space(fp);
    if (c1 != '\"')
        return 0;

    tags = 0;

    while (1)
    {
        if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
            break;

        c1 = skip_space(fp);
        if (c1 != ':')
            return 0;

        c1 = skip_space(fp);

        if (strcmp(tagname, "count") == 0)
        {
            if (!mesh_json_read_uint16(fp, c1, &node->network_transmit.count))
                return 0;
            tags |= MESH_JSON_CONFIG_NET_XMIT_TAG_COUNT;
        }
        else if (strcmp(tagname, "interval") == 0)
        {
            if (!mesh_json_read_uint32(fp, c1, &node->network_transmit.interval))
                return 0;
            tags |= MESH_JSON_CONFIG_NET_XMIT_TAG_INTERVAL;
        }
        else
        {
            // unknown tag
            if (!mesh_json_parse_skip_value(fp, c1))
                return 0;
        }
        c1 = skip_space(fp);

        if (c1 != ',')
            break;

        c1 = skip_space(fp);
    }
    if (c1 != '}')
        return 0;

    if ((tags & (MESH_JSON_CONFIG_NET_XMIT_TAG_COUNT | MESH_JSON_CONFIG_NET_XMIT_TAG_INTERVAL)) != (MESH_JSON_CONFIG_NET_XMIT_TAG_COUNT | MESH_JSON_CONFIG_NET_XMIT_TAG_INTERVAL))
        return 0;

    return 1;
}

int mesh_json_read_relay_rexmit(FILE *fp, char prefix, wiced_bt_mesh_db_transmit_t *relay_rexmit)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    uint32_t tags = 0;

    if (prefix != '{')
        return 0;

    c1 = skip_space(fp);
    if (c1 != '\"')
        return 0;

    tags = 0;

    while (1)
    {
        if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
            break;

        c1 = skip_space(fp);
        if (c1 != ':')
            return 0;

        c1 = skip_space(fp);

        if (strcmp(tagname, "count") == 0)
        {
            if (!mesh_json_read_uint16(fp, c1, &relay_rexmit->count))
                return 0;
            tags |= MESH_JSON_CONFIG_NET_XMIT_TAG_COUNT;
        }
        else if (strcmp(tagname, "interval") == 0)
        {
            if (!mesh_json_read_uint32(fp, c1, &relay_rexmit->interval))
                return 0;
            tags |= MESH_JSON_CONFIG_NET_XMIT_TAG_INTERVAL;
        }
        else
        {
            // unknown tag
            if (!mesh_json_parse_skip_value(fp, c1))
                return 0;
        }
        c1 = skip_space(fp);

        if (c1 != ',')
            break;

        c1 = skip_space(fp);
    }
    if (c1 != '}')
        return 0;

    if ((tags & (MESH_JSON_CONFIG_NET_XMIT_TAG_COUNT | MESH_JSON_CONFIG_NET_XMIT_TAG_INTERVAL)) != (MESH_JSON_CONFIG_NET_XMIT_TAG_COUNT | MESH_JSON_CONFIG_NET_XMIT_TAG_INTERVAL))
        return 0;

    return 1;
}

int mesh_json_read_features(FILE *fp, char prefix, wiced_bt_mesh_db_node_t *node)
{
    char tagname[MAX_TAG_NAME];
    char c1;

    if (prefix != '{')
        return 0;

    c1 = skip_space(fp);
    if (c1 != '\"')
        return 0;

    while (1)
    {
        if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
            break;

        c1 = skip_space(fp);
        if (c1 != ':')
            return 0;

        c1 = skip_space(fp);

        if (strcmp(tagname, "relay") == 0)
        {
            if ((!mesh_json_read_uint8(fp, c1, &node->feature.relay)) ||
                ((node->feature.relay != MESH_FEATURE_DISABLED) && (node->feature.relay != MESH_FEATURE_ENABLED) && (node->feature.relay != MESH_FEATURE_UNSUPPORTED)))
                return 0;
        }
        else if (strcmp(tagname, "proxy") == 0)
        {
            if ((!mesh_json_read_uint8(fp, c1, &node->feature.gatt_proxy)) ||
                ((node->feature.gatt_proxy != MESH_FEATURE_DISABLED) && (node->feature.gatt_proxy != MESH_FEATURE_ENABLED) && (node->feature.gatt_proxy != MESH_FEATURE_UNSUPPORTED)))
                return 0;
        }
        else if (strcmp(tagname, "friend") == 0)
        {
            if ((!mesh_json_read_uint8(fp, c1, &node->feature.friend)) ||
                ((node->feature.friend != MESH_FEATURE_DISABLED) && (node->feature.friend != MESH_FEATURE_ENABLED) && (node->feature.friend != MESH_FEATURE_UNSUPPORTED)))
                return 0;
        }
        else if (strcmp(tagname, "lowPower") == 0)
        {
            if ((!mesh_json_read_uint8(fp, c1, &node->feature.low_power)) ||
                ((node->feature.low_power != MESH_FEATURE_DISABLED) && (node->feature.low_power != MESH_FEATURE_ENABLED) && (node->feature.low_power != MESH_FEATURE_UNSUPPORTED)))
                return 0;
        }
        else
        {
            // unknown tag
            if (!mesh_json_parse_skip_value(fp, c1))
                return 0;
        }
        c1 = skip_space(fp);

        if (c1 != ',')
            break;

        c1 = skip_space(fp);
    }
    if (c1 != '}')
        return 0;

    return 1;
}

int mesh_json_read_models(FILE *fp, char prefix, wiced_bt_mesh_db_element_t *element)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    wiced_bt_mesh_db_model_t model;
    uint32_t tags = 0;
    wiced_bt_mesh_db_model_t *p_temp;

    if (prefix != '[')
        return 0;

    while (1)
    {
        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != '{')
            return 0;

        c1 = skip_space(fp);
        if (c1 != '\"')
            return 0;

        memset(&model, 0, sizeof(wiced_bt_mesh_db_model_t));
        tags = 0;

        while (1)
        {
            if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
                break;

            c1 = skip_space(fp);
            if (c1 != ':')
                return 0;

            c1 = skip_space(fp);

            if (strcmp(tagname, "modelId") == 0)
            {
                if (!mesh_json_read_model_id(fp, c1, &model.model))
                    return 0;
                tags |= MESH_JSON_MODEL_TAG_MODEL_ID;
            }
            else if (strcmp(tagname, "subscribe") == 0)
            {
                if (!mesh_json_read_model_subscribe(fp, c1, &model))
                    return 0;
                tags |= MESH_JSON_MODEL_TAG_SUBSCRIBE;
            }
            else if (strcmp(tagname, "publish") == 0)
            {
                if (!mesh_json_read_model_publish(fp, c1, &model.pub))
                    return 0;
                tags |= MESH_JSON_MODEL_TAG_PUBLISH;
            }
            else if (strcmp(tagname, "bind") == 0)
            {
                if (!mesh_json_read_model_bind(fp, c1, &model))
                    return 0;
                tags |= MESH_JSON_MODEL_TAG_BIND;
            }
            else if (strcmp(tagname, "sensor") == 0)
            {
                if (!mesh_json_read_model_sensors(fp, c1, &model))
                    return 0;
                tags |= MESH_JSON_MODEL_TAG_SENSOR;
            }
            else
            {
                // unknown tag
                if (!mesh_json_parse_skip_value(fp, c1))
                    return 0;
            }
            c1 = skip_space(fp);

            if (c1 != ',')
                break;

            c1 = skip_space(fp);
        }
        if (c1 != '}')
            return 0;

        // All the fields are retrieved. Verify that all mandatory fields are present
        if ((tags & MESH_JSON_TAG_MODEL_MANDATORY) != MESH_JSON_TAG_MODEL_MANDATORY)
            return 0;

        p_temp = element->model;
        element->model = (wiced_bt_mesh_db_model_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_model_t) * (element->num_models + 1));
        if (element->model == NULL)
            return 0;
        if (p_temp != 0)
        {
            memcpy(element->model, p_temp, sizeof(wiced_bt_mesh_db_model_t) * element->num_models);
            wiced_bt_free_buffer(p_temp);
        }
        memcpy(&element->model[element->num_models], &model, sizeof(wiced_bt_mesh_db_model_t));
        element->num_models++;

        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != ',')
            return 0;
    }
    return 1;
}

int mesh_json_read_model_bind(FILE *fp, char prefix, wiced_bt_mesh_db_model_t *model)
{
    char c1;
    uint16_t key_idx;

    if (prefix != '[')
        return 0;

    while (1)
    {
        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (!mesh_json_read_uint16(fp, c1, &key_idx))
            return 0;

        mesh_db_add_model_app_bind(model, key_idx);

        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != ',')
            return 0;
    }
    return 1;
}

int mesh_json_read_model_subscribe(FILE *fp, char prefix, wiced_bt_mesh_db_model_t *model)
{
    char c1;
    uint16_t addr;

    if (prefix != '[')
        return 0;

    while (1)
    {
        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (!mesh_json_read_sub_addr(fp, c1, &addr))
            return 0;

        mesh_db_add_model_sub(model, addr);

        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != ',')
            return 0;
    }
    return 1;
}

int mesh_json_read_cadence(FILE *fp, char prefix, wiced_bt_mesh_db_cadence_t *cadence)
{
    char c1;
    char tagname[MAX_TAG_NAME];
    uint32_t tags = 0;

    if (prefix != '{')
        return 0;

    c1 = skip_space(fp);
    if (c1 == '}')
        return 1;

    while (1)
    {
        if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
            break;

        c1 = skip_space(fp);
        if (c1 != ':')
            return 0;

        c1 = skip_space(fp);

        if (strcmp(tagname, "fastCadencePeriodDivisor") == 0)
        {
            if (!mesh_json_read_uint16(fp, c1, &cadence->fast_cadence_period_divisor))
                return 0;
            tags |= MESH_JSON_SENSOR_TAG_CADENCE_FAST_CAD_PERIOD_DEV;
        }
        else if (strcmp(tagname, "triggerType") == 0)
        {
            if (!mesh_json_read_boolean(fp, c1, &cadence->trigger_type))
                return 0;
            tags |= MESH_JSON_SENSOR_TAG_CADENCE_TRIGGER_TYPE;
        }
        else if (strcmp(tagname, "triggerDeltaDown") == 0)
        {
            if (!mesh_json_read_uint32(fp, c1, &cadence->trigger_delta_down))
                return 0;
            tags |= MESH_JSON_SENSOR_TAG_CADENCE_DELTA_DOWN;
        }
        else if (strcmp(tagname, "triggerDeltaUp") == 0)
        {
            if (!mesh_json_read_uint32(fp, c1, &cadence->trigger_delta_up))
                return 0;
            tags |= MESH_JSON_SENSOR_TAG_CADENCE_DELTA_UP;
        }
        else if (strcmp(tagname, "minInterval") == 0)
        {
            if (!mesh_json_read_uint32(fp, c1, &cadence->min_interval))
                return 0;
            tags |= MESH_JSON_SENSOR_TAG_CADENCE_MIN_INTERVAL;
        }
        else if (strcmp(tagname, "fastCadenceLow") == 0)
        {
            if (!mesh_json_read_uint32(fp, c1, &cadence->fast_cadence_low))
                return 0;
            tags |= MESH_JSON_SENSOR_TAG_CADENCE_FST_CAD_LOW;
        }
        else if (strcmp(tagname, "fastCadenceHigh") == 0)
        {
            if (!mesh_json_read_uint32(fp, c1, &cadence->fast_cadence_high))
                return 0;
            tags |= MESH_JSON_SENSOR_TAG_CADENCE_FST_CAD_HIGH;
        }
        else
        {
            // unknown tag
            if (!mesh_json_parse_skip_value(fp, c1))
                return 0;
        }
        c1 = skip_space(fp);

        if (c1 != ',')
            break;

        c1 = skip_space(fp);
    }
    if (c1 != '}')
        return 0;

    // All the fields are retrieved. Verify that all mandatory fields are present.
    if ((tags & MESH_JSON_TAG_SENSOR_CAD_MANDATORY) != MESH_JSON_TAG_SENSOR_CAD_MANDATORY)
        return 0;

    return 1;
}

int mesh_json_read_setting(FILE *fp, char prefix, wiced_bt_mesh_db_setting_t *setting)
{
    char c1;
    char tagname[MAX_TAG_NAME];
    uint32_t tags = 0;
    int len;

    if (prefix != '{')
        return 0;

    c1 = skip_space(fp);
    if (c1 == '}')
        return 1;

    while (1)
    {
        if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
            break;

        c1 = skip_space(fp);
        if (c1 != ':')
            return 0;

        c1 = skip_space(fp);

        if (strcmp(tagname, "settingPropertyId") == 0)
        {
            if (!mesh_json_read_hex16(fp, c1, &setting->setting_property_id))
                return 0;
            tags |= MESH_JSON_SENSOR_TAG_SETTING_PROP_ID;
        }
        else if (strcmp(tagname, "access") == 0)
        {
            if (!mesh_json_read_uint8(fp, c1, &setting->access))
                return 0;
            tags |= MESH_JSON_SENSOR_TAG_SETTING_ACCESS;
        }
        else if (strcmp(tagname, "value") == 0)
        {
            len = wiced_bt_mesh_property_len[setting->setting_property_id];
            if (len == 0)
                setting->val = NULL;
            else
            {
                setting->val = (uint8_t *)wiced_bt_get_buffer(len);
                if (setting->val == NULL)
                    return  0;
                if (!mesh_json_read_hex_array(fp, c1, setting->val))
                    return 0;
            }
            tags |= MESH_JSON_SENSOR_TAG_SETTING_VAL;
        }
        else
        {
            // unknown tag
            if (!mesh_json_parse_skip_value(fp, c1))
                return 0;
        }
        c1 = skip_space(fp);

        if (c1 != ',')
            break;

        c1 = skip_space(fp);
    }

    if (c1 != '}')
        return 0;

    // All the fields are retrieved. Verify that all mandatory fields are present.
    if ((tags & MESH_JSON_TAG_SENSOR_SETTING_MANDATORY) != MESH_JSON_TAG_SENSOR_SETTING_MANDATORY)
    {
        return 0;
    }

    return 1;
}

int mesh_json_read_settings(FILE *fp, char prefix, wiced_bt_mesh_db_sensor_t *sensor)
{
    char c1;
    wiced_bt_mesh_db_setting_t setting;
    wiced_bt_mesh_db_setting_t* p_temp;
    sensor->num_settings = 0;

    if (prefix != '[')
        return 0;

    while (1)
    {
        c1 = skip_space(fp);

        if (c1 == ']')
            return 1;

        memset(&setting,0, sizeof(wiced_bt_mesh_db_setting_t));
        if (mesh_json_read_setting(fp, c1, &setting) == 0)
            break;

        p_temp = sensor->settings;
        sensor->settings = (wiced_bt_mesh_db_setting_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_setting_t) * (sensor->num_settings + 1));
        if (sensor->settings == NULL)
            return 0;
        if (p_temp != 0)
        {
            memcpy(sensor->settings, p_temp, sizeof(wiced_bt_mesh_db_setting_t) * sensor->num_settings);
            wiced_bt_free_buffer(p_temp);
        }
        memcpy(&sensor->settings[sensor->num_settings], &setting, sizeof(wiced_bt_mesh_db_setting_t));
        sensor->num_settings++;

        c1 = skip_space(fp);
        if (c1 == ']')
        {
            return 1;
        }

        if (c1 != ',')
        {
            return 0;
        }

    }
    return 1;
}


int mesh_json_read_model_sensors(FILE *fp, char prefix, wiced_bt_mesh_db_model_t *model)
{
    char c1;
    wiced_bt_mesh_db_sensor_t sensor;
    wiced_bt_mesh_db_sensor_t *p_temp;
    char tagname[MAX_TAG_NAME];
    uint32_t tags = 0;

    model->num_sensors = 0;

    if (prefix != '[')
        return 0;

    while (1)
    {
        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != '{')
            return 0;

        c1 = skip_space(fp);
        if (c1 != '\"')
            return 0;

        memset(&sensor, 0, sizeof(wiced_bt_mesh_db_sensor_t));
        tags = 0;

        while (1)
        {
            if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
                break;

            c1 = skip_space(fp);
            if (c1 != ':')
                return 0;

            c1 = skip_space(fp);

            if (strcmp(tagname, "propertyId") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &sensor.property_id))
                    return 0;
                tags |= MESH_JSON_SENSOR_TAG_PROP_ID;
            }
            else if (strcmp(tagname, "propertyValueLen") == 0)
            {
                if (!mesh_json_read_uint8(fp, c1, &sensor.prop_value_len))
                    return 0;
                tags |= MESH_JSON_SENSOR_TAG_PROP_VAL_LEN;
            }
            else if (strcmp(tagname, "positiveTolerance") == 0)
            {
                if (!mesh_json_read_uint8(fp, c1, &sensor.descriptor.positive_tolerance_percentage))
                    return 0;
                tags |= MESH_JSON_SENSOR_TAG_DESC_POS_TOLERANCE;
            }
            else if (strcmp(tagname, "negativeTolerance") == 0)
            {
                if (!mesh_json_read_uint8(fp, c1, &sensor.descriptor.negative_tolerance_percentage))
                    return 0;
                tags |= MESH_JSON_SENSOR_TAG_DESC_NEG_TOLERANCE;
            }
            else if (strcmp(tagname, "samplingFunction") == 0)
            {
                if (!mesh_json_read_uint8(fp, c1, &sensor.descriptor.sampling_function))
                    return 0;
                if (sensor.descriptor.sampling_function > WICED_BT_MESH_SENSOR_MAX_SAMPLING_FUNCTION)
                    return 0;
                tags |= MESH_JSON_SENSOR_TAG_DESC_SAMPLING_FUNC;
            }
            else if (strcmp(tagname, "measurementPeriod") == 0)
            {
                if (!mesh_json_read_uint8(fp, c1, &sensor.descriptor.measurement_period))
                    return 0;
                tags |= MESH_JSON_SENSOR_TAG_DESC_MEASUREMENT_PERIOD;
            }
            else if (strcmp(tagname, "updateInterval") == 0)
            {
                if (!mesh_json_read_uint8(fp, c1, &sensor.descriptor.update_interval))
                    return 0;
                tags |= MESH_JSON_SENSOR_TAG_DESC_UPDATE_INTERVAL;
            }
            else if (strcmp(tagname, "cadence") == 0)
            {
                if (!mesh_json_read_cadence(fp, c1, &sensor.cadence))
                    return 0;
                sensor.cadence.property_id = sensor.property_id;
            }
            else if (strcmp(tagname, "settings") == 0)
            {
                if(!mesh_json_read_settings(fp, c1, &sensor))
                    return 0;
            }
            else
            {
                // unknown tag
                if (!mesh_json_parse_skip_value(fp, c1))
                    return 0;
            }
            c1 = skip_space(fp);

            if (c1 != ',')
                break;

            c1 = skip_space(fp);
        }
        if (c1 != '}')
            return 0;

        p_temp = model->sensor;
        model->sensor = (wiced_bt_mesh_db_sensor_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_sensor_t) * (model->num_sensors + 1));
        if (model->sensor == NULL)
            return 0;
        if (p_temp != 0)
        {
            memcpy(model->sensor, p_temp, sizeof(wiced_bt_mesh_db_sensor_t) * model->num_sensors);
            wiced_bt_free_buffer(p_temp);
        }
        memcpy(&model->sensor[model->num_sensors], &sensor, sizeof(wiced_bt_mesh_db_sensor_t));
        model->num_sensors++;

        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != ',')
            return 0;
    }
    return 1;
}

int mesh_json_read_model_publish(FILE *fp, char prefix, wiced_bt_mesh_db_publication_t *pub)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    uint32_t tags = 0;

    if (prefix != '{')
        return 0;

    c1 = skip_space(fp);
    if (c1 != '\"')
        return 0;

    tags = 0;

    while (1)
    {
        if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
            break;

        c1 = skip_space(fp);
        if (c1 != ':')
            return 0;

        c1 = skip_space(fp);

        if (strcmp(tagname, "address") == 0)
        {
            if (!mesh_json_read_hex16(fp, c1, &pub->address))
                return 0;
            tags |= MESH_JSON_PUBLISH_TAG_ADDRESS;
        }
        else if (strcmp(tagname, "index") == 0)
        {
            if (!mesh_json_read_uint16(fp, c1, &pub->index))
                return 0;
            tags |= MESH_JSON_PUBLISH_TAG_INDEX;
        }
        else if (strcmp(tagname, "ttl") == 0)
        {
            if (!mesh_json_read_uint8(fp, c1, &pub->ttl))
                return 0;
            tags |= MESH_JSON_PUBLISH_TAG_TTL;
        }
        else if (strcmp(tagname, "period") == 0)
        {
            if (!mesh_json_read_uint32(fp, c1, &pub->period))
                return 0;
            tags |= MESH_JSON_PUBLISH_TAG_PERIOD;
        }
        else if (strcmp(tagname, "retransmit") == 0)
        {
            if (!mesh_json_read_model_pub_rexmit(fp, c1, &pub->retransmit))
                return 0;
            tags |= MESH_JSON_PUBLISH_TAG_REXMIT;
        }
        else if (strcmp(tagname, "credentials") == 0)
        {
            if (!mesh_json_read_uint8(fp, c1, &pub->credentials))
                return 0;
            tags |= MESH_JSON_PUBLISH_TAG_CREDENTIALS;
        }
        else
        {
            // unknown tag
            if (!mesh_json_parse_skip_value(fp, c1))
                return 0;
        }
        c1 = skip_space(fp);

        if (c1 != ',')
            break;

        c1 = skip_space(fp);
    }
    if (c1 != '}')
        return 0;

    // All the fields are retrieved. Verify that all mandatory fields are present.
    if ((tags & MESH_JSON_TAG_PUBLISH_MANDATORY) != MESH_JSON_TAG_PUBLISH_MANDATORY)
        return 0;

    return 1;
}

int mesh_json_read_model_pub_rexmit(FILE *fp, char prefix, wiced_bt_mesh_db_transmit_t *relay_rexmit)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    uint32_t tags = 0;

    if (prefix != '{')
        return 0;

    c1 = skip_space(fp);
    if (c1 != '\"')
        return 0;

    tags = 0;

    while (1)
    {
        if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
            break;

        c1 = skip_space(fp);
        if (c1 != ':')
            return 0;

        c1 = skip_space(fp);

        if (strcmp(tagname, "count") == 0)
        {
            if (!mesh_json_read_uint16(fp, c1, &relay_rexmit->count))
                return 0;
            tags |= MESH_JSON_PUBLISH_TAG_REXMIT_COUNT;
        }
        else if (strcmp(tagname, "interval") == 0)
        {
            if (!mesh_json_read_uint32(fp, c1, &relay_rexmit->interval))
                return 0;
            tags |= MESH_JSON_PUBLISH_TAG_REXMIT_INTERVAL;
        }
        else
        {
            // unknown tag
            if (!mesh_json_parse_skip_value(fp, c1))
                return 0;
        }
        c1 = skip_space(fp);

        if (c1 != ',')
            break;

        c1 = skip_space(fp);
    }
    if (c1 != '}')
        return 0;

    if ((tags & (MESH_JSON_PUBLISH_TAG_REXMIT_COUNT | MESH_JSON_PUBLISH_TAG_REXMIT_INTERVAL)) != (MESH_JSON_PUBLISH_TAG_REXMIT_COUNT | MESH_JSON_PUBLISH_TAG_REXMIT_INTERVAL))
        return 0;

    return 1;
}


// Skip space charcters and return first non space one.
char skip_space(FILE *fp)
{
    int c = 0;
    while (fread(&c, 1, 1, fp) > 0)
    {
        if (!isspace(c))
            return c;
    }
    return 0;
}

// Skip ',' charcters and return first non space one.
char skip_comma(FILE *fp)
{
    int c = 0;
    while (fread(&c, 1, 1, fp) > 0)
    {
        if (isspace(c))
            continue;

        if (c == ',')
        {
            while (fread(&c, 1, 1, fp) > 0)
            {
                if (!isspace(c))
                    return c;

            }
        }
        break;
    }
    return 0;
}

uint8_t process_nibble(char n)
{
    if ((n >= '0') && (n <= '9'))
    {
        n -= '0';
    }
    else if ((n >= 'A') && (n <= 'F'))
    {
        n = ((n - 'A') + 10);
    }
    else if ((n >= 'a') && (n <= 'f'))
    {
        n = ((n - 'a') + 10);
    }
    else
    {
        n = (char)0xff;
    }
    return (n);
}

static char *mesh_header = "{\r  \"$schema\":\"file:///Users/robin/Mesh/json/provisioning_database/mesh.jsonschema\",\r";
static char *mesh_footer = "}\r";
static char *hex_digits = "0123456789ABCDEF";

void mesh_json_write_hex128(FILE *fp, int offset, char *tag, uint8_t *uuid, int is_last)
{
    char buffer[80];
    char *p = buffer;
    int i;

    for (i = 0; i < offset; i++)
        *p++ = ' ';
    *p++ = '\"';
    for (i = 0; tag[i] != 0; i++)
        *p++ = tag[i];
    *p++ = '\"';
    *p++ = ':';
    *p++ = '\"';
    for (i = 0; i < 16; i++)
    {
        *p++ = hex_digits[uuid[i] >> 4];
        *p++ = hex_digits[uuid[i] & 0x0f];
    }
    *p++ = '\"';
    if (!is_last)
        *p++ = ',';
    *p++ = '\n';
    *p++ = 0;
    fputs(buffer, fp);
};

void mesh_json_write_hex_byte_array(FILE *fp, int offset, char *tag, uint8_t *uuid, uint8_t len, int is_last)
{
    char buffer[80];
    char *p = buffer;
    int i;

    for (i = 0; i < offset; i++)
        *p++ = ' ';
    *p++ = '\"';
    for (i = 0; tag[i] != 0; i++)
        *p++ = tag[i];
    *p++ = '\"';
    *p++ = ':';
    *p++ = '\"';
    for (i = 0; i < len; i++)
    {
        *p++ = hex_digits[uuid[i] >> 4];
        *p++ = hex_digits[uuid[i] & 0x0f];
    }
    *p++ = '\"';
    if (!is_last)
        *p++ = ',';
    *p++ = '\n';
    *p++ = 0;
    fputs(buffer, fp);
};

void mesh_json_write_string(FILE *fp, int offset, char *tag, char *value, int is_last)
{
    char buffer[256];
    char *p = buffer;
    int i;

    for (i = 0; i < offset; i++)
        *p++ = ' ';
    *p++ = '\"';
    for (i = 0; tag[i] != 0; i++)
        *p++ = tag[i];
    *p++ = '\"';
    *p++ = ':';
    *p++ = '\"';
    for (i = 0; value[i] != 0; i++)
        *p++ = value[i];
    *p++ = '\"';
    if (!is_last)
        *p++ = ',';
    *p++ = '\n';
    *p++ = 0;
    fputs(buffer, fp);
};

void mesh_json_write_int(FILE *fp, int offset, char *tag, uint16_t value, int is_last)
{
    char buffer[80];
    int i;

    for (i = 0; i < offset; i++)
        buffer[i] = ' ';
    if (tag != NULL)
        sprintf(&buffer[offset], "\"%s\":", tag);
    else
        buffer[offset] = 0;
    sprintf(&buffer[strlen(buffer)], "%d", value);
    if (!is_last)
        strcat(buffer, ",\n");
    else
        strcat(buffer, "\n");
    fputs(buffer, fp);
};

void mesh_json_write_boolean(FILE *fp, int offset, char *tag, wiced_bool_t value, int is_last)
{
    char buffer[80];
    int i;

    for (i = 0; i < offset; i++)
        buffer[i] = ' ';
    if (tag != NULL)
        sprintf(&buffer[offset], "\"%s\":", tag);
    else
        buffer[offset] = 0;
    sprintf(&buffer[strlen(buffer)], "%s", value ? "true" : "false");
    if (!is_last)
        strcat(buffer, ",\n");
    else
        strcat(buffer, "\n");
    fputs(buffer, fp);
};

void mesh_json_write_hex16(FILE *fp, int offset, char *tag, uint16_t value, int is_last)
{
    char buffer[80];
    int i;

    for (i = 0; i < offset; i++)
        buffer[i] = ' ';
    if (tag != NULL)
        sprintf(&buffer[offset], "\"%s\":", tag);
    else
        buffer[offset] = 0;
    sprintf(&buffer[strlen(buffer)], "\"%04x\"", value);
    if (!is_last)
        strcat(buffer, ",\n");
    else
        strcat(buffer, "\n");
    fputs(buffer, fp);
};

void mesh_json_write_hex32(FILE *fp, int offset, char *tag, uint32_t value, int is_last)
{
    char buffer[80] = { 0 };
    int i;

    for (i = 0; i < offset; i++)
        buffer[i] = ' ';
    if (tag != NULL)
        sprintf(&buffer[offset], "\"%s\":", tag);
    sprintf(&buffer[strlen(buffer)], "\"%08x\"", value);
    if (!is_last)
        strcat(buffer, ",\n");
    else
        strcat(buffer, "\n");
    fputs(buffer, fp);
};

void mesh_json_write_netkey(FILE *fp, wiced_bt_mesh_db_net_key_t *net_key, int is_last)
{
    fputs("    {\n", fp);

    mesh_json_write_string(fp, 6, "name", net_key->name, 0);
    mesh_json_write_int(fp, 6, "index", net_key->index, 0);
    mesh_json_write_hex128(fp, 6, "key", net_key->key, 0);
    mesh_json_write_string(fp, 6, "minSecurity", net_key->min_security == MIN_SECURITY_HIGH ? "high" : "low", 0);
    mesh_json_write_int(fp, 6, "phase", net_key->phase, (net_key->phase == 0));
    if (net_key->phase)
        mesh_json_write_hex128(fp, 6, "oldKey", net_key->old_key, 1);

    if (is_last)
        fputs("    }\n", fp);
    else
        fputs("    },\n", fp);
}

void mesh_json_write_appkey(FILE *fp, wiced_bt_mesh_db_app_key_t *app_key, wiced_bool_t key_refresh_inprogress, int is_last)
{
    fputs("    {\n", fp);

    mesh_json_write_string(fp, 6, "name", app_key->name, 0);
    mesh_json_write_int(fp, 6, "index", app_key->index, 0);
    mesh_json_write_hex128(fp, 6, "key", app_key->key, 0);
    mesh_json_write_int(fp, 6, "boundNetKey", app_key->bound_net_key_index, !key_refresh_inprogress ? 1 : 0);
    if (key_refresh_inprogress)
        mesh_json_write_hex128(fp, 6, "oldKey", app_key->old_key, 1);

    if (is_last)
        fputs("    }\n", fp);
    else
        fputs("    },\n", fp);
}

void mesh_json_write_range(FILE *fp, int offset, wiced_bt_mesh_db_range_t *range, int is_last)
{
    fputs("        {\n", fp);

    mesh_json_write_hex16(fp, 10, "highAddress", range->high_addr, 0);
    mesh_json_write_hex16(fp, 10, "lowAddress", range->low_addr, 1);

    if (is_last)
        fputs("        }\n", fp);
    else
        fputs("        },\n", fp);
}

void mesh_json_write_provisioner(FILE *fp, wiced_bt_mesh_db_provisioner_t *provisioner, int is_last)
{
    int i;
    fputs("    {\n", fp);

    mesh_json_write_string(fp, 6, "provisionerName", provisioner->name, 0);
    mesh_json_write_hex128(fp, 6, "UUID", provisioner->uuid, 0);

    fputs("      \"allocatedGroupRange\":[\n", fp);
    for (i = 0; i < provisioner->num_allocated_group_ranges; i++)
    {
        mesh_json_write_range(fp, 6, &provisioner->p_allocated_group_range[i], i == provisioner->num_allocated_group_ranges - 1);
    }
    fputs("      ],\n", fp);
    fputs("      \"allocatedUnicastRange\":[\n", fp);
    for (i = 0; i < provisioner->num_allocated_unicast_ranges; i++)
    {
        mesh_json_write_range(fp, 6, &provisioner->p_allocated_unicast_range[i], i == provisioner->num_allocated_unicast_ranges - 1);
    }
    fputs("      ]\n", fp);

    if (is_last)
        fputs("    }\n", fp);
    else
        fputs("    },\n", fp);
}

void mesh_json_write_setting(FILE *fp, wiced_bt_mesh_db_setting_t *setting, int value_len, int is_last)
{
    fputs("                    {\n", fp);
    mesh_json_write_hex16(fp, 22, "settingPropertyId", setting->setting_property_id, 0);
    mesh_json_write_int(fp, 22, "access", setting->access, (value_len == 0) || (setting->val == NULL));
    if ((value_len != 0) && (setting->val != NULL))
        mesh_json_write_hex_byte_array(fp, 22, "value", setting->val, value_len, 1);
    if (is_last)
        fputs("                    }\n", fp);
    else
        fputs("                    },\n", fp);
}

void mesh_json_write_sensor(FILE *fp, wiced_bt_mesh_db_sensor_t *sensor, int is_last)
{

    int i;
    int len;
    fputs("                {\n", fp);
    mesh_json_write_hex16(fp, 18, "propertyId", sensor->property_id, 0);
    mesh_json_write_int(fp, 18, "propertyValueLen", sensor->prop_value_len, 0);
    mesh_json_write_int(fp, 18, "positiveTolerance", sensor->descriptor.positive_tolerance_percentage, 0);
    mesh_json_write_int(fp, 18, "negativeTolerance", sensor->descriptor.negative_tolerance_percentage, 0);
    mesh_json_write_int(fp, 18, "samplingFunction", sensor->descriptor.sampling_function, 0);
    mesh_json_write_int(fp, 18, "measurementPeriod", sensor->descriptor.measurement_period, 0);
    mesh_json_write_int(fp, 18, "updateInterval", sensor->descriptor.update_interval, ((sensor->cadence.property_id == 0) && (sensor->num_settings == 0)));

    //cadence values
    if(sensor->cadence.property_id != 0)
    {
        fputs("              \"cadence\":{\n", fp);
        mesh_json_write_int(fp, 18, "fastCadencePeriodDivisor", sensor->cadence.fast_cadence_period_divisor, 0);
        mesh_json_write_boolean(fp, 18, "triggerType", sensor->cadence.trigger_type, 0);
        mesh_json_write_int(fp, 18, "triggerDeltaDown", sensor->cadence.trigger_delta_down, 0);
        mesh_json_write_int(fp, 18, "triggerDeltaUp",  sensor->cadence.trigger_delta_up, 0);
        mesh_json_write_int(fp, 18, "minInterval", sensor->cadence.min_interval, 0);
        mesh_json_write_int(fp, 18, "fastCadenceLow", sensor->cadence.fast_cadence_low, 0);
        mesh_json_write_int(fp, 18, "fastCadenceHigh",  sensor->cadence.fast_cadence_high, 1);
        if (sensor->num_settings != 0)
            fputs("                  },\n", fp);
        else
            fputs("                  }\n", fp);
    }

    //settings values
    if (sensor->num_settings != 0)
    {
        fputs("                  \"settings\":[\n", fp);
        for (i = 0; i < sensor->num_settings; i++)
        {
            len = wiced_bt_mesh_property_len[sensor->settings[i].setting_property_id];
            mesh_json_write_setting(fp, &sensor->settings[i], len, i == sensor->num_settings - 1);
        }
        fputs("                  ]\n", fp);
    }

    if (is_last)
        fputs("                }\n", fp);
    else
        fputs("                },\n", fp);
}

void mesh_json_write_model(FILE *fp, wiced_bt_mesh_db_model_t *model, int is_last)
{
    int i;
    fputs("            {\n", fp);

    if (model->model.company_id == MESH_COMPANY_ID_BT_SIG)
        mesh_json_write_hex16(fp, 14, "modelId", model->model.id, (model->num_bound_keys == 0) && (model->num_subs == 0));
    else
        mesh_json_write_hex32(fp, 14, "modelId", MODEL_ID_TO_UIN32(model->model), (model->num_bound_keys == 0) && (model->num_subs == 0));
    if (model->num_subs)
    {
        fputs("              \"subscribe\":[\n", fp);

        for (i = 0; i < model->num_subs; i++)
        {
            mesh_json_write_hex16(fp, 16, NULL, (uint16_t)model->sub[i], i == model->num_subs - 1);
        }
        if ((model->num_bound_keys != 0) || (model->pub.address != 0))
            fputs("              ],\n", fp);
        else
            fputs("              ]\n", fp);
    }
    if (model->num_bound_keys)
    {
        fputs("              \"bind\":[\n", fp);

        for (i = 0; i < model->num_bound_keys; i++)
        {
            mesh_json_write_int(fp, 16, NULL, model->bound_key[i], i == model->num_bound_keys - 1);
        }
        if (model->pub.address != 0 || model->num_sensors != 0)
            fputs("              ],\n", fp);
        else
            fputs("              ]\n", fp);
    }
    if (model->pub.address != 0)
    {
        fputs("              \"publish\":{\n", fp);
        mesh_json_write_hex16(fp, 16, "address", model->pub.address, 0);
        mesh_json_write_int(fp, 16, "index", model->pub.index, 0);
        mesh_json_write_int(fp, 16, "ttl", model->pub.ttl, 0);
        mesh_json_write_int(fp, 16, "period", model->pub.period, 0);
        fputs("                \"retransmit\":{\n", fp);
        mesh_json_write_int(fp, 18, "count", model->pub.retransmit.count, 0);
        mesh_json_write_int(fp, 18, "interval", model->pub.retransmit.interval, 1);
        fputs("                },\n", fp);
        mesh_json_write_int(fp, 16, "credentials", model->pub.credentials, 1);
        if (model->num_sensors == 0)
            fputs("              }\n", fp);
        else
            fputs("              },\n", fp);
    }
    if (model->num_sensors != 0)
    {
        fputs("              \"sensor\":[\n", fp);
        for (i = 0; i < model->num_sensors; i++)
        {
            mesh_json_write_sensor(fp, &model->sensor[i], i == model->num_sensors - 1);
        }
        fputs("              ]\n", fp);
    }

    if (is_last)
        fputs("            }\n", fp);
    else
        fputs("            },\n", fp);
}

void mesh_json_write_element(FILE *fp, wiced_bt_mesh_db_element_t *element, int is_last)
{
    int i;
    fputs("        {\n", fp);

    mesh_json_write_int(fp, 10, "index", element->index, 0);
    mesh_json_write_hex16(fp, 10, "location", element->location, 0);

    fputs("          \"models\":[\n", fp);
    for (i = 0; i < element->num_models; i++)
    {
        mesh_json_write_model(fp, &element->model[i], i == element->num_models - 1);
    }
    if (element->name == NULL)
        fputs("          ]\n", fp);
    else
    {
        fputs("          ],\n", fp);

        mesh_json_write_string(fp, 10, "name", element->name, 1);
    }
    if (is_last)
        fputs("        }\n", fp);
    else
        fputs("        },\n", fp);
}

void mesh_json_write_config_key(FILE *fp, wiced_bt_mesh_db_key_idx_phase *p_key, int is_last)
{
    fputs("        {\n", fp);

    mesh_json_write_int(fp, 10, "key", p_key->index, 0);
    mesh_json_write_int(fp, 10, "phase", p_key->phase, 1);

    if (is_last)
        fputs("        }\n", fp);
    else
        fputs("        },\n", fp);
}

void mesh_json_write_node(FILE *fp, wiced_bt_mesh_db_node_t *node, int is_last)
{
    int i;
    wiced_bool_t no_other_featurs;

    fputs("    {\n", fp);

    mesh_json_write_hex128(fp, 6, "UUID", node->uuid, 0);
    mesh_json_write_hex16(fp, 6, "unicastAddress", node->unicast_address, 0);
    mesh_json_write_hex128(fp, 6, "deviceKey", node->device_key, 0);
    mesh_json_write_string(fp, 6, "security", node->security ? "high" : "low", 0);

    fputs("      \"netKeys\":[\n", fp);
    for (i = 0; i < node->num_net_keys; i++)
        mesh_json_write_config_key(fp, &node->net_key[i], i == node->num_net_keys - 1);
    fputs("      ],\n", fp);

    mesh_json_write_boolean(fp, 6, "configComplete", node->config_complete, 0);
    mesh_json_write_string(fp, 6, "name", node->name, 0);

    if ((node->cid != 0) || (node->pid != 0) || (node->vid != 0) || (node->crpl != 0))
    {
        mesh_json_write_hex16(fp, 6, "cid", node->cid, 0);
        mesh_json_write_hex16(fp, 6, "pid", node->pid, 0);
        mesh_json_write_hex16(fp, 6, "vid", node->vid, 0);
        mesh_json_write_hex16(fp, 6, "crpl", node->crpl, 0);
    }
    if ((node->feature.relay != MESH_FEATURE_SUPPORTED_UNKNOWN) ||
        (node->feature.gatt_proxy != MESH_FEATURE_SUPPORTED_UNKNOWN) ||
        (node->feature.low_power != MESH_FEATURE_SUPPORTED_UNKNOWN) ||
        (node->feature.friend != MESH_FEATURE_SUPPORTED_UNKNOWN))
    {
        fputs("      \"features\":{\n", fp);
        if (node->feature.relay != MESH_FEATURE_SUPPORTED_UNKNOWN)
        {
            no_other_featurs =
                (node->feature.gatt_proxy == MESH_FEATURE_SUPPORTED_UNKNOWN) &&
                (node->feature.low_power == MESH_FEATURE_SUPPORTED_UNKNOWN) &&
                (node->feature.friend == MESH_FEATURE_SUPPORTED_UNKNOWN);

            mesh_json_write_int(fp, 8, "relay", node->feature.relay, no_other_featurs);
        }
        if (node->feature.gatt_proxy != MESH_FEATURE_SUPPORTED_UNKNOWN)
        {
            no_other_featurs =
                (node->feature.low_power == MESH_FEATURE_SUPPORTED_UNKNOWN) &&
                (node->feature.friend == MESH_FEATURE_SUPPORTED_UNKNOWN);

            mesh_json_write_int(fp, 8, "proxy", node->feature.gatt_proxy, no_other_featurs);
        }
        if (node->feature.low_power != MESH_FEATURE_SUPPORTED_UNKNOWN)
        {
            no_other_featurs = (node->feature.friend == MESH_FEATURE_SUPPORTED_UNKNOWN);

            mesh_json_write_int(fp, 8, "lowPower", node->feature.low_power, no_other_featurs);
        }
        if (node->feature.friend != MESH_FEATURE_SUPPORTED_UNKNOWN)
        {
            mesh_json_write_int(fp, 8, "friend", node->feature.friend, 1);
        }
        fputs("      },\n", fp);
    }
    if (node->beacon != MESH_FEATURE_SUPPORTED_UNKNOWN)
        mesh_json_write_boolean(fp, 6, "secureNetworkBeacon", node->beacon, 0);

    if (node->default_ttl != MESH_FEATURE_SUPPORTED_UNKNOWN)
        mesh_json_write_int(fp, 6, "defaultTTL", node->default_ttl, 0);

    if (node->network_transmit.count != MESH_FEATURE_SUPPORTED_UNKNOWN)
    {
        fputs("      \"networkTransmit\":{\n", fp);
        mesh_json_write_int(fp, 8, "count", node->network_transmit.count, 0);
        mesh_json_write_int(fp, 8, "interval", node->network_transmit.interval, 1);
        fputs("      },\n", fp);
    }

    if (node->relay_rexmit.count != MESH_FEATURE_SUPPORTED_UNKNOWN)
    {
        fputs("      \"relayRetransmit\":{\n", fp);
        mesh_json_write_int(fp, 8, "count", node->relay_rexmit.count, 0);
        mesh_json_write_int(fp, 8, "interval", node->relay_rexmit.interval, 1);
        fputs("      },\n", fp);
    }

    if (node->num_app_keys != 0)
    {
        fputs("      \"appKeys\":[\n", fp);
        for (i = 0; i < node->num_app_keys; i++)
            mesh_json_write_config_key(fp, &node->app_key[i], i == node->num_app_keys - 1);
        fputs("      ],\n", fp);
    }

    fputs("      \"elements\":[\n", fp);
    for (i = 0; i < node->num_elements; i++)
        mesh_json_write_element(fp, &node->element[i], i == node->num_elements - 1);
    fputs("      ],\n", fp);

    mesh_json_write_boolean(fp, 6, "blacklisted", node->blocked, 1);
    if (is_last)
        fputs("    }\n", fp);
    else
        fputs("    },\n", fp);
}

void mesh_json_write_group(FILE *fp, wiced_bt_mesh_db_group_t *group, int is_last)
{
    fputs("    {\n", fp);

    if (group->name[0] == 0)
    {
        wiced_bt_free_buffer(group->name);
        group->name = (uint8_t *)wiced_bt_get_buffer(11);
        sprintf(group->name, "group_%04x", group->addr);
    }
    mesh_json_write_string(fp, 6, "name", group->name, 0);
    mesh_json_write_hex16(fp, 6, "address", group->addr, 0);
    mesh_json_write_hex16(fp, 6, "parent", group->parent_addr, 1);

    if (is_last)
        fputs("    }\n", fp);
    else
        fputs("    },\n", fp);
}

void mesh_json_write_scene(FILE *fp, wiced_bt_mesh_db_scene_t *scene, int is_last)
{
    fputs("    {\n", fp);

    mesh_json_write_string(fp, 6, "name", scene->name, 0);
    mesh_json_write_hex16(fp, 6, "address", scene->addr, 0);
    mesh_json_write_int(fp, 6, "number", scene->number, 1);

    if (is_last)
        fputs("    }\n", fp);
    else
        fputs("    },\n", fp);
}

void mesh_json_write_file(FILE *fp, wiced_bt_mesh_db_mesh_t *p_mesh)
{
    int i;
    fputs(mesh_header, fp);
    mesh_json_write_hex128(fp, 2, "meshUUID", p_mesh->uuid, 0);
    mesh_json_write_string(fp, 2, "meshName", p_mesh->name, 0);
    fputs("  \"provisioners\":[\n", fp);
    for (i = 0; i < p_mesh->num_provisioners; i++)
        mesh_json_write_provisioner(fp, &p_mesh->provisioner[i], i == p_mesh->num_provisioners - 1);
    fputs("  ],\n", fp);
    fputs("  \"netKeys\":[\n", fp);
    for (i = 0; i < p_mesh->num_net_keys; i++)
        mesh_json_write_netkey(fp, &p_mesh->net_key[i], i == p_mesh->num_net_keys - 1);
    fputs("  ],\n", fp);
    fputs("  \"appKeys\":[\n", fp);
    for (i = 0; i < p_mesh->num_app_keys; i++)
    {
        wiced_bt_mesh_db_net_key_t *net_key = wiced_bt_mesh_db_find_bound_net_key(p_mesh, &p_mesh->app_key[i]);
        if (net_key)
            mesh_json_write_appkey(fp, &p_mesh->app_key[i], net_key->phase != 0, i == p_mesh->num_app_keys - 1);
    }
    fputs("  ],\n", fp);
    fputs("  \"nodes\":[\n", fp);
    for (i = 0; i < p_mesh->num_nodes; i++)
        mesh_json_write_node(fp, &p_mesh->node[i], i == p_mesh->num_nodes - 1);
    fputs("  ],\n", fp);
    fputs("  \"groups\":[\n", fp);
    for (i = 0; i < p_mesh->num_groups; i++)
        mesh_json_write_group(fp, &p_mesh->group[i], i == p_mesh->num_groups - 1);
    fputs("  ],\n", fp);
    fputs("  \"scenes\":[\n", fp);
    for (i = 0; i < p_mesh->num_scenes; i++)
        mesh_json_write_scene(fp, &p_mesh->scene[i], i == p_mesh->num_scenes - 1);
    fputs("  ]\n", fp);
    fwrite(mesh_footer, 1, strlen(mesh_footer), fp);
}
