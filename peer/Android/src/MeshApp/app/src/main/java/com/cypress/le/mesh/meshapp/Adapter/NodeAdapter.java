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
package com.cypress.le.mesh.meshapp.Adapter;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import androidx.core.graphics.ColorUtils;
import androidx.appcompat.app.ActionBar;

import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;

import com.cypress.le.mesh.meshapp.Constants;
import com.cypress.le.mesh.meshapp.HSVColorPickerDialog;
import com.cypress.le.mesh.meshapp.LightController;
import com.cypress.le.mesh.meshapp.LightingService;
import com.cypress.le.mesh.meshapp.R;
import com.cypress.le.mesh.meshapp.SensorSetting;
import com.cypress.le.mesh.meshframework.MeshController;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import static com.cypress.le.mesh.meshapp.R.*;


public class NodeAdapter extends BaseAdapter {
    private static final String TAG = "NodeAdapter";
    private Context context = null;
    private HashMap<Integer, Boolean> checklist = new HashMap<Integer, Boolean>();
    private LightingService service = null;
    int progress = 0;
    int temp = 0;
    int lightness = 0;
    int mColor;
    int mComponentType = 0;
    String mCompName = null;
    ArrayList<String> targetMethodTypes = new ArrayList<String>();
    ArrayList<String> controlMethodTypes = new ArrayList<String>();
    TextView pubaddr;
    String mGroupName = null;
    List<Integer> propertyList = new ArrayList<Integer>();
    List<String> propertyListNames = new ArrayList<String>();
    static int propertySelected;

    String mVendorData = null;


    public NodeAdapter(LightingService serviceReference,
                       Context ctx,
                       String CompName,
                       int compType,
                       ArrayList<String> targetMethods,
                       ArrayList<String> controlMethods,
                       String groupName) {
        service = serviceReference;
        mCompName = CompName;
        context = ctx;
        mComponentType = compType;
        Log.d(TAG, "mComponentType " + mComponentType);
        targetMethodTypes = targetMethods;
        Log.d(TAG, "TargetMethods " + targetMethodTypes);
        controlMethodTypes = controlMethods;
        Log.d(TAG, "controlMethods " + controlMethodTypes);
        mGroupName = groupName;

    }

    @Override
    public int getCount() {
        return 1;
    }

    @Override
    public Object getItem(int position) {
        return position;
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(final int position, View convertView, final ViewGroup parent) {

        checklist.put(position, false);
        LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View modelrowView = inflater.inflate(layout.models, null);

        View onoffView = modelrowView.findViewById(id.model_gen_onoff);
        final View lightnessview = modelrowView.findViewById(id.model_lightness);
        View vendorview = modelrowView.findViewById(id.model_vendor);
        View config = modelrowView.findViewById(id.model_config);
        View levelview = modelrowView.findViewById(id.genericLevelView);
        View hslview = modelrowView.findViewById(id.HSLView);
        View OnOffClientview = modelrowView.findViewById(id.OnOffClient);
        View sensorClientView = modelrowView.findViewById(id.SensorClient);
        View ctlclientView = modelrowView.findViewById(id.ctlClientView);
        View vendorClientView = modelrowView.findViewById(id.vendorClientView);
        View lightLcView = modelrowView.findViewById(id.model_light_lc);

        final Button startListening = (Button) modelrowView.findViewById(R.id.start_listening);
        final Button stopListening = (Button) modelrowView.findViewById(R.id.stop_listening);

        ImageView lightnessget = (ImageView) modelrowView.findViewById(id.ctlimg);

        final Button vendorSend = (Button) modelrowView.findViewById(id.vendorSet);
        final EditText vendorData = (EditText) modelrowView.findViewById(id.vendorData);
        final Button Lcsetting = (Button) modelrowView.findViewById(id.LcSetting);


        ImageView hslget = (ImageView) modelrowView.findViewById(id.color);
        Button onBtn = (Button) modelrowView.findViewById(id.onbtn);
        Log.d(TAG, "position = " + position);

        Button offBtn = (Button) modelrowView.findViewById(id.offbtn);
        onBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final boolean val = true;
                Thread t = new Thread(new Runnable() {
                    public void run() {
                        service.getMesh().onoffSet(mCompName, val, true, Constants.DEFAULT_TRANSITION_TIME, (short) 0);
                    }
                });
                t.start();
            }
        });
        offBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final boolean val = false;
                Thread t = new Thread(new Runnable() {
                    public void run() {
                        service.getMesh().onoffSet(mCompName, val, true, Constants.DEFAULT_TRANSITION_TIME, (short) 0);
                    }
                });
                t.start();
            }
        });

        SeekBar level = (SeekBar) modelrowView.findViewById(id.levelBar);

        level.setMax(65535);

        level.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int prg, boolean fromUser) {
                progress = prg - 32768;

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                Log.d(TAG, "Level Bar onProgressChanged : " + progress);

                Thread t = new Thread(new Runnable() {
                    public void run() {
                        service.OnLevelChange(mCompName, progress);
                    }
                });
                t.start();

            }
        });
        SeekBar lightnessSeek = (SeekBar) modelrowView.findViewById(id.lightnessbar);
        lightnessSeek.setMax(65535);

        lightnessSeek.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int prg, boolean fromUser) {
                lightness = prg;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                Log.d(TAG, "Level Bar onProgressChanged : " + lightness);

                Thread t = new Thread(new Runnable() {
                    public void run() {
                        service.getMesh().lightnessSet(mCompName, lightness, true, Constants.DEFAULT_TRANSITION_TIME, (short) 0);
                    }
                });
                t.start();

            }
        });

        lightnessget.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                service.getMesh().lightnessGet(mCompName);
            }
        });
        hslget.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                service.getMesh().hslGet(mCompName);
            }
        });

        SeekBar tempSeekbar = (SeekBar) modelrowView.findViewById(id.tempBar);
        tempSeekbar.setMax(19200);

        tempSeekbar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int prg, boolean fromUser) {
                temp = prg + 800;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        SeekBar ctlLightness = (SeekBar) modelrowView.findViewById(id.ctllightness);
        ctlLightness.setMax(65534);
        ctlLightness.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int prg, boolean fromUser) {
                lightness = prg + 1;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        Button ctlsend = (Button) modelrowView.findViewById(id.ctlsend);
        ctlsend.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Thread t = new Thread(new Runnable() {
                    public void run() {
                        if (temp == 0)
                            temp = 800;
                        service.ctlSet(mCompName, temp, lightness, Constants.DEFAULT_TRANSITION_TIME, 0);
                    }
                });
                t.start();
            }
        });
        Button picker = (Button) modelrowView.findViewById(id.picker);
        picker.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                HSVColorPickerDialog cpd = new HSVColorPickerDialog(context, 0xFF4488CC, new HSVColorPickerDialog.HSVDailogChangeListener() {
                    @Override
                    public void onColorSelected(Integer color) {
                        Log.d(TAG, "onColorSelected");
                    }

                    @Override
                    public void onColorChanged(Integer color) {
                        Log.d(TAG, "onColorChanged");
                        mColor = color;
                        sendHSLSet(position);
                    }

                    @Override
                    public void onStopColorChanged() {
                        Log.d(TAG, "onStopColorChanged");
                        service.setHSLStopTracking();
                    }

                    @Override
                    public void onStartColorChanged() {
                        Log.d(TAG, "onStartColorChanged");
                        service.setHSLStartTracking();
                    }
                });

                cpd.setTitle("Pick a color");
                cpd.show();
            }
        });

        Button assignBtn = (Button) modelrowView.findViewById(id.assign);
        pubaddr = (TextView) modelrowView.findViewById(id.bulb_assign);

        pubaddr.setText("NA");
        assignBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                View assignView = View.inflate(context, layout.pop_up_assign, null);
                //check how to get method type
                popUpAssign(assignView, controlMethodTypes.get(position), true);
            }
        });

        //SENSOR
        final Button configSet = (Button) modelrowView.findViewById(id.configureSetting);
        configSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent intent;
                Bundle bundle = new Bundle();
                bundle.putInt("mPropertyId", propertySelected);
                bundle.putString("mComponentName", mCompName);
                intent = new Intent(context, SensorSetting.class);
                intent.putExtras(bundle);
                context.startActivity(intent);

            }
        });

        Button sensorGet = (Button) modelrowView.findViewById(id.sensordata);
        Spinner propertyIdList = (Spinner) modelrowView.findViewById(id.propertyId);

        propertyIdList.setOnItemSelectedListener(
                new AdapterView.OnItemSelectedListener() {
                    @Override
                    public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                        Log.d(TAG, "on propertyList item clicked pos : " + i + "propertyList :" + propertyList.get(i));
                        propertySelected = propertyList.get(i);
                    }

                    @Override
                    public void onNothingSelected(AdapterView<?> adapterView) {

                    }
                }
        );

        sensorGet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                service.getMesh().sensorGet(mCompName, propertySelected);
            }
        });

        lightnessview.setVisibility(View.GONE);
        vendorview.setVisibility(View.GONE);
        config.setVisibility(View.GONE);
        levelview.setVisibility(View.GONE);
        hslview.setVisibility(View.GONE);
        OnOffClientview.setVisibility(View.GONE);
        ctlclientView.setVisibility(View.GONE);
        onoffView.setVisibility(View.GONE);
        sensorClientView.setVisibility(View.GONE);
        vendorClientView.setVisibility(View.GONE);
        lightLcView.setVisibility(View.GONE);
        if (targetMethodTypes != null) {
            if (targetMethodTypes.contains("ONOFF")) {
                onoffView.setVisibility(View.VISIBLE);
                if(service != null)
                    if(service.getMesh().isLightController(mCompName) > 0)
                        lightLcView.setVisibility(View.VISIBLE);
            }

            if (targetMethodTypes.contains("LEVEL")) {
                levelview.setVisibility(View.VISIBLE);
            }

            if (targetMethodTypes.contains("LIGHTNESS")) {
                lightnessview.setVisibility(View.VISIBLE);
                levelview.setVisibility(View.GONE);
                if(service != null)
                    if(service.getMesh().isLightController(mCompName) > 0)
                        lightLcView.setVisibility(View.VISIBLE);
            }

            if (targetMethodTypes.contains("HSL")) {
                hslview.setVisibility(View.VISIBLE);
                levelview.setVisibility(View.GONE);
                lightnessview.setVisibility(View.GONE);
                onoffView.setVisibility(View.GONE);
                if(service != null)
                    if(service.getMesh().isLightController(mCompName) > 0)
                        lightLcView.setVisibility(View.VISIBLE);

            }

            if (targetMethodTypes.contains("CTL")) {
                ctlclientView.setVisibility(View.VISIBLE);
                levelview.setVisibility(View.GONE);
                lightnessview.setVisibility(View.GONE);
                if(service != null)
                    if(service.getMesh().isLightController(mCompName) > 0)
                        lightLcView.setVisibility(View.VISIBLE);
            }

            for (int i = 0; i < targetMethodTypes.size(); i++)
                if (targetMethodTypes.get(i).contains("VENDOR_")) {
                    vendorClientView.setVisibility(View.VISIBLE);
                }

            if (targetMethodTypes.contains("SENSOR")) {
                sensorClientView.setVisibility(View.VISIBLE);

                int[] propValues = service.getMesh().sensorPropertyListGet(mCompName);

                propertyList.clear();
                propertyListNames.clear();
                if (propValues != null) {
                    for (int i = 0; i < propValues.length; i++) {
                        propertyList.add(propValues[i]);
                        propertyListNames.add(getProperty(propValues[i]));
                    }
                    propertySelected = propertyList.get(0);
                }

                ArrayAdapter properties = new ArrayAdapter(context, android.R.layout.simple_spinner_item, propertyListNames);
                properties.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
                propertyIdList.setAdapter(properties);
            }
        }

        if (controlMethodTypes != null) {
            if (controlMethodTypes.contains("ONOFF")) {
                OnOffClientview.setVisibility(View.VISIBLE);
            }

            if (controlMethodTypes.contains("LEVEL")) {
                OnOffClientview.setVisibility(View.VISIBLE);
            }

//            if(controlMethodTypes.contains("SENSOR")) {
//                OnOffClientview.setVisibility(View.VISIBLE);
//            }
        }

        startListening.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Thread t = new Thread(new Runnable() {
                    public void run() {
                        service.getMesh().listenForAppGroupBroadcasts(MeshController.MESH_CONTROL_METHOD_SENSOR, mGroupName, true);
                    }
                });
                t.start();
            }
        });
        stopListening.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Thread t = new Thread(new Runnable() {
                    public void run() {
                        service.getMesh().listenForAppGroupBroadcasts(MeshController.MESH_CONTROL_METHOD_SENSOR, mGroupName, false);
                    }
                });
                t.start();
            }
        });

        //Vendor data
        vendorData.setText(mVendorData);
        vendorSend.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                short company_id = 0x131;
                short model_id = 0x01;
                byte opcode = 0x01;

                if (vendorData.getText().toString() != null) {
                    byte[] bytes = hexStringToByteArray(vendorData.getText().toString());
                    service.getMesh().vendorDataSend(mCompName, company_id, model_id, opcode,false, bytes, (short) bytes.length);
                }
            }
        });

        vendorData.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View view, boolean hasFocus) {
                if (!hasFocus) {
                    mVendorData = vendorData.getText().toString().trim();
                }
            }
        });

        Lcsetting.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent intent;
                Bundle bundle = new Bundle();
                bundle.putString("mComponentName", mCompName);
                intent = new Intent(context, LightController.class);
                intent.putExtras(bundle);
                context.startActivity(intent);
            }
        });
          return modelrowView;
    }


    private String getProperty(int propValue) {
        Log.d(TAG, "getProperty :"+propValue);
        switch (propValue)
        {
            case 0x2B: return "Lux level on";
            case 0x42: return "Motion sensed";
            case 0x4D: return "Presence detected";
            case 0x4E: return "Ambient light level";
            case 0x4F: return "Ambient temperature";
            case 0x6E: return "Device Runtime";
            default: return "Unknown prop: " + Integer.toHexString( propValue );
        }
    }

    void sendHSLSet(int position) {
        float hsl[] = new float[3];
        ColorUtils.colorToHSL(mColor,hsl);
        final int hue = Math.round(((float)(65535 * (int)hsl[0]))/(float)360);
        final int saturation = Math.round(65535 * (hsl[1]));
        final int lightness = Math.round(65535 * (hsl[2]));

        Log.d(TAG, "sendHSLSet hue: "+hsl[0]+ " saturation"+hsl[1]*100+ " lightness"+hsl[2]*100);
        Thread t = new Thread(new Runnable() {
            public void run() {
                service.getMesh().hslSet(mCompName, lightness, hue, saturation, true, Constants.DEFAULT_TRANSITION_TIME, (short)0);
            }
        });
        t.start();
    }

    void popUpAssign(View groupView, final String method, final boolean isControlMethod) {

        final ArrayList<String>components = service.getGroupComponents(mGroupName);
        components.addAll(service.getallRooms());
        for(int i=0;i<components.size();i++) {
            if(components.get(i).equals(mCompName)) {
                components.remove(i);
            }
        }
        components.add("all-nodes");
        components.add("all-proxies");
//        components.add("all_friends");
//        components.add("all-relays");
        final ArrayAdapter<String> itemsAdapter = new ArrayAdapter<String>(context, layout.node_list_item, components);
        final EditText pubTime = (EditText)groupView.findViewById(id.pub_time);
        final ListView listview =  (ListView)groupView.findViewById(id.listView2);
        listview.setAdapter(itemsAdapter);
        listview.setChoiceMode(AbsListView.CHOICE_MODE_SINGLE);

        if(listview.getCount() > 0)
            listview.setItemChecked(0, true);

        android.app.AlertDialog.Builder builder = new android.app.AlertDialog.Builder(context, android.app.AlertDialog.THEME_HOLO_LIGHT);
        builder.setView(groupView);

        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {

                Log.d(TAG, "User clicked OK button" + service.getCurrentGroup());

                int itemPos = listview.getCheckedItemPosition();
                String selectedDevice = (String)listview.getAdapter().getItem(itemPos);
                if (selectedDevice == null) {
                    return;
                }
                final String name = selectedDevice;
                Log.d(TAG, "Selected Light" + name + " method:"+method+" component name:"+mCompName );

                Thread t = new Thread(new Runnable() {
                    public void run() {
                        service.getMesh().setPublicationConfig(
                                Constants.DEFAULT_PUBLISH_CREDENTIAL_FLAG,
                                Constants.DEFAULT_RETRANSMIT_COUNT,
                                Constants.DEFAULT_RETRANSMIT_INTERVAL,
                                Constants.DEFAULT_PUBLISH_TTL
                        );

                        int ret = service.getMesh().configurePublication(mCompName, isControlMethod, method, name, !pubTime.getText().toString().isEmpty() ? Integer.parseInt(pubTime.getText().toString()):Constants.DEFAULT_PUBLISH_PERIOD);
                        Log.d(TAG, "config pub"+ret);
                    }
                });
                t.start();

                pubaddr.setText(selectedDevice);
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


    public static byte[] hexStringToByteArray(String s) {
        int len = s.length();
        if (len%2 != 0)
            len--;
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4)
                    + Character.digit(s.charAt(i+1), 16));
        }
        return data;
    }


}
