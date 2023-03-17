/*
 * Copyright 2016-2023, Cypress Semiconductor Corporation (an Infineon company) or
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

/*
 * Sample MCU application for implemeting serial port read/write on Linux OS.
 */

#include <QObject>
#include <QDebug>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hci_control_api.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QThread>

bool OpenSerialPort(unsigned int baudRate, char * str_port_name);
void TraceHciPkt(BYTE type, BYTE *buffer, USHORT length, USHORT serial_port=1);
extern "C"
{
extern void Log(const char *fmt, ...);
}

extern bool m_bClosing;
extern MainWindow * gMainWindow;

// valid baud rates
int as32BaudRate[] =
{
    115200,
    921600,
    3000000,
#ifndef __MACH__
    4000000
#endif
};

static QSerialPort * p_qt_serial_port;

bool MainWindow::SetupCommPortUI()
{
    // read settings for baudrate, serial port and flow-ctrl
    int baud_rate = m_settings.value("baud_rate",921600).toInt();
    QString comm_port = m_settings.value("comm_port","").toString();

    // get list of all available serial ports
    int port_inx = -1;
    m_strComPortsIDs.clear();
    m_strComPortsIDs.append("<select port>"); // dummy string to match combo box
    QList<QSerialPortInfo> port_list = QSerialPortInfo::availablePorts();
    for (int i =0; i < port_list.size(); i++)
    {
        QString strName = port_list.at(i).portName();
        QString strDesc =  port_list.at(i).description();
        strName += " (" + strDesc + ")";
        QString strPortID = port_list.at(i).systemLocation();

        // m_strComPortsIDs contains serial port ID used to open the port
        m_strComPortsIDs.append(strPortID);

        // cbCommport contains friendly names
        ui->cbCommPort->addItem(strName, strPortID);
    }

    if (g_bUseBsa)
    {
        // m_strComPortsIDs contains serial port ID used to open the port
        m_strComPortsIDs.append("0");

        // cbCommport contains friendly names
        ui->cbCommPort->addItem("BSA SERVER", "0");
    }

    if ( -1 != (port_inx = ui->cbCommPort->findText(comm_port)))
    {
        ui->cbCommPort->setCurrentIndex(port_inx);
    }

    // populate dropdown list of baud rates
    QString strBaud;
    int baud_inx = (sizeof(as32BaudRate) / sizeof(as32BaudRate[0])) - 1; // select default baud rate as highest allowed
    for (int i = 0; i < (int) (sizeof(as32BaudRate) / sizeof(as32BaudRate[0])); i++)
    {
        strBaud.sprintf( "%d", as32BaudRate[i]);
        ui->cbBaudRate->addItem(strBaud,as32BaudRate[i]);
        if (as32BaudRate[i] == baud_rate)
            baud_inx = i;
    }
    ui->cbBaudRate->setCurrentIndex(baud_inx);
    return true;
}

// Send WICED HCI commmand to embedded device
int MainWindow::PortWrite(unsigned char * data, DWORD len)
{
    qint64 written = 0;

    if(p_qt_serial_port)
    {
            qint64 written = 0;

         written = p_qt_serial_port->write((const char *)data, len);
         p_qt_serial_port->flush();
    }

    return written;
}

void MainWindow::setDmUI(bool connected)
{
    ui->btnConnectComm->setText(connected ? "Disconnect" : "Connect");
}

// User clicked button to open or close serial port
void MainWindow::on_btnConnectComm()
{
    if (ui->cbCommPort->currentText() == "BSA SERVER")
    {
        if (mBsaConnected)
            disconnect_bsa();
        else
            connect_bsa();
        setDmUI(mBsaConnected);
        return;
    }
    // If port is not open, open it
    if(!m_bPortOpen)
    {
        ui->btnConnectComm->setEnabled(false);
        bool bopen = OpenCommPort();
        ui->btnConnectComm->setEnabled(true);

        if(!bopen)
        {
            QMessageBox(QMessageBox::Information, "Serial Port", "Error opening serial port", QMessageBox::Ok).exec();
        }
        else
        {
            ui->cbCommPort->setEnabled(false);
            ui->btnConnectComm->setText("Disconnect");
            ui->cbBaudRate->setEnabled(false);
        }
    }
    // Close port if open
    else
    {
        ui->btnConnectComm->setText("Connect");
        ClearPort();
    }
    setDmUI(m_bPortOpen);
}

// Open and setup serial port
bool MainWindow::OpenCommPort()
{
    int baud_rate = ui->cbBaudRate->currentText().toInt();

    QString comm_port = ui->cbCommPort->currentData().toString();
    m_settings.setValue("port",comm_port);

    if (comm_port == "BSA SERVER")
    {

    }

    unsigned int serialPortBaudRate = ui->cbBaudRate->currentText().toInt();

    if (m_bPortOpen = OpenSerialPort(serialPortBaudRate,(char *)comm_port.toStdString().c_str()))
    {
        Log("Opened %s at speed: %u", comm_port.toStdString().c_str(), serialPortBaudRate);
    }
    else
    {
        Log("Error opening serial port %s: Error number %d", comm_port.toStdString().c_str(),errno);
    }

    return m_bPortOpen;
}

void MainWindow::CloseCommPort()
{
    m_bClosing = true;
    serial_read_wait.wakeAll();
    m_bPortOpen = false;
    if (p_qt_serial_port)
        if (p_qt_serial_port->isOpen())
            p_qt_serial_port->close();
}

// Clear port and UI
void MainWindow::ClearPort()
{
    CloseCommPort();
    QThread::sleep(1);

    Log("Serial port closed.");

    ui->cbCommPort->setEnabled(true);
    ui->cbBaudRate->setEnabled(true);

    ui->btnConnectComm->setText("Open Port");
}

DWORD Worker::ReadNewHciPacket(BYTE * pu8Buffer, int bufLen, int * pOffset)
{
    int dwLen, len = 0, offset = 0;

    dwLen = ReadPort(pu8Buffer, 1);

    if ((int)dwLen <= 0 || m_bClosing)
        return (-1);

    offset++;

    switch (pu8Buffer[0])
    {
    case HCI_EVENT_PKT:
    {
        dwLen = ReadPort(&pu8Buffer[offset], 2);
        if(dwLen == 2)
        {
            len = pu8Buffer[2];
            offset += 2;
            Log("HCI_EVENT_PKT len %d", len);
        }
        else
            Log("error HCI_EVENT_PKT, needed 2 got %d", dwLen);
    }
        break;

    case HCI_ACL_DATA_PKT:
    {
        dwLen = ReadPort( &pu8Buffer[offset], 4);
        if(dwLen == 4)
        {
            len = pu8Buffer[3] | (pu8Buffer[4] << 8);
            offset += 4;
            Log("HCI_ACL_DATA_PKT, len %d", len);
        }
        else
            Log("error HCI_ACL_DATA_PKT needed 4 got %d", dwLen);
    }
        break;

    case HCI_WICED_PKT:
    {
        dwLen = ReadPort(&pu8Buffer[offset], 4);
        if(dwLen == 4)
        {
            len = pu8Buffer[3] | (pu8Buffer[4] << 8);
            offset += 4;
        }
        else
            Log("error HCI_WICED_PKT,  needed 4 got %d", dwLen);
    }
        break;
    }

    if(len > 1024)
    {
        Log("bad packet length %d", len);
        return -1; // bad packet
    }

    if (len)
    {
        DWORD lenRd = (len < (DWORD)(bufLen-offset)) ? len : (DWORD)(bufLen-offset);
        dwLen = ReadPort( &pu8Buffer[offset], lenRd);
        if(dwLen != lenRd)
            Log("read error to read %d, read %d", lenRd, dwLen);
    }

    *pOffset = offset;

    return len;
}

void Worker::userial_read_thread()
{
    unsigned char au8Hdr[1024 + 6];
    memset(au8Hdr, 0, 1030);
    int           offset = 0, pktLen;
    int           packetType;

    // While the port is not closed, keep reading
    while (!m_bClosing)
    {
        memset(au8Hdr, 0, 1030);
        offset = 0;
        // Read HCI packet
        pktLen = ReadNewHciPacket(au8Hdr, sizeof(au8Hdr), &offset);
        if (m_bClosing || pktLen < 0) // skip this
            break;

        if (pktLen + offset == 0)
            continue;

        packetType = au8Hdr[0];
        if (HCI_WICED_PKT == packetType)
        {
            DWORD channel_id = au8Hdr[1] | (au8Hdr[2] << 8);
            DWORD len = au8Hdr[3] | (au8Hdr[4] << 8);

            if (len > 1024)
            {
                Log("Skip bad packet %d", len);
                continue;
            }

            BYTE * pBuf = NULL;

            if(len)
            {
                // malloc and create a copy of the data.
                //  MainWindow::onHandleWicedEvent deletes the data
                pBuf = (BYTE*)malloc(len);
                memcpy(pBuf, &au8Hdr[5], len);
            }
        // send it to main thread
        // Log("read_serial_thread: send to main, opcode=%d, len = %d", channel_id, len);
        emit gMainWindow->HandleWicedEvent(channel_id, len, pBuf);
        }
    }
    emit finished();
}

bool OpenSerialPort(unsigned int baudRate, char * str_port_name)
{
    bool bopen = false;

    if(p_qt_serial_port)
    {
        p_qt_serial_port->close();
    }
    else
    {
        p_qt_serial_port = new QSerialPort();
    }

    QString serialPortName = str_port_name;
    p_qt_serial_port->setPortName(serialPortName);
    if (!p_qt_serial_port->setBaudRate(baudRate))
    {
        Log("baud rate failed: %d, trying 115200",baudRate);
        p_qt_serial_port->setBaudRate(baudRate=115200);
    }
    p_qt_serial_port->setFlowControl(QSerialPort::HardwareControl);
    p_qt_serial_port->setStopBits(QSerialPort::OneStop);
    p_qt_serial_port->setDataBits(QSerialPort::Data8);
    p_qt_serial_port->setParity(QSerialPort::NoParity);

    bopen = p_qt_serial_port->open(QIODevice::ReadWrite);
    if (bopen)
    {
        QThread * m_port_read_thread = new QThread;
        Worker * m_port_read_worker = new Worker();
        m_port_read_worker->moveToThread(m_port_read_thread);
        m_bClosing = false;
        gMainWindow->connect(p_qt_serial_port, SIGNAL(readyRead()), gMainWindow, SLOT(handleReadyRead()));
        gMainWindow->connect(m_port_read_thread, SIGNAL(started()), m_port_read_worker, SLOT(userial_read_thread()));
        gMainWindow->connect(m_port_read_worker, SIGNAL(finished()), m_port_read_thread, SLOT(quit()));
        gMainWindow->connect(m_port_read_worker, SIGNAL(finished()), m_port_read_worker, SLOT(deleteLater()));
        gMainWindow->connect(m_port_read_thread, SIGNAL(finished()), m_port_read_thread, SLOT(deleteLater()));
        m_port_read_thread->start();
    }

    return bopen;
}

// Read from serial port
DWORD Worker::ReadPort(BYTE *lpBytes, DWORD dwLen)
{
    BYTE * p = lpBytes;
    DWORD Length = dwLen;
    DWORD dwRead = 0;
    DWORD dwTotalRead = 0;
    char buff_temp[1030];

    if (!p_qt_serial_port)
        return 0;

    QMutex mutex;
    int retry_cnt = 0;
    // Loop here until request is fulfilled or port is closed
    while (Length > 0 && !m_bClosing)
    {
        if(m_bClosing)
            return 0;

        dwRead = p_qt_serial_port->read((char *)p,Length);

            if(m_bClosing)
                return 0;

            if((int)dwRead < 0)
            {
                Log("Error in port read");
                return -1;
            }

        if (dwRead < Length)
                {
            unsigned long ulTimeout = 200;
            mutex.lock();
             //serial_read_wait is set when there is more data or when the serial port is closed
            gMainWindow->serial_read_wait.wait(&mutex, ulTimeout);
            mutex.unlock();
        }

        p += dwRead;
        Length -= dwRead;
        dwTotalRead += dwRead;
    }

    return dwTotalRead;
}

void CloseCommPort()
{
    if(p_qt_serial_port)
        p_qt_serial_port->close();
}

void MainWindow::handleReadyRead()
{
    serial_read_wait.wakeAll();
}
