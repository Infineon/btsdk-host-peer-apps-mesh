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
#define MESH_JSON_TAG_ID                            0x0002
#define MESH_JSON_TAG_VERSION                       0x0004
#define MESH_JSON_TAG_MESH_UUID                     0x0008
#define MESH_JSON_TAG_NAME                          0x0010
#define MESH_JSON_TAG_TIMESTAMP                     0x0020
#define MESH_JSON_TAG_PARTIAL                       0x0040
#define MESH_JSON_TAG_PROVISIONERS                  0x0080
#define MESH_JSON_TAG_NET_KEYS                      0x0100
#define MESH_JSON_TAG_APP_KEYS                      0x0200
#define MESH_JSON_TAG_NODES                         0x0400
#define MESH_JSON_TAG_GROUPS                        0x0800
#define MESH_JSON_TAG_SCENES                        0x1000
#define MESH_JSON_TAG_MANDATORY                     0x1FFF      // all of the above

#define MESH_JSON_KEY_TAG_NAME                      0x0001
#define MESH_JSON_KEY_TAG_KEY_INDEX                 0x0002
#define MESH_JSON_KEY_TAG_PHASE                     0x0004
#define MESH_JSON_KEY_TAG_KEY                       0x0008
#define MESH_JSON_KEY_TAG_MIN_SECURITY              0x0010
#define MESH_JSON_KEY_TAG_OLD_KEY                   0x0020
#define MESH_JSON_KEY_TAG_BOUND_NET_KEY             0x0040
#define MESH_JSON_KEY_TAG_TIMESTAMP                 0x0080
#define MESH_JSON_TAG_NET_KEY_MANDATORY (MESH_JSON_KEY_TAG_NAME | MESH_JSON_KEY_TAG_KEY_INDEX | MESH_JSON_KEY_TAG_PHASE | MESH_JSON_KEY_TAG_KEY | MESH_JSON_KEY_TAG_MIN_SECURITY | MESH_JSON_KEY_TAG_TIMESTAMP)
#define MESH_JSON_TAG_APP_KEY_MANDATORY (MESH_JSON_KEY_TAG_NAME | MESH_JSON_KEY_TAG_KEY_INDEX | MESH_JSON_KEY_TAG_BOUND_NET_KEY | MESH_JSON_KEY_TAG_KEY)

#define MESH_JSON_PROVISIONER_TAG_NAME              0x0001
#define MESH_JSON_PROVISIONER_TAG_UUID              0x0002
#define MESH_JSON_PROVISIONER_TAG_GROUP_RANGE       0x0004
#define MESH_JSON_PROVISIONER_TAG_UNICAST_RANGE     0x0008
#define MESH_JSON_PROVISIONER_TAG_SCENE_RANGE       0x0010

#define MESH_JSON_TAG_PROVISIONER_MANDATORY (MESH_JSON_PROVISIONER_TAG_NAME | MESH_JSON_PROVISIONER_TAG_UUID | MESH_JSON_PROVISIONER_TAG_GROUP_RANGE | MESH_JSON_PROVISIONER_TAG_UNICAST_RANGE | MESH_JSON_PROVISIONER_TAG_SCENE_RANGE)

// Mandatory Tags of the Nodes object
#define MESH_JSON_NODE_TAG_UUID                     0x0001
#define MESH_JSON_NODE_TAG_UNICAST_ADDR             0x0002
#define MESH_JSON_NODE_TAG_DEVICE_KEY               0x0004
#define MESH_JSON_NODE_TAG_SECURITY                 0x0008
#define MESH_JSON_NODE_TAG_NET_KEYS                 0x0010
#define MESH_JSON_NODE_TAG_CONFIG_COMPLETE          0x0020
#define MESH_JSON_NODE_TAG_APP_KEYS                 0x0040
#define MESH_JSON_NODE_TAG_ELEMENTS                 0x0080
#define MESH_JSON_NODE_TAG_EXCLUDED                 0x0100

// Otional Tags of the Nodes object
#define MESH_JSON_NODE_TAG_NAME                     0x00010000
#define MESH_JSON_NODE_TAG_CID                      0x00020000
#define MESH_JSON_NODE_TAG_PID                      0x00040000
#define MESH_JSON_NODE_TAG_VID                      0x00080000
#define MESH_JSON_NODE_TAG_CRPL                     0x00100000
#define MESH_JSON_NODE_TAG_FEATURES                 0x00200000
#define MESH_JSON_NODE_TAG_BEACON                   0x00400000
#define MESH_JSON_NODE_TAG_DEFAULT_TTL              0x00800000
#define MESH_JSON_NODE_TAG_NET_TRANSMIT             0x01000000
#define MESH_JSON_NODE_TAG_RELAY_REXMIT             0x02000000

#define MESH_JSON_TAG_NODE_MANDATORY (MESH_JSON_NODE_TAG_UUID | MESH_JSON_NODE_TAG_UNICAST_ADDR | MESH_JSON_NODE_TAG_DEVICE_KEY | MESH_JSON_NODE_TAG_SECURITY | MESH_JSON_NODE_TAG_NET_KEYS | MESH_JSON_NODE_TAG_CONFIG_COMPLETE | MESH_JSON_NODE_TAG_APP_KEYS | MESH_JSON_NODE_TAG_ELEMENTS | MESH_JSON_NODE_TAG_EXCLUDED)

#define MESH_JSON_ELEMENT_TAG_NAME                  0x0001
#define MESH_JSON_ELEMENT_TAG_INDEX                 0x0002
#define MESH_JSON_ELEMENT_TAG_LOCATION              0x0004
#define MESH_JSON_ELEMENT_TAG_MODELS                0x0008

#define MESH_JSON_TAG_ELEMENT_MANDATORY (MESH_JSON_ELEMENT_TAG_INDEX | MESH_JSON_ELEMENT_TAG_LOCATION | MESH_JSON_ELEMENT_TAG_MODELS)

#define MESH_JSON_MODEL_TAG_MODEL_ID                0x0001
#define MESH_JSON_MODEL_TAG_SUBSCRIBE               0x0002
#define MESH_JSON_MODEL_TAG_PUBLISH                 0x0004
#define MESH_JSON_MODEL_TAG_BIND                    0x0008

#define MESH_JSON_TAG_MODEL_MANDATORY (MESH_JSON_MODEL_TAG_MODEL_ID | MESH_JSON_MODEL_TAG_SUBSCRIBE | MESH_JSON_MODEL_TAG_BIND)

#define MESH_JSON_CONFIG_KEY_TAG_INDEX              0x0001
#define MESH_JSON_CONFIG_KEY_TAG_UPDATED            0x0002

#define MESH_JSON_TAG_CONFIG_KEY_MANDATORY (MESH_JSON_CONFIG_KEY_TAG_INDEX | MESH_JSON_CONFIG_KEY_TAG_UPDATED)

#define MESH_JSON_CONFIG_NET_XMIT_TAG_COUNT         0x0001
#define MESH_JSON_CONFIG_NET_XMIT_TAG_INTERVAL      0x0002

#define MESH_JSON_TAG_NET_XMIT_MANDATORY (MESH_JSON_CONFIG_NET_XMIT_TAG_COUNT | MESH_JSON_CONFIG_NET_XMIT_TAG_INTERVAL)

#define MESH_JSON_PUBLISH_TAG_ADDRESS               0x0001
#define MESH_JSON_PUBLISH_TAG_INDEX                 0x0002
#define MESH_JSON_PUBLISH_TAG_TTL                   0x0004
#define MESH_JSON_PUBLISH_TAG_PERIOD                0x0008
#define MESH_JSON_PUBLISH_TAG_REXMIT                0x0010
#define MESH_JSON_PUBLISH_TAG_CREDENTIALS           0x0020

#define MESH_JSON_TAG_PUBLISH_MANDATORY (MESH_JSON_PUBLISH_TAG_ADDRESS | MESH_JSON_PUBLISH_TAG_INDEX | MESH_JSON_PUBLISH_TAG_TTL | MESH_JSON_PUBLISH_TAG_PERIOD | MESH_JSON_PUBLISH_TAG_REXMIT | MESH_JSON_PUBLISH_TAG_CREDENTIALS)

#define MESH_JSON_PUBLISH_TAG_PERIOD_STEPS          0x0001
#define MESH_JSON_PUBLISH_TAG_PERIOD_RESOLUTION     0x0002

#define MESH_JSON_TAG_PERIOD_MANDATORY (MESH_JSON_PUBLISH_TAG_PERIOD_STEPS | MESH_JSON_PUBLISH_TAG_PERIOD_RESOLUTION)

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

#define MESH_JSON_TAG_GROUP_MANDATORY (MESH_JSON_GROUP_TAG_NAME | MESH_JSON_GROUP_TAG_ADDR | MESH_JSON_GROUP_TAG_PARENT)

#define MESH_JSON_SCENE_TAG_NAME                    0x0001
#define MESH_JSON_SCENE_TAG_ADDR                    0x0002
#define MESH_JSON_SCENE_TAG_NUMBER                  0x0004

#define MESH_JSON_TAG_SCENE_MANDATORY (MESH_JSON_SCENE_TAG_NAME | MESH_JSON_SCENE_TAG_ADDR | MESH_JSON_SCENE_TAG_NUMBER)

#define RANGE_TYPE_GROUP                            0
#define RANGE_TYPE_UNICAST                          1
#define RANGE_TYPE_SCENE                            2

#define FOUNDATION_FEATURE_BIT_RELAY                0x0001
#define FOUNDATION_FEATURE_BIT_PROXY                0x0002
#define FOUNDATION_FEATURE_BIT_FRIEND               0x0004
#define FOUNDATION_FEATURE_BIT_LOW_POWER            0x0008
#define FOUNDATION_FEATURE_BIT_PRIVATE_PROXY        0x0010

#define wiced_bt_get_buffer malloc
#define wiced_bt_free_buffer free

wiced_bt_mesh_db_mesh_t *mesh_json_read_file(FILE *fp);
void mesh_json_write_file(FILE *fp, wiced_bt_mesh_db_mesh_t *p_mesh);
int mesh_json_read_tag_name(FILE *fp, char *tagname, int len);
int mesh_json_read_string(FILE *fp, char prefix, char *p_string, int len);
int mesh_json_read_uuid(FILE *fp, char prefix, uint8_t *uuid);
int mesh_json_read_hex128(FILE *fp, char prefix, uint8_t *uuid);
int mesh_json_read_hex_array(FILE *fp, char prefix, uint8_t *value);
int mesh_json_read_uint8(FILE *fp, char prefix, uint8_t *value);
int mesh_json_read_uint16(FILE *fp, char prefix, uint16_t *value);
int mesh_json_read_uint32(FILE *fp, char prefix, uint32_t *value);
int mesh_json_read_boolean(FILE *fp, char prefix, uint8_t *value);
int mesh_json_read_hex16(FILE *fp, char prefix, uint16_t *value);
int mesh_json_read_hex32(FILE *fp, char prefix, uint32_t *value);
int mesh_json_read_name(FILE *fp, char prefix, int max_len, char **p_name);
int mesh_json_read_security(FILE *fp, char prefix, uint8_t *min_security);
int mesh_json_read_net_keys(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh);
int mesh_json_read_app_keys(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh);
int mesh_json_read_provisioners(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh);
int mesh_json_read_addr_range(FILE *fp, char c1, wiced_bt_mesh_db_provisioner_t *provisioner, int is_group);
int mesh_json_read_nodes(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh);
int mesh_json_read_groups(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh);
int mesh_json_read_scenes(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh);
int mesh_json_read_scene_addresses(FILE* fp, char prefix, wiced_bt_mesh_db_scene_t* scene);
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
int mesh_json_read_model_pub_period(FILE* fp, char prefix, uint32_t* period);
int mesh_json_read_model_pub_rexmit(FILE *fp, char prefix, wiced_bt_mesh_db_transmit_t *pub_rexmit);
int mesh_json_read_model_bind(FILE *fp, char prefix, wiced_bt_mesh_db_model_t *model);
int mesh_json_read_model_sensors(FILE *fp, char prefix, wiced_bt_mesh_db_model_t *model);
int mesh_json_parse_skip_value(FILE *fp, char prefix);
wiced_bool_t mesh_provisioner_ranges_overlap(wiced_bt_mesh_db_mesh_t *p_mesh);
char skip_space(FILE *fp);
char skip_comma(FILE *fp);
uint8_t process_nibble(char n);
extern uint8_t wiced_bt_mesh_property_len[WICED_BT_MESH_MAX_PROPERTY_ID + 1];
extern void Log(char *fmt, ...);

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

time_t mesh_read_utc_date_time(char *p_timestr)
{
    struct tm utc;
    time_t t, t1;
    memset(&utc, 0, sizeof(utc));
    if (sscanf(p_timestr, "%04d-%02d-%02dT%02d:%02d:%02dZ", &utc.tm_year, &utc.tm_mon, &utc.tm_mday, &utc.tm_hour, &utc.tm_min, &utc.tm_sec) == 6)
    {
        utc.tm_year -= 1900;
        utc.tm_mon -= 1;
        t = mktime(&utc);

        // convert UTC to local time
        t1 = mktime(gmtime(&t));
        t +=  t - t1;
    }
    else
        t = 0;
    return t;
}

wiced_bt_mesh_db_mesh_t *mesh_json_read_file(FILE *fp)
{
    char tagname[MAX_TAG_NAME];
    char buffer[100];
    char c1, c_temp;
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
            if (!mesh_json_read_string(fp, c1, buffer, 100))
            {
                failed = WICED_TRUE;
                break;
            }
            tags |= MESH_JSON_TAG_SCHEMA;
        }
        else if (strcmp(tagname, "id") == 0)
        {
            if (!mesh_json_read_string(fp, c1, buffer, 100))
            {
                failed = WICED_TRUE;
                break;
            }
            tags |= MESH_JSON_TAG_ID;
        }
        else if (strcmp(tagname, "version") == 0)
        {
            if (!mesh_json_read_string(fp, c1, buffer, 100) || strcmp(buffer, "1.0.0") != 0)    // only support version 1.0.0
            {
                failed = WICED_TRUE;
                break;
            }
            tags |= MESH_JSON_TAG_VERSION;
        }
        else if (strcmp(tagname, "meshUUID") == 0)
        {
            if (!mesh_json_read_uuid(fp, c1, p_mesh->uuid))
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
            if (!mesh_json_read_string(fp, c1, buffer, 100))
            {
                failed = WICED_TRUE;
                break;
            }
            p_mesh->timestamp = mesh_read_utc_date_time(buffer);
            tags |= MESH_JSON_TAG_TIMESTAMP;
        }
        else if (strcmp(tagname, "partial") == 0)
        {
            uint8_t partial;
            if (!mesh_json_read_boolean(fp, c1, &partial))
            {
                failed = WICED_TRUE;
                break;
            }
            // TBD - handle partial situation
            // if (partial) {}
            tags |= MESH_JSON_TAG_PARTIAL;
        }
        else if (strcmp(tagname, "provisioners") == 0)
        {
            if (!mesh_json_read_provisioners(fp, c1, p_mesh) || mesh_provisioner_ranges_overlap(p_mesh))
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
            Log("Unknown mesh property %s\n", tagname);
            if (!mesh_json_parse_skip_value(fp, c1))
            {
                failed = WICED_TRUE;
                break;
            }
        }

        if (skip_space(fp) != ',')
            break;

        c_temp = skip_space(fp);
    }
    // All the fields are retrieved. Verify that all mandatory fields are present
    if (failed || ((tags & MESH_JSON_TAG_MANDATORY) != MESH_JSON_TAG_MANDATORY))
    {
        if (failed)
            Log("Failed to read mesh property %s\n", tagname);
        else
            Log("Mesh property missing. Expected:%x, read:%x\n", MESH_JSON_TAG_MANDATORY, tags);
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

int mesh_json_read_string(FILE *fp, char prefix, char *buffer, int len)
{
    int i = 0;
    int c = 0;

    if (prefix != '\"')
        return 0;

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

int mesh_read_hex_string(char *str, uint8_t *value, int read_bytes, wiced_bool_t reverse)
{
    for (int i = 0; i < read_bytes; i++)
    {
        uint8_t n1 = process_nibble(str[i*2]);
        if (n1 == 0xff)
            return 0;

        uint8_t n2 = process_nibble(str[i*2 + 1]);
        if (n2 == 0xff)
            return 0;

        if (reverse)
            value[read_bytes - i - 1] = (n1 << 4) + n2;
        else
            value[i] = (n1 << 4) + n2;
    }
    return read_bytes;
}

int mesh_json_read_hex_array(FILE *fp, char prefix, uint8_t *value)
{
    char buffer[100];

    if (!mesh_json_read_string(fp, prefix, buffer, 100))
        return 0;

    return mesh_read_hex_string(buffer, value, strlen(buffer) / 2, WICED_FALSE);
}

int mesh_json_read_uuid(FILE *fp, char prefix, uint8_t *uuid)
{
    char buffer[37];
    size_t  i, j = 0;

    if (!mesh_json_read_string(fp, prefix, buffer, 37))
        return 0;

    for (i = 0; i < strlen(buffer); i += 2)
    {
        uint8_t n1 = process_nibble(buffer[i]);
        if (n1 == 0xff)
            return 0;

        uint8_t n2 = process_nibble(buffer[i + 1]);
        if (n2 == 0xff)
            return 0;

        uuid[j++] = (n1 << 4) + n2;
        if (buffer[i + 2] == '-')
            i++;
    }
    if (j == 16)
        return 1;
    return 0;
}

int mesh_json_read_hex128(FILE *fp, char prefix, uint8_t *value)
{
    char buffer[33];

    if (!mesh_json_read_string(fp, prefix, buffer, 33))
        return 0;

    return mesh_read_hex_string(buffer, value, 16, WICED_FALSE);
}

int mesh_json_read_hex16(FILE *fp, char prefix, uint16_t *value)
{
    char temp[5];
    int value_len;

    value_len = mesh_json_read_string(fp, prefix, temp, 5);
    if (value_len != 5)
        return 0;

    return mesh_read_hex_string(temp, (uint8_t *)value, 2, WICED_TRUE);
}

int mesh_json_read_hex32(FILE *fp, char prefix, uint32_t *value)
{
    char temp[9];
    int value_len;

    value_len = mesh_json_read_string(fp, prefix, temp, 9);
    if (value_len != 9)
        return 0;

    return mesh_read_hex_string(temp, (uint8_t *)value, 4, WICED_TRUE);
}

int mesh_json_read_address(FILE *fp, char prefix, wiced_bt_mesh_db_address_t *address)
{
    char buffer[33];
    int str_len;
    int value_len = 0;

    str_len = mesh_json_read_string(fp, prefix, buffer, 33);

    if (str_len == 5)
    {
        address->type = WICED_MESH_DB_ADDR_TYPE_ADDRESS;
        value_len = mesh_read_hex_string(buffer, (uint8_t *)&address->u.address, 2, WICED_TRUE);
    }
    else if (str_len == 33)
    {
        address->type = WICED_MESH_DB_ADDR_TYPE_LABEL_UUID;
        value_len = mesh_read_hex_string(buffer, address->u.label_uuid, 16, WICED_FALSE);
    }

    return value_len;
}

int mesh_json_read_model_id(FILE *fp, char prefix, wiced_bt_mesh_db_model_id_t *value)
{
    char temp[9];
    int value_len;
    int i, j = 0;
    uint8_t *p_value;

    value_len = mesh_json_read_string(fp, prefix, temp, 9);
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
            if (fseek(fp, -1, SEEK_CUR) != 0)
                return 0;

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
            if (fseek(fp, -1, SEEK_CUR) != 0)
                return 0;

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
            if (fseek(fp, -1, SEEK_CUR) != 0)
                return 0;

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

    pos = ftell(fp);

    namelen = mesh_json_read_string(fp, prefix, NULL, 512);
    if (namelen == 0)
        return 0;

    *p_name = (char *)wiced_bt_get_buffer(namelen);
    if (*p_name == NULL)
        return 0;

    if (fseek(fp, -namelen, SEEK_CUR) != 0)
        return 0;

    return mesh_json_read_string(fp, prefix, *p_name, namelen);
}

int mesh_json_read_security(FILE *fp, char prefix, uint8_t *security)
{
    char s[10];

    if (!mesh_json_read_string(fp, prefix, s, 10))
        return 0;

    if (strcmp(s, "insecure") == 0)
        *security = 0;
    else if (strcmp(s, "secure") == 0)
        *security = 1;
    else
        return 0;

    return 1;
}

int mesh_json_read_net_keys(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh)
{
    char tagname[MAX_TAG_NAME];
    char buffer[100];
    char c1;
    wiced_bt_mesh_db_net_key_t key;
    uint32_t tags = 0;
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
                    failed = WICED_TRUE;
                tags |= MESH_JSON_KEY_TAG_NAME;
            }
            else if (strcmp(tagname, "index") == 0)
            {
                if (!mesh_json_read_uint16(fp, c1, &key.index))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_KEY_TAG_KEY_INDEX;
            }
            else if (strcmp(tagname, "phase") == 0)
            {
                if (!mesh_json_read_uint8(fp, c1, &key.phase))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_KEY_TAG_PHASE;
            }
            else if (strcmp(tagname, "key") == 0)
            {
                if (!mesh_json_read_hex128(fp, c1, key.key))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_KEY_TAG_KEY;
            }
            else if (strcmp(tagname, "minSecurity") == 0)
            {
                if (!mesh_json_read_security(fp, c1, &key.min_security))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_KEY_TAG_MIN_SECURITY;
            }
            else if (strcmp(tagname, "oldKey") == 0)
            {
                if (!mesh_json_read_hex128(fp, c1, key.old_key))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_KEY_TAG_OLD_KEY;
            }
            else if (strcmp(tagname, "timestamp") == 0)
            {
                if (!mesh_json_read_string(fp, c1, buffer, 100))
                    failed = WICED_TRUE;
                else
                    key.timestamp = mesh_read_utc_date_time(buffer);
                tags |= MESH_JSON_KEY_TAG_TIMESTAMP;
            }
            else
            {
                // unknown tag
                Log("Unknown net key property %s\n", tagname);
                if (!mesh_json_parse_skip_value(fp, c1))
                    failed = WICED_TRUE;
            }
            if (failed)
            {
                Log("Failed to read net key %d property %s\n", key.index, tagname);
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
        if ((tags & MESH_JSON_TAG_NET_KEY_MANDATORY) != MESH_JSON_TAG_NET_KEY_MANDATORY)
        {
            Log("Net key %d property missing. Expected:%x, read:%x\n", key.index, MESH_JSON_TAG_NET_KEY_MANDATORY, tags);
            return 0;
        }
        // if phase is not 0, the old key should be present, or vice versa
        if (((key.phase != 0) && ((tags & MESH_JSON_KEY_TAG_OLD_KEY) == 0)) ||
            ((key.phase == 0) && ((tags & MESH_JSON_KEY_TAG_OLD_KEY) != 0)))
        {
            Log("Net key phase is %d but old key is %s\n", key.phase, (tags & MESH_JSON_KEY_TAG_OLD_KEY) == 0 ? "missing" : "present");
            return 0;
        }

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
                    failed = WICED_TRUE;
                tags |= MESH_JSON_KEY_TAG_NAME;
            }
            else if (strcmp(tagname, "index") == 0)
            {
                if (!mesh_json_read_uint16(fp, c1, &key.index))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_KEY_TAG_KEY_INDEX;
            }
            else if (strcmp(tagname, "boundNetKey") == 0)
            {
                if (!mesh_json_read_uint16(fp, c1, &key.bound_net_key_index))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_KEY_TAG_BOUND_NET_KEY;
            }
            else if (strcmp(tagname, "key") == 0)
            {
                if (!mesh_json_read_hex128(fp, c1, key.key))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_KEY_TAG_KEY;
            }
            else if (strcmp(tagname, "oldKey") == 0)
            {
                if (!mesh_json_read_hex128(fp, c1, key.old_key))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_KEY_TAG_OLD_KEY;
            }
            else
            {
                // unknown tag
                Log("Unknown app key property %s\n", tagname);
                if (!mesh_json_parse_skip_value(fp, c1))
                    failed = WICED_TRUE;
            }
            if (failed)
            {
                Log("Failed to read app key %d property %s\n", key.index, tagname);
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
        {
            Log("App key %d property missing. Expected:%x, read:%x\n", key.index, MESH_JSON_TAG_APP_KEY_MANDATORY, tags);
            return 0;
        }

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
                    failed = WICED_TRUE;
                tags |= MESH_JSON_PROVISIONER_TAG_NAME;
            }
            else if (strcmp(tagname, "UUID") == 0)
            {
                if (!mesh_json_read_uuid(fp, c1, provisioner.uuid))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_PROVISIONER_TAG_UUID;
            }
            else if (strcmp(tagname, "allocatedGroupRange") == 0)
            {
                if (!mesh_json_read_addr_range(fp, c1, &provisioner, RANGE_TYPE_GROUP))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_PROVISIONER_TAG_GROUP_RANGE;
            }
            else if (strcmp(tagname, "allocatedUnicastRange") == 0)
            {
                if (!mesh_json_read_addr_range(fp, c1, &provisioner, RANGE_TYPE_UNICAST))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_PROVISIONER_TAG_UNICAST_RANGE;
            }
            else if (strcmp(tagname, "allocatedSceneRange") == 0)
            {
                if (!mesh_json_read_addr_range(fp, c1, &provisioner, RANGE_TYPE_SCENE))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_PROVISIONER_TAG_SCENE_RANGE;
            }
            else
            {
                // unknown tag
                Log("Unknown provisioner property %s\n", tagname);
                if (!mesh_json_parse_skip_value(fp, c1))
                    failed = WICED_TRUE;
            }
            if (failed)
            {
                Log("Failed to read provisioner %s property %s\n", provisioner.name, tagname);
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
        {
            Log("Provisioner %s property missing. Expected:%x, read:%x\n", provisioner.name, MESH_JSON_TAG_APP_KEY_MANDATORY, tags);
            return 0;
        }

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

wiced_bool_t mesh_check_range_address(int range_type, uint16_t address)
{
    switch (range_type)
    {
    case RANGE_TYPE_GROUP:
        if (address < 0xC000)
        {
            Log("Group range address 0x%04x is too small\n", address);
            return WICED_FALSE;
        }
        break;
    case RANGE_TYPE_UNICAST:
        if (address == 0 || address > 0x7FFF)
        {
            Log("Unicast range address 0x%04x is out of range\n", address);
            return WICED_FALSE;
        }
        break;
    case RANGE_TYPE_SCENE:
        if (address == 0)
        {
            Log("Scene range value should not be 0\n");
            return WICED_FALSE;
        }
        break;
    }
    return WICED_TRUE;
}

int mesh_json_read_addr_range(FILE *fp, char c1, wiced_bt_mesh_db_provisioner_t *provisioner, int range_type)
{
    char tagname[MAX_TAG_NAME];
    wiced_bt_mesh_db_range_t *p_temp;
    wiced_bt_mesh_db_range_t range;
    wiced_bt_mesh_db_range_t **p_prov_range;
    uint16_t *num_ranges;
    wiced_bool_t failed = WICED_FALSE;
    char* range_name = range_type == RANGE_TYPE_GROUP ? "group" : (range_type == RANGE_TYPE_UNICAST ? "unicast" : "scene");

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

        while (1)
        {
            if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
                break;

            c1 = skip_space(fp);
            if (c1 != ':')
                return 0;

            c1 = skip_space(fp);

            if (range_type == RANGE_TYPE_GROUP || range_type == RANGE_TYPE_UNICAST)
            {
                if (strcmp(tagname, "highAddress") == 0)
                {
                    if (!mesh_json_read_hex16(fp, c1, &range.high_addr))
                        failed = WICED_TRUE;
                    else if (!mesh_check_range_address(range_type, range.high_addr))
                        return 0;
                }
                else if (strcmp(tagname, "lowAddress") == 0)
                {
                    if (!mesh_json_read_hex16(fp, c1, &range.low_addr))
                        failed = WICED_TRUE;
                    else if (!mesh_check_range_address(range_type, range.low_addr))
                        return 0;
                }
                else
                {
                    // unknown tag
                    Log("Unknown %s range property %s\n", range_name, tagname);
                    if (!mesh_json_parse_skip_value(fp, c1))
                        failed = WICED_TRUE;
                }
            }
            else // scene
            {
                if (strcmp(tagname, "firstScene") == 0)
                {
                    if (!mesh_json_read_hex16(fp, c1, &range.low_addr))
                        failed = WICED_TRUE;
                    else if (!mesh_check_range_address(range_type, range.low_addr))
                        return 0;
                }
                else if (strcmp(tagname, "lastScene") == 0)
                {
                    if (!mesh_json_read_hex16(fp, c1, &range.high_addr))
                        failed = WICED_TRUE;
                    else if (!mesh_check_range_address(range_type, range.high_addr))
                        return 0;
                }
                else
                {
                    // unknown tag
                    Log("Unknown %s range property %s\n", range_name, tagname);
                    if (!mesh_json_parse_skip_value(fp, c1))
                        failed = WICED_TRUE;
                }
            }
            if (failed)
            {
                Log("Failed to read %s range property %s\n", range_name, tagname);
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
        {
            Log("%s range %s is missing\n", range_name, range.high_addr == 0 ? "high_value" : "low_value");
            return 0;
        }
        // Check range high/low values
        if (range.high_addr < range.low_addr)
        {
            Log("%s range high_value 0x%04x is smaller than low_value 0x%04x\n", range_name, range.high_addr, range.low_addr);
            return 0;
        }

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
    wiced_bool_t failed = WICED_FALSE;

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
        node.feature.private_gatt_proxy = MESH_FEATURE_SUPPORTED_UNKNOWN;
        node.feature.low_power = MESH_FEATURE_SUPPORTED_UNKNOWN;
        node.feature.friend = MESH_FEATURE_SUPPORTED_UNKNOWN;
        node.beacon = MESH_FEATURE_SUPPORTED_UNKNOWN;
        node.private_beacon = MESH_FEATURE_SUPPORTED_UNKNOWN;
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
                if (!mesh_json_read_uuid(fp, c1, node.uuid))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_NODE_TAG_UUID;
            }
            else if (strcmp(tagname, "unicastAddress") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &node.unicast_address))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_NODE_TAG_UNICAST_ADDR;
            }
            else if (strcmp(tagname, "deviceKey") == 0)
            {
                if (!mesh_json_read_hex128(fp, c1, node.device_key))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_NODE_TAG_DEVICE_KEY;
            }
            else if (strcmp(tagname, "security") == 0)
            {
                if (!mesh_json_read_security(fp, c1, &node.security))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_NODE_TAG_SECURITY;
            }
            else if (strcmp(tagname, "netKeys") == 0)
            {
                if (!mesh_json_read_node_net_keys(fp, c1, &node))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_NODE_TAG_NET_KEYS;
            }
            else if (strcmp(tagname, "configComplete") == 0)
            {
                if (!mesh_json_read_boolean(fp, c1, &node.config_complete))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_NODE_TAG_CONFIG_COMPLETE;
            }
            else if (strcmp(tagname, "name") == 0)
            {
                if (!mesh_json_read_name(fp, c1, 512, &node.name))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_NODE_TAG_NAME;
            }
            else if (strcmp(tagname, "cid") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &node.cid))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_NODE_TAG_CID;
            }
            else if (strcmp(tagname, "pid") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &node.pid))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_NODE_TAG_PID;
            }
            else if (strcmp(tagname, "vid") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &node.vid))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_NODE_TAG_VID;
            }
            else if (strcmp(tagname, "crpl") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &node.crpl))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_NODE_TAG_CRPL;
            }
            else if (strcmp(tagname, "features") == 0)
            {
                if (!mesh_json_read_features(fp, c1, &node))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_NODE_TAG_FEATURES;
            }
            else if (strcmp(tagname, "secureNetworkBeacon") == 0)
            {
                if (!mesh_json_read_boolean(fp, c1, &node.beacon))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_NODE_TAG_BEACON;
            }
            else if (strcmp(tagname, "defaultTTL") == 0)
            {
                if (!mesh_json_read_uint8(fp, c1, &node.default_ttl))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_NODE_TAG_DEFAULT_TTL;
            }
            else if (strcmp(tagname, "networkTransmit") == 0)
            {
                if (!mesh_json_read_net_xmit(fp, c1, &node))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_NODE_TAG_NET_TRANSMIT;
            }
            else if (strcmp(tagname, "relayRetransmit") == 0)
            {
                if (!mesh_json_read_relay_rexmit(fp, c1, &node.relay_rexmit))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_NODE_TAG_RELAY_REXMIT;
            }
            else if (strcmp(tagname, "appKeys") == 0)
            {
                if (!mesh_json_read_node_app_keys(fp, c1, &node))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_NODE_TAG_APP_KEYS;
            }
            else if (strcmp(tagname, "elements") == 0)
            {
                if (!mesh_json_read_elements(fp, c1, &node))
                    failed = WICED_TRUE;
                else
                {
                    int num_of_primary_elements = 0;
                    for (int i = 0; i < node.num_elements; i++)
                    {
                        if (node.element[i].index == 0)
                            num_of_primary_elements++;
                    }
                    if (num_of_primary_elements != 1)
                    {
                        Log("Error: node %04x has %d primary elements\n", node.unicast_address, num_of_primary_elements);
                        return 0;
                    }
                }
                tags |= MESH_JSON_NODE_TAG_ELEMENTS;
            }
            else if (strcmp(tagname, "excluded") == 0)
            {
                if (!mesh_json_read_boolean(fp, c1, &node.blocked))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_NODE_TAG_EXCLUDED;
            }
            else
            {
                // unknown tag
                Log("Unknown node property %s\n", tagname);
                if (!mesh_json_parse_skip_value(fp, c1))
                    failed = WICED_TRUE;
            }
            if (failed)
            {
                Log("Failed to read node %04x property %s\n", node.unicast_address, tagname);
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
        {
            Log("Node %04x property missing. Expected:%x, read:%x\n", node.unicast_address, MESH_JSON_TAG_NODE_MANDATORY, tags);
            return 0;
        }

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

// Return 0 if two addresses are equal, non-zero otherwise
int mesh_address_compare(wiced_bt_mesh_db_address_t *addr1, wiced_bt_mesh_db_address_t *addr2)
{
    if (addr1->type == addr2->type)
    {
        if (addr1->type == WICED_MESH_DB_ADDR_TYPE_ADDRESS)
            return (int)addr1->u.address - (int)addr2->u.address;
        else
            return memcmp(addr1->u.label_uuid, addr2->u.label_uuid, 16);
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
    wiced_bool_t failed = WICED_FALSE;

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
                    failed = WICED_TRUE;
                tags |= MESH_JSON_GROUP_TAG_NAME;
            }
            else if (strcmp(tagname, "address") == 0)
            {
                if (!mesh_json_read_address(fp, c1, &group.addr))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_GROUP_TAG_ADDR;
            }
            else if (strcmp(tagname, "parentAddress") == 0)
            {
                if (!mesh_json_read_address(fp, c1, &group.parent_addr))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_GROUP_TAG_PARENT;
            }
            else
            {
                // unknown tag
                Log("Unknown group property %s\n", tagname);
                if (!mesh_json_parse_skip_value(fp, c1))
                    failed = WICED_TRUE;
            }
            if (failed)
            {
                Log("Failed to read group %s property %s\n", group.name, tagname);
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
        {
            Log("Group %s property missing. Expected:%x, read:%x\n", group.name, MESH_JSON_TAG_GROUP_MANDATORY, tags);
            return 0;
        }
        // Group address and parent address should be different
        if (mesh_address_compare(&group.addr, &group.parent_addr) == 0)
        {
            Log("Group %s address and parent address should not be the same\n", group.name);
            return 0;
        }

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
    wiced_bool_t failed = WICED_FALSE;

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
                    failed = WICED_TRUE;
                tags |= MESH_JSON_SCENE_TAG_NAME;
            }
            else if (strcmp(tagname, "addresses") == 0)
            {
                if (!mesh_json_read_scene_addresses(fp, c1, &scene))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_SCENE_TAG_ADDR;
            }
            else if (strcmp(tagname, "number") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &scene.number))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_SCENE_TAG_NUMBER;
            }
            else
            {
                // unknown tag
                Log("Unknown scene property %s\n", tagname);
                if (!mesh_json_parse_skip_value(fp, c1))
                    failed = WICED_TRUE;
            }
            if (failed)
            {
                Log("Failed to read scene %s property %s\n", scene.name, tagname);
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
        if ((tags & MESH_JSON_TAG_SCENE_MANDATORY) != MESH_JSON_TAG_SCENE_MANDATORY)
        {
            Log("Scene %s property missing. Expected:%x, read:%x\n", scene.name, MESH_JSON_TAG_SCENE_MANDATORY, tags);
            return 0;
        }

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

int mesh_json_read_scene_addresses(FILE *fp, char prefix, wiced_bt_mesh_db_scene_t *scene)
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

        if (!mesh_json_read_hex16(fp, c1, &addr))
            return 0;

        mesh_db_add_scene_address(scene, addr);

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
                    failed = WICED_TRUE;
                tags |= MESH_JSON_ELEMENT_TAG_NAME;
            }
            else if (strcmp(tagname, "index") == 0)
            {
                if (!mesh_json_read_uint8(fp, c1, &element.index))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_ELEMENT_TAG_INDEX;
            }
            else if (strcmp(tagname, "location") == 0)
            {
                if (!mesh_json_read_hex16(fp, c1, &element.location))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_ELEMENT_TAG_LOCATION;
            }
            else if (strcmp(tagname, "models") == 0)
            {
                if (!mesh_json_read_models(fp, c1, &element))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_ELEMENT_TAG_MODELS;
            }
            else
            {
                // unknown tag
                Log("Unknown element property %s\n", tagname);
                if (!mesh_json_parse_skip_value(fp, c1))
                    failed = WICED_TRUE;
            }
            if (failed)
            {
                Log("Failed to read element %d property %s\n", element.index, tagname);
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
        if ((tags & MESH_JSON_TAG_ELEMENT_MANDATORY) != MESH_JSON_TAG_ELEMENT_MANDATORY)
        {
            Log("Element %d property missing. Expected:%x, read:%x\n", element.index, MESH_JSON_TAG_ELEMENT_MANDATORY, tags);
            return 0;
        }

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

        tags = 0;

        while (1)
        {
            if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
                break;

            c1 = skip_space(fp);
            if (c1 != ':')
                return 0;

            c1 = skip_space(fp);

            if (strcmp(tagname, "index") == 0)
            {
                if (!mesh_json_read_uint16(fp, c1, &key.index))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_CONFIG_KEY_TAG_INDEX;
            }
            else if (strcmp(tagname, "updated") == 0)
            {
                uint8_t updated;
                if (!mesh_json_read_boolean(fp, c1, &updated))
                    failed = WICED_TRUE;
                else
                    key.phase = updated ? WICED_BT_MESH_KEY_REFRESH_PHASE_THIRD : WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL;
                tags |= MESH_JSON_CONFIG_KEY_TAG_UPDATED;
            }
            else
            {
                // unknown tag
                Log("Unknown node net key property %s\n", tagname);
                if (!mesh_json_parse_skip_value(fp, c1))
                    failed = WICED_TRUE;
            }
            if (failed)
            {
                Log("Failed to read node net key %d property %s\n", key.index, tagname);
                return 0;
            }

            c1 = skip_space(fp);

            if (c1 != ',')
                break;

            c1 = skip_space(fp);
        }
        if (c1 != '}')
            return 0;

        if ((tags & MESH_JSON_TAG_CONFIG_KEY_MANDATORY) != MESH_JSON_TAG_CONFIG_KEY_MANDATORY)
        {
            Log("Node net key %d property missing. Expected:%x, read:%x\n", key.index, MESH_JSON_TAG_CONFIG_KEY_MANDATORY, tags);
            return 0;
        }

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

        tags = 0;

        while (1)
        {
            if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
                break;

            c1 = skip_space(fp);
            if (c1 != ':')
                return 0;

            c1 = skip_space(fp);

            if (strcmp(tagname, "index") == 0)
            {
                if (!mesh_json_read_uint16(fp, c1, &key.index))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_CONFIG_KEY_TAG_INDEX;
            }
            else if (strcmp(tagname, "updated") == 0)
            {
                uint8_t updated;
                if (!mesh_json_read_boolean(fp, c1, &updated))
                    failed = WICED_TRUE;
                else
                    key.phase = updated ? WICED_BT_MESH_KEY_REFRESH_PHASE_THIRD : WICED_BT_MESH_KEY_REFRESH_PHASE_NORMAL;
                tags |= MESH_JSON_CONFIG_KEY_TAG_UPDATED;
            }
            else
            {
                // unknown tag
                Log("Unknown node app key property %s\n", tagname);
                if (!mesh_json_parse_skip_value(fp, c1))
                    failed = WICED_TRUE;
            }
            if (failed)
            {
                Log("Failed to read node app key %d property %s\n", key.index, tagname);
                return 0;
            }

            c1 = skip_space(fp);

            if (c1 != ',')
                break;

            c1 = skip_space(fp);
        }
        if (c1 != '}')
            return 0;

        if ((tags & MESH_JSON_TAG_CONFIG_KEY_MANDATORY) != MESH_JSON_TAG_CONFIG_KEY_MANDATORY)
        {
            Log("Node app key %d property missing. Expected:%x, read:%x\n", key.index, MESH_JSON_TAG_CONFIG_KEY_MANDATORY, tags);
            return 0;
        }

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
    wiced_bool_t failed = WICED_FALSE;

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
                failed = WICED_TRUE;
            tags |= MESH_JSON_CONFIG_NET_XMIT_TAG_COUNT;
        }
        else if (strcmp(tagname, "interval") == 0)
        {
            if (!mesh_json_read_uint32(fp, c1, &node->network_transmit.interval))
                failed = WICED_TRUE;
            tags |= MESH_JSON_CONFIG_NET_XMIT_TAG_INTERVAL;
        }
        else
        {
            // unknown tag
            Log("Unknown network transmit property %s\n", tagname);
            if (!mesh_json_parse_skip_value(fp, c1))
                failed = WICED_TRUE;
        }
        if (failed)
        {
            Log("Failed to read network transmit property %s\n", tagname);
            return 0;
        }

        c1 = skip_space(fp);

        if (c1 != ',')
            break;

        c1 = skip_space(fp);
    }
    if (c1 != '}')
        return 0;

    if ((tags & MESH_JSON_TAG_NET_XMIT_MANDATORY) != MESH_JSON_TAG_NET_XMIT_MANDATORY)
    {
        Log("Network transmit property missing. Expected:%x, read:%x\n", MESH_JSON_TAG_NET_XMIT_MANDATORY, tags);
        return 0;
    }

    return 1;
}

int mesh_json_read_relay_rexmit(FILE *fp, char prefix, wiced_bt_mesh_db_transmit_t *relay_rexmit)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    uint32_t tags = 0;
    wiced_bool_t failed = WICED_FALSE;

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
                failed = WICED_TRUE;
            tags |= MESH_JSON_CONFIG_NET_XMIT_TAG_COUNT;
        }
        else if (strcmp(tagname, "interval") == 0)
        {
            if (!mesh_json_read_uint32(fp, c1, &relay_rexmit->interval))
                failed = WICED_TRUE;
            tags |= MESH_JSON_CONFIG_NET_XMIT_TAG_INTERVAL;
        }
        else
        {
            // unknown tag
            Log("Unknown relay retransmit property %s\n", tagname);
            if (!mesh_json_parse_skip_value(fp, c1))
                failed = WICED_TRUE;
        }
        if (failed)
        {
            Log("Failed to read relay retransmit property %s\n", tagname);
            return 0;
        }

        c1 = skip_space(fp);

        if (c1 != ',')
            break;

        c1 = skip_space(fp);
    }
    if (c1 != '}')
        return 0;

    if ((tags & MESH_JSON_TAG_NET_XMIT_MANDATORY) != MESH_JSON_TAG_NET_XMIT_MANDATORY)
    {
        Log("Relay retransmit property missing. Expected:%x, read:%x\n", MESH_JSON_TAG_NET_XMIT_MANDATORY, tags);
        return 0;
    }

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
            {
                Log("Read features relay error, value:%d\n", node->feature.relay);
                return 0;
            }
        }
        else if (strcmp(tagname, "proxy") == 0)
        {
            if ((!mesh_json_read_uint8(fp, c1, &node->feature.gatt_proxy)) ||
                ((node->feature.gatt_proxy != MESH_FEATURE_DISABLED) && (node->feature.gatt_proxy != MESH_FEATURE_ENABLED) && (node->feature.gatt_proxy != MESH_FEATURE_UNSUPPORTED)))
            {
                Log("Read features proxy error, value:%d\n", node->feature.gatt_proxy);
                return 0;
            }
        }
        else if (strcmp(tagname, "friend") == 0)
        {
            if ((!mesh_json_read_uint8(fp, c1, &node->feature.friend)) ||
                ((node->feature.friend != MESH_FEATURE_DISABLED) && (node->feature.friend != MESH_FEATURE_ENABLED) && (node->feature.friend != MESH_FEATURE_UNSUPPORTED)))
            {
                Log("Read features friend error, value:%d\n", node->feature.friend);
                return 0;
            }
        }
        else if (strcmp(tagname, "lowPower") == 0)
        {
            if ((!mesh_json_read_uint8(fp, c1, &node->feature.low_power)) ||
                ((node->feature.low_power != MESH_FEATURE_DISABLED) && (node->feature.low_power != MESH_FEATURE_ENABLED) && (node->feature.low_power != MESH_FEATURE_UNSUPPORTED)))
            {
                Log("Read features lowPower error, value:%d\n", node->feature.low_power);
                return 0;
            }
        }
        else
        {
            // unknown tag
            Log("Unknown features property %s\n", tagname);
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
                    failed = WICED_TRUE;
                tags |= MESH_JSON_MODEL_TAG_MODEL_ID;
            }
            else if (strcmp(tagname, "subscribe") == 0)
            {
                if (!mesh_json_read_model_subscribe(fp, c1, &model))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_MODEL_TAG_SUBSCRIBE;
            }
            else if (strcmp(tagname, "publish") == 0)
            {
                if (!mesh_json_read_model_publish(fp, c1, &model.pub))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_MODEL_TAG_PUBLISH;
            }
            else if (strcmp(tagname, "bind") == 0)
            {
                if (!mesh_json_read_model_bind(fp, c1, &model))
                    failed = WICED_TRUE;
                tags |= MESH_JSON_MODEL_TAG_BIND;
            }
            else
            {
                // unknown tag
                Log("Unknown model property %s\n", tagname);
                if (!mesh_json_parse_skip_value(fp, c1))
                    failed = WICED_TRUE;
            }
            if (failed)
            {
                Log("Failed to read model property %s\n", tagname);
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
        {
            Log("Model property missing. Expected:%x, read:%x\n", MESH_JSON_TAG_MODEL_MANDATORY, tags);
            return 0;
        }

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
    wiced_bt_mesh_db_address_t addr;

    if (prefix != '[')
        return 0;

    while (1)
    {
        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (!mesh_json_read_address(fp, c1, &addr))
            return 0;

        mesh_db_add_model_sub(model, &addr);

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
            if (!mesh_json_read_hex16(fp, c1, &setting->setting_property_id) || setting->setting_property_id > WICED_BT_MESH_MAX_PROPERTY_ID)
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
    wiced_bool_t failed = WICED_FALSE;

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
            if (!mesh_json_read_address(fp, c1, &pub->address))
                failed = WICED_TRUE;
            tags |= MESH_JSON_PUBLISH_TAG_ADDRESS;
        }
        else if (strcmp(tagname, "index") == 0)
        {
            if (!mesh_json_read_uint16(fp, c1, &pub->index))
                failed = WICED_TRUE;
            tags |= MESH_JSON_PUBLISH_TAG_INDEX;
        }
        else if (strcmp(tagname, "ttl") == 0)
        {
            if (!mesh_json_read_uint8(fp, c1, &pub->ttl))
                failed = WICED_TRUE;
            tags |= MESH_JSON_PUBLISH_TAG_TTL;
        }
        else if (strcmp(tagname, "period") == 0)
        {
            if (!mesh_json_read_model_pub_period(fp, c1, &pub->period))
                failed = WICED_TRUE;
            tags |= MESH_JSON_PUBLISH_TAG_PERIOD;
        }
        else if (strcmp(tagname, "retransmit") == 0)
        {
            if (!mesh_json_read_model_pub_rexmit(fp, c1, &pub->retransmit))
                failed = WICED_TRUE;
            tags |= MESH_JSON_PUBLISH_TAG_REXMIT;
        }
        else if (strcmp(tagname, "credentials") == 0)
        {
            if (!mesh_json_read_uint8(fp, c1, &pub->credentials))
                failed = WICED_TRUE;
            tags |= MESH_JSON_PUBLISH_TAG_CREDENTIALS;
        }
        else
        {
            // unknown tag
            Log("Unknown publish property %s\n", tagname);
            if (!mesh_json_parse_skip_value(fp, c1))
                failed = WICED_TRUE;
        }
        if (failed)
        {
            Log("Failed to read publish property %s\n", tagname);
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
    {
        Log("Publish property missing. Expected:%x, read:%x\n", MESH_JSON_TAG_PUBLISH_MANDATORY, tags);
        return 0;
    }

    return 1;
}

int mesh_milliseconds_to_step_resolution(int milliseconds)
{
    int step_res = 0;

    switch (milliseconds)
    {
    case 100:
        step_res = 0;
        break;
    case 1000:
        step_res = 1;
        break;
    case 10000:
        step_res = 2;
        break;
    case 600000:
        step_res = 3;
        break;
    }
    return step_res;
}

int mesh_json_read_model_pub_period(FILE *fp, char prefix, uint32_t *period)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    uint8_t steps = 0;
    uint32_t resolution = 0;
    uint32_t tags = 0;
    wiced_bool_t failed = WICED_FALSE;

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

        if (strcmp(tagname, "numberOfSteps") == 0)
        {
            if (!mesh_json_read_uint8(fp, c1, &steps))
                failed = WICED_TRUE;
            tags |= MESH_JSON_PUBLISH_TAG_PERIOD_STEPS;
        }
        else if (strcmp(tagname, "resolution") == 0)
        {
            if (!mesh_json_read_uint32(fp, c1, &resolution))
                failed = WICED_TRUE;
            tags |= MESH_JSON_PUBLISH_TAG_PERIOD_RESOLUTION;
        }
        else
        {
            // unknown tag
            Log("Unknown publish period property %s\n", tagname);
            if (!mesh_json_parse_skip_value(fp, c1))
                failed = WICED_TRUE;
        }
        if (failed)
        {
            Log("Failed to read publish period property %s\n", tagname);
            return 0;
        }

        c1 = skip_space(fp);

        if (c1 != ',')
            break;

        c1 = skip_space(fp);
    }
    if (c1 != '}')
        return 0;

    if ((tags & MESH_JSON_TAG_PERIOD_MANDATORY) != MESH_JSON_TAG_PERIOD_MANDATORY)
    {
        Log("Publish period property missing. Expected:%x, read:%x\n", MESH_JSON_TAG_PERIOD_MANDATORY, tags);
        return 0;
    }

    *period = (steps | (mesh_milliseconds_to_step_resolution(resolution) << 6)) & 0xFF;

    return 1;
}

int mesh_json_read_model_pub_rexmit(FILE *fp, char prefix, wiced_bt_mesh_db_transmit_t *relay_rexmit)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    uint32_t tags = 0;
    wiced_bool_t failed = WICED_FALSE;

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
                failed = WICED_TRUE;
            tags |= MESH_JSON_CONFIG_NET_XMIT_TAG_COUNT;
        }
        else if (strcmp(tagname, "interval") == 0)
        {
            if (!mesh_json_read_uint32(fp, c1, &relay_rexmit->interval))
                failed = WICED_TRUE;
            tags |= MESH_JSON_CONFIG_NET_XMIT_TAG_INTERVAL;
        }
        else
        {
            // unknown tag
            Log("Unknown publish retransmit property %s\n", tagname);
            if (!mesh_json_parse_skip_value(fp, c1))
                failed = WICED_TRUE;
        }
        if (failed)
        {
            Log("Failed to read publish retransmit property %s\n", tagname);
            return 0;
        }

        c1 = skip_space(fp);

        if (c1 != ',')
            break;

        c1 = skip_space(fp);
    }
    if (c1 != '}')
        return 0;

    if ((tags & MESH_JSON_TAG_NET_XMIT_MANDATORY) != MESH_JSON_TAG_NET_XMIT_MANDATORY)
    {
        Log("Publish retransmit property missing. Expected:%x, read:%x\n", MESH_JSON_TAG_NET_XMIT_MANDATORY, tags);
        return 0;
    }

    return 1;
}

typedef struct range_list_item
{
    struct range_list_item   *p_next;
    wiced_bt_mesh_db_range_t  range;
} mesh_range_list_item;

int mesh_compare_range(wiced_bt_mesh_db_range_t *range1, wiced_bt_mesh_db_range_t *range2)
{
    if (range1->high_addr < range2->low_addr)
        return -1;
    if (range1->low_addr > range2->high_addr)
        return 1;
    Log("Range %04x-%04x overlaps with range %04x-%04x\n", range1->low_addr, range1->high_addr, range2->low_addr, range2->high_addr);
    return 0;
}

// Add a range to a sorted list, return FALSE if there's overlap
wiced_bool_t mesh_add_range_to_list(mesh_range_list_item **p_list, wiced_bt_mesh_db_range_t *range)
{
    mesh_range_list_item *item = (mesh_range_list_item *)wiced_bt_get_buffer(sizeof(mesh_range_list_item));
    mesh_range_list_item *p = *p_list;
    int cmp_result;

    if (item == NULL)
        return WICED_FALSE;

    memset(item, 0, sizeof(mesh_range_list_item));
    item->range = *range;

    if (*p_list == NULL)
    {
        *p_list = item;
        return WICED_TRUE;
    }
    // Compare with the first list item
    cmp_result = mesh_compare_range(range, &p->range);
    if (cmp_result < 0)
    {
        item->p_next = p;
        *p_list = item;
        return WICED_TRUE;
    }
    else if (cmp_result == 0)
    {
        wiced_bt_free_buffer(item);
        return WICED_FALSE;
    }
    // Compare down in the list
    while (p->p_next)
    {
        cmp_result = mesh_compare_range(range, &p->p_next->range);
        if (cmp_result < 0)
        {
            item->p_next = p->p_next;
            p->p_next = item;
            return WICED_TRUE;
        }
        else if (cmp_result == 0)
        {
            wiced_bt_free_buffer(item);
            return WICED_FALSE;
        }
        else
            p = p->p_next;
    }
    p->p_next = item;
    return WICED_TRUE;
}

void mesh_clean_range_list(mesh_range_list_item **p_list)
{
    mesh_range_list_item *p = *p_list;

    while (p)
    {
        mesh_range_list_item *p_temp = p->p_next;
        wiced_bt_free_buffer(p);
        p = p_temp;
    }
    *p_list = NULL;
}

wiced_bool_t mesh_provisioner_ranges_overlap(wiced_bt_mesh_db_mesh_t *p_mesh)
{
    mesh_range_list_item *list = NULL;
    int i, j;
    wiced_bool_t overlap = WICED_FALSE;

    // Check allocated group ranges for overlap
    for (i = 0; i < p_mesh->num_provisioners && !overlap; i++)
    {
        for (j = 0; j < p_mesh->provisioner[i].num_allocated_group_ranges && !overlap; j++)
        {
            if (!mesh_add_range_to_list(&list, &p_mesh->provisioner[i].p_allocated_group_range[j]))
                overlap = WICED_TRUE;
        }
    }
    mesh_clean_range_list(&list);
    if (overlap)
    {
        Log("Allocated group ranges has overlap\n");
        return WICED_TRUE;
    }
    // Check allocated unicast ranges for overlap
    for (i = 0; i < p_mesh->num_provisioners && !overlap; i++)
    {
        for (j = 0; j < p_mesh->provisioner[i].num_allocated_unicast_ranges && !overlap; j++)
        {
            if (!mesh_add_range_to_list(&list, &p_mesh->provisioner[i].p_allocated_unicast_range[j]))
                overlap = WICED_TRUE;
        }
    }
    mesh_clean_range_list(&list);
    if (overlap)
    {
        Log("Allocated unicast ranges has overlap\n");
        return WICED_TRUE;
    }
    // Check allocated scene ranges for overlap
    for (i = 0; i < p_mesh->num_provisioners && !overlap; i++)
    {
        for (j = 0; j < p_mesh->provisioner[i].num_allocated_scene_ranges && !overlap; j++)
        {
            if (!mesh_add_range_to_list(&list, &p_mesh->provisioner[i].p_allocated_scene_range[j]))
                overlap = WICED_TRUE;
        }
    }
    mesh_clean_range_list(&list);
    if (overlap)
    {
        Log("Allocated scene ranges has overlap\n");
    }
    return overlap;
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

static char *mesh_header = "{\r  \"$schema\":\"http://json-schema.org/draft-04/schema#\",\r  \"id\":\"http://www.bluetooth.com/specifications/assigned-numbers/mesh-profile/cdb-schema.json#\",\r  \"version\":\"1.0.0\",\r";
static char *mesh_footer = "}\r";
static char *hex_digits = "0123456789ABCDEF";

void mesh_json_write_uuid(FILE *fp, int offset, char *tag, uint8_t *uuid, int is_last)
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
        if (i == 3 || i == 5 || i == 7 || i == 9)
            *p++ = '-';
    }
    *p++ = '\"';
    if (!is_last)
        *p++ = ',';
    *p++ = '\n';
    *p++ = 0;
    fputs(buffer, fp);
}

void mesh_json_write_hex_byte_array(FILE *fp, int offset, char *tag, uint8_t *uuid, uint8_t len, int is_last)
{
    char buffer[80];
    char *p = buffer;
    int i;

    for (i = 0; i < offset; i++)
        *p++ = ' ';
    if (tag != NULL)
    {
        *p++ = '\"';
        for (i = 0; tag[i] != 0; i++)
            *p++ = tag[i];
        *p++ = '\"';
        *p++ = ':';
    }
    *p++ = '\"';
    for (i = 0; i < len; i++)
    {
        *p++ = hex_digits[uuid[i] >> 4];
        *p++ = hex_digits[uuid[i] & 0x0f];
    }
    *p++ = '\"';
    if (!is_last)
        *p++ = ',';
    if (tag != NULL)
        *p++ = '\n';
    *p++ = 0;
    fputs(buffer, fp);
}

void mesh_json_write_hex128(FILE* fp, int offset, char* tag, uint8_t* uuid, int is_last)
{
    mesh_json_write_hex_byte_array(fp, offset, tag, uuid, 16, is_last);
}

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
}

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
        strcat(buffer, ",");
    if (tag != NULL)
        strcat(buffer, "\n");
    fputs(buffer, fp);
}

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
}

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
        strcat(buffer, ",");
    if (tag != NULL)
        strcat(buffer, "\n");
    fputs(buffer, fp);
}

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
        strcat(buffer, ",");
    if (tag != NULL)
        strcat(buffer, "\n");
    fputs(buffer, fp);
}

void mesh_json_write_address(FILE *fp, int offset, char *tag, wiced_bt_mesh_db_address_t *address, int is_last)
{
    if (address->type == WICED_MESH_DB_ADDR_TYPE_ADDRESS)
        mesh_json_write_hex16(fp, offset, tag, address->u.address, is_last);
    else if (address->type == WICED_MESH_DB_ADDR_TYPE_LABEL_UUID)
        mesh_json_write_hex128(fp, offset, tag, address->u.label_uuid, is_last);
}

void mesh_print_utc_date_time(time_t t, char *p_buf, int max)
{
    struct tm *utc = gmtime(&t);
    strftime(p_buf, max, "%Y-%m-%dT%H:%M:%SZ", utc);
}

void mesh_json_write_netkey(FILE *fp, wiced_bt_mesh_db_net_key_t *net_key, int is_last)
{
    char buf[WICED_MESH_DB_TIMESTAMP_SIZE];

    fputs("    {\n", fp);

    mesh_json_write_string(fp, 6, "name", net_key->name, 0);
    mesh_json_write_int(fp, 6, "index", net_key->index, 0);
    mesh_json_write_int(fp, 6, "phase", net_key->phase, 0);
    mesh_json_write_hex128(fp, 6, "key", net_key->key, 0);
    mesh_json_write_string(fp, 6, "minSecurity", net_key->min_security == MIN_SECURITY_HIGH ? "secure" : "insecure", 0);
    if (net_key->phase)
        mesh_json_write_hex128(fp, 6, "oldKey", net_key->old_key, 0);
    mesh_print_utc_date_time(net_key->timestamp, buf, WICED_MESH_DB_TIMESTAMP_SIZE);
    mesh_json_write_string(fp, 6, "timestamp", buf, 1);

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

void mesh_json_write_scene_range(FILE *fp, int offset, wiced_bt_mesh_db_range_t *range, int is_last)
{
    fputs("        {\n", fp);

    mesh_json_write_hex16(fp, 10, "firstScene", range->low_addr, 0);
    mesh_json_write_hex16(fp, 10, "lastScene", range->high_addr, 1);

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
    mesh_json_write_uuid(fp, 6, "UUID", provisioner->uuid, 0);

    fputs("      \"allocatedUnicastRange\":[\n", fp);
    for (i = 0; i < provisioner->num_allocated_unicast_ranges; i++)
    {
        mesh_json_write_range(fp, 6, &provisioner->p_allocated_unicast_range[i], i == provisioner->num_allocated_unicast_ranges - 1);
    }
    fputs("      ],\n", fp);
    fputs("      \"allocatedGroupRange\":[\n", fp);
    for (i = 0; i < provisioner->num_allocated_group_ranges; i++)
    {
        mesh_json_write_range(fp, 6, &provisioner->p_allocated_group_range[i], i == provisioner->num_allocated_group_ranges - 1);
    }
    fputs("      ],\n", fp);
    fputs("      \"allocatedSceneRange\":[\n", fp);
    for (i = 0; i < provisioner->num_allocated_scene_ranges; i++)
    {
        mesh_json_write_scene_range(fp, 6, &provisioner->p_allocated_scene_range[i], i == provisioner->num_allocated_scene_ranges - 1);
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

int mesh_step_resolution_to_milliseconds(int step_res)
{
    int milliseconds = 0;

    switch (step_res)
    {
    case 0:
        milliseconds = 100;
        break;
    case 1:
        milliseconds = 1000;
        break;
    case 2:
        milliseconds = 10000;
        break;
    case 3:
        milliseconds = 600000;
        break;
    }
    return milliseconds;
}

void mesh_json_write_model(FILE *fp, wiced_bt_mesh_db_model_t *model, int is_last)
{
    int i;
    fputs("            {\n", fp);

    if (model->model.company_id == MESH_COMPANY_ID_BT_SIG)
        mesh_json_write_hex16(fp, 14, "modelId", model->model.id, 0);
    else
        mesh_json_write_hex32(fp, 14, "modelId", MODEL_ID_TO_UIN32(model->model), 0);

    fputs("              \"subscribe\":[", fp);
    for (i = 0; i < model->num_subs; i++)
    {
        mesh_json_write_address(fp, 1, NULL, &model->sub[i], i == model->num_subs - 1);
    }
    fputs(" ],\n", fp);

    if (model->pub.address.u.address != 0)
    {
        fputs("              \"publish\":{\n", fp);
        mesh_json_write_address(fp, 16, "address", &model->pub.address, 0);
        mesh_json_write_int(fp, 16, "index", model->pub.index, 0);
        mesh_json_write_int(fp, 16, "ttl", model->pub.ttl, 0);
        fputs("                \"period\":{\n", fp);
        mesh_json_write_int(fp, 18, "numberOfSteps", model->pub.period & 0x3F, 0);
        mesh_json_write_int(fp, 18, "resolution", mesh_step_resolution_to_milliseconds((model->pub.period & 0xFF) >> 6), 1);
        fputs("                },\n", fp);
        fputs("                \"retransmit\":{\n", fp);
        mesh_json_write_int(fp, 18, "count", model->pub.retransmit.count, 0);
        mesh_json_write_int(fp, 18, "interval", model->pub.retransmit.interval, 1);
        fputs("                },\n", fp);
        mesh_json_write_int(fp, 16, "credentials", model->pub.credentials, 1);
        fputs("              },\n", fp);
    }

    fputs("              \"bind\":[", fp);
    for (i = 0; i < model->num_bound_keys; i++)
    {
        mesh_json_write_int(fp, 1, NULL, model->bound_key[i], i == model->num_bound_keys - 1);
    }
    fputs(" ]\n", fp);

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

    mesh_json_write_int(fp, 10, "index", p_key->index, 0);
    mesh_json_write_boolean(fp, 10, "updated", p_key->phase == WICED_BT_MESH_KEY_REFRESH_PHASE_THIRD, 1);

    if (is_last)
        fputs("        }\n", fp);
    else
        fputs("        },\n", fp);
}

void mesh_json_write_node(FILE *fp, wiced_bt_mesh_db_node_t *node, int is_last)
{
    int i;
    uint16_t features = 0;

    fputs("    {\n", fp);

    mesh_json_write_uuid(fp, 6, "UUID", node->uuid, 0);
    mesh_json_write_hex16(fp, 6, "unicastAddress", node->unicast_address, 0);
    mesh_json_write_hex128(fp, 6, "deviceKey", node->device_key, 0);
    mesh_json_write_string(fp, 6, "security", node->security ? "secure" : "insecure", 0);

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
    if (node->feature.relay != MESH_FEATURE_SUPPORTED_UNKNOWN)
        features |= FOUNDATION_FEATURE_BIT_RELAY;
    if (node->feature.gatt_proxy != MESH_FEATURE_SUPPORTED_UNKNOWN)
        features |= FOUNDATION_FEATURE_BIT_PROXY;
    if (node->feature.low_power != MESH_FEATURE_SUPPORTED_UNKNOWN)
        features |= FOUNDATION_FEATURE_BIT_LOW_POWER;
    if (node->feature.friend != MESH_FEATURE_SUPPORTED_UNKNOWN)
        features |= FOUNDATION_FEATURE_BIT_FRIEND;
    if (features)
    {
        fputs("      \"features\":{\n", fp);
        if (features & FOUNDATION_FEATURE_BIT_RELAY)
        {
            features &= ~FOUNDATION_FEATURE_BIT_RELAY;
            mesh_json_write_int(fp, 8, "relay", node->feature.relay, features == 0);
        }
        if (features & FOUNDATION_FEATURE_BIT_PROXY)
        {
            features &= ~FOUNDATION_FEATURE_BIT_PROXY;
            mesh_json_write_int(fp, 8, "proxy", node->feature.gatt_proxy, features == 0);
        }
        if (features & FOUNDATION_FEATURE_BIT_LOW_POWER)
        {
            features &= ~FOUNDATION_FEATURE_BIT_LOW_POWER;
            mesh_json_write_int(fp, 8, "lowPower", node->feature.low_power, features == 0);
        }
        if (features & FOUNDATION_FEATURE_BIT_FRIEND)
        {
            features &= ~FOUNDATION_FEATURE_BIT_FRIEND;
            mesh_json_write_int(fp, 8, "friend", node->feature.friend, features == 0);
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

    fputs("      \"appKeys\":[\n", fp);
    for (i = 0; i < node->num_app_keys; i++)
        mesh_json_write_config_key(fp, &node->app_key[i], i == node->num_app_keys - 1);
    fputs("      ],\n", fp);

    fputs("      \"elements\":[\n", fp);
    for (i = 0; i < node->num_elements; i++)
        mesh_json_write_element(fp, &node->element[i], i == node->num_elements - 1);
    fputs("      ],\n", fp);

    mesh_json_write_boolean(fp, 6, "excluded", node->blocked, 1);
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
        sprintf(group->name, "group_%04x", group->addr.u.address);
    }
    mesh_json_write_string(fp, 6, "name", group->name, 0);
    mesh_json_write_address(fp, 6, "address", &group->addr, 0);
    mesh_json_write_address(fp, 6, "parentAddress", &group->parent_addr, 1);

    if (is_last)
        fputs("    }\n", fp);
    else
        fputs("    },\n", fp);
}

void mesh_json_write_scene(FILE *fp, wiced_bt_mesh_db_scene_t *scene, int is_last)
{
    fputs("    {\n", fp);

    mesh_json_write_string(fp, 6, "name", scene->name, 0);
    mesh_json_write_hex16(fp, 6, "number", scene->number, 0);
    fputs("      \"addresses\":[", fp);
    for (int i = 0; i < scene->num_addrs; i++)
    {
        mesh_json_write_hex16(fp, 1, NULL, scene->addr[i], i == scene->num_addrs - 1);
    }
    fputs(" ]\n", fp);

    if (is_last)
        fputs("    }\n", fp);
    else
        fputs("    },\n", fp);
}

void mesh_json_write_file(FILE *fp, wiced_bt_mesh_db_mesh_t *p_mesh)
{
    int i;
    char buf[WICED_MESH_DB_TIMESTAMP_SIZE];
    fputs(mesh_header, fp);
    mesh_json_write_uuid(fp, 2, "meshUUID", p_mesh->uuid, 0);
    mesh_json_write_string(fp, 2, "meshName", p_mesh->name, 0);
    mesh_print_utc_date_time(time(NULL), buf, WICED_MESH_DB_TIMESTAMP_SIZE);
    mesh_json_write_string(fp, 2, "timestamp", buf, 0);
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
    fputs("  ],\n", fp);
    mesh_json_write_boolean(fp, 2, "partial", WICED_FALSE, 1);
    fwrite(mesh_footer, 1, strlen(mesh_footer), fp);
}

int mesh_extra_params_read_models(FILE *fp, char prefix, wiced_bt_mesh_db_element_t *element)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    wiced_bt_mesh_db_model_id_t model_id;
    wiced_bt_mesh_db_model_t *p_model;

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

        p_model = NULL;

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
                if (!mesh_json_read_model_id(fp, c1, &model_id))
                    return 0;
                for (int i = 0; i < element->num_models; i++)
                {
                    if (element->model[i].model.id == model_id.id)
                    {
                        p_model = &element->model[i];
                        break;
                    }
                }
            }
            else if (p_model == NULL)
            {
                return 0;
            }
            else if (strcmp(tagname, "sensor") == 0)
            {
                if (!mesh_json_read_model_sensors(fp, c1, p_model))
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

        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != ',')
            return 0;
    }
    return 1;
}

int mesh_extra_params_read_features(FILE *fp, char prefix, wiced_bt_mesh_db_node_t *node)
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

        if (strcmp(tagname, "private_proxy") == 0)
        {
            if ((!mesh_json_read_uint8(fp, c1, &node->feature.private_gatt_proxy)) ||
                ((node->feature.private_gatt_proxy != MESH_FEATURE_DISABLED) && (node->feature.private_gatt_proxy != MESH_FEATURE_ENABLED) && (node->feature.private_gatt_proxy != MESH_FEATURE_UNSUPPORTED)))
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

int mesh_extra_params_read_elements(FILE *fp, char prefix, wiced_bt_mesh_db_node_t *node)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    uint8_t index;
    wiced_bt_mesh_db_element_t *p_element;

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

        p_element = NULL;

        while (1)
        {
            if (mesh_json_read_tag_name(fp, tagname, sizeof(tagname)) == 0)
                break;

            c1 = skip_space(fp);
            if (c1 != ':')
                return 0;

            c1 = skip_space(fp);

            if (strcmp(tagname, "index") == 0)
            {
                if (!mesh_json_read_uint8(fp, c1, &index))
                    return 0;
                for (int i = 0; i < node->num_elements; i++)
                {
                    if (node->element[i].index == index)
                    {
                        p_element = &node->element[i];
                        break;
                    }
                }
            }
            else if (p_element == NULL)
            {
                return 0;
            }
            else if (strcmp(tagname, "models") == 0)
            {
                if (!mesh_extra_params_read_models(fp, c1, p_element))
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

        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != ',')
            return 0;
    }
    return 1;
}

int mesh_extra_params_read_nodes(FILE *fp, char prefix, wiced_bt_mesh_db_mesh_t *p_mesh)
{
    char tagname[MAX_TAG_NAME];
    char c1;
    uint8_t uuid[WICED_MESH_DB_UUID_SIZE];
    wiced_bt_mesh_db_node_t *p_node;

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

        p_node = NULL;

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
                if (!mesh_json_read_uuid(fp, c1, uuid))
                    return 0;
                for (int i = 0; i < p_mesh->num_nodes; i++)
                {
                    if (memcmp(p_mesh->node[i].uuid, uuid, WICED_MESH_DB_UUID_SIZE) == 0)
                    {
                        p_node = &p_mesh->node[i];
                        break;
                    }
                }
            }
            else if (p_node == NULL)
            {
                return 0;
            }
            else if (strcmp(tagname, "features") == 0)
            {
                if (!mesh_extra_params_read_features(fp, c1, p_node))
                    return 0;
            }
            else if (strcmp(tagname, "privateBeacon") == 0)
            {
                if (!mesh_json_read_boolean(fp, c1, &p_node->private_beacon))
                    return 0;
            }
            else if (strcmp(tagname, "randomUpdateInterval") == 0)
            {
                if (!mesh_json_read_uint8(fp, c1, &p_node->random_update_interval))
                    return 0;
            }
            else if (strcmp(tagname, "onDemandPrivateProxy") == 0)
            {
                if (!mesh_json_read_uint8(fp, c1, &p_node->on_demand_private_proxy))
                    return 0;
            }
            else if (strcmp(tagname, "elements") == 0)
            {
                if (!mesh_extra_params_read_elements(fp, c1, p_node))
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

        c1 = skip_space(fp);
        if (c1 == ']')
            return 1;

        if (c1 != ',')
            return 0;
    }
    return 1;
}

void mesh_extra_params_read_file(FILE *fp, wiced_bt_mesh_db_mesh_t *p_mesh)
{
    char tagname[MAX_TAG_NAME];
    char c1, c_temp;
    uint8_t uuid[WICED_MESH_DB_UUID_SIZE];
    wiced_bool_t failed = WICED_FALSE;

    c1 = skip_space(fp);
    if (c1 != '{')
        failed = WICED_TRUE;
    else
    {
        c1 = skip_space(fp);
        if (c1 != '\"')
            failed = WICED_TRUE;
    }

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

        if (strcmp(tagname, "meshUUID") == 0)
        {
            if (!mesh_json_read_uuid(fp, c1, uuid) || memcmp(uuid, p_mesh->uuid, WICED_MESH_DB_UUID_SIZE) != 0)
            {
                failed = WICED_TRUE;
                break;
            }
        }
        else if (strcmp(tagname, "nodes") == 0)
        {
            if (!mesh_extra_params_read_nodes(fp, c1, p_mesh))
            {
                failed = WICED_TRUE;
                break;
            }
        }
        else if (strcmp(tagname, "solicitationSeqNum") == 0)
        {
            if (!mesh_json_read_hex32(fp, c1, &p_mesh->solicitation_seq_num))
            {
                failed = WICED_TRUE;
                break;
            }
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

        c_temp = skip_space(fp);
    }
}

void mesh_extra_params_write_model(FILE *fp, wiced_bt_mesh_db_model_t *model, int is_last)
{
    int i;
    fputs("            {\n", fp);

    if (model->model.company_id == MESH_COMPANY_ID_BT_SIG)
        mesh_json_write_hex16(fp, 14, "modelId", model->model.id, 0);
    else
        mesh_json_write_hex32(fp, 14, "modelId", MODEL_ID_TO_UIN32(model->model), 0);

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

wiced_bool_t mesh_model_has_extra_params(wiced_bt_mesh_db_model_t *model)
{
    wiced_bool_t has_extra_params = WICED_FALSE;

    if (model->num_sensors != 0)
        has_extra_params = WICED_TRUE;

    return has_extra_params;
}

int mesh_num_models_have_extra_params(wiced_bt_mesh_db_element_t *element)
{
    int num = 0;

    for (int i = 0; i < element->num_models; i++)
        if (mesh_model_has_extra_params(&element->model[i]))
            num++;

    return num;
}

void mesh_extra_params_write_element(FILE *fp, wiced_bt_mesh_db_element_t *element, int is_last)
{
    int num_models = mesh_num_models_have_extra_params(element);
    int written_models = 0;

    fputs("        {\n", fp);
    mesh_json_write_int(fp, 10, "index", element->index, num_models == 0);

    if (num_models > 0)
    {
        fputs("          \"models\":[\n", fp);
        for (int i = 0; i < element->num_models; i++)
        {
            if (mesh_model_has_extra_params(&element->model[i]))
            {
                mesh_extra_params_write_model(fp, &element->model[i], ++written_models == num_models);
            }
        }
        fputs("          ]\n", fp);
    }

    if (is_last)
        fputs("        }\n", fp);
    else
        fputs("        },\n", fp);
}

void mesh_extra_params_write_node(FILE *fp, wiced_bt_mesh_db_node_t *node, int is_last)
{
    int i;

    fputs("    {\n", fp);

    mesh_json_write_uuid(fp, 6, "UUID", node->uuid, 0);

    if (node->feature.private_gatt_proxy != MESH_FEATURE_SUPPORTED_UNKNOWN)
    {
        fputs("      \"features\":{\n", fp);
        mesh_json_write_int(fp, 8, "private_proxy", node->feature.private_gatt_proxy, 1);
        fputs("      },\n", fp);
    }
    if (node->private_beacon != MESH_FEATURE_SUPPORTED_UNKNOWN)
        mesh_json_write_boolean(fp, 6, "privateBeacon", node->private_beacon, 0);
    if (node->random_update_interval != 0)
        mesh_json_write_int(fp, 6, "randomUpdateInterval", node->random_update_interval, 0);
    if (node->on_demand_private_proxy != 0)
        mesh_json_write_int(fp, 6, "onDemandPrivateProxy", node->on_demand_private_proxy, 0);

    fputs("      \"elements\":[\n", fp);
    for (i = 0; i < node->num_elements; i++)
        mesh_extra_params_write_element(fp, &node->element[i], i == node->num_elements - 1);
    fputs("      ]\n", fp);

    if (is_last)
        fputs("    }\n", fp);
    else
        fputs("    },\n", fp);
}

void mesh_extra_params_write_file(FILE *fp, wiced_bt_mesh_db_mesh_t *p_mesh)
{
    int i;
    fputs("{\n", fp);
    mesh_json_write_uuid(fp, 2, "meshUUID", p_mesh->uuid, 0);
    if (p_mesh->solicitation_seq_num != 0)
        mesh_json_write_hex32(fp, 2, "solicitationSeqNum", p_mesh->solicitation_seq_num, 0);
    fputs("  \"nodes\":[\n", fp);
    for (i = 0; i < p_mesh->num_nodes; i++)
        mesh_extra_params_write_node(fp, &p_mesh->node[i], i == p_mesh->num_nodes - 1);
    fputs("  ]\n", fp);
    fwrite(mesh_footer, 1, strlen(mesh_footer), fp);
}
