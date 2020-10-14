/*
* Copyright 2018, Cypress Semiconductor Corporation or a subsidiary of Cypress Semiconductor
 *  Corporation. All rights reserved. This software, including source code, documentation and  related
 * materials ("Software"), is owned by Cypress Semiconductor  Corporation or one of its
 *  subsidiaries ("Cypress") and is protected by and subject to worldwide patent protection
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
 *  manufacturer of such system or application assumes  all risk of such use and in doing so agrees
 * to indemnify Cypress against all liability.
*/
package com.cypress.le.mesh.meshcore;

import java.util.UUID;

public interface IMeshNativeCallback {
    /**
     * Demonstrates some basic types that you can use as parameters
     * and return values in AIDL.
     */
    void onProvGattPktReceivedCallback(byte[] p_data, int len);
    void onProxyGattPktReceivedCallback(byte[] p_data, int len);
    void onDeviceFound(UUID uuid, String name);
    void onLinkStatus(byte isConnected, int connId, short addr, byte isOverGatt);
    void onNetworkOpenedCb(byte status);
    void onDatabaseChangedCallback(String meshName);
    void onComponentInfoStatus(byte status, String componentName, String componentInfo);
    void onLightLcModeStatus(String deviceName, int mode);
    void onLightLcOccupancyModeStatus(String deviceName, int mode);
    void onLightLcPropertyStatus(String deviceName, int propertyId, int value);

    void meshClientProvisionCompletedCb(byte isSuccess, byte[] uuid);
    void meshClientOnOffStateCb(String deviceName, byte targetOnOff, byte presentOnOff, int remainingTime);
    void meshClientLevelStateCb(String deviceName, short targetLevel, short presentLevel, int remainingTime);
    void meshClientHslStateCb(String deviceName, int lightness, int hue, int saturation, int remainingTime);
    void meshClientCtlStateCb(String deviceName, int presentLightness, short presentTemperature, int targetLightness, short targetTemperature, int remainingTime);
    void meshClientLightnessStateCb(String deviceName, int target, int present, int remainingTime);
    boolean meshClientDfuIsOtaSupportedCb();
    void meshClientDfuStartOtaCb(String firmwareFileName);
    void meshClientDfuStatusCb(byte state, byte[] data);
    void meshClientSensorStatusCb(String componentName, int propertyId, byte[] data);
    void meshClientVendorStatusCb(short source, short companyId, short modelId, byte opcode, byte ttl, byte[] data, short dataLen);

    //GATT APIS
    void meshClientAdvScanStartCb();
    void meshClientAdvScanStopCb();
    void meshClientConnect(byte[] bdaddr);
    void meshClientDisconnect(int connId);
    void meshClientNodeConnectStateCb(byte status, String componentName);
    void meshClientSetScanTypeCb(byte scanType);
}
