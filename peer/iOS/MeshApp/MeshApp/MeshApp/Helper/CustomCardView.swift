/*
 *$ Copyright 2016-YEAR Cypress Semiconductor $
 */

/** @file
 *
 * Customer card view implementation with additional features.
 */

import UIKit

class CustomCardView: UIView {
    @IBInspectable var cornerRadius: CGFloat = 2
    @IBInspectable var shadowOffsetWidth: Int = 0
    @IBInspectable var shadowOffsetHeight: Int = 3
    @IBInspectable var shadowColor: UIColor? = UIColor.black
    @IBInspectable var shadowOpacity: Float = 0.5

    override func layoutSubviews() {
        super.layoutSubviews()
        layer.cornerRadius = cornerRadius
        let shadowPath = UIBezierPath(roundedRect: bounds, cornerRadius: cornerRadius)
        layer.masksToBounds = false
        layer.shadowColor = shadowColor?.cgColor
        layer.shadowOffset = CGSize(width: shadowOffsetWidth, height: shadowOffsetHeight);
        layer.shadowOpacity = shadowOpacity
        layer.shadowPath = shadowPath.cgPath
    }

    func setCellBackgroundColor(isDark: Bool) {
        if isDark {
            // set to dark grey text color.
            backgroundColor = UIColor(red: 233/255, green: 233/255, blue:233/255, alpha: 1.0)
        }
        else {
            backgroundColor = UIColor.white
        }
    }
}
