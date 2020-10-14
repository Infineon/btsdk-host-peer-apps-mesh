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
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.ImageButton;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.HashMap;

import com.cypress.le.mesh.meshapp.R;

public class SelectListAdapter extends BaseAdapter {
    private static final String TAG = "SelectListAdapter";

    private final Context context;
    private HashMap<Integer,Boolean> checklist = new HashMap<Integer,Boolean>();
    private ArrayList<String> lightGroupList = new ArrayList<String>();
    private ArrayList<String> values = new ArrayList<String>();
    private ArrayList<Drawable> image = new ArrayList<Drawable>();
    private ArrayList<String> list = new ArrayList<String>();
    public SelectListAdapter(Context context, ArrayList<String> lightList, ArrayList<String> groupList) {
        this.context = context;
        int i;
        if(groupList != null)
        for(i=0; i<groupList.size();i++) {
            lightGroupList.add("group");
            list.add(groupList.get(i));
            checklist.put(i,false);
        }
        if(lightList != null)
        for(i=0; i<lightList.size();i++) {
            lightGroupList.add("light");
            list.add(lightList.get(i));
            if(groupList != null)
                checklist.put(i + groupList.size(),false);
        }
        Log.d(TAG, "SelectListAdapter");
        LayoutInflater inflater = (LayoutInflater) context
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    @Override
    public int getCount() {
        return lightGroupList.size();
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
        checklist.put(position,false);
        LayoutInflater inflater = (LayoutInflater) context
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View rowView = inflater.inflate(R.layout.adapter_select_light, null);

        if(position == 0 && list.isEmpty()) return rowView;
        final ImageButton imagebutton = (ImageButton) rowView.findViewById(R.id.imageButton);
        TextView textView = (TextView) rowView.findViewById(R.id.DevGrpName);

        if(lightGroupList.get(position).equals("group")) {

            imagebutton.setImageResource(R.mipmap.bulbs_red);

        } else if(lightGroupList.get(position).equals("light")){

            imagebutton.setImageResource(R.mipmap.bulb_red);

        } else if(lightGroupList.get(position).equals("activity_scene")) {

            imagebutton.setImageResource(R.drawable.scene1);

        } else if(lightGroupList.get(position).equals("room")) {

            imagebutton.setImageResource(R.drawable.location);

        }

        CheckBox box = (CheckBox) rowView.findViewById(R.id.checkBox2);
        box.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                checklist.put(position,true);
            }
        });
        textView.setText(list.get(position));
        return rowView;
    }
    public HashMap<Integer,Boolean> getCheckboxlist(){
        return checklist;
    }
}
