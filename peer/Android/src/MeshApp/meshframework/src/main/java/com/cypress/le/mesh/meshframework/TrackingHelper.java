/*
* Copyright 2018, Cypress Semiconductor Corporation or a subsidiary of Cypress Semiconductor
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

import android.util.Log;

import com.cypress.le.mesh.meshcore.MeshNativeHelper;

class TrackingHelper {

    int mTrackingType = 0;
    static levelSet mLevelSet = null;
    static hslSet mHslSet = null;
    static lightnessSet mLightness = null;
    static ctlSet mCtlSet = null;
    boolean isValueChanged = false;

    static final int TRACK_HSL_SET = 0;
    static final int TRACK_LEVEL_SET = 1;
    static final int TRACK_LIGHTNESS_SET = 2;
    static final int TRACK_CTL_SET = 3;

    private static MeshNativeHelper mMeshNativeHelper = MeshNativeHelper.getInstance();

    void execute(boolean reliable) {
        Log.d("trackingHelper", "executing" +reliable);
        switch(mTrackingType) {
            case TRACK_HSL_SET:{
                if(mHslSet != null && isValueChanged)
                    mMeshNativeHelper.meshClientHslSet(mHslSet.component, mHslSet.lightness, mHslSet.hue, mHslSet.saturation, reliable, mHslSet.transitionTime, mHslSet.delay);
            } break;
            case TRACK_LEVEL_SET: {
                if(mLevelSet != null && isValueChanged)
                    mMeshNativeHelper.meshClientLevelSet(mLevelSet.component, mLevelSet.level, reliable, mLevelSet.transitionTime, mLevelSet.delay);
            } break;
            case TRACK_LIGHTNESS_SET: {
                if(mLightness != null && isValueChanged)
                    mMeshNativeHelper.meshClientLightnessSet(mLightness.name, mLightness.lightness, reliable, mLightness.transitionTime, mLightness.delay);
            } break;
            case TRACK_CTL_SET: {
                if(mCtlSet != null && isValueChanged)
                    mMeshNativeHelper.meshClientCtlSet(mCtlSet.component, mCtlSet.lightness, mCtlSet.temperature, mCtlSet.deltaUv, reliable, mCtlSet.transitionTime, mCtlSet.delay);
            } break;
        }
    }

    public void stopTracking() {
        execute(true);
    }

    public void startTracking() {

    }

    public void levelSetMessage(String component, short val, int transitionTime, short delay) {
        if(mLevelSet == null) {
            mLevelSet = new levelSet(component, val, transitionTime, delay);
            isValueChanged = true;
        } else {
            if(mLevelSet.level == val && mLevelSet.component.equals(component)) {
                isValueChanged = false;
            } else {
                isValueChanged = true;
                mLevelSet = new levelSet(component, val, transitionTime, delay);
            }
        }
    }

    public void hslSetMessage(String componentName, short lightness, short hue, short saturation, int transitionTime, short delay) {
        if(mHslSet == null) {
            mHslSet = new hslSet(componentName, lightness, hue, saturation, transitionTime, delay);
            isValueChanged = true;
        } else {
            if(mHslSet.lightness == lightness &&
                    mHslSet.hue == hue &&
                    mHslSet.saturation == saturation &&
                    mHslSet.component.equals(componentName)) {
                isValueChanged = false;
            } else {
                isValueChanged = true;
                mHslSet = new hslSet(componentName, lightness, hue, saturation, transitionTime, delay);
            }
        }
    }

    public void setTrackingType(int trackingType) {
        mTrackingType = trackingType;
    }

    public void LightnessSetMessage(String name, int lightness, int transitionTime, short delay) {
        if(mLightness == null) {
            mLightness = new lightnessSet(name, lightness, transitionTime, delay);
            isValueChanged = true;
        } else {
            if(mLightness.lightness == lightness &&
                    mLightness.name.equals(name)) {
                isValueChanged = false;
            } else {
                isValueChanged = true;
                mLightness = new lightnessSet(name, lightness, transitionTime, delay);
            }
        }
    }

    public void CtlSetMessage(String name, int lightness, short temperature, short deltaUv, int transitionTime, short delay) {
        if(mCtlSet == null) {
            mCtlSet = new ctlSet(name, lightness, temperature, deltaUv, transitionTime, delay);
            isValueChanged = true;
        } else {
            if(mCtlSet.lightness == lightness &&
                    mCtlSet.deltaUv == deltaUv &&
                    mCtlSet.temperature == temperature &&
                    mCtlSet.component.equals(name)) {
                isValueChanged = false;
            } else {
                isValueChanged = true;
                mCtlSet = new ctlSet(name, lightness, temperature, deltaUv, transitionTime, delay);
            }
        }
    }


    private class levelSet {
        String component;
        short level;
        int transitionTime;
        short delay;

        public levelSet(String componentName, short level, int transitionTime, short delay) {
            this.component = componentName;
            this.level  = level;
            this.transitionTime = transitionTime;
            this.delay = delay;
        }
    }

    private class hslSet {
        String component;
        short lightness;
        short hue;
        short saturation;
        int transitionTime;
        short delay;

        public hslSet(String componentName, short lightness, short hue, short saturation, int transitionTime, short delay) {
            this.component = componentName;
            this.lightness = lightness;
            this.hue = hue;
            this.saturation = saturation;
            this.transitionTime = transitionTime;
            this.delay = delay;
        }
    }

    private class ctlSet {
        String component;
        int lightness;
        short temperature;
        short deltaUv;
        int transitionTime;
        short delay;

        public ctlSet(String name, int lightness, short temperature, short deltaUv, int transitionTime, short delay) {
            this.component = name;
            this.lightness = lightness;
            this.temperature = temperature;
            this.deltaUv = deltaUv;
            this.transitionTime = transitionTime;
            this.delay = delay;
        }
    }

    private class lightnessSet {
        String name;
        int lightness;
        int transitionTime;
        short delay;

        public lightnessSet(String componentName, int lightness, int transitionTime, short delay) {
            this.name = componentName;
            this.lightness = lightness;
            this.transitionTime = transitionTime;
            this.delay = delay;
        }
    }

}
