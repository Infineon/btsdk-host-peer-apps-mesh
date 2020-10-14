package com.cypress.le.mesh.meshcore;

class HciConstants {


    //HCI COMMANDS
    public static final short  HCI_CONTROL_GROUP_MESH                                                = (short) 0x16;
    public static final short  HCI_CONTROL_MESH_COMMAND_SET_LOCAL_DEVICE                             = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xe0 );  /* Terminate friendship with a Friend by sending a Friend Clear */
    public static final short  HCI_CONTROL_MESH_COMMAND_SET_DEVICE_KEY                               = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xe1 );  /* Setup device key.  Application can set it once and then send multiple configuration commands. */
    public static final short  HCI_CONTROL_MESH_COMMAND_CORE_LOW_POWER_SEND_FRIEND_CLEAR             = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xe2 );  /* Terminate friendship with a Friend by sending a Friend Clear */
    public static final short  HCI_CONTROL_MESH_COMMAND_CORE_PROVISION                               = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xe3 );  /* Sends command to provision remote device */
    public static final short  HCI_CONTROL_MESH_COMMAND_CORE_SET_GATT_MTU                            = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xed );  /* Sends command to set GATT MTU for the provisioning or proxy connection */

    /*
     * Mesh Commands
     */
    public static final short  HCI_CONTROL_MESH_COMMAND_CORE_NETWORK_LAYER_TRNSMIT                 = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x01 ) ; /* Network Layer Transmit Mesage Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_CORE_TRANSPORT_LAYER_TRNSMIT               = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x02 ) ; /* Transport Layer Transmit Mesage Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_CORE_IVUPDATE_SIGNAL_TRNSIT                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x03 ) ; /* IV UPDATE Transit Signal Command */

    public static final short  HCI_CONTROL_MESH_COMMAND_SCAN_UNPROVISIONED                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x04 ) ; /* Sends command to start/stop scanning for unprovisioned devices */
    public static final short  HCI_CONTROL_MESH_COMMAND_PROVISION_CONNECT                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x05 ) ; /* Sends command to establish provisioning link to remote device */
    public static final short  HCI_CONTROL_MESH_COMMAND_PROVISION_DISCONNECT                       = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x06 ) ; /* Sends command to disconnect provisioning link with remote device */
    public static final short  HCI_CONTROL_MESH_COMMAND_PROVISION_START                            = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x07 ) ; /* Sends command to start provisioning of the remote device */
    public static final short  HCI_CONTROL_MESH_COMMAND_PROVISION_OOB_CONFIGURE                    = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x08 ) ; /* Sends out of band configuration for provisioning device */
    public static final short  HCI_CONTROL_MESH_COMMAND_PROVISION_OOB_VALUE                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x09 ) ; /* Sends command with out of band value for confirmation calculation */
    public static final short  HCI_CONTROL_MESH_COMMAND_SEARCH_PROXY                               = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x0a ) ; /* Sends a command to start/stop scanning for GATT proxy devices */
    public static final short  HCI_CONTROL_MESH_COMMAND_PROXY_CONNECT                              = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x0b ) ; /* Sends a command to connect to a GATT proxy devices */
    public static final short  HCI_CONTROL_MESH_COMMAND_PROXY_DISCONNECT                           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x0c ) ; /* Sends a command to disconnect to a GATT proxy devices */
    public static final short  HCI_CONTROL_MESH_COMMAND_PROXY_FILTER_TYPE_SET                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x0d ) ; /* Set Proxy Filter Type */
    public static final short  HCI_CONTROL_MESH_COMMAND_PROXY_FILTER_ADDRESSES_ADD                 = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x0e ) ; /* Add Addresses to Filter */
    public static final short  HCI_CONTROL_MESH_COMMAND_PROXY_FILTER_ADDRESSES_DELETE              = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x0f ) ; /* Remove Addresses to Filter  */

    public static final short  HCI_CONTROL_MESH_COMMAND_ONOFF_GET                                  = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x10 ) ; /* Generic On/Off Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_ONOFF_SET                                  = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x11 ) ; /* Generic On/Off Get Command */

    public static final short  HCI_CONTROL_MESH_COMMAND_LEVEL_GET                                  = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x12 ) ; /* Generic Level Client Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LEVEL_SET                                  = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x13 ) ; /* Generic Level Client Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LEVEL_DELTA_SET                            = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x14 ) ; /* Generic Level Client Delta Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LEVEL_MOVE_SET                             = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x15 ) ; /* Generic Level Client Move Command */

    public static final short  HCI_CONTROL_MESH_COMMAND_DEF_TRANS_TIME_GET                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x16 ) ; /* Generic Default Transition Time Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_DEF_TRANS_TIME_SET                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x17 ) ; /* Generic Default Transition Time Get Command */

    public static final short  HCI_CONTROL_MESH_COMMAND_ONPOWERUP_GET                              = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x18 ) ; /* Power On/Off Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_ONPOWERUP_SET                              = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x19 ) ; /* Power On/Off Get Command */

    public static final short  HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_GET                            = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x1a ) ; /* Generic Power Level Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_SET                            = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x1b ) ; /* Generic Power Level Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_LAST_GET                       = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x1c ) ; /* Generic Power Level Last Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_DEFAULT_GET                    = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x1d ) ; /* Generic Power Level Default Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_DEFAULT_SET                    = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x1e ) ; /* Generic Power Level Default Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_RANGE_GET                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x1f ) ; /* Generic Power Level Range Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_POWER_LEVEL_RANGE_SET                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x20 ) ; /* Generic Power Level Range Set Command */

    public static final short  HCI_CONTROL_MESH_COMMAND_LOCATION_GLOBAL_SET                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x21 ) ; /* Set Global Location data */
    public static final short  HCI_CONTROL_MESH_COMMAND_LOCATION_LOCAL_SET                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x22 ) ; /* Set Local Location data */
    public static final short  HCI_CONTROL_MESH_COMMAND_LOCATION_GLOBAL_GET                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x23 ) ; /* Get Global Location data */
    public static final short  HCI_CONTROL_MESH_COMMAND_LOCATION_LOCAL_GET                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x24 ) ; /* Get_Local Location data */

    public static final short  HCI_CONTROL_MESH_COMMAND_BATTERY_GET                                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x25 ) ; /* Battery get state */
    public static final short  HCI_CONTROL_MESH_COMMAND_BATTERY_SET                                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x26 ) ; /* Battery state changed */

    public static final short  HCI_CONTROL_MESH_COMMAND_PROPERTIES_GET                             = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x27 ) ; /* Generic Set Value of the Property */
    public static final short  HCI_CONTROL_MESH_COMMAND_PROPERTY_GET                               = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x28 ) ; /* Generic Set Value of the Property */
    public static final short  HCI_CONTROL_MESH_COMMAND_PROPERTY_SET                               = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x29 ) ; /* Generic Value of the Property Changed Status*/

    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_GET                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x2a ) ; /* Light Lightness Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_SET                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x2b ) ; /* Light Lightness Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_LINEAR_GET                 = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x2c ) ; /* Light Lightness Linear Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_LINEAR_SET                 = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x2d ) ; /* Light Lightness Linear Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_LAST_GET                   = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x2e ) ; /* Light Lightness Last Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_DEFAULT_GET                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x2f ) ; /* Light Lightness Default Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_DEFAULT_SET                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x30 ) ; /* Light Lightness Default Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_RANGE_GET                  = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x31 ) ; /* Light Lightness Range Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_LIGHTNESS_RANGE_SET                  = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x32 ) ; /* Light Lightness Range Set Command */

    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_CTL_GET                              = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x33 ) ; /* Light CTL Client Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_CTL_SET                              = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x34 ) ; /* Light CTL Client Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_CTL_TEMPERATURE_GET                  = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x35 ) ; /* Light CTL Client Temperature Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_CTL_TEMPERATURE_SET                  = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x36 ) ; /* Light CTL Client Temperature Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_CTL_TEMPERATURE_RANGE_GET            = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x37 ) ; /* Light CTL Client Temperature Range Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_CTL_TEMPERATURE_RANGE_SET            = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x38 ) ; /* Light CTL Client Temperature Range Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_CTL_DEFAULT_GET                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x39 ) ; /* Light CTL Client Default Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_CTL_DEFAULT_SET                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x3a ) ; /* Light CTL Client Default Set Command */

    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_GET                              = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x3c ) ; /* Light HSL Client Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_SET                              = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x3d ) ; /* Light HSL Client Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_TARGET_GET                       = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x3e ) ; /* Light HSL Client Target Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_RANGE_GET                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x3f ) ; /* Light HSL Client Range Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_RANGE_SET                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x40) ; /* Light HSL Client Range Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_DEFAULT_GET                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x41 ) ; /* Light HSL Client Default Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_DEFAULT_SET                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x42 ) ; /* Light HSL Client Default Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_HUE_GET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x43 ) ; /* Light HSL Client Hue Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_HUE_SET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x44 ) ; /* Light HSL Client Hue Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_SATURATION_GET                   = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x45 ) ; /* Light HSL Client Saturation Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_HSL_SATURATION_SET                   = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x46 ) ; /* Light HSL Client Saturation Set Command */

    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_GET                              = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x47 ) ; /* Light XYL Client Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_SET                              = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x48 ) ; /* Light XYL Client Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_RANGE_GET                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x49 ) ; /* Light XYL Client Range Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_RANGE_SET                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x4a ) ; /* Light XYL Client Range Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_TARGET_GET                       = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x4b ) ; /* Light XYL Client Target Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_DEFAULT_GET                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x4c ) ; /* Light XYL Client Default Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_XYL_DEFAULT_SET                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x4d ) ; /* Light XYL Client Default Set Command */

    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_LC_MODE_GET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x50 ) ; /* Light LC Client Mode Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_LC_MODE_SET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x51 ) ; /* Light LC Client Mode Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_LC_OCCUPANCY_MODE_GET                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x53 ) ; /* Light LC Client Occupancy Mode Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_LC_OCCUPANCY_MODE_SET                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x54 ) ; /* Light LC Client Occupancy Mode Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_LC_ONOFF_GET                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x56 ) ; /* Light LC Client OnOff Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_LC_ONOFF_SET                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x57 ) ; /* Light LC Client OnOff Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_LC_PROPERTY_GET                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x59 ) ; /* Light LC Client Property Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_LC_PROPERTY_SET                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x5a ) ; /* Light LC Client Property Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_LIGHT_LC_OCCUPANCY_SET                     = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x5c ) ; /* Light LC Server Occupancy Detected Command */

    public static final short  HCI_CONTROL_MESH_COMMAND_SENSOR_DESCRIPTOR_GET                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x60 ) ; /* Sensor Descriptor Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_SENSOR_CADENCE_GET                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x61 ) ; /* Sensor Cadence Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_SENSOR_CADENCE_SET                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x62 ) ; /* Sensor Cadence Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_SENSOR_SETTINGS_GET                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x63 ) ; /* Sensor Settings Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_SENSOR_SETTING_GET                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x64 ) ; /* Sensor Setting Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_SENSOR_GET                                 = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x65 ) ; /* Sensor Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_SENSOR_COLUMN_GET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x66 ) ; /* Sensor Column Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_SENSOR_SERIES_GET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x67 ) ; /* Sensor Series Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_SENSOR_SETTING_SET                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x68 ) ; /* Sensor Setting Set Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_SENSOR_SET                                 = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x6b ) ; /* Sensor Status Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_SENSOR_COLUMN_SET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x6c ) ; /* Sensor Column Status Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_SENSOR_SERIES_SET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x6d ) ; /* Sensor Series Status Command */

    public static final short  HCI_CONTROL_MESH_COMMAND_SCENE_STORE                                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x70 ) ; /* Scene Store Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_SCENE_RECALL                               = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x71 ) ; /* Scene Delete Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_SCENE_GET                                  = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x72 ) ; /* Scene Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_SCENE_REGISTER_GET                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x73 ) ; /* Scene Register Get Command */
    public static final short  HCI_CONTROL_MESH_COMMAND_SCENE_DELETE                               = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x74 ) ; /* Scene Delete Command */


    public static final short HCI_CONTROL_MESH_COMMAND_SCHEDULER_GET                               = (short) ( (  HCI_CONTROL_GROUP_MESH << 8 ) | 0x75 ) ; /* Scheduler Register Get Command */
    public static final short HCI_CONTROL_MESH_COMMAND_SCHEDULER_ACTION_GET                        = (short) ( (  HCI_CONTROL_GROUP_MESH << 8 ) | 0x76 ) ; /* Scheduler Action Get Command */
    public static final short HCI_CONTROL_MESH_COMMAND_SCHEDULER_ACTION_SET                        = (short) ( (  HCI_CONTROL_GROUP_MESH << 8 ) | 0x77 ) ; /* Scheduler Action Set Command */

    public static final short HCI_CONTROL_MESH_COMMAND_TIME_GET                                    = (short) ( (  HCI_CONTROL_GROUP_MESH << 8 ) | 0x78 ) ; /* Time Get Command */
    public static final short HCI_CONTROL_MESH_COMMAND_TIME_SET                                    = (short) ( (  HCI_CONTROL_GROUP_MESH << 8 ) | 0x79 ) ; /* Time Set Command */
    public static final short HCI_CONTROL_MESH_COMMAND_TIME_ZONE_GET                               = (short) ( (  HCI_CONTROL_GROUP_MESH << 8 ) | 0x7a ) ; /* Time Zone Get Command */
    public static final short HCI_CONTROL_MESH_COMMAND_TIME_ZONE_SET                               = (short) ( (  HCI_CONTROL_GROUP_MESH << 8 ) | 0x7b ) ; /* Time Zone Set Command */
    public static final short HCI_CONTROL_MESH_COMMAND_TIME_TAI_UTC_DELTA_GET                      = (short) ( (  HCI_CONTROL_GROUP_MESH << 8 ) | 0x7c ) ; /* Time TAI_UTC Delta Get Command */
    public static final short HCI_CONTROL_MESH_COMMAND_TIME_TAI_UTC_DELTA_SET                      = (short) ( (  HCI_CONTROL_GROUP_MESH << 8 ) | 0x7d ) ; /* Time TAI_UTC Delta Set Command */
    public static final short HCI_CONTROL_MESH_COMMAND_TIME_ROLE_GET                               = (short) ( (  HCI_CONTROL_GROUP_MESH << 8 ) | 0x7e ) ; /* Time Role Get Command */
    public static final short HCI_CONTROL_MESH_COMMAND_TIME_ROLE_SET                               = (short) ( (  HCI_CONTROL_GROUP_MESH << 8 ) | 0x7f ) ; /* Time Role Set Command */
    public static final short HCI_CONTROL_MESH_COMMAND_PROXY_CONNECTED                             = (short)( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xec );  /* Proxy Filter Status */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_NODE_RESET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x80 ) ; /* Node Reset */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_BEACON_GET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x81 ) ; /* Beacon State Get */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_BEACON_SET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x82 ) ; /* Beacon State Set */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_COMPOSITION_DATA_GET                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x83 ) ; /* Composition Data Get */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_DEFAULT_TTL_GET                     = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x84 ) ; /* Default TTL Get */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_DEFAULT_TTL_SET                     = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x85 ) ; /* Default TTL Set */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_GATT_PROXY_GET                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x86 ) ; /* GATT Proxy State Get */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_GATT_PROXY_SET                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x87 ) ; /* GATT Proxy State Set */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_RELAY_GET                           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x88 ) ; /* Relay Configuration Get */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_RELAY_SET                           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x89 ) ; /* Relay Configuration Set */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_FRIEND_GET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x8a ) ; /* Relay Configuration Set */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_FRIEND_SET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x8b ) ; /* Relay Configuration Set */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_HEARBEAT_SUBSCRIPTION_GET           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x8c ) ; /* Hearbeat Subscription Set */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_HEARBEAT_SUBSCRIPTION_SET           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x8d ) ; /* Hearbeat Subscription Set */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_HEARBEAT_PUBLICATION_GET            = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x8e ) ; /* Hearbeat Publication Set */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_HEARBEAT_PUBLICATION_SET            = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x8f ) ; /* Hearbeat Publication Set */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_NETWORK_TRANSMIT_GET                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x90 ) ; /* Network Transmit Get */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_NETWORK_TRANSMIT_SET                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x91 ) ; /* Network Transmit Set */

    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_PUBLICATION_GET               = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x92 ) ; /* Model Publication Get */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_PUBLICATION_SET               = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x93 ) ; /* Model Publication Set */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_ADD              = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x94 ) ; /* Model Subscription Add */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_DELETE           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x95 ) ; /* Model Subscription Delete */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_OVERWRITE        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x96 ) ; /* Model Subscription Overwrite */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_DELETE_ALL       = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x97 ) ; /* Model Subscription Delete All */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_GET              = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x98 ) ; /* Model Subscription Get */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_ADD                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x99 ) ; /* NetKey Add */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_DELETE                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x9a ) ; /* NetKey Delete */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_UPDATE                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x9b ) ; /* NetKey Update */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_GET                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x9c ) ; /* NetKey Get */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_ADD                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x9d ) ; /* AppKey Add */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_DELETE                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x9e ) ; /* AppKey Delete */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_UPDATE                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x9f ) ; /* AppKey Update */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_GET                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xa0 ) ; /* AppKey Get */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_APP_BIND                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xa1 ) ; /* Model App Bind */

    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_APP_UNBIND                    = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xa2 ) ; /* Model App Unind */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_APP_GET                       = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xa3 ) ; /* Model App Get */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_NODE_IDENTITY_GET                   = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xa4 ) ; /* Node Identity Get */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_NODE_IDENTITY_SET                   = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xa5 ) ; /* Node Identity Get */

    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_LPN_POLL_TIMEOUT_GET                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xa6 ) ; /* LPN Poll Timeout Get */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_KEY_REFRESH_PHASE_GET               = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xa7 ) ; /* Key Refresh Phase Get */
    public static final short HCI_CONTROL_MESH_COMMAND_CONFIG_KEY_REFRESH_PHASE_SET               = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xa8 ) ; /* Key Refresh Phase Set */

    public static final short HCI_CONTROL_MESH_COMMAND_HEALTH_FAULT_GET                           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xa9 ) ; /* Health Fault Get */
    public static final short HCI_CONTROL_MESH_COMMAND_HEALTH_FAULT_CLEAR                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xaa ) ; /* Health Fault Clear */
    public static final short HCI_CONTROL_MESH_COMMAND_HEALTH_FAULT_TEST                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xab ) ; /* Health Fault Test */
    public static final short HCI_CONTROL_MESH_COMMAND_HEALTH_PERIOD_GET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xac ) ; /* Health Period Get */
    public static final short HCI_CONTROL_MESH_COMMAND_HEALTH_PERIOD_SET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xad ) ; /* Health Period Set */
    public static final short HCI_CONTROL_MESH_COMMAND_HEALTH_ATTENTION_GET                       = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xae ) ; /* Health Attention Get */

    /*
     * Mesh events
     */
    public static final short HCI_CONTROL_MESH_EVENT_COMMAND_STATUS                               = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x00 );     /* Command Status event */

    public static final short HCI_CONTROL_MESH_EVENT_ONOFF_SET                                    = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x08 );     /* ON/OFF Server Set */
    public static final short HCI_CONTROL_MESH_EVENT_ONOFF_STATUS                                 = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x09 );     /* ON/OFF Client Status */

    public static final short HCI_CONTROL_MESH_EVENT_LEVEL_SET                                    = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x0c );     /* Level Server Set */
    public static final short HCI_CONTROL_MESH_EVENT_LEVEL_STATUS                                 = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x0d );     /* Level Client Status */

    public static final short HCI_CONTROL_MESH_EVENT_LOCATION_GLOBAL_SET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x10 );     /* Set Global Location data */
    public static final short HCI_CONTROL_MESH_EVENT_LOCATION_LOCAL_SET                           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x11 );     /* Set Local Location data */
    public static final short HCI_CONTROL_MESH_EVENT_LOCATION_GLOBAL_STATUS                       = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x12 );     /* Global Location data changed */
    public static final short HCI_CONTROL_MESH_EVENT_LOCATION_LOCAL_STATUS                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x13 );     /* Local Location data changed */

    public static final short HCI_CONTROL_MESH_EVENT_BATTERY_STATUS                               = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x15 );     /* Battery status data */

    public static final short HCI_CONTROL_MESH_EVENT_DEF_TRANS_TIME_STATUS                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x1b );     /* Default Transition Time Client Status */

    public static final short HCI_CONTROL_MESH_EVENT_POWER_ONOFF_STATUS                           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x20 );     /* Power ON/OFF Client Status */

    public static final short HCI_CONTROL_MESH_EVENT_POWER_LEVEL_SET                              = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x25 );     /* Power Level Server Set */
    public static final short HCI_CONTROL_MESH_EVENT_POWER_LEVEL_DEFAULT_SET                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x26 );     /* Power Level Server Set Default Power Level */
    public static final short HCI_CONTROL_MESH_EVENT_POWER_LEVEL_RANGE_SET                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x27 );     /* Power Level Server Set Min/Max Power Level range */
    public static final short HCI_CONTROL_MESH_EVENT_POWER_LEVEL_STATUS                           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x28 );     /* Power Level Client Status */
    public static final short HCI_CONTROL_MESH_EVENT_POWER_LEVEL_LAST_STATUS                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x29 );     /* Last Power Level Client Status */
    public static final short HCI_CONTROL_MESH_EVENT_POWER_LEVEL_DEFAULT_STATUS                   = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x2a );     /* Default Power Level Client Status */
    public static final short HCI_CONTROL_MESH_EVENT_POWER_LEVEL_RANGE_STATUS                     = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x2b );     /* Default Power Level Client Status */

    public static final short HCI_CONTROL_MESH_EVENT_PROPERTY_SET                                 = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x30 );     /* Set Value of the Property */
    public static final short HCI_CONTROL_MESH_EVENT_PROPERTIES_STATUS                            = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x31 );     /* List of Properties reported by the Server */
    public static final short HCI_CONTROL_MESH_EVENT_PROPERTY_STATUS                              = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x32 );     /* Value of the Property Changed Status*/

    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_LIGHTNESS_SET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x38 );     /* Light Lightness Server Set */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_LIGHTNESS_STATUS                       = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x39 );     /* Light Lightness Client Status */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_LIGHTNESS_LINEAR_STATUS                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x3a );     /* Light Lightness Client Status */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_LIGHTNESS_LAST_STATUS                  = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x3b );     /* Last Light Lightness Client Status */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_LIGHTNESS_DEFAULT_STATUS               = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x3c );     /* Default Light Lightness Client Status */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_LIGHTNESS_RANGE_STATUS                 = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x3d );     /* Range Light Lightness Client Status */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_LIGHTNESS_RANGE_SET                    = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x3e );     /* Light Lightness Server Range Set Event */

    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_CTL_STATUS                             = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x40 );  /* Client Light CTL Status Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_CTL_TEMPERATURE_STATUS                 = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x41 );  /* Client Light CTL Temperature Status Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_CTL_TEMPERATURE_RANGE_STATUS           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x42 );  /* Client Light CTL Temperature Range Statust Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_CTL_DEFAULT_STATUS                     = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x43 );  /* Client Light CTL Default Status Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_CTL_SET                                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x44 );  /* Server Light CTL Set Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_CTL_TEMPERATURE_SET                    = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x45 );  /* Server Light CTL Temperature Set Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_CTL_TEMPERATURE_RANGE_SET              = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x46 );  /* Server Light CTL Temperature Range Set Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_CTL_DEFAULT_SET                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x47 );  /* Server Light CTL Default Set Event */

    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_HSL_SET                                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x50 );  /* Server Light HSL Set Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_HSL_STATUS                             = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x51 );  /* Client Light HSL Status Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_HSL_TARGET_STATUS                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x52 );  /* Client Light HSL Status Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_HSL_RANGE_SET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x53 );  /* Server Light HSL Temperature Range Set Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_HSL_RANGE_STATUS                       = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x54 );  /* Client Light HSL Default Status Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_HSL_DEFAULT_SET                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x55 );  /* Server Light HSL Default Set Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_HSL_DEFAULT_STATUS                     = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x56 );  /* Client Light HSL Default Status Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_HSL_HUE_SET                            = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x57 );  /* Server Light HSL Set Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_HSL_HUE_STATUS                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x58 );  /* Client Light HSL Status Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_HSL_SATURATION_SET                     = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x59 );  /* Server Light HSL Set Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_HSL_SATURATION_STATUS                  = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x5a );  /* Client Light HSL Status Event */

    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_XYL_SET                                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x60 );  /* Server Light XYL Set Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_XYL_STATUS                             = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x61 );  /* Client Light XYL Status Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_XYL_TARGET_STATUS                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x62 );  /* Client Light XYL Status Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_XYL_RANGE_SET                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x63 );  /* Server Light XYL Temperature Range Set Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_XYL_RANGE_STATUS                       = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x64 );  /* Client Light XYL Default Status Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_XYL_DEFAULT_SET                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x65 );  /* Server Light XYL Default Set Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_XYL_DEFAULT_STATUS                     = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x66 );  /* Client Light XYL Default Status Event */

    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_LC_MODE_SERVER_SET                     = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x70 );  /* Light LC Server Mode Set Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_LC_MODE_CLIENT_STATUS                  = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x71 );  /* Light LC Client Mode Status Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_LC_OCCUPANCY_MODE_SERVER_SET           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x72 );  /* Light LC Server Occupancy Mode Set Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_LC_OCCUPANCY_MODE_CLIENT_STATUS        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x73 );  /* Light LC Client Occupancy Mode Status Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_LC_ONOFF_SERVER_SET                    = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x74 );  /* Light LC Server OnOff Set Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_LC_ONOFF_CLIENT_STATUS                 = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x75 );  /* Light LC Client OnOff Status Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_LC_PROPERTY_SERVER_SET                 = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x76 );  /* Light LC Server Property Set Event */
    public static final short HCI_CONTROL_MESH_EVENT_LIGHT_LC_PROPERTY_CLIENT_STATUS              = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x77 );  /* Light LC Client Property Status Event */

    public static final short HCI_CONTROL_MESH_EVENT_SENSOR_DESCRIPTOR_STATUS                     = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x80 );  /* Value of the Sensor Descriptor Status*/
    public static final short HCI_CONTROL_MESH_EVENT_SENSOR_STATUS                                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x81 );  /* Value of the Sensor Status*/
    public static final short HCI_CONTROL_MESH_EVENT_SENSOR_COLUMN_STATUS                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x82 );  /* Value of the Sensor Column Status*/
    public static final short HCI_CONTROL_MESH_EVENT_SENSOR_SERIES_STATUS                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x83 );  /* Value of the Sensor Series Status*/
    public static final short HCI_CONTROL_MESH_EVENT_SENSOR_CADENCE_STATUS                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x84 );  /* Value of the Sensor Cadence Status*/
    public static final short HCI_CONTROL_MESH_EVENT_SENSOR_SETTING_STATUS                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x85 );  /* Value of the Sensor Setting Status*/
    public static final short HCI_CONTROL_MESH_EVENT_SENSOR_SETTINGS_STATUS                       = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x86 );  /* Value of the Sensor Settings Status*/
    public static final short HCI_CONTROL_MESH_EVENT_SENSOR_CADENCE_GET                           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x87 );  /* Sensor Cadence Get */
    public static final short HCI_CONTROL_MESH_EVENT_SENSOR_CADENCE_SET                           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x88 );  /* Sensor Cadence Set */
    public static final short HCI_CONTROL_MESH_EVENT_SENSOR_SETTING_GET                           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x89 );  /* Sensor Setting Get */
    public static final short HCI_CONTROL_MESH_EVENT_SENSOR_GET                                   = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x8a );  /* Sensor Get */
    public static final short HCI_CONTROL_MESH_EVENT_SENSOR_COLUMN_GET                            = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x8b );  /* Sensor Column Get */
    public static final short HCI_CONTROL_MESH_EVENT_SENSOR_SERIES_GET                            = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x8c );  /* Sensor Series Get */
    public static final short HCI_CONTROL_MESH_EVENT_SENSOR_SETTING_SET                           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x8d );  /* Sensor Setting Set */

    public static final short HCI_CONTROL_MESH_EVENT_SCENE_STATUS                                 = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x90 );  /* Scene status event */
    public static final short HCI_CONTROL_MESH_EVENT_SCENE_REGISTER_STATUS                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x91 );  /* Scene provisionClientInit status event */

    public static final short HCI_CONTROL_MESH_EVENT_SCHEDULER_STATUS                             = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x98 );  /* Scheduler provisionClientInit status event */
    public static final short HCI_CONTROL_MESH_EVENT_SCHEDULER_ACTION_STATUS                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0x99 );  /* Scheduler action status event */

    public static final short HCI_CONTROL_MESH_EVENT_TIME_STATUS                                  = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xa0 );  /* Time Status Event */
    public static final short HCI_CONTROL_MESH_EVENT_TIME_ZONE_STATUS                             = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xa1 );  /* Time Zone Status Event */
    public static final short HCI_CONTROL_MESH_EVENT_TIME_TAI_UTC_DELTA_STATUS                    = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xa2 );  /* Time TAI_UTC Delta Status Event */
    public static final short HCI_CONTROL_MESH_EVENT_TIME_ROLE_STATUS                             = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xa3 );  /* Time Role Status Event */
    public static final short HCI_CONTROL_MESH_EVENT_TIME_SET                                     = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xa4 );  /* Time Set Event */

    public static final short HCI_CONTROL_MESH_EVENT_UNPROVISIONED_DEVICE                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xd0 );  /* Unprovisioned device event */
    public static final short HCI_CONTROL_MESH_EVENT_PROVISION_LINK_STATUS                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xd1 );  /* Provision link established or dropped */
    public static final short HCI_CONTROL_MESH_EVENT_PROVISION_END                                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xd2 );  /* Provision end event */
    public static final short HCI_CONTROL_MESH_EVENT_PROVISION_DEVICE_CAPABITIES                  = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xd3 );  /* Provisioning device capabilities */
    public static final short HCI_CONTROL_MESH_EVENT_PROVISION_OOB_DATA                           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xd4 );  /* Provisioning OOB data request */
    public static final short HCI_CONTROL_MESH_EVENT_PROXY_DEVICE_NETWORK_DATA                    = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xd5 );  /* Proxy device network data event */

    public static final short HCI_CONTROL_MESH_EVENT_CORE_PROVISION_END                           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xe0 );  /* Provision end event */
    public static final short HCI_CONTROL_MESH_EVENT_NODE_RESET_STATUS                            = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xe1 );  /* Config Node Reset Status */
    public static final short HCI_CONTROL_MESH_EVENT_COMPOSITION_DATA_STATUS                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xe2 );  /* Config Composition Data Status */
    public static final short HCI_CONTROL_MESH_EVENT_FRIEND_STATUS                                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xe3 );  /* Config Friend Status */
    public static final short HCI_CONTROL_MESH_EVENT_GATT_PROXY_STATUS                            = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xe4 );  /* Config GATT Proxy Status */
    public static final short HCI_CONTROL_MESH_EVENT_RELAY_STATUS                                 = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xe5 );  /* Config Relay Status */
    public static final short HCI_CONTROL_MESH_EVENT_DEFAULT_TTL_STATUS                           = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xe6 );  /* Config Default TTL Status */
    public static final short HCI_CONTROL_MESH_EVENT_BEACON_STATUS                                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xe7 );  /* Config Beacon Status */
    public static final short HCI_CONTROL_MESH_EVENT_NODE_IDENTITY_STATUS                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xe8 );  /* Config Node Identity Status */
    public static final short HCI_CONTROL_MESH_EVENT_MODEL_PUBLICATION_STATUS                     = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xe9 );  /* Config Model Publication Status */
    public static final short HCI_CONTROL_MESH_EVENT_MODEL_SUBSCRIPTION_STATUS                    = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xea );  /* Config Model Subscription Status */
    public static final short HCI_CONTROL_MESH_EVENT_MODEL_SUBSCRIPTION_LIST                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xeb );  /* Config Model Subscription List */
    public static final short HCI_CONTROL_MESH_EVENT_NETKEY_STATUS                                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xec );  /* Config NetKey Status */
    public static final short HCI_CONTROL_MESH_EVENT_NETKEY_LIST                                  = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xed );  /* Config Netkey List */
    public static final short HCI_CONTROL_MESH_EVENT_APPKEY_STATUS                                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xee );  /* Config AppKey Status */
    public static final short HCI_CONTROL_MESH_EVENT_APPKEY_LIST                                  = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xef );  /* Config Appkey List */
    public static final short HCI_CONTROL_MESH_EVENT_MODEL_APP_STATUS                             = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xf0 );  /* Config Model App Status */
    public static final short HCI_CONTROL_MESH_EVENT_MODEL_APP_LIST                               = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xf1 );  /* Config Model App List */
    public static final short HCI_CONTROL_MESH_EVENT_HEARTBEAT_SUBSCRIPTION_STATUS                = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xf2 );  /* Config Heartbeat Subscription Status */
    public static final short HCI_CONTROL_MESH_EVENT_HEARTBEAT_PUBLICATION_STATUS                 = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xf3 );  /* Config Heartbeat Publication Status */
    public static final short HCI_CONTROL_MESH_EVENT_NETWORK_TRANSMIT_PARAMS_STATUS               = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xf4 );  /* Config Network Transmit Status */
    public static final short HCI_CONTROL_MESH_EVENT_HEALTH_CURRENT_STATUS                        = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xf5 );  /* Health Current Status */
    public static final short HCI_CONTROL_MESH_EVENT_HEALTH_FAULT_STATUS                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xf6 );  /* Health Fault Status */
    public static final short HCI_CONTROL_MESH_EVENT_HEALTH_PERIOD_STATUS                         = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xf7 );  /* Health Period Status */
    public static final short HCI_CONTROL_MESH_EVENT_HEALTH_ATTENTION_STATUS                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xf8 );  /* Health Attention Status */
    public static final short HCI_CONTROL_MESH_EVENT_LPN_POLL_TIMEOUT_STATUS                      = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xf9 );  /* Low Power node Poll Timeout Status */
    public static final short HCI_CONTROL_MESH_EVENT_KEY_REFRESH_PHASE_STATUS                     = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xfa );  /* Key Refresh Phase Status */
    public static final short HCI_CONTROL_MESH_EVENT_PROXY_FILTER_STATUS                          = (short) ( ( HCI_CONTROL_GROUP_MESH << 8 ) | 0xfb );  /* Proxy Filter Status */

}
