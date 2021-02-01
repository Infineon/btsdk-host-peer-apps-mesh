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
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import com.google.android.material.appbar.CollapsingToolbarLayout;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;

//import com.larswerkman.holocolorpicker.ColorPicker;

import com.cypress.le.mesh.meshapp.Adapter.SelectListAdapter;



public class NewRoom extends AppCompatActivity {
    private static final String TAG = "NewRoom";

    int selecteditem = -1;
    ArrayList<String> devicelist = new ArrayList<String>();
    SelectListAdapter adapter;
    TextView text;
    Toolbar toolbar;
    CollapsingToolbarLayout col;
//    ColorPicker picker;
    String name = "New Room";
    ImageButton imgbtn;
    CollapsingToolbarLayout collapsingToolbarLayout;
    Context context;
    LightingService serviceReference = null;
    private static Toast mToast = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.new_room);
        Intent intent= new Intent(this, LightingService.class);
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);

        //imgbtn = (ImageButton) findViewById(R.id.seekBar5);
      //  imgbtn.setBackgroundColor(getResources().getColor(R.color.text));
//        if(imgbtn != null) {
//            imgbtn.setOnClickListener(new View.OnClickListener() {
//                @Override
//                public void onClick(View v) {
//                    View lightView = View.inflate(NewRoom.this,R.layout.pop_up_setting_color, null);
//                    popUpLight(lightView);
//                }
//            });
//        }
        collapsingToolbarLayout = (CollapsingToolbarLayout) findViewById(R.id.toolbar_layout);
        final Button btn = (Button) findViewById(R.id.add);
        if(btn != null) {
            btn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    View addDeviceView = View.inflate(NewRoom.this, R.layout.pop_up_add_device, null);
                    popUpAddDevice(addDeviceView);
                }
            });
        }

        final Button save = (Button) findViewById(R.id.save);
        if(save != null) {
            save.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    Log.d(TAG, "hello im creating new room ---->" + name);
                    if(isNameAlreadyUsed(name)) {
                        show("RoomName is already in use Please change",
                                Toast.LENGTH_SHORT);
                    } else {
                        serviceReference.createRoom(name, LightingService.getCurrentNetwork());
                        NewRoom.this.onBackPressed();
                    }

                }
            });
        }
        col = (CollapsingToolbarLayout)findViewById(R.id.toolbar_layout);
        col.setTitle("New Room");
        toolbar = (Toolbar) findViewById(R.id.toolbar);
        toolbar.setTitle("New Room");
        toolbar.inflateMenu(R.menu.new_room_menu);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        toolbar.setOnMenuItemClickListener(new Toolbar.OnMenuItemClickListener() {
            @Override
            public boolean onMenuItemClick(MenuItem item) {
                // Handle the menu item
                Log.d(TAG, "home" + item.getItemId());
                switch (item.getItemId()) {
                    case android.R.id.home:
                        Log.d(TAG, "home");
                        NewRoom.this.onBackPressed();
                        break;
                    case R.id.edit:
                        Log.d(TAG, "edit");
                        View editView = View.inflate(NewRoom.this, R.layout.pop_up_edit_name, null);
                        popUpEdit(editView);
                        break;
                }
                return true;
            }
        });
        toolbar.inflateMenu(R.menu.new_room_menu);
    }

        @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.new_room_menu, menu);
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
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    @Override
    public void onResume(){
        Log.d(TAG, "OnResume");
        super.onResume();
    }

    @Override
    protected void onDestroy() {
        unbindService(mConnection);
        super.onDestroy();
        devicelist = null;
    }

//    void popUpLight(View lightView) {
//        picker = (ColorPicker) lightView.findViewById(R.id.picker);
//
//        AlertDialog.Builder builder = new AlertDialog.Builder(NewRoom.this, R.style.AlertDialogCustom);
//        builder.setView(lightView);
//        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
//            public void onClick(DialogInterface dialog, int id) {
//                Log.d(TAG, "User clicked OK button");
//                dialog.dismiss();
//                imgbtn.setBackgroundColor(picker.getColor());
//
//            }
//        });
//        builder.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
//            public void onClick(DialogInterface dialog, int id) {
//                Log.d(TAG, "User cancelled the dialog");
//            }
//        });
//
//        final AlertDialog alert = builder.create();
//        alert.setOnShowListener(new DialogInterface.OnShowListener() {
//            @Override
//            public void onShow(DialogInterface dialog) {
//                alert.getButton(AlertDialog.BUTTON_NEGATIVE).setTextColor(getResources().getColor(R.colorimg.accent));
//                alert.getButton(AlertDialog.BUTTON_POSITIVE).setTextColor(getResources().getColor(R.colorimg.accent));
//            }
//        });
//        alert.show();
//        alert.getWindow().setLayout(ActionBar.LayoutParams.WRAP_CONTENT, ActionBar.LayoutParams.WRAP_CONTENT);
//    }

    void popUpAddDevice(View addDevice) {
        ArrayAdapter<String> deviceListadapter =
                new ArrayAdapter<String>(NewRoom.this,
                        android.R.layout.simple_list_item_1,devicelist);
        final ListView listView = (ListView) addDevice.findViewById(R.id.listView6);
        listView.setAdapter(deviceListadapter);
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Log.d(TAG, "listener" + "selected item = " + position);
                selecteditem = position;
            }
        });

        AlertDialog.Builder builder = new AlertDialog.Builder(NewRoom.this, R.style.AlertDialogCustom);
        builder.setView(addDevice);
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

    void popUpEdit(View editView) {
        Log.d(TAG, "popUpEdit");
        AlertDialog.Builder builder = new AlertDialog.Builder(NewRoom.this, R.style.AlertDialogCustom);
        final EditText text = (EditText) editView.findViewById(R.id.editText);
        builder.setView(editView);
        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                toolbar.setTitle(text.getText());
                name = text.getText().toString();
                if(isNameAlreadyUsed(name)) {
                    show("RoomName is already in use",
                            Toast.LENGTH_SHORT);
                    collapsingToolbarLayout.setTitle(name);
                } else {
                    collapsingToolbarLayout.setTitle(name);
                }
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

    private ServiceConnection mConnection= new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "bound service connected");
            LightingService.MyBinder binder = (LightingService.MyBinder) service;
            serviceReference = binder.getService();

            int groupId =1;
//            ArrayList<BLEMeshLightGroup> groups = serviceReference.getTopLevelGroups(serviceReference.getCurrentGroup());
//            ArrayList<BLEMeshLightDevice> components = serviceReference.getTopLevelLights(groupId);
//            if((groups!=null) || (components!=null)) {
//                adapter = new SelectListAdapter(NewRoom.this, components, groups);
//            }
//
//            ExpandableHeightListView lv =  (ExpandableHeightListView)findViewById(R.id.listViewNewLocation);
//            lv.setAdapter(adapter);
//            lv.setExpanded(true);
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            serviceReference = null;
        }
    };

    public void show(final String text,final int duration) {
        runOnUiThread(new Runnable() {

            public void run() {
                if (mToast == null || !mToast.getView().isShown()) {
                    if (mToast != null) {
                        mToast.cancel();
                    }
                }
                mToast = Toast.makeText(getApplicationContext(), text, duration);
                mToast.show();
            }
        });

    }
}
