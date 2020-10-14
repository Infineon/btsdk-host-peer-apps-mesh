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

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.FragmentManager;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.location.LocationManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.support.design.widget.CollapsingToolbarLayout;
import android.support.design.widget.FloatingActionButton;
import android.support.v4.graphics.ColorUtils;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.AbsListView;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.cypress.le.mesh.meshframework.IMeshControllerCallback;
import com.cypress.le.mesh.meshframework.MeshController;
import com.larswerkman.holocolorpicker.ColorPicker;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.Semaphore;

import com.cypress.le.mesh.meshapp.Adapter.GrpDeviceListAdapter;

public class ActivityGroup extends AppCompatActivity implements LightingService.IServiceCallback {
    private static final String TAG = "ActivityGroup";

    public static final int LIGHT_NOT_ADDED = 0;
    public static final int LIGHT_ADDED = 1;
    public static final int PROVISION_SPINNER_TIMEOUT = 45000;
    ImageView imageView;
    Drawable image = null;
    ColorPicker picker;
    Toolbar toolbar;
    TextView text;
    ImageButton imgbtn;
    View proxyConnView;
    Button groupOn;
    Button groupOff;
    String groupName;
    String name;
    String type = null;
    Uri tempUri;
    private File file;
    MeshBluetoothDevice mPeerDevice = null;
    private static Toast mToast = null;

    //    ArrayList<String> groups= new ArrayList<String>();
    ArrayList<String> components = new ArrayList<String>();
    ArrayList<Integer> componentType = new ArrayList<Integer>();
    GrpDeviceListAdapter adapterGrplist;
    private ArrayAdapter<MeshBluetoothDevice> peerDevices;
    private static MeshApp mApp;
    private String newNode;
    private GrpDeviceListAdapter adapter;
    private final int  RESULT_LOAD_IMG = 1;
    private final int PIC_CROP = 2;
    private int mLightAddedStatus = LIGHT_NOT_ADDED;
    ExpandableHeightListView lv;
    CollapsingToolbarLayout collapsingToolbarLayout;
    static LightingService serviceReference = null;
    TextView transtime = null;
    static int mColor = 0;
    int time = 0;
    // ProgressDialog progress;
    boolean isConfigComplete = true;
    boolean isStoppedScan = false;
    boolean startProvision = false;
    String mDeviceName = "DEVICE";
    static Semaphore semaphore = new Semaphore(1);
    Handler mSpinTimer = new Handler();

    @Override
    protected void onResume() {
        Log.d(TAG, "onResume");
        super.onResume();
        if(components.size()!= 0 ) {
            components = new ArrayList<String>(Arrays.asList(serviceReference.getMesh().getGroupComponents(groupName)));
            componentType = new ArrayList<Integer>();
            for(int i=0; i< components.size(); i++) {
                componentType.add(serviceReference.getMesh().getComponentType(components.get(i)));
            }

            adapterGrplist = new GrpDeviceListAdapter(serviceReference, ActivityGroup.this, components, componentType, null, "group", groupName);
            if(adapterGrplist!=null)
                adapterGrplist.notifyDataSetChanged();
            lv.setAdapter(adapterGrplist);

        }
        if(serviceReference != null) {
            serviceReference.registerCb(ActivityGroup.this);
            if (!serviceReference.getMesh().isConnectedToNetwork() && (Arrays.asList(serviceReference.getMesh().getGroupComponents(groupName)).size() != 0)){
                show("Connecting to mesh network!", Toast.LENGTH_LONG);
                serviceReference.getMesh().connectNetwork((byte)5);
            }
        }

        runOnUiThread(new Runnable() {

            public void run() {
                if(mApp.getMesh().isConnectedToNetwork()) {
                    if(mApp.getPreferredTransport()==Constants.TRANSPORT_GATEWAY)
                        proxyConnView.setBackgroundColor(getResources().getColor(R.color.blue));
                    else
                        proxyConnView.setBackgroundColor(getResources().getColor(R.color.green));
                } else {
                    proxyConnView.setBackgroundColor(getResources().getColor(R.color.red));
                }
            }
        });
    }


    @Override
    public void onPause() {
        Log.d(TAG, "onPause");
        super.onPause();
    }


    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        unregisterReceiver(mReceiver);
        unbindService(mConnection);
        super.onDestroy();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_group);
        Intent serviceIntent= new Intent(this, LightingService.class);
        bindService(serviceIntent, mConnection, Context.BIND_AUTO_CREATE);
        Intent intent = getIntent();

        if(intent!=null) {
            Bundle extras = getIntent().getExtras();
            if(extras != null) {
                type = extras.getString("groupType");
                name = extras.getString("name");
                groupName = extras.getString("GroupName");
                Log.d(TAG, "name =" + name + " mId=" + groupName + " type=" + type);
            } else {
                Log.d(TAG, "Extras are null");
            }
        } else {
            Log.d(TAG, "Intent is null");
        }

        mApp = (MeshApp) getApplication();
        toolbar = (Toolbar) findViewById(R.id.toolbar);

     //   FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        toolbar.setTitle(name);
        toolbar.inflateMenu(R.menu.menu_main);
        setSupportActionBar(toolbar);
        Button lightset = (Button) findViewById(R.id.lightSet);
      //  Button config = (Button) findViewById(R.id.config);
        Button connectToProxy = (Button) findViewById(R.id.connectToProxy);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        imageView = (ImageView)findViewById(R.id.bgheader);
        proxyConnView = (View)findViewById(R.id.proxy);
        newNode = null;
        collapsingToolbarLayout = (CollapsingToolbarLayout) findViewById(R.id.toolbar_layout);

        if(type.equals("room")) {
            if(image != null) {

                if(image != null) {
                    imageView.setBackground(image);
                }
            }
        } else {
           // fab.setVisibility(View.INVISIBLE);
            imageView.setBackgroundColor(getResources().getColor(R.color.primary));
            lightset.setVisibility(View.VISIBLE);

        }


//        fab.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                Intent galleryIntent = new Intent(Intent.ACTION_PICK,
//                        android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
//                startActivityForResult(galleryIntent, RESULT_LOAD_IMG);
//
//            }
//        });
        connectToProxy.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG,"Connect to Network");
                AlertDialog alertDialog = new AlertDialog.Builder(ActivityGroup.this).create();
                alertDialog.setTitle("Connect To Network");
                alertDialog.setMessage("\nConnect to Mesh Network ?");
                alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "OK",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                dialog.dismiss();
                                if(!isLocationServiceEnabled()) {
                                    Log.d(TAG, "isLocationServiceEnabled : location is false");
                                    show("Please turn on the location!!!", Toast.LENGTH_SHORT);
                                }
                                else {
                                    Log.d(TAG, "isLocationServiceEnabled : location is true");
                                    if(mApp.connectToNetwork(Constants.TRANSPORT_GATT) == MeshController.MESH_CLIENT_SUCCESS)
                                    {
                                        proxyConnView.setBackgroundColor(getResources().getColor(R.color.red));
                                        show("Connecting...", Toast.LENGTH_SHORT);
                                    }

                                }

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


        // Set an OnMenuItemClickListener to handle menu item clicks
        toolbar.setOnMenuItemClickListener(new Toolbar.OnMenuItemClickListener() {
            @Override
            public boolean onMenuItemClick(MenuItem item) {
                // Handle the menu item
                Log.d(TAG, "home" + item.getItemId());
                switch (item.getItemId()) {
                    case R.id.edit:
                        Log.d(TAG, "edit");
                        View editView = View.inflate(ActivityGroup.this, R.layout.pop_up_edit_name, null);
                        popUpEdit(editView);
                        break;
                    case R.id.delete:
                        if (mApp.getPreferredTransport() == Constants.TRANSPORT_GATT) {
                            View deleteView = View.inflate(ActivityGroup.this, R.layout.pop_up_delete, null);
                            popUpDelete(deleteView);
                        } else {
                            show("Feature not supported in away mode", Toast.LENGTH_SHORT);
                        }
                        break;
                    case R.id.refresh:
                        updateDisplay();
                        break;
                    case R.id.disconnect:
                        AlertDialog alertDialog = new AlertDialog.Builder(ActivityGroup.this).create();
                        alertDialog.setTitle("Disconnect Network");
                        alertDialog.setMessage("\nDo you want to disconnect current Network ?");
                        alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "OK",
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int which) {
                                        dialog.dismiss();
                                        if (serviceReference.getMesh().isConnectedToNetwork()) {
                                            serviceReference.getMesh().disconnectNetwork();
                                        } else {
                                            show("Network is not connected!!!", Toast.LENGTH_SHORT);
                                        }
                                    }
                                });
                        alertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "CANCEL",
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int which) {
                                        dialog.dismiss();
                                    }
                                });
                        alertDialog.show();

                        break;
                }
                return true;
            }
        });
        findViewById(R.id.lightSet).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(type.equals("room")) {
                    View groupView = View.inflate(ActivityGroup.this, R.layout.pop_up_new_group, null);
                    //popUpNewGroup(groupView);
                }
            }
        });
        final ImageButton imgbtn = (ImageButton) findViewById(R.id.seekBar5);

        if(imgbtn != null) {
            imgbtn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    View lightView = View.inflate(ActivityGroup.this,R.layout.pop_up_setting_color, null);
                    popUpLight(lightView);
                }
            });
        }

        //Your toolbar is now an action bar and you can use it like you always do, for example:

        lv = (ExpandableHeightListView)findViewById(R.id.list);
        findViewById(R.id.button3).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "showScanDevicesDialog");
                if (mApp.getPreferredTransport() == Constants.TRANSPORT_GATT) {
                    View addDeviceView = View.inflate(ActivityGroup.this, R.layout.pop_up_add_device, null);
                    showScanDevicesDialog();
                } else {
                    show("Feature not supported in away mode", Toast.LENGTH_SHORT);
                }
            }
        });

        transtime = (TextView) findViewById(R.id.trans_time);
        transtime.setText("");

        transtime.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                View transtimeView = View.inflate(ActivityGroup.this, R.layout.pop_up_edit_trans_time, null);
                popUpTransTime(transtimeView);
            }
        });

        groupOn =(Button) findViewById(R.id.onbtn);
        groupOff =(Button) findViewById(R.id.offbtn);

        groupOn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!transtime.getText().toString().matches(""))
                    time = Integer.parseInt(transtime.getText().toString());
                else
                    time = Constants.DEFAULT_TRANSITION_TIME;
                serviceReference.getMesh().onoffSet(groupName, true, false,time, (short) 0);

            }
        });

        groupOff.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!transtime.getText().toString().matches(""))
                    time = Integer.parseInt(transtime.getText().toString());
                else
                    time = Constants.DEFAULT_TRANSITION_TIME;
                serviceReference.getMesh().onoffSet(groupName, false, false,time, (short) 0);

            }
        });

/*
        config.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
//                isConfigComplete = false;
//                if(newNode!=null){
//                    Log.d(TAG,"calling configure from UI");
//                    configure(newNode, groupName);
//                }

            }
        });
        */

/*
        brightness = (SeekBar)findViewById(R.id.seekBar6);
        brightness.setMax(255);
        brightness.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                Log.d(TAG, "onProgressChanged" + seekBar.getProgress());
                byte val =(byte) ( seekBar.getProgress()&0xFF);
                serviceReference.OnGroupBrightinessChange(groupId, val, 0);
                adapterGrplist.notifyDataSetChanged();
            }
        });
*/
        IntentFilter filter = new IntentFilter();
        filter.addAction(Constants.GATT_PROXY_CONNECTED);
        filter.addAction(Constants.GATT_PROXY_DISCONNECTED);
        filter.addAction(Constants.GATT_PROXY_CONNECTED_FOR_CONFIG);
        ActivityGroup.this.registerReceiver(mReceiver, filter);

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

    void popUpAddDevice(final View addDeviceView) {
        final ListView listView = (ListView)addDeviceView.findViewById(R.id.listView6);

        AlertDialog.Builder builder = new AlertDialog.Builder(ActivityGroup.this, R.style.AlertDialogCustom);
        builder.setView(addDeviceView);
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Log.d(TAG, "User clicked device : " + id);
                String light = components.get(position);
                //LightingService.identify(light);
            }
        });
        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {

                Log.d(TAG, "User clicked OK button");
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
        alert.getWindow().setLayout(ActionBar.LayoutParams.WRAP_CONTENT, ActionBar.LayoutParams.WRAP_CONTENT);
    }

    void popUpLight(View lightView) {
        HSVColorPickerDialog cpd = new HSVColorPickerDialog( ActivityGroup.this, 0xFF4488CC, new HSVColorPickerDialog.HSVDailogChangeListener() {
            @Override
            public void onColorSelected(Integer color) {
                Log.d(TAG, "onColorSelected");
            }

            @Override
            public void onColorChanged(Integer color) {
                Log.d(TAG, "onColorChanged");
                mColor = color;
                setGroupColor(mColor);
            }

            @Override
            public void onStopColorChanged() {
                Log.d(TAG, "onStopColorChanged");
                serviceReference.setHSLStopTracking();
            }
            @Override
            public void onStartColorChanged() {
                Log.d(TAG, "onStartColorChanged");
                serviceReference.setHSLStartTracking();
            }
        });

        cpd.setTitle( "Pick a color" );
        cpd.show();
    }

    /*
        void popUpNewGroup(View groupView) {

            //groups = serviceReference.getTopLevelGroups(serviceReference.getCurrentGroup());
            components = serviceReference.getTopLevelLights(groupId);
           // final SelectListAdapter adapter = new SelectListAdapter(ActivityGroup.this,components,groups);

            ListView listview =  (ListView)groupView.findViewById(R.id.listView2);
            listview.setAdapter(adapter);
            AlertDialog.Builder builder = new AlertDialog.Builder(ActivityGroup.this, R.style.AlertDialogCustom);
            builder.setView(groupView);

            ImageButton editButton = (ImageButton) groupView.findViewById(R.id.imageButton2);
            editButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {

                }
            });

            builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int id) {
                    boolean result = LightingService.addSubGroup(serviceReference.getCurrentGroup(), "NewGroup");
                    if (result)
                        Log.d(TAG, "added group successfully" + name);
                    else
                        Log.d(TAG, "added unsuccessful" + name);

                    Log.d(TAG, "User clicked OK button" + serviceReference.getCurrentGroup());
                    HashMap<Integer, Boolean> chkbox = adapter.getCheckboxlist();
                    if(groups != null)
                        for (int i = 0; i < groups.size(); i++) {
                            if (chkbox.get(i) == true) {
                                String name = groups.get(i).getName();
                            }
                        }
                    if(components != null)
                        for (int i = 0; i < components.size(); i++) {
                            if(groups != null) {
                                if (chkbox.get(groups.size() + i) == true) {
                                    String name = components.get(i).getName();
                                    result = serviceReference.addLight(serviceReference.getCurrentGroup(), components.get(i), type);
                                    if (result)
                                        Log.d(TAG, "added light successfully" + name);
                                    else
                                        Log.d(TAG, "added unsuccessful" + name);
                                }
                            } else {
                                if (chkbox.get(i) == true) {
                                    String name = components.get(i).getName();
                                    result = serviceReference.addLight(serviceReference.getCurrentGroup(), components.get(i), type);
                                    if (result)
                                        Log.d(TAG, "added light successfully" + name);
                                    else
                                        Log.d(TAG, "added unsuccessful" + name);
                                }
                            }
                        }
                    updateDisplay();
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
            alert.getWindow().setLayout(ActionBar.LayoutParams.WRAP_CONTENT, ActionBar.LayoutParams.WRAP_CONTENT);

        }

    */
    void popUpEdit(View editView) {
        AlertDialog.Builder builder = new AlertDialog.Builder(ActivityGroup.this, R.style.AlertDialogCustom);
        final EditText text = (EditText) editView.findViewById(R.id.editText);
        builder.setView(editView);

        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                Log.d(TAG, "User clicked OK button");
                getSupportActionBar().setTitle(text.getText());
                if (!isNameAlreadyUsed(text.getText().toString())) {
                    collapsingToolbarLayout.setTitle(text.getText());
                    serviceReference.getMesh().rename(groupName, text.getText().toString());
                } else {
                    show("RoomName is already in use",
                            Toast.LENGTH_SHORT);
                }
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
        alert.getWindow().setLayout(ActionBar.LayoutParams.WRAP_CONTENT, ActionBar.LayoutParams.WRAP_CONTENT);

    }

    /*
    void popUpEditNewSubgroupName(View editView) {
        AlertDialog.Builder builder = new AlertDialog.Builder(ActivityGroup.this, R.style.AlertDialogCustom);
        final EditText text = (EditText) editView.findViewById(R.id.editText);
        builder.setView(editView);

        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                Log.d(TAG, "User clicked OK button");
                getSupportActionBar().setTitle(text.getText());
                LightingService.setGroupName(groupId, text.getText().toString());
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
        alert.getWindow().setLayout(ActionBar.LayoutParams.WRAP_CONTENT, ActionBar.LayoutParams.WRAP_CONTENT);
    }
    */
    void popUpDelete(View deleteView) {
/*
        //groups = serviceReference.getTopLevelGroups(serviceReference.getCurrentGroup());
        components = serviceReference.getTopLevelLights(serviceReference.getCurrentGroup());
        final SelectListAdapter adapter = new SelectListAdapter(deleteView.getContext(),components,null);
        ListView listView = (ListView) deleteView.findViewById(R.id.listView10);
        listView.setAdapter(adapter);

        AlertDialog.Builder builder = new AlertDialog.Builder(ActivityGroup.this, R.style.AlertDialogCustom);
        builder.setView(deleteView);
        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                HashMap<Integer, Boolean> chkList = adapter.getCheckboxlist();

//                for (int i = 0; i < groups.size(); i++) {
//                    if (chkList.get(i) == true) {
//                        String name = groups.get(i).getName();
//                        Log.d(TAG, "selected group =" + name);
//                        boolean result = LightingService.deleteGroup(groups.get(i));
//                        if (result)
//                            Log.d(TAG, "Deleted group successfully" + name);
//                        else
//                            Log.d(TAG, "Delete unsuccessful" + name);
//                    }
//                }

                for (int i = 0; i < components.size(); i++) {
                    if (chkList.get(i) == true) {
                        String name = components.get(i).getName();
                        Log.d(TAG, "selected light =" + name);
                        boolean result = serviceReference.getMesh().resetDevice(components.get(i));
                        if (result) {
                            Log.d(TAG, "Deleted components successfully" + name);
                            Toast.makeText(ActivityGroup.this, "Deleted light successfully ", Toast.LENGTH_SHORT);
                        } else {
                            Log.d(TAG, "Deleted components unsuccessful" + name);
                            Toast.makeText(ActivityGroup.this, "Deleted light unsuccessful ", Toast.LENGTH_SHORT);
                        }
                    }
                }
                //groups = serviceReference.getTopLevelGroups(serviceReference.getCurrentGroup());
                components = serviceReference.getTopLevelLights(serviceReference.getCurrentGroup());
//                if(groups!= null)
//                    Log.d(TAG,"groups = "+groups);

                if(components!=null)
                    Log.d(TAG, "components = " + components);
                adapterGrplist = new GrpDeviceListAdapter(serviceReference, ActivityGroup.this, components, null, "group", groupId);

                if(adapterGrplist!=null)
                    adapterGrplist.notifyDataSetChanged();
                lv.setAdapter(adapterGrplist);
                Log.d(TAG, "User clicked OK button");
                dialog.dismiss();
            }
        });
        builder.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int idx) {
                Log.d(TAG, "User cancelled the dialog");
            }
        });
        AlertDialog alert = builder.create();
        alert.show();
        alert.getWindow().setLayout(ActionBar.LayoutParams.WRAP_CONTENT, ActionBar.LayoutParams.WRAP_CONTENT);
*/
    serviceReference.getMesh().deleteGroup(groupName);
    }

    void setGroupColor(int color) {
        float hsl[] = new float[3];
        ColorUtils.colorToHSL(color,hsl);
        final int hue = Math.round(((float)(65535 * (int)hsl[0]))/(float)360);
        final int saturation = Math.round(65535 * (hsl[1]));
        final int lightness = Math.round(65535 * (hsl[2]));

        Log.d(TAG, "sendHSLSet hue: "+hsl[0]+ " saturation"+hsl[1]*100+ " lightness"+hsl[2]*100);
        Thread t = new Thread(new Runnable() {
            public void run() {
                serviceReference.onHslValueChange(groupName, lightness, hue, saturation);
            }
        });
        t.start();
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
    public void onBackPressed(){
        FragmentManager fm = getFragmentManager();
        LightingService.popStack();
        if (fm.getBackStackEntryCount() > 0) {
            Log.i(TAG, "popping backstack");
            fm.popBackStack();
        } else {
            Log.i(TAG, "nothing on backstack, calling super");
            super.onBackPressed();
        }
    }
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);

        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int idx = item.getItemId();

        //noinspection SimplifiableIfStatement
        switch (item.getItemId()) {
            case android.R.id.home:
                Log.d(TAG, "home" + item.getItemId());
                onBackPressed();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    /*for image*/
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        try {
            Log.d(TAG, "onActivityResult: request code = " + requestCode);
            // When an Image is picked
            if (requestCode == RESULT_LOAD_IMG && resultCode == ActivityGroup.this.RESULT_OK
                    && null != data) {
                Uri selectedImage = data.getData();
                setImage(selectedImage);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void setImage(Uri selectedImage) {

        try {
            final InputStream imageStream = ActivityGroup.this.getContentResolver().openInputStream(selectedImage);
            final Bitmap selectedImage2 = BitmapFactory.decodeStream(imageStream);
            Drawable drawable = new BitmapDrawable(ActivityGroup.this.getResources(), selectedImage2);
            imageView.setBackground(drawable);
            image = drawable;
        } catch (FileNotFoundException e) {
            Log.e(TAG, "dint find image2");
            e.printStackTrace();
        }
    }


    @Override
    public void onMeshServiceStatusChangeCb(int status) {

    }

    @Override
    public void onDeviceFound(final UUID uuid, final String name) {
        Log.d(TAG, "onDeviceFound device:" + uuid.toString());

        //TODO: unique entries ??
        ActivityGroup.this.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if(peerDevices!=null) {
                    for (int i = 0; i < peerDevices.getCount(); i++) {
                        if (peerDevices.getItem(i).mUUID.equals(uuid)) {
                            //Discard the same device seen again
                            return;
                        }
                    }

                    peerDevices.add(new MeshBluetoothDevice(uuid, name));
                    peerDevices.notifyDataSetChanged();
                }
            }
        });
    }

    @Override
    public void onProvisionComplete(final UUID device, final byte status){
        Log.d(TAG, "onProvisionComplete remote node: status"+status);
        newNode = String.valueOf(status);

        //progress.setMessage("Provision complete for selected node");
        if(status == 5) {
            components = new ArrayList<String>(Arrays.asList(serviceReference.getMesh().getGroupComponents(groupName)));
            componentType.clear();
            for(int i =0; i<components.size(); i++)
            {
                componentType.add(serviceReference.getMesh().getComponentType(components.get(i)));
            }
            Log.d(TAG,"Component list"+components);
            show("Provision Complete", Toast.LENGTH_SHORT);
            mLightAddedStatus = LIGHT_ADDED;
            ActivityGroup.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if (ActivityGroup.this.components.size() != 0) {
                        adapterGrplist = new GrpDeviceListAdapter(serviceReference,
                                ActivityGroup.this,
                                ActivityGroup.this.components,
                                ActivityGroup.this.componentType,
                                null,
                                "group",
                                groupName);
                        if (adapterGrplist != null)
                            adapterGrplist.notifyDataSetChanged();
                        lv.setAdapter(adapterGrplist);

                        Log.d(TAG, "onProvisionComplete adding light groupId:" + groupName + " UUID= " + status + " type= " + type);
                    }
                }
            });
        }

    }

    @Override
    public void onHslStateChanged(final String deviceName, final int lightness, final int hue, final int saturation, final int remainingTime) {
//        show(" Received HSL :" + " lightness:"+lightness + " hue" + hue + " saturation:" +saturation + " remaining time:" + remainingTime + ", from :" + (deviceName), Toast.LENGTH_SHORT);
    }

    @Override
    public void onOnOffStateChanged(final String deviceName, final byte targetOnOff, final byte presentOnOff, final int remainingTime) {
        show(" Received Generic on/off :" + targetOnOff + ", from :" + (deviceName), Toast.LENGTH_SHORT);
    }

    @Override
    public void onLevelStateChanged(final String deviceName, final short targetLevel, final short presentLevel, final int remainingTime) {
        show(" Received level :" + targetLevel + ", from :" + (deviceName), Toast.LENGTH_SHORT);
    }

    @Override
    public void onNetworkConnectionStatusChanged(byte transport, final byte status) {
        Log.d(TAG, "recieved onNetworkConnectionStatusChanged , status = " + status);
        String text = null;
        if(status == IMeshControllerCallback.NETWORK_CONNECTION_STATE_CONNECTED)
        {
            text = "Connected to network";
        }
        if(status == IMeshControllerCallback.NETWORK_CONNECTION_STATE_DISCONNECTED)
            text = "Disconnected from network";

        if(text != null){
            show(text,Toast.LENGTH_SHORT);
        }
        runOnUiThread(new Runnable() {

            public void run() {
                if(status ==  IMeshControllerCallback.NETWORK_CONNECTION_STATE_CONNECTED) {
                    if(mApp.getPreferredTransport()==Constants.TRANSPORT_GATEWAY)
                        proxyConnView.setBackgroundColor(getResources().getColor(R.color.blue));
                    else
                        proxyConnView.setBackgroundColor(getResources().getColor(R.color.green));
                } else if(status == IMeshControllerCallback.NETWORK_CONNECTION_STATE_DISCONNECTED) {
                    proxyConnView.setBackgroundColor(getResources().getColor(R.color.red));
                }
            }
        });
    }

    @Override
    public void onCtlStateChanged(String deviceName, int presentLightness, short presentTemperature, final int targetLightness, short targetTemperature, int remainingTime) {
        show(" Received CTL : "+targetLightness, Toast.LENGTH_SHORT);
    }

    @Override
    public void onNodeConnStateChanged(final byte status, final String componentName) {
        Log.d(TAG,"onNodeConnStateChanged in Group UI");
/*
        final Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
            switch (status) {
            case IMeshControllerCallback.MESH_CLIENT_NODE_WARNING_UNREACHABLE:
                show("Node" +componentName+" failed to connect ", Toast.LENGTH_SHORT);
                break;

            case IMeshControllerCallback.MESH_CLIENT_NODE_ERROR_UNREACHABLE:
                show("!!! Action Required Node " +componentName+" unreachable", Toast.LENGTH_SHORT);
                break;
        }
            }
        }, 1000);
*/
    }

    @Override
    public void onOtaStatus(byte status, int percentComplete) {

    }

    @Override
    public void onNetworkOpenedCallback(byte status) {

    }

    @Override
    public void onComponentInfoStatus(byte status, String componentName, String componentInfo) {

    }

    @Override
    public void onDfuStatus(byte status, byte[] data) {

    }

    @Override
    public void onSensorStatusCb(String componentName, int propertyId, byte[] data) {

    }

    @Override
    public void onVendorStatusCb(short src, short companyId, short modelId, byte opcode, byte ttl, byte[] data, short dataLen) {

    }


    @Override
    public void onLightnessStateChanged(String deviceName, int target, int present, int remainingTime) {

    }

    @Override
    public void onLightLcModeStatus(String componentName, int mode) {

    }

    @Override
    public void onLightLcOccupancyModeStatus(String componentName, int mode) {

    }

    @Override
    public void onLightLcPropertyStatus(String componentName, int propertyId, int value) {

    }

    private void startScan() {
        Log.d(TAG, "startScan");
        serviceReference.getMesh().scanMeshDevices(true, null);
    }

    private void stopScan() {
        Log.d(TAG, "stopScan");
        serviceReference.getMesh().scanMeshDevices(false, null);
    }

    /*TODO check the implementation*/
    private void showScanDevicesDialog() {
        if(!isLocationServiceEnabled()) {
            Log.d(TAG, "isLocationServiceEnabled : location is false");
            show("Please turn on the location!!!", Toast.LENGTH_SHORT);
            //return;
        }

        LayoutInflater inflater = ActivityGroup.this.getLayoutInflater();
        View scanDevView = inflater.inflate(R.layout.pop_up_add_device, null);

        final ListView lvPeerDevices = (ListView) scanDevView.findViewById(R.id.listView6);
        final EditText deviceName = (EditText) scanDevView.findViewById(R.id.deviceName);
        List<MeshBluetoothDevice> listDevices = new ArrayList<MeshBluetoothDevice>();

        peerDevices = new ArrayAdapter<MeshBluetoothDevice>(ActivityGroup.this, R.layout.node_list_item, listDevices);
        lvPeerDevices.setAdapter(peerDevices);
        lvPeerDevices.setChoiceMode(AbsListView.CHOICE_MODE_SINGLE);

        if(lvPeerDevices.getCount() > 0)
            lvPeerDevices.setItemChecked(0, true);

        AlertDialog.Builder builder = new AlertDialog.Builder(ActivityGroup.this, AlertDialog.THEME_HOLO_LIGHT);
        builder.setView(scanDevView);
        //TODO check if network is already connected
      //  int res = serviceReference.getMesh().disconnectNetwork((byte)1);
        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        startScan();
        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                Log.d(TAG, "User clicked OK button");
                int itemPos = lvPeerDevices.getCheckedItemPosition();
                Log.d(TAG, "showScanDevicesDialog itemPos: " + itemPos);
                if (itemPos != -1) {
                    mPeerDevice = (MeshBluetoothDevice) lvPeerDevices.getAdapter().getItem(itemPos);
                    Log.d(TAG, "showScanDevicesDialog for node " + " BluetoothDevice:" + mPeerDevice);

                    if(mPeerDevice != null) {
                        Log.d(TAG, "provisioning the mPeerDevice = " + mPeerDevice);
                        mDeviceName = deviceName.getText().toString();
//                            isStoppedScan = true;
                        startProvision = true;

//                            if(!isStoppedScan)
                          isStoppedScan = true;
//                        stopScan();
//                        isStoppedScan = true;

                        //   mSpinTimer.postDelayed(myspinRunnable, PROVISION_SPINNER_TIMEOUT);
                        // startSpin();
                    }
                }

                dialog.dismiss();
            }
        });


        builder.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                Log.d(TAG, "User cancelled the dialog");
                isStoppedScan = true;
                stopScan();
            }
        });

        builder.setOnDismissListener(new DialogInterface.OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface dialog) {
                Log.d(TAG, "######## User dismissed the dialog ########");
                if(!isStoppedScan)
                    stopScan();
                isStoppedScan = false;

                if(startProvision) {
                    stopScan();

                    serviceReference.getMesh().setDeviceConfig(
                            null,
                            Constants.DEFAULT_IS_GATT_PROXY,
                            Constants.DEFAULT_IS_FRIEND,
                            Constants.DEFAULT_IS_RELAY,
                            Constants.DEFAULT_SEND_NET_BEACON,
                            Constants.DEFAULT_RELAY_XMIT_COUNT,
                            Constants.DEFAULT_RELAY_XMIT_INTERVAL,
                            Constants.DEFAULT_TTL,
                            Constants.DEFAULT_NET_XMIT_COUNT,
                            Constants.DEFAULT_NET_XMIT_INTERVAL
                    );

                    serviceReference.getMesh().setPublicationConfig(
                            Constants.DEFAULT_PUBLISH_CREDENTIAL_FLAG,
                            Constants.DEFAULT_RETRANSMIT_COUNT,
                            Constants.DEFAULT_RETRANSMIT_INTERVAL,
                            Constants.DEFAULT_PUBLISH_TTL
                    );
                    Log.d(TAG,"calling provision");
                    serviceReference.getMesh().provision(mDeviceName, groupName, mPeerDevice.mUUID, (byte)10);
                    startProvision = false;
                }

            }
        });

        AlertDialog alert = builder.create();
        alert.show();
    }
//    Runnable myspinRunnable = new Runnable() {
//        @Override
//        public void run() {
//            stopSpin();
//        }
//    };

    private ServiceConnection mConnection= new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "bound service connected");
            LightingService.MyBinder binder = (LightingService.MyBinder) service;
            serviceReference = binder.getService();
            serviceReference.setCurrentGroup(groupName);
            serviceReference.registerCb(ActivityGroup.this);
            updateDisplay();

            if(serviceReference.isConnectedToNetwork()) {
                if(mApp.getPreferredTransport() == Constants.TRANSPORT_GATEWAY)
                    proxyConnView.setBackgroundColor(getResources().getColor(R.color.blue));
                else
                    proxyConnView.setBackgroundColor(getResources().getColor(R.color.green));
            } else {
                proxyConnView.setBackgroundColor(getResources().getColor(R.color.red));
            }

            if (!serviceReference.getMesh().isConnectedToNetwork() && (Arrays.asList(serviceReference.getMesh().getGroupComponents(groupName)).size() != 0)){
                    show("Connecting to mesh network!", Toast.LENGTH_LONG);
                    serviceReference.getMesh().connectNetwork((byte)5);
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "bound service disconnected");
            serviceReference = null;
        }
    };

    private void updateDisplay() {
        components = new ArrayList<String>(Arrays.asList(serviceReference.getMesh().getGroupComponents(groupName)));
        componentType = new ArrayList<Integer>();
        if(components != null && components.size()!=0) {
            for(int i = 0; i< components.size(); i++) {
                Log.d(TAG, "Receiving light" + components.get(i));
                componentType.add(serviceReference.getMesh().getComponentType(components.get(i)));
            }

        }

        ActivityGroup.this.runOnUiThread(new Runnable() {

            public void run() {
                adapter = new GrpDeviceListAdapter(serviceReference, ActivityGroup.this, components,componentType, null, "group", groupName);
                lv.setAdapter(adapter);
                lv.setExpanded(true);
            }
        });
    }

    private boolean isNameAlreadyUsed(String name) {
        ArrayList<String> rooms = (ArrayList<String>) serviceReference.getallRooms();
        Boolean found = false;

        for(int i =0 ;i <rooms.size(); i++) {
            Log.d(TAG, "room name = " + rooms.get(i) + "current room name=" + name);
            if(rooms.get(i).equals(name)) {
                Log.d(TAG, "room name found");
                found = true;
            }
        }
        return found;
    }

//    void startSpin() {
//
//        runOnUiThread(new Runnable() {
//            @Override
//            public void run() {
//                progress = new ProgressDialog(ActivityGroup.this);
//                progress.setTitle("Provision and Configuration");
//                progress.setMessage("provision in Progress...");
//                progress.setCancelable(false);
//                progress.show();
//
//            }
//        });
//
//    }

    //    void stopSpin() {
//        Log.d(TAG,"Provision failure - Stopping spin");
//        runOnUiThread(new Runnable() {
//            @Override
//            public void run() {
//                progress.setMessage("Provision failure");
//                progress.dismiss();
//                Toast.makeText(ActivityGroup.this, "Provision failure", Toast.LENGTH_SHORT).show();
//            }
//        });
//
//}


    void popUpTransTime(View editView) {
        AlertDialog.Builder builder = new AlertDialog.Builder(ActivityGroup.this, R.style.AlertDialogCustom);
        final EditText text = (EditText) editView.findViewById(R.id.editText);
        builder.setView(editView);

        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                Log.d(TAG, "User clicked OK button");
                transtime.setText(text.getText().toString());
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
        alert.getWindow().setLayout(ActionBar.LayoutParams.WRAP_CONTENT, ActionBar.LayoutParams.WRAP_CONTENT);
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, final Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "Received intent: " + action);

            if(action.equals(Constants.GATT_PROXY_CONNECTED)){
                Log.d(TAG, Constants.GATT_PROXY_CONNECTED);
                proxyConnView.setBackgroundColor(getResources().getColor(R.color.green));

                if(ActivityGroup.this!=null) {
                    //serviceReference.isConnectedToNetwork = true;
          //          ActivityGroup.this.runOnUiThread(new Runnable() {
                  //      public void run() {

//                            if(mLightAddedStatus == LIGHT_ADDED) {
//                               progress.setMessage("Configuring...");
//                            }

            //            }
                //    });

                    Log.d(TAG, "Configure from config button");
//                    if(!isConfigComplete)
//                        configure(newNode, groupName);
//                    try {
//                        Thread.sleep(3000);
//                        Log.d(TAG, "Configure from config button");
//                        if(!isConfigComplete)
//                            configure(newNode, groupId);
//                    } catch (InterruptedException e) {
//                        e.printStackTrace();
//                    }
                }
            } else if(action.equals(Constants.GATT_PROXY_DISCONNECTED)){
                proxyConnView.setBackgroundColor(getResources().getColor(R.color.red));
            } else if(action.equals(Constants.GATT_PROXY_CONNECTED_FOR_CONFIG)) {
                proxyConnView.setBackgroundColor(getResources().getColor(R.color.green));
            }
        }
    };

    public boolean isLocationServiceEnabled(){
        LocationManager locationManager = null;
        boolean gps_enabled= false,network_enabled = false;

        if(locationManager ==null)
            locationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
        try{
            gps_enabled = locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER);
        }catch(Exception ex){
            //do nothing...
        }

        try{
            network_enabled = locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER);
        }catch(Exception ex){
            //do nothing...
        }

        return gps_enabled || network_enabled;

    }
}
