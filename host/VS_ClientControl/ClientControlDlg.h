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

// ClientControlDlg.h : header file
//

#include "ControlComm.h"
#include "MeshConfig.h"
#include "LightControl.h"
#include "MeshPerf.h"

#pragma once


#define LE_DWORD(p) (((DWORD)(p)[0]) + (((DWORD)(p)[1])<<8) + (((DWORD)(p)[2])<<16) + (((DWORD)(p)[3])<<24))

struct CBtDevice
{
    UINT8 address_type;
    UINT8 address[6];
    UINT16 con_handle;
};

extern void Log(WCHAR *fmt, ...);

// CClientControlDlg dialog
class CClientControlDlg : public CPropertyPage // public CDialogEx
{
    DECLARE_DYNCREATE(CClientControlDlg)

    // Construction
public:
    CClientControlDlg(CWnd* pParent = NULL);    // standard constructor
    virtual ~CClientControlDlg();

    DWORD GetHandle(DWORD id);
    DWORD GetHexValue(DWORD id, LPBYTE buf, DWORD buf_size);
    CListBox *m_trace;

// Dialog Data
    enum { IDD = IDD_CLIENTCONTROL_DIALOG };

    void DisableAll();
    void ProcessData(DWORD opcode, LPBYTE p_data, DWORD len);
    void ProcessEvent(LPBYTE p_data, DWORD len);

    void EnableOnOff();
    void EnableLevel();
    void EnableDefaultTransitionTime();
    void EnablePowerOnOff();
    void EnablePowerLevel();
    void EnableBattery();
    void EnableLocation();
    void EnableProperty();
    void EnableSensor();
    void EnableTime();
    void EnableScene();
    void EnableScheduler();
    void EnableLightLightness();
    void EnableLightCTL();
    void EnableLightHSL();
    void EnableLightxyL();
    void EnableLightLC();
    void EnableOther();
    USHORT GetDst();

    DWORD GetHexValue(char *szbuf, LPBYTE buf, DWORD buf_size);
    DWORD GetHexValueInt(DWORD id);
#if 1
    BOOL WicedBtMeshCoreSend(void *p_event, const unsigned char* params, unsigned short params_len);
#endif
    int GetBaudRateSelection();
    void ProcessOnOffSet(LPBYTE p_data, DWORD len);
    void ProcessOnOffStatus(LPBYTE p_data, DWORD len);
    void ProcessPowerOnOffStatus(LPBYTE p_data, DWORD len);
    void ProcessPowerLevelStatus(LPBYTE p_data, DWORD len);
    void ProcessPowerLevelLastStatus(LPBYTE p_data, DWORD len);
    void ProcessPowerLevelDefaultStatus(LPBYTE p_data, DWORD len);
    void ProcessPowerLevelRangeStatus(LPBYTE p_data, DWORD len);
    void ProcessLevelStatus(LPBYTE p_data, DWORD len);
    void ProcessDefaultTransitionTimeStatus(LPBYTE p_data, DWORD len);
    void ProcessBatteryStatus(LPBYTE p_data, DWORD len);
    void ProcessLocationGlobalSet(LPBYTE p_data, DWORD len);
    void ProcessLocationLocalSet(LPBYTE p_data, DWORD len);
    void ProcessLocationGlobalChanged(LPBYTE p_data, DWORD len);
    void ProcessLocationLocalChanged(LPBYTE p_data, DWORD len);
    void ReadValuesSendMsg(BYTE local_global);
    void ProcessPropertiesStatus(LPBYTE p_data, DWORD len);
    void ProcessPropertyStatus(LPBYTE p_data, DWORD len);
    void ProcessSceneRegisterStatus(LPBYTE p_data, DWORD len);
    void ProcessSceneStatus(LPBYTE p_data, DWORD len);
    void ProcessSchedulerStatus(LPBYTE p_data, DWORD len);
    void ProcessSchedulerActionStatus(LPBYTE p_data, DWORD len);
    void ProcessTimeStatus(LPBYTE p_data, DWORD len);
    void ProcessTimeZoneStatus(LPBYTE p_data, DWORD len);
    void ProcessTimeTaiDeltaStatus(LPBYTE p_data, DWORD len);
    void ProcessTimeRoleStatus(LPBYTE p_data, DWORD len);
    void ProcessLightnessStatus(BOOL is_linear, LPBYTE p_data, DWORD len);
    void ProcessLightnessLastStatus(LPBYTE p_data, DWORD len);
    void ProcessLightnessDefaultStatus(LPBYTE p_data, DWORD len);
    void ProcessLightnessRangeStatus(LPBYTE p_data, DWORD len);
    void ProcessLightCtlStatus(LPBYTE p_data, DWORD len);
    void ProcessLightCtlTemperatureStatus(LPBYTE p_data, DWORD len);
    void ProcessLightCtlTemperatureRangeStatus(LPBYTE p_data, DWORD len);
    void ProcessLightCtlDefaultStatus(LPBYTE p_data, DWORD len);
    void ProcessLightHslStatus(LPBYTE p_data, DWORD len);
    void ProcessLightHslTargetStatus(LPBYTE p_data, DWORD len);
    void ProcessLightHslHueStatus(LPBYTE p_data, DWORD len);
    void ProcessLightHslSaturationStatus(LPBYTE p_data, DWORD len);
    void ProcessLightHslRangeStatus(LPBYTE p_data, DWORD len);
    void ProcessLightHslDefaultStatus(LPBYTE p_data, DWORD len);
    void ProcessLightXylStatus(LPBYTE p_data, DWORD len);
    void ProcessLightXylTargetStatus(LPBYTE p_data, DWORD len);
    void ProcessLightXylRangeStatus(LPBYTE p_data, DWORD len);
    void ProcessLightXylDefaultStatus(LPBYTE p_data, DWORD len);
    void ProcessLightLcModeStatus(LPBYTE p_data, DWORD len);
    void ProcessLightLcOccupancyModeStatus(LPBYTE p_data, DWORD len);
    void ProcessLightLcLightOnOffStatus(LPBYTE p_data, DWORD len);
    void ProcessLightLcPropertyStatus(LPBYTE p_data, DWORD len);
    void ProcessVendorSpecificData(LPBYTE p_data, DWORD len);
    void ProcessDescriptorStatus(LPBYTE p_data, DWORD len);
    void ProcessSensorDescriptorStatus(LPBYTE p_data, DWORD len);
    void ProcessSensorStatus(LPBYTE p_data, DWORD len);

    void OnBnClickedGetComponentInfo();
    void OnBnClickedOtaUpgradeStart();
    void GetDfuImageChunk(uint8_t *p_data, uint32_t offset, uint16_t data_len);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

private:
    USHORT m_month_selection;
    BYTE m_day_of_week_selection;
    BYTE m_year;
    BYTE m_day;
    BYTE m_hour;
    BYTE m_minute;
    BYTE m_second;

    // Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    // virtual BOOL OnInitDialog();
    virtual BOOL OnSetActive();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
public:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnCbnSelchangeComPort();
    afx_msg void OnClose();
    afx_msg void OnBnClickedBatteryLevelGet();
    afx_msg void OnBnClickedBatteryLevelSet();
    afx_msg void OnBnClickedLocationGet();
    afx_msg void OnBnClickedLocationSet();
    afx_msg void OnBnClickedOnOffGet();
    afx_msg void OnBnClickedOnOffSet();
    afx_msg void OnBnClickedUsePublicationInfo();
    afx_msg void OnBnClickedLevelGet();
    afx_msg void OnBnClickedLevelSet();
    afx_msg void OnBnClickedDeltaSet();
    afx_msg void OnBnClickedMoveSet();
    afx_msg void OnBnClickedDefaultTransitionTimeGet();
    afx_msg void OnBnClickedDefaultTransitionTimeSet();
    afx_msg void OnBnClickedPowerOnOffGet();
    afx_msg void OnBnClickedPowerOnOffSet();
    afx_msg void OnBnClickedPowerLevelGet();
    afx_msg void OnBnClickedPowerLevelSet();
    afx_msg void OnBnClickedPowerLevelLastGet();
    afx_msg void OnBnClickedPowerLevelDefaultGet();
    afx_msg void OnBnClickedPowerLevelDefaultSet();
    afx_msg void OnBnClickedPowerLevelRangeGet();
    afx_msg void OnBnClickedPowerLevelRangeSet();
    afx_msg void OnBnClickedPowerLevelStatus();
    afx_msg void OnBnClickedTcNetLevelTrxSend();
    afx_msg void OnBnClickedTcTranspLevelSend();
    afx_msg void OnBnClickedTcIvUpdateTransit();
    afx_msg void OnBnClickedPropertiesGet();
    afx_msg void OnBnClickedPropertyGet();
    afx_msg void OnBnClickedPropertySet();
    afx_msg void OnBnClickedLightLightnessGet();
    afx_msg void OnBnClickedLightLightnessSet();
    afx_msg void OnBnClickedLightLightnessLastGet();
    afx_msg void OnBnClickedLightLightnessDefaultGet();
    afx_msg void OnBnClickedLightLightnessDefaultSet();
    afx_msg void OnBnClickedLightLightnessRangeGet();
    afx_msg void OnBnClickedLightLightnessRangeSet();
    afx_msg void OnBnClickedTcLpnFrndClear();
    afx_msg void OnBnClickedLightHslGet();
    afx_msg void OnBnClickedLightHslSet();
    afx_msg void OnBnClickedLightHslHueGet();
    afx_msg void OnBnClickedLightHslHueSet();
    afx_msg void OnBnClickedLightHslSaturationGet();
    afx_msg void OnBnClickedLightHslSaturationSet();
    afx_msg void OnBnClickedLightHslTargetGet();
    afx_msg void OnBnClickedLightHslDefaultGet();
    afx_msg void OnBnClickedLightHslDefaultSet();
    afx_msg void OnBnClickedLightHslRangeGet();
    afx_msg void OnBnClickedLightHslRangeSet();
    afx_msg void OnBnClickedLightCtlGet();
    afx_msg void OnBnClickedLightCtlSet();
    afx_msg void OnBnClickedLightCtlTemperatureGet();
    afx_msg void OnBnClickedLightCtlTemperatureSet();
    afx_msg void OnBnClickedLightCtlDefaultGet();
    afx_msg void OnBnClickedLightCtlDefaultSet();
    afx_msg void OnBnClickedLightCtlTemperatureRangeGet();
    afx_msg void OnBnClickedLightCtlTemperatureRangeSet();
    afx_msg void OnBnClickedLightXylGet();
    afx_msg void OnBnClickedLightXylSet();
    afx_msg void OnBnClickedLightXylStatus();
    afx_msg void OnBnClickedLightXylDefaultGet();
    afx_msg void OnBnClickedLightXylDefaultSet();
    afx_msg void OnBnClickedLightXylRangeGet();
    afx_msg void OnBnClickedLightXylRangeSet();
    afx_msg void OnBnClickedLightXylTargetGet();
    afx_msg void OnBnClickedVsData();
    afx_msg void OnCbnSelchangeSensorMsg();
    afx_msg void OnBnClickedSensorMsgSend();
    afx_msg void OnBnClickedTcClearRpl();
    afx_msg void OnCbnSelchangeTestSelection();
    afx_msg void OnBnClickedUseDefaultTransTimeSend();
    afx_msg void OnBnClickedTcIvUpdateSetTestMode();
    afx_msg void OnBnClickedTcIvUpdateSetRecoveryMode();
    afx_msg void OnBnClickedLocationLocalGet();
    afx_msg void OnBnClickedLocationLocalSet();
    afx_msg void OnBnClickedLightLcModeGet();
    afx_msg void OnBnClickedLightLcModeSet();
    afx_msg void OnBnClickedLightLcOccupancyModeGet();
    afx_msg void OnBnClickedLightLcOccupancyModeSet();
    afx_msg void OnBnClickedLightLcOnOffGet();
    afx_msg void OnBnClickedLightLcOnOffSet();
    afx_msg void OnBnClickedLightLcPropertyGet();
    afx_msg void OnBnClickedLightLcPropertySet();
    afx_msg void OnBnClickedLightLcOccupancySet();
    afx_msg void OnBnClickedSceneRegisterGet();
    afx_msg void OnBnClickedSceneRecall();
    afx_msg void OnBnClickedSceneGet();
    afx_msg void OnBnClickedSceneStore();
    afx_msg void OnBnClickedSceneDelete();
    afx_msg void OnBnClickedTcHealthFaultsSet();
    afx_msg void OnBnClickedSchedulerRegisterGet();
    afx_msg void OnBnClickedSchedulerActionGet();
    afx_msg void OnBnClickedSchedulerActionSet();
    afx_msg void OnBnClickedSchedulerAdvanced();
    afx_msg void OnBnClickedTimeGet();
    afx_msg void OnBnClickedTimeSet();
    afx_msg void OnBnClickedTimezoneGet();
    afx_msg void OnBnClickedTimezoneSet();
    afx_msg void OnBnClickedTaiUtcDeltaGet();
    afx_msg void OnBnClickedTaiUtcDeltaSet();
    afx_msg void OnBnClickedTimeAuthorityGet();
    afx_msg void OnBnClickedTimeAuthoritySet();
    afx_msg void OnBnClickedTcCfgIdentity();
    afx_msg void OnBnClickedTcAccessPdu();
    afx_msg void OnBnClickedTcLpnSendSubsAdd();
    afx_msg void OnBnClickedTcLpnSendSubsDel();
    void OnBnClickedTcLpnSendSubsUpdt(BOOL add);
    virtual BOOL OnInitDialog();
};

/////////////////////////////////////////////////////////////////////////////
// CClientDialog

class CClientDialog : public CPropertySheet
{
    DECLARE_DYNAMIC(CClientDialog)

    // Construction
public:
    CClientDialog(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    CClientDialog(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

    // Attributes
public:
    UINT m_active_page;

    CClientControlDlg pageMain;
    CConfig pageConfig;
#ifndef NO_LIGHT_CONTROL
    CLightControl pageLight;
#endif
    CMeshPerformance pageMeshPerf;
    // Operations
public:

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(COptions)
    //}}AFX_VIRTUAL

    // Implementation
public:
    virtual ~CClientDialog();
    BOOL OnInitDialog();

    // Generated message map functions
protected:
    //{{AFX_MSG(COptions)
    //afx_msg void OnApplyNow();
    //afx_msg void OnOK();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
