package com.cypress.le.mesh.meshapp;

import java.util.UUID;
import android.bluetooth.BluetoothDevice;

/**
 * Represents a remote mesh device and it is identified by its uuid and Bluetooth address.
 *
 */
public class MeshBluetoothDevice {

    public BluetoothDevice mBtDevice = null;
    public UUID mUUID;

    public String getmName() {
        return mName;
    }

    public void setName(String mName) {
        this.mName = mName;
    }

    public String mName;

    public MeshBluetoothDevice(BluetoothDevice bluetoothDevice) {
        mBtDevice = bluetoothDevice;
        mUUID = new UUID(0, 0);

    }
    public MeshBluetoothDevice(UUID uuid) {
        mUUID = uuid;

    }
    public MeshBluetoothDevice(UUID uuid, String name) {
        mUUID = uuid;
        mName = name;
    }

    /**
     * Gets the UUID or Bluetooth device Name
     * @return Name of the Bluetooth device
     */
    public String getName() {
        return (mUUID.getLeastSignificantBits() == 0) ? mBtDevice.getName() : mUUID.toString();
    }

    @Override
    public String toString() {
//        if(mBtDevice!=null)
//            return mBtDevice.toString();
//        else
//            return mUUID.toString();
        return mName + " "+mUUID.toString();
    }

}
