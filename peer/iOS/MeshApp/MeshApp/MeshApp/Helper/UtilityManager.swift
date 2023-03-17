/*
 * Copyright 2016-2023, Cypress Semiconductor Corporation (an Infineon company) or
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
 * Mesh App relative utilities definitions.
 */

import UIKit
import Foundation
import MeshFramework

public class UtilityManager: NSObject {
    public static func validateEmail(email: String?) -> Bool {
        guard let email = email else { return false }
        let emailRegex = "[A-Z0-9a-z._%+-]+@[A-Z0-9a-z.-]+\\.[A-Za-z]{2,4}"
        let emailTest = NSPredicate(format: "SELF MATCHES %@", argumentArray: [emailRegex])
        return emailTest.evaluate(with: email)
    }

    // Note, if the VC's modalPresentationStyle is set to the style that it doesn't support, then will cause the crash issue.
    public static func navigateToViewController(sender: UIViewController, targetVCClass: AnyClass, modalPresentationStyle: UIModalPresentationStyle = .fullScreen, storyboard: String = "Main") {
        meshLog("navigateToViewController, targetVCClass=\"\(String(describing:targetVCClass))\"")
        let vc = UIStoryboard(name: storyboard, bundle: nil).instantiateViewController(withIdentifier: String(describing:targetVCClass))
        vc.modalPresentationStyle = modalPresentationStyle
        sender.present(vc, animated: true, completion: nil)
    }

    public static func navigateToViewController(targetClass: AnyClass, storyboard: String = "Main", parentVC: UIViewController? = nil) {
        let vc = UIStoryboard(name: storyboard, bundle: nil).instantiateViewController(withIdentifier: String(describing: targetClass))
        guard vc.isKind(of: targetClass) else {
            meshLog("error: navigateToViewController, failed to initialize instance: \(String(describing: targetClass))")
            return
        }
        meshLog("navigateToViewController: initialize instance: \(String(describing: targetClass)) success")
        if let parentVC = parentVC {
            parentVC.navigationController?.pushViewController(vc, animated: true)
            return
        } else if let appDelegate = UIApplication.shared.delegate, let window = appDelegate.window {
            if let window = window {
                window.rootViewController = vc
                window.makeKeyAndVisible()
                return
            }
        }
        meshLog("error: failed to navigateToViewController: \(String(describing: targetClass))")
    }

    public static func showAlertDialogue(parentVC: UIViewController?, message: String, title: String = "Error",
                                         completion: (() -> Void)? = nil,
                                         action: UIAlertAction = UIAlertAction(title: "OK", style: .default,
                                                                               handler: { (action) in return })) {
        if let parentVC = parentVC {
            let alertConteroller = UIAlertController(title: title, message: message, preferredStyle: .alert)
            alertConteroller.addAction(action)
            parentVC.present(alertConteroller, animated: true, completion: completion)
        }
    }

    public static func showAlertDialogue(parentVC: UIViewController?,
                                         message: String,
                                         title: String = "Error",
                                         cancelHandler: @escaping (_ action: UIAlertAction) -> (),
                                         okayHandler: @escaping (_ action: UIAlertAction) -> ()) {
        if let parentVC = parentVC {
            let alertConteroller = UIAlertController(title: title, message: message, preferredStyle: .alert)
            let cancelAction = UIAlertAction(title: "Cancel", style: .default, handler: { (action) in cancelHandler(action) })
            let okayAction = UIAlertAction(title: "OK", style: .default, handler: { (action) in okayHandler(action) })
            alertConteroller.addAction(cancelAction)
            alertConteroller.addAction(okayAction)
            parentVC.present(alertConteroller, animated: true, completion: nil)
        }
    }

    public enum Quadrant {
        case TopLeft
        case TopRight
        case BottomLeft
        case BottomRight
        case Unknown
    }

    public static func getQuadrant(for point: CGPoint,
                                   bound: (topPoint: CGPoint, leftPoint: CGPoint, rightPoint: CGPoint, bottomPoint: CGPoint)) -> Quadrant {
        if point.x <= bound.topPoint.x && point.y <= bound.leftPoint.y {
            return .TopLeft
        } else if point.x >= bound.topPoint.x && point.y <= bound.rightPoint.y {
            return .TopRight
        } else if point.x < bound.bottomPoint.x && point.y >= bound.leftPoint.y {
            return .BottomLeft
        } else if point.x >= bound.bottomPoint.x && point.y >= bound.rightPoint.y {
            return .BottomRight
        } else {
            return .Unknown
        }
    }

    public static func area(p1: CGPoint, p2: CGPoint, p3: CGPoint) -> CGFloat{
        return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y)
    }

    public static func pointLiesInsideTringle(point: CGPoint, tp1: CGPoint, tp2: CGPoint, tp3: CGPoint)->Bool{
        let d1 = area(p1: point, p2: tp1, p3: tp2)
        let d2 = area(p1: point, p2: tp2, p3: tp3)
        let d3 = area(p1: point, p2: tp3, p3: tp1)
        return ((d1 >= 0.0 && d2 >= 0.0 && d3 >= 0.0) || (d1 <= 0.0 && d2 <= 0.0 && d3 <= 0.0))
    }

    public static func getIntersectionPoint(for line1: (p1: CGPoint, p2: CGPoint),
                                            and line2: (p1: CGPoint, p2: CGPoint)) -> CGPoint {
        struct LineParameters {
            var a: CGFloat
            var b: CGFloat
            var c: CGFloat
        }
        func getLineParameters(point1: CGPoint, point2: CGPoint) -> LineParameters {
            let a = point1.y - point2.y
            let b = point2.x - point1.x
            let c = point1.x * point2.y - point1.y * point2.x
            return LineParameters(a: a, b: b, c: c)
        }
        let line1Params = getLineParameters(point1: line1.p1, point2: line1.p2)
        let line2Params = getLineParameters(point1: line2.p1, point2: line2.p2)
        let determinant = line1Params.a * line2Params.b - line2Params.a * line1Params.b
        if determinant == 0 {   // 0 indicates the two lines won't cross.
            return CGPoint.zero
        }
        let x = (line1Params.b * line2Params.c - line2Params.b * line1Params.c) / determinant
        let y = (line1Params.c * line2Params.a - line2Params.c * line1Params.a) / determinant
        return CGPoint(x: x, y: y)
    }

    public static func getDistance(between p1: CGPoint, and p2: CGPoint) -> CGFloat {
        return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2))
    }

    // The mesh component name must be in the format of "nameYYYY (XXXX)". e.g.: Light Color (0008). The address string in hexdecimal format.
    public static func getMeshAddressString(meshComponentName: String) -> String {
        let addressComponent = String(meshComponentName.trimmingCharacters(in: CharacterSet.whitespaces).suffix(7))
        if addressComponent.hasPrefix(" ("), addressComponent.hasSuffix(")") {
            let addressString = String(addressComponent.suffix(5).prefix(4))
            let characterSet = CharacterSet(charactersIn: "0123456789abcdefABCDEF")
            let leftString = addressString.trimmingCharacters(in: characterSet)
            if leftString.isEmpty {
                return addressString
            }
        }
        return ""
    }

    public static func getMeshAddressSuffixString(meshComponentName: String) -> String {
        return " (" + UtilityManager.getMeshAddressString(meshComponentName: meshComponentName) + ")"
    }

    // The mesh component name must be in the format of "nameYYYY (XXXX)". e.g.: Light Color (0008). The address string in hexdecimal format.
    public static func getMeshAddress(meshComponentName: String) -> Int {
        let upperCased = UtilityManager.getMeshAddressString(meshComponentName: meshComponentName).uppercased()
        var sum: Int = 0
        for char in upperCased.utf8 {
            sum = sum * 16 + Int(char) - 48
            if char >= 65 {    // A=65, A..F should be wrapped to 10..15.
                sum -= 7
            }
        }
        return sum
    }

    public static func lightPerceivedLightnessToLightIntensity(_ perceivedLightness: Int) -> Int {
        if perceivedLightness >= MeshConstants.LIGHT_LIGHTNESS_MESH_RAW_VALUE_MAX {
            return MeshConstants.LIGHT_LIGHTNESS_MESH_RAW_VALUE_MAX
        }
        if perceivedLightness <= MeshConstants.LIGHT_LIGHTNESS_MESH_RAW_VALUE_MIN {
            return MeshConstants.LIGHT_LIGHTNESS_MESH_RAW_VALUE_MIN
        }
        let max = Double(MeshConstants.LIGHT_LIGHTNESS_MESH_RAW_VALUE_MAX)
        return Int(Double(perceivedLightness) * Double(perceivedLightness) / max)
    }

    public static func lightIntensityToLightPerceivedLightness(_ lightIntensity: Int) -> Int {
        if lightIntensity >= MeshConstants.LIGHT_LIGHTNESS_MESH_RAW_VALUE_MAX {
            return MeshConstants.LIGHT_LIGHTNESS_MESH_RAW_VALUE_MAX
        }
        if lightIntensity <= MeshConstants.LIGHT_LIGHTNESS_MESH_RAW_VALUE_MIN {
            return MeshConstants.LIGHT_LIGHTNESS_MESH_RAW_VALUE_MIN
        }
        let max = Double(MeshConstants.LIGHT_LIGHTNESS_MESH_RAW_VALUE_MAX)
        let v = Double(lightIntensity) / max
        return Int(max * sqrt(v))
    }

    /* Convert HSL light lightness value from (0~100, percentage) to mesh raw value (0~0xFFFF). */
    public static func lightLightnessToMeshRawValue(lightness: Double) -> Int {
        if lightness >= MeshConstants.LIGHT_LIGHTNESS_PERCENTAGE_MAX {
            return MeshConstants.LIGHT_LIGHTNESS_MESH_RAW_VALUE_MAX
        }
        if lightness <= MeshConstants.LIGHT_LIGHTNESS_PERCENTAGE_MIN {
            return MeshConstants.LIGHT_LIGHTNESS_MESH_RAW_VALUE_MIN
        }
        return Int(lightness * Double(MeshConstants.LIGHT_LIGHTNESS_MESH_RAW_VALUE_MAX) / MeshConstants.LIGHT_LIGHTNESS_PERCENTAGE_MAX)
    }

    /* Convert mesh raw value from (0~0xFFFF) to HSL light lightness value (0~100, percentage). */
    public static func meshRawValueToLighLightness(value: Int) -> Double {
        if value >= MeshConstants.LIGHT_LIGHTNESS_MESH_RAW_VALUE_MAX {
            return MeshConstants.LIGHT_LIGHTNESS_PERCENTAGE_MAX
        }
        if value <= MeshConstants.LIGHT_LIGHTNESS_MESH_RAW_VALUE_MIN {
            return MeshConstants.LIGHT_LIGHTNESS_PERCENTAGE_MIN
        }
        return Double(value) * MeshConstants.LIGHT_LIGHTNESS_PERCENTAGE_MAX / Double(MeshConstants.LIGHT_LIGHTNESS_MESH_RAW_VALUE_MAX)
    }

    /* Convert HSL light Hue value from (0~360, degrees) to mesh raw value (0~0xFFFF). */
    public static func lightHueToMeshRawValue(hue: Double) -> Int {
        if hue >= MeshConstants.LIGHT_HUE_MAX {
            return MeshConstants.LIGHT_HUE_MESH_RAW_VALUE_MAX
        }
        if hue <= MeshConstants.LIGHT_HUE_MIN {
            return MeshConstants.LIGHT_HUE_MESH_RAW_VALUE_MIN
        }
        return Int(hue * Double(MeshConstants.LIGHT_HUE_MESH_RAW_VALUE_MAX) / MeshConstants.LIGHT_HUE_MAX)
    }

    /* Convert mesh raw value from (0~0xFFFF) to HSL light Hue value (0~360, degrees). */
    public static func meshRawValueToLightHue(value: Int) -> Double {
        if value >= MeshConstants.LIGHT_HUE_MESH_RAW_VALUE_MAX {
            return MeshConstants.LIGHT_HUE_MAX
        }
        if value <= MeshConstants.LIGHT_HUE_MESH_RAW_VALUE_MIN {
            return MeshConstants.LIGHT_HUE_MIN
        }
        return Double(value) * MeshConstants.LIGHT_HUE_MAX / Double(MeshConstants.LIGHT_HUE_MESH_RAW_VALUE_MAX)
    }

    /* Convert HSL light saturation value from (0~100, percentage) to mesh raw value (0~0xFFFF). */
    public static func lightSaturationToMeshRawValue(saturation: Double) -> Int {
        if saturation >= MeshConstants.LIGHT_SATURATION_PERCENTAGE_MAX {
            return MeshConstants.LIGHT_SATURATION_MESH_RAW_VALUE_MAX
        }
        if saturation <= MeshConstants.LIGHT_SATURATION_PERCENTAGE_MIN {
            return MeshConstants.LIGHT_SATURATION_MESH_RAW_VALUE_MIN
        }
        return Int(saturation * Double(MeshConstants.LIGHT_SATURATION_MESH_RAW_VALUE_MAX) / MeshConstants.LIGHT_SATURATION_PERCENTAGE_MAX)
    }

    /* Convert mesh raw value from (0~0xFFFF) to HSL light saturation value (0~100, percentage). */
    public static func meshRawValueToLightSaturation(value: Int) -> Double {
        if value >= MeshConstants.LIGHT_SATURATION_MESH_RAW_VALUE_MAX {
            return MeshConstants.LIGHT_SATURATION_PERCENTAGE_MAX
        }
        if value <= MeshConstants.LIGHT_SATURATION_MESH_RAW_VALUE_MIN {
            return MeshConstants.LIGHT_SATURATION_PERCENTAGE_MIN
        }
        return Double(value) * MeshConstants.LIGHT_SATURATION_PERCENTAGE_MAX / Double(MeshConstants.LIGHT_SATURATION_MESH_RAW_VALUE_MAX)
    }

    /* Convert light temperature value from (800~20000, Kelvin, Double) to mesh raw value (800~20000, Kelvin, Int). */
    public static func lightTemperatureToMeshRawValue(temperature: Double) -> Int {
        if temperature >= Double(MeshAppConstants.LIGHT_TEMPERATURE_MAX) {
            return Int(MeshAppConstants.LIGHT_TEMPERATURE_MAX)
        }
        if temperature <= Double(MeshAppConstants.LIGHT_TEMPERATURE_MIN) {
            return Int(MeshAppConstants.LIGHT_TEMPERATURE_MIN)
        }
        return Int(temperature)
    }

    /* Convert mesh raw value from (800~20000, Kelvin, Int) to light temperature value (800~20000, Kelvin, Double). */
    public static func meshRawValueToLightTemperature(value: Int) -> Double {
        if Double(value) >= Double(MeshAppConstants.LIGHT_TEMPERATURE_MAX) {
            return Double(MeshAppConstants.LIGHT_TEMPERATURE_MAX)
        }
        if Double(value) <= Double(MeshAppConstants.LIGHT_TEMPERATURE_MIN) {
            return Double(MeshAppConstants.LIGHT_TEMPERATURE_MIN)
        }
        return Double(value)
    }

    public static func isValidDigitString(digit: String?, isHexdecimal: Bool = false) -> Bool {
        guard let numstr = digit, !numstr.isEmpty else { return false }
        for char in numstr {
            if char.isWhitespace {
                continue
            }
            if char.isHexDigit {
                if char.isLetter {
                    if isHexdecimal {
                        continue
                    } else {
                        return false
                    }
                } else {
                    continue
                }
            } else {
                return false
            }
        }
        return true
    }

    public static func convertDigitStringToInt(digit: String?) -> Int? {
        guard let digitstr = digit?.replacingOccurrences(of: " ", with: ""),
            !digitstr.isEmpty, isValidDigitString(digit: digitstr, isHexdecimal: false) else {
                return nil
        }
        return Int(digitstr)
    }

    public static func convertHexDigitStringToData(hexDigit: String?) -> Data? {
        guard var hexstr = hexDigit?.uppercased().replacingOccurrences(of: " ", with: ""),
            !hexstr.isEmpty, isValidDigitString(digit: hexstr, isHexdecimal: true) else {
                return nil
        }
        if hexstr.count % 2 != 0 {
            hexstr = "0" + hexstr
        }
        let bytes = hexstr.count / 2
        var index = 0
        var data = Data(repeating: UInt8(0), count: bytes)
        var subHexStr: String = ""
        for (i, char) in hexstr.enumerated() {  // i index started from 1.
            subHexStr.append(char)
            if i % 2 != 0 {
                meshLog("\(subHexStr)")
                if let value = UInt8(subHexStr, radix: 16) {
                    data[index] = value
                    index += 1
                } else {
                    return nil
                }

                subHexStr = ""
            }
        }
        return data
    }
}

extension UIViewController {
    func showToast(message: String) {
        let sceenSize = UIScreen.main.bounds.size

        let tempLabel = UILabel(frame: CGRect(x: 0, y: (sceenSize.height - 100), width: sceenSize.width, height: 100))
        tempLabel.font = UIFont.systemFont(ofSize: UIFont.systemFontSize)
        tempLabel.textAlignment = .center
        tempLabel.layer.cornerRadius = 5
        tempLabel.clipsToBounds = true
        tempLabel.lineBreakMode = NSLineBreakMode.byWordWrapping
        var totalLines: Int = 0
        let splitMsgs = message.components(separatedBy: "\n")
        for msg in splitMsgs {
            tempLabel.text = msg
            let textSize = tempLabel.systemLayoutSizeFitting(CGSize(width: 0, height: 0))
            let lines = ceilf(Float(textSize.width) / Float(sceenSize.width))
            totalLines += ((lines == 0) ? 1 : Int(lines))
        }
        tempLabel.text = "One Line"
        totalLines += 2

        let toastHeight: CGFloat = CGFloat(totalLines * Int(tempLabel.font.lineHeight))
        let toastLabel = UILabel(frame: CGRect(x: 0, y: (sceenSize.height - toastHeight), width: sceenSize.width, height: toastHeight))
        toastLabel.backgroundColor = UIColor.black.withAlphaComponent(0.5)
        toastLabel.textColor = UIColor.white
        toastLabel.textAlignment = .center
        toastLabel.font = UIFont.systemFont(ofSize: UIFont.systemFontSize)
        toastLabel.alpha = 1.0
        toastLabel.layer.cornerRadius = 5
        toastLabel.clipsToBounds = true
        toastLabel.lineBreakMode = NSLineBreakMode.byWordWrapping
        toastLabel.numberOfLines = totalLines
        toastLabel.text = message

        self.view.addSubview(toastLabel)
        UIView.animate(withDuration: 5.0, delay: 1.0, options: .curveEaseOut, animations: {
            toastLabel.alpha = 0.0
        }) { (isCompleted) in
            toastLabel.removeFromSuperview()
        }
    }
}
