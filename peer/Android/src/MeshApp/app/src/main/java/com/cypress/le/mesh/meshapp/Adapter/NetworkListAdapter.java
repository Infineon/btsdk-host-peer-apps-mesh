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
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.RadioButton;
import android.widget.TextView;

import com.cypress.le.mesh.meshapp.R;

import java.util.ArrayList;

public class NetworkListAdapter extends BaseAdapter {
    private static final String TAG = "NetworkListAdapter";

    private final Context context;
    private int selectedRadio =-1;
    private ArrayList<String> Networks = new ArrayList<String>();

    public NetworkListAdapter(Context context, ArrayList<String> NetwrkArry) {

        this.context = context;
        Networks = NetwrkArry;

        Log.d(TAG, "NetworkListAdapter");
        LayoutInflater inflater = (LayoutInflater) context
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    @Override
    public int getCount() {
        return Networks.size();
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
        Log.d(TAG, "Value = " + Networks.get(position));

        LayoutInflater inflater = (LayoutInflater) context
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View rowView = inflater.inflate(R.layout.adapter_network_list_item, null);
        if(position == 0 && Networks.isEmpty()) return rowView;

        TextView name = (TextView) rowView.findViewById(R.id.name);
        name.setText(Networks.get(position));
        final RadioButton radio = (RadioButton) rowView.findViewById(R.id.checkBox3);
        radio.setChecked(selectedRadio == position);

       rowView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                selectedRadio = position;
                notifyDataSetChanged();
                radio.setChecked(selectedRadio == position);
            }
        });

         return rowView;
    }

    public int getSelectedRadio() {
        return selectedRadio;
    }
}
