#include "mainframe.h"
#include "ui_mainframe.h"
#include <windows.h>
#include <stdio.h>
#include <QProgressDialog>
#include "rkbcore.h"
#include <QInputDialog>
#include "qcustomplot.h"
#include <QVector>
#include <QtGui>
#include <QMenu>
#include "rkbprotool.h"
#include <QKeyEvent>
#include <QDesktopServices>
#include <cstdio>
#include <memory>
#include <stdexcept>

using namespace std;
using namespace cv;

mainFrame::mainFrame(QWidget *parent) :
    QMainWindow(),
    ui(new Ui::mainFrame)
{
    ui->setupUi(this);
    this->setWindowTitle("UW Trace Center Photosensitive Epilepsy Analysis Tool (PEAT)");
    setWindowFlags(Qt::Widget | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    this->resize(QSize(870,591));
    this->setFixedSize(QSize(870,591));

    //QVBoxLayout *layout = new QVBoxLayout;
    //this->setLayout(layout);
    connect(ui->actionOpen_Video, &QAction::triggered, this, &mainFrame::on_folderButton_clicked);
    connect(ui->actionOpen_Report, &QAction::triggered, this, &mainFrame::openReport);
    connect(ui->actionGenerate_Report, &QAction::triggered, this, &mainFrame::on_reportButton_clicked);
    connect(ui->actionQuit, &QAction::triggered, qApp, QApplication::quit);
    connect(ui->actionRun_Prophylactic_Tool, &QAction::triggered, this, &mainFrame::openProTool);

    connect(ui->actionBack_5_Seconds, &QAction::triggered, this, &mainFrame::skipLeftFunc);
    connect(ui->actionForward_5_Seconds, &QAction::triggered, this, &mainFrame::skipRightFunc);
    connect(ui->actionPlay_Pause, &QAction::triggered, this,& mainFrame::playPauseFunc);
    connect(ui->actionForward_30_Frames, &QAction::triggered, this, &mainFrame::skipFrameRightFunc);
    connect(ui->actionBack_30_Frames, &QAction::triggered, this, &mainFrame::skipFrameLeftFunc);

    connect(ui->actionPEAT_Help, &QAction::triggered, this, &mainFrame::peatHelp);
    connect(ui->actionAbout_PEAT, &QAction::triggered, this, &mainFrame::aboutPEAT);
    connect(ui->actionTrace_Center_Homepage, &QAction::triggered, this, &mainFrame::traceHome);

    ui->customPlot->yAxis->setRange(0,1);
    ui->customPlot->yAxis->setVisible(false);
    ui->customPlot->xAxis->setLabel("Frame Number");
    ui->customPlot->setBackground(QBrush(QColor(214,214,214,255)));
    //9/15/18
    section = new QCPItemRect(ui->customPlot);
    ui->customPlot->addLayer("sectionBackground", ui->customPlot->layer("grid"), QCustomPlot::limBelow);
    section->setLayer("sectionBackground");
    section->topLeft->setTypeX(QCPItemPosition::ptPlotCoords);
    section->topLeft->setTypeY(QCPItemPosition::ptAxisRectRatio);
    section->topLeft->setAxes(ui->customPlot->xAxis, ui->customPlot->yAxis);
    section->topLeft->setAxisRect(ui->customPlot->axisRect());
    section->bottomRight->setTypeX(QCPItemPosition::ptPlotCoords);
    section->bottomRight->setTypeY(QCPItemPosition::ptAxisRectRatio);
    section->bottomRight->setAxes(ui->customPlot->xAxis, ui->customPlot->yAxis);
    section->bottomRight->setAxisRect(ui->customPlot->axisRect());
    section->setClipToAxisRect(true); // is by default true already, but this will change in QCP 2.0.0
    section->topLeft->setCoords(0, 1.0); // the y value is now in axis rect ratios, so -0.1 is "barely above" the top axis rect border
    section->bottomRight->setCoords(100000, 0.0); // the y value is now in axis rect ratios, so 1.1 is "barely below" the bottom axis rect border
    //section->setBrush(QBrush(QColor(200,200,200,255)));
    section->setPen(Qt::NoPen);
    QCPItemLine *line = new QCPItemLine(ui->customPlot);
    line->start->setCoords(0 , 0.25);
    line->end->setCoords(10000000 , 0.25);
    line->setClipToAxisRect(false);
    QCPItemText *textLabel = new QCPItemText(ui->customPlot);
    QCPItemText *textLabel2 = new QCPItemText(ui->customPlot);

    textLabel->setPositionAlignment(Qt::AlignVCenter|Qt::AlignRight);
    textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
    textLabel->position->setCoords(0.2, 0.7); // place position at center/top of axis rect
    textLabel->setText("Caution (Fail)");
    textLabel->setFont(QFont(font().family(), 12)); // make font a bit larger
    textLabel->setPen(QPen(Qt::black)); // show black border around text

    textLabel2->setPositionAlignment(Qt::AlignVCenter|Qt::AlignRight);
    textLabel2->position->setType(QCPItemPosition::ptAxisRectRatio);
    textLabel2->position->setCoords(0.2, 0.8); // place position at center/top of axis rect
    textLabel2->setText("Caution (Pass)");
    textLabel2->setFont(QFont(font().family(), 12)); // make font a bit larger
    textLabel2->setPen(QPen(Qt::black)); // show black border around text

    ui->customPlot->replot();

    player = new QMediaPlayer;
    playlist = new QMediaPlaylist;

    player->setNotifyInterval(50);
    connect(player, &QMediaPlayer::durationChanged, ui->slider, &QSlider::setMaximum);
    connect(player, &QMediaPlayer::positionChanged, ui->slider, &QSlider::setValue);
    connect(ui->slider, &QSlider::valueChanged, player, &QMediaPlayer::setPosition);
    connect(ui->slider, &QSlider::valueChanged, ui->horizontalScrollBar, [&](qint64 move) {
        qint64 sliderMax = ui->slider->maximum();
        qint64 scrollMax = ui->horizontalScrollBar->maximum();
        double percent = move/(double)(sliderMax);
        ui->horizontalScrollBar->setValue(percent*(double)scrollMax);
    });

    ui->customPlot->xAxis->scaleRange(24.0, ui->customPlot->xAxis->range().center());

    //Removed due to bug
/*
    connect(ui->horizontalScrollBar, &QScrollBar::valueChanged, ui->slider, [&](qint64 move2) {
        qint64 sliderMax = ui->slider->maximum();
        qint64 scrollMax = ui->horizontalScrollBar->maximum();
        double percent = move2/(double)(scrollMax);
        qDebug() << move2;

        ui->slider->setValue(percent*(double)sliderMax);
    });
*/
    connect(player, &QMediaPlayer::durationChanged, ui->slider, &QSlider::setMaximum);
    connect(player, &QMediaPlayer::positionChanged, ui->slider, &QSlider::setValue);
}

mainFrame::~mainFrame()
{
    delete ui;
}

void mainFrame::closeEvent(QCloseEvent *event) {
    if (saved == false) {
        QMessageBox messageBox;
        QMessageBox::StandardButton reply;
        reply = messageBox.question(this, "Report unsaved", "Would you like to save your report?", QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        messageBox.setFixedSize(600,400);

        if (reply == QMessageBox::Yes) {
            if (!on_actionSave_Report_triggered()) {
                event->ignore();
            }
        } else if (reply == QMessageBox::No) {
            event->accept();
        }
        else {
            event->ignore();
        }
    }
}

void mainFrame::on_folderButton_clicked()
{

    QString filename = QFileDialog::getOpenFileName(this, "Open a Video File", "", "Video File (*.mp4; *.avi; *.flv; *.mpeg)");
    QFileInfo fi(filename);
    QString ext = fi.suffix();
    bool didSelect = false;
    bool isVideo = false;
    bool continue_loading = true;
    string fileString = filename.toUtf8().constData();
    size_t pos = fileString.find("WebCourseVideo");

    if (ext == "avi" || ext == "AVI")
    {
        QMessageBox messageBox;
        messageBox.information(this, "AVI File Information", "AVI files are known to sometimes contain deprecated pixel/encoding formats that may cause unpredictable behaviour of PEAT. Consider converting the file before beginning a lengthy analysis.");
        messageBox.setFixedSize(600,400);

    }
    if (ext == "mp4" || ext == "avi" || ext == "mpeg" || ext == "flv" || ext == "mov" || ext == "wmv" || ext == "MP4" || ext == "AVI" || ext == "MPEG" || ext == "FLV" || ext == "MOV" || ext == "WMV"){
        isVideo = true;
        didSelect = true;
    }
    else if (ext != ""){
        didSelect = true;
    }

    if(!filename.isEmpty() && isVideo == true){
        if (pos != string::npos)
        {
            QMessageBox messageBox;
            messageBox.information(this, "Error", "Sorry, this encoding format is not supported by PEAT.");
            messageBox.setFixedSize(600,400);
        }
        else if (fileString.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_./\!@#$%^&*()-+=~`?<>,") == std::string::npos)
        {
            QMessageBox messageBox;
            messageBox.information(this, "Error", "Please remove the spaces in the file name.");
            messageBox.setFixedSize(600,400);
        }
        else {
            QFile file(filename);
            if(!file.open(QIODevice::ReadOnly))
            {
                QMessageBox messageBox;
                messageBox.information(this, "Error", "There was an error in loading the file. Please install the appropriate codecs (K-Lite Codec Pack) and try again.");
                messageBox.setFixedSize(600,400);
            }
            else
            {
                if (saved == false) {
                    QMessageBox messageBox;
                    QMessageBox::StandardButton reply;
                    reply = messageBox.question(this, "Report unsaved", "Would you like to save your report?", QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
                    messageBox.setFixedSize(600,400);

                    if (reply == QMessageBox::Yes) {
                        if (!on_actionSave_Report_triggered()) {
                            continue_loading = false;
                        }
                    } else if (reply == QMessageBox::No) {
                        continue_loading = true;
                    }
                    else {
                        continue_loading = false;
                    }
                }
                if (continue_loading == true)
                {
                    //RESET PLOT/UI IF A NEW VIDEO IS UPLOADED
                    no_report_loaded();

                    ui->horizontalSlider->setEnabled(true);
                    ui->horizontalSlider->setValue(0);
                    connect(ui->horizontalSlider, &QSlider::valueChanged, this, [&](int moved)
                    {
                        double alpha = 1.0;
                        int delta = moved - lastValue;
                        if (delta > 0)
                        {
                            for (int x = lastValue + 1; x < moved + 1; x++)
                            {
                                alpha *= 1/(1.1);
                            }
                        }
                        else {
                            for (int x = lastValue - 1; x > moved - 1; x--)
                            {
                                alpha *= 1.1;
                            }
                        }

                        ui->customPlot->xAxis->scaleRange(alpha, ui->customPlot->xAxis->range().center());
                        ui->customPlot->replot();
                        lastValue = moved;
                    });

                    playlist->clear();
                    playlist->addMedia(QUrl::fromLocalFile(filename));
                    playlist->setCurrentIndex(1);

                    player->setPlaylist(playlist);
                    player->setVideoOutput(ui->videoWidget_2);
                    ui->videoWidget_2->show();
                    ui->label_14->setEnabled(false);
                    ui->label_14->setText("Press play");
                    ui->label->setText(filename);
                    ui->timeLabel->setStyleSheet("background-color:rgb(210, 255, 189); border-style:solid; border-color:black; border-width:1px;");

                    ui->label_8->setText("." + ext);
                    string fpsCMD = "RKBVidCore.exe -i \"" + (fileString) + "\" -hide_banner 2>&1";

                    cout << fpsCMD;

                    const char * f = fpsCMD.c_str();
                    string output = execute(f);

                    string duration = output;
                    pos = output.find("Duration: ");
                    if (pos != string::npos)
                    {
                        duration = output.substr(pos);
                        duration = duration.substr(10,11);
                    }
                    ui->label_10->setText(QString::fromStdString(duration));

                    pos = output.find("Video:");
                    string fps = output;

                    if (pos != string::npos)
                        output = output.substr(pos);

                    if (pos != string::npos){
                        pos = output.find("kb/s");
                        fps = output.substr(pos);
                        fps.erase(0, 6);
                        if (pos != string::npos){
                            pos = fps.find(" ");
                            fps.erase(pos, fps.length());
                            cout << "A3" << endl;
                        }
                    }
                    auto fpsS = atof(fps.c_str());
                    ui->label_9->setText(QString::number(fpsS));

                    // To-do: allow videos of any FPS
                    if (round(fpsS) < 5)
                    {
                        playlist->clear();
                        ui->label_8->setText("");
                        ui->label_9->setText("");
                        ui->label_10->setText("");
                        ui->label_11->setText("");
                        ui->label_12->setText("");
                        ui->label_13->setText("");
                        QMessageBox messageBox;
                        messageBox.information(this, "Error", "PEAT only supports videos with an FPS of at least 5.");
                        messageBox.setFixedSize(600,400);
                    }
                    else {
                        string codec;
                        pos = output.find(" / 0x");
                        codec = output;
                        codec.erase(pos, codec.length());
                        pos = codec.find(" (");
                        codec = codec.substr(pos + 2);
                        pos = codec.find(") (");
                        if (pos != string::npos){
                            codec.erase(0, pos + 3);
                        }
                        ui->label_13->setText(QString::fromStdString(codec));

                        string nextCodec = output;
                        pos = output.find(" / 0x");
                        nextCodec.erase(pos, nextCodec.length());
                        pos = nextCodec.find(" (");
                        nextCodec.erase(pos, nextCodec.length());
                        nextCodec = nextCodec.substr(7, nextCodec.length());
                        ui->label_12->setText(QString::fromStdString(nextCodec));

                        string dimensions = output;
                        pos = dimensions.find("kb/s");
                        dimensions = dimensions.substr(0, pos);
                        qDebug() << QString::fromStdString(dimensions);
                        pos = dimensions.find(", ");
                        dimensions = dimensions.substr(pos + 2, dimensions.length());
                        pos = dimensions.find("), ");
                        if (pos != string::npos)
                        {
                            dimensions = dimensions.substr(pos + 3, dimensions.length());
                        }
                        else {
                            pos = dimensions.find(", ");
                            dimensions = dimensions.substr(pos + 2, dimensions.length());
                        }
                        pos = dimensions.find("[");
                        if (pos != string::npos)
                        {
                            dimensions = dimensions.substr(0, pos - 1);
                        }
                        else{
                            pos = dimensions.find(", ");
                            dimensions = dimensions.substr(0, pos);
                        }
                        ui->label_11->setText(QString::fromStdString(dimensions));

                        //***
                        connect(player, &QMediaPlayer::positionChanged, this, [&](qint64 dur) {
                            QString hours, minutes, seconds, milliseconds;

                            auto hrs = qFloor(dur / 3600000.0);
                            if (hrs < 10)
                            {
                                hours = "0" + QString::number(hrs);
                            }
                            else {
                                hours = QString::number(hrs);
                            }
                            auto mins = qFloor((dur - hrs*3600000.0) / 60000.0);
                            if (mins < 10)
                            {
                                minutes = "0" + QString::number(mins);
                            }
                            else {
                                minutes = QString::number(mins);
                            }
                            auto secs = qFloor((dur - hrs*3600000.0 - mins*60000.0) / 1000.0);
                            if (secs < 10)
                            {
                                seconds = "0" + QString::number(secs);
                            }
                            else {
                                seconds = QString::number(secs);
                            }
                            auto ms = (dur - hrs*3600000 - mins*60000 - secs*1000);
                            if (ms < 10)
                            {
                                milliseconds = "0" + QString::number(ms);
                            }
                            else if (100 < ms && ms >= 10)
                            {
                                milliseconds = "" + QString::number(ms);
                            }
                            else {
                                milliseconds = QString::number(ms);
                            }
                            ui->timeLabel->setText(hours + ":" + minutes +":" + seconds + ":" + milliseconds);
                        });

                        player->setVolume(5);
                        ui->rewindButton->setStyleSheet("#rewindButton { background-image: url(:/images/Res/FastBackUp.png); border-image: url(:/images/Res/FastBackUp.png); } #rewindButton:hover { border-image: url(:/images/Res/FastBackDownHilite.png); } #rewindButton:pressed { border-image: url(:/images/Res/FastBackPressed.png); }");
                        ui->playButton->setStyleSheet("#playButton { background-image: url(:/images/Res/PlayUp.png); border-image: url(:/images/Res/PlayUp.png); } #playButton:hover { border-image: url(:/images/Res/PlayDownHilite.png); } #playButton:pressed { border-image: url(:/images/Res/PlayPressed.png); }");
                        ui->pauseButton->setStyleSheet("#pauseButton { background-image: url(:/images/Res/StopUp.png); border-image: url(:/images/Res/StopUp.png); } #pauseButton:hover { border-image: url(:/images/Res/StopDownHilite.png); } #pauseButton:pressed { border-image: url(:/images/Res/StopPressed.png); }");
                        ui->forwardButton->setStyleSheet("#forwardButton { background-image: url(:/images/Res/FFwdUp.png); border-image: url(:/images/Res/FFwdUp.png); } #forwardButton:hover { border-image: url(:/images/Res/FFwdDownHilite.png); } #forwardButton:pressed { border-image: url(:/images/Res/FFwdPressed.png); }");
                        ui->reportButton->setStyleSheet("#reportButton { background-image: url(:/images/Res/AnalyzeUp.png); border-image: url(:/images/Res/AnalyzeUp.png); } #reportButton:hover { border-image: url(:/images/Res/AnalyzeDownHilite.png); } #reportButton:pressed { border-image: url(:/images/Res/AnalyzePressed.png); }");
                        ui->restartButton->setStyleSheet("#restartButton { background-image: url(:/images/Res/RewindUp.png); border-image: url(:/images/Res/RewindUp.png); } #restartButton:hover { border-image: url(:/images/Res/RewindDownHilite.png); } #restartButton:pressed { border-image: url(:/images/Res/RewindPressed.png); }");

                        ui->restartButton->setEnabled(true);
                        ui->rewindButton->setEnabled(true);
                        ui->playButton->setEnabled(true);
                        ui->pauseButton->setEnabled(true);
                        ui->forwardButton->setEnabled(true);
                        ui->reportButton->setEnabled(true);

                        ui->actionGenerate_Report->setEnabled(true);
                        ui->actionBack_5_Seconds->setEnabled(true);
                        ui->actionBack_30_Frames->setEnabled(true);
                        ui->actionForward_5_Seconds->setEnabled(true);
                        ui->actionForward_30_Frames->setEnabled(true);
                        ui->actionPlay_Pause->setEnabled(true);
                        ui->statusbar->showMessage(tr("Loading"));
                        connect(player, &QMediaPlayer::mediaStatusChanged,
                                [&](QMediaPlayer::MediaStatus status){
                            if(status == QMediaPlayer::LoadedMedia) {
                                ui->statusbar->showMessage(tr("Ready"));
                                player->play();
                            }
                        });
                    }
                }
            }
        }
    }
    else
    {
        if (didSelect == true)
        {
            QMessageBox messageBox;
            messageBox.information(this, "Error", "There was an error in loading the file. Please check the file and try again.");
            messageBox.setFixedSize(600,400);
        }
    }
}

void mainFrame::on_playButton_clicked()
{
    player->play();
}

void mainFrame::on_pauseButton_clicked()
{
    player->pause();
}

void mainFrame::on_forwardButton_clicked()
{

    if (x < 32.0)
    {
        x = 2.0*x;
        rate = x;
        player->setPlaybackRate(rate);
    }
}

void mainFrame::on_rewindButton_clicked()
{
    if (x > 0.03125)
    {
        x = 0.5*x;
        player->setPlaybackRate(x);
    }
}

void mainFrame::horzScrollBarChanged(int value)
{
  if (qAbs(ui->customPlot->xAxis->range().center()-value) > 0.01) // if user is dragging plot, we don't want to replot twice
  {
    ui->customPlot->xAxis->setRange(value, ui->customPlot->xAxis->range().size(), Qt::AlignCenter);
    ui->customPlot->replot();

  }
}

void mainFrame::xAxisChanged(QCPRange range)
{
  ui->horizontalScrollBar->setPageStep(qRound(range.size())); // adjust size of scroll bar slider
}


void mainFrame::updatePlot(vector<QVector<double > > points_x, vector<QVector<double > > points_y) {
    //Plot
    ui->customPlot->graph(0)->addData(points_x[0], points_y[0]);
    ui->customPlot->graph(1)->addData(points_x[1], points_y[1]);
    ui->customPlot->graph(2)->addData(points_x[2], points_y[2]);
    ui->customPlot->graph(3)->addData(points_x[3], points_y[3]);
    ui->customPlot->replot();

    //Slider
    //if ((int)points_x[0][points_x[0].size()-1] % 10 == 0) ui->slider->setValue(points_x[0][0]);
}

void mainFrame::on_reportButton_clicked() {
    try {
        for (int i = 0; i < ui->customPlot->graphCount(); i++) {
            ui->customPlot->graph(i)->data()->clear();
        }
        for (int i = 0; i < 2000; i++) {
            player->play();
        }
        player->play();
        player->pause();
        QString filename = ui->label->text();
        std::string stringedFile = filename.toLocal8Bit().constData();
        VideoCapture cap(stringedFile);
        int frameNum = cap.get(CAP_PROP_FRAME_COUNT);
        if (frameNum == 0) throw 200;
        Mat frame;
        cap >> frame;
        if (frame.empty()) throw 210;
        cap.release();

        //Initialize scrollbar
        ui->horizontalScrollBar->setEnabled(true);
        ui->horizontalScrollBar->setRange(0, frameNum);
        connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzScrollBarChanged(int)));
        connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));

        //Initialize graphs
        ui->customPlot->addGraph();
        ui->customPlot->addGraph();
        ui->customPlot->addGraph();
        ui->customPlot->addGraph();

        ui->customPlot->yAxis->setRange(0.0, 1.0);
        ui->customPlot->xAxis->setLabel("Frame Number");
        ui->customPlot->yAxis->setVisible(false);
        section->bottomRight->setCoords(frameNum, 0.0); // the y value is now in axis rect ratios, so 1.1 is "barely below" the bottom axis rect border
        ui->customPlot->replot();

        //Graph 1 style
        QPen solid;
        solid.setStyle(Qt::DotLine);
        solid.setWidthF(2);
        solid.setColor(Qt::black);
        ui->customPlot->graph(0)->setPen(solid);
        ui->customPlot->graph(0)->setLineStyle(QCPGraph::lsLine);
        ui->customPlot->graph(0)->addData(0.0, 0.0);
        //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

        //Graph 2 style
        QPen solidRed;
        solidRed.setStyle(Qt::DotLine);
        solidRed.setWidthF(2);
        solidRed.setColor(Qt::red);
        ui->customPlot->graph(1)->setPen(solidRed);
        ui->customPlot->graph(1)->setLineStyle(QCPGraph::lsLine);
        ui->customPlot->graph(1)->addData(0.0, 0.0);
        //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

        //Graph 3 style
        QPen dotted;
        dotted.setStyle(Qt::SolidLine);
        dotted.setWidthF(4);
        dotted.setColor(Qt::white);
        ui->customPlot->graph(2)->setPen(dotted);
        ui->customPlot->graph(2)->setLineStyle(QCPGraph::lsLine);
        ui->customPlot->graph(2)->addData(0.0, 0.0);
        //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

        //Graph 4 style
        QPen dottedRed;
        dottedRed.setStyle(Qt::SolidLine);
        dottedRed.setWidthF(4);
        dottedRed.setColor(Qt::red);
        ui->customPlot->graph(3)->setPen(dottedRed);
        ui->customPlot->graph(3)->setLineStyle(QCPGraph::lsLine);
        ui->customPlot->graph(3)->addData(0.0, 0.0);
        ui->customPlot->replot();
        //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

        //Connect slots (slider and QCP) and analyze video
        rObject* r_instance = new rObject;
        connect(r_instance, &rObject::updateUI, this, updatePlot);
        connect(r_instance, &rObject::progressCount, this, [&](int moved) {
            int delta = (int)(((double)moved/frameNum)*ui->slider->maximum());
            ui->slider->setValue(delta);
        });
        vid_data = r_instance->rkbcore(stringedFile);

        //Check to see vid_data has valid arrays of data points
        if (vid_data.size() < 4) {
            throw 220;
        }
        else {
            if (vid_data[0].size() < 2 || vid_data[1].size() < 2 || vid_data[2].size() < 2 || vid_data[3].size() < 2) throw 230;
        }

        ui->customPlot->graph(0)->addData(frameNum, 0);
        ui->customPlot->graph(1)->addData(frameNum, 0);
        ui->customPlot->graph(2)->addData(frameNum, 0);
        ui->customPlot->graph(3)->addData(frameNum, 0);
        ui->customPlot->replot();

        // Load warnings
        ui->backWarning->setStyleSheet("#backWarning { background-image: url(:/images/Res/PrevWarningUp.png); border-image: url(:/images/Res/PrevWarningUp.png); } #backWarning:hover { border-image: url(:/images/Res/PrevWarningDownHilite.png); } #backWarning:pressed { border-image: url(:/images/Res/PrevWarningPressed.png); }");
        ui->forwardWarning->setStyleSheet("#forwardWarning { background-image: url(:/images/Res/NextvWarningUp.png); border-image: url(:/images/Res/NextWarningUp.png); } #forwardWarning:hover { border-image: url(:/images/Res/NextWarningDownHilite.png); } #forwardWarning:pressed { border-image: url(:/images/Res/NextWarningPressed.png); }");
        ui->backWarning->setEnabled(true);
        ui->forwardWarning->setEnabled(true);
        warnings.clear();

        //Set warnings from seizure_frames (vid_data[2] and vid_data[3])
        int previous = vid_data[2][0] | vid_data[3][0];
        for (unsigned int i = 1; i < vid_data[2].size(); i++) {
            int current = vid_data[2][i] | vid_data[3][i];
            if (current == 1 && previous == 0) {
                warnings.push_back(i);
            }
            previous = current;
        }
        ui->label_6->setText(QString::number(warnings.size()));
        ui->actionSave_Report->setEnabled(true);
        ui->actionPrint_Report->setEnabled(true);
        ui->actionRun_Prophylactic_Tool->setEnabled(true);

        //Set report data to unsaved
        saved = false;
    }
    catch(int e) {
        QMessageBox mb;
        mb.critical(this, "Error: " + QString::number(e), "There was an error in decoding the video stream. Please try and install the appropriate codecs and try again.");
        mb.setFixedSize(600,400);
    }
}

void mainFrame::openReport()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open a report file", "", "Open PEAT Report (*.peat)");
    QFile file(filename);

    if (!filename.isNull()) {
        try {
            file.open(QIODevice::ReadOnly);

            //CHECK if file is open
            if (!file.isOpen()) throw 500;
            QDataStream stream(&file);

            //Check id
            quint32 id;
            stream >> id;
            if (id != 0x0A46BF100) throw 510;

            // Check video file
            QString filename;
            stream >> filename;
            if (!QFile::exists(filename)) throw 511;
            VideoCapture cap(filename.toStdString());
            Mat frame;
            cap >> frame;
            if (frame.empty()) throw 512;

            // Check video specs
            quint32 fps;
            stream >> fps;
            if (fps != round(cap.get(CAP_PROP_FPS))) throw 513;
            quint32 frame_count;
            stream >> frame_count;
            if (frame_count != cap.get(CAP_PROP_FRAME_COUNT)) throw 514;
            cap.release();

            //Declare vid_data template
            vector<vector<int > > vid_data_t(5);
            vid_data_t[4].push_back(fps);
            vid_data_t[4].push_back(frame_count);

            //READ data from file TO-DO: Write data to QCP
            QString header;
            stream >> header;
            if (header != "flash_diag") throw 520;
            for (unsigned int i = 0; i < frame_count; i++) {
                quint32 n;
                stream >> n;
                vid_data_t[0].push_back((int)n);
            }
            stream >> header;
            if (header != "red_flash_diag") throw 521;
            for (unsigned int i = 0; i < frame_count; i++) {
                quint32 n;
                stream >> n;
                vid_data_t[1].push_back((int)n);
            }
            stream >> header;
            if (header != "seizure_frames") throw 522;
            for (unsigned int i = 0; i < frame_count; i++) {
                quint32 n;
                stream >> n;
                vid_data_t[2].push_back((int)n);
            }
            stream >> header;
            if (header != "red_seizure_frames") throw 523;
            for (unsigned int i = 0; i < frame_count; i++) {
                quint32 n;
                stream >> n;
                vid_data_t[3].push_back((int)n);
            }
            qDebug() << "success";

            //Initialize slider and scrollbar
            ui->horizontalSlider->setEnabled(true);
            ui->horizontalSlider->setValue(0);
            ui->horizontalScrollBar->setEnabled(true);
            ui->horizontalScrollBar->setRange(0, frame_count);

            //Initialize graphs
            ui->customPlot->xAxis->scaleRange(8.0, ui->customPlot->xAxis->range().center());
            ui->customPlot->addGraph();
            ui->customPlot->addGraph();
            ui->customPlot->addGraph();
            ui->customPlot->addGraph();

            ui->customPlot->yAxis->setRange(0.0, 1.0);
            ui->customPlot->xAxis->setLabel("Frame Number");
            ui->customPlot->yAxis->setVisible(false);
            section->bottomRight->setCoords(frame_count, 0.0); // the y value is now in axis rect ratios, so 1.1 is "barely below" the bottom axis rect border
            ui->customPlot->replot();

            //Graph 1 style
            QPen solid;
            solid.setStyle(Qt::DotLine);
            solid.setWidthF(2);
            solid.setColor(Qt::black);
            ui->customPlot->graph(0)->setPen(solid);
            ui->customPlot->graph(0)->setLineStyle(QCPGraph::lsLine);
            ui->customPlot->graph(0)->addData(0.0, 0.0);
            //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

            //Graph 2 style
            QPen solidRed;
            solidRed.setStyle(Qt::DotLine);
            solidRed.setWidthF(2);
            solidRed.setColor(Qt::red);
            ui->customPlot->graph(1)->setPen(solidRed);
            ui->customPlot->graph(1)->setLineStyle(QCPGraph::lsLine);
            ui->customPlot->graph(1)->addData(0.0, 0.0);
            //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

            //Graph 3 style
            QPen dotted;
            dotted.setStyle(Qt::SolidLine);
            dotted.setWidthF(4);
            dotted.setColor(Qt::white);
            ui->customPlot->graph(2)->setPen(dotted);
            ui->customPlot->graph(2)->setLineStyle(QCPGraph::lsLine);
            ui->customPlot->graph(2)->addData(0.0, 0.0);
            //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

            //Graph 4 style
            QPen dottedRed;
            dottedRed.setStyle(Qt::SolidLine);
            dottedRed.setWidthF(4);
            dottedRed.setColor(Qt::red);
            ui->customPlot->graph(3)->setPen(dottedRed);
            ui->customPlot->graph(3)->setLineStyle(QCPGraph::lsLine);
            ui->customPlot->graph(3)->addData(0.0, 0.0);
            ui->customPlot->replot();
            //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

            //TO-DO: Write data to graph, copy vid_data_t to vid_data, open video file + information

            //Check to see vid_data has valid arrays of data points
            if (vid_data.size() < 4) {
                throw 220;
            }
            else {
                if (vid_data[0].size() < 2 || vid_data[1].size() < 2 || vid_data[2].size() < 2 || vid_data[3].size() < 2) throw 230;
            }

            ui->customPlot->graph(0)->addData(frame_count, 0);
            ui->customPlot->graph(1)->addData(frame_count, 0);
            ui->customPlot->graph(2)->addData(frame_count, 0);
            ui->customPlot->graph(3)->addData(frame_count, 0);

            // Load warnings
            ui->backWarning->setStyleSheet("#backWarning { background-image: url(:/images/Res/PrevWarningUp.png); border-image: url(:/images/Res/PrevWarningUp.png); } #backWarning:hover { border-image: url(:/images/Res/PrevWarningDownHilite.png); } #backWarning:pressed { border-image: url(:/images/Res/PrevWarningPressed.png); }");
            ui->forwardWarning->setStyleSheet("#forwardWarning { background-image: url(:/images/Res/NextvWarningUp.png); border-image: url(:/images/Res/NextWarningUp.png); } #forwardWarning:hover { border-image: url(:/images/Res/NextWarningDownHilite.png); } #forwardWarning:pressed { border-image: url(:/images/Res/NextWarningPressed.png); }");
            ui->backWarning->setEnabled(true);
            ui->forwardWarning->setEnabled(true);
            warnings.clear();

            //Set warnings from seizure_frames (vid_data[2] and vid_data[3])
            int previous = vid_data[2][0] | vid_data[3][0];
            for (int i = 1; i < vid_data[2].size(); i++) {
                int current = vid_data[2][i] | vid_data[3][i];
                if (current == 1 && previous == 0) {
                    warnings.push_back(i);
                }
                previous = current;
            }
            ui->label_6->setText(QString::number(warnings.size()));
            ui->actionSave_Report->setEnabled(true);
            ui->actionPrint_Report->setEnabled(true);
            ui->actionRun_Prophylactic_Tool->setEnabled(true);

            file.close();
        }
        catch (int e) {
            //TO-DO: Complete errors
            QString error;
            switch(e)
            {
                case 600:
                    error = "The report file could not be written. Please try again and check the directory";
                    break;
                case 610:
                    error = "The loaded video file is not valid. Please add the video back to the loaded path or reload the video and try again.";
                    break;
                default:
                    error = "The report file could not be written. Please try again and check the directory";
            }
            QMessageBox mb;
            mb.critical(this, "Error: " + QString::number(e), error);
            mb.setFixedSize(600,400);
        }
    }
}


void mainFrame::on_forwardWarning_clicked()
{
    if (warnings.size() > 0)
    {
        if (forwardWarningJustPressed == false)
        {
            ui->horizontalScrollBar->setValue(warnings[index]);
            if (index == (warnings.size() - 1))
            {
                index = 0;
            }
            else {
                index++;
            }
            forwardWarningJustPressed = true;
        }
        else {
            if (index == (warnings.size() - 1))
            {
                index = 0;
                ui->horizontalScrollBar->setValue(warnings[index]);
            }
            else {
                index++;
                ui->horizontalScrollBar->setValue(warnings[index]);
            }
        }
    }
}

void mainFrame::on_backWarning_clicked()
{

    if (warnings.size() > 0)
    {
        if (index == 0)
        {
            index = warnings.size() - 1;
            ui->horizontalScrollBar->setValue(warnings[index]);
        }
        else {
            index -= 1;
            ui->horizontalScrollBar->setValue(warnings[index]);
        }
    }
}

void mainFrame::openProTool()
{
    rkbprotool = new rkbProTool(this);
    rkbprotool->exec();

}

void mainFrame::on_restartButton_clicked()
{
    player->setPosition(0);
}

void mainFrame::keyPressEvent(QKeyEvent * event)
{
    if(event->key() == Qt::Key_Right)
    {
        qint64 position = player->position();
        position += 1000;
        player->setPosition(position);
    }
    if(event->key() == Qt::Key_Left)
    {
        qDebug() << "thx";
        qint64 position = player->position();
        position -= 1000;
        player->setPosition(position);
    }
    if(event->key() == Qt::Key_L)
    {
        qint64 position = player->position();
        position += 5000;
        player->setPosition(position);
    }
    if(event->key() == Qt::Key_J)
    {
        qint64 position = player->position();
        position -= 5000;
        player->setPosition(position);
    }
    if(event->key() == Qt::Key_Space)
    {
        if (player->state() == 1)
        {
            qDebug() << "playing";
            player->pause();
        }
        else if (player->state() == 2)
        {
            qDebug() << "paused ";
            player->play();
        }
    }
}

void mainFrame::skipLeftFunc()
{
    qint64 position = player->position();
    position += 5000;
    player->setPosition(position);
}
void mainFrame::skipRightFunc()
{
    qint64 position = player->position();
    position -= 5000;
    player->setPosition(position);
}
void mainFrame::playPauseFunc()
{
    if (player->state() == 1)
    {
        qDebug() << "playing";
        player->pause();
    }
    else if (player->state() == 2)
    {
        qDebug() << "paused ";
        player->play();
    }
}
void mainFrame::skipFrameRightFunc()
{
    qint64 position = player->position();
    position += 1000;
    player->setPosition(position);
}
void mainFrame::skipFrameLeftFunc()
{
    qint64 position = player->position();
    position -= 1000;
    player->setPosition(position);
}

void mainFrame::peatHelp()
{
    QString link = "http://trace.umd.edu/peat/help/";
    QDesktopServices::openUrl(QUrl(link));
}

void mainFrame::aboutPEAT()
{
    QString link = "http://trace.umd.edu/peat/";
    QDesktopServices::openUrl(QUrl(link));
}

void mainFrame::traceHome()
{
    QString link = "http://trace.umd.edu/";
    QDesktopServices::openUrl(QUrl(link));
}

string mainFrame::execute(const char * cmd) {

    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(_popen(cmd, "r"), _pclose);
    if (!pipe) {
        QMessageBox::critical(0, "Error", "Error running frame split script. Ensure that PEAT has sufficient permissions to System Applications.");
        throw std::runtime_error("popen() failed!");
    }
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
        {
            result += buffer.data();
        }
    }
    return result;
}

string mainFrame::replaceChar(string str, char ch1, char ch2) {
  for (int i = 0; i < str.length(); ++i) {
    if (str[i] == ch1)
      str[i] = ch2;
  }

  return str;
}

void mainFrame::on_actionLight_Blue_triggered()
{
    if (ui->actionLight_Blue->isChecked() == true) {
        ui->actionLight_Blue->setChecked(true);
        ui->actionTurquoise->setChecked(false);
        ui->actionPale->setChecked(false);
        ui->actionBeige->setChecked(false);
        ui->actionCharcoal_Grey->setChecked(false);
        ui->actionLight_Grey->setChecked(false);
        ui->actionWhite->setChecked(false);
        ui->customPlot->setBackground(QBrush(QColor(190, 212, 221, 255)));
        ui->customPlot->repaint();
    }
    else {
        ui->actionLight_Blue->setChecked(true);
    }
}

void mainFrame::on_actionBeige_triggered()
{
    if (ui->actionBeige->isChecked() == true) {
        ui->actionBeige->setChecked(true);
        ui->actionTurquoise->setChecked(false);
        ui->actionPale->setChecked(false);
        ui->actionLight_Blue->setChecked(false);
        ui->actionCharcoal_Grey->setChecked(false);
        ui->actionLight_Grey->setChecked(false);
        ui->actionWhite->setChecked(false);
        ui->customPlot->setBackground(QBrush(QColor(216, 201, 184, 255)));
        ui->customPlot->repaint();
    }
    else {
        ui->actionBeige->setChecked(true);
    }

}

void mainFrame::on_actionPale_triggered()
{
    if (ui->actionPale->isChecked() == true) {
        ui->actionPale->setChecked(true);
        ui->actionTurquoise->setChecked(false);
        ui->actionLight_Blue->setChecked(false);
        ui->actionBeige->setChecked(false);
        ui->actionCharcoal_Grey->setChecked(false);
        ui->actionLight_Grey->setChecked(false);
        ui->actionWhite->setChecked(false);
        ui->customPlot->setBackground(QBrush(QColor(247, 243, 239, 255)));
        ui->customPlot->repaint();
    }
    else {
        ui->actionPale->setChecked(true);
    }

}

void mainFrame::on_actionTurquoise_triggered()
{
    if (ui->actionTurquoise->isChecked() == true) {
        ui->actionTurquoise->setChecked(true);
        ui->actionLight_Blue->setChecked(false);
        ui->actionPale->setChecked(false);
        ui->actionBeige->setChecked(false);
        ui->actionCharcoal_Grey->setChecked(false);
        ui->actionLight_Grey->setChecked(false);
        ui->actionWhite->setChecked(false);
        ui->customPlot->setBackground(QBrush(QColor(192, 219, 217, 255)));
        ui->customPlot->repaint();
    }
    else {
        ui->actionTurquoise->setChecked(true);
    }

}

void mainFrame::on_actionCharcoal_Grey_triggered()
{
    if (ui->actionCharcoal_Grey->isChecked() == true) {
        ui->actionCharcoal_Grey->setChecked(true);
        ui->actionTurquoise->setChecked(false);
        ui->actionPale->setChecked(false);
        ui->actionBeige->setChecked(false);
        ui->actionLight_Blue->setChecked(false);
        ui->actionLight_Grey->setChecked(false);
        ui->actionWhite->setChecked(false);
        ui->customPlot->setBackground(QBrush(QColor(140, 144, 145, 255)));
        ui->customPlot->repaint();
    }
    else {
        ui->actionCharcoal_Grey->setChecked(true);
    }

}

void mainFrame::on_actionLight_Grey_triggered()
{
    if (ui->actionLight_Grey->isChecked() == true) {
        ui->actionLight_Grey->setChecked(true);
        ui->actionTurquoise->setChecked(false);
        ui->actionPale->setChecked(false);
        ui->actionBeige->setChecked(false);
        ui->actionCharcoal_Grey->setChecked(false);
        ui->actionLight_Blue->setChecked(false);
        ui->actionWhite->setChecked(false);
        ui->customPlot->setBackground(QBrush(QColor(214,214,214,255)));
        ui->customPlot->repaint();
    }
    else {
        ui->actionLight_Grey->setChecked(true);
    }

}

void mainFrame::on_actionWhite_triggered()
{
    if (ui->actionWhite->isChecked() == true) {
        ui->actionWhite->setChecked(true);
        ui->actionTurquoise->setChecked(false);
        ui->actionPale->setChecked(false);
        ui->actionBeige->setChecked(false);
        ui->actionCharcoal_Grey->setChecked(false);
        ui->actionLight_Blue->setChecked(false);
        ui->actionLight_Grey->setChecked(false);
        ui->customPlot->setBackground(QBrush(QColor(252,252,252,255)));
        ui->customPlot->repaint();
    }
    else {
        ui->actionWhite->setChecked(true);
    }


}

void mainFrame::on_actionInvert_UI_Colour_triggered()
{
    if (ui->actionInvert_UI_Colour->isChecked() == true) {
        this->setStyleSheet("background-color: rgb(66, 65, 64);");
        ui->customPlot->repaint();
        ui->label_14->setStyleSheet("QLabel { color : white; }");
        ui->label_7->setStyleSheet("QLabel { color : white; }");
    }
    else {
        this->setStyleSheet("");
        ui->label_14->setStyleSheet("QLabel { color : black; }");
        ui->label_7->setStyleSheet("QLabel { color : black; }");
    }
}

void mainFrame::on_actionPrint_Report_triggered()
{

    QString text =
            "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do\n"
            "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut\n"
            "enim ad minim veniam, quis nostrud exercitation ullamco laboris\n"
            "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor\n"
            "in reprehenderit in voluptate velit esse cillum dolore eu fugiat\n"
            "nulla pariatur. Excepteur sint occaecat cupidatat non proident,\n"
            "sunt in culpa qui officia deserunt mollit anim id est laborum.\n";

    QPrinter printer;

    printer.setPrinterName("Printer name");
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Rejected) return;
}

bool mainFrame::on_actionSave_Report_triggered()
{
    //Get path to save file
    QString savefile = QFileDialog::getSaveFileName(this, tr("Save PEAT Report"), "untitled.peat", tr("PEAT Report File (*.peat)"));
    QFile file(savefile);
    if (!savefile.isNull()) {
        try {
            file.open(QIODevice::WriteOnly);
            //Check if file is open
            if (!file.isOpen()) throw 600;
            QDataStream stream(&file);

            //Check if a report is loaded
            if (vid_data.size() != 5) throw 605;

            // Write an ID header, file path, FPS, frame count, and set serialization version
            QString filename = ui->label->text();
            // Check if filename is a valid path
            if (!QFile::exists(filename)) throw 610;

            stream << (quint32)0xA46BF100 << filename << (quint32)vid_data[4][0] << (quint32)vid_data[4][1];
            stream.setVersion(QDataStream::Qt_5_11);

            //Write data to file
            stream << (QString)"flash_diag";
            for (unsigned int i = 0; i < vid_data[0].size(); i++) {
                stream << (quint32)vid_data[0][i];
            }
            stream << (QString)"red_flash_diag";
            for (unsigned int i = 0; i < vid_data[1].size(); i++) {
                stream << (quint32)vid_data[1][i];
            }
            stream << (QString)"seizure_frames";
            for (unsigned int i = 0; i < vid_data[2].size(); i++) {
                stream << (quint32)vid_data[2][i];
            }
            stream << (QString)"red_seizure_frames";
            for (unsigned int i = 0; i < vid_data[3].size(); i++) {
                stream << (quint32)vid_data[3][i];
            }
            saved = true;
            file.close();
        }
        catch (int e) {
            file.remove();
            QString error;
            switch(e)
            {
                case 600:
                    error = "The report file could not be written. Please try again and check the directory";
                    break;
                case 605:
                    error = "No report file loaded.";
                    break;
                case 610:
                    error = "The loaded video file is not valid. Please add the video back to the loaded path or reload the video and try again.";
                    break;
            }
            QMessageBox mb;
            mb.critical(this, "Error: " + QString::number(e), error);
            mb.setFixedSize(600,400);
        }
    }
    return saved;
}

void mainFrame::no_report_loaded() {
    vid_data.clear();
    saved = true;
    for (int i = 0; i < ui->customPlot->graphCount(); i++) {
        ui->customPlot->graph(i)->data()->clear();
    }
    warnings.clear();
    ui->label_6->setText("0");
    ui->backWarning->setEnabled(false);
    ui->forwardWarning->setEnabled(false);
    ui->actionPrint_Report->setEnabled(false);
    ui->actionSave_Report->setEnabled(false);
    ui->actionRun_Prophylactic_Tool->setEnabled(false);

    ui->horizontalScrollBar->setEnabled(false);
    ui->horizontalScrollBar->setRange(0, 100);
    ui->horizontalScrollBar->setValue(0);
    disconnect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzScrollBarChanged(int)));
    disconnect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));

}
