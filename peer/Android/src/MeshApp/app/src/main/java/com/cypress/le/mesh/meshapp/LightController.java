package com.cypress.le.mesh.meshapp;

import android.app.AlertDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.Spinner;
import android.widget.Toast;

import com.cypress.le.mesh.meshframework.MeshController;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

public class LightController extends AppCompatActivity implements View.OnClickListener, LightingService.IServiceCallback {

    private static final String TAG = "LightController";
    String mComponentName = null;
    Button lcOn = null;
    Button lcOff = null;
    Button lightlcOn = null;
    Button lightlcOff = null;
    Button lightlcOccupancyOn = null;
    Button lightlcOccupancyOff = null;
    Button propertyGet = null;
    Button propertySet = null;
    Button LcModeGet = null;
    Button LcOccupancyModeGet = null;
    EditText propertyVal = null;
    int mLcPopertyId = 0;
    int mLcPropertyVal = 0;
    String mPropertyValue = null;
    private static Toast mToast = null;
    ArrayList<LightLcProp> propArray = new ArrayList<LightLcProp>();
    static LightingService serviceReference = null;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_light_controller);
        ActionBar toolbar = getSupportActionBar();
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setTitle("Light Controller");

        final Intent serviceIntent= new Intent(this, LightingService.class);
        bindService(serviceIntent, mConnection, Context.BIND_AUTO_CREATE);
        Intent intent = getIntent();

        if(intent!=null) {
            Bundle extras = getIntent().getExtras();
            if(extras != null) {
                mComponentName = extras.getString("mComponentName");
            } else {
                Log.d(TAG, "Extras are null");
            }
        } else {
            Log.d(TAG, "Intent is null");
        }

        lcOn = (Button) findViewById(R.id.lcOn);
        lcOff = (Button) findViewById(R.id.lcOff);
        lightlcOn = (Button) findViewById(R.id.lightLcOn);
        lightlcOff = (Button) findViewById(R.id.lightLcOff);
        lightlcOccupancyOn = (Button) findViewById(R.id.occupancyOn);
        lightlcOccupancyOff = (Button) findViewById(R.id.occupancyOff);
        propertyGet = (Button) findViewById(R.id.PropertyGet);
        propertySet = (Button) findViewById(R.id.PropertySet);
        LcModeGet = (Button) findViewById(R.id.lightlcget);
        LcOccupancyModeGet = (Button) findViewById(R.id.occupancyGet);
        propertyVal = (EditText) findViewById(R.id.LightLCPropertyValue);

        lcOn.setOnClickListener(this);
        lcOff.setOnClickListener(this);
        lightlcOn.setOnClickListener(this);
        lightlcOff.setOnClickListener(this);
        lightlcOccupancyOn.setOnClickListener(this);
        lightlcOccupancyOff.setOnClickListener(this);
        propertyGet.setOnClickListener(this);
        propertySet.setOnClickListener(this);
        LcOccupancyModeGet.setOnClickListener(this);
        LcModeGet.setOnClickListener(this);
        final Spinner propertySpinner = (Spinner) findViewById(R.id.lightLcPropertyId);

        propertySpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {

                String type = (String)propertySpinner.getItemAtPosition(position);
                Log.d(TAG," val :"+type);
                for(int i=0; i < propArray.size(); i++)
                {
                    if (type.equals(propArray.get(i).PropName))
                    {
                        mLcPopertyId = propArray.get(i).PropId;
                        if (!propertyVal.getText().toString().isEmpty())
                        {
                            mLcPropertyVal = Integer.parseInt(propertyVal.getText().toString());
                        }
                    }
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        fillPropertyList();
    }

    @Override
    protected void onResume() {
        if(serviceReference != null) {
            serviceReference.registerCb(LightController.this);
        }
        super.onResume();
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
    protected void onDestroy() {
        unbindService(mConnection);
        super.onDestroy();
    }

    private ServiceConnection mConnection= new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "bound service connected");
            LightingService.MyBinder binder = (LightingService.MyBinder) service;
            serviceReference = binder.getService();
            serviceReference.registerCb(LightController.this);
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "bound service disconnected");
            serviceReference = null;
        }
    };



    @Override
    public void onClick(View v) {
        Log.d(TAG,"onClick"+v.getId()+ " prop"+propertyGet.getId());

        if (v.getId() == lcOn.getId())
        {
            Thread t = new Thread(new Runnable() {
                public void run() {
                    serviceReference.getMesh().setLightLcMode(mComponentName, MeshController.MESH_CLIENT_LC_MODE_ON);
                }
            });
            t.start();
        }
        if (v.getId() == lcOff.getId())
        {
            Thread t = new Thread(new Runnable() {
                public void run() {
                    serviceReference.getMesh().setLightLcMode(mComponentName, MeshController.MESH_CLIENT_LC_MODE_OFF);

                }
            });
            t.start();
        }
        if ( v.getId() == lightlcOn.getId())
        {
            Thread t = new Thread(new Runnable() {
                public void run() {
                    serviceReference.getMesh().setLightLcOnOff(mComponentName, (byte) 1, true,Constants.DEFAULT_TRANSITION_TIME,0);
                }
            });
            t.start();
        }
        if (v.getId() == lightlcOff.getId())
        {
            Thread t = new Thread(new Runnable() {
                public void run() {
                    serviceReference.getMesh().setLightLcOnOff(mComponentName, (byte) 0, true,Constants.DEFAULT_TRANSITION_TIME,0);
                }
            });
            t.start();
        }
        if (v.getId() == lightlcOccupancyOn.getId())
        {
            Thread t = new Thread(new Runnable() {
                public void run() {
                    serviceReference.getMesh().setLightLcOccupancyMode(mComponentName, MeshController.MESH_CLIENT_LC_OCCUPANCY_MODE_ON);
                }
            });
            t.start();
        }
        if (v.getId() == lightlcOccupancyOff.getId())
        {
            Thread t = new Thread(new Runnable() {
                public void run() {
                    serviceReference.getMesh().setLightLcOccupancyMode(mComponentName, MeshController.MESH_CLIENT_LC_OCCUPANCY_MODE_OFF);
                }
            });
            t.start();
        }
        if (v.getId() == propertyGet.getId())
        {
            Log.d(TAG,"sending property get" + mLcPopertyId);
            Thread t = new Thread(new Runnable() {
                public void run() {
                    serviceReference.getMesh().getLightLcProperty(mComponentName, mLcPopertyId);
                }
            });
            t.start();
        }
        if (v.getId() == propertySet.getId())
        {
            Log.d(TAG,"sending property set"+Integer.parseInt(propertyVal.getText().toString()));
            Thread t = new Thread(new Runnable() {
                public void run() {
                    serviceReference.getMesh().setLightLcProperty(mComponentName, mLcPopertyId, Integer.parseInt(propertyVal.getText().toString()));
                }
            });
            t.start();
        }
        if (v.getId() == LcOccupancyModeGet.getId())
        {
            Log.d(TAG,"sending LcOccupancyModeGet"+mPropertyValue);
            Thread t = new Thread(new Runnable() {
                public void run() {
                    serviceReference.getMesh().getLightLcOccupancyMode(mComponentName);
                }
            });
            t.start();
        }
        if (v.getId() == LcModeGet.getId())
        {
            Log.d(TAG,"sending LcModeGet set"+mPropertyValue);
            Thread t = new Thread(new Runnable() {
                public void run() {
                    serviceReference.getMesh().getLightLcMode(mComponentName);
                }
            });
            t.start();
        }
    }

    void fillPropertyList() {
        propArray.add(new LightLcProp("Time Occupancy Delay (0x3a)", 0x3a, 3));
        propArray.add(new LightLcProp("Time Fade On (0x37)", 0x37, 3));
        propArray.add(new LightLcProp("Time Run On (0x3c)", 0x3c, 3));
        propArray.add(new LightLcProp("Time Fade (0x36)", 0x36, 3));
        propArray.add(new LightLcProp("Time Prolong (0x3b)", 0x3b, 3));
        propArray.add(new LightLcProp("Time Fade Standby Auto (0x38)", 0x38, 3));
        propArray.add(new LightLcProp("Time Fade Standby Manual (0x39)", 0x39, 3));
        propArray.add(new LightLcProp("Lightness On (0x2e)", 0x2e, 2));
        propArray.add(new LightLcProp("Lightness Prolong (0x2f)", 0x2f, 2));
        propArray.add(new LightLcProp("Lightness Standby (0x30)", 0x39, 2));
        propArray.add(new LightLcProp("Ambient LuxLevel On (0x2b)", 0x2b, 2));
        propArray.add(new LightLcProp("Ambient LuxLevel Prolong (0x2c)", 0x2c, 2));
        propArray.add(new LightLcProp("Ambient LuxLevel Standby (0x2d)", 0x2d, 2));
        propArray.add(new LightLcProp("Regulator Kiu (0x33)", 0x33, 4));
        propArray.add(new LightLcProp("Regulator Kid (0x32)", 0x32, 4));
        propArray.add(new LightLcProp("Regulator Kpu (0x35)", 0x35, 4));
        propArray.add(new LightLcProp("Regulator Kpd (0x34)", 0x34, 4));
        propArray.add(new LightLcProp("Regulator Accuracy (0x31)", 0x31, 1));
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
    public void onHslStateChanged(String deviceName, int lightness, int hue, int saturation, int remainingTime) {

    }

    @Override
    public void onOnOffStateChanged(final String deviceName, final byte targetOnOff, final byte presentOnOff, final int remainingTime) {
        show("Light onoff status "+ targetOnOff, Toast.LENGTH_SHORT);
    }

    @Override
    public void onLevelStateChanged(String deviceName, short targetLevel, short presentLevel, int remainingTime) {

    }

    @Override
    public void onNetworkConnectionStatusChanged(byte transport, byte status) {

    }

    @Override
    public void onCtlStateChanged(String deviceName, int presentLightness, short presentTemperature, int targetLightness, short targetTemperature, int remainingTime) {

    }

    @Override
    public void onNodeConnStateChanged(byte isConnected, String componentName) {

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
        show("Light LC Mode status "+mode, Toast.LENGTH_SHORT);
    }

    @Override
    public void onLightLcOccupancyModeStatus(String componentName, int mode) {
        show("Light LC Occupancy Mode status "+mode, Toast.LENGTH_SHORT);
    }

    @Override
    public void onLightLcPropertyStatus(String componentName, int propertyId, int value) {
        show("Light LC property status "+propertyId, Toast.LENGTH_SHORT);
    }

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

class LightLcProp
{
    String PropName;
    int PropId;
    int len;

    public LightLcProp(String propName, int propId, int len) {
        PropName = propName;
        PropId = propId;
        this.len = len;
    }
}
