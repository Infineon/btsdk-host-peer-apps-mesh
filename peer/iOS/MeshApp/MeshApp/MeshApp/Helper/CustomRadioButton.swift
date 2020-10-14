/*
 * Copyright 2016-2020, Cypress Semiconductor Corporation or a subsidiary of
 * Cypress Semiconductor Corporation. All Rights Reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software"), is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
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
 * Customer radio button implementation.
 */

import Foundation
import UIKit

@IBDesignable
class CustomRadioButton: UIButton {
    var outerCircleLineLayer = CAShapeLayer()
    var innerCircleFillLayer = CAShapeLayer()

    // Indicates the circles on left or right side. Default is on left size of the button.
    @IBInspectable public var isRightRadioButton: Bool = false {
        didSet {
            udpateTitleEdgeInsets()
            updateCircleLayouts()
        }
    }

    @IBInspectable public var outerCircleLineColor: UIColor = UIColor.darkGray {
        didSet {
            outerCircleLineLayer.strokeColor = outerCircleLineColor.cgColor
        }
    }

    @IBInspectable public var innerCircleFillColor: UIColor = UIColor.darkGray {
        didSet {
            updateButtonState()
        }
    }

    @IBInspectable public var outerCircleLineWidth: CGFloat = 2.0 {
        didSet {
            updateCircleLayouts()
        }
    }

    @IBInspectable public var innerOuterCircleGap: CGFloat = 2.0 {   // Gap between inner and outer circles.
        didSet {
            updateCircleLayouts()
        }
    }


    @IBInspectable var circleLineRadius: CGFloat {
        let width = bounds.width
        let height = bounds.height

        let maxDiamater = width > height ? height : width
        return (maxDiamater / 2 - outerCircleLineWidth)
    }

    @IBInspectable var circleLineFrame: CGRect {
        let width = bounds.width
        let height = bounds.height

        let radius = circleLineRadius
        let x: CGFloat
        let y: CGFloat

        if width > height {
            y = outerCircleLineWidth
            if isRightRadioButton {
                x = width - (radius + outerCircleLineWidth) * 2
            } else {
                x = outerCircleLineWidth
            }
        } else {
            x = outerCircleLineWidth
            y = height / 2 + radius + outerCircleLineWidth
        }

        let diameter = 2 * radius
        return CGRect(x: x, y: y, width: diameter, height: diameter)
    }

    private var outCircleLinePath: UIBezierPath {
        return UIBezierPath(roundedRect: circleLineFrame, cornerRadius: circleLineRadius)
    }

    private var innerCircleFillPath: UIBezierPath {
        let trueGap = innerOuterCircleGap + outerCircleLineWidth
        return UIBezierPath(roundedRect: circleLineFrame.insetBy(dx: trueGap, dy: trueGap), cornerRadius: circleLineRadius)

    }

    private func initialize() {
        outerCircleLineLayer.frame = bounds
        outerCircleLineLayer.lineWidth = outerCircleLineWidth
        outerCircleLineLayer.fillColor = UIColor.clear.cgColor
        outerCircleLineLayer.strokeColor = outerCircleLineColor.cgColor
        layer.addSublayer(outerCircleLineLayer)

        innerCircleFillLayer.frame = bounds
        innerCircleFillLayer.lineWidth = outerCircleLineWidth
        innerCircleFillLayer.fillColor = UIColor.clear.cgColor
        innerCircleFillLayer.strokeColor = UIColor.clear.cgColor
        layer.addSublayer(innerCircleFillLayer)

        udpateTitleEdgeInsets()
        updateButtonState()
    }

    private func udpateTitleEdgeInsets() {
        // Adjust button title left/right edge insets to avoid the circle to be overwritten by the text.
        if isRightRadioButton {
            self.titleEdgeInsets.left = 0
            self.titleEdgeInsets.right = (outerCircleLineWidth + circleLineRadius + 20)
        } else {
            self.titleEdgeInsets.left = (outerCircleLineWidth + circleLineRadius + 20)
            self.titleEdgeInsets.right = 0
        }
    }

    private func updateCircleLayouts() {
        outerCircleLineLayer.frame = bounds
        outerCircleLineLayer.lineWidth = outerCircleLineWidth
        outerCircleLineLayer.path = outCircleLinePath.cgPath

        innerCircleFillLayer.frame = bounds
        innerCircleFillLayer.lineWidth = outerCircleLineWidth
        innerCircleFillLayer.path = innerCircleFillPath.cgPath
    }

    private func updateButtonState() {
        if self.isSelected {
            innerCircleFillLayer.fillColor = outerCircleLineColor.cgColor
        } else {
            innerCircleFillLayer.fillColor = UIColor.clear.cgColor
        }
    }

    override public func layoutSubviews() {
        super.layoutSubviews()
        updateCircleLayouts()
    }

    override public var isSelected: Bool {
        didSet {
            updateButtonState()
        }
    }

    override public init(frame: CGRect) {
        super.init(frame: frame)
        initialize()
    }

    required public init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        initialize()
    }

    override func prepareForInterfaceBuilder() {
        initialize()
    }
}
