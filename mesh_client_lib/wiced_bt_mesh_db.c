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
* wiced_bt_mesh_db.c : Implementation file for mesh db operations
*
*/

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

#include <fcntl.h>
#include <wiced_bt_mesh_models.h>
#include "wiced_memory.h"
#include "wiced_bt_mesh_db.h"
#include "meshdb.h"

#define FIRST_UNICAST_ADDR                  0x0001
#define LAST_UNICAST_ADDR                   0x7FFF
#define FIRST_GROUP_ADDR                    0xC000
#define LAST_GROUP_ADDR                     0xFF00
#define PROVISIONER_RANGE_SIZE              255

#define FOUNDATION_FEATURE_BIT_RELAY        0x0001
#define FOUNDATION_FEATURE_BIT_PROXY        0x0002
#define FOUNDATION_FEATURE_BIT_FRIEND       0x0004
#define FOUNDATION_FEATURE_BIT_LOW_POWER    0x0008

#define wiced_bt_get_buffer malloc
#define wiced_bt_free_buffer free

#define WICED_BT_DB_TRACE Log
extern void Log(char *fmt, ...);

uint8_t wiced_bt_mesh_property_len[WICED_BT_MESH_MAX_PROPERTY_ID + 1] =
{
    WICED_BT_MESH_PROPERTY_LEN_UNKNOWN, //PROPERTY_ID  0 prohibited
    WICED_BT_MESH_PROPERTY_LEN_AVERAGE_AMBIENT_TEMPERATURE_IN_A_PERIOD_OF_DAY,
    WICED_BT_MESH_PROPERTY_LEN_AVERAGE_INPUT_CURRENT,
    WICED_BT_MESH_PROPERTY_LEN_AVERAGE_INPUT_VOLTAGE,
    WICED_BT_MESH_PROPERTY_LEN_AVERAGE_OUTPUT_CURRENT,
    WICED_BT_MESH_PROPERTY_LEN_AVERAGE_OUTPUT_VOLTAGE,
    WICED_BT_MESH_PROPERTY_LEN_CENTER_BEAM_INTENSITY_AT_FULL_POWER,
    WICED_BT_MESH_PROPERTY_LEN_CHROMATICALLY_TOLERANCE,
    WICED_BT_MESH_PROPERTY_LEN_COLOR_RENDERING_INDEX_R9,
    WICED_BT_MESH_PROPERTY_LEN_COLOR_RENDERING_INDEX_RA,
    WICED_BT_MESH_PROPERTY_LEN_DEVICE_APPEARANCE,
    WICED_BT_MESH_PROPERTY_LEN_DEVICE_COUNTRY_OF_ORIGIN,
    WICED_BT_MESH_PROPERTY_LEN_DEVICE_DATE_OF_MANUFACTURE,
    WICED_BT_MESH_PROPERTY_LEN_DEVICE_ENERGY_USE_SINCE_TURN_ON,
    WICED_BT_MESH_PROPERTY_LEN_DEVICE_FIRMWARE_REVISION,
    WICED_BT_MESH_PROPERTY_LEN_DEVICE_GLOBAL_TRADE_ITEM_NUMBER,
    WICED_BT_MESH_PROPERTY_LEN_DEVICE_HARDWARE_REVISION,
    WICED_BT_MESH_PROPERTY_LEN_DEVICE_MANUFACTURER_NAME,
    WICED_BT_MESH_PROPERTY_LEN_DEVICE_MODEL_NUMBER,
    WICED_BT_MESH_PROPERTY_LEN_DEVICE_OPERATING_TEMPERATURE_RANGE_SPECIFICATION,
    WICED_BT_MESH_PROPERTY_LEN_DEVICE_OPERATING_TEMPERATURE_STATISTICAL_VALUES,
    WICED_BT_MESH_PROPERTY_LEN_DEVICE_OVER_TEMPERATURE_EVENT_STATISTICS,
    WICED_BT_MESH_PROPERTY_LEN_DEVICE_POWER_RANGE_SPECIFICATION,
    WICED_BT_MESH_PROPERTY_LEN_DEVICE_RUNTIME_SINCE_TURN_ON,
    WICED_BT_MESH_PROPERTY_LEN_DEVICE_RUNTIME_WARRANTY,
    WICED_BT_MESH_PROPERTY_LEN_DEVICE_SERIAL_NUMBER,
    WICED_BT_MESH_PROPERTY_LEN_DEVICE_SOFTWARE_REVISION,
    WICED_BT_MESH_PROPERTY_LEN_DEVICE_UNDER_TEMPERATURE_EVENT_STATISTICS,
    WICED_BT_MESH_PROPERTY_LEN_INDOOR_AMBIENT_TEMPERATURE_STATISTICAL_VALUES,
    WICED_BT_MESH_PROPERTY_LEN_INITIAL_CIE_CHROMATICITY_COORDINATES,
    WICED_BT_MESH_PROPERTY_LEN_INITIAL_CORRELATED_COLOR_TEMPERATURE,
    WICED_BT_MESH_PROPERTY_LEN_INITIAL_LUMINOUS_FLUX ,
    WICED_BT_MESH_PROPERTY_LEN_INITIAL_PLANCKIAN_DISTANCE,
    WICED_BT_MESH_PROPERTY_LEN_INPUT_CURRENT_RANGE_SPECIFICATION,
    WICED_BT_MESH_PROPERTY_LEN_INPUT_CURRENT_STATISTICS,
    WICED_BT_MESH_PROPERTY_LEN_INPUT_OVER_CURRENT_EVENT_STATISTICS,
    WICED_BT_MESH_PROPERTY_LEN_INPUT_OVER_RIPPLE_VOLTAGE_EVENT_STATISTICS,
    WICED_BT_MESH_PROPERTY_LEN_INPUT_OVER_VOLTAGE_EVENT_STATISTICS,
    WICED_BT_MESH_PROPERTY_LEN_INPUT_UNDER_CURRENT_EVENT_STATISTICS,
    WICED_BT_MESH_PROPERTY_LEN_INPUT_UNDER_VOLTAGE_EVENT_STATISTICS,
    WICED_BT_MESH_PROPERTY_LEN_INPUT_VOLTAGE_RANGE_SPECIFICATION,
    WICED_BT_MESH_PROPERTY_LEN_INPUT_VOLTAGE_RIPPLE_SPECIFICATION,
    WICED_BT_MESH_PROPERTY_LEN_INPUT_VOLTAGE_STATISTICS,
    WICED_BT_MESH_PROPERTY_LEN_AMBIENT_LUX_LEVEL_ON,
    WICED_BT_MESH_PROPERTY_LEN_AMBIENT_LUX_LEVEL_PROLONG,
    WICED_BT_MESH_PROPERTY_LEN_AMBIENT_LUX_LEVEL_STANDBY,
    WICED_BT_MESH_PROPERTY_LEN_LIGHTNESS_ON,
    WICED_BT_MESH_PROPERTY_LEN_LIGHTNESS_PROLONG,
    WICED_BT_MESH_PROPERTY_LEN_LIGHTNESS_STANDBY,
    WICED_BT_MESH_PROPERTY_LEN_REGULATOR_ACCURACY,
    WICED_BT_MESH_PROPERTY_LEN_REGULATOR_KID,
    WICED_BT_MESH_PROPERTY_LEN_REGULATOR_KIU,
    WICED_BT_MESH_PROPERTY_LEN_REGULATOR_KPD,
    WICED_BT_MESH_PROPERTY_LEN_REGULATOR_KPU,
    WICED_BT_MESH_PROPERTY_LEN_TIME_FADE,
    WICED_BT_MESH_PROPERTY_LEN_TIME_FADE_ON,
    WICED_BT_MESH_PROPERTY_LEN_TIME_FADE_STANDBY_AUTO,
    WICED_BT_MESH_PROPERTY_LEN_TIME_FADE_STANDBY_MANUAL,
    WICED_BT_MESH_PROPERTY_LEN_TIME_OCCUPANCY_DELAY,
    WICED_BT_MESH_PROPERTY_LEN_TIME_PROLONG,
    WICED_BT_MESH_PROPERTY_LEN_TIME_RUN_ON,
    WICED_BT_MESH_PROPERTY_LEN_LUMEN_MAINTENANCE_FACTOR,
    WICED_BT_MESH_PROPERTY_LEN_LUMINOUS_EFFICICACY,
    WICED_BT_MESH_PROPERTY_LEN_LUMINOUS_ENERGY_SINCE_TURN_ON,
    WICED_BT_MESH_PROPERTY_LEN_LUMINOUS_EXPOSURE,
    WICED_BT_MESH_PROPERTY_LEN_LUMINOUS_FLUX_RANGE,
    WICED_BT_MESH_PROPERTY_LEN_MOTION_SENSED,
    WICED_BT_MESH_PROPERTY_LEN_MOTION_THRESHOLD,
    WICED_BT_MESH_PROPERTY_LEN_OPEN_CIRCUIT_EVENT_STATISTICS,
    WICED_BT_MESH_PROPERTY_LEN_OUTDOOR_STATISTICAL_VALUES,
    WICED_BT_MESH_PROPERTY_LEN_OUTPUT_CURRENT_RANGE,
    WICED_BT_MESH_PROPERTY_LEN_OUTPUT_CURRENT_STATISTICS,
    WICED_BT_MESH_PROPERTY_LEN_OUTPUT_RIPPLE_VOLTAGE_SPECIFICATION,
    WICED_BT_MESH_PROPERTY_LEN_OUTPUT_VOLTAGE_RANGE,
    WICED_BT_MESH_PROPERTY_LEN_OUTPUT_VOLTAGE_STATISTICS,
    WICED_BT_MESH_PROPERTY_LEN_OVER_OUTPUT_RIPPLE_VOLTAGE_EVENT_STATISTICS,
    WICED_BT_MESH_PROPERTY_LEN_PEOPLE_COUNT,
    WICED_BT_MESH_PROPERTY_LEN_PRESENCE_DETECTED,
    WICED_BT_MESH_PROPERTY_LEN_PRESENT_AMBIENT_LIGHT_LEVEL,
    WICED_BT_MESH_PROPERTY_LEN_PRESENT_AMBIENT_TEMPERATURE,
    WICED_BT_MESH_PROPERTY_LEN_PRESENT_CIE_CHROMATICITY_COORDINATES,
    WICED_BT_MESH_PROPERTY_LEN_PRESENT_CORRELATED_COLOR_TEMPERATURE,
    WICED_BT_MESH_PROPERTY_LEN_PRESENT_DEVICE_INPUT_POWER,
    WICED_BT_MESH_PROPERTY_LEN_PRESENT_DEVICE_OPERATING_EFFICIENCY,
    WICED_BT_MESH_PROPERTY_LEN_PRESENT_DEVICE_OPERATING_TEMPERATURE,
    WICED_BT_MESH_PROPERTY_LEN_PRESENT_ILLUMINANCE,
    WICED_BT_MESH_PROPERTY_LEN_PRESENT_INDOOR_AMBIENT_TEMPERATURE,
    WICED_BT_MESH_PROPERTY_LEN_PRESENT_INPUT_CURRENT,
    WICED_BT_MESH_PROPERTY_LEN_PRESENT_INPUT_RIPPLE_VOLTAGE,
    WICED_BT_MESH_PROPERTY_LEN_PRESENT_INPUT_VOLTAGE,
    WICED_BT_MESH_PROPERTY_LEN_PRESENT_LUMINOUS_FLUX,
    WICED_BT_MESH_PROPERTY_LEN_PRESENT_OUTDOOR_AMBIENT_TEMPERATURE,
    WICED_BT_MESH_PROPERTY_LEN_PRESENT_OUTPUT_CURRENT,
    WICED_BT_MESH_PROPERTY_LEN_PRESENT_OUTPUT_VOLTAGE,
    WICED_BT_MESH_PROPERTY_LEN_PRESENT_PLANCKIAN_DISTANCE,
    WICED_BT_MESH_PROPERTY_LEN_PRESENT_RELATIVE_OUTPUT_RIPPLE_VOLTAGE,
    WICED_BT_MESH_PROPERTY_LEN_RELATIVE_DEVICE_ENERGY_USE_IN_A_PERIOD_OF_DAY,
    WICED_BT_MESH_PROPERTY_LEN_RELATIVE_DEVICE_RUNTIME_IN_A_GENERIC_LEVEL_RANGE,
    WICED_BT_MESH_PROPERTY_LEN_RELATIVE_EXPOSURE_TIME_IN_AN_ILLUMINANCE_RANGE,
    WICED_BT_MESH_PROPERTY_LEN_RELATIVE_RUNTIME_IN_A_CORRELATED_COLOR_TEMPERATURE_RANGE,
    WICED_BT_MESH_PROPERTY_LEN_RELATIVE_RUNTIME_IN_A_DEVICE_OPERATING_TEMPERATURE_RANGE,
    WICED_BT_MESH_PROPERTY_LEN_RELATIVE_RUNTIME_IN_AN_INPUT_CURRENT_RANGE,
    WICED_BT_MESH_PROPERTY_LEN_RELATIVE_RUNTIME_IN_AN_INPUT_VOLTAGE_RANGE,
    WICED_BT_MESH_PROPERTY_LEN_SHORT_CIRCUIT_EVENT_STATISTICS,
    WICED_BT_MESH_PROPERTY_LEN_TIME_SINCE_MOTION_SENSED,
    WICED_BT_MESH_PROPERTY_LEN_TIME_SINCE_PRESENCE_DETECTED,
    WICED_BT_MESH_PROPERTY_LEN_TOTAL_DEVICE_ENERGY_USE,
    WICED_BT_MESH_PROPERTY_LEN_TOTAL_DEVICE_OFF_ON_CYCLES,
    WICED_BT_MESH_PROPERTY_LEN_TOTAL_DEVICE_POWER_ON_CYCLES,
    WICED_BT_MESH_PROPERTY_LEN_TOTAL_DEVICE_POWER_ON_TIME,
    WICED_BT_MESH_PROPERTY_LEN_TOTAL_DEVICE_RUNTIME,
    WICED_BT_MESH_PROPERTY_LEN_TOTAL_LIGHT_EXPOSURE_TIME,
    WICED_BT_MESH_PROPERTY_LEN_TOTAL_LUMINOUS_ENERGY
};

static uint8_t process_nibble(char n);
static uint32_t get_hex_value(char *szbuf, uint8_t *buf, uint32_t buf_size);
static uint8_t composition_data_get_num_elements(uint8_t *p_composition_data, uint16_t len);
static wiced_bool_t is_group_address(uint16_t addr);
static void free_node(wiced_bt_mesh_db_node_t *node);
uint32_t get_int_value( uint8_t *value, int len);
static wiced_bt_mesh_db_sensor_t *find_model_sensor(wiced_bt_mesh_db_model_t *model, uint16_t property_id);

char *copy_name(const char *name)
{
    char *p_name = (char *)wiced_bt_get_buffer((uint16_t)(strlen(name) + 1));
    if (p_name != NULL)
        strcpy(p_name, name);
    return p_name;
}

#if defined __ANDROID__ || defined __APPLE__ || defined WICEDX_LINUX || defined BSA
#ifndef _WIN32
static int parse_ext(const struct dirent *dir)
{
    if(!dir)
        return 0;

    if(dir->d_type == DT_REG) { /* only deal with regular file */
        const char *ext = strrchr(dir->d_name,'.');

        if((!ext) || (ext == dir->d_name))
            return 0;
        else {
            if(strcmp(ext, ".json") == 0)
                return 1;
        }
    }

    return 0;
}
#endif
#endif  /* #if defined __ANDROID__ || defined __APPLE__ */

char *wiced_bt_mesh_db_get_all_networks(void)
{
#if !(defined(_WIN32)) && (defined __ANDROID__ || defined __APPLE__ || defined WICEDX_LINUX || defined BSA )
    WICED_BT_DB_TRACE("wiced_bt_mesh_db_get_all_networks");
    size_t buf_size = 0;
    char *buf = NULL, *p_buf;
    long hFile;
    DIR* directory;

    struct dirent **namelist;
    int n;

    n = scandir(".", &namelist, parse_ext, alphasort);
    if (n < 0) {

        perror("scandir");
        return NULL;
    }
    else {

        while (n--) {
            WICED_BT_DB_TRACE("%s\n", namelist[n]->d_name);
            buf_size += strlen(namelist[n]->d_name) - 4;
            wiced_bt_free_buffer(namelist[n]);
        }
        wiced_bt_free_buffer(namelist);
    }

    buf_size++;
    if ((buf = wiced_bt_get_buffer(buf_size + 5)) == NULL)
        return NULL;

    p_buf = buf;

    n = scandir(".", &namelist, parse_ext, alphasort);
    if (n < 0) {
        perror("scandir");
        return NULL;
    }
    else {
        while (n--) {

            strcpy(p_buf, namelist[n]->d_name);
            p_buf[strlen(namelist[n]->d_name) - 5] = 0;
            p_buf += strlen(namelist[n]->d_name) - 4;
            wiced_bt_free_buffer(namelist[n]);
        }
        wiced_bt_free_buffer(namelist);
    }
    *p_buf = 0;
    WICED_BT_DB_TRACE("wiced_bt_mesh_db_get_all_networks log %s", buf);
    return buf;
#else /* #if defined __ANDROID__ || defined __APPLE__ */
    size_t buf_size = 0;
    char *buf = NULL, *p_buf;
    struct _finddata_t json_file;
#ifdef _WIN32
    intptr_t hFile;
#else
    long hFile;
#endif

    if ((hFile = _findfirst("*.json", &json_file)) == -1L)
        return NULL;
    do
    {
        buf_size += strlen(json_file.name) - 4;
    } while (_findnext(hFile, &json_file) == 0);

    _findclose(hFile);
    buf_size++;
    if ((buf = (char *)wiced_bt_get_buffer((uint16_t)(buf_size + 5))) == NULL)
        return NULL;

    p_buf = buf;
    if ((hFile = _findfirst("*.json", &json_file)) == -1L)
        return NULL;
    do
    {
        strcpy(p_buf, json_file.name);
        p_buf[strlen(json_file.name) - 5] = 0;
        p_buf += strlen(json_file.name) - 4;
    } while (_findnext(hFile, &json_file) == 0);
    *p_buf = 0;
    return buf;
#endif  /* #if defined __ANDROID__ || defined __APPLE__ */
}

wiced_bool_t wiced_bt_mesh_db_network_exists(const char *mesh_name)
{
    char *p_filename = (char *)wiced_bt_get_buffer((uint16_t)(strlen(mesh_name) + 6));
    if (p_filename == NULL)
        return 0;

    strcpy(p_filename, mesh_name);
    strcat(p_filename, ".json");

    FILE *fp = fopen(p_filename, "rb");
    if (fp == NULL)
    {
    }
    else
    {
        fclose(fp);
    }
    wiced_bt_free_buffer(p_filename);
    return fp != NULL;
}

wiced_bool_t wiced_bt_mesh_db_network_delete(const char *mesh_name)
{
    wiced_bool_t res;

    char *p_filename = (char *)wiced_bt_get_buffer((uint16_t)(strlen(mesh_name) + 6));
    if (p_filename == NULL)
        return 0;

    strcpy(p_filename, mesh_name);
    strcat(p_filename, ".json");

    res = (remove(p_filename) == 0);
    wiced_bt_free_buffer(p_filename);
    return res;
}

wiced_bt_mesh_db_mesh_t *wiced_bt_mesh_db_init(const char *mesh_name)
{
    wiced_bt_mesh_db_mesh_t *mesh_db = NULL;

    char *p_filename = (char *)wiced_bt_get_buffer((uint16_t)(strlen(mesh_name) + 6));
    if (p_filename == NULL)
        return 0;

    strcpy(p_filename, mesh_name);
    strcat(p_filename, ".json");

    FILE *fp = fopen(p_filename, "rb");
    if (fp == NULL)
    {
        wiced_bt_free_buffer(p_filename);
        return NULL;
    }
    wiced_bt_free_buffer(p_filename);
    mesh_db = mesh_json_read_file(fp);
    fclose(fp);
    return mesh_db;
}

void wiced_bt_mesh_db_deinit(wiced_bt_mesh_db_mesh_t *mesh_db)
{
    int i;

    if (mesh_db == NULL)
        return;

    if (mesh_db->name != NULL)
    {
        wiced_bt_free_buffer(mesh_db->name);
        mesh_db->name = NULL;
    }
    for (i = 0; i < mesh_db->num_net_keys; i++)
    {
        if (mesh_db->net_key[i].name != NULL)
        {
            wiced_bt_free_buffer(mesh_db->net_key[i].name);
            mesh_db->net_key[i].name = NULL;
        }
    }
    if (mesh_db->net_key != NULL)
    {
        wiced_bt_free_buffer(mesh_db->net_key);
        mesh_db->net_key = NULL;
    }
    for (i = 0; i < mesh_db->num_app_keys; i++)
    {
        if (mesh_db->app_key[i].name != NULL)
        {
            wiced_bt_free_buffer(mesh_db->app_key[i].name);
            mesh_db->app_key[i].name = NULL;
        }
    }
    if (mesh_db->app_key != NULL)
    {
        wiced_bt_free_buffer(mesh_db->app_key);
        mesh_db->app_key = NULL;
    }
    for (i = 0; i < mesh_db->num_provisioners; i++)
    {
        if (mesh_db->provisioner[i].name != NULL)
        {
            wiced_bt_free_buffer(mesh_db->provisioner[i].name);
            mesh_db->provisioner[i].name = NULL;
        }
        if (mesh_db->provisioner[i].num_allocated_group_ranges != 0)
        {
            if (mesh_db->provisioner[i].p_allocated_group_range != NULL)
            {
                wiced_bt_free_buffer(mesh_db->provisioner[i].p_allocated_group_range);
                mesh_db->provisioner[i].p_allocated_group_range = NULL;
            }
        }
        if (mesh_db->provisioner[i].num_allocated_unicast_ranges != 0)
        {
            if (mesh_db->provisioner[i].p_allocated_unicast_range != NULL)
            {
                wiced_bt_free_buffer(mesh_db->provisioner[i].p_allocated_unicast_range);
                mesh_db->provisioner[i].p_allocated_unicast_range = NULL;
            }
        }
        if (mesh_db->provisioner[i].num_allocated_scene_ranges != 0)
        {
            if (mesh_db->provisioner[i].p_allocated_scene_range != NULL)
            {
                wiced_bt_free_buffer(mesh_db->provisioner[i].p_allocated_scene_range);
                mesh_db->provisioner[i].p_allocated_scene_range = NULL;
            }
        }
    }
    if (mesh_db->provisioner != NULL)
    {
        wiced_bt_free_buffer(mesh_db->provisioner);
        mesh_db->provisioner = NULL;
    }
    if (mesh_db->node != NULL)
    {
        for (i = 0; i < mesh_db->num_nodes; i++)
            free_node(&mesh_db->node[i]);

        wiced_bt_free_buffer(mesh_db->node);
        mesh_db->node = NULL;
    }
    if ((mesh_db->num_groups != 0) && (mesh_db->group != NULL))
    {
        for (i = 0; i < mesh_db->num_groups; i++)
        {
            if (mesh_db->group[i].name != NULL)
            {
                wiced_bt_free_buffer(mesh_db->group[i].name);
                mesh_db->group[i].name = NULL;
            }
        }
        wiced_bt_free_buffer(mesh_db->group);
        mesh_db->group = NULL;
    }
    if ((mesh_db->num_scenes != 0) && (mesh_db->scene != NULL))
    {
        for (i = 0; i < mesh_db->num_scenes; i++)
        {
            if (mesh_db->scene[i].name != NULL)
            {
                wiced_bt_free_buffer(mesh_db->scene[i].name);
                mesh_db->scene[i].name = NULL;
            }
        }
        wiced_bt_free_buffer(mesh_db->scene);
        mesh_db->scene = NULL;
    }
    wiced_bt_free_buffer(mesh_db);
    mesh_db = NULL;
}

void wiced_bt_mesh_db_store(wiced_bt_mesh_db_mesh_t *mesh_db)
{
    char *p_filename = (char *)wiced_bt_get_buffer((uint16_t)(strlen(mesh_db->name) + 6));
    if (p_filename == NULL)
        return;

    strcpy(p_filename, mesh_db->name);
    strcat(p_filename, ".json");

    FILE *fp = fopen(p_filename, "wb");
    if (fp == NULL)
    {
        wiced_bt_free_buffer(p_filename);
        return;
    }
    mesh_json_write_file(fp, mesh_db);
    fclose(fp);

    wiced_bt_free_buffer(p_filename);
}

uint8_t wiced_bt_mesh_db_num_net_keys(wiced_bt_mesh_db_mesh_t *mesh_db)
{
    return mesh_db->num_net_keys;
}

wiced_bt_mesh_db_net_key_t *wiced_bt_mesh_db_net_key_get(wiced_bt_mesh_db_mesh_t *mesh_db, uint8_t index)
{
    if (mesh_db->net_key == NULL || index >= mesh_db->num_net_keys)
        return NULL;

    return &mesh_db->net_key[index];
}

wiced_bool_t wiced_bt_mesh_db_net_key_add(wiced_bt_mesh_db_mesh_t *mesh_db, wiced_bt_mesh_db_net_key_t *key)
{
    wiced_bt_mesh_db_net_key_t *p_temp;

    p_temp = mesh_db->net_key;
    mesh_db->net_key = (wiced_bt_mesh_db_net_key_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_net_key_t) * (mesh_db->num_net_keys + 1));
    if (mesh_db->net_key == NULL)
        return WICED_FALSE;

    if (p_temp != 0)
    {
        memcpy(mesh_db->net_key, p_temp, sizeof(wiced_bt_mesh_db_net_key_t) * mesh_db->num_net_keys);
        wiced_bt_free_buffer(p_temp);
    }
    memcpy(&mesh_db->net_key[mesh_db->num_net_keys], key, sizeof(wiced_bt_mesh_db_net_key_t));
    mesh_db->num_net_keys++;
    return WICED_TRUE;
}

uint8_t wiced_bt_mesh_db_num_app_keys(wiced_bt_mesh_db_mesh_t *mesh_db)
{
    return mesh_db->num_app_keys;
}

wiced_bt_mesh_db_app_key_t *wiced_bt_mesh_db_app_key_get_by_key_index(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t app_key_idx)
{
    int i;
    for (i = 0; i < mesh_db->num_app_keys; i++)
    {
        if (mesh_db->app_key[i].index == app_key_idx)
        {
            return &mesh_db->app_key[i];
        }
    }
    return NULL;
}

wiced_bt_mesh_db_app_key_t *wiced_bt_mesh_db_app_key_get(wiced_bt_mesh_db_mesh_t *mesh_db, uint8_t index)
{
    if (mesh_db->app_key == NULL || index >= mesh_db->num_app_keys)
        return NULL;

    return &mesh_db->app_key[index];
}

wiced_bt_mesh_db_app_key_t *wiced_bt_mesh_db_app_key_get_by_name(wiced_bt_mesh_db_mesh_t *mesh_db, const char *p_name)
{
    int i;
    for (i = 0; i < mesh_db->num_app_keys; i++)
    {
        if (strcmp(mesh_db->app_key[i].name, p_name) == 0)
        {
            return &mesh_db->app_key[i];
        }
    }
    return NULL;
}

wiced_bool_t wiced_bt_mesh_db_app_key_add(wiced_bt_mesh_db_mesh_t *mesh_db, wiced_bt_mesh_db_app_key_t *app_key)
{
    wiced_bt_mesh_db_app_key_t *p_temp;

    p_temp = mesh_db->app_key;
    mesh_db->app_key = (wiced_bt_mesh_db_app_key_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_app_key_t) * (mesh_db->num_app_keys + 1));
    if (mesh_db->app_key == NULL)
        return WICED_FALSE;

    if (p_temp != 0)
    {
        memcpy(mesh_db->app_key, p_temp, sizeof(wiced_bt_mesh_db_app_key_t) * mesh_db->num_app_keys);
        wiced_bt_free_buffer(p_temp);
    }
    memcpy(&mesh_db->app_key[mesh_db->num_app_keys], app_key, sizeof(wiced_bt_mesh_db_app_key_t));
    mesh_db->num_app_keys++;
    return WICED_TRUE;
}

/*
 * Check addresses is in the ranges
 */
wiced_bool_t addr_belongs_to_range(wiced_bt_mesh_db_range_t *range, uint16_t addr)
{
    return ((range->high_addr != 0) && (range->low_addr != 0) && (addr >= range->low_addr) && (addr <= range->high_addr));
}

/*
 * Check addresses is in one of the ranges of the array
 */
wiced_bool_t addr_belongs_to_range_array(uint16_t num_ranges, wiced_bt_mesh_db_range_t *range, uint16_t addr)
{
    int i;

    for (i = 0; i < num_ranges; i++)
    {
        if (addr_belongs_to_range(&range[i], addr))
            return WICED_TRUE;
    }
    return WICED_FALSE;
}

/*
 * Check all ranges
 */
wiced_bool_t addr_in_use(wiced_bt_mesh_db_mesh_t *p_mesh, uint16_t addr_low, uint16_t addr_high)
{
    int i;

    for (i = 0; i < p_mesh->num_provisioners; i++)
    {
        if ((addr_belongs_to_range_array(p_mesh->provisioner[i].num_allocated_group_ranges, p_mesh->provisioner[i].p_allocated_group_range, addr_low)) ||
            (addr_belongs_to_range_array(p_mesh->provisioner[i].num_allocated_group_ranges, p_mesh->provisioner[i].p_allocated_group_range, addr_high)))
            return WICED_TRUE;

        if ((addr_belongs_to_range_array(p_mesh->provisioner[i].num_allocated_unicast_ranges, p_mesh->provisioner[i].p_allocated_unicast_range, addr_low)) ||
            (addr_belongs_to_range_array(p_mesh->provisioner[i].num_allocated_unicast_ranges, p_mesh->provisioner[i].p_allocated_unicast_range, addr_high)))
            return WICED_TRUE;
    }
    return WICED_FALSE;
}

/*
 * return number of unicast or group ranges in the provisioner object
 */
uint16_t num_provisioner_ranges(wiced_bt_mesh_db_provisioner_t *provisioner, const char *p_range_name)
{
    return (strcmp(p_range_name, "allocatedGroupRange") == 0) ? provisioner->num_allocated_group_ranges : provisioner->num_allocated_unicast_ranges;
}

/*
 * Returns provisioner range object at specified index of the provisioner object
 */
wiced_bt_mesh_db_range_t *provisioner_range_get(wiced_bt_mesh_db_provisioner_t *provisioner, const char *p_range_name, uint16_t range_index)
{
    return (strcmp(p_range_name, "allocatedGroupRange") == 0) ? &provisioner->p_allocated_group_range[range_index] : &provisioner->p_allocated_unicast_range[range_index];
}


/*
 * Get low/high addresses of a specified range in the provisioner object
 */
wiced_bool_t provisioner_range_values_get(wiced_bt_mesh_db_provisioner_t *provisioner, const char *p_range_name, uint16_t range_index, uint16_t *low_addr, uint16_t *high_addr)
{
    wiced_bt_mesh_db_range_t *range = provisioner_range_get(provisioner, p_range_name, range_index);

    if (range == NULL)
        return WICED_FALSE;

    *low_addr = range->low_addr;
    *high_addr = range->high_addr;
    return WICED_TRUE;
}

/*
 * create a new provisioner unicast or group range for a specified provisioner
 */
wiced_bool_t provisioner_range_add(wiced_bt_mesh_db_provisioner_t *provisioner, const char *range_name, uint16_t addr_low, uint16_t addr_high)
{
    wiced_bt_mesh_db_range_t *p_temp;
    wiced_bt_mesh_db_range_t range;
    range.low_addr = addr_low;
    range.high_addr = addr_high;

    if (strcmp(range_name, "allocatedGroupRange") == 0)
    {
        p_temp = provisioner->p_allocated_group_range;
        provisioner->p_allocated_group_range = (wiced_bt_mesh_db_range_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_range_t) * (provisioner->num_allocated_group_ranges + 1));
        if (provisioner->p_allocated_group_range == NULL)
            return WICED_FALSE;

        if (p_temp != 0)
        {
            memcpy(provisioner->p_allocated_group_range, p_temp, sizeof(wiced_bt_mesh_db_range_t) * provisioner->num_allocated_group_ranges);
            wiced_bt_free_buffer(p_temp);
        }
        memcpy(&provisioner->p_allocated_group_range[provisioner->num_allocated_group_ranges], &range, sizeof(wiced_bt_mesh_db_range_t));
        provisioner->num_allocated_group_ranges++;
    }
    else
    {
        p_temp = provisioner->p_allocated_unicast_range;
        provisioner->p_allocated_unicast_range = (wiced_bt_mesh_db_range_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_range_t) * (provisioner->num_allocated_unicast_ranges + 1));
        if (provisioner->p_allocated_unicast_range == NULL)
            return WICED_FALSE;

        if (p_temp != 0)
        {
            memcpy(provisioner->p_allocated_unicast_range, p_temp, sizeof(wiced_bt_mesh_db_range_t) * provisioner->num_allocated_unicast_ranges);
            wiced_bt_free_buffer(p_temp);
        }
        memcpy(&provisioner->p_allocated_unicast_range[provisioner->num_allocated_unicast_ranges], &range, sizeof(wiced_bt_mesh_db_range_t));
        provisioner->num_allocated_unicast_ranges++;
    }
    return WICED_TRUE;
}

/*
 * Check all provisioner's unicast addresses and all ranges
 */
uint16_t provisioner_alloc_range(wiced_bt_mesh_db_mesh_t *p_mesh, wiced_bt_mesh_db_provisioner_t *provisioner, const char *p_range_name)
{
    int i;
    uint16_t addr;

    addr = (strcmp(p_range_name, "allocatedGroupRange") == 0) ? FIRST_GROUP_ADDR : FIRST_UNICAST_ADDR;
    for (i = 0; i < p_mesh->num_provisioners; i++)
    {
        uint16_t range_index;
        uint16_t num_ranges = num_provisioner_ranges(&p_mesh->provisioner[i], p_range_name);

        for (range_index = 0; range_index < num_ranges; range_index++)
        {
            uint16_t low_address, high_address;
            if (provisioner_range_values_get(&p_mesh->provisioner[i], p_range_name, range_index, &low_address, &high_address))
            {
                if (high_address > addr)
                    addr = high_address + 1;
            }
        }
    }
    provisioner_range_add(provisioner, p_range_name, addr, addr + PROVISIONER_RANGE_SIZE);
    return addr;
}


uint8_t wiced_bt_mesh_db_num_provisioners(wiced_bt_mesh_db_mesh_t *mesh_db)
{
    return mesh_db->num_provisioners;
}

uint16_t wiced_bt_mesh_db_provisioner_add(wiced_bt_mesh_db_mesh_t *mesh_db, const char *name, uint8_t *uuid, uint8_t *dev_key)
{
    wiced_bt_mesh_db_provisioner_t *p_temp;
    wiced_bt_mesh_db_provisioner_t provisioner;
    uint16_t unicast_addr;

    memset(&provisioner, 0, sizeof(provisioner));

    provisioner.name = copy_name(name);
    memcpy(provisioner.uuid, uuid, sizeof(provisioner.uuid));

    p_temp = mesh_db->provisioner;
    mesh_db->provisioner = (wiced_bt_mesh_db_provisioner_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_provisioner_t) * (mesh_db->num_provisioners + 1));
    if (mesh_db->provisioner == NULL)
        return 0;

    if (p_temp != 0)
    {
        memcpy(mesh_db->provisioner, p_temp, sizeof(wiced_bt_mesh_db_provisioner_t) * mesh_db->num_provisioners);
        wiced_bt_free_buffer(p_temp);
    }
    memcpy(&mesh_db->provisioner[mesh_db->num_provisioners], &provisioner, sizeof(wiced_bt_mesh_db_provisioner_t));

    unicast_addr = provisioner_alloc_range(mesh_db, &mesh_db->provisioner[mesh_db->num_provisioners], "allocatedUnicastRange");
    if (unicast_addr != 0)
    {
        wiced_bt_mesh_db_node_create(mesh_db, name, unicast_addr, uuid, 1, dev_key, 0);
        mesh_db->num_provisioners++;
    }
    return unicast_addr;
}

wiced_bt_mesh_db_provisioner_t *wiced_bt_mesh_db_provisioner_get_by_uuid(wiced_bt_mesh_db_mesh_t *mesh_db, const uint8_t *provisioner_uuid)
{
    int i;
    for (i = 0; i < mesh_db->num_provisioners; i++)
    {
        if (memcmp(provisioner_uuid, mesh_db->provisioner[i].uuid, 16) == 0)
            return &mesh_db->provisioner[i];
    }
    return NULL;
}

wiced_bt_mesh_db_provisioner_t *wiced_bt_mesh_db_provisioner_get_by_addr(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr)
{
    int i;
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_addr(mesh_db, unicast_addr);

    if (node == NULL)
        return NULL;

    for (i = 0; i < mesh_db->num_provisioners; i++)
    {
        if (memcmp(mesh_db->provisioner[i].uuid, node->uuid, sizeof(node->uuid)) == 0)
            return &mesh_db->provisioner[i];
    }
    return NULL;
}

/*
 * Get Provisioner address
 * Returns address of the provisioner with specified name
 */
uint16_t wiced_bt_mesh_db_get_provisioner_addr(wiced_bt_mesh_db_mesh_t *mesh_db, const uint8_t *provisioner_uuid)
{
    wiced_bt_mesh_db_provisioner_t *provisioner = wiced_bt_mesh_db_provisioner_get_by_uuid(mesh_db, provisioner_uuid);
    int node_idx;

    if (provisioner != NULL)
    {
        for (node_idx = 0; node_idx < mesh_db->num_nodes; node_idx++)
        {
            if (memcmp(provisioner->uuid, mesh_db->node[node_idx].uuid, sizeof(provisioner->uuid)) == 0)
                return mesh_db->node[node_idx].unicast_address;
        }
    }
    return 0;
}

/*
 * Find nodes, locates the node with the specified address and returns the index in the nodes array.
 */
int find_node_index(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t node_addr)
{
    int node_idx;

    for (node_idx = 0; node_idx < mesh_db->num_nodes; node_idx++)
    {
        if ((mesh_db->node[node_idx].unicast_address <= node_addr) &&
            (mesh_db->node[node_idx].unicast_address + mesh_db->node[node_idx].num_elements > node_addr))
        {
            return node_idx;
        }
    }
    return -1;
}

uint16_t wiced_bt_mesh_db_alloc_unicast_addr(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t provisioner_addr, uint8_t num_elements, uint8_t *db_changed)
{
    wiced_bt_mesh_db_provisioner_t *provisioner = wiced_bt_mesh_db_provisioner_get_by_addr(mesh_db, provisioner_addr);
    uint16_t num_ranges;
    uint16_t range_index;
    uint16_t addr = 0;

    if (provisioner == NULL)
        return 0;

    *db_changed = WICED_FALSE;

    num_ranges = num_provisioner_ranges(provisioner, "allocatedUnicastRange");
    for (range_index = 0; range_index < num_ranges; range_index++)
    {
        uint16_t low_address, high_address;

        if (provisioner_range_values_get(provisioner, "allocatedUnicastRange", range_index, &low_address, &high_address))
        {
            // We know how many elements need to be allocated.
            for (addr = low_address; addr < high_address - num_elements; addr++)
            {
                // the new node will take addr, addr + 1, ... addr + num_elements addresses
                uint16_t i;
                for (i = addr; i < addr + num_elements; i++)
                {
                    if (find_node_index(mesh_db, i) > -1)
                    {
                        break;
                    }
                }
                if (i == addr + num_elements)
                {
                    return addr;
                }
            }
        }
    }
    // all ranges are full, allocate a new range
    *db_changed = WICED_TRUE;
    return provisioner_alloc_range(mesh_db, provisioner, "allocatedUnicastRange");
}

wiced_bt_mesh_db_group_t *wiced_bt_mesh_db_group_get_by_name(wiced_bt_mesh_db_mesh_t *mesh_db, const char *group_name)
{
    int i;

    for (i = 0; i < mesh_db->num_groups; i++)
    {
        if (strcmp(mesh_db->group[i].name, group_name) == 0)
        {
            return &mesh_db->group[i];
        }
    }
    return NULL;
}

wiced_bool_t wiced_bt_mesh_db_group_rename(wiced_bt_mesh_db_mesh_t *mesh_db, const char *old_name, const char *new_name)
{
    int i;

    for (i = 0; i < mesh_db->num_groups; i++)
    {
        if (strcmp(mesh_db->group[i].name, old_name) == 0)
        {
            wiced_bt_free_buffer(mesh_db->group[i].name);
            mesh_db->group[i].name = copy_name(new_name);
            return WICED_TRUE;
        }
    }
    return WICED_FALSE;
}

wiced_bt_mesh_db_group_t *wiced_bt_mesh_db_group_get_by_addr(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t addr)
{
    int i;

    for (i = 0; i < mesh_db->num_groups; i++)
    {
        if (mesh_db->group[i].addr == addr)
        {
            return &mesh_db->group[i];
        }
    }
    return NULL;
}

uint16_t wiced_bt_mesh_db_group_get_addr(wiced_bt_mesh_db_mesh_t *mesh_db, const char *group_name)
{
    int i;

    if (strcmp(mesh_db->name, group_name) == 0)
    {
        return 0xFFFF;
    }
    for (i = 0; i < mesh_db->num_groups; i++)
    {
        if (strcmp(mesh_db->group[i].name, group_name) == 0)
        {
            return mesh_db->group[i].addr;
        }
    }
    return 0;
}

uint16_t wiced_bt_mesh_db_alloc_group_addr(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t provisioner_addr)
{
    wiced_bt_mesh_db_provisioner_t *provisioner = wiced_bt_mesh_db_provisioner_get_by_addr(mesh_db, provisioner_addr);
    uint16_t num_ranges;
    uint16_t range_index;
    uint16_t addr = 0;

    if (provisioner == NULL)
        return 0;

    num_ranges = num_provisioner_ranges(provisioner, "allocatedGroupRange");
    for (range_index = 0; range_index < num_ranges; range_index++)
    {
        uint16_t low_address, high_address;

        if (provisioner_range_values_get(provisioner, "allocatedGroupRange", range_index, &low_address, &high_address))
        {
            for (addr = low_address; addr <= high_address; addr++)
            {
                if (wiced_bt_mesh_db_group_get_by_addr(mesh_db, addr) == NULL)
                {
                    return addr;
                }
            }
        }
    }
    // all ranges are full, allocate a new range
    return provisioner_alloc_range(mesh_db, provisioner, "allocatedGroupRange");
}

uint16_t wiced_bt_mesh_db_group_add(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t provisioner_addr, const char *group_name, const char *parent_group_name)
{
    wiced_bt_mesh_db_group_t *p_temp;
    wiced_bt_mesh_db_group_t *p_parent = wiced_bt_mesh_db_group_get_by_name(mesh_db, parent_group_name);
    wiced_bt_mesh_db_group_t group;

    if ((group.addr = wiced_bt_mesh_db_alloc_group_addr(mesh_db, provisioner_addr)) == 0)
        return 0;

    group.name = copy_name(group_name);
    group.parent_addr = (p_parent != NULL) ? p_parent->addr : 0;

    p_temp = mesh_db->group;
    mesh_db->group = (wiced_bt_mesh_db_group_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_group_t) * (mesh_db->num_groups + 1));
    if (mesh_db->group == NULL)
        return WICED_FALSE;

    if (p_temp != 0)
    {
        memcpy(mesh_db->group, p_temp, sizeof(wiced_bt_mesh_db_group_t) * mesh_db->num_groups);
        wiced_bt_free_buffer(p_temp);
    }
    memcpy(&mesh_db->group[mesh_db->num_groups], &group, sizeof(wiced_bt_mesh_db_group_t));
    mesh_db->num_groups++;
    return group.addr;
}

void group_set_parent(wiced_bt_mesh_db_mesh_t *mesh_db, const char *group_name, uint16_t parent_addr)
{
    wiced_bt_mesh_db_group_t *group = wiced_bt_mesh_db_group_get_by_name(mesh_db, group_name);

    if (group != NULL)
        group->parent_addr = parent_addr;
}

wiced_bool_t wiced_bt_mesh_db_group_delete(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t provisioner_addr, const char *group_name)
{
    int i;
    for (i = 0; i < mesh_db->num_groups; i++)
    {
        if (strcmp(mesh_db->group[i].name, group_name) == 0)
        {
            if (mesh_db->num_groups == 1)
            {
                mesh_db->num_groups = 0;
                wiced_bt_free_buffer(mesh_db->group);
                mesh_db->group = NULL;
            }
            else
            {
                for (; i < mesh_db->num_groups - 1; i++)
                {
                    memcpy(&mesh_db->group[i], &mesh_db->group[i + 1], sizeof(wiced_bt_mesh_db_group_t));
                }
                mesh_db->num_groups--;
            }
            return WICED_TRUE;
        }
    }
    return WICED_FALSE;
}

#if 0
uint16_t wiced_bt_mesh_db_get_group_addr(wiced_bt_mesh_db_mesh_t *mesh_db, const char *group_name)
{
    wiced_bt_mesh_db_group_t *group;

    if (strcmp(group_name, mesh_db->name) == 0)
        return 0xffff;

    group = wiced_bt_mesh_db_group_get_by_name(mesh_db, group_name);
    return (group != NULL) ? group->addr : 0;
}
#endif

const char *wiced_bt_mesh_db_get_group_name(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t addr)
{
    wiced_bt_mesh_db_group_t *group = wiced_bt_mesh_db_group_get_by_addr(mesh_db, addr);
    return (group != NULL) ? group->name : NULL;
}

char *wiced_bt_mesh_db_get_all_groups(wiced_bt_mesh_db_mesh_t *mesh_db, char *in_group)
{
    int i;
    wiced_bt_mesh_db_group_t *parent;
    uint16_t parent_addr = 0;
    size_t buf_size = 0;
    char *buf = NULL, *p_buf;

    if (in_group == NULL)
        return NULL;

    if (strcmp(in_group, mesh_db->name) != 0)
    {
        parent = wiced_bt_mesh_db_group_get_by_name(mesh_db, in_group);
        if (parent != NULL)
            parent_addr = parent->addr;
    }
    for (i = 0; i < mesh_db->num_groups; i++)
    {
        if (mesh_db->group[i].parent_addr == parent_addr)
        {
            buf_size += (strlen(mesh_db->group[i].name) + 1);
        }
    }
    buf_size++;
    if ((buf = (char *)wiced_bt_get_buffer((uint16_t)buf_size)) == NULL)
        return NULL;

    p_buf = buf;
    for (i = 0; i < mesh_db->num_groups; i++)
    {
        if (mesh_db->group[i].parent_addr == parent_addr)
        {
            strcpy(p_buf, mesh_db->group[i].name);
            p_buf += (strlen(mesh_db->group[i].name));
            *p_buf++ = '\0';
        }
    }
    *p_buf = 0;
    return buf;
}

char *wiced_bt_mesh_db_get_all_provisioners(wiced_bt_mesh_db_mesh_t *mesh_db)
{
    int i;
    size_t buf_size = 0;
    char *buf = NULL, *p_buf;

    for (i = 0; i < mesh_db->num_provisioners; i++)
    {
        buf_size += (strlen(mesh_db->provisioner[i].name) + 1);
    }
    buf_size++;
    if ((buf = (char *)wiced_bt_get_buffer((uint16_t)buf_size)) == NULL)
        return NULL;

    p_buf = buf;
    for (i = 0; i < mesh_db->num_provisioners; i++)
    {
        strcpy(p_buf, mesh_db->provisioner[i].name);
        p_buf += (strlen(mesh_db->provisioner[i].name));
        *p_buf++ = '\0';
    }
    *p_buf = 0;
    return buf;
}

/*
 * go recursiverly through parents to figure out if the group is a decendant of a parent
 */
wiced_bool_t is_group_a_perent_group(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t parent_group_addr, uint16_t group_addr)
{
    wiced_bt_mesh_db_group_t *group = wiced_bt_mesh_db_group_get_by_addr(mesh_db, group_addr);
    if (group == NULL)
        return WICED_FALSE;
    if (group->parent_addr == parent_group_addr)
        return WICED_TRUE;
    if (group->parent_addr == 0)
        return WICED_FALSE;
    return is_group_a_perent_group(mesh_db, parent_group_addr, group->parent_addr);
}

wiced_bt_mesh_db_element_t *element_get_by_addr(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr)
{
    int node_idx;
    int elem_idx;

    for (node_idx = 0; node_idx < mesh_db->num_nodes; node_idx++)
    {
        if ((mesh_db->node[node_idx].unicast_address <= element_addr) &&
            (mesh_db->node[node_idx].unicast_address + mesh_db->node[node_idx].num_elements > element_addr))
        {
            elem_idx = element_addr - mesh_db->node[node_idx].unicast_address;
            return &mesh_db->node[node_idx].element[elem_idx];
        }
    }
    return NULL;
}

uint16_t *wiced_bt_mesh_db_get_element_group_list(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr)
{
    wiced_bt_mesh_db_element_t *element = element_get_by_addr(mesh_db, element_addr);
    int model_idx, sub_idx;
    int num_subscriptions = 0;
    uint16_t *p_group_list = NULL;
    int i, j, num_groups = 0;

    if (element == NULL)
        return NULL;

    for (model_idx = 0; model_idx < element->num_models; model_idx++)
    {
        // first calculate number of subscriptions to know what size of array to allocate. We will not check for
        // duplicates, so array may happen to be a bit larger than necessary.
        num_subscriptions += element->model[model_idx].num_subs;
    }
    // publication can also add a group
    num_subscriptions += 2;

    if ((p_group_list = (uint16_t *)wiced_bt_get_buffer(sizeof(p_group_list) * num_subscriptions)) == NULL)
        return NULL;

    memset(p_group_list, 0, (sizeof(p_group_list) * num_subscriptions));

    for (model_idx = 0; model_idx < element->num_models; model_idx++)
    {
        for (sub_idx = 0; sub_idx < element->model[model_idx].num_subs; sub_idx++)
        {
            // check if this sub address already present in the list
            for (i = 0; i < num_groups; i++)
            {
                if (p_group_list[i] == element->model[model_idx].sub[sub_idx])
                    break;
            }
            if (i == num_groups)
            {
                p_group_list[num_groups++] = element->model[model_idx].sub[sub_idx];
            }
        }
        // TBD add support for virtual addresses
        // skip publication to broadcast
        if ((element->model[model_idx].pub.address != 0) && (element->model[model_idx].pub.address != 0xffff))
        {
            if (is_group_address(element->model[model_idx].pub.address))
            {
                for (i = 0; i < num_groups; i++)
                {
                    if (p_group_list[i] == element->model[model_idx].pub.address)
                        break;
                }
                if (i == num_groups)
                {
                    p_group_list[num_groups++] = element->model[model_idx].pub.address;
                }
            }
            else
            {
                // device is configured to publish messages to unicast address.  The device is
                // a member of a group if destination address is part of that group
                uint16_t *p_pub_group_list = wiced_bt_mesh_db_get_element_group_list(mesh_db, element_addr);
                for (j = 0; (p_pub_group_list != NULL) && (p_pub_group_list[j] != 0); j++)
                {
                    for (i = 0; i < num_groups; i++)
                    {
                        if (p_group_list[i] == p_pub_group_list[j])
                            break;
                    }
                    if (i == num_groups)
                    {
                        p_group_list[num_groups++] = p_pub_group_list[j];
                    }
                }
                if (p_pub_group_list)
                    wiced_bt_free_buffer(p_pub_group_list);
            }
        }
    }
    return p_group_list;
}

/*
 * check if the element is in the group.  The element is considered to be in the group if one of its models is subscribed to
 * receive packets destined for the group, or if one of its models is configured to publish messages to the group address or
 * to a device which belongs to this group.
 */
wiced_bool_t wiced_bt_mesh_db_element_is_in_group(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, uint16_t group_addr)
{
    wiced_bt_mesh_db_element_t *element = element_get_by_addr(mesh_db, element_addr);
    int sub_idx;
    int model_idx;
    uint16_t publish_addr = 0;
    int num_subscriptions = 0;
    wiced_bool_t subscribed_to_group = WICED_FALSE;

    if (element == NULL)
        return WICED_FALSE;

    // If group address is 0xffff, means the appp is just checking if that element is in the database
    if (group_addr == 0xFFFF)
        return WICED_TRUE;

    for (model_idx = 0; model_idx < element->num_models; model_idx++)
    {
        num_subscriptions += element->model[model_idx].num_subs;
        for (sub_idx = 0; sub_idx < element->model[model_idx].num_subs; sub_idx++)
        {
            if (group_addr == element->model[model_idx].sub[sub_idx])
                subscribed_to_group = WICED_TRUE;
            else
            {
                // If we are subscribed to a child group of the group in question, no go.
                if (is_group_a_perent_group(mesh_db, group_addr, element->model[model_idx].sub[sub_idx]))
                    return WICED_FALSE;
            }
        }
        if (element->model[model_idx].pub.address == group_addr)
            return WICED_TRUE;
    }
    if ((num_subscriptions == 0) && (publish_addr == 0) && (group_addr == 0))
        return WICED_TRUE;

    if (subscribed_to_group)
        return WICED_TRUE;

    for (model_idx = 0; model_idx < element->num_models; model_idx++)
    {
        if (element->model[model_idx].pub.address != 0)
        {
            if (wiced_bt_mesh_db_element_is_in_group(mesh_db, element->model[model_idx].pub.address, group_addr))
                return WICED_TRUE;
        }
    }
    return WICED_FALSE;
}

uint16_t wiced_bt_mesh_db_num_nodes(wiced_bt_mesh_db_mesh_t *mesh_db)
{
    return mesh_db->num_nodes;
}

wiced_bt_mesh_db_node_t *wiced_bt_mesh_db_node_get_by_addr(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t node_addr)
{
    int i;

    for (i = 0; i < mesh_db->num_nodes; i++)
    {
        if (mesh_db->node[i].num_elements == 0)
            continue;

        if (mesh_db->node[i].unicast_address == node_addr)
        {
            return &mesh_db->node[i];
        }
    }
    return NULL;
}

wiced_bt_mesh_db_node_t* wiced_bt_mesh_db_node_get_by_uuid(wiced_bt_mesh_db_mesh_t* mesh_db, uint8_t* p_uuid)
{
    int i;

    for (i = 0; i < mesh_db->num_nodes; i++)
    {
        if (memcmp(mesh_db->node[i].uuid, p_uuid, WICED_MESH_DB_UUID_SIZE) == 0)
        {
            return &mesh_db->node[i];
        }
    }
    return NULL;
}

wiced_bt_mesh_db_node_t *wiced_bt_mesh_db_node_get_by_element_addr(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t node_addr)
{
    int node_idx;

    for (node_idx = 0; node_idx < mesh_db->num_nodes; node_idx++)
    {
        if ((mesh_db->node[node_idx].unicast_address <= node_addr) &&
            (mesh_db->node[node_idx].unicast_address + mesh_db->node[node_idx].num_elements > node_addr))
        {
            return &mesh_db->node[node_idx];
        }
    }
    return NULL;
}

wiced_bt_mesh_db_node_t *wiced_bt_mesh_db_node_get_by_element_name(wiced_bt_mesh_db_mesh_t *mesh_db, const char *name)
{
    int node_idx;
    int elem_idx;

    for (node_idx = 0; node_idx < mesh_db->num_nodes; node_idx++)
    {
        for (elem_idx = 0; elem_idx < mesh_db->node[node_idx].num_elements; elem_idx++)
        {
            if ((mesh_db->node[node_idx].element[elem_idx].name != NULL) &&
                (strcmp(mesh_db->node[node_idx].element[elem_idx].name, name) == 0))
            {
                return &mesh_db->node[node_idx];
            }
        }
    }
    return NULL;
}

/*
 * Create node function can be called at the end of the provisioning.  At this time
 * provisioner knows only address and device key
 */
wiced_bt_mesh_db_node_t *wiced_bt_mesh_db_node_create(wiced_bt_mesh_db_mesh_t *mesh_db, const char *name, uint16_t node_addr, uint8_t *uuid, uint8_t num_elements, uint8_t *dev_key, uint16_t net_key_index)
{
    int i;
    wiced_bt_mesh_db_node_t *p_temp;
    wiced_bt_mesh_db_node_t node;

    memset(&node, 0, sizeof(node));
    node.name = copy_name(name);
    memcpy(node.uuid, uuid, sizeof(node.uuid));
    memcpy(node.device_key, dev_key, sizeof(node.uuid));
    node.unicast_address = node_addr;

    node.num_net_keys = 1;
    node.net_key = (wiced_bt_mesh_db_key_idx_phase *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_key_idx_phase));
    if (node.net_key == NULL)
    {
        wiced_bt_free_buffer(node.name);
        return NULL;
    }
    node.net_key[0].index = net_key_index;
    node.net_key[0].phase = 0;

    node.num_elements = num_elements;
    if (num_elements)
    {
        if ((node.element = (wiced_bt_mesh_db_element_t*)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_element_t) * num_elements)) == NULL)
            return NULL;

        memset(node.element, 0, sizeof(wiced_bt_mesh_db_element_t) * num_elements);
        for (i = 0; i < num_elements; i++)
        {
            node.element[i].index = i;
        }
    }
    node.feature.relay = MESH_FEATURE_SUPPORTED_UNKNOWN;
    node.feature.gatt_proxy = MESH_FEATURE_SUPPORTED_UNKNOWN;
    node.feature.low_power = MESH_FEATURE_SUPPORTED_UNKNOWN;
    node.feature.friend = MESH_FEATURE_SUPPORTED_UNKNOWN;
    node.beacon = MESH_FEATURE_SUPPORTED_UNKNOWN;
    node.default_ttl = MESH_FEATURE_SUPPORTED_UNKNOWN;
    node.network_transmit.count = MESH_FEATURE_SUPPORTED_UNKNOWN;
    node.relay_rexmit.count = MESH_FEATURE_SUPPORTED_UNKNOWN;
    node.security = 1;

    p_temp = mesh_db->node;
    mesh_db->node = (wiced_bt_mesh_db_node_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_node_t) * (mesh_db->num_nodes + 1));
    if (mesh_db->node == NULL)
    {
        wiced_bt_free_buffer(node.name);
        wiced_bt_free_buffer(node.element);
        return NULL;
    }
    if (p_temp != 0)
    {
        memcpy(mesh_db->node, p_temp, sizeof(wiced_bt_mesh_db_node_t) * mesh_db->num_nodes);
        wiced_bt_free_buffer(p_temp);
    }
    memcpy(&mesh_db->node[mesh_db->num_nodes], &node, sizeof(wiced_bt_mesh_db_node_t));
    mesh_db->num_nodes++;
    return &mesh_db->node[mesh_db->num_nodes - 1];
}

/*
 * Mark node as blocked
 */
wiced_bool_t wiced_bt_mesh_db_node_block(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t node_addr)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_addr(mesh_db, node_addr);
    if (node != NULL)
    {
        node->blocked = 1;
        return WICED_TRUE;
    }
    return WICED_FALSE;
}

void free_node(wiced_bt_mesh_db_node_t *node)
{
    int elem_idx, model_idx, sensor_idx, setting_idx;

    if (node->name != NULL)
    {
        wiced_bt_free_buffer(node->name);
        node->name = NULL;
    }
    if (node->num_net_keys != 0)
    {
        node->num_net_keys = 0;
        if (node->net_key != NULL)
        {
            wiced_bt_free_buffer(node->net_key);
            node->net_key = NULL;
        }
    }
    if (node->num_app_keys != 0)
    {
        node->num_app_keys = 0;
        if (node->app_key != NULL)
        {
            wiced_bt_free_buffer(node->app_key);
            node->app_key = NULL;
        }
    }
    if (node->element != NULL)
    {
        for (elem_idx = 0; elem_idx < node->num_elements; elem_idx++)
        {
            if (node->element[elem_idx].model != NULL)
            {
                for (model_idx = 0; model_idx < node->element[elem_idx].num_models; model_idx++)
                {
                    if (node->element[elem_idx].model[model_idx].bound_key != NULL)
                    {
                        wiced_bt_free_buffer(node->element[elem_idx].model[model_idx].bound_key);
                        node->element[elem_idx].model[model_idx].bound_key = NULL;
                    }
                    if (node->element[elem_idx].model[model_idx].sub != NULL)
                    {
                        wiced_bt_free_buffer(node->element[elem_idx].model[model_idx].sub);
                        node->element[elem_idx].model[model_idx].sub = NULL;
                    }
                    if (node->element[elem_idx].model[model_idx].sensor != NULL)
                    {
                        for (sensor_idx = 0; sensor_idx < node->element[elem_idx].model[model_idx].num_sensors; sensor_idx++)
                        {
                            if (node->element[elem_idx].model[model_idx].sensor->settings != NULL)
                            {
                                for (setting_idx = 0; setting_idx < node->element[elem_idx].model[model_idx].sensor[sensor_idx].num_settings; setting_idx++)
                                {
                                    if (node->element[elem_idx].model[model_idx].sensor[sensor_idx].settings[setting_idx].val != NULL)
                                    {
                                        wiced_bt_free_buffer(node->element[elem_idx].model[model_idx].sensor[sensor_idx].settings[setting_idx].val);
                                        node->element[elem_idx].model[model_idx].sensor[sensor_idx].settings[setting_idx].val = NULL;
                                    }
                                }
                                wiced_bt_free_buffer(node->element[elem_idx].model[model_idx].sensor[sensor_idx].settings);
                                node->element[elem_idx].model[model_idx].sensor[sensor_idx].settings = NULL;
                            }
                        }
                        wiced_bt_free_buffer(node->element[elem_idx].model[model_idx].sensor);
                        node->element[elem_idx].model[model_idx].sensor = NULL;
                    }
                }
                wiced_bt_free_buffer(node->element[elem_idx].model);
                node->element[elem_idx].model = NULL;
            }
            if (node->element[elem_idx].name != NULL)
            {
                wiced_bt_free_buffer(node->element[elem_idx].name);
                node->element[elem_idx].name = NULL;
            }
        }
        wiced_bt_free_buffer(node->element);
        node->element = NULL;
    }
}

/*
 * Delete node from the database
 */
wiced_bool_t wiced_bt_mesh_db_node_remove(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t node_addr)
{
    int i;

    for (i = 0; i < mesh_db->num_nodes; i++)
    {
        if (mesh_db->node[i].num_elements == 0)
            continue;

        if (mesh_db->node[i].unicast_address == node_addr)
        {
            free_node(&mesh_db->node[i]);

            if (mesh_db->num_nodes == 1)
            {
                mesh_db->num_nodes = 0;
                wiced_bt_free_buffer(mesh_db->node);
                mesh_db->node = NULL;
            }
            else
            {
                for (; i < mesh_db->num_nodes - 1; i++)
                {
                    memcpy(&mesh_db->node[i], &mesh_db->node[i + 1], sizeof(wiced_bt_mesh_db_node_t));
                }
                mesh_db->num_nodes--;
            }
            return WICED_TRUE;
        }
    }
    return WICED_FALSE;
}

/*
 * return true if the node is provisioner
 */
wiced_bool_t wiced_bt_mesh_db_is_provisioner(wiced_bt_mesh_db_mesh_t* mesh_db, wiced_bt_mesh_db_node_t* p_node)
{
    int i;

    for (i = 0; i < mesh_db->num_provisioners; i++)
    {
        if (memcmp(mesh_db->provisioner[i].uuid, p_node->uuid, sizeof(p_node->uuid)) == 0)
        {
            return WICED_TRUE;
        }
    }
    return WICED_FALSE;
}

uint16_t *wiced_bt_mesh_db_get_all_elements(wiced_bt_mesh_db_mesh_t *mesh_db)
{
    int i, j;
    uint16_t num_elements = 0;
    uint16_t *p_element_array = NULL;

    for (i = 0; i < mesh_db->num_nodes; i++)
    {
        num_elements += mesh_db->node[i].num_elements;
    }
    if ((p_element_array = (uint16_t *)wiced_bt_get_buffer(sizeof(uint16_t) * (num_elements + 1))) == NULL)
        return NULL;

    memset(p_element_array, 0, sizeof(uint16_t) * (num_elements + 1));
    num_elements = 0;
    for (i = 0; i < mesh_db->num_nodes; i++)
    {
        if (mesh_db->node[i].num_elements == 0)
            continue;

        // skip the node if it belongs to provisioner
        wiced_bool_t is_provisioner = WICED_FALSE;
        for (j = 0; j < mesh_db->num_provisioners; j++)
        {
            if (memcmp(mesh_db->provisioner[j].uuid, mesh_db->node[i].uuid, sizeof(mesh_db->node[i].uuid)) == 0)
            {
                is_provisioner = WICED_TRUE;
                break;
            }
        }
        if (is_provisioner)
            continue;

        for (j = 0; j < mesh_db->node[i].num_elements; j++)
        {
            p_element_array[num_elements++] = mesh_db->node[i].unicast_address + j;
        }
    }

    p_element_array[num_elements] = 0;
    return p_element_array;
}

wiced_bt_mesh_db_element_t *wiced_bt_mesh_db_get_element(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr)
{
    int node_idx;

    for (node_idx = 0; node_idx < mesh_db->num_nodes; node_idx++)
    {
        if ((mesh_db->node[node_idx].unicast_address <= element_addr) &&
            (mesh_db->node[node_idx].unicast_address + mesh_db->node[node_idx].num_elements > element_addr))
        {
            return &mesh_db->node[node_idx].element[element_addr - mesh_db->node[node_idx].unicast_address];
        }
    }
    return NULL;
}

uint16_t wiced_bt_mesh_db_get_node_addr(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr)
{
    int node_idx;

    for (node_idx = 0; node_idx < mesh_db->num_nodes; node_idx++)
    {
        if ((mesh_db->node[node_idx].unicast_address <= element_addr) &&
            (mesh_db->node[node_idx].unicast_address + mesh_db->node[node_idx].num_elements > element_addr))
        {
            return mesh_db->node[node_idx].unicast_address;
        }
    }
    return 0;
}

uint16_t *wiced_bt_mesh_db_get_device_elements(wiced_bt_mesh_db_mesh_t *mesh_db, uint8_t *p_uuid)
{
    int i, j;
    uint16_t num_elements = 0;
    uint16_t *p_element_array = NULL;

    for (i = 0; i < mesh_db->num_nodes; i++)
    {
        if (mesh_db->node[i].blocked)
            continue;

        if (memcmp(p_uuid, mesh_db->node[i].uuid, sizeof(mesh_db->node[i].uuid)) == 0)
        {
            if ((p_element_array = (uint16_t *)wiced_bt_get_buffer(sizeof(uint16_t) * (mesh_db->node[i].num_elements + 1))) == NULL)
                return NULL;

            memset(p_element_array, 0, sizeof(uint16_t) * (mesh_db->node[i].num_elements + 1));
            for (j = 0; j < mesh_db->node[i].num_elements; j++)
            {
                p_element_array[num_elements++] = mesh_db->node[i].unicast_address + j;
            }
            p_element_array[num_elements] = 0;
            return p_element_array;
        }
    }
    return NULL;
}

const char *wiced_bt_mesh_db_get_element_name(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t addr)
{
    wiced_bt_mesh_db_element_t *element = wiced_bt_mesh_db_get_element(mesh_db, addr);
    return (element != NULL) ? element->name : NULL;
}

void wiced_bt_mesh_db_set_element_name(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t addr, const char *p_element_name)
{
    wiced_bt_mesh_db_element_t *element = wiced_bt_mesh_db_get_element(mesh_db, addr);
    char* name;
    if (element != NULL)
    {
        wiced_bt_free_buffer(element->name);
        name = (char *)wiced_bt_get_buffer((uint16_t)(strlen(p_element_name) + 8));
        sprintf(name, "%s (%04x)", p_element_name, addr);
        element->name = copy_name(name);
        wiced_bt_free_buffer(name);
    }
}

wiced_bool_t wiced_bt_mesh_db_is_model_subscribed_to_group(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, uint16_t company_id, uint16_t model_id, uint16_t in_group_addr)
{
    wiced_bt_mesh_db_element_t *element = wiced_bt_mesh_db_get_element(mesh_db, element_addr);
    wiced_bt_mesh_db_model_id_t *p_models = NULL;
    int index;
    int model_idx;
    wiced_bool_t model_in_group = WICED_FALSE;

    if (element == NULL)
        return WICED_FALSE;

    if ((p_models = (wiced_bt_mesh_db_model_id_t *)malloc(sizeof(wiced_bt_mesh_db_model_id_t) * (element->num_models + 1))) == NULL)
        return WICED_FALSE;

    memset(p_models, 0, (sizeof(wiced_bt_mesh_db_model_id_t) * (element->num_models + 1)));

    for (model_idx = 0; model_idx < element->num_models; model_idx++)
    {
        if ((model_id == element->model[model_idx].model.id) &&
            (company_id == element->model[model_idx].model.company_id) &&
            (model_id == element->model[model_idx].model.id))
        {
            model_in_group = WICED_FALSE;
            // go through all subscriptions
            for (index = 0; index < element->model[model_idx].num_subs; index++)
            {
                // TBD add support for virtual addresses
                if (element->model[model_idx].sub[index] == in_group_addr)
                {
                    model_in_group = WICED_TRUE;
                    break;
                }
            }
            break;
        }
    }
    free(p_models);
    return model_in_group;
}

wiced_bt_mesh_db_model_id_t *wiced_bt_mesh_db_get_all_models_of_element(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, uint16_t in_group_addr)
{
    wiced_bt_mesh_db_element_t *element = wiced_bt_mesh_db_get_element(mesh_db, element_addr);
    int model_idx, num_models = 0;
    wiced_bt_mesh_db_model_id_t *p_models = NULL;
    int index;
    wiced_bool_t model_in_group;

    if ((element = element_get_by_addr(mesh_db, element_addr)) == NULL)
        return NULL;

    if ((p_models = (wiced_bt_mesh_db_model_id_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_model_id_t) * (element->num_models + 1))) == NULL)
        return NULL;

    memset(p_models, 0, (sizeof(wiced_bt_mesh_db_model_id_t) * (element->num_models + 1)));

    for (model_idx = 0; model_idx < element->num_models; model_idx++)
    {
        model_in_group = WICED_FALSE;

        if (in_group_addr == 0)
        {
            model_in_group = WICED_TRUE;
        }
        else
        {
            if (element->model[model_idx].pub.address == in_group_addr)
            {
                model_in_group = WICED_TRUE;
            }
            else
            {
                // go through all subscriptions
                for (index = 0; index < element->model[model_idx].num_subs; index++)
                {
                    // TBD add support for virtual addresses
                    if (element->model[model_idx].sub[index] == in_group_addr)
                    {
                        model_in_group = WICED_TRUE;
                        break;
                    }
                }
            }
        }
        if (model_in_group)
        {
            p_models[num_models].company_id = element->model[model_idx].model.company_id;
            p_models[num_models].id = element->model[model_idx].model.id;
            num_models++;
        }
    }
    p_models[num_models].company_id = MESH_COMPANY_ID_UNUSED;
    p_models[num_models].id = 0xFFFF;
    return p_models;
}

uint8_t composition_data_get_num_elements(uint8_t *p_composition_data, uint16_t len)
{
    uint8_t elem_idx = 0;

    if (len < 10)
        return 0;

    p_composition_data += 10;
    len -= 10;

    while (len)
    {
        if (len < 4)
            return elem_idx;

        uint16_t location = p_composition_data[0] + (p_composition_data[1] << 8);
        uint8_t num_models = p_composition_data[2];
        uint8_t num_vs_models = p_composition_data[3];

        p_composition_data += 4;
        len -= 4;

        if (len < 2 * num_models + 4 * num_vs_models)
            return elem_idx;

        len -= (2 * num_models + 4 * num_vs_models);
        p_composition_data += (2 * num_models + 4 * num_vs_models);

        elem_idx++;
    }
    return elem_idx;
}

wiced_bool_t wiced_bt_mesh_db_node_set_composition_data(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t node_addr, uint8_t *comp_data, uint16_t comp_data_len)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_element_addr(mesh_db, node_addr);
    uint8_t num_elements = composition_data_get_num_elements(comp_data, comp_data_len);
    uint8_t  *p = comp_data;
    uint8_t elem_idx, model_idx;
    uint8_t num_models, num_vs_models;
    uint16_t features;

    if (node == NULL)
        return WICED_FALSE;

    if (node->element != NULL)
    {
        for (elem_idx = 0; elem_idx < node->num_elements; elem_idx++)
        {
            if (node->element[elem_idx].model != NULL)
            {
                for (model_idx = 0; model_idx < node->element[elem_idx].num_models; model_idx++)
                {
                    if (node->element[elem_idx].model[model_idx].bound_key != NULL)
                        wiced_bt_free_buffer(node->element[elem_idx].model[model_idx].bound_key);

                    if (node->element[elem_idx].model[model_idx].sub != NULL)
                        wiced_bt_free_buffer(node->element[elem_idx].model[model_idx].sub);
                }
                wiced_bt_free_buffer(node->element[elem_idx].model);
            }
            wiced_bt_free_buffer(node->element[elem_idx].name);
        }
        wiced_bt_free_buffer(node->element);
        node->element = NULL;
    }
    node->num_elements = num_elements;
    if (num_elements)
    {
        node->element = (wiced_bt_mesh_db_element_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_element_t) * num_elements);
        memset(node->element, 0, sizeof(wiced_bt_mesh_db_element_t) * num_elements);
        for (elem_idx = 0; elem_idx < num_elements; elem_idx++)
        {
            node->element[elem_idx].index = elem_idx;
            node->element[elem_idx].name = (char *)wiced_bt_get_buffer((uint16_t)(strlen(node->name) + 8));
            if (node->element[elem_idx].name != NULL)
                sprintf(node->element[elem_idx].name, "%s (%04x)", node->name, node->unicast_address + elem_idx);
        }
    }

    node->cid = p[0] + (p[1] << 8);
    node->pid = p[2] + (p[3] << 8);
    node->vid = p[4] + (p[5] << 8);
    node->crpl = p[6] + (p[7] << 8);

    features = p[8] + (p[9] << 8);
    node->feature.relay      = (features & FOUNDATION_FEATURE_BIT_RELAY)  ? MESH_FEATURE_SUPPORTED_UNKNOWN : MESH_FEATURE_UNSUPPORTED;
    node->feature.gatt_proxy = (features & FOUNDATION_FEATURE_BIT_PROXY)  ? MESH_FEATURE_SUPPORTED_UNKNOWN : MESH_FEATURE_UNSUPPORTED;
    node->feature.friend     = (features & FOUNDATION_FEATURE_BIT_FRIEND) ? MESH_FEATURE_SUPPORTED_UNKNOWN : MESH_FEATURE_UNSUPPORTED;
    node->feature.low_power  = (features & FOUNDATION_FEATURE_BIT_LOW_POWER) ? MESH_FEATURE_ENABLED : MESH_FEATURE_UNSUPPORTED; // lpn is not configurable, it is either on or unsupported

    p += 10;

    for (elem_idx = 0; elem_idx < num_elements; elem_idx++)
    {
        node->element[elem_idx].location = p[0] + (p[1] << 8);
        num_models = p[2];
        num_vs_models = p[3];

        p += 4;

        if ((num_models != 0) || (num_vs_models != 0))
        {
            node->element[elem_idx].model = (wiced_bt_mesh_db_model_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_model_t) * (num_models + num_vs_models));
            if (node->element[elem_idx].model == NULL)
            {
                // TBD delete node
                return WICED_FALSE;
            }
            memset(node->element[elem_idx].model, 0, sizeof(wiced_bt_mesh_db_model_t) * (num_models + num_vs_models));

            for (model_idx = 0; model_idx < num_models; model_idx++)
            {
                node->element[elem_idx].model[model_idx].model.company_id = MESH_COMPANY_ID_BT_SIG;
                node->element[elem_idx].model[model_idx].model.id = p[0] + (p[1] << 8);
                p += 2;
            }
            for (model_idx = 0; model_idx < num_vs_models; model_idx++)
            {
                node->element[elem_idx].model[model_idx + num_models].model.company_id = p[0] + (p[1] << 8);
                node->element[elem_idx].model[model_idx + num_models].model.id = p[2] + (p[3] << 8);
                p += 4;
            }
            node->element[elem_idx].num_models = num_models + num_vs_models;
        }
    }
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_db_node_check_composition_data(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t node_addr, uint8_t *comp_data, uint16_t comp_data_len)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_element_addr(mesh_db, node_addr);
    uint8_t num_elements = composition_data_get_num_elements(comp_data, comp_data_len);
    uint8_t  *p = comp_data;
    uint8_t elem_idx, model_idx;
    uint8_t num_models, num_vs_models;
    uint16_t features;

    if (node == NULL)
        return WICED_FALSE;

    if ((node->num_elements != num_elements) ||
        (node->cid != p[0] + (p[1] << 8)) ||
        (node->pid != p[2] + (p[3] << 8)) ||
        (node->vid != p[4] + (p[5] << 8)) ||
        (node->crpl != p[6] + (p[7] << 8)))
        return WICED_FALSE;

    features = p[8] + (p[9] << 8);
    if (((node->feature.relay != MESH_FEATURE_UNSUPPORTED) != ((features & FOUNDATION_FEATURE_BIT_RELAY) == FOUNDATION_FEATURE_BIT_RELAY)) ||
        ((node->feature.gatt_proxy != MESH_FEATURE_UNSUPPORTED) != ((features & FOUNDATION_FEATURE_BIT_PROXY) == FOUNDATION_FEATURE_BIT_PROXY)) ||
        ((node->feature.low_power != MESH_FEATURE_UNSUPPORTED) != ((features & FOUNDATION_FEATURE_BIT_LOW_POWER) == FOUNDATION_FEATURE_BIT_LOW_POWER)) ||
        ((node->feature.friend != MESH_FEATURE_UNSUPPORTED) != ((features & FOUNDATION_FEATURE_BIT_FRIEND) == FOUNDATION_FEATURE_BIT_FRIEND)))
        return WICED_FALSE;

    if (num_elements == 0)
        return WICED_TRUE;

    p += 10;

    for (elem_idx = 0; elem_idx < num_elements; elem_idx++)
    {
        if (node->element[elem_idx].location != p[0] + (p[1] << 8))
            return WICED_FALSE;

        num_models = p[2];
        num_vs_models = p[3];

        if (node->element[elem_idx].num_models != (num_models + num_vs_models))
            return WICED_FALSE;

        p += 4;

        if ((num_models != 0) || (num_vs_models != 0))
        {
            for (model_idx = 0; model_idx < num_models; model_idx++)
            {
                if ((node->element[elem_idx].model[model_idx].model.company_id != MESH_COMPANY_ID_BT_SIG) ||
                    (node->element[elem_idx].model[model_idx].model.id != p[0] + (p[1] << 8)))
                    return WICED_FALSE;
                p += 2;
            }
            for (model_idx = 0; model_idx < num_vs_models; model_idx++)
            {
                if ((node->element[elem_idx].model[model_idx + num_models].model.company_id != p[0] + (p[1] << 8)) ||
                    (node->element[elem_idx].model[model_idx + num_models].model.id != p[2] + (p[3] << 8)))
                    return WICED_FALSE;
                p += 4;
            }
        }
    }
    return WICED_TRUE;
}

// Find net key with the specified net key index
static wiced_bt_mesh_db_net_key_t *find_net_key(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t net_key_index)
{
    wiced_bt_mesh_db_net_key_t *net_key = NULL;
    int i;

    for (i = 0; i < wiced_bt_mesh_db_num_net_keys(mesh_db); i++)
    {
        net_key = wiced_bt_mesh_db_net_key_get(mesh_db, i);
        if ((net_key != NULL) && (net_key->index == net_key_index))
            break;
    }
    return net_key;
}

wiced_bool_t wiced_bt_mesh_db_node_net_key_add(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint16_t net_key_idx)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_element_addr(mesh_db, unicast_addr);
    wiced_bt_mesh_db_net_key_t *net_key = find_net_key(mesh_db, net_key_idx);
    wiced_bt_mesh_db_key_idx_phase key;
    int i;

    if ((node == NULL) || (net_key == NULL))
        return WICED_FALSE;

    for (i = 0; i < node->num_net_keys; i++)
    {
        if (node->net_key[i].index == net_key_idx)
        {
            if (node->net_key[i].phase == 0)
                node->net_key[i].phase = 1;
            return WICED_TRUE;
        }
    }
    key.index = net_key_idx;
    key.phase = net_key->phase;
    return mesh_db_add_node_net_key(node, &key);
}

// Find app key with the specified app key index
static wiced_bt_mesh_db_app_key_t *find_app_key(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t app_key_index)
{
    wiced_bt_mesh_db_app_key_t *app_key = NULL;
    int i;

    for (i = 0; i < wiced_bt_mesh_db_num_app_keys(mesh_db); i++)
    {
        app_key = wiced_bt_mesh_db_app_key_get(mesh_db, i);
        if ((app_key != NULL) && (app_key->index == app_key_index))
            break;
    }
    return app_key;
}

wiced_bool_t wiced_bt_mesh_db_node_app_key_add(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint16_t net_key_idx, uint16_t app_key_idx)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_element_addr(mesh_db, unicast_addr);
    wiced_bt_mesh_db_app_key_t *app_key = find_app_key(mesh_db, app_key_idx);
    wiced_bt_mesh_db_net_key_t *net_key = wiced_bt_mesh_db_find_bound_net_key(mesh_db, app_key);
    wiced_bt_mesh_db_key_idx_phase key;
    int i;

    if ((node == NULL) || (net_key == NULL) || (app_key == NULL))
        return WICED_FALSE;

    for (i = 0; i < node->num_app_keys; i++)
    {
        if (node->app_key[i].index == app_key_idx)
        {
            if (node->app_key[i].phase == 0)
                node->app_key[i].phase = 1;
            return WICED_TRUE;
        }
    }
    key.index = app_key_idx;
    key.phase = net_key->phase;
    return mesh_db_add_node_app_key(node, &key);
}

wiced_bt_mesh_db_net_key_t *wiced_bt_mesh_db_find_bound_net_key(wiced_bt_mesh_db_mesh_t *mesh_db, wiced_bt_mesh_db_app_key_t *app_key)
{
    wiced_bt_mesh_db_net_key_t *net_key;
    int i;

    if (app_key == NULL)
        return NULL;

    // first net key was added during provisiong.  Need to refresh if the phase is not 0
    for (i = 0; i < wiced_bt_mesh_db_num_net_keys(mesh_db); i++)
    {
        net_key = wiced_bt_mesh_db_net_key_get(mesh_db, i);
        if ((net_key != NULL) && (app_key->bound_net_key_index == net_key->index))
            return net_key;
    }
    return NULL;
}

wiced_bool_t wiced_bt_mesh_db_node_net_key_update(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint16_t net_key_idx, uint8_t phase)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_element_addr(mesh_db, unicast_addr);
    int i;

    if (node == NULL)
        return WICED_FALSE;

    // find index of the net key for net_key_idx
    for (i = 0; i < node->num_net_keys; i++)
    {
        if (node->net_key[i].index == net_key_idx)
        {
            node->net_key[i].phase = phase;
            return WICED_TRUE;
        }
    }
    return WICED_FALSE;
}

wiced_bool_t wiced_bt_mesh_db_node_app_key_update(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint16_t net_key_idx, uint16_t app_key_idx)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_element_addr(mesh_db, unicast_addr);
    int i;

    if (node == NULL)
        return WICED_FALSE;

    for (i = 0; i < node->num_app_keys; i++)
    {
        if (node->app_key[i].index == app_key_idx)
        {
            node->app_key[i].phase = 1;
            return WICED_TRUE;
        }
    }
    return WICED_FALSE;
}

wiced_bt_mesh_db_model_t *find_node_model(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, uint16_t company_id, uint16_t model_id)
{
    wiced_bt_mesh_db_element_t *element = wiced_bt_mesh_db_get_element(mesh_db, element_addr);
    int model_idx;

    if (element == NULL)
    {
        Log("find_node_model element is not present\n");
        return NULL;
    }
    for (model_idx = 0; model_idx < element->num_models; model_idx++)
    {
        if ((company_id == element->model[model_idx].model.company_id) &&
            (model_id == element->model[model_idx].model.id))
            return &element->model[model_idx];
    }
    return NULL;
}

wiced_bt_mesh_db_sensor_t *find_model_sensor(wiced_bt_mesh_db_model_t *model, uint16_t property_id)
{
    int sensor_idx;
    if (property_id == 0)
    {
        Log("property id 0 is prohibited!");
        return NULL;
    }
    for (sensor_idx = 0; sensor_idx < model->num_sensors; sensor_idx++)
    {
        if (property_id == model->sensor[sensor_idx].property_id)
           return &model->sensor[sensor_idx];
    }
    return NULL;
}

wiced_bool_t wiced_bt_mesh_db_node_model_app_bind_add(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint16_t element_addr, uint16_t company_id, uint16_t model_id, uint16_t app_key_idx)
{
    wiced_bt_mesh_db_model_t *model = find_node_model(mesh_db, element_addr, company_id, model_id);

    if (model == NULL)
        return WICED_FALSE;

    return mesh_db_add_model_app_bind(model, app_key_idx);
}

wiced_bool_t wiced_bt_mesh_db_sensor_property_present(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, uint16_t property_id)
{
    wiced_bt_mesh_db_model_t *model = find_node_model(mesh_db, element_addr, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV);
    int i;

    if (model != NULL)
    {
        for (i = 0; i < model->num_sensors; i++)
        {
            if (model->sensor[i].property_id == property_id)
            {
                return WICED_TRUE;
            }
        }
    }
    return WICED_FALSE;
}

int *wiced_bt_mesh_db_node_sensor_get_property_ids(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr)
{
    wiced_bt_mesh_db_model_t *model = find_node_model(mesh_db, element_addr, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV);
    int i;
    int *p_properties;

    if (model == NULL)
        return NULL;

    p_properties = (int *)wiced_bt_get_buffer(sizeof(int) * (model->num_sensors + 1));
    if (p_properties == NULL)
        return NULL;

    memset(p_properties, 0, sizeof(int) * (model->num_sensors + 1));
    for (i = 0; i < model->num_sensors; i++)
    {
        p_properties[i] = model->sensor[i].property_id;
    }
    p_properties[i] = 0;
    return p_properties;

}

wiced_bool_t wiced_bt_mesh_db_sensor_descriptor_add(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, wiced_bt_mesh_sensor_descriptor_status_data_t *data)
{
    wiced_bt_mesh_db_model_t *model = find_node_model(mesh_db, element_addr, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV);
    int i;

    if (model == NULL)
    {
        Log("did not find sensor model\n");
        return WICED_FALSE;
    }
    if (model->sensor != NULL)
    {
        Log("sensors already present\n");
        return WICED_FALSE;
    }
    for (i = 0; i < data->num_descriptors; i++)
    {
        //property id 0 is prohibited
        if ((data->descriptor_list[i].property_id == 0) || (data->descriptor_list[i].property_id > WICED_BT_MESH_MAX_PROPERTY_ID))
        {
            Log("bad prop id:%x\n", data->descriptor_list[i].property_id);
            return WICED_FALSE;
        }
    }

    model->sensor = (wiced_bt_mesh_db_sensor_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_sensor_t) * data->num_descriptors);
    if (model->sensor == NULL)
        return WICED_FALSE;

    memset(model->sensor, 0, sizeof(wiced_bt_mesh_db_sensor_t) * data->num_descriptors);

    for (i = 0; i < data->num_descriptors; i++)
    {
        model->sensor[model->num_sensors].descriptor.positive_tolerance_percentage = (uint8_t)((data->descriptor_list[i].positive_tolerance * 100 + 2048) / 4095);
        model->sensor[model->num_sensors].descriptor.negative_tolerance_percentage = (uint8_t)((data->descriptor_list[i].negative_tolerance * 100 + 2048) / 4095);
        model->sensor[model->num_sensors].descriptor.sampling_function = data->descriptor_list[i].sampling_function;
        model->sensor[model->num_sensors].descriptor.measurement_period = data->descriptor_list[i].measurement_period;
        model->sensor[model->num_sensors].descriptor.update_interval = data->descriptor_list[i].update_interval;
        model->sensor[model->num_sensors].property_id = data->descriptor_list[i].property_id;
        model->sensor[model->num_sensors].prop_value_len = wiced_bt_mesh_property_len[data->descriptor_list[i].property_id];
        model->num_sensors++;
    }
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_db_sensor_settings_add(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, wiced_bt_mesh_sensor_settings_status_data_t *data)
{
    wiced_bt_mesh_db_model_t *model = find_node_model(mesh_db, element_addr, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV);
    wiced_bt_mesh_db_sensor_t *sensor;
    int  i;

    Log("wiced_bt_mesh_db_sensor_settings_add\n");

    if ((model == NULL) || (model->sensor == NULL))
    {
        Log("did not find model returning false\n");
        return WICED_FALSE;
    }

    sensor = find_model_sensor(model, data->property_id);
    if (sensor == NULL)
        return WICED_FALSE;

    if (sensor->settings != NULL)
    {
        for (i = 0; i < sensor->num_settings; i++)
        {
            if (sensor->settings[i].val != NULL)
            {
                wiced_bt_free_buffer(sensor->settings[i].val);
                sensor->settings[i].val = NULL;
            }
        }
        wiced_bt_free_buffer(sensor->settings);
    }
    sensor->settings = (wiced_bt_mesh_db_setting_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_setting_t) * data->num_setting_property_id);
    if (sensor->settings == NULL)
        return WICED_FALSE;

    memset(sensor->settings, 0, sizeof(wiced_bt_mesh_db_setting_t) * data->num_setting_property_id);
    sensor->num_settings = 0;

    for (i = 0; i < data->num_setting_property_id; i++)
    {
        if (wiced_bt_mesh_property_len[data->setting_property_id_list[i]] != 0)
        {
            sensor->settings[sensor->num_settings].val = (uint8_t *)wiced_bt_get_buffer(wiced_bt_mesh_property_len[data->setting_property_id_list[i]]);
            if (sensor->settings[sensor->num_settings].val == NULL)
            {
                wiced_bt_free_buffer(sensor->settings);
                sensor->settings = NULL;
                return WICED_FALSE;
            }
            memset(sensor->settings[sensor->num_settings].val, 0, wiced_bt_mesh_property_len[data->setting_property_id_list[i]]);
        }
        else
        {
            sensor->settings[sensor->num_settings].val = NULL;
        }
        sensor->settings[sensor->num_settings].setting_property_id = data->setting_property_id_list[i];
        sensor->num_settings++;
    }
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_db_sensor_setting_add(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, wiced_bt_mesh_sensor_setting_status_data_t *data)
{
    wiced_bt_mesh_db_model_t *model = find_node_model(mesh_db, element_addr, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV);
    wiced_bt_mesh_db_sensor_t *sensor;
    int i;

    Log("wiced_bt_mesh_db_sensor_setting_add\n");
    if ((model == NULL) || (model->sensor == NULL))
    {
        Log("did not find model returning false\n");
        return WICED_FALSE;
    }
    sensor = find_model_sensor(model, data->property_id);
    if (sensor == NULL)
        return WICED_FALSE;

    for (i = 0; i < sensor->num_settings; i++)
    {
        if (sensor->settings[i].setting_property_id == data->setting.setting_property_id)
        {
            sensor->settings[i].access = data->setting.access;
            if (wiced_bt_mesh_property_len[sensor->settings[i].setting_property_id] != 0)
            {
                if (sensor->settings[i].val == NULL)
                    sensor->settings[i].val = (uint8_t*)wiced_bt_get_buffer(wiced_bt_mesh_property_len[sensor->settings[i].setting_property_id]);

                if (sensor->settings[i].val != NULL)
                    memcpy(sensor->settings[i].val, data->setting.val, wiced_bt_mesh_property_len[sensor->settings[i].setting_property_id]);
            }
            return WICED_TRUE;
        }
    }
    return WICED_FALSE;
}

wiced_bool_t mesh_db_add_sensor_cadence(wiced_bt_mesh_db_model_t *model, wiced_bt_mesh_sensor_cadence_status_data_t *data)
{
    wiced_bt_mesh_db_sensor_t *sensor;

    if (model->sensor == NULL)
        return WICED_FALSE;

    sensor = find_model_sensor(model, data->property_id);
    if (sensor == NULL)
        return WICED_FALSE;

    sensor->cadence.property_id = data->property_id;
    sensor->cadence.trigger_type = data->cadence_data.trigger_type;
    sensor->cadence.min_interval = data->cadence_data.min_interval;
    sensor->cadence.fast_cadence_period_divisor = data->cadence_data.fast_cadence_period_divisor;
    sensor->cadence.trigger_delta_down = data->cadence_data.trigger_delta_down;
    sensor->cadence.trigger_delta_up = data->cadence_data.trigger_delta_up;
    sensor->cadence.fast_cadence_low = data->cadence_data.fast_cadence_low;
    sensor->cadence.fast_cadence_high = data->cadence_data.fast_cadence_high;

    return WICED_FALSE;
}

wiced_bool_t wiced_bt_mesh_db_sensor_cadence_add(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, wiced_bt_mesh_sensor_cadence_status_data_t *data)
{
    Log("wiced_bt_mesh_db_sensor_cadence_add\n");
    wiced_bt_mesh_db_model_t *model = find_node_model(mesh_db, element_addr, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV);
    if (model == NULL)
    {
        Log("did not find model returning false\n");
        return WICED_FALSE;
    }
    return mesh_db_add_sensor_cadence(model, data);
}

wiced_bool_t wiced_bt_mesh_db_node_model_setting_get(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, uint16_t property_id,
    wiced_bt_mesh_db_setting_t *setting_data, uint16_t setting_prop_id, uint8_t *prop_value_len)
{
    int i, j;
    wiced_bt_mesh_db_model_t *model = find_node_model(mesh_db, element_addr, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV);
    if (model == NULL)
    {
        Log("wiced_bt_mesh_db_node_model_setting_get return false");
        return WICED_FALSE;
    }
    for (i = 0; i < model->num_sensors; i++)
    {
        if (model->sensor[i].property_id == property_id)
        {
            for (j = 0; j < model->sensor[i].num_settings; j++)
            {
                if (setting_prop_id == model->sensor[i].settings[j].setting_property_id)
                {
                    *setting_data = model->sensor[i].settings[j];
                    *prop_value_len = wiced_bt_mesh_property_len[model->sensor[i].settings[j].setting_property_id];
                    return WICED_TRUE;
                }
            }
        }
    }
    return WICED_FALSE;
}

wiced_bool_t wiced_bt_mesh_db_node_model_cadence_get(wiced_bt_mesh_db_mesh_t *mesh_db, wiced_bool_t  *is_data_present, uint16_t element_addr, uint16_t property_id,
                                                     wiced_bt_mesh_db_cadence_t *cadence_data, uint8_t *prop_value_len)
{
    int i;
    wiced_bt_mesh_db_model_t *model = find_node_model(mesh_db, element_addr, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV);
    if (model == NULL)
    {
        Log("wiced_bt_mesh_db_node_model_cadence_get return false");
        return WICED_FALSE;
    }
    for (i = 0; i < model->num_sensors; i++)
    {
        if (model->sensor[i].property_id == property_id)
        {
            if (model->sensor[i].cadence.property_id == 0)
            {
                *is_data_present = WICED_FALSE;
            }
            else
            {
                *is_data_present = WICED_TRUE;
                *cadence_data = model->sensor[i].cadence;
            }
            *prop_value_len = model->sensor[i].prop_value_len;
            return  WICED_TRUE;
        }
    }
    return WICED_FALSE;
}

int *wiced_bt_mesh_db_node_model_get_setting_property_ids(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, uint16_t property_id)
{
    int i, j;
    int *setting_prop_ids = NULL;
    wiced_bt_mesh_db_model_t *model = find_node_model(mesh_db, element_addr, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_SENSOR_SRV);
    if (model == NULL)
    {
        Log("get_setting_property_ids no sensor model");
        return NULL;
    }
    for (i = 0; i < model->num_sensors; i++)
    {
        if (model->sensor[i].property_id == property_id)
        {
            if (model->sensor[i].num_settings != 0)
            {
                if ((setting_prop_ids = (int *)wiced_bt_get_buffer(sizeof(int) * (model->sensor[i].num_settings + 1))) != NULL)
                {
                    for (j = 0; j < model->sensor[i].num_settings; j++)
                    {
                        setting_prop_ids[j] = model->sensor[i].settings[j].setting_property_id;
                    }
                    setting_prop_ids[model->sensor[i].num_settings] = 0;
                }
            }
            break;
        }
    }
    return setting_prop_ids;
}

wiced_bool_t wiced_bt_mesh_db_node_config_complete(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t addr, wiced_bool_t is_complete)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_addr(mesh_db, addr);

    if (node == NULL)
        return WICED_FALSE;

    node->config_complete = is_complete;
    return WICED_TRUE;
}


wiced_bool_t wiced_bt_mesh_db_node_model_pub_get(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, uint16_t company_id, uint16_t model_id, uint16_t *pub_addr, uint16_t *app_key_idx, uint8_t *publish_ttl, uint32_t *publish_period, uint16_t *publish_retransmit_count, uint32_t *publish_retransmit_interval, uint8_t *credentials)
{
    wiced_bt_mesh_db_model_t *model = find_node_model(mesh_db, element_addr, company_id, model_id);
    if (model == NULL)
        return WICED_FALSE;

    *pub_addr = model->pub.address;
    *app_key_idx = model->pub.index;
    *publish_ttl = model->pub.ttl;
    *publish_period = model->pub.period;
    *publish_retransmit_count = model->pub.retransmit.count;
    *publish_retransmit_interval = model->pub.retransmit.interval;
    *credentials = model->pub.credentials;
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_db_node_model_pub_add(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, uint16_t company_id, uint16_t model_id, uint16_t pub_addr, uint16_t app_key_idx, uint8_t publish_ttl, uint32_t publish_period, uint16_t publish_retransmit_count, uint32_t publish_retransmit_interval, uint8_t credentials)
{
    wiced_bt_mesh_db_model_t *model = find_node_model(mesh_db, element_addr, company_id, model_id);
    if (model == NULL)
        return WICED_FALSE;

    model->pub.address = pub_addr;
    model->pub.index = app_key_idx;
    model->pub.ttl = publish_ttl;
    model->pub.period = publish_period;
    model->pub.retransmit.count = publish_retransmit_count;
    model->pub.retransmit.interval = publish_retransmit_interval;
    model->pub.credentials = credentials;
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_db_node_model_pub_delete(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, uint16_t company_id, uint16_t model_id)
{
    wiced_bt_mesh_db_model_t *model = find_node_model(mesh_db, element_addr, company_id, model_id);
    if (model == NULL)
        return WICED_FALSE;

    model->pub.address = 0;
    return WICED_TRUE;
}

wiced_bool_t wiced_bt_mesh_db_node_model_sub_add(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, uint16_t company_id, uint16_t model_id, uint16_t addr)
{
    int i;
    wiced_bt_mesh_db_model_t *model = find_node_model(mesh_db, element_addr, company_id, model_id);
    if (model == NULL)
        return WICED_FALSE;

    for (i = 0; i < model->num_subs; i++)
    {
        if (model->sub[i] == addr)
            return WICED_TRUE;
    }
    return mesh_db_add_model_sub(model, addr);
}

wiced_bool_t wiced_bt_mesh_db_node_model_sub_delete(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, uint16_t company_id, uint16_t model_id, uint16_t addr)
{
    wiced_bt_mesh_db_model_t *model = find_node_model(mesh_db, element_addr, company_id, model_id);
    if (model == NULL)
        return WICED_FALSE;

    return mesh_db_delete_model_sub(model, addr);
}

/*
 * Delete All Subscriptions.
 * The function deletes all subscription objtects for the model of the mesh device.  A provisioner should call this function after successfully executing delete all subscriptions operation on a server model device.
 */
wiced_bool_t wiced_bt_mesh_db_node_model_sub_delete_all(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t element_addr, uint16_t company_id, uint16_t model_id)
{
    wiced_bt_mesh_db_model_t *model = find_node_model(mesh_db, element_addr, company_id, model_id);
    if (model == NULL)
        return WICED_FALSE;

    model->num_subs = 0;
    wiced_bt_free_buffer(model->sub);
    model->sub = NULL;
    return WICED_TRUE;
}

/*
 * Set Network Transmit Parameters.
 * The function sets network transmit paramaneters in the configuration of the mesh device.  A provisioner should call this function after successfully executing Set or Get network transmit parameters procedure.
 */
wiced_bool_t wiced_bt_mesh_db_net_transmit_set(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint16_t count, uint32_t interval)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_element_addr(mesh_db, unicast_addr);
    if (node == NULL)
        return WICED_FALSE;

    node->network_transmit.count = count;
    node->network_transmit.interval = interval;
    return WICED_TRUE;
}

/*
 * Get Network Transmit Parameters.
 * The function gets network transmit paramaneters in the configuration of the mesh device.
 */
wiced_bool_t wiced_bt_mesh_db_net_transmit_get(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint16_t *count, uint32_t *interval)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_element_addr(mesh_db, unicast_addr);
    if (node == NULL)
        return WICED_FALSE;

    *count = node->network_transmit.count;
    *interval = node->network_transmit.interval;
    return WICED_TRUE;
}

/*
 * Set Device Default TTL.
 * The function sets Default TTL in the configuration of the mesh device.  A provisioner should call this function after successfully executing Set or Get Default TTL procedure.
 */
wiced_bool_t wiced_bt_mesh_db_default_ttl_set(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint8_t ttl)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_element_addr(mesh_db, unicast_addr);
    if (node == NULL)
        return WICED_FALSE;

    node->default_ttl = ttl;
    return WICED_TRUE;
}

/*
 * Get Default TTL.
 * The function gets the Default TTL in the configuration of the mesh device.
 */
wiced_bool_t wiced_bt_mesh_db_default_ttl_get(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint8_t *ttl)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_element_addr(mesh_db, unicast_addr);
    if (node == NULL)
        return WICED_FALSE;

    *ttl = node->default_ttl;
    return WICED_TRUE;
}

/*
 * Get Relay State.
 * The function sets the Relay State in the configuration of the mesh device.  A provisioner should call this function after successfully executing Set or Get Relay State procedure.
 */
wiced_bool_t wiced_bt_mesh_db_relay_get(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint8_t *state, uint16_t *retransmit_count, uint32_t *retransmit_interval)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_element_addr(mesh_db, unicast_addr);
    if ((node == NULL) || (node->feature.relay == MESH_FEATURE_SUPPORTED_UNKNOWN))
        return WICED_FALSE;

    *state = node->feature.relay;
    *retransmit_count = node->relay_rexmit.count;
    *retransmit_interval = node->relay_rexmit.interval;
    return WICED_TRUE;
}

/*
 * Set Relay State.
 * The function sets Relay State in the configuration of the mesh device.  A provisioner should call this function after successfully executing Set or Get Relay State procedure.
 */
wiced_bool_t wiced_bt_mesh_db_relay_set(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint8_t state, uint16_t retransmit_count, uint32_t retransmit_interval)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_element_addr(mesh_db, unicast_addr);
    if (node == NULL)
        return WICED_FALSE;

    node->feature.relay = state;
    node->relay_rexmit.count = retransmit_count;
    node->relay_rexmit.interval = retransmit_interval;
    return WICED_TRUE;
}

/*
 * Get Friend State.
 * The function gets the Friend State in the configuration of the mesh device.
 */
wiced_bool_t wiced_bt_mesh_db_friend_get(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint8_t *state)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_element_addr(mesh_db, unicast_addr);
    if ((node == NULL) || (node->feature.friend == MESH_FEATURE_SUPPORTED_UNKNOWN))
        return WICED_FALSE;

    *state = node->feature.friend;
    return WICED_TRUE;
}
/*
 * Set Friend State.
 * The function sets Friend State in the configuration of the mesh device.  A provisioner should call this function after successfully executing Set or Get Friend State procedure.
 */
wiced_bool_t wiced_bt_mesh_db_friend_set(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint8_t state)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_element_addr(mesh_db, unicast_addr);
    if (node == NULL)
        return WICED_FALSE;

    node->feature.friend = state;
    return WICED_TRUE;
}

/*
 * Get GATT Proxy State.
 * The function gets the GATT Proxy State in the configuration of the mesh device.
 */
wiced_bool_t wiced_bt_mesh_db_gatt_proxy_get(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint8_t *state)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_element_addr(mesh_db, unicast_addr);
    if ((node == NULL) || (node->feature.gatt_proxy == MESH_FEATURE_SUPPORTED_UNKNOWN))
        return WICED_FALSE;

    *state = node->feature.gatt_proxy;
    return WICED_TRUE;
}

/*
 * Set GATT Proxy State.
 * The function sets GATT Proxy State in the configuration of the mesh device.  A provisioner should call this function after successfully executing Set or Get GATT Proxy State procedure.
 */
wiced_bool_t wiced_bt_mesh_db_gatt_proxy_set(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint8_t state)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_element_addr(mesh_db, unicast_addr);
    if (node == NULL)
        return WICED_FALSE;

    node->feature.gatt_proxy = state;
    return WICED_TRUE;
}

/*
 * Set Send Network Beacons State.
 * The function sets the Send Network Beacons State in the configuration of the mesh device.  A provisioner should call this function after successfully executing Set or Get Network Beacon State procedure.
 */
wiced_bool_t wiced_bt_mesh_db_beacon_set(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint8_t state)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_element_addr(mesh_db, unicast_addr);
    if (node == NULL)
        return WICED_FALSE;

    node->beacon = state;
    return WICED_TRUE;
}

/*
 * Get Send Network Beacons State.
 * The function gets the Send Network Beacons State of the mesh device.
 */
wiced_bool_t wiced_bt_mesh_db_beacon_get(wiced_bt_mesh_db_mesh_t *mesh_db, uint16_t unicast_addr, uint8_t *state)
{
    wiced_bt_mesh_db_node_t *node = wiced_bt_mesh_db_node_get_by_element_addr(mesh_db, unicast_addr);
    if (node == NULL)
        return WICED_FALSE;

    *state = node->beacon;
    return WICED_TRUE;
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

uint32_t get_hex_value(char *szbuf, uint8_t *buf, uint32_t buf_size)
{
    uint8_t *pbuf = buf;
    uint32_t res = 0;
    uint32_t i;

    // remove leading white space
    while (*szbuf != 0 && (uint8_t)*szbuf <= 0x20) szbuf++;
    uint32_t len = (uint32_t)strlen(szbuf);
    // remove terminating white space
    while (len > 0 && (uint8_t)szbuf[len - 1] <= 0x20) len--;

    memset(buf, 0, buf_size);

    if (len == 1)
    {
        szbuf[2] = 0;
        szbuf[1] = szbuf[0];
        szbuf[0] = '0';
    }
    else if (len == 3)
    {
        szbuf[4] = 0;
        szbuf[3] = szbuf[2];
        szbuf[2] = szbuf[1];
        szbuf[1] = szbuf[0];
        szbuf[0] = '0';
    }
    else if (len == 5)
    {
        szbuf[6] = 0;
        szbuf[5] = szbuf[4];
        szbuf[4] = szbuf[3];
        szbuf[3] = szbuf[2];
        szbuf[2] = szbuf[1];
        szbuf[1] = szbuf[0];
        szbuf[0] = '0';
    }
    for (i = 0; (i < len) && (res < buf_size); i++)
    {
        if (isxdigit(szbuf[i]) && isxdigit(szbuf[i + 1]))
        {
            *pbuf++ = (process_nibble((char)szbuf[i]) << 4) + process_nibble((char)szbuf[i + 1]);
            res++;
            i++;
        }
    }
    return res;
}

wiced_bool_t is_group_address(uint16_t addr)
{
    return (addr & 0xc000) == 0xc000;
}

wiced_bool_t mesh_db_add_node_net_key(wiced_bt_mesh_db_node_t *node, wiced_bt_mesh_db_key_idx_phase *net_key_idx)
{
    wiced_bt_mesh_db_key_idx_phase *p_temp;
    p_temp = node->net_key;
    node->net_key = (wiced_bt_mesh_db_key_idx_phase *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_key_idx_phase) * (node->num_net_keys + 1));
    if (node->net_key == NULL)
        return WICED_FALSE;
    if (p_temp != 0)
    {
        memcpy(node->net_key, p_temp, sizeof(wiced_bt_mesh_db_key_idx_phase) * node->num_net_keys);
        wiced_bt_free_buffer(p_temp);
    }
    memcpy(&node->net_key[node->num_net_keys], net_key_idx, sizeof(wiced_bt_mesh_db_key_idx_phase));
    node->num_net_keys++;
    return WICED_TRUE;
}

wiced_bool_t mesh_db_add_node_app_key(wiced_bt_mesh_db_node_t *node, wiced_bt_mesh_db_key_idx_phase *app_key_idx)
{
    wiced_bt_mesh_db_key_idx_phase *p_temp;
    p_temp = node->app_key;
    node->app_key = (wiced_bt_mesh_db_key_idx_phase *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_db_key_idx_phase) * (node->num_app_keys + 1));
    if (node->app_key == NULL)
        return WICED_FALSE;
    if (p_temp != 0)
    {
        memcpy(node->app_key, p_temp, sizeof(wiced_bt_mesh_db_key_idx_phase) * node->num_app_keys);
        wiced_bt_free_buffer(p_temp);
    }
    memcpy(&node->app_key[node->num_app_keys], app_key_idx, sizeof(wiced_bt_mesh_db_key_idx_phase));
    node->num_app_keys++;
    return WICED_TRUE;
}

wiced_bool_t mesh_db_add_model_app_bind(wiced_bt_mesh_db_model_t *model, uint16_t key_idx)
{
    uint16_t *p_temp;

    p_temp = model->bound_key;
    model->bound_key = (uint16_t *)wiced_bt_get_buffer(sizeof(uint16_t) * (model->num_bound_keys + 1));
    if (model->bound_key == NULL)
        return 0;
    if (p_temp != 0)
    {
        memcpy(model->bound_key, p_temp, sizeof(uint16_t) * model->num_bound_keys);
        wiced_bt_free_buffer(p_temp);
    }
    memcpy(&model->bound_key[model->num_bound_keys], &key_idx, sizeof(uint16_t));
    model->num_bound_keys++;
    return WICED_TRUE;
}

wiced_bool_t mesh_db_add_model_sub(wiced_bt_mesh_db_model_t *model, uint16_t addr)
{
    uint16_t *p_temp;

    p_temp = model->sub;
    model->sub = (uint16_t *)wiced_bt_get_buffer(sizeof(uint16_t) * (model->num_subs + 1));
    if (model->sub == NULL)
        return 0;
    if (p_temp != 0)
    {
        memcpy(model->sub, p_temp, sizeof(uint16_t) * model->num_subs);
        wiced_bt_free_buffer(p_temp);
    }
    memcpy(&model->sub[model->num_subs], &addr, sizeof(uint16_t));
    model->num_subs++;
    return WICED_TRUE;
}

wiced_bool_t mesh_db_delete_model_sub(wiced_bt_mesh_db_model_t *model, uint16_t addr)
{
    int i;

    if (model->num_subs == 1)
    {
        if (model->sub[0] == addr)
        {
            model->num_subs = 0;
            wiced_bt_free_buffer(model->sub);
            model->sub = NULL;
            return WICED_TRUE;
        }
    }
    else
    {
        for (i = 0; i < model->num_subs; i++)
        {
            if (model->sub[i] == addr)
            {
                for (; i < model->num_subs - 1; i++)
                {
                    memcpy(&model->sub[i], &model->sub[i + 1], sizeof(uint16_t));
                }
                model->num_subs--;
                return WICED_TRUE;
            }
        }
    }
    return WICED_TRUE;
}

uint32_t get_int_value( uint8_t *value, int len)
{
    uint32_t return_val = 0;
    if (len == 1)
    {
        return_val = value[0];
    }
    else if (len == 2)
    {
        return_val = (int)(value[1] << 8 | value[0]);
    }
    else if (len == 3)
    {
        return_val = (int)(value[2] << 16 | value[1] << 8 | value[0]);
    }
    else if (len == 4)
    {
        return_val = (int)(value[3] << 24 | value[2] << 16 | value[1] << 8 | value[0]);
    }
    return return_val;
}

uint8_t wiced_bt_mesh_db_get_property_value_len(uint16_t property_id)
{
    if (property_id <= WICED_BT_MESH_MAX_PROPERTY_ID)
        return wiced_bt_mesh_property_len[property_id];
    else
        return 0;
}
