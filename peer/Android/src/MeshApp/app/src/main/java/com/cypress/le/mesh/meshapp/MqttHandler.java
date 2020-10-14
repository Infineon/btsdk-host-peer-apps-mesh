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

import com.amazonaws.org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import com.amazonaws.org.eclipse.paho.client.mqttv3.MqttCallback;
import com.amazonaws.org.eclipse.paho.client.mqttv3.MqttClient;
import com.amazonaws.org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import com.amazonaws.org.eclipse.paho.client.mqttv3.MqttException;
import com.amazonaws.org.eclipse.paho.client.mqttv3.MqttMessage;
import com.amazonaws.org.eclipse.paho.client.mqttv3.MqttPersistenceException;
import com.amazonaws.org.eclipse.paho.client.mqttv3.persist.MqttDefaultFilePersistence;


class MqttHandler implements MqttCallback {
	private final static String DEFAULT_TCP_PORT = "1883";
	private final static String DEFAULT_SSL_PORT = "8883";
	private static final String TAG = "MQTT Handler";

	private MqttClient client = null;

	public MqttHandler() {

	}

	@Override
	public void connectionLost(Throwable throwable) {
		if (throwable != null) {
			throwable.printStackTrace();
		}
	}

	/**
	 * One message is successfully published
	 */
	@Override
	public void deliveryComplete(IMqttDeliveryToken iMqttDeliveryToken) {
		Log.d(TAG, ".deliveryComplete() entered");
	}

	/**
	 * Received one subscribed message
	 */
	@Override
	public void messageArrived(String topic, MqttMessage mqttMessage)
			throws Exception {
		String payload = new String(mqttMessage.getPayload());
		System.out.println(".messageArrived - Message received on topic "
				+ topic + ": message is " + payload);
	}

	public void connect(String serverHost, String clientId, String authmethod,
			String authtoken, boolean isSSL) {
		MqttDefaultFilePersistence persistence = new MqttDefaultFilePersistence();

		// check if client is already connected
		if (!isMqttConnected()) {
			String connectionUri = null;

			//tcp://<org-id>.messaging.internetofthings.ibmcloud.com:1883
			//ssl://<org-id>.messaging.internetofthings.ibmcloud.com:8883
			if (isSSL) {
				connectionUri = "ssl://" + serverHost + ":" + DEFAULT_SSL_PORT;
			} else {
				connectionUri = "tcp://" + serverHost + ":" + DEFAULT_TCP_PORT;
			}

			if (client != null) {
				try {
					client.disconnect();
				} catch (MqttException e) {
					e.printStackTrace();
				}
				client = null;
			}

			try {
				client = new MqttClient(connectionUri, clientId, null);
			} catch (MqttException e) {
				e.printStackTrace();
			}

			client.setCallback(this);

			// create MqttConnectOptions and set the clean session flag
			MqttConnectOptions options = new MqttConnectOptions();
			options.setCleanSession(true);

			options.setUserName(authmethod);
			options.setPassword(authtoken.toCharArray());

			//If SSL is used, do not forget to use TLSv1.2
			if (isSSL) {
				java.util.Properties sslClientProps = new java.util.Properties();
				sslClientProps.setProperty("com.ibm.ssl.protocol", "TLSv1.2");
				options.setSSLProperties(sslClientProps);
			}

			try {
				// connect
				client.connect(options);
				Log.d(TAG,"Connected to " + connectionUri);
			} catch (MqttException e) {
				e.printStackTrace();
			}

		}

	}

	/**
	 * Disconnect MqttClient from the MQTT server
	 */
	public void disconnect() {

		// check if client is actually connected
		if (isMqttConnected()) {
			try {
				// disconnect
				client.disconnect();
			} catch (MqttException e) {
				e.printStackTrace();
			}
		}
	}

	/**
	 * Subscribe MqttClient to a topic
	 *
	 * @param topic
	 *            to subscribe to
	 * @param qos
	 *            to subscribe with
	 */
	public void subscribe(String topic, int qos) {

		// check if client is connected
		if (isMqttConnected()) {
			try {
				client.subscribe(topic, qos);
				Log.d(TAG,"Subscribed: " + topic);

			} catch (MqttException e) {
				e.printStackTrace();
			}
		} else {
			connectionLost(null);
		}
	}

	/**
	 * Unsubscribe MqttClient from a topic
	 *
	 * @param topic
	 *            to unsubscribe from
	 */
	public void unsubscribe(String topic) {
		// check if client is connected
		if (isMqttConnected()) {
			try {

				client.unsubscribe(topic);
			} catch (MqttException e) {
				e.printStackTrace();
			}
		} else {
			connectionLost(null);
		}
	}

	/**
	 * Publish message to a topic
	 *
	 * @param topic
	 *            to publish the message to
	 * @param message
	 *            JSON object representation as a string
	 * @param retained
	 *            true if retained flag is requred
	 * @param qos
	 *            quality of service (0, 1, 2)
	 */
	public void publish(String topic, String message, boolean retained, int qos) {
		// check if client is connected
		if (isMqttConnected()) {
			// create a new MqttMessage from the message string
			MqttMessage mqttMsg = new MqttMessage(message.getBytes());
			// set retained flag
			mqttMsg.setRetained(retained);
			// set quality of service
			mqttMsg.setQos(qos);
			try {
				client.publish(topic, mqttMsg);
			} catch (MqttPersistenceException e) {
				e.printStackTrace();
			} catch (MqttException e) {
				e.printStackTrace();
			}
		} else {
			connectionLost(null);
		}
	}

	/**
	 * Checks if the MQTT client has an active connection
	 *
	 * @return True if client is connected, false if not.
	 */
	private boolean isMqttConnected() {
		boolean connected = false;
		try {
			if ((client != null) && (client.isConnected())) {
				connected = true;
			}
		} catch (Exception e) {
			// swallowing the exception as it means the client is not connected
		}
		return connected;
	}

}
