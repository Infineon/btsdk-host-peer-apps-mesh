/*
 * Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of Cypress Semiconductor
 * Corporation. All rights reserved. This software, including source code, documentation and
 * related materials ("Software"), is owned by Cypress Semiconductor  Corporation or one of its
 * subsidiaries ("Cypress") and is protected by and subject to worldwide patent protection
 * (United States and foreign), United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license agreement accompanying the
 * software package from which you obtained this Software ("EULA"). If no EULA applies, Cypress
 * hereby grants you a personal, nonexclusive, non-transferable license to  copy, modify, and
 * compile the Software source code solely for use in connection with Cypress's  integrated circuit
 * products. Any reproduction, modification, translation, compilation,  or representation of this
 * Software except as specified above is prohibited without the express written permission of
 * Cypress. Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO  WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING,  BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. Cypress reserves the right to make changes to
 * the Software without notice. Cypress does not assume any liability arising out of the application
 * or use of the Software or any product or circuit  described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or failure of the
 * Cypress product may reasonably be expected to result  in significant property damage, injury
 * or death ("High Risk Product"). By including Cypress's product in a High Risk Product, the
 * manufacturer of such system or application assumes  all risk of such use and in doing so agrees
 * to indemnify Cypress against all liability.
 */
package com.cypress.le.mesh.meshapp;
import android.app.Activity;
import android.content.Context;
import android.location.LocationManager;
import android.widget.Toast;

public class Constants {
    public static final int NETWORK_DEVICES_PHONES = 0x01;
    public static final int NETWORK_DEVICES_LIGHTS = 0x02;

    public static final int TRANSPORT_VSC  = 0;
    public static final int TRANSPORT_GATT = 1;
    public static final int TRANSPORT_GATEWAY = 2;

    public static final short WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_SRV  = (short) 0x1000;
    public static final short WICED_BT_MESH_CORE_MODEL_ID_GENERIC_ONOFF_CLNT = (short) 0x1001;
    public static final short WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_SRV  = (short) 0x1002;
    public static final short WICED_BT_MESH_CORE_MODEL_ID_GENERIC_LEVEL_CLNT = (short) 0x1003;
    public static final short WICED_BT_MESH_CORE_MODEL_ID_HSL_SRV            = (short) 0x1307;

    public static final int DEVICE_TYPE_UNKNOWN                = 0;
    public static final int DEVICE_TYPE_GENERIC_ON_OFF_CLIENT  = 1;
    public static final int DEVICE_TYPE_GENERIC_LEVEL_CLIENT   = 2;
    public static final int DEVICE_TYPE_GENERIC_ON_OFF_SERVER  = 3;
    public static final int DEVICE_TYPE_GENERIC_LEVEL_SERVER   = 4;
    public static final int DEVICE_TYPE_LIGHT_DIMMABLE         = 5;
    public static final int DEVICE_TYPE_POWER_OUTLET           = 6;
    public static final int DEVICE_TYPE_LIGHT_HSL              = 7;
    public static final int DEVICE_TYPE_LIGHT_CTL              = 8;
    public static final int DEVICE_TYPE_LIGHT_XYL              = 9;
    public static final int DEVICE_TYPE_VENDOR_SPECIFIC        = 12;
    public static final int DEVICE_TYPE_SENSOR_SERVER          = 10;
    public static final int DEVICE_TYPE_SENSOR_CLIENT          = 11;


    public static final boolean DEFAULT_IS_GATT_PROXY   = true;
    public static final boolean DEFAULT_IS_FRIEND       = true;
    public static final boolean DEFAULT_IS_RELAY        = true;
    public static final boolean DEFAULT_SEND_NET_BEACON = true;
    public static final int DEFAULT_RELAY_XMIT_COUNT = 3;
    public static final int DEFAULT_RELAY_XMIT_INTERVAL = 100;
    public static final int DEFAULT_TTL             = 63;
    public static final int DEFAULT_NET_XMIT_COUNT  = 3;
    public static final int DEFAULT_NET_XMIT_INTERVAL  = 100;

    public static final int DEFAULT_PUBLISH_CREDENTIAL_FLAG = 0;
    public static final int DEFAULT_PUBLISH_TTL = 63;
    public static final int DEFAULT_PUBLISH_PERIOD = 2000;
    public static final int DEFAULT_RETRANSMIT_COUNT = 0;
    public static final int DEFAULT_RETRANSMIT_INTERVAL  = 500;
    public static final int DEFAULT_TRANSITION_TIME = 0xFFFFFFFF;


    public static final String GATT_PROXY_CONNECTED    = "com.cypress.meshproxy.action.CONNECTED";
    public static final String GATT_PROXY_DISCONNECTED = "com.cypress.meshproxy.action.DISCONNECTED";
    public static final String GATT_PROXY_CONNECTED_FOR_CONFIG = "com.cypress.meshproxy.action.config.CONNECTED";
    public static final String SERVICE_CONNECTED = "com.cypress.le.mesh.service.connected";
    public static final String SERVICE_DISCONNECTED = "com.cypress.le.mesh.service.disconnected";
    public static final String GENERIC_ON_OFF_STATUS   = "GENERIC_ON_OFF_STATUS";

    static final int MESH_PROPERTY_UNKNOWN                                                  = 0x0000;
    static final int MESH_PROPERTY_AVERAGE_AMBIENT_TEMPERATURE_IN_A_PERIOD_OF_DAY           = 0x0001;
    static final int MESH_PROPERTY_AVERAGE_INPUT_CURRENT                                    = 0x0002;
    static final int MESH_PROPERTY_AVERAGE_INPUT_VOLTAGE                                    = 0x0003;
    static final int MESH_PROPERTY_AVERAGE_OUTPUT_CURRENT                                   = 0x0004;
    static final int MESH_PROPERTY_AVERAGE_OUTPUT_VOLTAGE                                   = 0x0005;
    static final int MESH_PROPERTY_CENTER_BEAM_INTENSITY_AT_FULL_POWER                      = 0x0006;
    static final int MESH_PROPERTY_CHROMATICALLY_TOLERANCE                                  = 0x0007;
    static final int MESH_PROPERTY_COLOR_RENDERING_INDEX_R9                                 = 0x0008;
    static final int MESH_PROPERTY_COLOR_RENDERING_INDEX_RA                                 = 0x0009;
    static final int MESH_PROPERTY_DEVICE_APPEARANCE                                        = 0x000A;
    static final int MESH_PROPERTY_DEVICE_COUNTRY_OF_ORIGIN                                 = 0x000B;
    static final int MESH_PROPERTY_DEVICE_DATE_OF_MANUFACTURE                               = 0x000C;
    static final int MESH_PROPERTY_DEVICE_ENERGY_USE_SINCE_TURN_ON                          = 0x000D;
    static final int MESH_PROPERTY_DEVICE_FIRMWARE_REVISION                                 = 0x000E;
    static final int MESH_PROPERTY_DEVICE_GLOBAL_TRADE_ITEM_NUMBER                          = 0x000F;
    static final int MESH_PROPERTY_DEVICE_HARDWARE_REVISION                                 = 0x0010;
    static final int MESH_PROPERTY_DEVICE_MANUFACTURER_NAME                                 = 0x0011;
    static final int MESH_PROPERTY_DEVICE_MODEL_NUMBER                                      = 0x0012;
    static final int MESH_PROPERTY_DEVICE_OPERATING_TEMPERATURE_RANGE_SPECIFICATION         = 0x0013;
    static final int MESH_PROPERTY_DEVICE_OPERATING_TEMPERATURE_STATISTICAL_VALUES          = 0x0014;
    static final int MESH_PROPERTY_DEVICE_OVER_TEMPERATURE_EVENT_STATISTICS                 = 0x0015;
    static final int MESH_PROPERTY_DEVICE_POWER_RANGE_SPECIFICATION                         = 0x0016;
    static final int MESH_PROPERTY_DEVICE_RUNTIME_SINCE_TURN_ON                             = 0x0017;
    static final int MESH_PROPERTY_DEVICE_RUNTIME_WARRANTY                                  = 0x0018;
    static final int MESH_PROPERTY_DEVICE_SERIAL_NUMBER                                     = 0x0019;
    static final int MESH_PROPERTY_DEVICE_SOFTWARE_REVISION                                 = 0x001A;
    static final int MESH_PROPERTY_DEVICE_UNDER_TEMPERATURE_EVENT_STATISTICS                = 0x001B;
    static final int MESH_PROPERTY_INDOOR_AMBIENT_TEMPERATURE_STATISTICAL_VALUES            = 0x001C;
    static final int MESH_PROPERTY_INITIAL_CIE_CHROMATICITY_COORDINATES                     = 0x001D;
    static final int MESH_PROPERTY_INITIAL_CORRELATED_COLOR_TEMPERATURE                     = 0x001E;
    static final int MESH_PROPERTY_INITIAL_LUMINOUS_FLUX                                    = 0x001F;
    static final int MESH_PROPERTY_INITIAL_PLANCKIAN_DISTANCE                               = 0x0020;
    static final int MESH_PROPERTY_INPUT_CURRENT_RANGE_SPECIFICATION                        = 0x0021;
    static final int MESH_PROPERTY_INPUT_CURRENT_STATISTICS                                 = 0x0022;
    static final int MESH_PROPERTY_INPUT_OVER_CURRENT_EVENT_STATISTICS                      = 0x0023;
    static final int MESH_PROPERTY_INPUT_OVER_RIPPLE_VOLTAGE_EVENT_STATISTICS               = 0x0024;
    static final int MESH_PROPERTY_INPUT_OVER_VOLTAGE_EVENT_STATISTICS                      = 0x0025;
    static final int MESH_PROPERTY_INPUT_UNDER_CURRENT_EVENT_STATISTICS                     = 0x0026;
    static final int MESH_PROPERTY_INPUT_UNDER_VOLTAGE_EVENT_STATISTICS                     = 0x0027;
    static final int MESH_PROPERTY_INPUT_VOLTAGE_RANGE_SPECIFICATION                        = 0x0028;
    static final int MESH_PROPERTY_INPUT_VOLTAGE_RIPPLE_SPECIFICATION                       = 0x0029;
    static final int MESH_PROPERTY_INPUT_VOLTAGE_STATISTICS                                 = 0x002A;
    static final int MESH_PROPERTY_AMBIENT_LUX_LEVEL_ON                                     = 0x002B;
    static final int MESH_PROPERTY_AMBIENT_LUX_LEVEL_PROLONG                                = 0x002C;
    static final int MESH_PROPERTY_AMBIENT_LUX_LEVEL_STANDBY                                = 0x002D;
    static final int MESH_PROPERTY_LIGHTNESS_ON                                             = 0x002E;
    static final int MESH_PROPERTY_LIGHTNESS_PROLONG                                        = 0x002F;
    static final int MESH_PROPERTY_LIGHTNESS_STANDBY                                        = 0x0030;
    static final int MESH_PROPERTY_REGULATOR_ACCURACY                                       = 0x0031;
    static final int MESH_PROPERTY_REGULATOR_KID                                            = 0x0032;
    static final int MESH_PROPERTY_REGULATOR_KIU                                            = 0x0033;
    static final int MESH_PROPERTY_REGULATOR_KPD                                            = 0x0034;
    static final int MESH_PROPERTY_REGULATOR_KPU                                            = 0x0035;
    static final int MESH_PROPERTY_TIME_FADE                                                = 0x0036;
    static final int MESH_PROPERTY_TIME_FADE_ON                                             = 0x0037;
    static final int MESH_PROPERTY_TIME_FADE_STANDBY_AUTO                                   = 0x0038;
    static final int MESH_PROPERTY_TIME_FADE_STANDBY_MANUAL                                 = 0x0039;
    static final int MESH_PROPERTY_TIME_OCCUPANCY_DELAY                                     = 0x003A;
    static final int MESH_PROPERTY_TIME_PROLONG                                             = 0x003B;
    static final int MESH_PROPERTY_TIME_RUN_ON                                              = 0x003C;
    static final int MESH_PROPERTY_LUMEN_MAINTENANCE_FACTOR                                 = 0x003D;
    static final int MESH_PROPERTY_LUMINOUS_EFFICICACY                                      = 0x003E;
    static final int MESH_PROPERTY_LUMINOUS_ENERGY_SINCE_TURN_ON                            = 0x003F;
    static final int MESH_PROPERTY_LUMINOUS_EXPOSURE                                        = 0x0040;
    static final int MESH_PROPERTY_LUMINOUS_FLUX_RANGE                                      = 0x0041;
    static final int MESH_PROPERTY_MOTION_SENSED                                            = 0x0042;
    static final int MESH_PROPERTY_MOTION_THRESHOLD                                         = 0x0043;
    static final int MESH_PROPERTY_OPEN_CIRCUIT_EVENT_STATISTICS                            = 0x0044;
    static final int MESH_PROPERTY_OUTDOOR_STATISTICAL_VALUES                               = 0x0045;
    static final int MESH_PROPERTY_OUTPUT_CURRENT_RANGE                                     = 0x0046;
    static final int MESH_PROPERTY_OUTPUT_CURRENT_STATISTICS                                = 0x0047;
    static final int MESH_PROPERTY_OUTPUT_RIPPLE_VOLTAGE_SPECIFICATION                      = 0x0048;
    static final int MESH_PROPERTY_OUTPUT_VOLTAGE_RANGE                                     = 0x0049;
    static final int MESH_PROPERTY_OUTPUT_VOLTAGE_STATISTICS                                = 0x004A;
    static final int MESH_PROPERTY_OVER_OUTPUT_RIPPLE_VOLTAGE_EVENT_STATISTICS              = 0x004B;
    static final int MESH_PROPERTY_PEOPLE_COUNT                                             = 0x004C;
    static final int MESH_PROPERTY_PRESENCE_DETECTED                                        = 0x004D;
    static final int MESH_PROPERTY_PRESENT_AMBIENT_LIGHT_LEVEL                              = 0x004E;
    static final int MESH_PROPERTY_PRESENT_AMBIENT_TEMPERATURE                              = 0x004F;
    static final int MESH_PROPERTY_PRESENT_CIE_CHROMATICITY_COORDINATES                     = 0x0050;
    static final int MESH_PROPERTY_PRESENT_CORRELATED_COLOR_TEMPERATURE                     = 0x0051;
    static final int MESH_PROPERTY_PRESENT_DEVICE_INPUT_POWER                               = 0x0052;
    static final int MESH_PROPERTY_PRESENT_DEVICE_OPERATING_EFFICIENCY                      = 0x0053;
    static final int MESH_PROPERTY_PRESENT_DEVICE_OPERATING_TEMPERATURE                     = 0x0054;
    static final int MESH_PROPERTY_PRESENT_ILLUMINANCE                                      = 0x0055;
    static final int MESH_PROPERTY_PRESENT_INDOOR_AMBIENT_TEMPERATURE                       = 0x0056;
    static final int MESH_PROPERTY_PRESENT_INPUT_CURRENT                                    = 0x0057;
    static final int MESH_PROPERTY_PRESENT_INPUT_RIPPLE_VOLTAGE                             = 0x0058;
    static final int MESH_PROPERTY_PRESENT_INPUT_VOLTAGE                                    = 0x0059;
    static final int MESH_PROPERTY_PRESENT_LUMINOUS_FLUX                                    = 0x005A;
    static final int MESH_PROPERTY_PRESENT_OUTDOOR_AMBIENT_TEMPERATURE                      = 0x005B;
    static final int MESH_PROPERTY_PRESENT_OUTPUT_CURRENT                                   = 0x005C;
    static final int MESH_PROPERTY_PRESENT_OUTPUT_VOLTAGE                                   = 0x005D;
    static final int MESH_PROPERTY_PRESENT_PLANCKIAN_DISTANCE                               = 0x005E;
    static final int MESH_PROPERTY_PRESENT_RELATIVE_OUTPUT_RIPPLE_VOLTAGE                   = 0x005F;
    static final int MESH_PROPERTY_RELATIVE_DEVICE_ENERGY_USE_IN_A_PERIOD_OF_DAY            = 0x0060;
    static final int MESH_PROPERTY_RELATIVE_DEVICE_RUNTIME_IN_A_GENERIC_LEVEL_RANGE         = 0x0061;
    static final int MESH_PROPERTY_RELATIVE_EXPOSURE_TIME_IN_AN_ILLUMINANCE_RANGE           = 0x0062;
    static final int MESH_PROPERTY_RELATIVE_RUNTIME_IN_A_CORRELATED_COLOR_TEMPERATURE_RANGE = 0x0063;
    static final int MESH_PROPERTY_RELATIVE_RUNTIME_IN_A_DEVICE_OPERATING_TEMPERATURE_RANGE = 0x0064;
    static final int MESH_PROPERTY_RELATIVE_RUNTIME_IN_AN_INPUT_CURRENT_RANGE               = 0x0065;
    static final int MESH_PROPERTY_RELATIVE_RUNTIME_IN_AN_INPUT_VOLTAGE_RANGE               = 0x0066;
    static final int MESH_PROPERTY_SHORT_CIRCUIT_EVENT_STATISTICS                           = 0x0067;
    static final int MESH_PROPERTY_TIME_SINCE_MOTION_SENSED                                 = 0x0068;
    static final int MESH_PROPERTY_TIME_SINCE_PRESENCE_DETECTED                             = 0x0069;
    static final int MESH_PROPERTY_TOTAL_DEVICE_ENERGY_USE                                  = 0x006A;
    static final int MESH_PROPERTY_TOTAL_DEVICE_OFF_ON_CYCLES                               = 0x006B;
    static final int MESH_PROPERTY_TOTAL_DEVICE_POWER_ON_CYCLES                             = 0x006C;
    static final int MESH_PROPERTY_TOTAL_DEVICE_POWER_ON_TIME                               = 0x006D;
    static final int MESH_PROPERTY_TOTAL_DEVICE_RUNTIME                                     = 0x006E;
    static final int MESH_PROPERTY_TOTAL_LIGHT_EXPOSURE_TIME                                = 0x006F;
    static final int MESH_PROPERTY_TOTAL_LUMINOUS_ENERGY                                    = 0x0070;

    public static final byte WICED_BT_MESH_DFU_STATE_INIT                       = 0;
    public static final byte WICED_BT_MESH_DFU_STATE_VALIDATE_NODES             = 1;
    public static final byte WICED_BT_MESH_DFU_STATE_GET_DISTRIBUTOR            = 2;
    public static final byte WICED_BT_MESH_DFU_STATE_UPLOAD                     = 3;
    public static final byte WICED_BT_MESH_DFU_STATE_DISTRIBUTE                 = 4;
    public static final byte WICED_BT_MESH_DFU_STATE_APPLY                      = 5;
    public static final byte WICED_BT_MESH_DFU_STATE_COMPLETE                   = 6;
    public static final byte WICED_BT_MESH_DFU_STATE_FAILED                     = 7;

    public static final byte WICED_BT_MESH_FW_UPDATE_PHASE_IDLE                 = 0x00;
    public static final byte WICED_BT_MESH_FW_UPDATE_PHASE_TRANSFER_ERROR       = 0x01;
    public static final byte WICED_BT_MESH_FW_UPDATE_PHASE_TRANSFER_ACTIVE      = 0x02;
    public static final byte WICED_BT_MESH_FW_UPDATE_PHASE_VERIFICATION_ACTIVE  = 0x03;
    public static final byte WICED_BT_MESH_FW_UPDATE_PHASE_VERIFICATION_SUCCESS = 0x04;
    public static final byte WICED_BT_MESH_FW_UPDATE_PHASE_VERIFICATION_FAILED  = 0x05;
    public static final byte WICED_BT_MESH_FW_UPDATE_PHASE_APPLY_ACTIVE         = 0x06;
    public static final byte WICED_BT_MESH_FW_UPDATE_PHASE_TRANSFER_CANCELLED   = 0x07;
    public static final byte WICED_BT_MESH_FW_UPDATE_PHASE_UNKNOWN              = 0x08;
    public static final byte WICED_BT_MESH_FW_UPDATE_PHASE_APPLY_SUCCESS        = 0x09;
    public static final byte WICED_BT_MESH_FW_UPDATE_PHASE_APPLY_FAILED         = 0x0A;


    public static String toHexString(byte[] bytes) {
        return toHexString(bytes, ' ');
    }

    public static String toHexString(byte[] bytes, char separator) {
        int len = bytes.length;
        if (len == 0)
            return null;

        char[] buffer = new char[len * 3 - 1];

        for (int i = 0, index = 0; i < len; i++) {
            if (i > 0) {
                buffer[index++] = separator;
            }

            int data = bytes[i];
            if (data < 0) {
                data += 256;
            }

            byte n = (byte) (data >>> 4);
            if (n < 10) {
                buffer[index++] = (char) ('0' + n);
            }
            else {
                buffer[index++] = (char) ('A' + n - 10);
            }

            n = (byte) (data & 0x0F);
            if (n < 10) {
                buffer[index++] = (char) ('0' + n);
            }
            else {
                buffer[index++] = (char) ('A' + n - 10);
            }
        }
        return new String(buffer);
    }


    public static boolean IsLocationServiceEnabled(Context context) {
        LocationManager locationManager = null;
        boolean gps_enabled = false;
        boolean network_enabled = false;
        if (locationManager == null) {
            locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        }
        try {
            gps_enabled = locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER);
        }
        catch(Exception ex){
            //do nothing...
        }
        try {
            network_enabled = locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER);
        }
        catch(Exception ex){
            //do nothing...
        }
        return gps_enabled || network_enabled;
    }


    private static Toast mToast = null;

    public static void Show(final Activity activity, final String text, final int duration) {
        activity.runOnUiThread(new Runnable() {
            public void run() {
                if (mToast == null || !mToast.getView().isShown()) {
                    if (mToast != null) {
                        mToast.cancel();
                    }
                }
                mToast = Toast.makeText(activity, text, duration);
                mToast.show();
            }
        });
    }
}
