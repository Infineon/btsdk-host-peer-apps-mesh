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
 * Customer Popover view controller implementation.
 */

import UIKit

enum PopoverType: String {
    case unknownType = "Unknown Type"
    case deleteNetwork = "Delete Network"
    case exportNetwork = "Export Network"
    case importNetwork = "Import Network"
    case componentAddToGroup = "Add to Group"
    case componentMoveToGroup = "Move to Group"
}

enum PopoverButtonType: String {
    case none = "None"  // indicates called from non popover VC.
    case confirm = "Confirm"
    case cancel = "Cancel"
}

typealias PopoverSelectionCallback = (_ btnType: PopoverButtonType, _ selectedItem: String?) -> ()

class PopoverViewController: UIViewController {
    @IBOutlet weak var titleLabel: UILabel!
    @IBOutlet weak var confirmButton: UIButton!
    @IBOutlet weak var cancelButton: UIButton!
    @IBOutlet weak var tableView: UITableView!

    private let indicatorView = CustomIndicatorView()

    static var parentViewController: UIViewController?          // must be set before init.
    static var popoverCompletion: PopoverSelectionCallback?     // must be set before init.
    static var popoverType: PopoverType = .unknownType          // must be set before init.
    static var popoverItems: [String] = []                      // must be set before init.
    static var popoverTitle: String?                            // optional, set before init.

    var selectedItem: String? {
        return PopoverTableViewCell.selectedItem
    }

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        tableView.dataSource = self
        tableView.delegate = self
        tableView.separatorStyle = .none

        titleLabel.text = PopoverViewController.popoverTitle ?? PopoverViewController.popoverType.rawValue
    }

    @IBAction func onConfirmButtonClick(_ sender: UIButton) {
        if let completion = PopoverViewController.popoverCompletion {
            DispatchQueue.main.async {
                completion(PopoverButtonType.confirm, self.selectedItem)
            }
        }
        self.dismiss(animated: false, completion: nil)
    }

    @IBAction func onCancelButtonClick(_ sender: UIButton) {
        if let completion = PopoverViewController.popoverCompletion {
            DispatchQueue.main.async {
                completion(PopoverButtonType.cancel, nil)
            }
        }
        self.dismiss(animated: false, completion: nil)
    }
}

extension PopoverViewController: UITableViewDelegate, UITableViewDataSource {
    func numberOfSections(in tableView: UITableView) -> Int {
        return 1
    }

    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return PopoverViewController.popoverItems.count
    }

    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        guard let cell = tableView.dequeueReusableCell(withIdentifier: MeshAppStoryBoardIdentifires.POPOVER_CELL, for: indexPath) as? PopoverTableViewCell else {
            return UITableViewCell()
        }
        cell.item = PopoverViewController.popoverItems[indexPath.row]
        return cell
    }
}
