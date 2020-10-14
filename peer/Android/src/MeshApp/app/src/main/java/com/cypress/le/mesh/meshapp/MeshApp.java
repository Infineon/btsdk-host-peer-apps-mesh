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

import android.app.Application;
import android.content.SharedPreferences;
import com.cypress.le.mesh.meshframework.MeshController;


public class MeshApp extends Application {
    private static final String TAG = "MeshApp";

    private static SharedPreferences prefs;
    private static int mTransport;
    MeshController mMesh = null;
    String mCurrNetwork = null;

    private static int mPrefNetDevices = Constants.NETWORK_DEVICES_PHONES;
    private static int mPrefTransport  = Constants.TRANSPORT_GATT;

    private String mBigIp = null;


    @Override
    public void onCreate() {
        super.onCreate();
    }

    public void setMesh(MeshController mesh) {
        mMesh = mesh;
    }

    public void setBigIP(String bigIP){
        mBigIp = bigIP;
    }


    public String getBigIP(){
        return mBigIp;
    }

    public MeshController getMesh() {
        return mMesh;
    }

    public void setCurrNetwork(String network) {
        mCurrNetwork = network;
    }

    public String getCurrNetwork() {
        return mCurrNetwork;
    }

    public byte connectToNetwork(int transport) {
        if(mPrefTransport == Constants.TRANSPORT_GATT && transport != mPrefTransport)
            mMesh.disconnectNetwork();

        Sleep(100);
        mPrefTransport = transport;
        if(mCurrNetwork!=null) {
            return mMesh.connectNetwork((byte)10);
        }
        return MeshController.MESH_CLIENT_ERR_INVALID_STATE;

    }


    void Sleep(int ms) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public int getPreferredTransport() {
        return mPrefTransport;
    }

    public void setPreferredTransport(int transport) {
         mPrefTransport = transport;
    }

}
