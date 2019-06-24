#include "rkbprotool.h"
#include "ui_rkbprotool.h"
#include <windows.h>
#include <stdio.h>
#include <QProgressDialog>
#include "rkbcore.h"
#include <QInputDialog>
#include <QVector>
#include <QtGui>
#include <QMenu>
#include <QDialog>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QFileDialog>
#include <QProgressBar>
#include <QSlider>
#include <QMediaPlaylist>
#include <QMessageBox>
#include <QtMath>
#include <string>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>

rkbProTool::rkbProTool(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::rkbProTool)
{
    ui->setupUi(this);

    connect(ui->slider, &QSlider::valueChanged, this, [&](qint64 moved)
    {
        double alpha = (((double)moved)/100.0);
        ui->label_4->setText(QString::number(alpha));
    });
}

rkbProTool::~rkbProTool()
{
    delete ui;


}

void rkbProTool::on_radioButton_2_clicked()
{
    ui->slider->setEnabled(true);
    ui->label_3->setEnabled(true);
    ui->label_4->setEnabled(true);
    decision  = 2;


}

void rkbProTool::on_radioButton_3_clicked()
{
    ui->slider->setEnabled(true);
    ui->label_3->setEnabled(true);
    ui->label_4->setEnabled(true);
    decision  = 3;


}

void rkbProTool::on_radioButton_4_clicked()
{
    ui->slider->setEnabled(true);
    ui->label_3->setEnabled(true);
    ui->label_4->setEnabled(true);
    decision  = 4;

}

void rkbProTool::on_radioButton_clicked()
{
    ui->slider->setEnabled(false);
    ui->label_3->setEnabled(false);
    ui->label_4->setEnabled(false);
    decision  = 1;
}


void rkbProTool::on_pushButton_clicked()
{
    QString folder = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "\\", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
}

void rkbProTool::on_buttonBox_accepted()
{

    if (decision != 0)
    {
        qDebug() << (ui->label_4->text()).toDouble();
        rObject r_instance;
        r_instance.proTool(ui->lineEdit->text(), ui->lineEdit_2->text(), decision, (ui->label_4->text()).toDouble());
    }
    else {
        QMessageBox::critical(0, "Error", "Please input valid values.");
    }
}
