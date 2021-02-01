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
 * Customer Drop Down View implementation.
 */

import UIKit
import MeshFramework



protocol CustomDropDownViewDelegate : NSObjectProtocol {
    func customDropDwonViewWillShowDropList(_ dropDownView: CustomDropDownView)
    func customDropDownViewDidUpdateValue(_ dropDownView: CustomDropDownView, selectedIndex: Int)
}

class CustomDropDownView: UITextField {
    @IBInspectable public var rowHeight: CGFloat = 30
    @IBInspectable public var rowBackgroundColor: UIColor = .white
    @IBInspectable public var selectedRowColor: UIColor = .yellow

    @IBInspectable public var borderColor: UIColor =  UIColor.lightGray {
        didSet {
            layer.borderColor = borderColor.cgColor
        }
    }
    @IBInspectable public var listHeight: CGFloat = 210
    @IBInspectable public var borderWidth: CGFloat = 0.0 {
        didSet {
            layer.borderWidth = borderWidth
        }
    }
    @IBInspectable public var cornerRadius: CGFloat = 5.0 {
        didSet {
            layer.cornerRadius = cornerRadius
        }
    }
    @IBInspectable public var arrowSize: CGFloat = 15 {
        didSet{
            let center =  arrow.superview!.center
            arrow.frame = CGRect(x: center.x - arrowSize / 2,
                                 y: center.y - arrowSize / 2,
                                 width: arrowSize,
                                 height: arrowSize)
        }
    }

    var arrow: CustomDropDownArrow!
    var dropDownListTable: UITableView!
    var shadow: UIView!
    var selectedIndex: Int?
    var tableHeight: CGFloat = 100
    private var dataArray: [String] = []
    var dropDownDelegate: CustomDropDownViewDelegate?
    public var dropListItems: [String] {
        get {
            return dataArray
        }
        set {
            self.dataArray = newValue
        }
    }

    public override init(frame: CGRect) {
        super.init(frame: frame)
        viewInit()
    }

    public required init(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)!
        viewInit()
    }

    func viewInit () {
        self.delegate = self
        registerGestureActions()

        // Add arrow icon on the right edge.
        let size = self.frame.height
        let rightView = UIView(frame: CGRect(x: 0.0, y: 0.0, width: size, height: size))
        self.rightView = rightView
        self.rightViewMode = .always
        let arrowContainerView = UIView(frame: rightView.frame)
        self.rightView?.addSubview(arrowContainerView)
        let center = arrowContainerView.center
        arrow = CustomDropDownArrow(origin: CGPoint(x: center.x - arrowSize / 2,
                                                    y: center.y - arrowSize / 2),
                                    size: arrowSize)
        arrowContainerView.addSubview(arrow)
    }

    func registerGestureActions () {
        let gesture =  UITapGestureRecognizer(target: self, action: #selector(onTapGestureHandler))
        self.addGestureRecognizer(gesture)
    }

    @objc public func onTapGestureHandler() {
        isSelected ? hideDropDownList() : showDropDwonList()
    }

    private func bringTableViewToFront() {
        guard self.dropDownListTable != nil else { return }
        dropDownListTable.becomeFirstResponder()
        self.bringSubviewToFront(dropDownListTable)
        var subview: UIView? = self
        var superview: UIView? = self.superview
        while let parentView = superview, let toFrontView = subview {
            parentView.bringSubviewToFront(toFrontView)
            subview = parentView
            superview = parentView.superview
        }
    }

    public func showDropDwonList() {
        if listHeight > (rowHeight * CGFloat( dataArray.count)) {
            self.tableHeight = rowHeight * CGFloat(dataArray.count)
        } else {
            self.tableHeight = listHeight
        }
        dropDownListTable = UITableView(frame: CGRect(x: self.frame.minX,
                                                      y: self.frame.minY,
                                                      width: self.frame.width,
                                                      height: self.frame.height))
        shadow = UIView(frame: CGRect(x: self.frame.minX,
                                      y: self.frame.minY,
                                      width: self.frame.width,
                                      height: self.frame.height))
        shadow.backgroundColor = .clear

        dropDownListTable.dataSource = self
        dropDownListTable.delegate = self
        dropDownListTable.alpha = 0
        dropDownListTable.separatorStyle = .none
        dropDownListTable.layer.cornerRadius = 3
        dropDownListTable.backgroundColor = rowBackgroundColor
        dropDownListTable.rowHeight = rowHeight

        self.superview?.insertSubview(shadow, belowSubview: self)
        self.superview?.insertSubview(dropDownListTable, aboveSubview: self)
        self.isSelected = true

        bringTableViewToFront()

        dropDownDelegate?.customDropDwonViewWillShowDropList(self)
        UIView.animate(withDuration: 0.9,
                       delay: 0,
                       usingSpringWithDamping: 0.4,
                       initialSpringVelocity: 0.1,
                       options: .curveEaseInOut,
                       animations: { () -> Void in
                        var tableFrame = self.frame
                        var tableOriginX = tableFrame.minX
                        let tableOriginY = tableFrame.maxY + 5
                        var tableWidth = tableFrame.width
                        if let superview = self.superview {
                            tableFrame = superview.frame
                            tableOriginX = tableFrame.minX + 10
                            tableWidth = tableFrame.width - 20
                        }
                        self.dropDownListTable.frame = CGRect(x: tableOriginX,
                                                              y: tableOriginY,
                                                              width: tableWidth,
                                                              height: self.tableHeight)
                        self.dropDownListTable.alpha = 1
                        self.dropDownListTable.backgroundColor = UIColor.white
                        self.shadow.frame = self.dropDownListTable.frame
                        self.shadow.dropDownShadow()
                        self.arrow.position = .up
        })
    }

    public func hideDropDownList() {
        UIView.animate(withDuration: 0.5,
                       delay: 0.1,
                       usingSpringWithDamping: 0.9,
                       initialSpringVelocity: 0.1,
                       options: .curveEaseInOut,
                       animations: { () -> Void in
                        self.dropDownListTable.frame = CGRect(x: self.frame.minX,
                                                              y: self.frame.minY,
                                                              width: self.frame.width,
                                                              height: 0)
                        self.shadow.alpha = 0
                        self.shadow.frame = self.dropDownListTable.frame
                        self.arrow.position = .down
        },
                       completion: { (didFinish) -> Void in
                        self.dropDownListTable.endEditing(true)
                        self.shadow.removeFromSuperview()
                        self.dropDownListTable.removeFromSuperview()
                        self.isSelected = false
                        self.dropDownDelegate?.customDropDownViewDidUpdateValue(self, selectedIndex: self.selectedIndex ?? 0)
        })
    }
}

extension CustomDropDownView : UITextFieldDelegate {
    public func textFieldShouldReturn(_ textField: UITextField) -> Bool {
        superview?.endEditing(true)
        return true
    }

    public func  textFieldDidBeginEditing(_ textField: UITextField) {
        textField.text = ""
    }

    public func textFieldShouldBeginEditing(_ textField: UITextField) -> Bool {
        return false
    }

    public func textField(_ textField: UITextField, shouldChangeCharactersIn range: NSRange, replacementString string: String) -> Bool {
        if !isSelected {
            showDropDwonList()
        }
        return false;
    }
}

// avoid super view take the control of scroll.
extension CustomDropDownView {
    open override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        guard dropDownListTable != nil else { return }
        dropDownListTable.touchesBegan(touches, with: event)
    }

    open override func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent?) {
        guard dropDownListTable != nil else { return }
        dropDownListTable.touchesMoved(touches, with: event)
    }

    open override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
        guard dropDownListTable != nil else { return }
        dropDownListTable.touchesEnded(touches, with: event)
    }

    open override func touchesCancelled(_ touches: Set<UITouch>, with event: UIEvent?) {
        guard dropDownListTable != nil else { return }
        dropDownListTable.touchesCancelled(touches, with: event)
    }
}

extension CustomDropDownView: UITableViewDataSource, UITableViewDelegate {
    public func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return dataArray.count
    }

    public func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cellIdentifier = "CustomDropDownViewCell"
        let cell = tableView.dequeueReusableCell(withIdentifier: "CustomDropDownViewCell") ?? UITableViewCell(style: .default, reuseIdentifier: cellIdentifier)

        if indexPath.row != selectedIndex {
            cell.backgroundColor = rowBackgroundColor
        }else {
            cell.backgroundColor = selectedRowColor
        }

        cell.textLabel?.text = "\(dataArray[indexPath.row])"
        cell.accessoryType = (indexPath.row == selectedIndex) ? .checkmark : .none
        cell.selectionStyle = .none
        cell.textLabel?.font = self.font
        cell.textLabel?.textAlignment = self.textAlignment
        return cell
    }

    public func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        selectedIndex = indexPath.row
        let selectedText = dataArray[indexPath.row]
        tableView.cellForRow(at: indexPath)?.alpha = 0
        UIView.animate(withDuration: 0.1,
                       animations: { () -> Void in
                        tableView.cellForRow(at: indexPath)?.alpha = 1.0
                        tableView.cellForRow(at: indexPath)?.backgroundColor = self.selectedRowColor
        },
                       completion: { (didFinish) -> Void in
                        self.text = "\(selectedText)"
                        //tableView.reloadData()
                        self.hideDropDownList()
        })
    }
}

enum CustomDropDownPosition {
    case left
    case right
    case up
    case down
}

class CustomDropDownArrow: UIView {
    var position: CustomDropDownPosition = .down {
        didSet {
            switch position {
            case .left:
                self.transform = CGAffineTransform(rotationAngle: -(CGFloat.pi / 2))
            case .right:
                self.transform = CGAffineTransform(rotationAngle: CGFloat.pi * 2)
            case .up:
                self.transform = CGAffineTransform(rotationAngle: CGFloat.pi / 2)
            case .down:
                self.transform = CGAffineTransform(rotationAngle: CGFloat.pi)
            }
        }
    }

    required init?(coder aDecoder: NSCoder) {
        fatalError("\(#function) has not been implemented")
    }

    init(origin: CGPoint, size: CGFloat) {
        super.init(frame: CGRect(x: origin.x, y: origin.y, width: size, height: size))
    }

    // draw the custom arrow.
    override func draw(_ rect: CGRect) {
        let width = self.layer.frame.width
        let bezierPath = UIBezierPath()
        let quaterWidth = width / 4

        bezierPath.move(to: CGPoint(x: 0, y: quaterWidth))
        bezierPath.addLine(to: CGPoint(x: width, y: quaterWidth))
        bezierPath.addLine(to: CGPoint(x: width/2, y: quaterWidth * 3))
        bezierPath.addLine(to: CGPoint(x: 0, y: quaterWidth))
        bezierPath.close()

        let shapeLayer = CAShapeLayer()
        shapeLayer.path = bezierPath.cgPath
        if #available(iOS 12.0, *) {
            self.layer.addSublayer (shapeLayer)
        } else {
            self.layer.mask = shapeLayer
        }
    }
}

extension UIView {
    func dropDownShadow(scale: Bool = true) {
        layer.masksToBounds = false
        layer.shadowColor = UIColor.black.cgColor
        layer.shadowOpacity = 0.5
        layer.shadowOffset = CGSize(width: 1, height: 1)
        layer.shadowRadius = 2
        layer.shadowPath = UIBezierPath(rect: bounds).cgPath
        layer.shouldRasterize = true
        layer.rasterizationScale = scale ? UIScreen.main.scale : 1
    }
}
