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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "win_data_types.h"

#ifdef BSA
#include "bsa_mesh_api.h"
#endif

#include <QMutex>
#include <QWaitCondition>

extern bool g_bUseBsa;

extern "C" uint16_t wiced_hci_send(uint16_t opcode, uint8_t *p_buffer, uint16_t length);
class CommHelper
{
public:
    uint16_t SendWicedCommand(uint16_t opcode, uint8_t *p_buffer, uint16_t length);
};

extern CommHelper * m_ComHelper;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setActive();
    void OnClose();
    QSettings m_settings;
    void ProcessData(DWORD opcode, LPBYTE p_data, DWORD len);
    void closeEvent(QCloseEvent *event);
    bool OpenCommPort();
    bool SetupCommPortUI();
    bool connect_bsa();
    void saveNetParams();
    void disconnect_bsa();
    //WicedSerialPort *m_CommPort;

    QMutex m_write;
    QWaitCondition serial_read_wait;
    int PortWrite(unsigned char * data, DWORD Length);
    bool m_bPortOpen;
    bool mBsaConnected;
    void setDmUI(bool connected);
    int FindBaudRateIndex(int baud);
    void CloseCommPort();
    void ClearPort();
    //void serialPortError(QSerialPort::SerialPortError error);

signals:
   void HandleWicedEvent(unsigned int opcode, unsigned int len, unsigned char *p_data);
   void HandleTrace(QString *pTrace);
   void ScrollToTop();
   void NodeReset();
   void DfuProgress(int pos, int param);

public slots:
    void onHandleWicedEvent(unsigned int, unsigned int, unsigned char *);
    void handleReadyRead();
    void on_btnConnectComm();
    void onDeleteNetwork();
    void onCreateNetwork();
    void on_btnOpen();
    void on_btnClose();
    void on_btnConnect();
    void on_btnImport();
    void on_btnExport();
    void on_btnCreateGrp();
    void on_btnGrpDel();
    void on_btnScan();
    void on_btnProv();
    void on_btnReConfig();
    void on_btnConfigSub();
    void on_btnConfigPub();
    void on_btnGetStatus();
    void on_btnGetColor();
    void on_btnGetHue();
    void on_btnGetInfo();
    void on_btnGetLevel();
    void on_btnGetLight();
    void on_btnGetOnOff();
    void on_btnSetColor();
    void on_btnSetHue();
    void on_btnSetLevel();
    void on_btnSetLight();
    void on_btnSetOnOff();
    void on_btnSetVen();
    void on_btnSensorGet();
    void on_btnConfigSensor();
    void on_btnDfuStart();
    void on_btnDfuStop();
    void on_btnClearTrace();
    void onGrpIndexChanged(int);
    void onCbControlIndexChanged(int);
    void on_btnNodeReset();
    void on_btnIdentify();
    void on_btnFindDfuFile();
    void on_off_tmr_timeout();
    //void on_dfu_timer_timeout();

    // utility methods
    void processTrace(QString * trace);
    void processScrollToTop();

public:
    Ui::MainWindow *ui;

public:
    BOOL        m_scan_started;
    BOOL        m_bConnecting;
    BOOL        m_bScanning;
    uint8_t     m_dfuMethod;
    BOOL m_bConnected;
    QStringList m_strComPortsIDs;

    void SetDlgItemHex(DWORD id, DWORD val);
    DWORD GetHexValue(DWORD id, LPBYTE buf, DWORD buf_size);
    DWORD GetHexValue(char *szBuf, LPBYTE buf, DWORD buf_size);

    void DisplayCurrentGroup();
    void ProvisionCompleted();

    char   m_szCurrentGroup[80];

    BOOL    m_fw_download_active;
    BYTE    m_received_evt[261];
    DWORD   m_received_evt_len;
    HANDLE m_event;
#define STATE_IDLE                          0
#define STATE_LOCAL_GET_COMPOSITION_DATA    1
#define STATE_LOCAL_ADD_APPLICATION_KEY     2
#define STATE_LOCAL_BIND_MODELS             3
#define STATE_REMOTE_GET_COMPOSITION_DATA   4
#define STATE_REMOTE_ADD_APPLICATION_KEY    5
#define STATE_REMOTE_BIND_MODELS            6
    int m_state;

//    CProgressCtrl m_Progress;
    LRESULT OnProgress(WPARAM completed, LPARAM total);
//    WSDownloader *m_pDownloader;
    LPBYTE m_pPatch;
    DWORD m_dwPatchSize;

    void ProcessUnprovisionedDevice(uint8_t *p_uuid, uint16_t oob, uint8_t *name, uint8_t name_len);
    void LinkStatus(uint8_t is_connected, uint32_t conn_id, uint16_t addr, uint8_t is_over_gatt);
    void ProcessUnprovisionedDevice(LPBYTE p_data, DWORD len);
    void ProcessVendorSpecificData(LPBYTE p_data, DWORD len);
};


class Worker : public QObject
{
    Q_OBJECT

public slots:
    void userial_read_thread();

signals:
    void finished();

public:
    DWORD ReadNewHciPacket(BYTE * pu8Buffer, int bufLen, int * pOffset);
    DWORD ReadPort(BYTE *lpBytes, DWORD dwLen);
};

#endif // MAINWINDOW_H
