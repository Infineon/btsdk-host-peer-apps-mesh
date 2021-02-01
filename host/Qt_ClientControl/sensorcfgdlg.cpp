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
#include "sensorcfgdlg.h"
#include "ui_sensorcfgdlg.h"
#include "wiced_mesh_client.h"
#include "qmutex.h"

static QMutex cs;

SensorCfgDlg::SensorCfgDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorCfgDlg)
{
    ui->setupUi(this);

    connect(ui->btnConfigPub, SIGNAL(clicked()), this, SLOT(onBtnConfigPub()));
    connect(ui->btnConfigCadence, SIGNAL(clicked()), this, SLOT(onBtnConfigCadence()));
    connect(ui->cbPublishTo, SIGNAL(currentIndexChanged(int)), this, SLOT(on_displayControls(int)));
    connect(ui->cbFastCadence, SIGNAL(currentIndexChanged(int)), this, SLOT(on_displayControls(int)));
    connect(ui->chbPubPeriod, SIGNAL(stateChanged(int)), this, SLOT(on_displayControls(int)));
    property_id = 0;
    init();
}

SensorCfgDlg::~SensorCfgDlg()
{
    delete ui;
}

void SensorCfgDlg::on_displayControls(int i)
{
    DisplayControls();
}
void SensorCfgDlg::onBtnConfigCadence()
{
    bool bFastCadence = ui->chbFastCadence->isChecked();
    bool bTriggerPub = ui->chbPubPeriod->isChecked();

    int new_fast_cadence_period_divisor = 1;
    uint32_t new_fast_cadence_low = 0;
    uint32_t new_fast_cadence_high = 0;

    if (bFastCadence)
    {
        new_fast_cadence_period_divisor = 1 << (ui->cbFastCadence->currentIndex()+ 1);
        int low = ui->edCadenceLow->text().toInt();
        int high = ui->edCadenceLow->text().toInt();
        bool bInside = ui->cbCadenceInOut->currentIndex() == 1;
        new_fast_cadence_low = bInside ? low : high;
        new_fast_cadence_high = bInside ? high : low;
    }

    uint32_t new_trigger_delta_up = 0;
    uint32_t new_trigger_delta_down = 0;
    uint8_t  new_trigger_type = 0;

    if (bTriggerPub)
    {
        new_trigger_delta_up = ui->edtriggerUp->text().toInt();
        new_trigger_delta_down = ui->edTriggerDown->text().toInt();
    }
    new_trigger_type = ui->cbTriggerType->currentIndex() == 1;
    uint32_t min_interval = ui->edSendInterval->text().toInt() * 1000;
    cs.lock();
    mesh_client_sensor_cadence_set(component_name, property_id, new_fast_cadence_period_divisor, new_trigger_type,
        new_trigger_delta_down, new_trigger_delta_up, min_interval, new_fast_cadence_low, new_fast_cadence_high);
    cs.unlock();
}

void SensorCfgDlg::onBtnConfigPub()
{
    bool bSendPeriodic = ui->chbPubPeriod->isChecked();

    const char *publish_to_name = mesh_client_get_publication_target(component_name, FALSE, "SENSOR");
    int current_period = mesh_client_get_publication_period(component_name, FALSE, "SENSOR");

    int new_period = 0;
    if (bSendPeriodic)
        new_period = ui->edPubFreq->text().toInt();
    char * new_name = (char *)ui->cbPublishTo->currentText().toStdString().c_str();

    if (publish_to_name == NULL || new_name == NULL)
        return;

    if ((current_period != new_period) || (strcmp(publish_to_name, new_name) != 0))
    {
        cs.lock();
        mesh_client_configure_publication(component_name, 0, "SENSOR", new_name, new_period);
        cs.unlock();
    }
}

void SensorCfgDlg::init()
{
    ui->cbPublishTo->clear();
    ui->cbPublishTo->addItem("none");
    ui->cbPublishTo->addItem("all-nodes");
    ui->cbPublishTo->addItem("all-proxies");
    ui->cbPublishTo->addItem("all_friends");
    ui->cbPublishTo->addItem("all-relays");
    ui->cbPublishTo->setCurrentIndex(0);

    ui->edPubFreq->setText("10000");

    char *p;
    char *p_groups = mesh_client_get_all_groups(NULL);
    for (p = p_groups; p != NULL && *p != 0; p += (strlen(p) + 1))
    {
        ui->cbPublishTo->addItem(p);
    }
    free(p_groups);

    const char *name = mesh_client_get_publication_target(component_name, FALSE, "SENSOR");
    if (name == NULL)
        return;
    int period = mesh_client_get_publication_period(component_name, FALSE, "SENSOR");

    for (int sel = 0; sel < ui->cbPublishTo->count(); sel++)
    {
        char * item_name = (char *)ui->cbPublishTo->itemText(sel).toStdString().c_str();
        if (strcmp(name, item_name) == 0)
        {
            ui->cbPublishTo->setCurrentIndex(sel);
            break;
        }
    }
    if (period == 0)
    {
        ui->chbPubPeriod->setCheckState(Qt::Unchecked);
        ui->chbFastCadence->setCheckState(Qt::Unchecked);
    }
    else
    {
        ui->chbPubPeriod->setCheckState(Qt::Checked);
        ui->edPubFreq->setText(QString::number(period));
    }

    uint16_t fast_cadence_period_divisor;
    wiced_bool_t trigger_type;
    uint32_t trigger_delta_down, trigger_delta_up;
    uint32_t min_interval, fast_cadence_low, fast_cadence_high;

    if (mesh_client_sensor_cadence_get(component_name, property_id, &fast_cadence_period_divisor, &trigger_type,
        &trigger_delta_down, &trigger_delta_up, &min_interval, &fast_cadence_low, &fast_cadence_high) != MESH_CLIENT_SUCCESS)
    {
        ui->chbPubPeriod->setCheckState(Qt::Unchecked);
        ui->chbTriggers->setCheckState(Qt::Unchecked);
    }
    else
    {
        if (fast_cadence_period_divisor > 1)
        {
            ui->chbPubPeriod->setCheckState(Qt::Checked);

            if (fast_cadence_period_divisor < 4)
                ui->cbFastCadence->setCurrentIndex(0);
            else if (fast_cadence_period_divisor < 8)
                ui->cbFastCadence->setCurrentIndex(1);
            else if (fast_cadence_period_divisor < 16)
                ui->cbFastCadence->setCurrentIndex(2);
            else if (fast_cadence_period_divisor < 32)
                ui->cbFastCadence->setCurrentIndex(3);
            else if (fast_cadence_period_divisor < 64)
                ui->cbFastCadence->setCurrentIndex(4);
            else if (fast_cadence_period_divisor < 128)
                ui->cbFastCadence->setCurrentIndex(5);
            else if (fast_cadence_period_divisor < 256)
                ui->cbFastCadence->setCurrentIndex(6);
            else if (fast_cadence_period_divisor < 512)
                ui->cbFastCadence->setCurrentIndex(7);
            else if (fast_cadence_period_divisor < 1024)
                ui->cbFastCadence->setCurrentIndex(8);
            else
                ui->cbFastCadence->setCurrentIndex(9);

            if (fast_cadence_low > fast_cadence_high)
            {
                ui->cbCadenceInOut->setCurrentIndex(1);
                ui->edCadenceLow->setText(QString::number(fast_cadence_high));
                ui->edCadenceHigh->setText(QString::number(fast_cadence_low));
           }
            else
            {
                ui->cbCadenceInOut->setCurrentIndex(0);
                ui->edCadenceLow->setText(QString::number(fast_cadence_low));
                ui->edCadenceHigh->setText(QString::number(fast_cadence_high));
            }
        }
        if ((trigger_delta_down != 0) || (trigger_delta_up != 0))
        {
            ui->chbTriggers->setCheckState(Qt::Checked);
            ui->edtriggerUp->setText(QString::number(trigger_delta_up));
            ui->edTriggerDown->setText(QString::number(trigger_delta_down));
        }
        ui->cbTriggerType->setCurrentIndex(trigger_type);
        ui->edSendInterval->setText(QString::number(min_interval/1000));
    }
    DisplayControls();
}

void SensorCfgDlg::DisplayControls()
{
    int sel = ui->cbPublishTo->currentIndex();

    bool bSendPeriodic = ui->chbPubPeriod->isChecked();;
    bool bFastCadence = ui->chbFastCadence->isChecked();
    bool bTriggerPub = ui->chbTriggers->isChecked();

    ui->chbPubPeriod->setEnabled(sel != 0);
    ui->btnConfigCadence->setEnabled(sel != 0);
    ui->edSendInterval->setEnabled((sel != 0) && bSendPeriodic);
    ui->chbFastCadence->setEnabled((sel != 0) && bSendPeriodic);
    ui->edCadenceLow->setEnabled((sel != 0) && bSendPeriodic && bFastCadence);
    ui->edCadenceHigh->setEnabled((sel != 0) && bSendPeriodic && bFastCadence);
    ui->edtriggerUp->setEnabled((sel != 0) && bTriggerPub);
    ui->edTriggerDown->setEnabled((sel != 0) && bTriggerPub);
    ui->chbTriggers->setEnabled(sel != 0);
}
