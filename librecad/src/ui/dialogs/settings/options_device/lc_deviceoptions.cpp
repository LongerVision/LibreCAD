/*
**********************************************************************************
**
** This file was created for the LibreCAD project (librecad.org), a 2D CAD program.
**
** Copyright (C) 2016 ravas (github.com/r-a-v-a-s)
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
**********************************************************************************
*/

#include "lc_deviceoptions.h"
#include "rs_settings.h"
#include "ui_lc_deviceoptions.h"

LC_DeviceOptions::LC_DeviceOptions(QWidget* parent) :
    QFrame(parent),
    ui(new Ui::LC_DeviceOptions){
    ui->setupUi(this);

    const QString device = LC_GET_ONE_STR("Hardware","Device", "Mouse");
    int index = ui->device_combobox->findText(device);
    ui->device_combobox->setCurrentIndex(index);

    connect(ui->save_button,  &QPushButton::pressed, this, &LC_DeviceOptions::save);
    connect(ui->save_button, &QPushButton::released, parent, &LC_DeviceOptions::close);
}

LC_DeviceOptions::~LC_DeviceOptions(){
    delete ui;
}

void LC_DeviceOptions::save(){
    int index = ui->device_combobox->currentIndex();
    QString device = ui->device_combobox->itemText(index);
    LC_SET_ONE("Hardware","Device", device);
}
