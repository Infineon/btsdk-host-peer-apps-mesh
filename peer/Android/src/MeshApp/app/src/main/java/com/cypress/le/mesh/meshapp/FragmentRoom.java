/*
 * Copyright 2016-2022, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

package com.cypress.le.mesh.meshapp;

import android.accounts.Account;
import android.accounts.AccountManager;
import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.location.LocationManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.IBinder;
import com.google.android.material.floatingactionbutton.FloatingActionButton;
import androidx.fragment.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.Toolbar;

import com.cypress.le.mesh.meshapp.Adapter.NetworkListAdapter;
import com.cypress.le.mesh.meshapp.leotaapp.FileChooser;
import com.cypress.le.mesh.meshframework.IMeshControllerCallback;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.Properties;
import java.util.UUID;

import com.cypress.le.mesh.meshapp.Adapter.GrpDeviceListAdapter;
import com.cypress.le.mesh.meshframework.MeshController;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * A simple {@link Fragment} subclass.
 * Activities that contain this fragment must implement the
 * {@link FragmentRoom.OnFragmentInteractionListener} interface
 * to handle interaction events.
 * Use the {@link FragmentRoom#newInstance} factory method to
 * create an instance of this fragment.
 */
public class FragmentRoom extends Fragment implements LightingService.IServiceCallback, CloudConfigHelper.IHelperCallback{
    private static final String TAG = "FragmentRoom";

    // TODO: Rename parameter arguments, choose names that match
    // the fragment initialization parameters, e.g. ARG_ITEM_NUMBER

    private static final String ARG_PARAM1 = "param1";
    private static final String ARG_PARAM2 = "param2";
    private static final int MENU_LOCATION = 3;
    private static final int MENU_BIG = 2;
    private static final byte DEFAULT_PROXY_TIMEOUT_IN_SEC = 11;
    private MeshController mController;
    private String mAccessKey = null;
    private String mSecretKey = null;
    private String mRestShadowEndpoint = null;
    private String mTopic = null;
    private static boolean mIsApplicationActive = false;
    String EVENT_ID = "mesh";
    String CMD_ID = "meshcmd";
    String DEVICE_TYPE = "meshdevice";
    String DEVICE_ID = "123456";
    String IS_SSL;
    private Boolean READ_PERM_GRANTED = false;
    private static CloudConfigHelper mConfigHelper = new CloudConfigHelper();
    private static Bluemix mBluemixHelper = new Bluemix();
    private static boolean isPrevNetwork = false;
    private static boolean isChangeNetwork = false;
    private static String mNewNetwork = null;
    private static Toast mToast = null;

    String ORG;
    String ID;
    String AUTHMETHOD;
    String AUTHTOKEN;

    String BIGaddress =null;
    Drawable image = null;
    MeshBluetoothDevice mPeerDevice = null;
    private static String BIGNode;
    private static ArrayAdapter<MeshBluetoothDevice> peerDevices;
    int NetworkSelected = -1;
    static String mCurrentNetwork = "NA";

    int menuaddBigItem = 0;
    private static int RESULT_LOAD_IMG = 1;

    ListView ListRooms;
    TextView textView;
    Toolbar toolbar;
//    ProgressDialog progress;
    View v;

    GrpDeviceListAdapter adapterGrplist;
    ArrayList<String> network;
    ArrayList<String> rooms;
    private static MeshApp mApp;

    SharedPreferences sharedpreferences;
    LightingService serviceReference = null;

    public static final String MyPREFERENCES = "MyPrefs" ;
    public static final String location = "locKey";
    public static final String CurrentNetwork = "CurrentNetworkKey";

    private OnFragmentInteractionListener mListener;


    /**
     * Use this factory method to create a new instance of
     * this fragment using the provided parameters.
     *
     * @param param1 Parameter 1.
     * @param param2 Parameter 2.
     * @return A new instance of fragment FragmentRoom.
     */
    // TODO: Rename and change types and number of parameters
    public static FragmentRoom newInstance(String param1, String param2) {
        FragmentRoom fragment = new FragmentRoom();
        Bundle args = new Bundle();
        args.putString(ARG_PARAM1, param1);
        args.putString(ARG_PARAM2, param2);
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onResume() {
        Log.d(TAG, "onResume");
        updateRooms();
        mIsApplicationActive = true;
        if(serviceReference != null)
            serviceReference.registerCb(FragmentRoom.this);
        super.onResume();
    }

    private boolean isServiceConnected() {
        if(serviceReference!=null)
            return true;
        else
            return false;
    }

    @Override
    public void onPause() {
        Log.d(TAG, "onPause");
        mIsApplicationActive = false;
        super.onPause();
    }

    @Override
    public void onDestroy() {

        mIsApplicationActive = false;
        mApp = (MeshApp) getActivity().getApplication();
        getActivity().unbindService(mConnection);
        if(mApp.getCurrNetwork() != null) {
            Log.e(TAG, "calling getCurrentNetwork.close()");
            mApp.getMesh().closeNetwork();
        }

        //clear our global data
        mApp.setMesh(null);
        mApp.setCurrNetwork(null);
        getActivity().unregisterReceiver(mReceiver);
        super.onDestroy();
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        try {
            mListener = (OnFragmentInteractionListener) activity;
        } catch (ClassCastException e) {
            throw new ClassCastException(activity.toString()
                    + " must implement OnFragmentInteractionListener");
        }
    }

    @Override
    public void onDetach() {
        super.onDetach();
        mListener = null;
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {

    }

    @Override
    public void onMeshServiceStatusChangeCb(int status) {

    }

    @Override
    public void onDeviceFound(final UUID uuid, final String name) {
        Log.d(TAG, "onDeviceFound device:" + uuid.toString());

        Log.d(TAG, "onDeviceFound device:" + uuid.toString());

        //TODO: unique entries ??
        getActivity().runOnUiThread(new Runnable() {
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
    public void onProvisionComplete(UUID device, byte status) {
        if(status == 5) {
            Log.d(TAG, "onProvisionComplete");
            show( "Provision complete", Toast.LENGTH_SHORT);
            if(mApp != null && mApp.getBigIP() != null){

                if(mApp.getBigIP().equals("bluemix-iot"))
                    setBluemixCredentials();
            }
//                if(progress != null) {
//                    progress.dismiss();
//                }
        }

    }

    @Override
    public void onHslStateChanged(final String deviceName, final int lightness, final int hue, final int saturation, final int remainingTime) {
        show( "onHslStateChanged : "+" lightness:"+lightness + " hue" + hue + " saturation:" +saturation + " remaining time:" + remainingTime,
                Toast.LENGTH_SHORT);
    }

    @Override
    public void onOnOffStateChanged(final String deviceName, final byte targetOnOff, final byte presentOnOff, final int remainingTime) {
        show("onOnOffStateChanged : "+ targetOnOff, Toast.LENGTH_SHORT);

    }

    @Override
    public void onLevelStateChanged(final String deviceName, final short targetLevel, final short presentLevel, final int remainingTime) {
        show("onLevelStateChanged : "+ targetLevel,
                Toast.LENGTH_SHORT);
    }

    @Override
    public void onNetworkConnectionStatusChanged(byte transport, final byte status) {
        Log.d(TAG,"recieved onNetworkConnectionStatusChanged status = " + status);
        getActivity().runOnUiThread(new Runnable() {

            public void run() {
                String text = null;
                if(status == IMeshControllerCallback.NETWORK_CONNECTION_STATE_CONNECTED)
                    text = "Connected to network";
                if(status == IMeshControllerCallback.NETWORK_CONNECTION_STATE_DISCONNECTED)
                    text = "Disconnected from network";
                if(text != null && mIsApplicationActive){
                  show(text, Toast.LENGTH_SHORT);
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
        Log.d(TAG,"onNodeConnStateChanged in Fragment Room");
/*
        getActivity().runOnUiThread(new Runnable() {

            public void run() {
                switch (status) {
                    case IMeshControllerCallback.MESH_CLIENT_NODE_WARNING_UNREACHABLE:
                        Toast.makeText(getActivity(), "Node" +componentName+" failed to connect ", Toast.LENGTH_SHORT).
                                show();
                        break;

                    case IMeshControllerCallback.MESH_CLIENT_NODE_ERROR_UNREACHABLE:

                        Toast.makeText(getActivity(), "!!! Action Required Node " +componentName+" unreachable", Toast.LENGTH_SHORT).
                                show();
                        break;
                }
            }
        });
 */

    }

    @Override
    public void onOtaStatus(byte status, int percentComplete) {

    }

    @Override
    public void onNetworkOpenedCallback(byte status) {
        Log.d(TAG, "onNetworkOpenedCallback "+status+ " isPrev"+isPrevNetwork+ " ischange"+isChangeNetwork);
        if(status == MeshController.MESH_CLIENT_SUCCESS) {
            show("network opened successfully", Toast.LENGTH_SHORT);

            if(isPrevNetwork) {
                setUpPrevNetwork();
                isPrevNetwork = false;
            } else if(isChangeNetwork){
                setUpChangeNetwork();
                isChangeNetwork = false;
            } else {
                setUpNewNetwork();
            }
        }
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

    public FragmentRoom() {
        // Required empty public constructor
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate start");
        mIsApplicationActive = true;
        Intent intent= new Intent(getActivity(), LightingService.class);
        getActivity().bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
        mConfigHelper.init(getContext());
        mConfigHelper.registerCb(this);
        IntentFilter filter = new IntentFilter();
        filter.addAction(Constants.GATT_PROXY_CONNECTED);
        filter.addAction(Constants.GATT_PROXY_DISCONNECTED);
        filter.addAction(Constants.GENERIC_ON_OFF_STATUS);
        filter.addAction(Constants.SERVICE_CONNECTED);
        filter.addAction(Constants.SERVICE_DISCONNECTED);
        getActivity().registerReceiver(mReceiver, filter);

    }

    @Override
    public View onCreateView(final LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView");
        View view = inflater.inflate(R.layout.fragment_room, container, false);

        mApp = (MeshApp) getActivity().getApplication();
        v=view;
        toolbar = (Toolbar) view.findViewById(R.id.toolbar_top);
        ListRooms= (ListView) view.findViewById(R.id.listView8);
        textView = (TextView) view.findViewById(R.id.textView18);
        // Inflate a menu to be displayed in the toolbar
        toolbar.inflateMenu(R.menu.room_fragment_menu);

        // Set an OnMenuItemClickListener to handle menu item clicks
        toolbar.setOnMenuItemClickListener(new Toolbar.OnMenuItemClickListener() {
            @Override
            public boolean onMenuItemClick(MenuItem item) {
                // Handle the menu item
                Log.d(TAG, "item selected" + item.getItemId());
                switch (item.getItemId()) {
                    case R.id.changeNetwork:
                        View changeview = inflater.inflate(R.layout.network_list, null);
                        popUpChangeNetwork(changeview);
                        break;
                    case R.id.createNetwork:
                        View editview = inflater.inflate(R.layout.pop_up_new_network, null);
                        if (mApp.getPreferredTransport() == Constants.TRANSPORT_GATT) {
                            popUpEdit(editview);
                        }
                        break;
                    case R.id.deleteNetwork:
                        View deleteNtwView = inflater.inflate(R.layout.network_list, null);
                        popUpDeleteNetwork(deleteNtwView);
                        break;
                    case R.id.addBig:

                        if (item.getTitle().equals("Add Bluetooth Internet Gateway")) {
                            menuaddBigItem = item.getItemId();
                            int res = serviceReference.getMesh().disconnectNetwork();
                            showBIGDevicesDialog();
                            //TODO set title after onProv complete for BIG
                        } else if (item.getTitle().equals("Remove Bluetooth Internet Gateway")) {
                            Log.d(TAG, "mApp.getPreferredTransport() = " + mApp.getPreferredTransport());
                            if (mApp.getPreferredTransport() == Constants.TRANSPORT_GATT) {
                                Log.d(TAG, "setting UI to remove big");
//                            BIGNode = serviceReference.getCurrentNetwork().getBIGNode();
//                            serviceReference.deleteBIG(BIGNode);
                                BIGNode = null;
                                BIGaddress = null;
                                item.setTitle("Add Bluetooth Internet Gateway");
                            }
                        }
                        break;
//                case R.id.delete:
//                    Log.d(TAG, "item selected delete rooms");
                    //View deleteView = inflater.inflate(R.layout.pop_up_delete, null);
                    //popUpDeleteRooms(deleteView);
//                    break;

                case R.id.location:
                    if (item.getTitle().equals("Go to Home")) {

                       // mApp.connectToNetwork(Constants.TRANSPORT_GATT);
                        int trans = mApp.getPreferredTransport();
                        if(trans == MeshController.TRANSPORT_IP) {
                            mConfigHelper.disconnectGateway();
                        }

                        show("Disconnecting from Cloud, Connect to network in device setting", Toast.LENGTH_SHORT);

                        Log.d(TAG, "Currently in home");
                        item.setTitle("Go to Away");
                        String ntwrk = serviceReference.getCurrentNetwork();
                        if (ntwrk != null) {
                            textView.setText(ntwrk + " : " + "Home");
                        }
                        setSharedPreferenceLocation("Home");
                        // enable addBig
                        toolbar.getMenu().getItem(2).setEnabled(true);
                    } else {
                        Log.d(TAG, "Currently in away");
                        if (serviceReference.getCurrentNetwork() != null) {
                            mCurrentNetwork = serviceReference.getCurrentNetwork();
                            String BIGip = getNetworkIP();
                            Log.d(TAG, "BIGip" + BIGip);
                            if (BIGip != null) {
                                Log.d(TAG, "Setting transport to REST");
                                toolbar.getMenu().getItem(2).setEnabled(false);
                                mApp.setBigIP(BIGip);
                                show( "Connecting to Cloud", Toast.LENGTH_SHORT);

                                setRestTransport();
                                item.setTitle("Go to Home");
                                Log.d(TAG, "Making text to Home");
                                String ntwrk = serviceReference.getCurrentNetwork();
                                if (ntwrk != null) {
                                    textView.setText(ntwrk + " : " + "Away");
                                }
                                Log.d(TAG, "Making text to Away =" + sharedpreferences.getString(location, "default"));
                                setSharedPreferenceLocation("Away");
                            } else {
                                Log.d(TAG, "currently in away but BIG is not set, Add BIG, changing to go home");
                                show("ADD BIG to network", Toast.LENGTH_SHORT);
                            }
                        }
                    }
                    break;

//                case R.id.menu_network_dfu:
//                    Log.d(TAG, "DFU");
//                    Intent intent;
//                    intent = new Intent(getContext(), com.cypress.le.mesh.meshapp.ActivityDfu.class);
//                    startActivity(intent);
//                    break;
                case R.id.importfile:

                    readFromSdcard();
                    break;
                case R.id.exportfile:
                    View exportview = inflater.inflate(R.layout.network_list, null);
                    popUpExportNetwork(exportview);
                    break;
//                case R.id.closeNtw:
//                    mApp.getMesh().closeNetwork();
//                    break;
                }
                return true;
            }
        });

        FloatingActionButton newRoomButton = (FloatingActionButton) view.findViewById(R.id.fab);
        newRoomButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mApp.getPreferredTransport() == Constants.TRANSPORT_GATT) {
                    if (serviceReference.getCurrentNetwork() != null) {
                        Intent intent = new Intent(getActivity(), NewRoom.class);
                        getActivity().startActivity(intent);
                    } else {
                        show( "Please create/open a network", Toast.LENGTH_SHORT);
                    }
                } else {
                    show("Not supported in away mode", Toast.LENGTH_SHORT);
                }
            }
        });

        return view;
    }

    private String getLocation(){

        String currentloc = "";
        SharedPreferences.Editor editor = sharedpreferences.edit();
        String loc = sharedpreferences.getString(location, "Home");

        Log.d(TAG, "getLocation = " + loc);
        if(loc.equals("Home")) {
            currentloc = "Home";
            Log.d(TAG, "getLocation set Home ");
        } else if(loc.equals("Away")){
            currentloc = "Away";
            Log.d(TAG, "getLocation set Away ");
        } else {
            currentloc = "Home";
            Log.d(TAG, "getLocation set Home ");
        }
        return currentloc;
    }

    private void setRestProperties() {
        String BIGip = getNetworkIP();
        Log.d(TAG, "setRestProperties BIGip =" + BIGip);
        if(BIGip!=null) {

            mApp.setBigIP(BIGip);
            setRestTransport();
        }
    }

    private void setSharedPreferenceLocation(String value){
        SharedPreferences.Editor editor = sharedpreferences.edit();
        editor.putString(location, value);
        editor.commit();
    }
    private void setRestTransport(){
        if(mApp.getBigIP() != null){
            Log.e(TAG, "bigIp = " + mApp.getBigIP());
            if(serviceReference.getMesh().isConnectedToNetwork() && mApp.getPreferredTransport() == Constants.TRANSPORT_GATT)
                serviceReference.getMesh().disconnectNetwork();
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            mConfigHelper.connectRest(mApp.getBigIP());
            mApp.setPreferredTransport(Constants.TRANSPORT_GATEWAY);
            mConfigHelper.connectGateway();
        }
    }
    void popUpEdit(View editView) {
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity(), R.style.AlertDialogCustom);
        final EditText text = (EditText) editView.findViewById(R.id.editText);
        builder.setView(editView);

        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                Log.d(TAG, "User clicked OK button" + text.getText().toString());
                String currentNetwork = serviceReference.getCurrentNetwork();
                final String newNetwork = text.getText().toString();
                if(isNameAlreadyUsed(newNetwork)) {
                    show("NetworkName is already in use",
                            Toast.LENGTH_SHORT);
                } else {
                    if(currentNetwork!=null) {
                        Log.d(TAG, "currentNetwork : "+currentNetwork);
                        int result = serviceReference.getMesh().closeNetwork();
                        mApp.setCurrNetwork(null);
                        Log.d(TAG, "currentNetwork!=null close=" + result + "name =" + currentNetwork);
                    }
                    if(newNetwork != null && !newNetwork.isEmpty()) {

                        final String user = getUsername();
                        Log.d(TAG, "newNetwork : "+newNetwork+" user:"+user);
                        int result = serviceReference.getMesh().createNetwork(user, newNetwork);

                        Thread t = new Thread(new Runnable() {
                            public void run() {
                                if (serviceReference.getMesh().openNetwork(user, newNetwork) != 0) {
                                    Log.e(TAG, "failed to open network!!");
                                    show("failed to open network!!", Toast.LENGTH_SHORT);
                                } else {
                                    mNewNetwork = newNetwork;
                                }
                            }
                        });
                        t.start();
                    } else {
                        show("Please enter network name!!", Toast.LENGTH_SHORT);
                    }
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

    void popUpChangeNetwork(View changeView) {
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity(), R.style.AlertDialogCustom);
        final ListView list = (ListView) changeView.findViewById(R.id.listView3);
        if(mApp != null) {
            Log.d(TAG, "mApp is not null");
            if(serviceReference.getMesh()!=null) {
                Log.d(TAG, "mApp.getMesh() is not null");
                network = serviceReference.getAllNetworks();
            } else {
                Log.d(TAG, "mApp.getMesh() is null");
            }
        } else {
            Log.d(TAG, "mApp is null");
        }

        final NetworkListAdapter adapter= new NetworkListAdapter(getActivity(),network);
        list.setAdapter(adapter);
        builder.setView(changeView);
        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                Log.d(TAG, "User clicked OK button");
                NetworkSelected = adapter.getSelectedRadio();
                if (NetworkSelected == -1) {
                    show("please select a network!!", Toast.LENGTH_SHORT);
                    return;
                }
                String netwrk = network.get(NetworkSelected);
                if (netwrk != null) {
                    String currentNetwork = serviceReference.getCurrNetwork();
                    if (currentNetwork != null) {
                        int result = serviceReference.getMesh().closeNetwork();
                        mApp.setCurrNetwork(null);
                        Log.d(TAG, "currentNetwork!=null result=" + result);
                    } else {
                        Log.d(TAG, "currentNetwork==null");
                    }
                    String user = getUsername();
                    if (serviceReference.getMesh().openNetwork(user,netwrk) != 0) {
                        Log.e(TAG, "failed to open network!!");
                        show("failed to open network!!", Toast.LENGTH_SHORT);
                    } else {
                        mNewNetwork = netwrk;
                       isChangeNetwork = true;
                    }
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
        alert.getWindow().setLayout(ActionBar.LayoutParams.WRAP_CONTENT,
                ActionBar.LayoutParams.WRAP_CONTENT);
    }

    void popUpExportNetwork(View changeView) {
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity(), R.style.AlertDialogCustom);
        final ListView list = (ListView) changeView.findViewById(R.id.listView3);
        if(mApp != null) {
            Log.d(TAG, "mApp is not null");
            if(serviceReference.getMesh()!=null) {
                Log.d(TAG, "mApp.getMesh() is not null");
                network = serviceReference.getAllNetworks();
            } else {
                Log.d(TAG, "mApp.getMesh() is null");
            }
        } else {
            Log.d(TAG, "mApp is null");
        }

        final NetworkListAdapter adapter= new NetworkListAdapter(getActivity(),network);
        list.setAdapter(adapter);
        builder.setView(changeView);
        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                Log.d(TAG, "User clicked OK button");
                NetworkSelected = adapter.getSelectedRadio();
                if (NetworkSelected == -1) {
                    show("please select a network!!", Toast.LENGTH_SHORT);
                    return;
                }
                String netwrk = network.get(NetworkSelected);
                if (netwrk != null) {
                    String jsonString = mApp.getMesh().exportNetwork(netwrk);
                    Log.d(TAG, "json string obtaines : "+jsonString);
                    show("Exporting to : /sdcard/exports", Toast.LENGTH_SHORT);
                    writeToSdcard(netwrk, jsonString);
                    netwrk += ".ifx";
                    jsonString = mApp.getMesh().exportNetwork(netwrk);
                    writeToSdcard(netwrk, jsonString);
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
        alert.getWindow().setLayout(ActionBar.LayoutParams.WRAP_CONTENT,
                ActionBar.LayoutParams.WRAP_CONTENT);
    }

    private void writeToSdcard(String netwrk, String jsonString) {
        File exportDir = new File(Environment.getExternalStorageDirectory()+"/exports/");
// have the object build the directory structure, if needed.
        exportDir.mkdirs();

        File log = new File(Environment.getExternalStorageDirectory()+"/exports/", netwrk+".json");
        Log.e(TAG, "File : "+log.getAbsolutePath());
        try {
//            BufferedWriter out = new BufferedWriter(new FileWriter(log.getAbsolutePath(), false));
//            out.write(jsonString);
//            out.close();

            FileOutputStream fos = new FileOutputStream(log);
            Log.e(TAG, "string : "+jsonString);
            fos.write(jsonString.getBytes());
            fos.close();
        } catch (Exception e) {
            Log.e(TAG, "Error opening Log.", e);
        }
    }

    private void readFromSdcard(){

        FileChooser filechooser = new FileChooser(getActivity());
        filechooser.setFileListener(new FileChooser.FileSelectedListener() {
            @Override
            public void fileSelected(final File file) {
                String filename = file.getPath();
                String ifxfile = filename.substring(0, filename.length() - 4) + "ifx.json";
                String mFileName = file.getAbsolutePath();
                String json = null;
                String ifxjson = null;
                try {
                    FileInputStream is = new FileInputStream(file);
                    int size = is.available();
                    byte[] buffer = new byte[size];
                    is.read(buffer);
                    is.close();
                    json = new String(buffer, "UTF-8");
                    Log.d(TAG,"content : "+json.length() + "actual len:"+size);
                    JSONObject obj = new JSONObject(json);
                    try {
                        is = new FileInputStream(ifxfile);
                        is.read(buffer);
                        is.close();
                        ifxjson = new String(buffer, "UTF-8");
                    }
                    catch (FileNotFoundException e) {
                        ifxjson = new String("");
                    }
                    isChangeNetwork = true;

                    //calling import network API
                    String networkName = mApp.getMesh().importNetwork(getUsername(),json, ifxjson);
                    mNewNetwork = networkName;
                    Log.d(TAG,"import network result : "+networkName );
                    if(networkName!=null)
                        show("Import Successful", Toast.LENGTH_SHORT);
                    else
                        show("Import Failure", Toast.LENGTH_SHORT);

                } catch (IOException ex) {
                    ex.printStackTrace();

                } catch (JSONException e) {
                    e.printStackTrace();
                }

            }
        });

        filechooser.showDialog();

    }



    private void connectToNetwork() {
        if(!isLocationServiceEnabled())
            Log.d(TAG, "isLocationServiceEnabled : location is false");
        else
            Log.d(TAG, "isLocationServiceEnabled : location is true");

        ArrayList<String> groups = serviceReference.getMesh().getAllGroups(mCurrentNetwork);
        Log.d(TAG,"groups"+groups);
        boolean componentFound = false;
        for(int i=0; i<groups.size(); i++)
        {
            Log.d(TAG,"groups ["+i+"]"+groups.get(i));
            ArrayList<String> components = new ArrayList<String>(Arrays.asList(serviceReference.getMesh().getGroupComponents(groups.get(i))));
            if(components!= null && !components.isEmpty()){
                componentFound = true;
                break;
            }
        }
        if(componentFound) {
            serviceReference.getMesh().connectNetwork(DEFAULT_PROXY_TIMEOUT_IN_SEC);
        }
    }


    void popUpDeleteNetwork(View changeView) {
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity(),
                R.style.AlertDialogCustom);
        final ListView list = (ListView) changeView.findViewById(R.id.listView3);
        if(mApp != null) {
            Log.d(TAG, "mApp is not null");
            if(serviceReference.getMesh()!=null) {
                Log.d(TAG, "mApp.getMesh() is not null");
                network = serviceReference.getAllNetworks();
            } else {
                Log.d(TAG, "mApp.getMesh() is null");
            }
        } else {
            Log.d(TAG, "mApp is null");
        }

        final NetworkListAdapter adapter= new NetworkListAdapter(getActivity(),network);
        list.setAdapter(adapter);

        builder.setView(changeView);
        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                Log.d(TAG, "User clicked OK button");
                NetworkSelected = adapter.getSelectedRadio();
                if (NetworkSelected == -1) {
                    Toast.makeText(getActivity(), "please select a network!!",
                            Toast.LENGTH_SHORT).show();
                    return;
                }
                String netwrk = network.get(NetworkSelected);
                String user = getUsername();
                if (netwrk != null && user != null) {
                    String currentNetwork = serviceReference.getCurrNetwork();
                    if (currentNetwork.equals(netwrk)) {
                        int result = serviceReference.getMesh().closeNetwork();
                        mApp.setCurrNetwork(null);
                        Log.d(TAG, "currentNetwork is deleted. result=" + result);
                    } else {
                        Log.d(TAG, "otherNetwork is deleted");
                    }
                    int res = serviceReference.getMesh().deleteNetwork(user,netwrk);
                    if (res == 0) {
                        Log.d(TAG, "Delete network successfully");
                        if (currentNetwork.equals(netwrk)) {
                            mCurrentNetwork = "NA";
                            updateMenu();
                            updateRooms();
                            getActivity().runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    textView.setText("Please create/open a network.");
                                }
                            });
                        }
//                        adapterGrplist = new GrpDeviceListAdapter(serviceReference, getActivity(), null,
//                                null, "room", 0);
//                        adapterGrplist.notifyDataSetChanged();
//                        ListRooms.setAdapter(adapterGrplist);
//                        serviceReference.deleteNetwork(NetworkSelected);
                    }
                    else{
                        Log.d(TAG, "Fail to delete network");
                    }
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


    private void networkChanged() {
        Log.d(TAG, "networkChanged");
        updateMenu();
        updateRooms();
        sharedpreferences = getActivity().getSharedPreferences(MyPREFERENCES, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedpreferences.edit();
        editor.putString(CurrentNetwork, serviceReference.getCurrentNetwork());
        editor.commit();
        if(serviceReference.getCurrentNetwork() != null) {
            String BIGip = getNetworkIP();
            if(BIGip != null) {
                if(BIGip.equals("bluemix-iot")){
                    setBluemixCredentials();
                }
            }
            mApp.setCurrNetwork(serviceReference.getCurrNetwork());

            Log.d(TAG, " changing network BIGip = " + BIGip);
            if(BIGip!=null) {
                toolbar.getMenu().getItem(2).setTitle("Remove Bluetooth Internet Gateway");
            } else {
                toolbar.getMenu().getItem(2).setTitle("Add Bluetooth Internet Gateway");
            }

            textView.setText(serviceReference.getCurrentNetwork() + " : " + getLocation());
//            String device = serviceReference.getCurrentNetwork().getBIGNode();
            String device = "Big Node";
            List<String> groups = serviceReference.getMesh().getAllGroups(serviceReference.getCurrentNetwork());
            Log.d(TAG,"groups size = " +groups.size());
            if(groups.size() != 0){
            Log.d(TAG, " changing network BIgNode = " + device);
            updateRooms();
            }else{
                Log.d(TAG,"groups are null , MeshController UI cannot display Devices ");
            }
        }

        mCurrentNetwork = mNewNetwork;

        if(mApp.getPreferredTransport() == MeshController.TRANSPORT_IP)
        {
            mConfigHelper.connectRest(mApp.getBigIP());
            mConfigHelper.connectGateway();

        }
    }

    private void setBluemixCredentials() {
        try {
            Properties props = readProperties("/sdcard/Bluemix.conf");
            if(props != null) {
                ORG = props.getProperty("org");
                ID = props.getProperty("appid");
                AUTHMETHOD = props.getProperty("key");
                AUTHTOKEN = props.getProperty("token");
                //isSSL property
                IS_SSL = props.getProperty("isSSL");
                boolean is_SSL = false;
                if (IS_SSL.equals("T")) {
                    is_SSL = true;
                }
                EVENT_ID = props.getProperty("eventid");
                CMD_ID = props.getProperty("commandid");
                DEVICE_TYPE = props.getProperty("devicetype");
                DEVICE_ID = props.getProperty("deviceid");

                mBluemixHelper.setBluemixCredentials(ORG, ID, AUTHMETHOD, AUTHTOKEN,
                        is_SSL, EVENT_ID, CMD_ID, DEVICE_TYPE, DEVICE_ID);
            } else {
                Log.d(TAG,"Cannot read Bluemix Credentials");
            }

        } catch (Exception e) {
            Log.d(TAG,"Cannot read Bluemix Credentials");
            e.printStackTrace();
        }

    }


    @Override
    public void onRequestPermissionsResult(int permsRequestCode, String[] permissions, int[] grantResults){
        Log.d(TAG, "onRequestPermissionsResult");
        switch(permsRequestCode){

            case 200:

                boolean readAccepted = grantResults[0]== PackageManager.PERMISSION_GRANTED;
                if(readAccepted) {
                    READ_PERM_GRANTED = true;
                    Log.d(TAG, "onRequestPermissionsResult : Permission granted");
                } else {
                    READ_PERM_GRANTED = false;
                    Log.d(TAG, "onRequestPermissionsResult : Permission denied");
                }
                break;

        }

    }
    public static Properties readProperties(String filePath) {
        Properties props = new Properties();
        try {
            InputStream in = new BufferedInputStream(new FileInputStream(
                    filePath));
            props.load(in);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return null;
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
        return props;
    }
/*
    void popUpDeleteRooms(View deleteView) {
        Log.d(TAG, "popUpDeleteRooms");

        final SelectListAdapter adapter = new SelectListAdapter(deleteView.getContext(),null,rooms);

        ListView listView = (ListView) deleteView.findViewById(R.id.listView10);
        listView.setAdapter(adapter);

        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity(), R.style.AlertDialogCustom);
        builder.setView(deleteView);
        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                HashMap<Integer, Boolean> chkList = adapter.getCheckboxlist();

                rooms = (ArrayList) serviceReference.getallRooms();

                for (int i = 0; i < rooms.size(); i++) {
                    if (chkList.get(i) == true) {

                        String name = rooms.get(i).getName();
                        Log.d(TAG, "chkList.get(i) selected name = " + name);
                        boolean result = serviceReference.deleteGroup(rooms.get(i));
                        if (result) {
                            Log.d(TAG, "Deleted room successfully" + name);
                        } else
                            Log.d(TAG, "Delete room unsuccessful" + name);
                    }
                }
//                 delete from the list
                    rooms = (ArrayList<BLEMeshGroup>) serviceReference.getallRooms();
                    adapterGrplist = new GrpDeviceListAdapter(serviceReference, getActivity(), null, rooms, "room", 0);
                    adapterGrplist.notifyDataSetChanged();
                    ListRooms.setAdapter(adapterGrplist);
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

    // TODO: Rename method, update argument and hook method into UI event
    public void onButtonPressed(Uri uri) {
        if (mListener != null) {
            mListener.onFragmentInteraction(uri);
        }
    }

    public String getNetworkIP(){
        JSONObject ntwObj = getNetworkDetails(mCurrentNetwork);
        String gatewayType = null;
        if(ntwObj != null) {
            try {
                gatewayType = ntwObj.getString("gatewayType");
                String networkName = ntwObj.getString("name");
                String gatewayIP = ntwObj.getString("gatewayIP");
                if(gatewayType.equals("REST"))
                    return  gatewayIP;
                else
                    return gatewayType;
            } catch (JSONException e) {
                e.printStackTrace();
            }
        }
        Log.d(TAG, "network obj is null");
        return  null;
    }

    public static void sendData(byte[] data, int length) {
        Log.d(TAG,"sendData len"+length);
        if(mCurrentNetwork != null) {
            String BIGip = mApp.getBigIP();
            if(BIGip != null) {
                if(BIGip.equals("aws-iot")) {
                    Log.d(TAG,"AWS");
                    mConfigHelper.send(data);
                } else if(BIGip.equals("bluemix-iot")){
                    Log.d(TAG,"Bluemix");
                    mBluemixHelper.sendViaBluemix(data);
                } else {
                    Log.d(TAG,"Rest");
                    mConfigHelper.send(data);
                }
            } else {
                Log.d(TAG,"BIGIP is null");
            }
        } else {
            Log.d(TAG,"network is null");
        }
    }

    @Override
    public void onConnectionStateChange(boolean state) {
        Log.d(TAG,"onConnectionStateChange state:"+state);
            serviceReference.getMesh().networkConnectionChanged(state== true?0xFFFF:0);
        if(!state) {
          //  serviceReference.getMesh().disconnectNetwork((byte) MeshController.TRANSPORT_GATEWAY);
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            mApp.connectToNetwork(Constants.TRANSPORT_GATT);
        }

    }

    @Override
    public void onDataReceived(byte[] data) {
        Log.d(TAG,"onDataReceived ");
      serviceReference.getMesh().sendReceivedProxyPacket(data);
    }

    /**
     * This interface must be implemented by activities that contain this
     * fragment to allow an interaction in this fragment to be communicated
     * to the activity and potentially other fragments contained in that
     * activity.
     * <p/>
     * See the Android Training lesson <a href=
     * "http://developer.android.com/training/basics/fragments/communicating.html"
     * >Communicating with Other Fragments</a> for more information.
     */
    public interface OnFragmentInteractionListener {
        // TODO: Update argument type and name
        public void onFragmentInteraction(Uri uri);
    }

    private void showBIGDevicesDialog() {

        LayoutInflater inflater = getActivity().getLayoutInflater();
        View scanDevView = inflater.inflate(R.layout.pop_up_add_big, null);
        final EditText text = (EditText)scanDevView.findViewById(R.id.editText4);
        final TextView textHeader = (TextView)scanDevView.findViewById(R.id.textView39);
        final RadioButton aws = (RadioButton)scanDevView.findViewById(R.id.aws);
        final RadioButton bluemix = (RadioButton)scanDevView.findViewById(R.id.bluemix);
        final RadioButton rest = (RadioButton)scanDevView.findViewById(R.id.bluemix);
        final RadioGroup radioGroup = (RadioGroup) scanDevView.findViewById(R.id.myRadioGroup);
        final View view = (View)scanDevView.findViewById(R.id.ip);

        final String[] ip = new String[1];
        final ListView lvPeerDevices = (ListView) scanDevView.findViewById(R.id.listView6);
        String IP;
        List<MeshBluetoothDevice> listDevices = new ArrayList<MeshBluetoothDevice>();
        peerDevices = new ArrayAdapter<MeshBluetoothDevice>(getActivity(), R.layout.node_list_item, listDevices);
        lvPeerDevices.setAdapter(peerDevices);
        lvPeerDevices.setChoiceMode(AbsListView.CHOICE_MODE_SINGLE);
        if(lvPeerDevices.getCount() > 0)
            lvPeerDevices.setItemChecked(0, true);

        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity(), AlertDialog.THEME_HOLO_LIGHT);
        builder.setView(scanDevView);

        radioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                // find which radio button is selected
                if (checkedId == R.id.aws) {
                    view.setVisibility(View.GONE);
                } else if (checkedId == R.id.bluemix) {
                    view.setVisibility(View.GONE);
                } else if (checkedId == R.id.rest) {
                    view.setVisibility(View.VISIBLE);
                }
            }

        });


        startScan();
        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                Log.d(TAG, "showBIGDevicesDialog User clicked OK button");
                //do what user asked for
                BIGaddress = text.getText().toString();

                if(aws.isChecked()) {
                    BIGaddress = "aws-iot";
                    saveNetworkDetails(mCurrentNetwork, BIGaddress, "");
                }
                if(bluemix.isChecked()) {
                    BIGaddress = "bluemix-iot";
                    saveNetworkDetails(mCurrentNetwork, BIGaddress, "");
                }
                if(!BIGaddress.equals("")) {
                    saveNetworkDetails(mCurrentNetwork, "REST", BIGaddress);
                    int itemPos = lvPeerDevices.getCheckedItemPosition();
                    Log.d(TAG, "showBIGDevicesDialog showScanDevicesDialog itemPos: " + itemPos
                            + " adapter count:" + lvPeerDevices.getAdapter().getCount() + " address = " + BIGaddress);
                    if (itemPos != -1) {
                        mPeerDevice = (MeshBluetoothDevice) lvPeerDevices.getAdapter().getItem(itemPos);
                        Log.d(TAG, "showBIGDevicesDialog  for node " + " BluetoothDevice:" + mPeerDevice);
                        //Create new node
                        BIGNode = "BIGNode";

                        if (BIGNode != null) {
                            Log.d(TAG, "showBIGDevicesDialog create new node success, " + BIGNode + BIGNode);
                            if(serviceReference.isConnectedToNetwork()) {
                                Log.d(TAG, "showBIGDevicesDialog proxy connected");

                            } else {
                                Log.d(TAG, "showBIGDevicesDialog proxy is not connected");
                            }
                            if(mPeerDevice != null) {
                                mApp.setBigIP(BIGaddress);
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

                                serviceReference.getMesh().provision("BIG",mCurrentNetwork, mPeerDevice.mUUID, (byte)10);

                              //  startSpin();
                                toolbar.getMenu().getItem(2).setTitle("Remove Bluetooth Internet Gateway");
                            } else{
                                Log.d(TAG, "Please select device");
                            }
                        }


                    }
                } else {
                    Log.d(TAG, "Please set IP address");

                }

                dialog.dismiss();
            }
        });

        builder.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                Log.d(TAG, "User cancelled the dialog");
                stopScan();
            }
        });

        builder.setOnDismissListener(new DialogInterface.OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface dialog) {
                Log.d(TAG, "User dismissed the dialog");
                //stopScan();
            }
        });

        AlertDialog alert = builder.create();
        alert.show();
    }
    private void startScan() {
        Log.d(TAG, "startScan");
        if(!isLocationServiceEnabled())
            Log.d(TAG, "isLocationServiceEnabled : location is false");
        else
            Log.d(TAG, "isLocationServiceEnabled : location is true");
        if(serviceReference.getMesh() != null)
            serviceReference.getMesh().scanMeshDevices(true, null);
    }

    private void stopScan() {
        Log.d(TAG, "stopScan");

        if(serviceReference.getMesh() != null)
            serviceReference.getMesh().scanMeshDevices(false, null);
    }
//
//    void startSpin() {
//
//        getActivity().runOnUiThread(new Runnable() {
//            @Override
//            public void run() {
//                progress = new ProgressDialog(getActivity());
//                progress.setTitle("Provision");
//                progress.setMessage("provision in Progress...");
//                progress.setCancelable(false);
//                progress.show();
//
//            }
//        });
//
//    }

    private ServiceConnection mConnection= new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "bound service connected");
            LightingService.MyBinder binder = (LightingService.MyBinder) service;
            serviceReference = binder.getService();
            serviceReference.registerCb(FragmentRoom.this);
            mConfigHelper.registerCb(FragmentRoom.this);

            MeshController mesh =serviceReference.initializeMesh();
            mApp.setMesh(mesh);
//            Runnable runnable = new Runnable() {
//                @Override
//                public void run() {
//                    openPrevNetwork();
//                }
//            };
//
//            Thread thread = new Thread(runnable);
//            thread.start();

        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            serviceReference = null;
        }
    };

    private void updateMenu() {
        if(serviceReference.getCurrentNetwork()!=null) {
            String BIGip = getNetworkIP();
            Log.d(TAG, " changing network BIGip = " + BIGip);
            if(BIGip!=null)
                toolbar.getMenu().getItem(2).setTitle("Remove Bluetooth Internet Gateway");

            BIGNode = "BIGNode";
            Log.d(TAG, " changing network BIgNode = " + BIGNode);
        }
        sharedpreferences = getActivity().getSharedPreferences(MyPREFERENCES, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedpreferences.edit();
        String loc = sharedpreferences.getString(location, "Home");

        if(loc.equals("Home")) {
            Log.d(TAG, "in home enabling add/remove Big shardpref =" + loc);

            toolbar.getMenu().getItem(MENU_LOCATION).setTitle("Go to Away");
            toolbar.getMenu().getItem(MENU_BIG).setEnabled(true);

            String ntwrk = serviceReference.getCurrentNetwork();
            if(ntwrk!=null) {
               // mApp.connectToNetwork(Constants.TRANSPORT_GATT);
                textView.setAllCaps(true);
                textView.setText(ntwrk+" : " + getLocation());
            }
            if(image != null) {
                v.setBackground(image);
            }
            if(serviceReference.getCurrentNetwork()!=null) {
                textView.setText(ntwrk+" : " + "Home");
            }
        } else if (loc.equals("Away")) {
            Log.d(TAG, "away from home : disabling add/remove Big shared pref =" + loc);

            toolbar.getMenu().getItem(MENU_LOCATION).setTitle("Go to Home");
            toolbar.getMenu().getItem(MENU_BIG).setEnabled(false);
            mApp.setBigIP(getNetworkIP());
            if(serviceReference.getCurrentNetwork()!=null) {
                setRestProperties();
                textView.setText(serviceReference.getCurrentNetwork() +" : "+ "Away");
            }

        } else {
            Log.d(TAG, "shared pref is not set shardpref =" + loc);
            mApp.connectToNetwork(Constants.TRANSPORT_GATT);
            toolbar.getMenu().getItem(2).setEnabled(true);
            toolbar.getMenu().getItem(3).setTitle("Go to Away");
            if(serviceReference.getCurrentNetwork()!=null) {
                textView.setText(serviceReference.getCurrentNetwork() +" : "+ "Home");
            }
        }
    }

    private void openPrevNetwork() {
        Log.e(TAG, "openPrevNetwork");
        sharedpreferences = getActivity().getSharedPreferences(MyPREFERENCES, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedpreferences.edit();
        mCurrentNetwork = sharedpreferences.getString(CurrentNetwork,"NA");
        String loc = sharedpreferences.getString(location, "Home");
        editor.commit();
        if(mApp!=null){
            Log.e(TAG, "mApp is !null network : "+mCurrentNetwork);
            serviceReference.setMeshApp(mApp);
            serviceReference.setMesh(mApp.getMesh());
            mApp.setMesh(serviceReference.getMesh());
        } else {
            Log.e(TAG, "mApp is null");
        }

        if(serviceReference.getMesh()!=null) {
            Log.e(TAG, "getmesh() is !null");
            network = (ArrayList)serviceReference.getAllNetworks();
        } else {
            Log.d(TAG, "mesh is null");
        }

        if(network!=null) {
            if(mCurrentNetwork!=null) {
                String username = getUsername();
                if (serviceReference.getMesh().openNetwork(username, mCurrentNetwork) != 0) {
                    Log.e(TAG, "failed to open network!!");
                    show("failed to open network!!", Toast.LENGTH_SHORT);
                } else {
                    isPrevNetwork = true;
                }
            }
        }
    }

    void setUpChangeNetwork() {
        show("setUpChangeNetwork open network successful", Toast.LENGTH_SHORT);
        Boolean result = serviceReference.changeNetwork(mNewNetwork);
        Log.d(TAG, "changeNetwork result=" + result);
        networkChanged();
    }

    void setUpNewNetwork() {
        serviceReference.addNetworkToList(mNewNetwork);
        Log.d(TAG, "setUpNewNetwork network opened successfully!!");

        serviceReference.changeNetwork(mNewNetwork);
        networkChanged();
    }

    void setUpPrevNetwork() {
        Log.e(TAG, "setUpPrevNetwork successful to open network!!" + mCurrentNetwork);
        sharedpreferences = getActivity().getSharedPreferences(MyPREFERENCES, Context.MODE_PRIVATE);
        serviceReference.changeNetwork(mCurrentNetwork);
        SharedPreferences.Editor editor = sharedpreferences.edit();
        editor = sharedpreferences.edit();
        editor.putString(CurrentNetwork, mCurrentNetwork);
        editor.commit();
        mApp.setCurrNetwork(mCurrentNetwork);
        List<String> groups = serviceReference.getMesh().getAllGroups(mCurrentNetwork);
        if(groups != null && groups.size() > 0) {
            String ntwrk = serviceReference.getCurrentNetwork();
            if(ntwrk!=null) {
                textView.setAllCaps(true);
                textView.setText(ntwrk + " : " + getLocation());
            }
            if(image != null) {
                v.setBackground(image);
            }

            //set cloud details
            if(ntwrk != null) {
                String BIGip = getNetworkIP();
                if(BIGip != null) {
                    if(BIGip.equals("bluemix-iot")){
                        setBluemixCredentials();
                    }
                }
            }
            networkChanged();
        }
    }


    private boolean isNameAlreadyUsed(String name) {
        if(serviceReference.getMesh()!=null) {
            Log.d(TAG, "mApp.getMesh() is not null");
            network = (ArrayList)serviceReference.getAllNetworks();
        } else {
            Log.d(TAG, "mApp.getMesh() is null");
        }
        Boolean found = false;

        for(int i =0 ;i <network.size(); i++) {
            Log.d(TAG, "network name = " + network.get(i) + "current network name=" + name);
            if(network.get(i).equals(name)) {
                Log.d(TAG, "network name found");
                found = true;
            }
        }
        return found;
    }
    private void updateRooms() {
        if (isServiceConnected()) {
            rooms = (ArrayList)serviceReference.getallRooms();

            Log.d(TAG, "rooms =" + rooms);
            adapterGrplist = new GrpDeviceListAdapter(serviceReference,getActivity(), null, null, rooms, "room" ,"NA");
            adapterGrplist.notifyDataSetChanged();
            ListRooms.setAdapter(adapterGrplist);
        }
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, final Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "Received intent: " + action);
           if(action.equals(Constants.GATT_PROXY_CONNECTED)){
               Log.d(TAG, Constants.GATT_PROXY_CONNECTED);
               if(getActivity()!=null) {
                   show("proxy is connected", Toast.LENGTH_SHORT);
               }
           } else if(action.equals(Constants.GATT_PROXY_DISCONNECTED)) {
               Log.d(TAG, Constants.GATT_PROXY_DISCONNECTED);
               show("proxy disconnected", Toast.LENGTH_SHORT);
            } else if(action.equals(Constants.GENERIC_ON_OFF_STATUS)) {
               Log.d(TAG, Constants.GENERIC_ON_OFF_STATUS);

           } else if(action.equals(Constants.SERVICE_CONNECTED)) {
               openPrevNetwork();
               updateMenu();
               updateRooms();
           }  else if (action.equals(Constants.SERVICE_DISCONNECTED)) {

           }
        }
    };

    public String getUsername() {
        AccountManager manager = AccountManager.get(getActivity());
        Account[] accounts = manager.getAccountsByType("com.google");
        List<String> possibleEmails = new LinkedList<String>();

        for (Account account : accounts) {
            // TODO: Check possibleEmail against an email regex or treat
            // account.name as an email address only for certain account.type values.
            possibleEmails.add(account.name);
        }

        if (!possibleEmails.isEmpty() && possibleEmails.get(0) != null) {
            String email = possibleEmails.get(0);
            String[] parts = email.split("@");

            if (parts.length > 1)
                return parts[0];
        }
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();

        String username = adapter.getName();
        username = username+adapter.getAddress();
        return username;
    }


    JSONObject getNetworkDetails(String ntwName) {
        try {
            String data = getContext().getSharedPreferences("NETWORK_DATA" , 0).getString("savedData", null);
            Log.d(TAG,"getNetworkDetails preference : "+data);
            if(data != null) {
                JSONArray arr = new JSONArray(data);
                for(int i = 0; i < arr.length(); i++) {
                    JSONObject json = arr.getJSONObject(i);
                    if(json.getString("name").equals(ntwName))
                        return json;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    void saveNetworkDetails(String ntwName, String gatewayType, String gatewayIP) {
        Log.d(TAG, "saveNetworkDetails");
        try {
            int i;
            String data = getContext().getSharedPreferences("NETWORK_DATA" , 0).getString("savedData", null);

            if(data != null) {
                JSONArray arr = new JSONArray(data);
                for(i = 0; i < arr.length(); i++) {
                    JSONObject json = arr.getJSONObject(i);
                    if(json.getString("name").equals(ntwName)) {
                        break;
                    }
                }
                if(i < arr.length()) {
                    arr.remove(i);
                    JSONObject json = new JSONObject();
                    json.put("gatewayType", gatewayType);
                    json.put("name", ntwName);
                    json.put("gatewayIP", gatewayIP);
                    arr.put(json);
                } else {
                    JSONObject json = new JSONObject();
                    json.put("gatewayType", gatewayType);
                    json.put("name", ntwName);
                    json.put("gatewayIP", gatewayIP);
                    arr.put(json);
                }
                Log.d(TAG,"saving preference : "+arr.toString());
                getContext().getSharedPreferences("NETWORK_DATA",0).edit().putString("savedData", arr.toString()).apply();
            } else {
                JSONArray arr = new JSONArray();
                JSONObject json = new JSONObject();
                json.put("gatewayType", gatewayType);
                json.put("name", ntwName);
                json.put("gatewayIP", gatewayIP);
                arr.put(json);
                getContext().getSharedPreferences("NETWORK_DATA",0).edit().putString("savedData", arr.toString()).apply();
            }

        } catch (JSONException exception) {
            // Do something with exception
        }
    }

    public void show(final String text,final int duration) {
        getActivity().runOnUiThread(new Runnable() {

            public void run() {
                if (mToast == null || !mToast.getView().isShown()) {
                    if (mToast != null) {
                        mToast.cancel();
                    }
                }
                mToast = Toast.makeText(getActivity().getApplicationContext(), text, duration);
                mToast.show();
            }
        });

    }

    public boolean isLocationServiceEnabled(){
        LocationManager locationManager = null;
        boolean gps_enabled= false,network_enabled = false;

        if(locationManager ==null)
            locationManager = (LocationManager) getActivity().getSystemService(Context.LOCATION_SERVICE);
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
