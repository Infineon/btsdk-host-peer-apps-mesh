package com.cypress.le.mesh.meshapp;

import android.app.AlertDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Spinner;

import com.cypress.le.mesh.meshframework.MeshController;

import java.util.ArrayList;
import java.util.List;

public class SensorSetting extends AppCompatActivity {

    private static final String TAG = "SensorConfiguration";

    boolean triggerTypeVal;
    boolean isInside;
    String pubAddress = "";
    static int propertySelected;
    int selectedSetting;
    int mFastCadence = 0;
    String mComponentName = "";
    static LightingService serviceReference = null;
    List<Integer> settingsList = new ArrayList<Integer>();
    List<String> settingsListNames = new ArrayList<String>();
    Spinner settings;
    Spinner pubAddrs;
    Spinner insideOutside;
    EditText pubTime;
    CheckBox triggerCheckBox;
    CheckBox fastCadenceCheckBox;
    View fastCadenceView;
    View triggerView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sensor_setting);
        ActionBar toolbar = getSupportActionBar();
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setTitle("Sensor Configuration");

        final Intent serviceIntent= new Intent(this, LightingService.class);
        bindService(serviceIntent, mConnection, Context.BIND_AUTO_CREATE);
        Intent intent = getIntent();

        if(intent!=null) {
            Bundle extras = getIntent().getExtras();
            if(extras != null) {
                propertySelected = extras.getInt("mPropertyId");
                mComponentName = extras.getString("mComponentName");
            } else {
                Log.d(TAG, "Extras are null");
            }
        } else {
            Log.d(TAG, "Intent is null");
        }
        final EditText triggerDeltaLow = (EditText)findViewById(R.id.trigger_delta_low);
        final EditText triggerDeltaHigh = (EditText)findViewById(R.id.trigger_delta_high);
        final EditText fastCadenceHigh = (EditText)findViewById(R.id.fast_cadence_high);
        final EditText fastCadenceLow = (EditText)findViewById(R.id.fast_cadence_low);
        final Spinner fastCadence = (Spinner)findViewById(R.id.fast_cadence);
        final Spinner triggerType = (Spinner)findViewById(R.id.trigger_type);
        pubAddrs = (Spinner)findViewById(R.id.pub_addrs);
        pubTime = (EditText)findViewById(R.id.pub_time);
        final EditText minInterval = (EditText)findViewById(R.id.minInterval);
        final EditText settingval = (EditText)findViewById(R.id.settingval);
        final Button cadenceSet = (Button)findViewById(R.id.cadenceset);
        final Button publishSet = (Button)findViewById(R.id.publishset);
        final Button settingSet = (Button)findViewById(R.id.settingbtn);
        settings = (Spinner)findViewById(R.id.setting);
        triggerCheckBox = (CheckBox) findViewById(R.id.tick_trigger);
        fastCadenceCheckBox = (CheckBox) findViewById(R.id.tick_fast_cadence);
        fastCadenceView = (View)findViewById(R.id.fastCadView);
        triggerView = (View)findViewById(R.id.triggerView);
        insideOutside = (Spinner) findViewById(R.id.insideOutsideVal);

        insideOutside.setSelection(0);

        triggerCheckBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                if(b == true)
                {
                    triggerView.setVisibility(View.VISIBLE);
                } else
                {
                    triggerView.setVisibility(View.GONE);
                    triggerDeltaLow.setText("");
                    triggerDeltaHigh.setText("");
                    minInterval.setText("");
                }
            }
        });

        fastCadenceCheckBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                if(b == true)
                {
                    fastCadenceView.setVisibility(View.VISIBLE);
                } else
                {
                    fastCadenceView.setVisibility(View.GONE);
                    fastCadenceLow.setText("");
                    fastCadenceHigh.setText("");
                }
            }
        });
        triggerType.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {

                String type = (String)triggerType.getItemAtPosition(position);
                Log.d(TAG," type :"+type);
                if(type.equals("Native")) {
                    triggerTypeVal = MeshController.MESH_CLIENT_TRIGGER_TYPE_NATIVE;
                } else {
                    triggerTypeVal = MeshController.MESH_CLIENT_TRIGGER_TYPE_PERCENTAGE;
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        insideOutside.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {

                String type = (String)insideOutside.getItemAtPosition(position);
                Log.d(TAG," val :"+type);
                if(type.equals("Inside")) {
                    isInside = true;
                } else {
                    isInside = false;
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        pubAddrs.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                pubAddress = (String)pubAddrs.getItemAtPosition(position);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        fastCadence.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                mFastCadence = Integer.parseInt(fastCadence.getItemAtPosition(position).toString());
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        cadenceSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (!minInterval.getText().toString().isEmpty())
                {
                    if(Integer.parseInt(minInterval.getText().toString()) < 1)
                    {
                        //min interval has to be atleast 1sec
                        AlertDialog alertDialog = new AlertDialog.Builder(SensorSetting.this).create();
                        alertDialog.setTitle("Invalid Input!");
                        alertDialog.setMessage("\nPlease enter minimum interval more than 1sec");
                        alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "OK",
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int which) {
                                        dialog.dismiss();
                                    }
                                });
                        alertDialog.show();
                        return;
                    }
                }

                final int fastCadHighVal = (!fastCadenceHigh.getText().toString().isEmpty())?(getSensorValueForProperty(propertySelected,Integer.parseInt(fastCadenceHigh.getText().toString()))) : 0;
                final int fastCadLowVal = (!fastCadenceLow.getText().toString().isEmpty())?(getSensorValueForProperty(propertySelected, Integer.parseInt(fastCadenceLow.getText().toString()))) : 0;
                Thread t = new Thread(new Runnable() {
                    public void run() {
                        serviceReference.getMesh().sensorCadenceSet(mComponentName,
                                propertySelected,
                                (short) mFastCadence,
                                triggerTypeVal,
                                (!triggerDeltaLow.getText().toString().isEmpty())? Integer.parseInt(triggerDeltaLow.getText().toString()): 0,
                                (!triggerDeltaHigh.getText().toString().isEmpty())?Integer.parseInt(triggerDeltaHigh.getText().toString()): 0,
                                (!minInterval.getText().toString().isEmpty())?Integer.parseInt(minInterval.getText().toString())*1000:1000,
                                isInside == true ? fastCadLowVal : fastCadHighVal,
                                isInside == true ? fastCadHighVal : fastCadLowVal);
                    }
                });
                t.start();
            }


        });

        publishSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Thread t = new Thread(new Runnable() {
                    public void run() {
                        int res =serviceReference.getMesh().setPublicationConfig(
                                Constants.DEFAULT_PUBLISH_CREDENTIAL_FLAG,
                                Constants.DEFAULT_RETRANSMIT_COUNT,
                                Constants.DEFAULT_RETRANSMIT_INTERVAL,
                                Constants.DEFAULT_PUBLISH_TTL
                        );
                        Log.d(TAG, "result setPublicationConfig:"+res);

                        int ret = serviceReference.getMesh().configurePublication(mComponentName, false, "SENSOR", pubAddress, !pubTime.getText().toString().isEmpty() ? Integer.parseInt(pubTime.getText().toString()):Constants.DEFAULT_PUBLISH_PERIOD);
                        Log.d(TAG, "config pub"+ret);
                    }
                });
                t.start();
            }
        });

        settings.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
               @Override
               public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                   Log.d(TAG,"on settings item clicked pos : "+i + "setting :"+settingsList.get(i));
                   selectedSetting = settingsList.get(i);
               }
               @Override
               public void onNothingSelected(AdapterView<?> adapterView) {

               }
           }
        );

        settingSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                byte[] val = new byte[2];
                val[0] = (!settingval.getText().toString().isEmpty())? Byte.parseByte(settingval.getText().toString()) : 0;
                serviceReference.getMesh().sensorSettingSet(mComponentName, propertySelected, (short) selectedSetting, val);
            }
        });
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

    private int getSensorValueForProperty(int propertyId, int value) {
        int res = value;
        switch (propertyId)
        {
            case Constants.MESH_PROPERTY_MOTION_SENSED :
                return res;
            case Constants.MESH_PROPERTY_PRESENCE_DETECTED :
                return res;
            case Constants.MESH_PROPERTY_PRESENT_AMBIENT_LIGHT_LEVEL :
                return res * 100;
            case Constants.MESH_PROPERTY_PRESENT_AMBIENT_TEMPERATURE :
                return res * 2;
            case Constants.MESH_PROPERTY_AMBIENT_LUX_LEVEL_ON:
                return res * 100;
            default:
                return res;
        }
    }

    private ServiceConnection mConnection= new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "bound service connected");
            LightingService.MyBinder binder = (LightingService.MyBinder) service;
            serviceReference = binder.getService();
            short[] val= serviceReference.getMesh().sensorSettingsGet(mComponentName, propertySelected);
            settingsList.clear();
            settingsListNames.clear();
            if(val != null) {
                for (int j=0; j< val.length; j++) {
                    settingsList.add((int) val[j]);
                    settingsListNames.add(getProperty(val[j]));
                }

            }
            ArrayAdapter aa = new ArrayAdapter(getApplicationContext(), android.R.layout.simple_spinner_item, settingsListNames);
            aa.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            settings.setAdapter(aa);

            final ArrayList<String>components = (ArrayList<String>) serviceReference.getallRooms();
            components.add("all-nodes");
            components.add("all-proxies");
            components.add("all_friends");
            components.add("all-relays");

            ArrayAdapter ab = new ArrayAdapter(getApplicationContext(), android.R.layout.simple_spinner_item, components);
            ab.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            pubAddrs.setAdapter(ab);
            int period = serviceReference.getMesh().getPublicationPeriod(mComponentName,false,MeshController.MESH_CONTROL_METHOD_SENSOR);
            Log.d(TAG, "period "+period);
            String pubName = serviceReference.getMesh().getPublicationTarget(mComponentName,false,MeshController.MESH_CONTROL_METHOD_SENSOR);
            Log.d(TAG, "pub name "+pubName);

            for(int i=0; i<components.size();i++) {
                if(components.get(i).equals(pubName)) {
                    pubAddrs.setSelection(i);
                    break;
                }
            }
            pubTime.setText(period+"");
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "bound service disconnected");
            serviceReference = null;
        }
    };

    private String getProperty(int propValue) {
        Log.d(TAG, "getProperty :"+propValue);
        switch (propValue)
        {
            case Constants.MESH_PROPERTY_AMBIENT_LUX_LEVEL_ON: return "Lux level on";
            case Constants.MESH_PROPERTY_MOTION_SENSED: return "Motion sensed";
            case Constants.MESH_PROPERTY_PRESENCE_DETECTED: return "Presence detected";
            case Constants.MESH_PROPERTY_PRESENT_AMBIENT_LIGHT_LEVEL: return "Ambient light level";
            case Constants.MESH_PROPERTY_PRESENT_AMBIENT_TEMPERATURE: return "Ambient temperature";
            case Constants.MESH_PROPERTY_TOTAL_DEVICE_RUNTIME: return "Device Runtime";
            default: return "Unknown prop: " + Integer.toHexString( propValue );
        }
    }

}
