//
//  colordialog.cpp
//  PEAT_V2
//
//  Created by Hossain, Rakeeb on 2018-01-02.
//  Copyright © 2018 Hossain, Rakeeb. All rights reserved.
//
#include "colordialog.h"
#include "ui_colordialog.h"

ColorDialog::ColorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ColorDialog)
{
    ui->setupUi(this);
    ui->comboBox->addItem("BGR24");
    ui->comboBox->addItem("BGR32");
    ui->comboBox->addItem("RGB24");
    ui->comboBox->addItem("RGB32");
    ui->comboBox->addItem("YCrCb");

    ui->comboBox->setCurrentIndex(0);
}

int ColorDialog::currentIndex() {
    return ui->comboBox->currentIndex();
}

ColorDialog::~ColorDialog()
{
    delete ui;
}
