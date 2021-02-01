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

import android.app.AlertDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import android.text.TextUtils;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.AbsListView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.cypress.le.mesh.meshapp.Adapter.NodeAdapter;
import com.cypress.le.mesh.meshapp.LightingService.IServiceCallback;
import com.cypress.le.mesh.meshframework.IMeshControllerCallback;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.UUID;

public class ActivityModel extends AppCompatActivity implements IServiceCallback{
    private static final String TAG = "ActivityModel";

    LightingService serviceReference = null;

    int mId = -1;
    int mSeek =0;
    String mGroupName = null;
    int time = 0;
    String name;
    String type = null;
    ActionBar toolbar;
    ExpandableHeightListView lv;
    ImageButton delete;
    ImageButton deleteFromGroup;
    ImageButton componentInfo;

    ImageButton move;
    ImageButton addToGroup;
    ImageButton status;
    ImageButton mBtnOta;
    TextView compInfoTxt;
    TextView statusTxt;

    String mFileName = null;
    byte mDfuMethod = 0;
    static boolean isUpgradeInProgress = false;
    private static Toast mToast = null;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_model);

        Intent intent = new Intent(ActivityModel.this, LightingService.class);
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);

        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            Log.d(TAG, "Extras is not null");
            type = extras.getString("groupType");
            name = extras.getString("Name");
            // mId = extras.getInt("mId");
            mGroupName = extras.getString("groupName");
            mSeek = extras.getInt("seek");
            Log.d(TAG, "type =" + type + " name=" + name);
        } else {
            Log.d(TAG, "Extras is null");
        }

        toolbar = getSupportActionBar();
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setTitle(name);
        lv = (ExpandableHeightListView) findViewById(R.id.list);
        delete = (ImageButton) findViewById(R.id.imageButton3);
        mBtnOta = (ImageButton) findViewById(R.id.activity_model_btn_ota_upgrade);

        move = (ImageButton) findViewById(R.id.movebtn);
        addToGroup = (ImageButton) findViewById(R.id.addToGroup);
        deleteFromGroup = (ImageButton) findViewById(R.id.deleteFromGroup);
        componentInfo = (ImageButton) findViewById(R.id.version);
        compInfoTxt = (TextView) findViewById(R.id.componentInfotxt);
        statusTxt = (TextView) findViewById(R.id.status);

        Button clear = (Button) findViewById(R.id.clear);

        clear.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                statusTxt.setText("");
            }
        });

        statusTxt.setMovementMethod(new ScrollingMovementMethod());

        delete.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "Delete !");
                AlertDialog alertDialog = new AlertDialog.Builder(ActivityModel.this).create();
                alertDialog.setTitle("Delete !");
                alertDialog.setMessage("\nDo you want to delete the device ? The device will be reset to default");
                alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "OK",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                dialog.dismiss();
                                serviceReference.getMesh().resetDevice(name);
                            }
                        });
                alertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "CANCEL",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                dialog.dismiss();
                            }
                        });
                alertDialog.show();

            }
        });

        deleteFromGroup.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "Delete !");
                AlertDialog alertDialog = new AlertDialog.Builder(ActivityModel.this).create();
                alertDialog.setTitle("Delete !");
                alertDialog.setMessage("\nDo you want to delete the device from the group?");
                alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "OK",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                dialog.dismiss();
                                serviceReference.getMesh().removeComponentFromGroup(name, mGroupName);
                            }
                        });
                alertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "CANCEL",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                dialog.dismiss();
                            }
                        });
                alertDialog.show();
            }
        });

        mBtnOta.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.d(TAG, "OTA");
                Intent intent;
                Bundle bundle = new Bundle();
                bundle.putString("name", name);
                intent = new Intent(ActivityModel.this, ActivityOta.class);
                intent.putExtras(bundle);

                startActivity(intent);
            }
        });

        componentInfo.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG,"componentInfo");
                AlertDialog alertDialog = new AlertDialog.Builder(ActivityModel.this).create();
                alertDialog.setTitle("Component Information");
                alertDialog.setMessage("\nGet Component information of the device ?");
                alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "OK",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                serviceReference.getMesh().getComponentInfo(name);
                                dialog.dismiss();
                            }
                        });
                alertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "CANCEL",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                dialog.dismiss();
                            }
                        });
                alertDialog.show();

            }
        });


        move.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final ArrayList<String>rooms = (ArrayList<String>) serviceReference.getallRooms();

                String [] grpList= serviceReference.getMesh().getComponentGroupList(name);
                Log.d(TAG, "grpList:"+Arrays.asList(grpList));
                Log.d(TAG, "Rooms:"+rooms);
                ArrayList<String> groupsfinal = new ArrayList<String>();
                groupsfinal = rooms;
                if(rooms.size() > 0) {
                    for(int i=0;i<grpList.length;i++) {

                        if(rooms.contains(grpList[i])) {
                            groupsfinal.remove(grpList[i]);
                        }
                    }
                }

                View moveView = View.inflate(ActivityModel.this, R.layout.pop_up_movecomponent, null);
                popUpMoveComponentToGroup(moveView, groupsfinal);
            }
        });
        addToGroup.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final ArrayList<String>rooms = (ArrayList<String>) serviceReference.getallRooms();

                String [] grpList= serviceReference.getMesh().getComponentGroupList(name);
                Log.d(TAG, "grpList:"+Arrays.asList(grpList));
                Log.d(TAG, "Rooms:"+rooms);
                ArrayList<String> groupsfinal = new ArrayList<String>();
                groupsfinal = rooms;
                if(rooms.size() > 0) {
                    for(int i=0;i<grpList.length;i++) {

                        if(rooms.contains(grpList[i])) {
                            groupsfinal.remove(grpList[i]);
                        }
                    }
                }

       //         serviceReference.getMesh().addComponentToGroup(name, rooms.get(0));
                View moveView = View.inflate(ActivityModel.this, R.layout.pop_up_add_to_group, null);
                addDeviceToGroup(moveView, groupsfinal);
            }
        });
    }

    @Override
    protected void onResume() {
        Log.d(TAG, "onResume");
        super.onResume();

        if(serviceReference != null) {
            serviceReference.registerCb(ActivityModel.this);
        }

    }

    public void show(final String text,final int duration) {
        runOnUiThread(new Runnable() {

            public void run() {
                if (mToast == null || !mToast.getView().isShown()) {
                    if (mToast != null) {
                        mToast.cancel();
                    }
                }
                // if (mToast != null) mToast.cancel();
                mToast = Toast.makeText(getApplicationContext(), text, duration);
                mToast.show();
            }
        });

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_light, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        switch (item.getItemId()) {
            case android.R.id.home:
                Log.d(TAG, "home" + item.getItemId());
                onBackPressed();
                return true;
            case R.id.edit:
                View editView = View.inflate(ActivityModel.this, R.layout.pop_up_edit_name, null);
                popUpEdit(editView);
            case R.id.info:
                String[] groups = serviceReference.getMesh().getComponentGroupList(name);
                final String groupArray = TextUtils.join(", ",groups);
                AlertDialog alertDialog = new AlertDialog.Builder(ActivityModel.this).create();
                alertDialog.setTitle("Component Info");
                alertDialog.setMessage("\nThis component belongs to:\n\n "+groupArray);
                alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "OK",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                dialog.dismiss();
                            }
                        });
                alertDialog.show();
            default:
                return super.onOptionsItemSelected(item);
        }
    }
    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        try {
            return super.dispatchTouchEvent(ev);
        } catch (Exception e) {
            return false;
        }
    }

    @Override
    protected void onDestroy() {
        unbindService(mConnection);
        super.onDestroy();
    }

    private ServiceConnection mConnection= new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName compname, IBinder service) {
            Log.d(TAG, "bound service connected");
            LightingService.MyBinder binder = (LightingService.MyBinder) service;
            serviceReference = binder.getService();
            serviceReference.registerCb(ActivityModel.this);

            int componentType = serviceReference.getMesh().getComponentType(name);

            ArrayList<String> targetMethodTypes = new ArrayList<String>(Arrays.asList(serviceReference.getMesh().getTargetMethods(name)));
            Log.d(TAG,"TargetMethods "+ targetMethodTypes);
            ArrayList<String> controlMethodTypes = new ArrayList<String>(Arrays.asList(serviceReference.getMesh().getControlMethods(name)));
            Log.d(TAG,"controlMethods "+controlMethodTypes);

            NodeAdapter adapter = new NodeAdapter(serviceReference,
                    ActivityModel.this,
                    name,
                    componentType,
                    targetMethodTypes,
                    controlMethodTypes,
                    mGroupName);

            synchronized(lv){
                lv.setAdapter(adapter);
                lv.notify();
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            serviceReference = null;
        }
    };

    void popUpEdit(View editView) {
        AlertDialog.Builder builder = new AlertDialog.Builder(ActivityModel.this, R.style.AlertDialogCustom);
        final EditText text = (EditText) editView.findViewById(R.id.editText);
        builder.setView(editView);

        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                Log.d(TAG, "User clicked OK button");
                getSupportActionBar().setTitle(text.getText());
                toolbar.setTitle(text.getText());
                serviceReference.getMesh().rename(name, text.getText().toString());
                dialog.dismiss();
            }
        });
        builder.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                Log.d(TAG, "User cancelled the dialog");
            }
        });
        AlertDialog alert = builder.create();
        alert.show();
        alert.getWindow().setLayout(android.app.ActionBar.LayoutParams.WRAP_CONTENT, android.app.ActionBar.LayoutParams.WRAP_CONTENT);
    }

    @Override
    public void onMeshServiceStatusChangeCb(int status) {

    }

    @Override
    public void onDeviceFound(UUID uuid, String name) {

    }

    @Override
    public void onProvisionComplete(UUID device, byte status) {

    }

    @Override
    public void onHslStateChanged(final String deviceName, final int lightness, final int hue, final int saturation, final int remainingTime) {
        final int lightnessVal, hueVal, saturationVal;
        lightnessVal = Math.round((100 * lightness) / (float)65535);
        hueVal = Math.round((360 * hue) / (float)65535);
        saturationVal = Math.round((100 * saturation) / (float)65535);

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                statusTxt.append("\n"+"HSL Status hue:"+hueVal+" lightness:"+lightnessVal+" sat:"+saturationVal);
            }
        });
    }

    @Override
    public void onOnOffStateChanged(final String deviceName, final byte targetOnOff, final byte presentOnOff, final int remainingTime) {
        show("onOnOffStateChanged : "+ targetOnOff, Toast.LENGTH_SHORT);
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                statusTxt.append("\n"+"On Off Status : "+ targetOnOff);
            }
        });
    }

    @Override
    public void onLevelStateChanged(final String deviceName, final short targetLevel, final short presentLevel, final int remainingTime) {
        show("onLevelStateChanged : "+ targetLevel,
                Toast.LENGTH_SHORT);
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                statusTxt.append("\n"+"Level Status : "+ targetLevel);
            }
        });
    }

    @Override
    public void onNetworkConnectionStatusChanged(final byte transport, final byte status) {
        Log.d(TAG,"recieved onNetworkConnectionStatusChanged status = " +status);
        String text = null;
        if(status == IMeshControllerCallback.NETWORK_CONNECTION_STATE_CONNECTED)
            text = "Connected to network";
        if(status == IMeshControllerCallback.NETWORK_CONNECTION_STATE_DISCONNECTED)
            text = "Disconnected from network";
        if(text != null)
            show(text, Toast.LENGTH_SHORT);
    }

    @Override
    public void onCtlStateChanged(String deviceName, int presentLightness, final short presentTemperature, int targetLightness, short targetTemperature, int remainingTime) {
        final int lightness = Math.round((100 * presentLightness) / (float)65535);
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                statusTxt.append("\n"+"CTL Status lightness:"+lightness+" temp:"+presentTemperature);
            }
        });
    }

    @Override
    public void onNodeConnStateChanged(final byte status, final String componentName) {
        Log.d(TAG,"onNodeConnStateChanged in Model UI");
    }

    @Override
    public void onOtaStatus(byte status, int percentComplete) {

    }


    @Override
    public void onNetworkOpenedCallback(byte status) {

    }

    @Override
    public void onComponentInfoStatus(byte status, String componentName, final String componentInfoStr) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                compInfoTxt.setText(componentInfoStr);
            }
        });
    }

    @Override
    public void onDfuStatus(byte status, byte[] data) {

    }

    @Override
    public void onSensorStatusCb(final String componentName, final int propertyId, final byte[] data) {
        Log.d(TAG,"onSensorStatusCb "+componentName+" Sensor Status : "+(double)getSensorValue(propertyId, data));

        if(!componentName.equals(name))
            return;

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                statusTxt.append("\n"+"Sensor Status : "+(double)getSensorValue(propertyId, data));
            }
        });
    }

    @Override
    public void onVendorStatusCb(short componentName, short companyId, short modelId, byte opcode, byte ttl, final byte[] data, short dataLen) {
        Log.d(TAG,"onVendorStatusCb "+componentName);
       runOnUiThread(new Runnable() {
            @Override
            public void run() {
                statusTxt.append("\n"+"Vendor Status : "+toHexString(data));
            }
        });
    }
    @Override
    public void onLightnessStateChanged(String deviceName, int target, final int present, int remainingTime) {
        final int lightness = Math.round((100 * present) / (float)65535);
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                statusTxt.append("\n"+"Lightness Status : "+lightness);
            }
        });
    }

    @Override
    public void onLightLcModeStatus(String componentName, final int mode) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                statusTxt.append("\n"+"LC mode Status : "+mode);
            }
        });
    }

    @Override
    public void onLightLcOccupancyModeStatus(String componentName, final int mode) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                statusTxt.append("\n"+"LC occupancy mode Status : "+mode);
            }
        });
    }

    @Override
    public void onLightLcPropertyStatus(String componentName, final int propertyId, final int value) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                statusTxt.append("\n"+"LC property Status prop:"+propertyId+ " val:"+value);
            }
        });
    }

    void popUpMoveComponentToGroup(View groupView, ArrayList<String> rooms) {

        final ArrayAdapter<String> itemsAdapter = new ArrayAdapter<String>(ActivityModel.this, R.layout.node_list_item, rooms);

        final ListView listview =  (ListView)groupView.findViewById(R.id.grouplist);
        listview.setAdapter(itemsAdapter);
        listview.setChoiceMode(AbsListView.CHOICE_MODE_SINGLE);

        if(listview.getCount() > 0)
            listview.setItemChecked(0, true);

        android.app.AlertDialog.Builder builder = new android.app.AlertDialog.Builder(ActivityModel.this, android.app.AlertDialog.THEME_HOLO_LIGHT);
        builder.setView(groupView);

        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {

                Log.d(TAG, "User clicked OK button" + serviceReference.getCurrentGroup());

                int itemPos = listview.getCheckedItemPosition();
                String selectedDevice = (String)listview.getAdapter().getItem(itemPos);
                if (selectedDevice == null) {
                    return;
                }
                final String groupName = selectedDevice;
                Log.d(TAG, "Selected Light" + name );

                Thread t = new Thread(new Runnable() {
                    public void run() {

                        serviceReference.getMesh().moveComponentToGroup(name, mGroupName, groupName);
                    }
                });
                t.start();

                dialog.dismiss();
            }
        });
        builder.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                Log.d(TAG, "User cancelled the dialog");
            }
        });
        android.app.AlertDialog alert = builder.create();
        alert.show();
        alert.getWindow().setLayout(ActionBar.LayoutParams.WRAP_CONTENT, ActionBar.LayoutParams.WRAP_CONTENT);
    }

    void addDeviceToGroup(View groupView, ArrayList<String> groups) {

        final ArrayAdapter<String> itemsAdapter = new ArrayAdapter<String>(ActivityModel.this, R.layout.node_list_item, groups);

        final ListView listview =  (ListView)groupView.findViewById(R.id.grouplist1);
        listview.setAdapter(itemsAdapter);
        listview.setChoiceMode(AbsListView.CHOICE_MODE_SINGLE);

        if(listview.getCount() > 0)
            listview.setItemChecked(0, true);

        android.app.AlertDialog.Builder builder = new android.app.AlertDialog.Builder(ActivityModel.this, android.app.AlertDialog.THEME_HOLO_LIGHT);
        builder.setView(groupView);


        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {

                Log.d(TAG, "User clicked OK button");
                int itemPos = listview.getCheckedItemPosition();
                String selectedDevice = (String)listview.getAdapter().getItem(itemPos);
                if (selectedDevice == null) {
                    return;
                }
                final String groupName = selectedDevice;
                Log.d(TAG, "Selected Light" + name );
                int res = serviceReference.getMesh().addComponentToGroup(name, groupName);
                Log.d(TAG, "result" + res );
                dialog.dismiss();
            }
        });
        builder.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                Log.d(TAG, "User cancelled the dialog");
            }
        });
        android.app.AlertDialog alert = builder.create();
        alert.show();
        alert.getWindow().setLayout(ActionBar.LayoutParams.WRAP_CONTENT, ActionBar.LayoutParams.WRAP_CONTENT);
    }


    double getSensorValue(int propertyId, byte[] data)
    {
        double res = 0;
        switch (propertyId)
        {
            case Constants.MESH_PROPERTY_PRESENCE_DETECTED :
                // Property PRESENCE_DETECTED uses the Boolean(uint8) characteristic.
                // See https://www.bluetooth.com/specifications/mesh-specifications/mesh-viewer?xmlFile=org.bluetooth.characteristic.boolean
                res = data[0];
                break;
            case Constants.MESH_PROPERTY_AMBIENT_LUX_LEVEL_ON:
                // Property AMBIENT_LUX_LEVEL_ON uses the Illuminance(uint24) characteristic.
                // Unit is lux with a resolution of 0.01. Range from 0 to 167772.14.
                // See https://www.bluetooth.com/specifications/mesh-specifications/mesh-viewer?xmlFile=org.bluetooth.characteristic.illuminance
                res = ((double)(((data[2] & 0xF) << 16) | ((data[1] & 0xFF) << 8) | (data[0] & 0xFF)) / (double)100.0);
                break;
            case Constants.MESH_PROPERTY_PRESENT_AMBIENT_LIGHT_LEVEL :
                // Property PRESENT_AMBIENT_LIGHT_LEVEL uses the Illuminance(uint24) characteristic.
                res = ((double)(((data[2] & 0xF) << 16) | ((data[1] & 0xFF) << 8) | (data[0] & 0xFF)) / (double)100.0);
                break;
            case Constants.MESH_PROPERTY_PRESENT_AMBIENT_TEMPERATURE :
                // Property PRESENT_AMBIENT_TEMPERATURE uses the Temperature8(sint8) characteristic.
                // Unit is degree Celsius with a resolution of 0.5. Range from -64 to 63.5.
                // See https://www.bluetooth.com/specifications/mesh-specifications/mesh-viewer?xmlFile=org.bluetooth.characteristic.temperature_8
                // Change the condition for negative value
                float val = (data[0]%2 > 0)? (float) 0.5 :0;
                res =(float)(data[0]/2 + val);
                break;
            case Constants.MESH_PROPERTY_MOTION_SENSED:
                // Property MOTION_SENSED uses the Percentage8(uint8) characteristic.
                // Unit is a percentage with a resolution of 0.5. Range from 0 to 100.
                // See https://www.bluetooth.com/specifications/mesh-specifications/mesh-viewer?xmlFile=org.bluetooth.characteristic.percentage_8
                val = (data[0]%2 > 0)? (float) 0.5 :0;
                res =(float)(data[0]/2 + val);
                break;
            default:
                if(data.length == 4)
                    res = (((data[3] & 0xF) << 24) | ((data[2] & 0xF) << 16) | ((data[1] & 0xFF) << 8) | (data[0] & 0xFF));
                else if(data.length == 3)
                    res = (((data[2] & 0xF) << 16) | ((data[1] & 0xFF) << 8) | (data[0] & 0xFF));
                else if(data.length == 2)
                    res = (((data[1] & 0xFF) << 8) | (data[0] & 0xFF));
                break;
        }
        Log.d(TAG,"data : "+res);
        return res;
    }

    static String toHexString(byte[] bytes) {
        int len = bytes.length;
        if(len == 0)
            return null;

        char[] buffer = new char[len * 3 - 1];

        for (int i = 0, index = 0; i < len; i++) {
            if (i > 0) {
                buffer[index++] = ' ';
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


}
