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

rkbProTool::rkbProTool(vector<vector<int> > vid_info, QString vid_file, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::rkbProTool)
{
    ui->setupUi(this);
    this->setWindowTitle("Prophylactic Tool");

    vidData = vid_info;
    vidFile = vid_file;

    if (vidData.size() == 0 || vidFile == NULL || vidFile == "") {
        QMessageBox::critical(0, "Error", "There was an error in reading the analysis data.");
        delete ui;
    }

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
    newFile = QFileDialog::getSaveFileName(this, tr("Set output video path"), "untitled.mp4", tr("MP4 File (*.mp4)"));
    ui->lineEdit->setText(newFile);
}

void rkbProTool::on_buttonBox_accepted()
{

    if (decision != 0)
    {
        QFileInfo fi(newFile);
        QString ext = fi.suffix();
        if (ext == "mp4" || ext == "MP4") {
            rObject r_instance;
            QString error = r_instance.proTool(vidData, newFile, vidFile, decision, (ui->label_4->text()).toDouble());

            /*
            rObject *r_instance = new rObject;
            QThread* workerThread = new QThread;
            r_instance->moveToThread(workerThread);
            connect(workerThread, &QThread::started, r_instance, [&]{
                r_instance->proTool(vidData, newFile, vidFile, decision, (ui->label_4->text()).toDouble());
            });
            connect(r_instance, &rObject::finished, workerThread, &QThread::quit);
            connect(r_instance, &rObject::finished, r_instance, &rObject::deleteLater);
            connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);

            QEventLoop loop;
            workerThread->start();
            connect(r_instance, &rObject::finished, &loop, &QEventLoop::quit);
            connect(r_instance, &rObject::error, &loop, &QEventLoop::quit);
            loop.exec();
            */
            if (error != "Success") QMessageBox::critical(0, "Error", error);
        }
        else {
            QMessageBox::critical(0, "Error", "Please select a valid mp4 output.");
        }
    }
    else {
        QMessageBox::critical(0, "Error", "Please select a prophylactic method.");
    }
}
