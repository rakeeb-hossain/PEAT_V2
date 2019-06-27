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
    ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
    this->setFixedSize(640, 445);


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
    connect(ui->lineEdit, &QLineEdit::textChanged, this, [&]{
        if (ui->lineEdit->text() == "") {
           ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
           ui->radioButton->setEnabled(false);
           ui->radioButton_2->setEnabled(false);
           ui->radioButton_3->setEnabled(false);
           ui->radioButton_4->setEnabled(false);
           ui->label_2->setEnabled(false);
           ui->label_5->setEnabled(false);
           ui->label_7->setEnabled(false);
           ui->label_8->setEnabled(false);
        } else {
           ui->radioButton->setEnabled(true);
           ui->radioButton_2->setEnabled(true);
           ui->radioButton_3->setEnabled(true);
           ui->radioButton_4->setEnabled(true);
           ui->label_2->setEnabled(true);
           ui->label_5->setEnabled(true);
           ui->label_7->setEnabled(true);
           ui->label_8->setEnabled(true);
           if (decision != 0) ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
        }
    });

    connect(ui->radioButton, &QRadioButton::clicked, this, [&]{
        ui->label_8->setText("");
    });
    connect(ui->radioButton_2, &QRadioButton::clicked, this, [&]{
        ui->label_8->setText("Places a grey overlay on failing frames. Strength is the alpha value of the overlay.\n1.0 = fully opaque, 0.0 = fully transparent.");
    });
    connect(ui->radioButton_3, &QRadioButton::clicked, this, [&]{
        ui->label_8->setText("Lowers the contrast ratio of failing frames. Strength corresponds to decrease in contrast ratio (CR).\n1.0 = 1:1 CR, 0.0 = preserves original CR.");
    });
    connect(ui->radioButton_4, &QRadioButton::clicked, this, [&]{
        ui->label_8->setText("Blurs failing frames via a normalized box filter. Strength is the kernal size of blur.\n1.0 = kernal size of 255 (intense blur), 0.0 = kernal size of 0 (no blur).\nATTENTION: Blurring does not block failing frames. It can be used with other filters to enhance viewing.");
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
    ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );

}

void rkbProTool::on_radioButton_3_clicked()
{
    ui->slider->setEnabled(true);
    ui->label_3->setEnabled(true);
    ui->label_4->setEnabled(true);
    decision  = 3;
    ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );

}

void rkbProTool::on_radioButton_4_clicked()
{
    ui->slider->setEnabled(true);
    ui->label_3->setEnabled(true);
    ui->label_4->setEnabled(true);
    decision  = 4;
    ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
}

void rkbProTool::on_radioButton_clicked()
{
    ui->slider->setEnabled(false);
    ui->label_3->setEnabled(false);
    ui->label_4->setEnabled(false);
    decision  = 1;
    ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
}

void rkbProTool::on_pushButton_clicked()
{
    newFile = QFileDialog::getSaveFileName(this, tr("Set output video path"), "output_video.mp4", tr("MP4 File (*.mp4)"));
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
