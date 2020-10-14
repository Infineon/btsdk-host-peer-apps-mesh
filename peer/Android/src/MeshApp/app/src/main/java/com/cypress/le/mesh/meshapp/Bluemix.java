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

package com.cypress.le.mesh.meshapp;

import android.util.Log;

import com.amazonaws.org.eclipse.paho.client.mqttv3.MqttMessage;

import org.apache.commons.json.JSONException;
import org.apache.commons.json.JSONObject;


import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 *  Work in progress API document coming soon...
 */
class Bluemix {

	private MqttHandler handler;
	private static final String TAG = "Bluemix";

	String EVENT_ID = "mesh";
	String CMD_ID = "meshcmd";
	String DEVICE_TYPE = "meshdevice";
	String DEVICE_ID = "123456";
    boolean IS_SSL;

    String ORG;
    String ID;
    String AUTHMETHOD;
    String AUTHTOKEN;

	void sendViaBluemix(byte[] data)
	{
		Log.d(TAG, "sendViaBluemix");
		String dataStr = toHexString(data);
		publish(data);
	}

	/**
	 * Run the app
	 */
	public void setBluemixCredentials(String org, String id, String authmethod, String authtoken,
                                      boolean isSSL,String eventId, String cmdId, String devType, String devId) {

		ORG          = org;
		ID           = id;
		AUTHMETHOD   = authmethod;
		AUTHTOKEN    = authtoken;
        IS_SSL       = isSSL;
		EVENT_ID     = eventId;
		CMD_ID       = cmdId;
		DEVICE_TYPE  = devType;
		DEVICE_ID    = devId;

		Log.d(TAG,"org: " + ORG);
        Log.d(TAG,"id: " + ID);
        Log.d(TAG,"authmethod: " + AUTHMETHOD);
        Log.d(TAG, "authtoken" + AUTHTOKEN);
        Log.d(TAG, "isSSL: " + IS_SSL);

        Log.d(TAG, "other params...");
        Log.d(TAG, "EVENT_ID: " + EVENT_ID);
        Log.d(TAG, "CMD_ID: " + CMD_ID);
        Log.d(TAG, "DEVICE_TYPE: " + DEVICE_TYPE);
        Log.d(TAG, "DEVICE_ID: " + DEVICE_ID);
		connect();
	}

    //Connect To Bluemix
    void connect() {
        //Format: a:<orgid>:<app-id>
        String clientId = "a:" + ORG + ":" + ID;
        String serverHost = ORG + MqttUtil.SERVER_SUFFIX;
        handler = new AppMqttHandler();
        handler.connect(serverHost, clientId, AUTHMETHOD, AUTHTOKEN, IS_SSL);
    }

    void disconnect() {
        handler.disconnect();
    }

	/**
	 * This class implements as the application MqttHandler
	 *
	 */
	private class AppMqttHandler extends MqttHandler {

		//Pattern to check whether the events comes from a device for an event
		Pattern pattern = Pattern.compile("iot-2/type/"
				+ DEVICE_TYPE + "/id/(.+)/evt/"
				+ EVENT_ID + "/fmt/json");

		/**
		 * Once a subscribed message is received
		 */
		@Override
		public void messageArrived(String topic, MqttMessage mqttMessage)
				throws Exception {
			super.messageArrived(topic, mqttMessage);

			Matcher matcher = pattern.matcher(topic);
			if (matcher.matches()) {
				String deviceid = matcher.group(1);
				String payload = new String(mqttMessage.getPayload());

				//Parse the payload in Json Format
				JSONObject jsonObject = new JSONObject(payload);
				JSONObject contObj = jsonObject.getJSONObject("d");

			}
		}

        @Override
        public void connectionLost(Throwable throwable) {
            Log.d(TAG,"connectionLost");
            super.connectionLost(throwable);
        }
    }

	public void publish(byte[] data) {
        Log.d(TAG,"publish ");
		JSONObject jsonObj = new JSONObject();
		try {

			jsonObj.put("meshdata", toHexString(data));
		} catch (JSONException e) {
			e.printStackTrace();
		}
		Log.d(TAG,"publish for device " + DEVICE_ID);
        Log.d(TAG,"publishing : "+"iot-2/type/" + DEVICE_TYPE
                + "/id/" + DEVICE_ID + "/cmd/" + CMD_ID
                + "/fmt/json"+ jsonObj.toString());

		//Publish command to one specific device
		//iot-2/type/<type-id>/id/<device-id>/cmd/<cmd-id>/fmt/<format-id>

		handler.publish("iot-2/type/" + DEVICE_TYPE
				+ "/id/" + DEVICE_ID + "/cmd/" + CMD_ID
				+ "/fmt/json", jsonObj.toString(), false, 0);
	}

    public static String toHexString(byte[] bytes) {
        int len = bytes.length;
        if (len == 0)
            return null;

        char[] buffer = new char[len * 2];

        for (int i = 0, index = 0; i < len; i++) {

            int data = bytes[i];
            if (data < 0) {
                data += 256;
            }

            byte n = (byte) (data >>> 4);
            if (n < 10) {
                buffer[index++] = (char) ('0' + n);
            } else {
                buffer[index++] = (char) ('A' + n - 10);
            }

            n = (byte) (data & 0x0F);
            if (n < 10) {
                buffer[index++] = (char) ('0' + n);
            } else {
                buffer[index++] = (char) ('A' + n - 10);
            }
        }
        return new String(buffer);
    }
}
