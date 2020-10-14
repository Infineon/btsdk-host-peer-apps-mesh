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

import android.content.Context;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.amazonaws.SDKGlobalConfiguration;
import com.amazonaws.auth.AWSCredentialsProvider;
import com.amazonaws.auth.BasicAWSCredentials;
import com.amazonaws.auth.CognitoCachingCredentialsProvider;
import com.amazonaws.internal.StaticCredentialsProvider;
import com.amazonaws.regions.Region;
import com.amazonaws.regions.Regions;
import com.amazonaws.services.iotdata.AWSIotDataClient;
import com.amazonaws.services.iotdata.model.GetThingShadowRequest;
import com.amazonaws.services.iotdata.model.GetThingShadowResult;
import com.amazonaws.services.iotdata.model.PublishRequest;

import com.amazonaws.services.iot.AWSIotClient;


import org.apache.http.HttpResponse;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.ResponseHandler;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.DefaultHttpClient;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.nio.ByteBuffer;
import java.security.KeyStore;
import java.util.Properties;
import java.util.UUID;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.amazonaws.mobileconnectors.iot.AWSIotKeystoreHelper;
import com.amazonaws.mobileconnectors.iot.AWSIotMqttClientStatusCallback;
import com.amazonaws.mobileconnectors.iot.AWSIotMqttLastWillAndTestament;
import com.amazonaws.mobileconnectors.iot.AWSIotMqttManager;
import com.amazonaws.mobileconnectors.iot.AWSIotMqttNewMessageCallback;
import com.amazonaws.mobileconnectors.iot.AWSIotMqttQos;
import com.amazonaws.services.iot.model.AttachPrincipalPolicyRequest;
import com.amazonaws.services.iot.model.CreateKeysAndCertificateRequest;
import com.amazonaws.services.iot.model.CreateKeysAndCertificateResult;

import static com.cypress.le.mesh.meshapp.FragmentRoom.readProperties;

class CloudConfigHelper {

    private Context mCtx = null;
    private static final String TAG = "CloudConfigHelper";
    private String bigIpAddr = "";
    AWSIotDataClient client;
    IHelperCallback mCallback = null;
    HttpRequest mRequest;
    HttpURLConnection mUrlConnection;
    AWSIotClient mIotAndroidClient;
    AWSIotMqttManager mqttManager = null;

    String clientId;
    String keystorePath;
    String keystoreName;
    String keystorePassword;
    KeyStore clientKeyStore = null;
    String certificateId;
    final int EVENT_CONNECT  = 1;
    final int EVENT_INIT     = 2;

    String meshUrlBaseString;

    // customer specific endpoint should be placed in AWS.Conf
    String CUSTOMER_SPECIFIC_ENDPOINT ;
    String AWS_IOT_POLICY_NAME = "mesh_app_policy";
    final String SUBSCRIBE_CONN_TOPIC = "proxy_conn_response";
    final String SUBSCRIBE_DATA_TOPIC = "proxy_data";
    final String PUBLISH_CONN_TOPIC = "proxy_conn_request";
    final String PUBLISH_DATA_TOPIC = "mesh_data";
    String CONNECT = "1";
    String DISCONNECT = "0";

    private String Data;
    String desiredState = "";

    Region region = Region.getRegion(Regions.US_EAST_1);
    CognitoCachingCredentialsProvider credentialsProvider;
    // cognito pool id should be placed in AWS.conf
    private static String COGNITO_POOL_ID ;
    // Region of AWS IoT
    private static final Regions MY_REGION = Regions.US_EAST_1;



    // Filename of KeyStore file on the filesystem
    private static final String KEYSTORE_NAME = "iot_keystore";
    // Password for the private key in the KeyStore
    private static final String KEYSTORE_PASSWORD = "password";
    // Certificate and key aliases in the KeyStore
    private static final String CERTIFICATE_ID = "default";


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

    public boolean init(Context ctx) {
        mCtx = ctx;
        return true;
    }


    public boolean connectRest(String ipAddr){
        bigIpAddr = ipAddr;
        Log.d(TAG, "ipAddr = " + ipAddr);
        if(!ipAddr.equals("aws-iot"))
        {
            mRequest = new HttpRequest();
            mRequest.execute();
        }
        return true;
    }

    private void AWSSubscribe(final String topic) {

        mqttManager.subscribeToTopic(topic, AWSIotMqttQos.QOS0,
                new AWSIotMqttNewMessageCallback() {
                    @Override
                    public void onMessageArrived(final String topic, final byte[] data) {
                        try {
                            String message = new String(data, "UTF-8");
                            Log.d(TAG,"onMessageArrived topic"+topic+" data"+message);
                            switch (topic)
                            {
                                case SUBSCRIBE_CONN_TOPIC :
                                    mCallback.onConnectionStateChange(message.equals("1")?true:false);
                                    break;
                                case SUBSCRIBE_DATA_TOPIC :
                                    final byte[] databyte = parseHexBinary(message);
//                                    byte[] proxypkt = new byte[data.length];
//                                    System.arraycopy(databyte,0,proxypkt,0,databyte.length);
                                    Log.d(TAG,"SUBSCRIBE_DATA_TOPIC topic"+topic+" data"+toHexString(databyte));
                                    mCallback.onDataReceived(databyte);
                                    break;
                            }
                        } catch (UnsupportedEncodingException e) {
                            e.printStackTrace();
                        }
                    }
                });
    }

    private void AWSPublish(final String data, final String topic) {

        try {
            mqttManager.publishString(data, topic, AWSIotMqttQos.QOS0);
        } catch (Exception e) {
            Log.e(TAG, "Publish error.", e);
        }
    }

    private void AWSConnect() {
        if(mqttManager == null)
            mHandler.sendEmptyMessage(EVENT_INIT);
        else
            mHandler.sendEmptyMessage(EVENT_CONNECT);
    }

    private void AWSinit() {
        clientId = UUID.randomUUID().toString();
        // get the cognito pool id from the AWS conf, file

        try {
            Properties props = readProperties("/sdcard/AWS.conf");
            if(props != null) {
//                Log.d(TAG, "## reading COGNITO_POOL_ID ");
                COGNITO_POOL_ID = props.getProperty("CognitoPoolId");
            } else {
                Log.d(TAG, "Error reading AWS conf");
            }
        } catch (Exception e) {
            Log.d(TAG,"Cannot read AWS Credentials");
            e.printStackTrace();
        }

        // Initialize the AWS Cognito credentials provider
        credentialsProvider = new CognitoCachingCredentialsProvider(
                mCtx, // context
                COGNITO_POOL_ID, // Identity Pool ID
                MY_REGION // Region
        );

        Region region = Region.getRegion(MY_REGION);
        initIoTClient();
    }


    private byte[] offsetBuffer(byte[] value, int offset) {
        if (offset >= value.length)
            return new byte[0];
        byte[] res = new byte[value.length - offset];
        for (int i = offset; i != value.length; ++i) {
            res[i - offset] = value[i];
        }
        return res;
    }

    private ResponseHandler<String> mHttpResponseHandler = new ResponseHandler<String>() {

        @Override
        public String handleResponse(final HttpResponse response)
                throws ClientProtocolException {
            int status = response.getStatusLine().getStatusCode();
            Log.d(TAG, "Status: " + status);
            if (status >= 200 && status < 300) {
                Log.i(TAG, "status is >=200 and < 300");
                Log.e(TAG,"action_sent_rest_data");
                //reset the semaphore
                try{
                    response.getEntity().getContent().close();
                }catch(IOException e){
                    e.printStackTrace();
                }

            } else {
                throw new ClientProtocolException(
                        "Unexpected response status: " + status);
            }
            return "SUCCESS";
        }

    };


    boolean send(byte[] data) {
        //meshUrlBaseString = "http://" + bigIpAddr +"/mesh/sendp2m/112233445566778899";
        if(bigIpAddr == null){
            Log.e(TAG,"send : bigIp set is null , cannot execute the command");
            return false;
        }
        Log.d(TAG, "send : bigIpAddr = " + bigIpAddr + "data"+toHexString(data));
        //sendViaRest(data, type);
        if(bigIpAddr.equals("aws-iot")){
            sendViaAWS(data);
        } else{
            sendViaRest(data);
        }
        return true;
    }


    private void sendViaAWS(byte[] data)
    {
        Log.d(TAG, "sendViaAWS");
        desiredState = toHexString(data);
        desiredState = "{ \"status\": \""+desiredState+"\"}";

        //updateShadow();
        AWSPublish(desiredState, PUBLISH_DATA_TOPIC);
    }

    public void registerCb(IHelperCallback callback) {
        mCallback = callback;
    }

    public void connectGateway() {
        Log.d(TAG, "connectGateway");
        if(bigIpAddr.equals("aws-iot")){
            AWSConnect();
        } else{
            connectGatewayRest();
        }
    }

    public void disconnectGateway() {
        Log.d(TAG, "disconnectGateway");
        if(bigIpAddr.equals("aws-iot")){
            disconnectGatewayAWS();
        } else{
            disconnectGatewayRest();
        }
    }

    public interface IHelperCallback {
        void onConnectionStateChange(boolean state);
        void onDataReceived(byte[] data);
    }

    private class HttpRequest extends AsyncTask {
        @Override
        protected Object doInBackground(Object[] params){
            try {
                String urlStr = "http:"+"//"+bigIpAddr+"/mesh/subscribe/sse";
                URL url = new URL(urlStr);
                mUrlConnection = (HttpURLConnection) url.openConnection();
                Log.d("SSE", "http response: " + mUrlConnection.getResponseCode());

                //Object inputStream = urlConnection.getContent();
                InputStream inputStream = new BufferedInputStream(mUrlConnection.getInputStream());

                String str = readStream(inputStream);
                Log.d("SSE reading stream", str+"");


            } catch (MalformedURLException e) {
                e.printStackTrace();
            } catch (IOException e) {
                Log.e("SSE activity", "Error on url openConnection: "+e.getMessage());
                e.printStackTrace();
            }

            return null;
        }
    }

    public static byte[] hexStringToByteArray(String s) {
        byte[] b = new byte[s.length() / 2];
        for (int i = 0; i < b.length; i++) {
            int index = i * 2;
            int v = Integer.parseInt(s.substring(index, index + 2), 16);
            b[i] = (byte) v;
        }
        return b;
    }
    String incomingPacket = "";
    private String readStream(InputStream inputStream) {
        Log.d(TAG,"ServerSentEvents SSE DATA readstream start");
        BufferedReader reader = null;
        StringBuffer response = new StringBuffer();

        try{
            reader = new BufferedReader(new InputStreamReader(inputStream));
            String line = "";
            JSONObject obj = null;
            while((line = reader.readLine()) != null){
                Log.d("ServerSentEvents 1", "SSE event: "+line);
                obj = new JSONObject(line);
                String data = obj.getString("data ");
                final byte[] databyte = parseHexBinary(data);
                Log.d(TAG,"ServerSentEvents SSE DATA : "+toHexString(databyte)+"len"+databyte.length);
                handleProxyData(databyte);
            }

        } catch (IOException e){
            e.printStackTrace();
        } catch (JSONException e) {
            e.printStackTrace();
        } finally {
            if(reader != null){
                try{
                    reader.close();
                }catch (IOException e){
                    e.printStackTrace();
                }
            }
        }

        Log.d(TAG,"ServerSentEvents SSE DATA return");
        return response.toString();
    }

    private void handleProxyData(byte[] databyte) {
        byte type = databyte[0];
        Log.d(TAG,"handleProxyData"+type);
        switch (type)
        {
            case 0x00 :
                mCallback.onConnectionStateChange(databyte[1] > 0 ?true:false);
                if(databyte[1] == 0)
                {
                    mUrlConnection.disconnect();
                    mRequest.cancel(true);
                }
                break;
            case 0x01 :
                byte[] proxypkt = new byte[databyte.length - 1];
                System.arraycopy(databyte,1,proxypkt,0,databyte.length-1);
                mCallback.onDataReceived(proxypkt);
                break;
        }
    }


    private void sendViaRest(byte[] data) {
        Log.d(TAG,"sendViaRest");
        Data = new String();
        meshUrlBaseString = "http://" + bigIpAddr +"/mesh/";
        Data = "meshdata" + "/value/" + toHexString(data);
        new Thread() {
            @Override
            public void run() {
                try {

                    Log.i(TAG, "constructed url  = " + meshUrlBaseString +Data);
                    URI uri = new URI(meshUrlBaseString + Data);
                    HttpClient httpclient = new DefaultHttpClient();
                    Log.i(TAG, "calling new HttpGet(uri) : strlen of uri = " + uri.toString().length());
                    HttpGet get = new HttpGet(uri);
                    Log.i(TAG, "calling httpclient.execute(put, mHttpResponseHandler)");
                    httpclient.execute(get, mHttpResponseHandler);


                    Log.i(TAG, "completed httpclient.execute(put, mHttpResponseHandler)");
                    //meshUrlBaseString = null;
                    get = null;
                } catch (URISyntaxException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                } catch (Exception e) {
                    Log.i(TAG, "catching exception in general");
                    e.printStackTrace();
                }
            }
        }.start();
        Log.e(TAG, "end of send");
    }

    private void disconnectGatewayAWS() {
        Log.d(TAG,"connectGatewayAWS");
        AWSPublish(DISCONNECT, PUBLISH_CONN_TOPIC);
    }

    private void connectGatewayRest() {
        Log.d(TAG,"connectGatewayRest");
        Data = new String();
        meshUrlBaseString = "http://" + bigIpAddr +"/mesh/connect";

        new Thread() {
            @Override
            public void run() {
                try {

                    Log.i(TAG, "constructed url  = " + meshUrlBaseString );
                    URI uri = new URI(meshUrlBaseString);
                    HttpClient httpclient = new DefaultHttpClient();
                    Log.i(TAG, "calling new HttpGet(uri) : strlen of uri = " + uri.toString().length());
                    HttpGet get = new HttpGet(uri);
                    Log.i(TAG, "calling httpclient.execute(put, mHttpResponseHandler)");
                    httpclient.execute(get, mHttpResponseHandler);


                    Log.i(TAG, "completed httpclient.execute(put, mHttpResponseHandler)");
                    //meshUrlBaseString = null;
                    get = null;
                } catch (URISyntaxException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                } catch (Exception e) {
                    Log.i(TAG, "catching exception in general");
                    e.printStackTrace();
                }
            }
        }.start();
        Log.e(TAG, "end of send");
    }

    private void disconnectGatewayRest() {
        Log.d(TAG,"disconnectGatewayRest");
        Data = new String();
        meshUrlBaseString = "http://" + bigIpAddr +"/mesh/disconnect";

        new Thread() {
            @Override
            public void run() {
                try {

                    Log.i(TAG, "constructed url  = " + meshUrlBaseString );
                    URI uri = new URI(meshUrlBaseString);
                    HttpClient httpclient = new DefaultHttpClient();
                    Log.i(TAG, "calling new HttpGet(uri) : strlen of uri = " + uri.toString().length());
                    HttpGet get = new HttpGet(uri);
                    Log.i(TAG, "calling httpclient.execute(put, mHttpResponseHandler)");
                    httpclient.execute(get, mHttpResponseHandler);

                    Log.i(TAG, "completed httpclient.execute(put, mHttpResponseHandler)");
                    //meshUrlBaseString = null;
                    get = null;
                } catch (URISyntaxException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                } catch (Exception e) {
                    Log.i(TAG, "catching exception in general");
                    e.printStackTrace();
                }
            }
        }.start();
        Log.e(TAG, "end of send");
    }

    Handler mHandler = new Handler(){
        @Override
        public void handleMessage(Message msg){
            if(msg.what == EVENT_INIT) {
                AWSinit();
            } else if(msg.what == EVENT_CONNECT) {
                try {

                    mqttManager.connect(clientKeyStore, new AWSIotMqttClientStatusCallback() {
                        @Override
                        public void onStatusChanged(final AWSIotMqttClientStatus status,
                                                    final Throwable throwable) {
                            Log.d(TAG, "Status = " + String.valueOf(status));

                            if(status == AWSIotMqttClientStatus.Connected)
                            {
                                Log.d(TAG, "connected! subscribing");

                                AWSSubscribe(SUBSCRIBE_CONN_TOPIC);

                                AWSSubscribe(SUBSCRIBE_DATA_TOPIC);

                                AWSPublish(CONNECT, PUBLISH_CONN_TOPIC);
                            }

                        }
                    });
                } catch (final Exception e) {
                    Log.e(TAG, "Connection error.", e);
                }
            }
        }
    };

    public byte[] parseHexBinary(String s) {
        final int len = s.length();

        // "111" is not a valid hex encoding.
        if( len%2 != 0 )
            throw new IllegalArgumentException("hexBinary needs to be even-length: "+s);

        byte[] out = new byte[len/2];

        for( int i=0; i<len; i+=2 ) {
            int h = hexToBin(s.charAt(i  ));
            int l = hexToBin(s.charAt(i+1));
            if( h==-1 || l==-1 )
                throw new IllegalArgumentException("contains illegal character for hexBinary: "+s);

            out[i/2] = (byte)(h*16+l);
        }

        return out;
    }

    private static int hexToBin( char ch ) {
        if( '0'<=ch && ch<='9' )    return ch-'0';
        if( 'A'<=ch && ch<='F' )    return ch-'A'+10;
        if( 'a'<=ch && ch<='f' )    return ch-'a'+10;
        return -1;
    }

    void initIoTClient() {
        Log.d(TAG, "initIoTClient");
        // read the customer specific endpoint from AWS.conf file
        try {
            Properties props = readProperties("/sdcard/AWS.conf");
            if(props != null) {
                CUSTOMER_SPECIFIC_ENDPOINT = props.getProperty("CustomerSpecificEndPoint");
//                Log.d(TAG, "## customer specific end point = " +CUSTOMER_SPECIFIC_ENDPOINT);
            } else {
                Log.d(TAG, "Error reading AWS conf");
            }
        } catch (Exception e) {
            Log.d(TAG,"Cannot read AWS Credentials");
            e.printStackTrace();
        }
        // MQTT Client
        mqttManager = new AWSIotMqttManager(clientId, CUSTOMER_SPECIFIC_ENDPOINT);

        // Set keepalive to 10 seconds.  Will recognize disconnects more quickly but will also send
        // MQTT pings every 10 seconds.
        mqttManager.setKeepAlive(10);

        // Set Last Will and Testament for MQTT.  On an unclean disconnect (loss of connection)
        // AWS IoT will publish this message to alert other clients.
        AWSIotMqttLastWillAndTestament lwt = new AWSIotMqttLastWillAndTestament("my/lwt/topic",
                "Android client lost connection", AWSIotMqttQos.QOS0);
        mqttManager.setMqttLastWillAndTestament(lwt);

        // IoT Client (for creation of certificate if needed)
        mIotAndroidClient = new AWSIotClient(credentialsProvider);
        mIotAndroidClient.setRegion(region);

        keystorePath = mCtx.getFilesDir().getPath();
        keystoreName = KEYSTORE_NAME;
        keystorePassword = KEYSTORE_PASSWORD;
        certificateId = CERTIFICATE_ID;

        // To load cert/key from keystore on filesystem
        try {
            if (AWSIotKeystoreHelper.isKeystorePresent(keystorePath, keystoreName)) {
                if (AWSIotKeystoreHelper.keystoreContainsAlias(certificateId, keystorePath,
                        keystoreName, keystorePassword)) {
                    Log.i(TAG, "Certificate " + certificateId
                            + " found in keystore - using for MQTT.");
                    // load keystore from file into memory to pass on connection
                    clientKeyStore = AWSIotKeystoreHelper.getIotKeystore(certificateId,
                            keystorePath, keystoreName, keystorePassword);

                } else {
                    Log.i(TAG, "Key/cert " + certificateId + " not found in keystore.");
                }
            } else {
                Log.i(TAG, "Keystore " + keystorePath + "/" + keystoreName + " not found.");
            }
        } catch (Exception e) {
            Log.e(TAG, "An error occurred retrieving cert/key from keystore.", e);
        }

        if (clientKeyStore == null) {
            Log.i(TAG, "Cert/key was not found in keystore - creating new key and certificate.");

            new Thread(new Runnable() {
                @Override
                public void run() {
                    try {
                        // Create a new private key and certificate. This call
                        // creates both on the server and returns them to the
                        // device.
                        CreateKeysAndCertificateRequest createKeysAndCertificateRequest =
                                new CreateKeysAndCertificateRequest();
                        createKeysAndCertificateRequest.setSetAsActive(true);
                        final CreateKeysAndCertificateResult createKeysAndCertificateResult;
                        createKeysAndCertificateResult =
                                mIotAndroidClient.createKeysAndCertificate(createKeysAndCertificateRequest);
                        Log.i(TAG,
                                "Cert ID: " +
                                        createKeysAndCertificateResult.getCertificateId() +
                                        " created.");

                        // store in keystore for use in MQTT client
                        // saved as alias "default" so a new certificate isn't
                        // generated each run of this application
                        AWSIotKeystoreHelper.saveCertificateAndPrivateKey(certificateId,
                                createKeysAndCertificateResult.getCertificatePem(),
                                createKeysAndCertificateResult.getKeyPair().getPrivateKey(),
                                keystorePath, keystoreName, keystorePassword);

                        // load keystore from file into memory to pass on
                        // connection
                        clientKeyStore = AWSIotKeystoreHelper.getIotKeystore(certificateId,
                                keystorePath, keystoreName, keystorePassword);

                        // Attach a policy to the newly created certificate.
                        // This flow assumes the policy was already created in
                        // AWS IoT and we are now just attaching it to the
                        // certificate.
                        AttachPrincipalPolicyRequest policyAttachRequest =
                                new AttachPrincipalPolicyRequest();
                        policyAttachRequest.setPolicyName(AWS_IOT_POLICY_NAME);
                        policyAttachRequest.setPrincipal(createKeysAndCertificateResult
                                .getCertificateArn());
                        mIotAndroidClient.attachPrincipalPolicy(policyAttachRequest);
                        mHandler.sendEmptyMessage(EVENT_CONNECT);
                    } catch (Exception e) {
                        Log.e(TAG,
                                "Exception occurred when generating new private key and certificate.",
                                e);
                    }
                }
            }).start();
        } else {
            mHandler.sendEmptyMessage(EVENT_CONNECT);
        }
    }
}
