/*
 * Copyright 2022, Cypress Semiconductor Corporation (an Infineon company) or
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

/** @file
 *
 * This file implements the TrackingHelper class to help avoid send level, hsl, lightness and ctl set command too frequently.
 */

import Foundation

open class TrackingHelper: NSObject {
    public static let shared = TrackingHelper()

    private var componentType: Int = MeshConstants.MESH_COMPONENT_UNKNOWN
    private var levelSet: LevelSet?
    private var hslSet: HslSet?
    private var lightnessSet: LightnessSet?
    private var ctlSet: CtlSet?
    private var isValueChanged: Bool = false

    private let lock = NSLock()
    private var trackingTimer: Timer?
    private var mIsTracking: Bool = false
    open var isTracking: Bool {
        lock.lock()
        let tracking = mIsTracking
        lock.unlock()
        return tracking
    }

    private func execute(reliable: Bool) {
        // For reliable message, always send it out even the data no changed.
        // Because last unreliable message maybe not received or processed by remote device.
        if reliable {
            isValueChanged = true
        }

        switch componentType {
        case MeshConstants.MESH_COMPONENT_LIGHT_HSL:
            if let hslSet = self.hslSet, isValueChanged {
                MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error: Int) in
                    guard error == MeshErrorCode.MESH_SUCCESS else {
                        meshLog("TrackingHelper, failed to connect to mesh network")
                        return
                    }

                    let error = Int(MeshNativeHelper.meshClientHslSet(hslSet.componentName,
                                                                      lightness: UInt16(hslSet.lightness),
                                                                      hue: UInt16(hslSet.hue),
                                                                      saturation: UInt16(hslSet.saturation),
                                                                      reliable: reliable,
                                                                      transitionTime: UInt32(hslSet.transitionTime),
                                                                      delay: UInt16(hslSet.delay)))
                    meshLog("TrackingHelper, meshClientHslSet, \(hslSet), reliable=\(reliable), error=\(error)")
                    self.isValueChanged = false
                }
            }
        case MeshConstants.MESH_COMPONENT_GENERIC_LEVEL_CLIENT:
            fallthrough
        case MeshConstants.MESH_COMPONENT_GENERIC_LEVEL_SERVER:
            if let levelSet = self.levelSet, isValueChanged {
                MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error: Int) in
                    guard error == MeshErrorCode.MESH_SUCCESS else {
                        meshLog("TrackingHelper, failed to connect to mesh network")
                        return
                    }

                    let error = Int(MeshNativeHelper.meshClientLevelSet(levelSet.componentName,
                                                                        level: Int16(levelSet.level),
                                                                        reliable: reliable,
                                                                        transitionTime: UInt32(levelSet.transitionTime),
                                                                        delay: UInt16(levelSet.delay)))
                    meshLog("TrackingHelper, meshClientLevelSet, \(levelSet), reliable=\(reliable), error=\(error)")
                    self.isValueChanged = false
                }
            }
        case MeshConstants.MESH_COMPONENT_LIGHT_DIMMABLE:
            if let lightnessSet = self.lightnessSet, isValueChanged {
                MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error: Int) in
                    guard error == MeshErrorCode.MESH_SUCCESS else {
                        meshLog("TrackingHelper, failed to connect to mesh network")
                        return
                    }

                    let error = Int(MeshNativeHelper.meshClientLightnessSet(lightnessSet.componentName,
                                                                            lightness: UInt16(lightnessSet.lightness),
                                                                            reliable: reliable,
                                                                            transitionTime: UInt32(lightnessSet.transitionTime),
                                                                            delay: UInt16(lightnessSet.delay)))
                    meshLog("TrackingHelper, meshClientLightnessSet, \(lightnessSet), reliable=\(reliable), error=\(error)")
                    self.isValueChanged = false
                }
            }
        case MeshConstants.MESH_COMPONENT_LIGHT_CTL:
            if let ctlSet = self.ctlSet, isValueChanged {
                MeshFrameworkManager.shared.runHandlerWithMeshNetworkConnected { (error: Int) in
                    guard error == MeshErrorCode.MESH_SUCCESS else {
                        meshLog("TrackingHelper, failed to connect to mesh network")
                        return
                    }

                    let error = Int(MeshNativeHelper.meshClientCtlSet(ctlSet.componentName,
                                                                      lightness: UInt16(ctlSet.lightness),
                                                                      temperature: UInt16(ctlSet.temperature),
                                                                      deltaUv: UInt16(ctlSet.deltaUv),
                                                                      reliable: reliable,
                                                                      transitionTime: UInt32(ctlSet.transitionTime),
                                                                      delay: UInt16(ctlSet.delay)))
                    meshLog("TrackingHelper, meshClientCtlSet, \(ctlSet), reliable=\(reliable), error=\(error)")
                    self.isValueChanged = false
                }
            }
        default:
            break
        }
    }

    open func levelSetMessage(componentName: String, level: Int, transitionTime: UInt32, delay: Int) {
        guard let levelSet = self.levelSet else {
            self.levelSet = LevelSet(componentName: componentName, level: level, transitionTime: transitionTime, delay: delay)
            self.isValueChanged = true
            return
        }

        if levelSet.componentName == componentName, levelSet.level != level {
            self.levelSet = LevelSet(componentName: componentName, level: level, transitionTime: transitionTime, delay: delay)
            self.isValueChanged = true
        }
    }

    open func hslSetMessage(componentName: String, lightness: Int, hue: Int, saturation: Int, transitionTime: UInt32, delay: Int) {
        guard let hslSet = self.hslSet else {
            self.hslSet = HslSet(componentName: componentName, lightness: lightness, hue: hue, saturation: saturation, transitionTime: transitionTime, delay: delay)
            self.isValueChanged = true
            return
        }

        if hslSet.componentName == componentName  {
            if hslSet.lightness != lightness || hslSet.hue != hue ||  hslSet.saturation != saturation {
                self.hslSet = HslSet(componentName: componentName, lightness: lightness, hue: hue, saturation: saturation, transitionTime: transitionTime, delay: delay)
                self.isValueChanged = true
            }
        }
    }

    open func lightnessSetMessage(componentName: String, lightness: Int, transitionTime: UInt32, delay: Int) {
        guard let lightnessSet = self.lightnessSet else {
            self.lightnessSet = LightnessSet(componentName: componentName, lightness: lightness, transitionTime: transitionTime, delay: delay)
            self.isValueChanged = true
            return
        }

        if lightnessSet.componentName == componentName, lightnessSet.lightness != lightness {
            self.lightnessSet = LightnessSet(componentName: componentName, lightness: lightness, transitionTime: transitionTime, delay: delay)
            self.isValueChanged = true
        }
    }

    open func ctlSetMessage(componentName: String, lightness: Int, temperature: Int, deltaUv: Int, transitionTime: UInt32, delay: Int) {
        guard let ctlSet = self.ctlSet else {
            self.ctlSet = CtlSet(componentName: componentName, lightness: lightness, temperature: temperature, deltaUv: deltaUv, transitionTime: transitionTime, delay: delay)
            self.isValueChanged = true
            return
        }

        if ctlSet.componentName == componentName  {
            if ctlSet.lightness != lightness || ctlSet.temperature != temperature ||  ctlSet.deltaUv != deltaUv {
                self.ctlSet = CtlSet(componentName: componentName, lightness: lightness, temperature: temperature, deltaUv: deltaUv, transitionTime: transitionTime, delay: delay)
                self.isValueChanged = true
            }
        }
    }

    private func _stopTracking() {
        stopTrackingTimer()
        mIsTracking = false
        execute(reliable: true)
        levelSet = nil
        hslSet = nil
        lightnessSet = nil
        ctlSet = nil
        isValueChanged = false
        componentType = MeshConstants.MESH_COMPONENT_UNKNOWN
    }

    open func stopTracking() {
        lock.lock()
        _stopTracking()
        lock.unlock()
    }

    open func startTracking(componentType: Int) {
        lock.lock()
        if mIsTracking {
            lock.unlock()
            return  // the tracking has been started already, return early.
        }

        // start a new tracking.
        mIsTracking = true
        self.componentType = componentType
        startTrackingTimer()
        lock.unlock()
    }

    @objc private func onTrackingTimerTimeout() {
        self.execute(reliable: false)
    }

    private func stopTrackingTimer() {
        trackingTimer?.invalidate()
        trackingTimer = nil
    }

    private func startTrackingTimer() {
        stopTrackingTimer()

        guard let tracking_timer_interval_str = UserDefaults.standard.string(forKey: "tracking_timer_interval"),
            let tracking_timer_interval = Int(tracking_timer_interval_str),
            tracking_timer_interval > 0 else {
                // The tracking timer is not support or disabled (0 indicates disabled),
                // so only when the tracking stopped, the control command will be really sent out.
                mIsTracking = false
                return
        }

        let interval = (TimeInterval(exactly: tracking_timer_interval) ?? 1000.0) / 1000.0
        if #available(iOS 10.0, *) {
            self.trackingTimer = Timer.scheduledTimer(withTimeInterval: interval,
                                                      repeats: true, block: { (Timer) in
                                                        self.onTrackingTimerTimeout()
            })
        } else {
            self.trackingTimer = Timer.scheduledTimer(timeInterval: interval, target: self,
                                                      selector: #selector(self.onTrackingTimerTimeout),
                                                      userInfo: nil, repeats: true)
        }
    }


    private struct LevelSet {
        var componentName: String
        var level: Int
        var transitionTime: UInt32
        var delay: Int

        init(componentName: String, level: Int, transitionTime: UInt32, delay: Int) {
            self.componentName = componentName
            self.level = level
            self.transitionTime = transitionTime
            self.delay = delay
        }
    }

    private struct HslSet {
        var componentName: String
        var lightness: Int
        var hue: Int
        var saturation: Int
        var transitionTime: UInt32
        var delay: Int

        init(componentName: String, lightness: Int, hue: Int, saturation: Int, transitionTime: UInt32, delay: Int) {
            self.componentName = componentName
            self.lightness = lightness
            self.hue = hue
            self.saturation = saturation
            self.transitionTime = transitionTime
            self.delay = delay
        }
    }

    private struct CtlSet {
        var componentName: String
        var lightness: Int
        var temperature: Int
        var deltaUv: Int
        var transitionTime: UInt32
        var delay: Int

        init(componentName: String, lightness: Int, temperature: Int, deltaUv: Int, transitionTime: UInt32, delay: Int) {
            self.componentName = componentName
            self.lightness = lightness
            self.temperature = temperature
            self.deltaUv = deltaUv
            self.transitionTime = transitionTime
            self.delay = delay
        }
    }

    private struct LightnessSet {
        var componentName: String
        var lightness: Int
        var transitionTime: UInt32
        var delay: Int

        init(componentName: String, lightness: Int, transitionTime: UInt32, delay: Int) {
            self.componentName = componentName
            self.lightness = lightness
            self.transitionTime = transitionTime
            self.delay = delay
        }
    }
}
