/*
 * Copyright 2021, Cypress Semiconductor Corporation (an Infineon company) or
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
 * This file implements the MeshUtility class which supplies commonly usage functions for this framwork.
 */

import Foundation

open class MeshUtility: NSObject {
    public static func MD5(data: Data) -> Data {
        return MeshNativeHelper.md5(data)
    }

    public static func MD5(string: String) -> String {
        let data = string.data(using: .utf8)!
        let md5Data = MD5(data: data)
        let md5String: NSMutableString = NSMutableString()
        for i in 0 ..< md5Data.count {
            md5String.append(String(format: "%02X", UInt8(md5Data[i])))
        }
        return md5String.description
    }
}

extension Data {
    public func dumpHexBytes() -> String {
        let msg: NSMutableString = NSMutableString()
        if (self.count > 0) {
            // dump firstly byte.
            msg.append(String(format: "%02X", UInt8(self[0])))
            // dump second and continue bytes if exists.
            for i in 1 ..< self.count {
                msg.append(String(format: " %02X", UInt8(self[i])))
            }
            return msg.description
        }
        return ""
    }
}

extension Array where Element: Hashable {
    func removingDuplicates() -> [Element] {
        var dict = [Element: Bool]()
        return filter {
            dict.updateValue(true, forKey: $0) == nil
        }
    }

    mutating func removeDuplicates() {
        self = self.removingDuplicates()
    }
}
