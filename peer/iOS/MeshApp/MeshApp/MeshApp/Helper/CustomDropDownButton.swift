/*
 * Copyright 2016-2021, Cypress Semiconductor Corporation (an Infineon company) or
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
 * Customer Drop Down button implementation.
 */

import UIKit

/*
 * When use this CustomDropDownButton in the storyboard, should
 *   1) set the button type to Custom.
 *   2) set the text color to what required.
 */
class CustomDropDownButton: UIButton {
    @IBInspectable public var isRightTriangle: Bool = true {
        didSet {
            udpateTitleEdgeInsets()
        }
    }

    public var dropDownItems: [String] = [] {
        didSet {
            initializeDefaultSelection()
            initializePopoverController()
        }
    }

    private var mSelectIndex: Int = 0
    private var mSelectedString: String = ""
    public var selectedString: String {
        return mSelectedString
    }
    public var selectedIndex: Int {
        return mSelectIndex
    }
    public func setSelection(select: Int) {
        guard select < dropDownItems.count else { return }
        updateSelection(at: select, with: dropDownItems[select])
    }
    public func setSelection(select: String) {
        for (index, value) in dropDownItems.enumerated() {
            if value == select {
                updateSelection(at: index, with: value)
                return
            }
        }
    }

    private func updateSelection(at index: Int, with value: String) {
        self.mSelectedString = value
        self.mSelectIndex = index
        self.setTitle(value, for: .normal)
        self.setTitle(value, for: .selected)
    }

    private func initializeDefaultSelection() {
        if dropDownItems.isEmpty || dropDownItems[0].isEmpty {
            updateSelection(at: 0, with: "")
        } else {
            updateSelection(at: 0, with: dropDownItems[0])
        }
    }

    private func updateTitleColor() {
        self.setTitleColor(self.titleColor(for: .normal), for: .selected)
        self.titleLabel?.backgroundColor = nil
    }

    private func udpateTitleEdgeInsets() {
        guard let imageView = self.imageView else { return }
        let pedding: CGFloat = 2
        if isRightTriangle {
            self.titleEdgeInsets.left = 0 - imageView.bounds.width + pedding
            self.titleEdgeInsets.right = 0 - imageView.bounds.width - pedding
        } else {
            self.titleEdgeInsets.right = pedding
        }
    }

    private var popoverViewController: PopoverChoiceTableViewController<String>?
    private func initialize() {
        if isRightTriangle {
            self.setImage(UIImage(named: MeshAppImageNames.leftTriangleImage), for: .normal)
        } else {
            self.setImage(UIImage(named: MeshAppImageNames.rightTriangleImage), for: .normal)
        }
        self.setImage(UIImage(named: MeshAppImageNames.downTriangleImage), for: .selected)
        self.isSelected = false
        udpateTitleEdgeInsets()
        updateTitleColor()
        initializeDefaultSelection()

        self.addTarget(self, action: #selector(onTouchUpInside), for: .touchUpInside)

        initializePopoverController()
    }

    private func initializePopoverController() {
        popoverViewController = PopoverChoiceTableViewController(choices: dropDownItems) { (index: Int, selection: String) in
            self.updateSelection(at: index, with: selection)
            self.isSelected = false
        }
    }

    public func showDropList(width: Int, parent: UIViewController, sourceView: UIView? = nil, selectCompletion: (() -> Void)? = nil) {
        guard let controller = popoverViewController else { return }
        controller.preferredContentSize = CGSize(width: width, height: controller.getPreferredPopoverViewSize())
        controller.showPopoverPresentation(parent: parent, sourceView: self) { () -> Void in
            self.isSelected = false
            selectCompletion?()
        }
    }

    @objc func onTouchUpInside() {
        self.isSelected = !self.isSelected
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

    override func layoutSubviews() {
        super.layoutSubviews()
        guard let imageView = self.imageView, let titleLabel = self.titleLabel else {
            return
        }

        self.contentHorizontalAlignment = .left
        self.contentVerticalAlignment = .center

        if isRightTriangle {
            imageView.center.x = self.bounds.width - imageView.bounds.width / 2
            let newTitleLabelBounds = CGRect(x: 0,
                                             y: titleLabel.frame.origin.y,
                                             width: self.bounds.width - imageView.bounds.width - 2,
                                             height: titleLabel.frame.height)
            self.titleLabel?.frame = newTitleLabelBounds
            self.titleLabel?.textRect(forBounds: newTitleLabelBounds, limitedToNumberOfLines: 1)
        }
    }
}
