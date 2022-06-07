/*
 * Copyright 2016-2022, Cypress Semiconductor Corporation (an Infineon company) or
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
 * Customer Popover choise table view controller implementation.
 */

import UIKit

/* Define the popover view position compared to its parent view. */
enum PopoverTableViewPosition {
    case left
    case right
    case center
}

class PopoverChoiceTableViewController<Element>: UITableViewController {
    typealias SelectionHandler = (Int, Element) -> Void
    typealias LabelProvider = (Element) -> String

    private let choices: [Element]
    private let labels: LabelProvider
    private let onSelected: SelectionHandler?
    private var dismissCompletion: (() -> Void)?

    init(choices: [Element], labels: @escaping LabelProvider = String.init(describing:), onSelected: SelectionHandler? = nil) {
        self.choices = choices
        self.labels = labels
        self.onSelected = onSelected
        super.init(style: .plain)
    }

    required init?(coder aDecoder: NSCoder) {
        fatalError("PopoverChoiceTableViewController, init(coder:) not implemented")
    }

    override func viewDidLoad() {
        super.viewDidLoad()

        // Uncomment the following line to preserve selection between presentations
        // self.clearsSelectionOnViewWillAppear = false

        // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
        // self.navigationItem.rightBarButtonItem = self.editButtonItem
    }

    override func viewDidDisappear(_ animated: Bool) {
        dismissCompletion?()
        super.viewDidDisappear(animated)
    }

    // MARK: - Table view data source

    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return choices.count
    }

    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = UITableViewCell(style: .default, reuseIdentifier: nil)
        cell.textLabel?.text = labels(choices[indexPath.row])
        return cell
    }

    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        self.dismiss(animated: true, completion: nil)
        onSelected?(indexPath.row, choices[indexPath.row])
    }

    func showPopoverPresentation(parent: UIViewController, sourceView: UIView?, position: PopoverTableViewPosition = .center, dismissCompletion: (() -> Void)? = nil) {
        //guard choices.count > 0 else {
        //    dismissCompletion?()
        //    return
        //}
        let presentationController = AlwaysPopoverPresentationController.configurePresentation(for: self)
        presentationController.sourceView = sourceView ?? parent.view
        // default position is pointing to the center of the parent view.
        var positionX = parent.view.bounds.origin.x + parent.view.bounds.size.width / 2
        var positionY = parent.view.bounds.origin.y + 30
        if let srcView = sourceView {
            positionX = srcView.bounds.origin.x + srcView.bounds.size.width / 2
            positionY = srcView.bounds.origin.y + 30
        }
        switch position {
        case .left:
            positionX = parent.view.bounds.origin.x + 20
        case .right:
            positionX = parent.view.bounds.origin.x + parent.view.bounds.size.width - 20
        default:
            break
        }
        presentationController.sourceRect = /*sourceView?.bounds ?? */CGRect(x: positionX, y: positionY, width: 0, height: 0)
        presentationController.permittedArrowDirections = [.down, .up]
        self.dismissCompletion = dismissCompletion
        parent.present(self, animated: true, completion: nil)
    }

    // Calculate the possible popover view size based on how many items will be shown in the popover view.
    func getPreferredPopoverViewSize() -> Int {
        guard choices.count > 0 else {
            return 45
        }
        return choices.count * 45 - 1
    }
}
