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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QFileDialog>
#include <QMutex>
#include <QTimer>
#include <wchar.h>
#include <stdarg.h>
#include <QMessageBox>
#include "add_defines.h"
#include "win_data_types.h"
#include "wiced_mesh_client.h"
#include <string.h>
#include "hci_control_api.h"
#include "wiced_bt_mesh_model_defs.h"
#ifdef __windows__
#include <winsock.h>
#else
#ifdef __linux__
//#include "mesh_client_script.h"
#endif
#endif
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#ifndef __windows__
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif
#include <errno.h>
#include "wiced_bt_mesh_event.h"
#define __ANDROID__ 1
#include "wiced_bt_mesh_provision.h"

#define APP_DUMP Log
#ifdef BSA
#include "add_defines.h"
#include "bt_types.h"
#include "app_utils.h"
extern "C"
{
    #include "app_mesh.h"
}
#endif

typedef unsigned int UINT32;
#define wiced_bt_free_buffer free

#define WICED_TRUE  1
#define APP_DEFAULT_UIPC_PATH "./"
int provision_test = 0;
DWORD provision_test_scan_unprovisioned_time;
DWORD provision_test_connect_unprovisioned_time;
DWORD provision_test_provision_start_time;
DWORD provision_test_connect_provisioned_time;
DWORD provision_test_config_start_time;
DWORD provision_test_reset_time;
BOOL  provision_test_bScanning;

const char *log_filename = "trace.txt";  // if you add full path make sure that directory exists, otherwise it will crash
// #define MESH_AUTOMATION_ENABLED TRUE
#if defined(MESH_AUTOMATION_ENABLED) && (MESH_AUTOMATION_ENABLED == TRUE)
#include "mesh_client_script.h"
extern void mesh_client_enqueue_and_check_event(tMESH_CLIENT_SCRIPT_EVENT event, void *p_params, UINT16 size_of_params);
#endif

extern "C"
{
       char *mesh_client_get_device_name(uint8_t *p_uuid);
extern char *mesh_client_get_device_name_(uint8_t *p_uuid);
extern void TraceHciPkt(BYTE type, BYTE *buffer, USHORT length, USHORT serial_port_index);
}
static unsigned int m_dwPatchSize = 0;
static BYTE * m_pPatch = NULL;

class ProgressBar
{
public:
    void SetPos(int pos);
    void SetRange32(int l,int h);

};
char provisioner_uuid[50];
static ProgressBar m_Progress;

int m_hWnd = 0;
#define MB_ICONERROR 0

static QMutex cs;

void EnterCriticalSection(QMutex * m)
{
    m->lock();
}

void LeaveCriticalSection(QMutex * m)
{
    m->unlock();
}

extern "C" {
extern int mesh_client_network_exists(char *mesh_name);
extern int mesh_client_group_delete(char *p_group_name);
extern uint8_t mesh_client_reset_device(const char *component_name);
}

static QTimer * g_on_off_tmr = NULL;

static void OnOffGet();
static unsigned int *p_control_test_results=NULL;
static unsigned int n_control_test_results_size=0;
static int control_test=0;
static int control_test_itterration = 0;
static DWORD control_test_start_time=0;

extern void TraceHciPkt(BYTE type, BYTE *buffer, USHORT length, USHORT serial_port_index);

static const WCHAR *szDeviceType[] =
{
    L"Unknown",
    L"On/Off Client",
    L"Level Client",
    L"On/Off Server",
    L"Level Server",
    L"Dimmable Light",
    L"Power Outlet",
    L"HSL Light",
    L"CTL Light",
    L"XYL Light",
    L"Vendor Specific",
};

typedef struct
{
    int is_gatt_proxy;
    int is_friend;
    int is_relay;
    int send_net_beacon;
    int relay_xmit_count;
    int relay_xmit_interval;
    int default_ttl;
    int net_xmit_count;
    int net_xmit_interval;
    int publish_credential_flag;       ///< Value of the Friendship Credential Flag
    int publish_ttl;                   ///< Default TTL value for the outgoing messages
    int publish_period;                ///< Period for periodic status publishing
    int publish_retransmit_count;      ///< Number of retransmissions for each published message
    int publish_retransmit_interval;   ///< Interval in milliseconds between retransmissions
} device_config_params_t;

device_config_params_t DeviceConfig = { 1, 1, 1, 1, 3, 100, 63, 3, 100, 0, 63, 0, 0, 500 };
bool m_bClosing=false;
void network_opened(uint8_t status);
/*extern "C" */ void unprovisioned_device(uint8_t *uuid, uint16_t oob, uint8_t *name_len, uint8_t name);
/*extern "C" */ void link_status(uint8_t is_connected, uint32_t conn_id, uint16_t addr, uint8_t is_over_gatt);
/*extern "C" */ void node_connect_status(uint8_t is_connected, char *p_device_name);
/*extern "C" */ void provision_status(uint8_t status, uint8_t *p_uuid);
/*extern "C" */ void database_changed(char *mesh_name);
/*extern "C" */ void component_info_status(uint8_t status, char *component_name, char *component_info);
/*extern "C" */ void onoff_status(const char *device_name, uint8_t present, uint8_t target, uint32_t remaining_time);
/*extern "C" */ void level_status(const char *device_name, int16_t present, int16_t target, uint32_t remaining_time);
/*extern "C" */ void lightness_status(const char *device_name, uint16_t present, uint16_t target, uint32_t remaining_time);
/*extern "C" */ void hsl_status(const char *device_name, uint16_t lightness, uint16_t hue, uint16_t saturation, uint32_t remaining_time);
/*extern "C" */ void ctl_status(const char *device_name, uint16_t present_lightness, uint16_t present_temperature, uint16_t target_lightness, uint16_t target_temperature, uint32_t remaining_time);
/*extern "C" */ void fw_distribution_status(uint8_t status, uint16_t current_block_num);

#define DFU_METHOD_PROXY_TO_ALL                     0
#define DFU_METHOD_APP_TO_ALL                       1
#define DFU_METHOD_APP_TO_DEVICE                    2

const WCHAR *dfuMethods[] = {
    L"Proxy DFU to all",
    L"Proxy DFU to device",
    L"App DFU to all",
    L"App DFU to device",
};

extern wiced_bool_t mesh_adv_scanner_open();
extern void mesh_adv_scanner_close(void);
extern "C" void mesh_client_advert_report(uint8_t *bd_addr, uint8_t addr_type, int8_t rssi, uint8_t *adv_data);
extern "C" {
extern char *wiced_bt_mesh_db_get_all_networks(void);
extern int app_mgr_open(char *uipc_path);
extern void app_mgr_close();
extern char *mesh_client_get_all_groups(char *in_group);
extern int mesh_client_move_component_to_group(const char *component_name, const char *from_group_name, const char *to_group_name);
extern int mesh_client_configure_publication(const char *component_name, uint8_t is_command, const char *method, const char *target_name, int publish_period);
//extern int mesh_client_dfu_get_status(mesh_client_dfu_status_t p_dfu_status_callback, uint32_t interval);
}

#define STR_TO_CHAR(A) (char *)(A.toStdString().c_str())
#define ED_TO_CHAR(A)   (char *)(A->text().toStdString().c_str())
DWORD GetHexValue(char * szVal, LPBYTE buf, DWORD buf_size);
QTimer * g_dfu_timer = NULL;

mesh_client_init_t mesh_client_init_callbacks =
{
    unprovisioned_device,
    provision_status,
    link_status,
    node_connect_status,
    database_changed,
    onoff_status,
    level_status,
    lightness_status,
    hsl_status,
    ctl_status,
};

void TraceHciPkt(BYTE type, BYTE *buffer, USHORT length, USHORT serial_port=1);
extern int FwDownload(char *sHCDFileName);
void FwDownloadProcessEvent(LPBYTE p_data, DWORD len);
extern "C" void wiced_hci_process_data(uint16_t opcode, uint8_t *p_buffer, uint16_t len);

static CommHelper gComHelper;
CommHelper * m_ComHelper = &gComHelper;
MainWindow * gMainWindow=NULL;
FILE *g_fpLogFile = NULL;

void ProgressBar::SetPos(int pos)
{
    if (gMainWindow)
        gMainWindow->ui->pbDfu->setValue(pos);
}

void ProgressBar::SetRange32(int l,int h)
{
    if (gMainWindow)
        gMainWindow->ui->pbDfu->setRange(l,h);
}

unsigned long GetTickCount();

int SendWicedCommand(uint16_t opcode, BYTE p_buffer, uint16_t length)
{
    return 0;
}

extern "C" uint16_t wiced_hci_send(uint16_t opcode, uint8_t *p_buffer, uint16_t length)
{
    return ((uint16_t)m_ComHelper->SendWicedCommand(opcode, p_buffer, length) != 0);
}

uint16_t CommHelper::SendWicedCommand(uint16_t opcode, uint8_t *p_buffer, uint16_t length)
{
    BYTE    data[1024];
    char    descr[30];
    int     header = 0;

    if (gMainWindow->m_bPortOpen)
        data[header++] = 0x19;

    data[header++] = opcode & 0xff;
    data[header++] = (opcode >> 8) & 0xff;
    data[header++] = length & 0xff;
    data[header++] = (length >> 8) & 0xff;

    memcpy(&data[header], p_buffer, length);

    if (gMainWindow->m_bPortOpen)
        gMainWindow->PortWrite(data, length + header);
#ifdef BSA
    else
        app_mesh_send_message(data, length + header);
#endif
    return 0;
}

void log_to_file(QString s)
{
    if (log_filename != NULL)
    {
        if (g_fpLogFile == NULL)
        {
            if (NULL == (g_fpLogFile = fopen(log_filename, "a")))
            {
                printf("fopen() failed: %d\n",errno);
                return;
            }
        }
        fprintf(g_fpLogFile , "%s\n", s.toStdString().c_str());
        fflush(g_fpLogFile );
    }
}

extern "C" void ods(char *fmt, ...)
{
    va_list cur_arg;
    va_start(cur_arg, fmt);
    char trace[1000];
    memset(trace, 0, sizeof(trace));
    vsprintf(trace, fmt, cur_arg);

    QString s = QDateTime::currentDateTime().toString("MM-dd-yyyy hh:mm:ss.zzz: ") + trace;
    va_end(cur_arg);

    // add trace to file and UI screen in UI thread
    emit gMainWindow->HandleTrace(new QString(s));
    log_to_file(s);
}

extern "C" void Log(const char *fmt, ...)
{
    va_list cur_arg;
    va_start(cur_arg, fmt);
    char trace[1000];
    memset(trace, 0, sizeof(trace));
    vsprintf(trace, fmt, cur_arg);

    QString s = QDateTime::currentDateTime().toString("MM-dd-yyyy hh:mm:ss.zzz: ") + trace;
    va_end(cur_arg);

    // add trace to file and UI screen in UI thread
    emit gMainWindow->HandleTrace(new QString(s));
    log_to_file(s);
    TraceHciPkt(0,(BYTE*)trace,strlen(trace));
}

void messageBox(int unused,const char * body, const char * title, int unused1)
{
    QString st(body);

    QMessageBox msg;
    msg.setText(st);
    msg.setWindowTitle(title);
    msg.setStandardButtons(QMessageBox::Ok);
    msg.exec();
}

// Show message box
void on_BSA_show_message_box(QString st)
{
    QMessageBox msg;
    msg.setText(st);

    int iRet = msg.exec();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
     m_settings("mesh_app.ini",QSettings::IniFormat),
     m_bPortOpen(false),
    ui(new Ui::MainWindow)
{
    gMainWindow = this;
    mBsaConnected = false;

    ui->setupUi(this);
    setFixedSize(this->size());
    srand(clock());
    connect(this, SIGNAL(HandleWicedEvent(unsigned int, unsigned int, unsigned char *)),this, SLOT(onHandleWicedEvent(unsigned int, unsigned int, unsigned char *)));
    connect(this, SIGNAL(HandleTrace(QString*)), this, SLOT(processTrace(QString*)), Qt::QueuedConnection);
    connect(this, SIGNAL(ScrollToTop()), this, SLOT(processScrollToTop()), Qt::QueuedConnection);
    connect(ui->btnCreateNetwork, SIGNAL(clicked()), this, SLOT(onCreateNetwork()));
    connect(ui->btnDeleteNetwork, SIGNAL(clicked()), this, SLOT(onDeleteNetwork()));
    connect(ui->btnOpen, SIGNAL(clicked()), this, SLOT(on_btnOpen()));
    connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(on_btnClose()));
    connect(ui->btnConnect, SIGNAL(clicked()), this, SLOT(on_btnConnect()));
    connect(ui->btnImport, SIGNAL(clicked()), this, SLOT(on_btnImport()));
    connect(ui->btnExport, SIGNAL(clicked()), this, SLOT(on_btnExport()));
    connect(ui->btnCreateGrp, SIGNAL(clicked()), this, SLOT(on_btnCreateGrp()));
    connect(ui->btnGrpDel, SIGNAL(clicked()), this, SLOT(on_btnGrpDel()));
    connect(ui->btnScan, SIGNAL(clicked()), this, SLOT(on_btnScan()));
    connect(ui->btnProv, SIGNAL(clicked()), this, SLOT(on_btnProv()));
    connect(ui->btnReConfig, SIGNAL(clicked()), this, SLOT(on_btnReConfig()));
    connect(ui->btnConfigSub, SIGNAL(clicked()), this, SLOT(on_btnConfigSub()));
    connect(ui->btnConfigPub, SIGNAL(clicked()), this, SLOT(on_btnConfigPub()));
    connect(ui->btnGetStatus, SIGNAL(clicked()), this, SLOT(on_btnGetStatus()));
    connect(ui->btnGetColor, SIGNAL(clicked()), this, SLOT(on_btnGetColor()));
    connect(ui->btnGetHue, SIGNAL(clicked()), this, SLOT(on_btnGetHue()));
    connect(ui->btnGetInfo, SIGNAL(clicked()), this, SLOT(on_btnGetInfo()));
    connect(ui->btnGetLevel, SIGNAL(clicked()), this, SLOT(on_btnGetLevel()));
    connect(ui->btnGetLight, SIGNAL(clicked()), this, SLOT(on_btnGetLight()));
    connect(ui->btnGetOnOff, SIGNAL(clicked()), this, SLOT(on_btnGetOnOff()));
    connect(ui->btnSetColor, SIGNAL(clicked()), this, SLOT(on_btnSetColor()));
    connect(ui->btnSetHue, SIGNAL(clicked()), this, SLOT(on_btnSetHue()));
    connect(ui->btnSetLevel, SIGNAL(clicked()), this, SLOT(on_btnSetLevel()));
    connect(ui->btnSetLight, SIGNAL(clicked()), this, SLOT(on_btnSetLight()));
    connect(ui->btnSetOnOff, SIGNAL(clicked()), this, SLOT(on_btnSetOnOff()));
    connect(ui->btnSetVen, SIGNAL(clicked()), this, SLOT(on_btnSetVen()));
    connect(ui->btn_dfu_start, SIGNAL(clicked()), this, SLOT(on_btnDfuStart()));
    connect(ui->btn_dfu_stop, SIGNAL(clicked()), this, SLOT(on_btnDfuStop()));
    connect(ui->btn_clear_trace, SIGNAL(clicked()), this, SLOT(on_btnClearTrace()));
    connect(ui->btnFindDfuFile, SIGNAL(clicked()), this ,SLOT(on_btnFindDfuFile()));
    connect(ui->btnConnectComm, SIGNAL(clicked()), this, SLOT(on_btnConnectComm()));
    connect(ui->btnNodeReset, SIGNAL(clicked()), this, SLOT(on_btnNodeReset()));
    connect(ui->btnIdentity, SIGNAL(clicked()), this, SLOT(on_btnIdentify()));
    connect(ui->cbControl, SIGNAL(currentIndexChanged(int)), this, SLOT(onCbControlIndexChanged(int)));
    connect(ui->btnGetSensor, SIGNAL(clicked()), this, SLOT(on_btnSensorGet()));
    connect(ui->btnConfigSensor, SIGNAL(clicked()), this, SLOT(on_btnConfigSensor()));

    ui->cbPushMethod->addItem("Proxy DFU to all");
    ui->cbPushMethod->addItem("Proxy DFU to device");
    ui->cbPushMethod->addItem("App DFU to all");
    ui->cbPushMethod->addItem("App DFU to device");

    ui->cbPubModel->addItem("Flooding Security");
    ui->cbPubModel->addItem("Friendship Security");
    ui->cbPubModel->setCurrentIndex(0);

    // g_dfu_timer = new QTimer(this);
    // connect(g_dfu_timer , SIGNAL(timeout()), this, SLOT(on_dfu_timer_timeout()));

    SetupCommPortUI();
    ui->cbOnOff->addItem("On");
    ui->cbOnOff->addItem("Off");
    m_szCurrentGroup[0] = 0;
    m_fw_download_active = FALSE;
    m_bScanning = FALSE;

    FILE *fp = fopen("NetParameters.bin", "rb");
    if (fp)
    {
        if(fread(&DeviceConfig, 1, sizeof(DeviceConfig), fp) !=  sizeof(DeviceConfig))
        {
            printf("fread failed");
        }
        fclose(fp);
    }

    setActive();
    m_scan_started = FALSE;
    m_bConnecting = FALSE;
    m_dfuMethod = 0;
    m_bConnected = FALSE;
    m_received_evt_len = 0;
    m_event = 0;
    m_state = STATE_IDLE;
    m_pPatch = NULL;
    m_dwPatchSize = 0;

    // Move to group is not implemented yet. Hide all related controls(widgets)
    ui->label_6->setVisible(false);
    ui->label_15->setVisible(false);
    ui->label_48->setVisible(false);
    ui->cbMoveDevice->setVisible(false);
    ui->cbFrom->setVisible(false);
    ui->cbToGrp->setVisible(false);
    ui->btnConfigSub->setVisible(false);

    // Hide Connect button because support for Connect/Disconnect isn't implemented yet.
    // It works fine without that button. If it is connected then it transmits through GATT. If disconnected then it transmits through ADV.
    ui->btnConnect->setVisible(false);

    Log("Mesh Client ready");
}

void MainWindow::saveNetParams()
{
    DeviceConfig.is_gatt_proxy = ui->chkGattProxy->isChecked();
    DeviceConfig.is_friend = ui->chkFriend->isChecked();
    DeviceConfig.is_relay = ui->chkRelay->isChecked();
    DeviceConfig.send_net_beacon = ui->chkNetBeacon->isChecked();
    DeviceConfig.relay_xmit_count = ui->edRetran->text().toInt();
    DeviceConfig.relay_xmit_interval = ui->edRelayInterval->text().toInt();
    DeviceConfig.default_ttl = ui->edDefTTL->text().toInt();
    DeviceConfig.net_xmit_count = ui->edNetRetran->text().toInt();
    DeviceConfig.net_xmit_interval = (USHORT)ui->edNetInterval->text().toInt();
    DeviceConfig.publish_credential_flag = (BYTE)ui->cbPubModel->currentIndex();
    DeviceConfig.publish_period = ui->edPubPeriod->text().toInt();
    DeviceConfig.publish_ttl = ui->edPubTTL->text().toInt();
    DeviceConfig.publish_retransmit_count = ui->edRetranCnt->text().toInt();
    DeviceConfig.publish_retransmit_interval = ui->edRetranInterval->text().toInt();

    FILE *fp = fopen("NetParameters.bin", "wb");
    if (fp)
    {
        fwrite(&DeviceConfig, 1, sizeof(DeviceConfig), fp);
        fclose(fp);
    }
}

bool MainWindow::connect_bsa()
{
    // Connect to BSA server
#ifdef BSA
    if (app_mgr_open(APP_DEFAULT_UIPC_PATH) == -1)
    {
        // if connection fails, show error message
        QString st("Could not connect to BSA server. Please make sure that bsa server is running from the same location as this app. Close this app and try again.");

        QMessageBox msg;
        msg.setText(st);
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();

        return false;
    }

    /* Enable Mesh */
    int iRet = app_mesh_enable(NULL);

    if(iRet != 0)
    {
        on_BSA_show_message_box("app_mesh_enable failed");
        return false;
    }
    mBsaConnected = true;
#endif
    return true;
}

void MainWindow::disconnect_bsa()
{
#ifdef BSA
    app_mgr_close();

    mBsaConnected = false;
#endif
}

extern "C" {
extern char *mesh_client_get_device_components(uint8_t *p_uuid);
}

void MainWindow::on_btnNodeReset()
{
    mesh_client_reset_device(STR_TO_CHAR(ui->cbControl->currentText()));
    DisplayCurrentGroup();
}

void MainWindow::on_btnFindDfuFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("OTA/BIN File"), "", tr("OTA Files (*.ota *.bin);;All files (*.*)"));
    if (fileName.length() == 0)
        return;
    ui->edFileName->setText(fileName);
}

void MainWindow::on_btnIdentify()
{
    mesh_client_identify(gMainWindow->ui->cbControl->currentText().toStdString().c_str(), 10);
}

void addNewProvUuid(QString str_uuid)
{
    if (str_uuid == "")
        return;
    if (-1 == gMainWindow->ui->cbProUUID->findText(str_uuid))
    {
        gMainWindow->ui->cbProUUID->addItem(str_uuid);
        gMainWindow->ui->cbProUUID->setCurrentIndex(gMainWindow->ui->cbProUUID->count()-1);
    }
}
#define INVALID_SOCKET  -1
typedef unsigned char BYTE;
typedef unsigned short USHORT;

static int log_sock = INVALID_SOCKET;

// mapping between wiced trace types and spy trace types (evt, cmd, rx data, tx data)
static int wiced_trace_to_spy_trace[] = { 0, 4, 3, 6, 7 };

void MainWindow::onHandleWicedEvent(unsigned int opcode, unsigned int len, unsigned char *p_data)
{
    ProcessData(opcode, p_data, len);
    free(p_data);
}
extern "C"
{
extern wiced_bt_mesh_event_t *wiced_bt_mesh_event_from_hci_header(uint8_t **p_buffer, uint16_t *len);
}

void ProcessScanReport(LPBYTE p_data, uint16_t len)
{
    wiced_bt_mesh_event_t *p_event = wiced_bt_mesh_event_from_hci_header(&p_data, &len);
    if (p_event == NULL)
        return;

    uint16_t provisioner_addr = p_event->src;
    int8_t   rssi = p_data[0];
    QString str_uuid;
    for (int i = 1; i < 17; i++)
        str_uuid += QString::asprintf("%02x ", p_data[i]);

    char *device_name = mesh_client_get_device_name(p_data+1);
    str_uuid += device_name;
    addNewProvUuid(str_uuid);
    uint16_t oob = p_data[17] + (p_data[18] << 8);
    uint32_t uri_hash = p_data[19] + ((uint32_t)p_data[20] << 8) + ((uint32_t)p_data[21] << 16) + ((uint32_t)p_data[22] << 24);

    QString str = QString::asprintf("From %04x RSSI:%d Unprovisioned Device UUID:", provisioner_addr, rssi);
    for (int i = 1; i < 17; i++)
        str += QString::asprintf("%02x ", p_data[i]);

    str += QString::asprintf("OOB:%x URI hash:%x", oob, uri_hash);
    Log((char *)str.toStdString().c_str());
    wiced_bt_free_buffer(p_event);
}

void MainWindow::ProcessData(DWORD opcode, LPBYTE p_data, DWORD len)
{
    if (opcode == HCI_CONTROL_EVENT_WICED_TRACE)
    {
        if (len >= 2)
        {
            if ((len > 2) && (p_data[len - 2] == '\n'))
            {
                p_data[len - 2] = 0;
                len--;
            }
            TraceHciPkt(0, p_data, (USHORT)len);
        }
        return;
    }
    else if (opcode == HCI_CONTROL_EVENT_HCI_TRACE)
    {
        TraceHciPkt(p_data[0] + 1, &p_data[1], (USHORT)(len - 1));
        return;
    }
    else if (opcode == HCI_CONTROL_MESH_EVENT_VENDOR_DATA)
    {
        ProcessVendorSpecificData(p_data, len);
    }
    else if (opcode == HCI_CONTROL_MESH_EVENT_PROVISION_SCAN_REPORT)
    {
        ProcessScanReport(p_data, (uint16_t)len);
        wiced_hci_process_data((uint16_t)opcode, p_data, (uint16_t)len);
    }
    else
    {
        wiced_hci_process_data((uint16_t)opcode, p_data, (uint16_t)len);
    }
}

void MainWindow::ProcessVendorSpecificData(LPBYTE p_data, DWORD len)
{
    DWORD i;
    USHORT src = p_data[0] + (p_data[1] << 8);
    USHORT app_key_idx = (USHORT)p_data[2] + ((USHORT)p_data[3] << 8);
    BYTE   element_idx = p_data[4];
    p_data += 5;
    len -= 5;

    QString strTrace;
    strTrace.sprintf("VS Data from addr:%x element:%x app_key_idx:%x %d bytes:", src, app_key_idx, element_idx, len);
    ui->lstTrace->addItem(strTrace);
    char buf[100];
    while (len != 0)
    {
        buf[0] = 0;
        for (i = 0; i < len && i < 32; i++)
            sprintf(&buf[strlen(buf)], "%02x ", p_data[i]);

        len -= i;
        if (len != 0)
            ui->lstTrace->addItem(buf);
    }
    ui->lstTrace->addItem(buf);
}

void MainWindow::LinkStatus(uint8_t is_connected, uint32_t conn_id, uint16_t addr, uint8_t is_over_gatt)
{
    Log("Link Status:%d", is_connected);

    m_bConnected = is_connected;
    if (m_bConnected)
        ui->btnConnect->setText("Disconnect");
    else
        ui->btnConnect->setText("Connect");
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveNetParams();

    m_settings.sync();
    m_settings.setValue("User",ui->edUser->text());
    m_settings.setValue("baud_rate",ui->cbBaudRate->currentText());
    m_settings.setValue("comm_port",ui->cbCommPort->currentText().toStdString().c_str());
    m_settings.setValue("identity_duration",ui->edIdDuration->text());
    m_settings.sync();

    CloseCommPort();
    if (p_control_test_results)
    {
        free(p_control_test_results);
        p_control_test_results = NULL;
    }
    QMainWindow::closeEvent(event);
}

void MainWindow::setActive()
{
    WCHAR szHostName[128];
    char name[128]={0};
    DWORD dw = 128;

    bool useBsa = m_settings.value("use_bsa",true).toBool();
    ui->edIdDuration->setText(m_settings.value("identity_duration",1).toString());
    unsigned int baud_rate = m_settings.value("baud_rate",3000000).toInt();
    int i;
    for (i = 0; i < ui->cbBaudRate->count(); i++)
    {
        if (baud_rate == (unsigned int)ui->cbBaudRate->itemText(i).toInt())
        {
            ui->cbBaudRate->setCurrentIndex(i);
            break;
        }
    }

    ui->cbBaudRate->setEnabled(true);
    ui->cbCommPort->setEnabled(true);

    if (i == ui->cbBaudRate->count())
    {
        ui->cbBaudRate->addItem(QString::number(baud_rate));
        ui->cbBaudRate->setCurrentIndex(ui->cbBaudRate->count()-1);
    }
    gethostname(name, 128);
    QString::fromStdString(name).toWCharArray(szHostName);
    //ui->edName->setText(name);

    ui->edUser->setText(m_settings.value("User",name).toString());
    ui->cbNetwork->clear();

    ui->chkFriend->setChecked(DeviceConfig.is_friend);
    ui->chkGattProxy->setChecked(DeviceConfig.is_gatt_proxy);
    ui->chkRelay->setChecked(DeviceConfig.is_relay );
    ui->chkNetBeacon->setChecked(DeviceConfig.send_net_beacon );

    ui->edRelayInterval->setText(QString::number(DeviceConfig.net_xmit_interval));
    ui->edDefTTL->setText(QString::number(DeviceConfig.default_ttl));
    ui->edPubPeriod->setText(QString::number(DeviceConfig.publish_period ));
    ui->edPubTTL->setText(QString::number(DeviceConfig.publish_ttl ));
    ui->edRetran->setText(QString::number(DeviceConfig.relay_xmit_count ));
    ui->edNetRetran->setText(QString::number(DeviceConfig.net_xmit_count));
    ui->edRetranCnt->setText(QString::number(DeviceConfig.publish_retransmit_count));
    ui->edNetInterval->setText(QString::number(DeviceConfig.net_xmit_interval ));
    ui->edRetranInterval->setText(QString::number(DeviceConfig.publish_retransmit_interval ));

    char *p_networks = wiced_bt_mesh_db_get_all_networks();//mesh_client_get_all_networks();
    char *p = p_networks;
    WCHAR szNetwork[80];
    int num_networks = 0;
    while (p != NULL && *p != NULL)
    {
        ui->cbNetwork->addItem(p);
        p += strlen(p) + 1;
        num_networks++;
    }
   // addNewProvUuid(QString((char *)provisioner_uuid));
    if (num_networks)
    {
        ui->cbNetwork->setCurrentIndex(0);
    }

    ui->chkGattProxy->setChecked(DeviceConfig.is_gatt_proxy);
    ui->chkFriend->setChecked(DeviceConfig.is_friend);
    ui->chkRelay->setChecked(DeviceConfig.is_relay);
    ui->chkNetBeacon->setChecked(DeviceConfig.send_net_beacon);
    ui->edRetran->setText(QString::number(DeviceConfig.relay_xmit_count));
    ui->edRelayInterval->setText(QString::number(DeviceConfig.relay_xmit_interval));
    ui->edDefTTL->setText(QString::number(DeviceConfig.default_ttl));
    ui->edNetRetran->setText(QString::number(DeviceConfig.net_xmit_count));
    ui->edNetInterval->setText(QString::number(DeviceConfig.net_xmit_interval));
    ui->cbPubModel->setCurrentIndex(DeviceConfig.publish_credential_flag);
    ui->edPubPeriod->setText(QString::number(DeviceConfig.publish_period));
    ui->edPubTTL->setText(QString::number(DeviceConfig.publish_ttl));
    ui->edRetranCnt->setText(QString::number(DeviceConfig.publish_retransmit_count));
    ui->edRetranInterval->setText(QString::number(DeviceConfig.publish_retransmit_interval));

    mesh_client_init(&mesh_client_init_callbacks);
    free(p_networks);
}

void MainWindow::OnClose()
{
    mesh_client_network_close();
}

// add trace to window
void MainWindow::processTrace(QString * trace)
{
    // Keep a max of 50 lines of traces in windows, otherwise
    // it slows down the rendering.
    if(ui->lstTrace->count() > 50)
    {
        QListWidgetItem *pRemove = ui->lstTrace->takeItem(0);
        delete pRemove;
    }

    ui->lstTrace->addItem(*trace);
    ui->lstTrace->scrollToBottom();
    ui->lstTrace->scrollToItem(ui->lstTrace->item( ui->lstTrace->count()));

    delete trace;
}

void MainWindow::processScrollToTop()
{
    ui->lstTrace->scrollToTop();
}

void MainWindow::onDeleteNetwork()
{
    QString strMeshName = ui->cbNetwork->currentText();
    QString strProvName = ui->edUser->text();

    if (strMeshName.length() == 0 || strProvName.length() == 0)
         messageBox(m_hWnd, "Provide mesh name and provisioner name", "Error", MB_ICONERROR);
     else if (!mesh_client_network_exists((char *)strMeshName.toStdString().c_str()))
         messageBox(m_hWnd, (char *)(strMeshName.toStdString().c_str()), "Network Already Exists", MB_ICONERROR);
     else
     {
         int res = mesh_client_network_delete(STR_TO_CHAR(strProvName), ui->cbProUUID->currentText().left(48).toStdString().c_str(), STR_TO_CHAR(strMeshName));
         if (res == MESH_CLIENT_SUCCESS)
         {
             Log("Network %s deleted", STR_TO_CHAR(strMeshName));
         }
         else
         {
             Log("Failed to delete network:%d", res);
         }
     }

}

void MainWindow::onCreateNetwork()
{
    QString strMeshName = ui->cbNetwork->currentText();
    QString strProvName = ui->edUser->text();

    if (strMeshName.length() == 0 || strProvName.length() == 0)
         messageBox(m_hWnd, "Provide mesh name and provisioner name", "Error", MB_ICONERROR);
     else if (mesh_client_network_exists((char *)strMeshName.toStdString().c_str()))
         messageBox(m_hWnd, "Network Already Exists",(char *)(strMeshName.toStdString().c_str()),  MB_ICONERROR);
     else
     {
         int res = mesh_client_network_create(STR_TO_CHAR(strProvName), ui->cbProUUID->currentText().left(48).toStdString().c_str(), STR_TO_CHAR(strMeshName));
         if (res == MESH_CLIENT_SUCCESS)
         {
             Log("Network %s created", STR_TO_CHAR(strMeshName));
             DisplayCurrentGroup();
         }
         else
         {
             Log("Failed to create network:%d", res);
         }
     }
}

void MainWindow::on_btnOpen()
{
    QString strMeshName = ui->cbNetwork->currentText();
    QString strProv = ui->edUser->text();
    char pUuid[100];
    strncpy(pUuid, provisioner_uuid, 100-1);
    if (ui->cbProUUID->currentText() != "")
        strncpy(pUuid, (char *)ui->cbProUUID->currentText().left(48).toStdString().c_str(), 100-1);
    if (mesh_client_network_open(STR_TO_CHAR(strProv), pUuid, STR_TO_CHAR(strMeshName), network_opened) != MESH_CLIENT_SUCCESS)
    {
        messageBox(m_hWnd,  "Network Does Not Exists",STR_TO_CHAR(strMeshName), MB_ICONERROR);
        return;
    }
    strncpy(m_szCurrentGroup,strMeshName.toStdString().c_str(), sizeof(m_szCurrentGroup)-1);

    DisplayCurrentGroup();
}

void MainWindow::on_btnClose()
{
    mesh_client_network_close();
}

void MainWindow::on_btnConnect()
{
}

void MainWindow::on_btnImport()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("JSON File"), "", tr("JSON Files (*.json)"));
    if (fileName.length() == 0)
        return;
    FILE *fJsonFile = fopen(STR_TO_CHAR(fileName), "rb");
    if (fJsonFile == NULL)
    {
        messageBox(0,"Failed to open the json file", "Error", 0);
        return;
    }

    // Load OTA FW file into memory
    fseek(fJsonFile, 0, SEEK_END);
    long json_string_size = ftell(fJsonFile);
    rewind(fJsonFile);

    if(json_string_size <= 0)
    {
        fclose(fJsonFile);
        return;
    }

    char *json_string = new char[json_string_size+1];
    if (json_string == NULL)
    {
        fclose(fJsonFile);
        return;
    }
    memset(json_string, 0, json_string_size+1);
    strcpy(json_string, "");

    if(fread(json_string, 1, json_string_size, fJsonFile) != (size_t)json_string_size)
    {
        Log("on_btnImport, fread error");
    }
    json_string[json_string_size] = 0;

    char *ifx_json_string = NULL;
    if (fileName.endsWith(".json", Qt::CaseInsensitive))
    {
        QString ifxFileName = fileName.insert(fileName.length() - 4, "ifx.");
        FILE *fIfxJsonFile = fopen(STR_TO_CHAR(ifxFileName), "rb");
        if (fIfxJsonFile != NULL)
        {
            fseek(fIfxJsonFile, 0, SEEK_END);
            json_string_size = ftell(fIfxJsonFile);
            rewind(fIfxJsonFile);

            ifx_json_string = new char[json_string_size+1];
            if (ifx_json_string != NULL)
            {
                fread(ifx_json_string, 1, json_string_size, fIfxJsonFile);
                ifx_json_string[json_string_size] = 0;
            }
            fclose(fIfxJsonFile);
        }
    }

    char *mesh_name;
    if ((mesh_name = mesh_client_network_import(ED_TO_CHAR(ui->edUser),
                                                ui->cbProUUID->currentText().left(48).toStdString().c_str(), json_string, ifx_json_string, network_opened)) == NULL)
    {
        messageBox(0,"Failed to import json file", "Error", 0);
    }
    else
    {
        Log("Network %s imported", mesh_name);

        ui->cbNetwork->clear();
        char *p_networks = mesh_client_get_all_networks();
        char *p = p_networks;

        int i = 0, sel = -1;
        while (p != NULL && *p != NULL)
        {
            ui->cbNetwork->addItem(p);
            p += strlen(p) + 1;
            i++;
            if (strcmp(p, mesh_name) == 0)
                sel = i;
        }
        if (sel >= 0)
        {
            ui->cbNetwork->setCurrentIndex(sel);
        }
        QString strMesh(mesh_name);
        strncpy(m_szCurrentGroup, strMesh.toStdString().c_str(), sizeof(m_szCurrentGroup)-1);

        DisplayCurrentGroup();
        wiced_bt_free_buffer(p_networks);
    }
    fclose(fJsonFile);
    delete[] json_string;
    if (ifx_json_string != NULL)
        delete[] ifx_json_string;
}

void MainWindow::on_btnExport()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("JSON File"), "", tr("JSON Files (*.json)"));
    if (fileName.length() == 0)
        return;

    char *json_string = mesh_client_network_export(STR_TO_CHAR(ui->cbNetwork->currentText()));
    if (json_string != NULL)
    {
        FILE *fJsonFile = fopen(STR_TO_CHAR(fileName), "w");
        if (fJsonFile == NULL)
        {
            messageBox(0,"Failed to open the json file", "Error", 0);
            free(json_string);
            return;
        }
        fwrite(json_string, 1, strlen(json_string), fJsonFile);
        fclose(fJsonFile);

        free(json_string);
    }
}

void MainWindow::on_btnCreateGrp()
{
    int res;
    char mesh_name[80], group[64], parent[64];
    strncpy(mesh_name,ui->cbNetwork->currentText().toStdString().c_str(), 80-1);
    strncpy(group,(char *)ui->edGroup->text().toStdString().c_str(), 64-1);
    strncpy(parent,(char *)ui->cbCurGrp->currentText().toStdString().c_str(), 64-1);
    ui->edGroup->setText("");

    res = mesh_client_group_create(group , parent );

    if (res == MESH_CLIENT_SUCCESS)
    {
        Log("Group %s created in group %s", group, parent);
        DisplayCurrentGroup();
    }
    else
    {
        Log("Failed to create group:%d", res);
    }
}

void MainWindow::on_btnGrpDel()
{
    mesh_client_group_delete(ED_TO_CHAR(ui->edGroup));
    DisplayCurrentGroup();
}

void MainWindow::on_btnScan()
{
    if (!m_bScanning)
    {
        provision_test_scan_unprovisioned_time = clock();
        ui->btnScan->setText("Stop Scanning");
        m_bScanning = TRUE;
        ui->cbProUUID->clear();
    }
    else
    {
        ui->btnScan->setText("Scan Unprovisioned");
        m_bScanning = FALSE;
    }

    Log("scan unprovisioned:%d", m_bScanning);
    mesh_client_scan_unprovisioned(m_bScanning, NULL);
}

BYTE ProcNibble(char n)
{
    if ((n >= '0') && (n <= '9'))
    {
        n -= '0';
    }
    else if ((n >= 'A') && (n <= 'F'))
    {
        n = ((n - 'A') + 10);
    }
    else if ((n >= 'a') && (n <= 'f'))
    {
        n = ((n - 'a') + 10);
    }
    else
    {
        n = (char)0xff;
    }
    return (n);
}

DWORD MainWindow::GetHexValue(char * szVal, LPBYTE buf, DWORD buf_size)
{
    char szbuf[1300] ={0};
    char *psz = szbuf;
    BYTE *pbuf = buf;
    DWORD res = 0;

    memset(buf, 0, buf_size);

    strncpy(szbuf, szVal, 1300-1);

    if (strlen(szbuf) == 1)
    {
        szbuf[2] = 0;
        szbuf[1] = szbuf[0];
        szbuf[0] = '0';
    }
    else if (strlen(szbuf) == 3)
    {
        szbuf[4] = 0;
        szbuf[3] = szbuf[2];
        szbuf[2] = szbuf[1];
        szbuf[1] = szbuf[0];
        szbuf[0] = '0';
    }
    for (DWORD i = 0; i < strlen(szbuf); i++)
    {
        if (isxdigit(psz[i]) && isxdigit(psz[i + 1]))
        {
            *pbuf++ = (ProcNibble(psz[i]) << 4) + ProcNibble(psz[i + 1]);
            res++;
            i++;
        }
    }
    return res;
}

void MainWindow::on_btnProv()
{
    uint8_t identify_duration = ui->edIdDuration->text().toInt();
    uint8_t uuid[16];
//    ui->edProUUID->text();

    GetHexValue((char*)ui->cbProUUID->currentText().left(48).toStdString().c_str(), uuid, 16);

    saveNetParams();

    mesh_client_set_device_config(NULL, DeviceConfig.is_gatt_proxy, DeviceConfig.is_friend, DeviceConfig.is_relay, DeviceConfig.send_net_beacon, DeviceConfig.relay_xmit_count, DeviceConfig.relay_xmit_interval, DeviceConfig.default_ttl, DeviceConfig.net_xmit_count, DeviceConfig.net_xmit_interval);
    mesh_client_set_publication_config(DeviceConfig.publish_credential_flag, DeviceConfig.publish_retransmit_count, DeviceConfig.publish_retransmit_interval, DeviceConfig.publish_ttl);
    //mesh_client_set_publication_config(DeviceConfig.publish_period, DeviceConfig.publish_credential_flag, DeviceConfig.publish_retransmit_count, DeviceConfig.publish_retransmit_interval, DeviceConfig.publish_ttl);

    //uint8_t mesh_client_provision(const char *device_name, const char *group_name, uint8_t *uuid, uint8_t identify_duration)
    ui->cbControl->clear();
    mesh_client_provision(STR_TO_CHAR(ui->cbProUUID->currentText().right(ui->cbProUUID->currentText().length()-48)),
                          STR_TO_CHAR(ui->cbCurGrp->currentText()),
                          uuid, identify_duration);
}

void MainWindow::on_btnReConfig()
{
    int sel = ui->cbRename->currentIndex();
    if (sel < 0)
        return;

    QString strOldName = ui->cbRename->currentText();

    if (ui->cbProUUID->currentText().length() > 48)
    {
        QString strNewName = ui->edNewName->text();
        ui->edNewName->setText("");

        mesh_client_rename(STR_TO_CHAR(strOldName), STR_TO_CHAR(strNewName));
        strOldName = strNewName;
        QString strUuidName = ui->cbProUUID->currentText().left(48) + strNewName;
        ui->cbProUUID->setCurrentText(strUuidName);
    }

    QString group_name = ui->cbCurGrp->currentText();
    saveNetParams();

    mesh_client_set_device_config(STR_TO_CHAR(strOldName), DeviceConfig.is_gatt_proxy, DeviceConfig.is_friend, DeviceConfig.is_relay, DeviceConfig.send_net_beacon, DeviceConfig.relay_xmit_count, DeviceConfig.relay_xmit_interval, DeviceConfig.default_ttl, DeviceConfig.net_xmit_count, DeviceConfig.net_xmit_interval);
    mesh_client_set_publication_config(DeviceConfig.publish_credential_flag, DeviceConfig.publish_retransmit_count, DeviceConfig.publish_retransmit_interval, DeviceConfig.publish_ttl);
    //mesh_client_set_publication_config(DeviceConfig.publish_period, DeviceConfig.publish_credential_flag, DeviceConfig.publish_retransmit_count, DeviceConfig.publish_retransmit_interval, DeviceConfig.publish_ttl);
    DisplayCurrentGroup();
}

void MainWindow::on_btnConfigSub()
{
    char device_name[80];
    char group_to_name[80] = { 0 };
    char group_from_name[80] = { 0 };
    WCHAR szDevName[80];
    WCHAR szGroupName[80];

    int sel = ui->cbMoveDevice->currentIndex();
    if (sel < 0)
        return;

    QString strDevName = ui->cbMoveDevice->currentText();
    QString strGroupToName , strGroupFromName;

    if ((sel = ui->cbToGrp->currentIndex()) >= 0)
    {
        strGroupToName = ui->cbToGrp->currentText();
    }

    if ((sel = ui->cbFrom->currentIndex()) >= 0)
    {
        strGroupFromName = ui->cbFrom->currentText();
    }
    if (strGroupFromName.length() == 0 && strGroupToName.length())
        mesh_client_add_component_to_group(STR_TO_CHAR(strDevName), STR_TO_CHAR(strGroupToName));
    else if ((strGroupFromName.length() != 0) && (strGroupToName.length() == 0))
        mesh_client_remove_component_from_group(STR_TO_CHAR(strDevName), STR_TO_CHAR(strGroupFromName));
    else if (strGroupFromName.length() && strGroupToName.length())
        mesh_client_move_component_to_group(STR_TO_CHAR(strDevName), STR_TO_CHAR(strGroupFromName), STR_TO_CHAR(strGroupToName));
}

void MainWindow::on_btnConfigPub()
{
    int sel = ui->cbUseDevice->currentIndex();

    if (sel < 0)
        return;

    QString szDevName = ui->cbUseDevice->currentText();
    if ((sel = ui->cbDevFrom->currentIndex()) < 0)
        return;

    char szPublishMethod[80], szPublishString[80];
    char *p;
    strncpy(szPublishString,STR_TO_CHAR(ui->cbDevFrom->currentText()), sizeof(szPublishMethod) - 1);

    uint8_t client_control;
    if (strncmp(szPublishString, "control \"", strlen("control \"")) == 0)
    {
        client_control = 1;
        p = &szPublishString[strlen("control \"")];
    }
    else
    {
        client_control = 0;
        p = &szPublishString[wcslen(L"send \"")];
    }
    int i;
    for (i = 0; *p != '\"'; i++)
        szPublishMethod[i] = *p++;
    szPublishMethod[i] = 0;
    char publish_method[80];
    strcpy(publish_method, szPublishMethod);
    if ((sel = ui->cbToGrp2->currentIndex()) < 0)
        return;

    char szPublishToName[80];
    strncpy(szPublishToName, STR_TO_CHAR(ui->cbToGrp2->currentText()), 80-1);

    // Get publication parameters from the dialog and tell Client that new parameters should be used
    int publish_credential_flag = ui->cbPubModel->currentIndex();
    int publish_period = ui->edPubPeriod->text().toInt();
    int publish_ttl = ui->edPubTTL->text().toInt();
    int publish_retransmit_count = ui->edRetranCnt->text().toInt();
    int publish_retransmit_interval = ui->edRetranInterval->text().toInt();

    mesh_client_set_publication_config(publish_credential_flag, publish_retransmit_count, publish_retransmit_interval, publish_ttl);
    mesh_client_configure_publication(STR_TO_CHAR(szDevName), client_control, publish_method, szPublishToName, publish_period);
}

void fw_distribution_status(uint8_t status, int current_block_num, int total_blocks)
{
}


void MainWindow::on_btnGetStatus()
{
    //mesh_client_dfu_get_status(NULL, 0, NULL, 0, &fw_distribution_status);
}

void MainWindow::on_btnGetColor()
{
    mesh_client_ctl_get(STR_TO_CHAR(ui->cbControl->currentText()));
}

void MainWindow::on_btnGetHue()
{
      mesh_client_hsl_get(STR_TO_CHAR(ui->cbControl->currentText()));
}

void component_info_status_callback(uint8_t status, char *component_name, char *component_info)
{
    Log("Component Info status:%d from %s Info:%s\n", status, component_name, component_info);
}

void MainWindow::on_btnGetInfo()
{
    mesh_client_get_component_info(STR_TO_CHAR(ui->cbControl->currentText()),
                                   component_info_status_callback);
}

void MainWindow::on_btnGetLevel()
{
    mesh_client_level_get(STR_TO_CHAR(ui->cbControl->currentText()));
}

void MainWindow::on_btnGetLight()
{
    mesh_client_lightness_get(STR_TO_CHAR(ui->cbControl->currentText()));
}

void MainWindow::on_btnGetOnOff()
{
    control_test = ui->edIterations->text().toInt();
    if (control_test)
    {
        if (g_on_off_tmr == NULL)
        {
            g_on_off_tmr = new QTimer(this);
            connect(g_on_off_tmr , SIGNAL(timeout()), this, SLOT(on_off_tmr_timeout()));
        }

        p_control_test_results = (unsigned int *)realloc(p_control_test_results,control_test * sizeof(DWORD));
        if (p_control_test_results != 0)
        {
            memset(p_control_test_results, 0, control_test * sizeof(DWORD));
        }
        control_test_itterration = 0;
        g_on_off_tmr->start(5000);
    }
    OnOffGet();
}

void MainWindow::on_btnSetColor()
{
    short target_lightness = ui->edLight->text().toInt();
    short target_temperature = ui->edColor1->text().toInt();
    short target_delta_uv = ui->edColor2->text().toInt();
    mesh_client_ctl_set(STR_TO_CHAR(ui->cbControl->currentText()), target_lightness, target_temperature, target_delta_uv, WICED_TRUE, DEFAULT_TRANSITION_TIME, 0);
}

void MainWindow::on_btnSetHue()
{
    short target_lightness = ui->edLight->text().toInt();
    short target_hue = ui->edHue1->text().toInt();
    short target_saturation = ui->edHue2->text().toInt();
    mesh_client_hsl_set(STR_TO_CHAR(ui->cbControl->currentText()), target_lightness, target_hue, target_saturation, WICED_TRUE, DEFAULT_TRANSITION_TIME, 0);
}

void MainWindow::on_btnSetLevel()
{
    short target_level = ui->edLevel->text().toInt();
    mesh_client_level_set(STR_TO_CHAR(ui->cbControl->currentText()), target_level, WICED_TRUE, DEFAULT_TRANSITION_TIME, 0);
}

void MainWindow::on_btnSetLight()
{
    short target_lightness = ui->edLight->text().toInt();
    mesh_client_lightness_set(STR_TO_CHAR(ui->cbControl->currentText()), target_lightness, WICED_TRUE, DEFAULT_TRANSITION_TIME, 0);
}

void control_test_status(unsigned long duration)
{
    if (control_test != 0)
    {
        g_on_off_tmr->stop();
        if (p_control_test_results != NULL)
        {
            p_control_test_results[control_test_itterration++] = duration;
        }
        if (--control_test == 0)
        {
            if (p_control_test_results != NULL)
            {
                char buf[1000] = {0};
                for (int i = 0; i < control_test_itterration; i++)
                    sprintf(&buf[i], "%d ", p_control_test_results[i]);

                Log("Control Test result %s", buf);
            }
        }
        else
        {
            g_on_off_tmr->start();
            OnOffGet();
        }
    }
}

void onoff_status(const char *device_name, uint8_t present, uint8_t target, uint32_t remaining_time)
{
    size_t name_len = device_name ? strlen(device_name) + 1 : 0;
    Log("%s OnOff state:%d\n", device_name, present);
    unsigned long duration = GetTickCount() - control_test_start_time;
    gMainWindow->ui->cbOnOff->setCurrentIndex(target ? 0 : 1);
    control_test_status(duration);
}

void MainWindow::on_off_tmr_timeout()
{
    control_test_status(0);
}

#ifndef __windows__
unsigned long GetTickCount()
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now))
        return 0;
    return (unsigned int)(now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0);
}
#endif

void OnOffGet()
{
    control_test_start_time = GetTickCount();
    mesh_client_on_off_get(STR_TO_CHAR(gMainWindow->ui->cbControl->currentText()));
}

void MainWindow::on_btnSetOnOff()
{
    int iterate = ui->edIdDuration->text().toInt();
    if (iterate)
    {
        if (g_on_off_tmr == NULL)
        {
            g_on_off_tmr = new QTimer(this);
            connect(g_on_off_tmr , SIGNAL(timeout()), this, SLOT(on_off_tmr_timeout()));
        }
        g_on_off_tmr->start(5000);
    }

    mesh_client_on_off_set(STR_TO_CHAR(ui->cbControl->currentText()), ui->cbOnOff->currentIndex() ? 0 : 1, WICED_TRUE, DEFAULT_TRANSITION_TIME, 0);
}

void MainWindow::on_btnSetVen()
{
    BYTE buffer[400];
    DWORD len = GetHexValue(ED_TO_CHAR(ui->edVendorData), buffer, sizeof(buffer));

//    mesh_client_vendor_data_set(STR_TO_CHAR(ui->cbControl->currentText()), buffer, (uint16_t)len);
    mesh_client_vendor_data_set(STR_TO_CHAR(ui->cbControl->currentText()), 0x131, 0x01, 0x01, 0x0, buffer, (uint16_t)len);

}

#include "sensorcfgdlg.h"
void MainWindow::on_btnConfigSensor()
{
    SensorCfgDlg dlg;
    dlg.setModal(true);
    dlg.exec();

}

void MainWindow::on_btnSensorGet()
{
    int sel;
    if ((sel = ui->cbSensor->currentIndex()) < 0)
        return;

    int property_id = (int)ui->cbSensor->currentData().toInt();
    mesh_client_sensor_get((char *)ui->cbControl->currentText().toStdString().c_str(), property_id);
}

void MainWindow::onCbControlIndexChanged(int ndx)
{
    ui->cbSensor->clear();

    int *property_ids = mesh_client_sensor_property_list_get((char *)ui->cbControl->currentText().toStdString().c_str());
    if (property_ids == NULL)
        return;
    int *p;
    char szPropertyId[80];
    for (p = property_ids; *p != 0; p++)
    {
        sprintf(szPropertyId, "%04X", *p);
        ui->cbSensor->addItem(szPropertyId,*p);
    }
    free(property_ids);
    if (ui->cbSensor->count() != 0)
        ui->cbSensor->setCurrentIndex(0);
}


#include "qthread.h"
void mssleep(int ms)
{
    QThread::msleep(ms);
}
void MainWindow::on_btnDfuStart()
{
    // Regardless of what are we trying to do, need to connect to a specific component
    // then OtaUpgradeContinue will be executed
    mesh_client_connect_component(STR_TO_CHAR(ui->cbControl->currentText()), 1, 10);
}

void MainWindow::on_btnDfuStop()
{
    //mesh_client_dfu_stop();
}

void MainWindow::on_btnClearTrace()
{
    ui->lstTrace->clear();
}

void MainWindow::onGrpIndexChanged(int)
{
    int sel = ui->cbCurGrp->currentIndex(); //p_current_group->GetCurSel();
    if (sel < 0)
        return;

    strncpy(m_szCurrentGroup, ui->cbCurGrp->currentText().toStdString().c_str(), sizeof(m_szCurrentGroup)-1);

    DisplayCurrentGroup();
}

void MainWindow::ProvisionCompleted()
{
    DisplayCurrentGroup();
}

MainWindow::~MainWindow()
{
    delete ui;
}

#if 0
const WCHAR* data2str(const UINT8* p_data, UINT32 len)
{
    UINT32 i;
    static WCHAR wbuf[32 * 3 + 4]; // 32 hex bytes with space delimiter and with terminating 0 and with possible "..."
    static char buf[32 * 3 + 4]; // 32 hex bytes with space delimiter and with terminating 0 and with possible "..."

    buf[0] = 0;
    for (i = 0; i < len && i < sizeof(buf) / sizeof(buf[0]) - 4; i++)
    {
        sprintf(&buf[i * 3], "%02x ", p_data[i]);
        //wsprintf(&buf[i * 3], L"%02x ", p_data[i]);
    }
    if (i < len)
        strcpy(&buf[i * 3],  "...");

    static QString strBuf(buf);
    return strBuf.toStdWString().c_str();
}
#endif

const WCHAR* keyidx2str(const UINT8* p_data, UINT32 len)
{
    static WCHAR buf[8 * 4 + 4]; // 8 key indexes(3 characters each) with space delimiter and with terminating 0 and with possible "..."
    WCHAR *p_out = buf;
    buf[0] = 0;

    while (len > 1)
    {
        if ((p_out - buf) >= (sizeof(buf) / sizeof(buf[0]) - 4))
            break;

        swprintf (p_out, 4, L"%03x ", LE2TOUINT12(p_data));

        p_out += 4;
        p_data++;
        len--;
        if (len < 2)
            break;
        if ((p_out - buf) >= (sizeof(buf) / sizeof(buf[0]) - 4))
            break;
        swprintf(p_out, 4, L"%03x ", LE2TOUINT12_2(p_data));
        p_out += 4;
        p_data += 2;
        len -= 2;
    }
    if (len > 1)
        wcsncpy (p_out, L"...", sizeof(buf) / sizeof(buf[0]) - (p_out - buf));

    return buf;
}

void MainWindow::ProcessUnprovisionedDevice(uint8_t *p_uuid, uint16_t oob, uint8_t *name, uint8_t name_len)
{
    char buf[512];
    char uuid[50] = { 0 };

    for (int i = 0; i < 16; i++)
        sprintf(&uuid[strlen(uuid)], "%02x ", p_uuid[i]);

    QString uuid_name = uuid;
    if (name_len != 0)
    {
        uuid_name += QString((char*)name);
    }

    addNewProvUuid(uuid_name);
    Log("Unprovisioned Device name/UUID: %s",uuid_name.toStdString().c_str());

    if (m_bScanning && provision_test && p_uuid[12] == 0x11 && p_uuid[13] == 0x11 && p_uuid[14] == 0x11 && p_uuid[15] == 0x11)
    {
        emit ui->btnScan->clicked();
        emit ui->btnProv->clicked();
    }
}


void unprovisioned_device(uint8_t *p_uuid, uint16_t oob, uint8_t *name, uint8_t name_len)
{
#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    tMESH_CLIENT_SCRIPT_UNPROVISIONED_DEVICE unprovisioned_device;
    memset(&unprovisioned_device, 0, sizeof(unprovisioned_device));
    unprovisioned_device.oob = oob;
    unprovisioned_device.name_len = name_len;
    memcpy(unprovisioned_device.uuid, p_uuid, sizeof(unprovisioned_device.uuid));
    memcpy(unprovisioned_device.name, name, name_len);
    mesh_client_enqueue_and_check_event(MESH_CLIENT_SCRIPT_EVT_UNPROVISIONED_DEVICE, &unprovisioned_device, sizeof (unprovisioned_device));
#endif
    gMainWindow->ProcessUnprovisionedDevice(p_uuid, oob, name, name_len);
}

void network_opened(uint8_t status)
{
    Log("Network opened");
}

/*
 * in general the application knows better when connection to the proxy is established or lost.
 * The only case when this function is called, when search for a node or a network times out.
 */
extern void link_status(uint8_t is_connected, uint32_t conn_id, uint16_t addr, uint8_t is_over_gatt)
{
    Log("link_status: is_connected: %d", is_connected);

    gMainWindow->LinkStatus(is_connected,conn_id,addr,is_over_gatt);
#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    // Hook location where callback received from the mesh core is queued and forwarded to the Mesh Automation Script
    tMESH_CLIENT_SCRIPT_CONNECT_STATUS connect_status;
    connect_status.is_connected = is_connected;
    connect_status.conn_id = conn_id;
    connect_status.addr = addr;
    connect_status.is_over_gatt = is_over_gatt;
    mesh_client_enqueue_and_check_event(MESH_CLIENT_SCRIPT_EVT_CONNECT_STATUS, &connect_status, sizeof(connect_status));

#endif
}

/*
 * Result of the componeent connect operation
 */
extern void node_connect_status(uint8_t status, char *p_device_name)
{
        switch (status)
        {
        case MESH_CLIENT_NODE_CONNECTED:
        Log("Node %s connected continue OTA upgrade", p_device_name);
        //gMainWindow->OnOtaUpgradeContinue();
            break;

        case MESH_CLIENT_NODE_WARNING_UNREACHABLE:
        Log("Node %s failed to connect", p_device_name);
            break;

        case MESH_CLIENT_NODE_ERROR_UNREACHABLE:
        Log("!!! Action Required Node %s unreachable", p_device_name);
            break;
        }
    }


struct timespec res;
#ifndef __windows__
uint64_t GetTickCount64()
{
    uint64_t time;
    clock_gettime(CLOCK_MONOTONIC,&res);
    time = ((uint64_t)1000 * res.tv_sec) +  (res.tv_nsec/1000000);
    return time;
}
#endif

void provision_status(uint8_t status, uint8_t *p_uuid)
{
    char buf[512];
    char *p_devices = mesh_client_get_device_components(p_uuid);

    sprintf(buf, "Provision status:%d Device UUID: ", status);

    for (int i = 0; i < 16; i++)
        sprintf(&buf[strlen(buf)], "%02x ", p_uuid[i]);
    strcat(buf, "\n");

    Log(buf);

    if (status == MESH_CLIENT_PROVISION_STATUS_CONNECTING)
    {
        provision_test_connect_unprovisioned_time = GetTickCount();
    }
    if (status == MESH_CLIENT_PROVISION_STATUS_PROVISIONING)
    {
        provision_test_provision_start_time = GetTickCount();
    }
    if (status == MESH_CLIENT_PROVISION_STATUS_END)
    {
        provision_test_connect_provisioned_time = GetTickCount();
    }
    if (status == MESH_CLIENT_PROVISION_STATUS_CONFIGURING)
    {
        provision_test_config_start_time = GetTickCount();
    }
    if (status == MESH_CLIENT_PROVISION_STATUS_SUCCESS)
    {
        provision_test_reset_time = GetTickCount();
    }

    if (status != MESH_CLIENT_PROVISION_STATUS_SUCCESS)
    {
        wiced_bt_free_buffer(p_devices);
        return;
    }

    for (char *p_component_name = p_devices; p_component_name != NULL && *p_component_name != 0; p_component_name += (strlen(p_component_name) + 1))
    {
        char *target_methods = mesh_client_get_target_methods(p_component_name);
        char *control_methods = mesh_client_get_control_methods(p_component_name);
        sprintf(buf, "Name:%s", p_component_name);
        Log(buf);
        if ((target_methods != NULL) && (target_methods[0] != 0))
        {
            strncpy(buf, "Can be controlled using: ", strlen("Can be controlled using: ")-1);
            for (char *p = target_methods; *p != 0; p = p + strlen(p) + 1)
            {
                strncat(buf, p, sizeof(buf) - strlen(buf) -1);
                strncat(buf, ", ",  sizeof(buf) - strlen(buf) -1);
            }
            Log(buf);
        }
        if ((control_methods != NULL) && (control_methods[0] != 0))
        {
            strncpy(buf, "Can control: ", strlen("Can control: ")-1);
            for (char *p = control_methods; *p != 0; p = p + strlen(p) + 1)
            {
                strncat(buf, p, sizeof(buf) - strlen(buf) -1);
                strncat(buf, ", ", sizeof(buf) - strlen(buf) -1);
            }
            Log(buf);
        }
        if(control_methods)
            wiced_bt_free_buffer(control_methods);
        if(target_methods)
            wiced_bt_free_buffer(target_methods);
    }

    gMainWindow->ProvisionCompleted();

#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    // Hook location where callback received from the mesh core is queued and forwarded to the Mesh Automation Script
    tMESH_CLIENT_SCRIPT_PROVISION_STATUS provision_status = { 0 };
    provision_status.status = status;
    if (p_uuid)
    {
        memcpy(&provision_status.uuid[0], p_uuid, sizeof(provision_status.uuid));
    }
    if (p_devices)
    {
        memcpy(&provision_status.name, p_devices, strlen(p_devices));
    }
    mesh_client_enqueue_and_check_event(MESH_CLIENT_SCRIPT_EVT_PROVISION_STATUS, &provision_status, sizeof(provision_status));
#endif
   wiced_bt_free_buffer(p_devices);
}

void database_changed(char *mesh_name)
{
    Log("database changed\n");
}

void level_status(const char *device_name, int16_t present, int16_t target, uint32_t remaining_time)
{
    Log("Level state:%s, %d, %d, %d", device_name ? device_name : "",
        present, target, remaining_time);
    gMainWindow->ui->edLevel->setText(QString::number(target));
#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    size_t name_len = (device_name == nullptr) ? 0: strlen(device_name);
    // Hook location where callback received from the mesh core is queued and forwarded to the Mesh Automation Script
    tMESH_CLIENT_SCRIPT_LEVEL_STATUS level_status = { 0 };
    if (device_name && name_len)
    {
        memcpy(&level_status.device_name, device_name, name_len);
    }
    level_status.present = present;
    level_status.target = target;
    level_status.remaining_time = remaining_time;
    mesh_client_enqueue_and_check_event(MESH_CLIENT_SCRIPT_EVT_LEVEL_STATUS, &level_status, sizeof(level_status));
#endif
}

void lightness_status(const char *device_name, uint16_t present, uint16_t target, uint32_t remaining_time)
{
    Log("lightness_status: %s, %d, %d, %d",
        device_name ? device_name : "",
        present, target, remaining_time);
    gMainWindow->ui->edLight->setText(QString::number(present));

#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    // Hook location where callback received from the mesh core is queued and forwarded to the Mesh Automation Script
    tMESH_CLIENT_SCRIPT_LIGHTNESS_STATUS lightness_status = { 0 };
    size_t name_len = (device_name == nullptr) ? 0: strlen(device_name);
    if (device_name && name_len)
    {
        memcpy(&lightness_status.device_name, device_name, name_len);
    }
    lightness_status.present = present;
    lightness_status.target = target;
    lightness_status.remaining_time = remaining_time;
    mesh_client_enqueue_and_check_event(MESH_CLIENT_SCRIPT_EVT_LIGHTNESS_STATUS, &lightness_status, sizeof(lightness_status));
#endif
}

void hsl_status(const char *device_name, uint16_t lightness, uint16_t hue, uint16_t saturation, uint32_t remaining_time)
{
    Log("%s Light:%d Hue:%d Sat:%d", device_name, lightness, hue, saturation);
    gMainWindow->ui->edHue1->setText(QString::number(hue));
    gMainWindow->ui->edHue2->setText(QString::number(saturation));

#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    // Hook location where callback received from the mesh core is queued and forwarded to the Mesh Automation Script
    tMESH_CLIENT_SCRIPT_HSL_STATUS hsl_status = { 0 };
    size_t name_len = (device_name == nullptr) ? 0: strlen(device_name);
    if (device_name && name_len)
    {
        memcpy(&hsl_status.device_name, device_name, name_len);
    }
    hsl_status.lightness = lightness;
    hsl_status.hue = hue;
    hsl_status.saturation = saturation;
    hsl_status.remaining_time = remaining_time;
    mesh_client_enqueue_and_check_event(MESH_CLIENT_SCRIPT_EVT_HSL_STATUS, &hsl_status, sizeof(hsl_status));
#endif
}

void ctl_status(const char *device_name, uint16_t present_lightness, uint16_t present_temperature, uint16_t target_lightness, uint16_t target_temperature, uint32_t remaining_time)
{
    Log("%s present Light/Temp:%d/%d target Light/Temp:%d/%d", device_name, present_lightness, present_temperature, target_lightness, target_temperature);
    gMainWindow->ui->edColor1->setText(QString::number(present_lightness));
    gMainWindow->ui->edColor2->setText(QString::number(present_temperature));

#if defined( MESH_AUTOMATION_ENABLED ) && (MESH_AUTOMATION_ENABLED == TRUE)
    // Hook location where callback received from the mesh core is queued and forwarded to the Mesh Automation Script
    tMESH_CLIENT_SCRIPT_CTL_STATUS ctl_status = { 0 };
    size_t name_len = (device_name == nullptr) ? 0: strlen(device_name);
    if (device_name && name_len)
    {
        memcpy(&ctl_status.device_name, device_name, name_len);
    }
    ctl_status.present_lightness = present_lightness;
    ctl_status.present_temperature = present_temperature;
    ctl_status.target_lightness = target_lightness;
    ctl_status.target_temperature = target_temperature;
    ctl_status.remaining_time = remaining_time;
    mesh_client_enqueue_and_check_event(MESH_CLIENT_SCRIPT_EVT_CTL_STATUS, &ctl_status, sizeof(ctl_status));
#endif
}

void MainWindow::DisplayCurrentGroup()
{
    int cur_group = ui->cbCurGrp->currentIndex();
    ui->cbCurGrp->clear();
    ui->cbRename->clear();
    ui->cbMoveDevice->clear();
    ui->cbToGrp->clear();
    ui->cbControl->clear();
    ui->cbFrom->clear();
    ui->cbToGrp2->clear();

    QString strName = ui->cbNetwork->currentText();
    ui->cbCurGrp->addItem(strName);
    ui->cbControl->addItem(strName);
    ui->cbMoveDevice->addItem(strName);

    ui->cbToGrp2->addItem("none");
    ui->cbToGrp2->addItem("all-nodes");
    ui->cbToGrp2->addItem("all-proxies");
    ui->cbToGrp2->addItem("all_friends");
    ui->cbToGrp2->addItem("all-relays");
    ui->cbToGrp2->addItem("this-device");

    char *p;
    char *p_groups = mesh_client_get_all_groups(NULL);
    for (p = p_groups; p != NULL && *p != 0; p += (strlen(p) + 1))
    {
        ui->cbCurGrp->addItem(p);
        ui->cbToGrp2->addItem(p);
        ui->cbControl->addItem(p);
        ui->cbUseDevice->addItem(p);
        ui->cbRename->addItem(p);
    }
    free(p_groups);

    if ((cur_group >= 0) && (ui->cbCurGrp->count() >= cur_group))
        ui->cbCurGrp->setCurrentIndex(cur_group);
    else if (ui->cbCurGrp->count() > 0)
        ui->cbCurGrp->setCurrentIndex(0);

    // get groups and components for the current group
    QString group_name = QString(m_szCurrentGroup);

    Log("Current Group: %s\n", m_szCurrentGroup);
    Log("Groups:\n");
    p_groups = mesh_client_get_all_groups(STR_TO_CHAR(group_name));
    for (p = p_groups; p != NULL && *p != 0; p += (strlen(p) + 1))
    {
        Log("%s\n", p);
    }
    free(p_groups);

    Log("Components:\n");
    char *p_components = mesh_client_get_group_components(STR_TO_CHAR(group_name));
    for (p = p_components; p != NULL && *p != 0; p += (strlen(p) + 1))
    {
        Log("%s\n", p);
        ui->cbRename->addItem(p);

        uint8_t component_type = mesh_client_get_component_type(p);
        switch (component_type)
        {
        case DEVICE_TYPE_GENERIC_ON_OFF_SERVER:
        case DEVICE_TYPE_GENERIC_LEVEL_SERVER:
        case DEVICE_TYPE_LIGHT_DIMMABLE:
//        case DEVICE_TYPE_POWER_OUTLET:
        case DEVICE_TYPE_LIGHT_HSL:
        case DEVICE_TYPE_LIGHT_CTL:
        case DEVICE_TYPE_LIGHT_XYL:
        case DEVICE_TYPE_SENSOR_SERVER:
            ui->cbMoveDevice->addItem(p);
            ui->cbUseDevice->addItem(p);
            ui->cbControl->addItem(p);
            ui->cbToGrp2->addItem(p);
            break;

        case DEVICE_TYPE_GENERIC_ON_OFF_CLIENT:
        case DEVICE_TYPE_GENERIC_LEVEL_CLIENT:
            ui->cbUseDevice->addItem(p);
            ui->cbControl->addItem(p);
            break;

        case DEVICE_TYPE_UNKNOWN:
        case DEVICE_TYPE_VENDOR_SPECIFIC:
            ui->cbMoveDevice->addItem(p);
            ui->cbUseDevice->addItem(p);
            ui->cbControl->addItem(p);
            ui->cbToGrp2->addItem(p);
            break;
        }
    }
    free(p_components);
}

#define wiced_timer_callback_fp wiced_timer_callback_t // TODO: remove. Only for backward compatability
// stubs for building in BSA repo
wiced_result_t wiced_init_timer(wiced_timer_t* p_timer, wiced_timer_callback_fp TimerCb, TIMER_PARAM_TYPE cBackparam, wiced_timer_type_t type)
{
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_deinit_timer(wiced_timer_t* p)
{
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_start_timer(wiced_timer_t* wt, uint32_t timeout)
{
    return WICED_ERROR;
}

wiced_result_t wiced_stop_timer(wiced_timer_t* wt)
{
    return WICED_ERROR;
}

extern "C" void wiced_bt_mesh_gatt_client_connection_state_changed(uint16_t conn_id, uint16_t mtu)
{
}

extern "C" void wiced_bt_mesh_remote_provisioning_connection_state_changed(uint16_t conn_id, uint16_t reason)
{
}
