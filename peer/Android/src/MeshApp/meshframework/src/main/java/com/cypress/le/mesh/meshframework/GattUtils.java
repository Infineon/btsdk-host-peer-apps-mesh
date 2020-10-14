/*
* Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of Cypress Semiconductor
 *  Corporation. All rights reserved. This software, including source code, documentation and  related
 * materials ("Software"), is owned by Cypress Semiconductor  Corporation or one of its
 *  subsidiaries ("Cypress") and is protected by and subject to worldwide patent protection
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
 *  manufacturer of such system or application assumes  all risk of such use and in doing so agrees
 * to indemnify Cypress against all liability.
*/
package com.cypress.le.mesh.meshframework;

import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.util.Log;

import java.util.ArrayDeque;
import java.util.Queue;
import java.util.UUID;

/**
 * Helper functions to work with services and characteristics
 */
class GattUtils {
    private static final String TAG = "GattUtils";

    /**
     * Queue for ensuring read/write requests are serialized so that each
     * read/write completes for the the next read/write request is performed
     */
    public static class RequestQueue {

        /**
         * Internal object used to manage a specific read or write request
         */
        private class GattRequest {
            private static final int REQUEST_READ_CHAR = 1;
            private static final int REQUEST_WRITE_CHAR = 2;
            private static final int REQUEST_READ_DESC = 11;
            private static final int REQUEST_WRITE_DESC = 12;

            private final int mRequestType;
            private final byte[] mValue;
            private final BluetoothGatt mGatt;
            private final BluetoothGattCharacteristic mCharacteristic;
            private final BluetoothGattDescriptor mDescriptor;

            public GattRequest(int requestType, BluetoothGatt gatt,
                    BluetoothGattCharacteristic characteristic) {
                mGatt = gatt;
                mCharacteristic = characteristic;
                mDescriptor = null;
                mRequestType = requestType;
                mValue = null;
            }

            public GattRequest(int requestType, BluetoothGatt gatt,
                               BluetoothGattCharacteristic characteristic, byte [] value) {
                mGatt = gatt;
                mValue = value;
                mCharacteristic = characteristic;
                mDescriptor = null;
                mRequestType = requestType;
            }

            public GattRequest(int requestType, BluetoothGatt gatt,
                    BluetoothGattDescriptor descriptor) {
                mGatt = gatt;
                mValue = null;
                mDescriptor = descriptor;
                mCharacteristic = null;
                mRequestType = requestType;
            }

            public GattRequest(int requestType, BluetoothGatt gatt,
                               BluetoothGattDescriptor descriptor, byte[] value) {
                mGatt = gatt;
                mValue = value;
                mDescriptor = descriptor;
                mCharacteristic = null;
                mRequestType = requestType;
            }
        }

        // Queue containing requests
        private final Queue<GattRequest> mRequestQueue = new ArrayDeque<GattRequest>();

        // If true, the request queue is currently processing read/write
        // requests
        private boolean mIsRunning;

        /**
         * Add a read descriptor request to the queue
         *
         * @param gatt
         * @param descriptor
         */
        public synchronized void addReadDescriptor(BluetoothGatt gatt,
                BluetoothGattDescriptor descriptor) {
            if (gatt == null || descriptor == null) {
                Log.d(TAG, "addReadDescriptor(): invalid data");
                return;
            }
            mRequestQueue.add(new GattRequest(GattRequest.REQUEST_READ_DESC, gatt, descriptor));
        }

        /**
         * Add a write descriptor request to the queue
         *
         * @param gatt
         * @param descriptor
         */
        public synchronized void addWriteDescriptor(BluetoothGatt gatt,
                BluetoothGattDescriptor descriptor, byte[] value) {
            if (gatt == null || descriptor == null) {
                Log.d(TAG, "addWriteDescriptor(): invalid data");
                return;
            }
            mRequestQueue.add(new GattRequest(GattRequest.REQUEST_WRITE_DESC, gatt, descriptor, value));
        }

        /**
         * Add a read characteristic request to the queue
         *
         * @param gatt
         * @param characteristic
         */
        public synchronized void addReadCharacteristic(BluetoothGatt gatt,
                BluetoothGattCharacteristic characteristic) {
            if (gatt == null || characteristic == null) {
                Log.d(TAG, "addReadCharacteristic(): invalid data");
                return;
            }
            mRequestQueue.add(new GattRequest(GattRequest.REQUEST_READ_CHAR, gatt, characteristic));
        }

        /**
         * Add a write characteristic request to the queue
         *
         * @param gatt
         * @param characteristic
         */
        public synchronized void addWriteCharacteristic(BluetoothGatt gatt,
                BluetoothGattCharacteristic characteristic, byte[] value) {
            if (gatt == null || characteristic == null) {
                Log.d(TAG, "addWriteCharacteristic(): invalid data");
                return;
            }
            mRequestQueue.add(new GattRequest(GattRequest.REQUEST_WRITE_CHAR, gatt, characteristic, value));
        }

        /**
         * Clear all the requests in the queue
         */
        public void clear() {
            mRequestQueue.clear();
            mIsRunning = false;
        }

        /**
         * Get the next queued request, if any, and perform the requested
         * operation
         */
        public void next() {
            GattRequest request = null;
            int i = 0;
            synchronized (this) {
                Log.d(TAG, "next: queue size=" + mRequestQueue.size() + " mIsRunning=" + mIsRunning);
                request = mRequestQueue.poll();
                if (request == null) {
                    Log.d(TAG, "next: no request()");
                    mIsRunning = false;
                    return;
                }


                if (request.mRequestType == GattRequest.REQUEST_READ_CHAR) {
                    Log.d(TAG, "REQUEST_READ_CHAR");
                    boolean res = request.mGatt.readCharacteristic(request.mCharacteristic);
                } else if (request.mRequestType == GattRequest.REQUEST_WRITE_CHAR) {
                    Log.d(TAG, "REQUEST_WRITE_CHAR");
                    boolean res = request.mCharacteristic.setValue(request.mValue);
                    Log.d(TAG, "REQUEST_WRITE_CHAR setval = "+res);
                    res = false;
                    for(i=0; i<4 && !res; i++) {
                        res = request.mGatt.writeCharacteristic(request.mCharacteristic);
                        Log.d(TAG, "REQUEST_WRITE_CHAR writechar i= "+i+" res="+res);
                        if(res) {
                            break;
                        }
                        try {
                            Thread.sleep(100);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                    if(i == 4) {
                        Log.d(TAG, "Retry failure disconnecting gatt");
                        request.mGatt.disconnect();
                    }

                } else if (request.mRequestType == GattRequest.REQUEST_READ_DESC) {
                    Log.d(TAG, "REQUEST_READ_DESC");
                    boolean res = request.mGatt.readDescriptor(request.mDescriptor);
                    Log.d(TAG, "REQUEST_READ_DESC readDesc = "+res);
                } else if (request.mRequestType == GattRequest.REQUEST_WRITE_DESC) {
                    Log.d(TAG, "REQUEST_WRITE_DESC");
                    boolean res = request.mDescriptor.setValue(request.mValue);
                    Log.d(TAG, "REQUEST_WRITE_DESC readdesc = "+res);
                    res = request.mGatt.writeDescriptor(request.mDescriptor);
                    Log.d(TAG, "REQUEST_WRITE_DESC writeDescriptor : " + res);
                }
            }

        }

        /**
         * Start processing the queued requests, if not already started.
         * Otherwise, this method does nothing if processing already started
         */
        public void execute() {
            synchronized (this) {
                Log.d(TAG, "execute: queue size=" + mRequestQueue.size() + " mIsRunning=" + mIsRunning);
                if (mIsRunning) {
                    return;
                }
                mIsRunning = true;
            }
            next();
        }
    }

    /**
     * Create a read/write request queue
     *
     * @return RequestQueue
     */
    public static RequestQueue createRequestQueue() {
        return new RequestQueue();
    }

    /**
     * Get a characteristic with the specified service UUID and characteristic
     * UUID
     *
     * @param gatt
     * @param serviceUuid
     * @param characteristicUuid
     * @return
     */
    public static BluetoothGattCharacteristic getCharacteristic(BluetoothGatt gatt,
            UUID serviceUuid, UUID characteristicUuid) {
        if (gatt == null) {
            return null;
        }
        BluetoothGattService service = gatt.getService(serviceUuid);
        if (service == null) {
            return null;
        }
        return service.getCharacteristic(characteristicUuid);
    }

    /**
     * Get a descriptor with the specified service UUID, characteristic UUID,
     * and descriptor UUID
     *
     * @param gatt
     * @param serviceUuid
     * @param characteristicUuid
     * @param descriptorUuid
     * @return
     */
    public static BluetoothGattDescriptor getDescriptor(BluetoothGatt gatt, UUID serviceUuid,
            UUID characteristicUuid, UUID descriptorUuid) {
        BluetoothGattCharacteristic characteristic = getCharacteristic(gatt, serviceUuid,
                characteristicUuid);
        if (characteristic != null) {
            return characteristic.getDescriptor(descriptorUuid);
        }
        return null;

    }

    /**
     * Converts a byte array into a long value
     *
     * @param bytes
     * @param bytesToRead
     * @param offset
     * @return
     */
    public static long unsignedBytesToLong(byte[] bytes, int bytesToRead, int offset) {
        int shift = 0;
        long l = 0;
        for (int i = offset; i < bytesToRead; i++) {
            int byteFF = bytes[i] & 0xFF;
            l = l + ((long) (bytes[i] & 0xFF) << (8 * shift++));
        }
        return l;
    }

}
