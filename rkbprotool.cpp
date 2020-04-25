//
//  rkbprotool.cpp
//  PEAT_V2
//
//  Created by Hossain, Rakeeb on 2018-01-02.
//  Copyright © 2018 Hossain, Rakeeb. All rights reserved.
//
#include "rkbprotool.h"
#include "ui_rkbprotool.h"
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
    this->setFixedSize(QSize(640, 445));

    mv = new QMovie(":/images/Res/pkmngif.gif");
    mv->setScaledSize(ui->label_6->size());

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
           ui->radioButton_5->setEnabled(false);
           //ui->radioButton_6->setEnabled(false);
           ui->radioButton_7->setEnabled(false);
           ui->label_2->setEnabled(false);
           ui->label_5->setEnabled(false);
           ui->label_7->setEnabled(false);
           ui->label_8->setEnabled(false);
        } else {
           ui->radioButton->setEnabled(true);
           ui->radioButton_2->setEnabled(true);
           ui->radioButton_3->setEnabled(true);
           ui->radioButton_4->setEnabled(true);
           ui->radioButton_5->setEnabled(true);
           //ui->radioButton_6->setEnabled(true);
           ui->radioButton_7->setEnabled(true);
           ui->label_2->setEnabled(true);
           ui->label_5->setEnabled(true);
           ui->label_7->setEnabled(true);
           ui->label_8->setEnabled(true);
           if (decision != 0) {
               ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
           } else {
               mv->start();
               ui->label_6->setAttribute(Qt::WA_NoSystemBackground);
               ui->label_6->setMovie(mv);
           }
        }
    });

    connect(ui->radioButton, &QRadioButton::clicked, this, [&]{
        ui->label_8->setText("Decreases intensity of green and red channels in failing frames, lowering intensity of flashes.\n\n0.0 = no decrease, 1.0 = remove red and green channels");
    });
    connect(ui->radioButton_2, &QRadioButton::clicked, this, [&]{
        ui->label_8->setText("Sets a grey overlay on failing frames.\n\n0.0 = fully transparent, 1.0 = fully opaque");
    });
    connect(ui->radioButton_3, &QRadioButton::clicked, this, [&]{
        ui->label_8->setText("Lowers the contrast ratio (CR) of failing frames.\n\n0.0 = preserves original CR, 1.0 = 1:1 CR");
    });
    connect(ui->radioButton_4, &QRadioButton::clicked, this, [&]{
        ui->label_8->setText("Blurs failing frames.\n\n0.0 = no blur, 1.0 = intense blur.\n\nATTENTION: Blurring does not block failing frames. It can be used with other filters.");
    });
    connect(ui->radioButton_5, &QRadioButton::clicked, this, [&]{
        ui->label_8->setText("Swaps red and blue channels in failing frames, lowering effect of saturated red flashes.");
    });
    /*
    connect(ui->radioButton_6, &QRadioButton::clicked, this, [&]{
        ui->label_8->setText("Decreases frame rate of video at failing frames, lowering frequency of flashes per second.\n1 = preserve frame rate, 30 = 30x slower frame rate");
    });
    */
    connect(ui->radioButton_7, &QRadioButton::clicked, this, [&]{
        ui->label_8->setText("Removes failing frames from video.");
    });

    preview = new QMovie;

    ui->label->installEventFilter(this);
    ui->label_2->installEventFilter(this);
    ui->label_3->installEventFilter(this);
    ui->label_4->installEventFilter(this);
    ui->label_5->installEventFilter(this);
    ui->label_6->installEventFilter(this);
    ui->label_7->installEventFilter(this);
    ui->label_8->installEventFilter(this);
    ui->label_9->installEventFilter(this);
    ui->label_10->installEventFilter(this);
    ui->lineEdit->installEventFilter(this);
    ui->radioButton->installEventFilter(this);
    ui->radioButton_2->installEventFilter(this);
    ui->radioButton_3->installEventFilter(this);
    ui->radioButton_4->installEventFilter(this);
    ui->radioButton_5->installEventFilter(this);
    ui->radioButton_7->installEventFilter(this);
    ui->pushButton->installEventFilter(this);
    ui->slider->installEventFilter(this);
    ui->buttonBox->installEventFilter(this);
}

rkbProTool::~rkbProTool()
{
    delete ui;
}

void rkbProTool::on_radioButton_2_clicked()
{
    ui->slider->setEnabled(true);
    ui->slider->setRange(0, 100);
    ui->slider->setSingleStep(1);
    ui->slider->setPageStep(1);
    ui->slider->setTickInterval(1);
    ui->label_3->setText("Set strength");
    ui->label_3->setEnabled(true);
    ui->label_4->setEnabled(true);
    decision  = 2;
    ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );

    mv->stop();
    mv->start();
    preview->stop();
    ui->label_10->setText("");
    preview->setFileName(":/images/Res/overlay.gif");
    preview->setScaledSize(ui->label_10->size());
    ui->label_10->setMovie(preview);
    preview->start();
}

void rkbProTool::on_radioButton_3_clicked()
{
    ui->slider->setEnabled(true);
    ui->slider->setRange(0, 100);
    ui->slider->setSingleStep(1);
    ui->slider->setPageStep(1);
    ui->slider->setTickInterval(1);
    ui->label_3->setText("Set strength");
    ui->label_3->setEnabled(true);
    ui->label_4->setEnabled(true);
    decision  = 3;
    ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );

    mv->stop();
    mv->start();
    preview->stop();
    ui->label_10->setText("");
    preview->setFileName(":/images/Res/contrast.gif");
    preview->setScaledSize(ui->label_10->size());
    ui->label_10->setMovie(preview);
    preview->start();
}

void rkbProTool::on_radioButton_4_clicked()
{
    ui->slider->setEnabled(true);
    ui->slider->setRange(0, 100);
    ui->slider->setSingleStep(1);
    ui->slider->setPageStep(1);
    ui->slider->setTickInterval(1);
    ui->label_3->setText("Set strength");
    ui->label_3->setEnabled(true);
    ui->label_4->setEnabled(true);
    decision  = 4;
    ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );

    mv->stop();
    mv->start();
    preview->stop();
    ui->label_10->setText("");
    preview->setFileName(":/images/Res/blur.gif");
    preview->setScaledSize(ui->label_10->size());
    ui->label_10->setMovie(preview);
    preview->start();
}

void rkbProTool::on_radioButton_clicked()
{
    ui->slider->setEnabled(true);
    ui->slider->setRange(0, 100);
    ui->slider->setSingleStep(1);
    ui->slider->setPageStep(1);
    ui->slider->setTickInterval(1);
    ui->label_3->setText("Set strength");
    ui->label_3->setEnabled(true);
    ui->label_4->setEnabled(true);
    decision  = 1;
    ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );

    mv->stop();
    mv->start();
    preview->stop();
    ui->label_10->setText("");
    preview->setFileName(":/images/Res/colorShift.gif");
    preview->setScaledSize(ui->label_10->size());
    ui->label_10->setMovie(preview);
    preview->start();
}

void rkbProTool::on_radioButton_5_clicked()
{
    ui->slider->setEnabled(false);
    ui->label_3->setEnabled(false);
    ui->label_4->setEnabled(false);
    decision  = 5;
    ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );

    mv->stop();
    mv->start();
    preview->stop();
    ui->label_10->setText("");
    preview->setFileName(":/images/Res/swap.gif");
    preview->setScaledSize(ui->label_10->size());
    ui->label_10->setMovie(preview);
    preview->start();
}

/*
void rkbProTool::on_radioButton_6_clicked()
{
    ui->slider->setEnabled(true);
    ui->slider->setRange(100, 3000);
    ui->slider->setSingleStep(100);
    ui->slider->setPageStep(100);
    ui->slider->setTickInterval(100);
    ui->label_3->setText("Set slow factor");
    ui->label_3->setEnabled(true);
    ui->label_4->setEnabled(true);
    decision  = 6;
    ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
}
*/

void rkbProTool::on_radioButton_7_clicked()
{
    ui->slider->setEnabled(false);
    ui->label_3->setEnabled(false);
    ui->label_4->setEnabled(false);
    decision  = 7;
    ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );

    mv->start();
    preview->stop();
    ui->label_10->setText("No\nPreview");
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
            rObject *r_instance = new rObject;
            QString error = r_instance->proTool(vidData, newFile, vidFile, decision, (ui->label_4->text()).toDouble());

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
            if (error != "Success") {
                QMessageBox::critical(0, "Error", error);
                delete r_instance;
            }
        }
        else {
            QMessageBox::critical(0, "Error", "Please select a valid mp4 output path.");
        }
    }
    else {
        QMessageBox::critical(0, "Error", "Please select a prophylactic method.");
    }
}

bool rkbProTool::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Enter) {
        if (obj != ui->lineEdit) QApplication::setOverrideCursor(Qt::ArrowCursor);
    }
    return QDialog::eventFilter(obj, event);
}
