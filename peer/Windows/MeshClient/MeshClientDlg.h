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
* MeshClientDlg.h : header file
*
*/

#pragma once
#include "Win10Interface.h"
#include "afxcmn.h"
#include "WsOtaDownloader.h"
#include "wiced_bt_mesh_models.h"
#ifdef MESH_DFU_ENABLED
#include "wiced_bt_mesh_dfu.h"
#endif


#define WM_USER_PROXY_DATA              (WM_USER + (WICED_BT_MESH_CORE_UUID_CHARACTERISTIC_PROXY_DATA_OUT & 0xFF))
#define WM_USER_PROVISIONING_DATA       (WM_USER + (WICED_BT_MESH_CORE_UUID_CHARACTERISTIC_PROVISIONING_DATA_OUT & 0xFF))
#define WM_USER_WS_UPGRADE_CTRL_POINT   (WM_USER + (GUID_OTA_FW_UPGRADE_CHARACTERISTIC_CONTROL_POINT.Data1 & 0xFF))

#define WM_PROGRESS                     (WM_USER + 103)
#define WM_MESH_DEVICE_CONNECTED        (WM_USER + 104)
#define WM_MESH_DEVICE_DISCONNECTED     (WM_USER + 105)
#define WM_MESH_DEVICE_CONNECT          (WM_USER + 106)
#define WM_MESH_DEVICE_DISCONNECT       (WM_USER + 107)
#define WM_MESH_DEVICE_ADV_REPORT       (WM_USER + 108)
#define WM_USER_LOG                     (WM_USER + 109)
#define WM_TIMER_CALLBACK               (WM_USER + 110)
#define WM_MESH_DEVICE_CCCD_PUT_COMPLETE    (WM_USER + 111)

#define WM_SOCKET (WM_USER + 181)


// CMeshClientDlg dialog
class CMeshClientDlg : public CDialogEx
{
    // Construction
public:
    CMeshClientDlg(CWnd* pParent = NULL);	// standard constructor
    virtual ~CMeshClientDlg();

    // Dialog Data
    enum { IDD = IDD_MESH_CONTROLLER_DIALOG };

    //    BLUETOOTH_ADDRESS m_bth;
    HMODULE m_hLib;
    CBtInterface *m_btInterface;

    CListBox    *m_trace;
    BOOL        m_scan_started;
    BOOL        m_bConnecting;
    BOOL        m_bScanning;
    BOOL        m_bDfuStatus;
    CString     m_sDfuImageFilePath;
#ifdef MESH_DFU_ENABLED
    mesh_dfu_fw_id_t        m_DfuFwId;
    mesh_dfu_meta_data_t    m_DfuMetaData;
#endif

    void OnCancel();
    void SetDlgItemHex(DWORD id, DWORD val);
    DWORD GetHexValue(DWORD id, LPBYTE buf, DWORD buf_size);
//    DWORD GetHexValue(char *szBuf, LPBYTE buf, DWORD buf_size);
    DWORD GetHexValueInt(DWORD id);

    void DisplayCurrentGroup();
    void ProvisionCompleted();

    CProgressCtrl m_Progress;
    LRESULT OnProgress(WPARAM completed, LPARAM total);
    WSDownloader *m_pDownloader;
    LPBYTE m_pPatch;
    DWORD m_dwPatchSize;

    WCHAR   m_szCurrentGroup[80];
    BOOL m_bConnected;
    void ProcessUnprovisionedDevice(uint8_t *p_uuid, uint16_t oob, uint8_t *name, uint8_t name_len);
    void LinkStatus(uint8_t is_connected, uint32_t conn_id, uint16_t addr, uint8_t is_over_gatt);
    void ProcessVendorSpecificData(LPBYTE p_data, DWORD len);

    void UpdateScanState(BOOL bScanning);

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_LIGHT_CONTROL };
#endif


protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:
    HANDLE m_hDevice;
    HANDLE m_hCfgEvent;
    int m_dfuState;

    // Implementation
protected:
    HICON m_hIcon;
    LRESULT OnWsUpgradeCtrlPoint(WPARAM op, LPARAM lparam);
    LRESULT OnMeshDeviceConnected(WPARAM Instance, LPARAM lparam);
    LRESULT OnMeshDeviceDisconnected(WPARAM Instance, LPARAM lparam);
    LRESULT OnMeshDeviceAdvReport(WPARAM Instance, LPARAM lparam);
    LRESULT OnMeshDeviceConnect(WPARAM state, LPARAM param);
    LRESULT OnMeshDeviceDisconnect(WPARAM state, LPARAM param);
    LRESULT OnSocketMessage(WPARAM wParam, LPARAM lParam);
    LRESULT OnMeshDeviceCCCDPutComplete(WPARAM state, LPARAM param);

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
    virtual void PostNcDestroy();
    LRESULT OnUserLog(WPARAM op, LPARAM lparam);
    LRESULT OnTimerCallback(WPARAM op, LPARAM lparam);
    LRESULT OnProxyDataIn(WPARAM op, LPARAM lparam);
    LRESULT OnProvisioningDataIn(WPARAM op, LPARAM lparam);

    void SetHexValueInt(DWORD id, DWORD val, DWORD val_len);
    void GetDlgItemTextUTF8(int id, char* buf, DWORD buf_len);

public:
    void Disconnect();
    void DeviceConnected(BOOL provisioning);
    void OnOffGet();

    void trace(char * fmt_str, ...);
public:
    void CMeshClientDlg::updateProvisionerUuid();
    void SetHexValue(DWORD id, LPBYTE val, DWORD val_len);
    void OnNodeConnected();
    BOOL IsOtaSupported();
    void StartOta();
#ifdef MESH_DFU_ENABLED
    BOOL ReadDfuManifestFile(CString sFilePath);
    uint32_t GetDfuImageSize();
    void GetDfuImageChunk(uint8_t *p_data, uint32_t offset, uint16_t data_len);
    BOOL GetDfuImageInfo(void *p_fw_id, void *p_va_data);
    void OnDfuStatusCallback(uint8_t state, uint8_t* p_data, uint32_t data_length);
#endif

public:
    afx_msg void OnTimer(UINT_PTR nIDEvent);


    afx_msg void OnClose();
    afx_msg void OnBnClickedClearTrace();
    afx_msg void OnBnClickedScanUnprovisioned();
    afx_msg void OnBnClickedProvision();
    afx_msg void OnBnClickedNetworkCreate();
    afx_msg void OnBnClickedNetworkDelete();
    afx_msg void OnBnClickedNetworkOpen();
    afx_msg void OnBnClickedNetworkClose();
    afx_msg void OnBnClickedGroupCreate();
    afx_msg void OnBnClickedGroupDelete();
    afx_msg void OnBnClickedNodeReset();
    afx_msg void OnSelchangeNetwork();
    afx_msg void OnBnClickedConfigureNewName();
    afx_msg void OnSelchangeCurrentGroup();
    afx_msg void OnBnClickedMoveToGroup();
    afx_msg void OnBnClickedConfigurePub();
    afx_msg void OnBnClickedOnOffGet();
    afx_msg void OnBnClickedOnOffSet();
    afx_msg void OnBnClickedLevelGet();
    afx_msg void OnBnClickedLevelSet();
    afx_msg void OnBnClickedLightHslGet();
    afx_msg void OnBnClickedLightHslSet();
    afx_msg void OnBnClickedVsData();
    afx_msg void OnBnClickedLightCtlGet();
    afx_msg void OnBnClickedLightCtlSet();
    afx_msg void OnBnClickedLightnessGet();
    afx_msg void OnBnClickedLightnessSet();

    afx_msg void OnBnClickedOtaUpgradeStart();
    afx_msg void OnBnClickedConnectdisconnect();
    afx_msg void OnBnClickedIdentify();
    afx_msg void OnBnClickedReconfigure();
    afx_msg void OnBnClickedBrowse();
    afx_msg void OnCbnSelchangeConfigureControlDevice();
    afx_msg void OnCbnSelchangeConfigureMoveDevice();
    afx_msg void OnBnClickedNetworkImport();
    afx_msg void OnBnClickedNetworkExport();
    afx_msg void OnBnClickedOtaUpgradeStop();
    afx_msg void OnBnClickedGetComponentInfo();
    afx_msg void OnBnClickedOtaUpgradeStatus();
    afx_msg void OnBnClickedSensorGet();
    afx_msg void OnCbnSelchangeControlDevice();
    afx_msg void OnBnClickedSensorConfigure();
    afx_msg void OnBnClickedLcConfigure();
};
