/*
 * Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of Cypress Semiconductor
 * Corporation. All rights reserved. This software, including source code, documentation and  related
 * materials ("Software"), is owned by Cypress Semiconductor  Corporation or one of its
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
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ImageButton;
import android.widget.Switch;
import android.widget.TextView;
import java.util.ArrayList;
import com.cypress.le.mesh.meshapp.ActivityGroup;
import com.cypress.le.mesh.meshapp.ActivityModel;
import com.cypress.le.mesh.meshapp.Constants;
import com.cypress.le.mesh.meshapp.LightingService;
import com.cypress.le.mesh.meshapp.R;
import com.cypress.le.mesh.meshframework.MeshController;


public class GrpDeviceListAdapter extends BaseAdapter {
    private static final String TAG = "GrpDeviceListAdapter";

    private final Context context;
    private ArrayList<String> list = new ArrayList<String>();
    private ArrayList<String> grpDev = new ArrayList<String>();
    private ArrayList<String> values = new ArrayList<String>();
    private ArrayList<Drawable> image = new ArrayList<Drawable>();
    ArrayList<String> componentList = new ArrayList<String>();
    ArrayList<Integer> componentTypeList = new ArrayList<Integer>();
    ArrayList<String> groupList = new ArrayList<String>();
    ArrayList<Integer> seekList = new ArrayList<Integer>();
    ArrayList<Integer> lightseekList = new ArrayList<Integer>();
    View mOffBtnView;
    private String groupType;
    LightingService serviceReference = null;

    String groupName;

    public GrpDeviceListAdapter(LightingService serviceReference, Context context,
                                ArrayList<String> lightList,
                                ArrayList<Integer> componentType,
                                ArrayList<String> groupList,
                                String groupType, String groupName) {
        this.componentTypeList = componentType;
        this.componentList = lightList;
        this.groupList = groupList;
        this.context = context;
        this.groupType = groupType;
        this.serviceReference = serviceReference;
        this.groupName = groupName;

        int i;
//        lightseekList = serviceReference.getBrightnessValues(lightList);
        if(groupList != null) {
            Log.d(TAG, "groupList not null" + groupList.size());
            for (i = 0; i < groupList.size(); i++) {
                grpDev.add("group");
                Log.d(TAG, "adding group" + groupList.get(i));
                list.add(groupList.get(i));
//              seekList.add(serviceReference.getMesh().onoffGet(groupList.get(i)));
                seekList.add(0);
            }
        }
        if( lightList != null) {
            Log.d(TAG, "componentList not null" + lightList.size());
            for(i=0; i<lightList.size();i++) {
                grpDev.add("light");
                Log.d(TAG, "adding light" + lightList.get(i));
                list.add(lightList.get(i));
//                seekList.add(lightseekList.get(i));
                seekList.add(0);
            }

        }

        Log.d(TAG, "GrpDeviceListAdapter");

    }

    @Override
    public int getCount() {
        if (grpDev!=null) {
            return grpDev.size();
        } else return 0;

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
    public View getView(final int position, View convertView, ViewGroup parent) {
        Log.d(TAG, "Value = " + list.get(position) + " Value=" + grpDev.get(position));

        LayoutInflater inflater = (LayoutInflater) context
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View rowView = inflater.inflate(R.layout.adapter_device_list_item, null);

        if(position == 0 && list.isEmpty())
            return rowView;

        final ImageButton imagebutton = (ImageButton) rowView.findViewById(R.id.imageButton);
        TextView textView = (TextView) rowView.findViewById(R.id.DevGrpName);
        mOffBtnView = (View) rowView.findViewById(R.id.onoffBtnView);
        Button onBtn = (Button) rowView.findViewById(R.id.onbtn);
        Button offBtn = (Button) rowView.findViewById(R.id.offbtn);
        if(grpDev.get(position).equals("group")) {
            imagebutton.setImageResource(R.drawable.groups);

        } else {
           imagebutton.setImageResource(R.drawable.meshdev);
        }
        if(grpDev.get(position).equals("group")) {

            //seekBar.setProgress(serviceReference.getGroupBrightness(groupList.get(position).getId()));
//            if(seekList.get(position)!= null) {
//                if(serviceReference.getMesh().onoffGet(groupList.get(position)) > 0) {
//                    seekBar.setChecked(true);
//                } else {
//                    seekBar.setChecked(false);
//                }
//            }

        }

        String component = null;
        if(grpDev.get(position).equals("light")) {
            if (componentList != null && groupList != null) {
                {
                    component = componentList.get(position - groupList.size());
                }

            } else if(componentList !=null) {
                component = componentList.get(position);
            }
            if(component != null) {
                int compType = componentTypeList.get(position);
                Log.d(TAG, "component Type :" + compType);
                switch (compType) {
                    case MeshController.COMPONENT_TYPE_GENERIC_ON_OFF_CLIENT : {
                        imagebutton.setImageResource(R.drawable.light_switch);
                        mOffBtnView.setVisibility(View.GONE);
                    } break;
                    case MeshController.COMPONENT_TYPE_GENERIC_ON_OFF_SERVER : {
                        imagebutton.setImageResource(R.drawable.lightbulb_on_outline);
                    } break;
                    case MeshController.COMPONENT_TYPE_LIGHT_DIMMABLE : {
                        imagebutton.setImageResource(R.drawable.lightbulb_on_outline);
                    } break;
                    case MeshController.COMPONENT_TYPE_GENERIC_LEVEL_CLIENT : {
                        imagebutton.setImageResource(R.drawable.light_switch);
                        mOffBtnView.setVisibility(View.GONE);
                    } break;
                    case MeshController.COMPONENT_TYPE_LIGHT_HSL : {
                        imagebutton.setImageResource(R.drawable.lightbulb_on_outline);
                        mOffBtnView.setVisibility(View.GONE);
                    } break;
                    case MeshController.COMPONENT_TYPE_LIGHT_CTL : {
                        imagebutton.setImageResource(R.drawable.lightbulb_on_outline);
                    } break;
                    case MeshController.COMPONENT_TYPE_SENSOR : {
                        imagebutton.setImageResource(R.drawable.sensor);
                        mOffBtnView.setVisibility(View.GONE);
                    } break;
                    case MeshController.COMPONENT_TYPE_SENSOR_CLIENT : {
                        imagebutton.setImageResource(R.drawable.client);
                        mOffBtnView.setVisibility(View.GONE);
                    } break;
                    default: {
                        imagebutton.setImageResource(R.drawable.meshdev);
                        mOffBtnView.setVisibility(View.GONE);
                    } break;
                }
            }
        }

        onBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final boolean val = true;
                if (grpDev.get(position).equals("group")) {
                    Log.d(TAG, "group");
                    if (position < groupList.size()) {

                        if (groupList != null) {
                            serviceReference.setGroupBrightness(groupList.get(position), val);
                            Log.d(TAG, "group lighting");
                        }
                    }
                } else if(grpDev.get(position).equals("light")) {
                    Log.d(TAG, "else");
                    if (componentList != null && groupList != null) {
                        {
                            Log.d(TAG, "sending brightnesschange both not null");
                            Thread t = new Thread(new Runnable() {
                                public void run() {
                                    serviceReference.getMesh().onoffSet(componentList.get(position - groupList.size()), val, false, Constants.DEFAULT_TRANSITION_TIME, (short) 0);

                                }
                            });
                            t.start();

                        }

                    } else if(componentList !=null) {
                        Thread t = new Thread(new Runnable() {
                            public void run() {
                                serviceReference.getMesh().onoffSet(componentList.get(position), val, false, Constants.DEFAULT_TRANSITION_TIME, (short) 0);
                            }
                        });
                        t.start();
                    }
                }
            }
        });
        offBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final boolean val = false;
                if (grpDev.get(position).equals("group")) {
                    Log.d(TAG, "group");
                    if (position < groupList.size()) {

                        if (groupList != null) {
                            serviceReference.setGroupBrightness(groupList.get(position), val);
                            Log.d(TAG, "group lighting");
                        }
                    }
                } else if(grpDev.get(position).equals("light")) {
                    Log.d(TAG, "else");
                    if (componentList != null && groupList != null) {
                        {
                            Log.d(TAG, "sending brightnesschange both not null");
                            Thread t = new Thread(new Runnable() {
                                public void run() {
                                    serviceReference.getMesh().onoffSet(componentList.get(position - groupList.size()), val, false, Constants.DEFAULT_TRANSITION_TIME, (short) 0);

                                }
                            });
                            t.start();

                        }

                    } else if(componentList !=null) {
                        Thread t = new Thread(new Runnable() {
                            public void run() {
                                serviceReference.getMesh().onoffSet(componentList.get(position), val, false, Constants.DEFAULT_TRANSITION_TIME, (short) 0);
                            }
                        });
                        t.start();
                    }
                }
            }
        });

        textView.setText(list.get(position));

        if (imagebutton != null) {

            //set as the tag the position parameter
            imagebutton.setTag(new Integer(position));
            rowView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    Integer realPosition = (Integer) v.getTag();

                    if (grpDev.get(position).equals("group")) {
                        Intent intent;
                        if(position < groupList.size()) {
                            Bundle bundle = new Bundle();
                            if(groupList!=null) {
                                bundle.putString("GroupName", groupList.get(position));
                                bundle.putString("name", list.get(position));
                                bundle.putString("groupType", groupType);
                                bundle.putInt("seek", seekList.get(position));
                                serviceReference.pushStack(groupList.get(position));
                            }
                            Log.d(TAG, "openeing activitygroup groupName:"+groupList.get(position));
                            intent = new Intent(context, ActivityGroup.class);
                            intent.putExtras(bundle);
                        } else {
                            Bundle bundle = new Bundle();
                            if(componentList !=null && groupList!=null ) {
                                bundle.putString("name", componentList.get(position-groupList.size()));
                                serviceReference.pushStack(groupList.get(position));
                            }

                            if(componentList !=null)
                                bundle.putString("name", list.get(position));
                            bundle.putString("groupType", groupType);
                            bundle.putInt("seek", seekList.get(position));
                            Log.d(TAG, "**opening activitygroup groupName:"+groupList.get(position));
                            intent = new Intent(context, ActivityGroup.class);
                            intent.putExtras(bundle);
                        }

//                        if(groupType.equals("room"))
                            context.startActivity(intent);
                    } else if(grpDev.get(position).equals("light")) {

                        /**/
                        Bundle bundle = new Bundle();
//                        if(componentList!=null && groupList!=null ) {
//                            bundle.putInt("id",componentList.get(position-groupList.size()).getId());
//                            LightingService.pushStack(groupList.get(position).getId());
//                        }

                        if(componentList !=null) {
                            bundle.putString("name", list.get(position));
                            bundle.putString("Name", componentList.get(position));
                            bundle.putInt("seek",seekList.get(position));
                        }
                        bundle.putString("groupName", groupName);
                        bundle.putString("groupType", groupType);
                        Intent intent = new Intent(context, ActivityModel.class);
                        intent.putExtras(bundle);
                        /**/
                        context.startActivity(intent);
                    }
                }
            });
        }
        return rowView;
    }
/*
    void popUpDeleteFromGroup(final BLEMeshDevice device){
        Log.d(TAG,"popUpDeleteFromGroup");
        AlertDialog.Builder dialog = new AlertDialog.Builder(context);
        dialog.setCancelable(false);
        dialog.setTitle("Delete From Group");
        dialog.setMessage("Do you want to delete this device from the group?" );
        dialog.setPositiveButton("Delete", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                serviceReference.removeDeviceFromGroup(device, groupId);
            }
        })
                .setNegativeButton("Cancel ", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        //Action for "Cancel".
                    }
                });

        final AlertDialog alert = dialog.create();
        alert.show();
    }
*/
}
