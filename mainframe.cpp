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
#include <QDate>
#include <algorithm>

using namespace std;
using namespace cv;

mainFrame::mainFrame(QWidget *parent) :
    QMainWindow(),
    ui(new Ui::mainFrame)
{
    ui->setupUi(this);
    this->setWindowTitle("UW Trace Center Photosensitive Epilepsy Analysis Tool (PEAT)");
    setWindowFlags(Qt::Widget | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    this->resize(QSize(863,585));
    this->setFixedSize(QSize(863,585));

    //Connect toolbar actions
    connect(ui->actionOpen_Video, &QAction::triggered, this, &mainFrame::on_folderButton_clicked);
    connect(ui->actionOpen_Report, &QAction::triggered, this, &mainFrame::openReport);
    connect(ui->actionGenerate_Report, &QAction::triggered, this, &mainFrame::on_reportButton_clicked);
    connect(ui->actionQuit, &QAction::triggered, qApp, QApplication::quit);
    connect(ui->actionRun_Prophylactic_Tool, &QAction::triggered, this, &mainFrame::openProTool);

    connect(ui->actionBack_5_Seconds, &QAction::triggered, this, &mainFrame::skipLeftFunc);
    connect(ui->actionForward_5_Seconds, &QAction::triggered, this, &mainFrame::skipRightFunc);
    connect(ui->actionPlay_Pause, &QAction::triggered, this, &mainFrame::playPauseFunc);
    connect(ui->actionForward_30_Frames, &QAction::triggered, this, &mainFrame::skipFrameRightFunc);
    connect(ui->actionBack_30_Frames, &QAction::triggered, this, &mainFrame::skipFrameLeftFunc);

    connect(ui->actionPEAT_Help, &QAction::triggered, this, &mainFrame::peatHelp);
    connect(ui->actionAbout_PEAT, &QAction::triggered, this, &mainFrame::aboutPEAT);
    connect(ui->actionTrace_Center_Homepage, &QAction::triggered, this, &mainFrame::traceHome);

    //Setup plot
    ui->customPlot->yAxis->setRange(0.0, 1.05);
    ui->customPlot->yAxis->setVisible(false);
    ui->customPlot->xAxis->setLabel("Frame Number");
    ui->customPlot->axisRect()->setAutoMargins(QCP::msNone);
    ui->customPlot->axisRect()->setMargins(QMargins(0,0,0,38));
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
    //section->topLeft->setCoords(0, 1.0); // the y value is now in axis rect ratios, so -0.1 is "barely above" the top axis rect border
    //section->bottomRight->setCoords(100000, 0.0); // the y value is now in axis rect ratios, so 1.1 is "barely below" the bottom axis rect border
    section->topLeft->setCoords(0, 2.0); // the y value is now in axis rect ratios, so -0.1 is "barely above" the top axis rect border
    section->bottomRight->setCoords(100000, -1.0); // the y value is now in axis rect ratios, so 1.1 is "barely below" the bottom axis rect border

    // ADDED: Changed color from 200 -> 194
    section->setBrush(QBrush(QColor(194,194,194,255)));
    section->setPen(Qt::NoPen);
    QCPItemLine *line = new QCPItemLine(ui->customPlot);
    // ADDED: Line begins at -10000 instead of 0
    line->start->setCoords(-100000, 0.26);
    line->end->setCoords(1000000, 0.26);
    line->setClipToAxisRect(false);
    textLabel = new QCPItemText(ui->customPlot);
    textLabel2 = new QCPItemText(ui->customPlot);

    textLabel->setPositionAlignment(Qt::AlignVCenter|Qt::AlignRight);
    textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
    textLabel->position->setCoords(0.2, 0.71); // place position at center/top of axis rect
    // ADDED: message changed from Caution (Fail/Pass)
    textLabel->setText("Status: FAIL");
    textLabel->setFont(QFont(font().family(), 10.5, QFont::Medium)); // make font a bit larger
    // ADDED: disabled border around text label
    //textLabel->setPen(QPen(Qt::black)); // show black border around text
    textLabel->setPen(Qt::NoPen); // show no border around text
    textLabel->setColor(QColor(214,214,214,255));

    textLabel2->setPositionAlignment(Qt::AlignVCenter|Qt::AlignRight);
    textLabel2->position->setType(QCPItemPosition::ptAxisRectRatio);
    textLabel2->position->setCoords(0.209, 0.78); // place position at center/top of axis rect
    textLabel2->setText("Status: PASS");
    textLabel2->setFont(QFont(font().family(), 10.5, QFont::Medium)); // make font a bit larger
    //textLabel2->setPen(QPen(Qt::black)); // show black border around text
    textLabel2->setPen(Qt::NoPen); // show no border around text
    textLabel2->setColor(QColor(214,214,214,255));

    ui->customPlot->xAxis->scaleRange(32.0, ui->customPlot->xAxis->range().center());

    ui->customPlot->replot();

    //Set frame for QCP
    QPainterPath path;
    path.addRoundedRect(ui->customPlot->rect(), 10, 10);
    QRegion mask = QRegion(path.toFillPolygon().toPolygon());
    ui->customPlot->setMask(mask);

    //Declare player and set properties
    player = new QMediaPlayer;
    playlist = new QMediaPlaylist;

    player->setNotifyInterval(40);
    connect(player, &QMediaPlayer::durationChanged, ui->slider, &QSlider::setMaximum);
    connect(player, &QMediaPlayer::positionChanged, ui->slider, &QSlider::setValue);
    connect(ui->slider, &QSlider::valueChanged, player, &QMediaPlayer::setPosition);
    connect(ui->slider, &QSlider::valueChanged, ui->horizontalScrollBar, [&](qint64 move) {
        qint64 sliderMax = ui->slider->maximum();
        qint64 scrollMax = ui->horizontalScrollBar->maximum();
        double percent = move/(double)(sliderMax);
        ui->horizontalScrollBar->setValue(percent*(double)scrollMax);
    });
    QImage image(":/images/Res/logoLARGE.bmp");
    QImage out(image.width(), image.height(), QImage::Format_ARGB32);
    out.fill(Qt::transparent);

    QBrush brush(image);

    QPen pen;
    pen.setColor(Qt::darkGray);
    pen.setJoinStyle(Qt::RoundJoin);

    QPainter painter(&out);
    painter.setBrush(brush);
    painter.setPen(pen);
    painter.drawRoundedRect(0, 0, image.width(), image.height(), 15, 15);

    ui->label_14->setPixmap(QPixmap::fromImage(out.scaled(35,35)));

    //player status handling
    connect(player, &QMediaPlayer::stateChanged, this, [&](QMediaPlayer::State state) {
        if (state == QMediaPlayer::StoppedState) {
            ui->rewindButton->setStyleSheet("#rewindButton { background-image: url(:/images/Res/FastBackDisabled.png); border-image: url(:/images/Res/FastBackDisabled.png); }");
            ui->pauseButton->setStyleSheet("background-image: url(:/images/Res/StopDisabled.png); border-image: url(:/images/Res/StopDisabled.png);");
            ui->forwardButton->setStyleSheet("background-image: url(:/images/Res/FFwdDisabled.png); border-image: url(:/images/Res/FFwdDisabled.png);");
            ui->reportButton->setStyleSheet("background-image: url(:/images/Res/AnalyzeDisabled.png); border-image: url(:/images/Res/AnalyzeDisabled.png);");
            ui->restartButton->setStyleSheet("background-image: url(:/images/Res/RewindDisabled.png); border-image: url(:/images/Res/RewindDisabled.png);");

            ui->restartButton->setEnabled(false);
            ui->rewindButton->setEnabled(false);
            ui->pauseButton->setEnabled(false);
            ui->forwardButton->setEnabled(false);
            ui->reportButton->setEnabled(false);

            ui->actionGenerate_Report->setEnabled(false);
            ui->actionBack_5_Seconds->setEnabled(false);
            ui->actionBack_30_Frames->setEnabled(false);
            ui->actionForward_5_Seconds->setEnabled(false);
            ui->actionForward_30_Frames->setEnabled(false);
            ui->actionPlay_Pause->setEnabled(false);

            ui->label_14->setVisible(true);
            ui->slider->setValue(0);
            vidLabel->setText(tr("Stopped"));
        }
        else if (state == QMediaPlayer::PausedState){
            vidLabel->setText(tr("Paused"));
        }
        else if (state == QMediaPlayer::PlayingState) {
            vidLabel->setText(tr("Playing"));
        }
    });
    connect(player, &QMediaPlayer::mediaStatusChanged,
            [&](QMediaPlayer::MediaStatus status){
        qDebug() << status;
        if(status == QMediaPlayer::BufferedMedia) {
            ui->rewindButton->setStyleSheet("#rewindButton { background-image: url(:/images/Res/FastBackUp.png); border-image: url(:/images/Res/FastBackUp.png); } #rewindButton:hover { border-image: url(:/images/Res/FastBackDownHilite.png); } #rewindButton:pressed { border-image: url(:/images/Res/FastBackPressed.png); }");
            ui->pauseButton->setStyleSheet("#pauseButton { background-image: url(:/images/Res/StopUp.png); border-image: url(:/images/Res/StopUp.png); } #pauseButton:hover { border-image: url(:/images/Res/StopDownHilite.png); } #pauseButton:pressed { border-image: url(:/images/Res/StopPressed.png); }");
            ui->forwardButton->setStyleSheet("#forwardButton { background-image: url(:/images/Res/FFwdUp.png); border-image: url(:/images/Res/FFwdUp.png); } #forwardButton:hover { border-image: url(:/images/Res/FFwdDownHilite.png); } #forwardButton:pressed { border-image: url(:/images/Res/FFwdPressed.png); }");
            ui->reportButton->setStyleSheet("#reportButton { background-image: url(:/images/Res/AnalyzeUp.png); border-image: url(:/images/Res/AnalyzeUp.png); } #reportButton:hover { border-image: url(:/images/Res/AnalyzeDownHilite.png); } #reportButton:pressed { border-image: url(:/images/Res/AnalyzePressed.png); }");
            ui->restartButton->setStyleSheet("#restartButton { background-image: url(:/images/Res/RewindUp.png); border-image: url(:/images/Res/RewindUp.png); } #restartButton:hover { border-image: url(:/images/Res/RewindDownHilite.png); } #restartButton:pressed { border-image: url(:/images/Res/RewindPressed.png); }");

            ui->restartButton->setEnabled(true);
            ui->rewindButton->setEnabled(true);
            ui->pauseButton->setEnabled(true);
            ui->forwardButton->setEnabled(true);
            ui->reportButton->setEnabled(true);

            ui->actionGenerate_Report->setEnabled(true);
            ui->actionBack_5_Seconds->setEnabled(true);
            ui->actionBack_30_Frames->setEnabled(true);
            ui->actionForward_5_Seconds->setEnabled(true);
            ui->actionForward_30_Frames->setEnabled(true);
            ui->actionPlay_Pause->setEnabled(true);

            //TO-DO: Format time
            if (media_first_load == true) {
                player->play();
                ui->label_10->setText(QString::number(player->duration()));
            }

            ui->label_14->setVisible(false);
        }
        else if (status == QMediaPlayer::LoadedMedia) {
            if (media_first_load == true) {
                player->play();
                ui->label_10->setText(QString::number(player->duration()));
                media_first_load = false;
            }
        }
        else if (status == QMediaPlayer::NoMedia || status == QMediaPlayer::EndOfMedia) player->stop();
    });

    //Create statusbar
    vidLabel = new QLabel;
    vidLabel->setText("No media loaded");
    vidLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    descriptionLabel = new QLabel;
    descriptionLabel->setText("");
    descriptionLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    placeholderLabel = new QLabel;
    placeholderLabel->setText("");
    placeholderLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    ui->statusbar->addPermanentWidget(vidLabel, 2);
    ui->statusbar->addPermanentWidget(descriptionLabel, 6);
    ui->statusbar->addPermanentWidget(placeholderLabel, 2);

    //Create shortcuts
    ui->actionOpen_Video->setShortcut(Qt::Key_V | Qt::CTRL);
    ui->actionOpen_Report->setShortcut(Qt::Key_R | Qt::CTRL);
    ui->actionSave_Report->setShortcut(Qt::Key_S | Qt::CTRL);
    ui->actionPrint_Report->setShortcut(Qt::Key_P | Qt::CTRL);
    ui->actionRun_Prophylactic_Tool->setShortcut(Qt::Key_X | Qt::CTRL);
    ui->actionGenerate_Report->setShortcut(Qt::Key_G | Qt::CTRL);
    ui->actionPlay_Pause->setShortcut(Qt::Key_Space);
    ui->actionForward_5_Seconds->setShortcut(Qt::Key_L);
    ui->actionBack_5_Seconds->setShortcut(Qt::Key_J);
    ui->actionForward_30_Frames->setShortcut(Qt::Key_Right);
    ui->actionBack_30_Frames->setShortcut(Qt::Key_Left);

    //Install event filters
    ui->folderButton->installEventFilter(this);
    ui->reportButton->installEventFilter(this);
    ui->playButton->installEventFilter(this);
    ui->pauseButton->installEventFilter(this);
    ui->rewindButton->installEventFilter(this);
    ui->forwardButton->installEventFilter(this);
    ui->restartButton->installEventFilter(this);
    ui->label->installEventFilter(this);
    ui->label_2->installEventFilter(this);
    ui->label_5->installEventFilter(this);
    ui->label_6->installEventFilter(this);
    ui->label_7->installEventFilter(this);
    ui->label_8->installEventFilter(this);
    ui->label_9->installEventFilter(this);
    ui->label_10->installEventFilter(this);
    ui->label_11->installEventFilter(this);
    ui->label_12->installEventFilter(this);
    ui->label_13->installEventFilter(this);
    ui->label_14->installEventFilter(this);
    ui->frame_2->installEventFilter(this);
    ui->backWarning->installEventFilter(this);
    ui->forwardWarning->installEventFilter(this);
    ui->customPlot->installEventFilter(this);
    ui->videoWidget_2->installEventFilter(this);
    ui->slider->installEventFilter(this);
    ui->horizontalScrollBar->installEventFilter(this);
    ui->horizontalSlider->installEventFilter(this);
    ui->timeLabel->installEventFilter(this);

    //Plot tooltip
    connect(ui->customPlot, &QCustomPlot::mouseMove, this, &mainFrame::plotTooltip);
    ui->customPlot->setInteraction(QCP::iSelectPlottables, true);

    //Legend
    QFont legendFont = font();
    legendFont.setPointSize(8);
    ui->customPlot->legend->setBrush(QBrush(QColor(115,115,115, 115)));
    ui->customPlot->legend->setFont(legendFont);
    ui->customPlot->legend->setSelectedFont(legendFont);
    ui->customPlot->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

    phaseTracer = new QCPItemTracer(ui->customPlot);
    // ADDED
    ui->customPlot->addLayer("tracer", ui->customPlot->layer("overlay"), QCustomPlot::limAbove);
    phaseTracer->setLayer("tracer");
    // End
    phaseTracer->setInterpolating(true);
    phaseTracer->setStyle(QCPItemTracer::tsCircle);
    phaseTracer->setPen(QPen(QColor(8,54,117)));
    phaseTracer->setBrush(QColor(8,54,117));
    phaseTracer->setSize(7);
    phaseTracer->setSelectable(false);
    phaseTracer->setVisible(false);

    // ADDED: Caution layer
    cautionLayer = new QCPItemRect(ui->customPlot);
    ui->customPlot->addLayer("cautionLayer", ui->customPlot->layer("grid"), QCustomPlot::limBelow);
    cautionLayer->setLayer("cautionLayer");
    cautionLayer->topLeft->setTypeX(QCPItemPosition::ptPlotCoords);
    cautionLayer->topLeft->setTypeY(QCPItemPosition::ptAxisRectRatio);
    cautionLayer->topLeft->setAxes(ui->customPlot->xAxis, ui->customPlot->yAxis);
    cautionLayer->topLeft->setAxisRect(ui->customPlot->axisRect());
    cautionLayer->bottomRight->setTypeX(QCPItemPosition::ptPlotCoords);
    cautionLayer->bottomRight->setTypeY(QCPItemPosition::ptAxisRectRatio);
    cautionLayer->bottomRight->setAxes(ui->customPlot->xAxis, ui->customPlot->yAxis);
    cautionLayer->bottomRight->setAxisRect(ui->customPlot->axisRect());
    cautionLayer->setClipToAxisRect(true); // is by default true already, but this will change in QCP 2.0.0
    cautionLayer->topLeft->setCoords(-100000, 0.68); // the y value is now in axis rect ratios, so -0.1 is "barely above" the top axis rect border
    cautionLayer->bottomRight->setCoords(1000000, 0.82); // the y value is now in axis rect ratios, so 1.1 is "barely below" the bottom axis rect border
    cautionLayer->setBrush(QBrush(QColor(184,184,184,255)));
    cautionLayer->setPen(Qt::NoPen);

    //Setup x-axis

    //Graph interactions
    ui->customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->customPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

    // Bug fix: force update label font
    QFont font("Segoe UI", 11, QFont::Bold);
    ui->label->setFont(font);
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
            delete player;
            event->accept();
        }
        else {
            event->ignore();
        }
    }
    else {
        delete player;
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
    Mat frame;
    VideoCapture cap;

    bool vid_is_valid = true;

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
    if (ext != ""){
        didSelect = true;
        cap.open(fileString);
        if (!cap.isOpened()) isVideo = false;
        cap >> frame;
        if (frame.empty()) isVideo = false;
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
                    QString dimensions = QString::number(frame.cols) + "x" + QString::number(frame.rows);
                    double fps = cap.get(CAP_PROP_FPS);
                    quint32 frame_count = cap.get(CAP_PROP_FRAME_COUNT);
                    int fourcc = cap.get(CAP_PROP_FOURCC);
                    string fourcc_str = format("%c%c%c%c", fourcc & 255, (fourcc >> 8) & 255, (fourcc >> 16) & 255, (fourcc >> 24) & 255);
                    cap.release();

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
                    media_first_load = true;

                    player->setPlaylist(playlist);
                    player->setVideoOutput(ui->videoWidget_2);
                    ui->videoWidget_2->show();
                    ui->label_14->setVisible(false);
                    vid_file = filename;
                    string s = filename.toStdString();
                    auto pos = s.rfind('/');
                    if (pos != std::string::npos) {
                        s.erase(0, pos+1);
                    }
                    ui->label->setText(QString::fromStdString(s));
                    ui->timeLabel->setStyleSheet("background-color:#EBFFCF; border-style:solid; border-radius:5px; border-color:#406D00; border-width:2px; color: #406D00;");
                    nFrame = frame_count;
                    ui->label_8->setText(QString::number(frame_count));
                    ui->label_9->setText(QString::number(fps));
                    ui->label_10->setText("...");
                    ui->label_11->setText(dimensions);
                    ui->label_12->setText(QString::fromStdString(fourcc_str));

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
                    ui->playButton->setStyleSheet("#playButton { background-image: url(:/images/Res/PlayUp.png); border-image: url(:/images/Res/PlayUp.png); } #playButton:hover { border-image: url(:/images/Res/PlayDownHilite.png); } #playButton:pressed { border-image: url(:/images/Res/PlayPressed.png); }");
                    ui->playButton->setEnabled(true);

                    vidLabel->setText("Ready");
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
    emit stopped();
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

void mainFrame::updateSlider(int moved, int frameNum) {
    int delta = (int)(((double)moved/frameNum)*ui->slider->maximum());
    ui->slider->setValue(delta);
}

void mainFrame::updatePlot(vector<QVector<double > > points_x, vector<QVector<double > > points_y) {
    //Plot
    //qDebug() << "DATA: " << points_y[0][points_y[0].size()-1];

    vid_data[0].push_back(points_y[0][points_y[0].size()-1]*10);
    vid_data[1].push_back(points_y[1][points_y[1].size()-1]*10);
    //qDebug() << points_y[0][points_y[0].size()-1]*10 << ", " << vid_data[0][vid_data[0].size()-1];
    vid_data[2].push_back(points_y[2][points_y[2].size()-1]);
    vid_data[3].push_back(points_y[3][points_y[3].size()-1]);

    ui->customPlot->graph(0)->addData(points_x[0], points_y[0]);
    ui->customPlot->graph(1)->addData(points_x[1], points_y[1]);
    ui->customPlot->graph(2)->addData(points_x[2], points_y[2]);
    ui->customPlot->graph(3)->addData(points_x[3], points_y[3]);
    if ((int)points_x[0][0] % 3 == 0) ui->customPlot->replot();

    //Update slider and warnings
    int currentRed = vid_data[3][vid_data[3].size()-1];
    int currentLum = vid_data[2][vid_data[2].size()-1];
    int previous = vid_data[2][vid_data[2].size()-2] | vid_data[3][vid_data[3].size()-2];
    int current = currentRed | currentLum;
    double ind = points_x[0][0];
    firstHalfStylesheet = firstHalfStylesheet.substr(0, firstHalfStylesheet.length()-21);
    if (previous == 0 && current == 1) {
        warnings.push_back(points_x[2][0]);
        ui->label_6->setText(QString::number(ui->label_6->text().toInt() + 1));
        firstHalfStylesheet = firstHalfStylesheet + "stop:" + to_string(round(1000.0*((ind-1)/vid_data[4][1]+0.001))/1000.0) + "#c10707,stop:" + to_string(ind/vid_data[4][1]) + "#c10707,stop:" + to_string(round(1000.0*(ind/vid_data[4][1]+0.001))/1000.0) + "#6d6b6b,";
    }
    else if (previous == 1 && current == 0) {
        firstHalfStylesheet = firstHalfStylesheet + "stop:" + to_string(round(1000.0*((ind-1)/vid_data[4][1]+0.001))/1000.0) + "#25c660,stop:" + to_string(ind/vid_data[4][1]) + "#25c660,stop:" + to_string(round(1000.0*(ind/vid_data[4][1]+0.001))/1000.0) + "#6d6b6b,";
    }
    else if (previous == 0 && current == 0) {
        firstHalfStylesheet = firstHalfStylesheet + "stop:" + to_string(round(1000.0*ind/vid_data[4][1])/1000.0) + "#25c660,stop:" + to_string(round(1000.0*(ind/vid_data[4][1]+0.001))/1000.0) + "#6d6b6b,";
    }
    else {
        firstHalfStylesheet = firstHalfStylesheet + "stop:" + to_string(round(1000.0*ind/vid_data[4][1])/1000.0) + "#c10707,stop:" + to_string(round(1000.0*(ind/vid_data[4][1]+0.001))/1000.0) + "#6d6b6b,";
    }
    ui->slider->setStyleSheet(QString::fromStdString(firstHalfStylesheet) + secondHalfStylesheet);

    if (currentLum) {
        ui->label_16->setText(QString::number(ui->label_16->text().toInt() + 1));
    }
    if (currentRed) {
        ui->label_20->setText(QString::number(ui->label_20->text().toInt() + 1));
    }
    if (currentRed | currentLum) {
        ui->label_22->setText(QString::number(ui->label_22->text().toInt() + 1));
    }
    emit received();

    //if ((int)points_x[0][points_x[0].size()-1] % 10 == 0) ui->slider->setValue(points_x[0][0]);
}

void mainFrame::on_reportButton_clicked() {
    try {
        ui->customPlot->clearPlottables();
        ui->frame_2->setEnabled(true);
        ui->label_18->setEnabled(true);
        ui->label_19->setText("NONE");
        ui->label_19->setStyleSheet("color: rgb(193, 147, 28);");
        ui->label_16->setText("0");
        ui->label_20->setText("0");
        ui->label_22->setText("0");
        player->pause();
        ui->label_6->setText("0");
        reset_slider();
        QString filename = vid_file;
        std::string stringedFile = filename.toLocal8Bit().constData();
        VideoCapture cap(stringedFile);
        int frameNum = cap.get(CAP_PROP_FRAME_COUNT);
        int fps = round(cap.get(CAP_PROP_FPS));
        if (frameNum < 1) throw frameNum;
        Mat frame;
        cap >> frame;
        if (frame.empty()) throw 210;
        cap.release();

        //Initialize scrollbar
        ui->horizontalScrollBar->setEnabled(true);
        ui->horizontalScrollBar->setRange(0, frameNum);
        connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzScrollBarChanged(int)));
        connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));

        //Disable interaction
        ui->rewindButton->setStyleSheet("#rewindButton { background-image: url(:/images/Res/FastBackDisabled.png); border-image: url(:/images/Res/FastBackDisabled.png); }");
        ui->playButton->setStyleSheet("background-image: url(:/images/Res/PlayDisabled.png); border-image: url(:/images/Res/PlayDisabled.png);");
        ui->forwardButton->setStyleSheet("background-image: url(:/images/Res/FFwdDisabled.png); border-image: url(:/images/Res/FFwdDisabled.png);");
        ui->reportButton->setStyleSheet("background-image: url(:/images/Res/AnalyzeDisabled.png); border-image: url(:/images/Res/AnalyzeDisabled.png);");
        ui->restartButton->setStyleSheet("background-image: url(:/images/Res/RewindDisabled.png); border-image: url(:/images/Res/RewindDisabled.png);");
        //ui->folderButton->setStyleSheet("background-image: url(:/images/Res/EjectDisabled.png); border-image: url(:/images/Res/EjectDisabled.png);");

        ui->restartButton->setEnabled(false);
        ui->rewindButton->setEnabled(false);
        ui->playButton->setEnabled(false);
        ui->forwardButton->setEnabled(false);
        ui->reportButton->setEnabled(false);
        ui->folderButton->setEnabled(false);

        ui->actionGenerate_Report->setEnabled(false);
        ui->actionBack_5_Seconds->setEnabled(false);
        ui->actionBack_30_Frames->setEnabled(false);
        ui->actionForward_5_Seconds->setEnabled(false);
        ui->actionForward_30_Frames->setEnabled(false);
        ui->actionPlay_Pause->setEnabled(false);
        ui->actionOpen_Report->setEnabled(false);
        ui->actionOpen_Video->setEnabled(false);

        //Initialize graphs
        ui->customPlot->addGraph();
        ui->customPlot->addGraph();
        ui->customPlot->addGraph();
        ui->customPlot->addGraph();
        ui->customPlot->graph(0)->setName("Luminance flash diag.");
        ui->customPlot->graph(1)->setName("Red flash diag.");
        ui->customPlot->graph(2)->setName("Luminance flash");
        ui->customPlot->graph(3)->setName("Red flash");

        ui->customPlot->yAxis->setRange(0.0, 1.05);
        ui->customPlot->xAxis->setLabel("Frame Number");
        ui->customPlot->yAxis->setVisible(false);
        section->bottomRight->setCoords(frameNum, 0.0); // the y value is now in axis rect ratios, so 1.1 is "barely below" the bottom axis rect border
        ui->customPlot->replot();

        //Graph 1 style
        QPen solid;
        solid.setStyle(Qt::DotLine);
        solid.setWidthF(1.5);
        solid.setColor(Qt::black);
        ui->customPlot->graph(0)->setPen(solid);
        ui->customPlot->graph(0)->setLineStyle(QCPGraph::lsLine);
        ui->customPlot->graph(0)->addData(0.0, 0.0);
        //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

        //Graph 2 style
        QPen solidRed;
        solidRed.setStyle(Qt::DotLine);
        solidRed.setWidthF(1.5);
        solidRed.setColor(Qt::red);
        ui->customPlot->graph(1)->setPen(solidRed);
        ui->customPlot->graph(1)->setLineStyle(QCPGraph::lsLine);
        ui->customPlot->graph(1)->addData(0.0, 0.0);
        //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

        //Graph 3 style
        QPen dotted;
        dotted.setStyle(Qt::SolidLine);
        dotted.setWidthF(3);
        dotted.setColor(Qt::white);
        ui->customPlot->graph(2)->setPen(dotted);
        ui->customPlot->graph(2)->setLineStyle(QCPGraph::lsLine);
        ui->customPlot->graph(2)->addData(0.0, 0.0);
        //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

        //Graph 4 style
        QPen dottedRed;
        dottedRed.setStyle(Qt::SolidLine);
        dottedRed.setWidthF(3);
        dottedRed.setColor(Qt::red);
        ui->customPlot->graph(3)->setPen(dottedRed);
        ui->customPlot->graph(3)->setLineStyle(QCPGraph::lsLine);
        ui->customPlot->graph(3)->addData(0.0, 0.0);
        ui->customPlot->replot();
        //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
        ui->customPlot->legend->setVisible(true);

        //Connect slots (slider and QCP)
        vidLabel->setText("Analysing...");
        rObject* r_instance = new rObject;
        connect(r_instance, &rObject::updateUI, this, updatePlot, Qt::QueuedConnection);
        connect(r_instance, &rObject::progressCount, this, updateSlider, Qt::QueuedConnection);

        vid_data.clear();
        vid_data.resize(5);
        vid_data[4].push_back(fps);
        vid_data[4].push_back(frameNum);
        vid_data[0].push_back(0);
        vid_data[1].push_back(0);
        vid_data[2].push_back(0);
        vid_data[3].push_back(0);

        ui->customPlot->legend->setVisible(true);
        //r_instance->rkbcore(stringedFile);
        //Threading attempt
        QThread* workerThread = new QThread;
        r_instance->moveToThread(workerThread);
        connect(workerThread, &QThread::started, r_instance, [&]{
            r_instance->rkbcore(stringedFile);
        });
        connect(r_instance, &rObject::finished, workerThread, &QThread::quit);
        connect(r_instance, &rObject::finished, r_instance, &rObject::deleteLater);
        connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);

        bool threadStopped = false;
        QEventLoop loop;
        workerThread->start();
        connect(r_instance, &rObject::finished, &loop, &QEventLoop::quit);
        connect(r_instance, &rObject::error, &loop, &QEventLoop::quit);

        // TO-DO: Stopping
        connect(ui->pauseButton, &QPushButton::clicked, r_instance, &rObject::stopLoop, Qt::DirectConnection);
        connect(r_instance, &rObject::stopped, this, [&]{
            threadStopped = true;
        });
        /*connect(this, &mainFrame::thread_stopped, [=]{
            workerThread->terminate();
            r_instance->~rObject();
        });*/
        //connect(this, &mainFrame::thread_stopped, &loop, &QEventLoop::quit);
        loop.exec();
        if (!threadStopped) {
            ui->slider->setStyleSheet(QString::fromStdString(firstHalfStylesheet) + "stop:1.0#25c660); margin: 2px 0; } QSlider::handle:horizontal { background-color: rgba(143,143,143, 255); border: 1px solid rgb(143,143,143); width: 8px; margin: -6px 0; border-radius: 5px; }");

            //Check to see vid_data has valid arrays of data points
            if (vid_data.size() < 4) {
                throw 220;
            }
            else if (vid_data[0].size() != frameNum || vid_data[1].size() != frameNum || vid_data[2].size() != frameNum || vid_data[3].size() != frameNum) {
                throw 230;
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

            ui->actionSave_Report->setEnabled(true);
            ui->actionPrint_Report->setEnabled(true);
            ui->actionRun_Prophylactic_Tool->setEnabled(true);

            QDate date = QDate::currentDate();
            qDebug() << date;
            ui->label_13->setText(date.toString("dd.MM.yyyy"));

            //Set report data to unsaved
            saved = false;
            vidLabel->setText("Done");

            if (ui->label_16->text() != "0" || ui->label_20->text() != "0" || ui->label_22->text() != "0") {
                ui->label_19->setText("FAIL");
                ui->label_19->setStyleSheet("color: #c10707;");
            } else {
                ui->label_19->setText("PASS");
                ui->label_19->setStyleSheet("color: #25c660;");
            }
        }
        else {
            vidLabel->setText("Analysis stopped");
        }
    }
    catch(int e) {
        vidLabel->setText(tr("Error"));
        QMessageBox mb;
        mb.critical(this, "Error: " + QString::number(e), "There was an error in decoding the video stream. Please try and install the appropriate codecs and try again.");
        mb.setFixedSize(600,400);
    }
    ui->rewindButton->setStyleSheet("#rewindButton { background-image: url(:/images/Res/FastBackUp.png); border-image: url(:/images/Res/FastBackUp.png); } #rewindButton:hover { border-image: url(:/images/Res/FastBackDownHilite.png); } #rewindButton:pressed { border-image: url(:/images/Res/FastBackPressed.png); }");
    ui->playButton->setStyleSheet("#playButton { background-image: url(:/images/Res/PlayUp.png); border-image: url(:/images/Res/PlayUp.png); } #playButton:hover { border-image: url(:/images/Res/PlayDownHilite.png); } #playButton:pressed { border-image: url(:/images/Res/PlayPressed.png); }");
    ui->forwardButton->setStyleSheet("#forwardButton { background-image: url(:/images/Res/FFwdUp.png); border-image: url(:/images/Res/FFwdUp.png); } #forwardButton:hover { border-image: url(:/images/Res/FFwdDownHilite.png); } #forwardButton:pressed { border-image: url(:/images/Res/FFwdPressed.png); }");
    ui->reportButton->setStyleSheet("#reportButton { background-image: url(:/images/Res/AnalyzeUp.png); border-image: url(:/images/Res/AnalyzeUp.png); } #reportButton:hover { border-image: url(:/images/Res/AnalyzeDownHilite.png); } #reportButton:pressed { border-image: url(:/images/Res/AnalyzePressed.png); }");
    ui->restartButton->setStyleSheet("#restartButton { background-image: url(:/images/Res/RewindUp.png); border-image: url(:/images/Res/RewindUp.png); } #restartButton:hover { border-image: url(:/images/Res/RewindDownHilite.png); } #restartButton:pressed { border-image: url(:/images/Res/RewindPressed.png); }");
    ui->folderButton->setStyleSheet("#folderButton { background-image: url(:/images/Res/EjectUp.bmp); border-image: url(:/images/Res/EjectUp.bmp); } #folderButton:hover { border-image: url(:/images/Res/EjectDownHilite.png); } #folderButton:pressed { border-image: url(:/images/Res/EjectPressed.png); }");

    ui->restartButton->setEnabled(true);
    ui->rewindButton->setEnabled(true);
    ui->playButton->setEnabled(true);
    ui->forwardButton->setEnabled(true);
    ui->reportButton->setEnabled(true);
    ui->folderButton->setEnabled(true);

    ui->actionGenerate_Report->setEnabled(true);
    ui->actionBack_5_Seconds->setEnabled(true);
    ui->actionBack_30_Frames->setEnabled(true);
    ui->actionForward_5_Seconds->setEnabled(true);
    ui->actionForward_30_Frames->setEnabled(true);
    ui->actionPlay_Pause->setEnabled(true);
    ui->actionOpen_Report->setEnabled(true);
    ui->actionOpen_Video->setEnabled(true);
    // ADDED
    ui->actionShow_all_graphs->setEnabled(true);
    ui->actionHide_All_Graphs->setEnabled(true);
    ui->actionHide_Selected_Graph->setEnabled(true);
    ui->actionLuminance_diag_graph->setEnabled(true);
    ui->actionRed_Flash_Diag_Graph->setEnabled(true);
    ui->actionLuminance_Flash_Graph->setEnabled(true);
    ui->actionRed_Flash_Graph->setEnabled(true);
    ui->actionPlot_Tooltips->setEnabled(true);
    this->repaint();
    qApp->processEvents();
}

void mainFrame::openReport()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open a report file", "", "Open PEAT Report (*.peat)");
    QFile file(filename);
    QString vidFile = "/";
    bool continue_loading = true;

    if (!filename.isNull()) {
        try {
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
                vidLabel->setText("Loading report...");
                player->pause();
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
                vidFile = filename;
                if (!QFile::exists(filename)) throw 511;
                VideoCapture cap(filename.toStdString());
                Mat frame;
                cap >> frame;
                if (frame.empty()) throw 512;
                QString dimensions = QString::number(frame.cols) + "x" + QString::number(frame.rows);

                // Check video specs
                quint32 fps;
                stream >> fps;
                if (fps != round(cap.get(CAP_PROP_FPS))) throw 513;
                quint32 frame_count;
                stream >> frame_count;
                if (frame_count != cap.get(CAP_PROP_FRAME_COUNT)) throw 514;
                int fourcc = cap.get(CAP_PROP_FOURCC);
                string fourcc_str = format("%c%c%c%c", fourcc & 255, (fourcc >> 8) & 255, (fourcc >> 16) & 255, (fourcc >> 24) & 255);
                cap.release();

                //Get date of analysis
                QString date;
                stream >> date;
                if (date.isEmpty()) throw 514;

                //Declare vid_data template
                vector<vector<int > > vid_data_t(5);
                vid_data_t[4].push_back(fps);
                vid_data_t[4].push_back(frame_count);

                //Declare plot data
                QVector<double> diag_x, diag_y, red_diag_x, red_diag_y, flash_x, flash_y, red_flash_x, red_flash_y;
                diag_x.push_back(0.0);
                diag_y.push_back(0.0);
                red_diag_x.push_back(0.0);
                red_diag_y.push_back(0.0);
                flash_x.push_back(0.0);
                flash_y.push_back(0.0);
                red_flash_x.push_back(0.0);
                red_flash_y.push_back(0.0);

                //READ data from file TO-DO: Write data to QCP (BUGGY PLOT)
                QString header;
                stream >> header;
                if (header != "flash_diag") throw 520;
                for (unsigned int i = 0; i < frame_count; i++) {
                    quint32 n;
                    stream >> n;

                    if (i != 0) {
                        if (n != diag_y[diag_y.size()-1]*10) {
                            diag_x.push_back((double)i);
                            diag_y.push_back(diag_y[diag_y.size()-1]);
                            diag_x.push_back((double)i);
                            diag_y.push_back((double)n/10.0);
                        }
                        else {
                            diag_x.push_back((double)i);
                            diag_y.push_back((double)n/10.0);
                        }
                    }
                    vid_data_t[0].push_back((int)n);
                }
                stream >> header;
                if (header != "red_flash_diag") throw 521;
                for (unsigned int i = 0; i < frame_count; i++) {
                    quint32 n;
                    stream >> n;
                    if (i != 0) {
                        if (n != red_diag_y[red_diag_y.size()-1]*10) {
                            red_diag_x.push_back((double)i);
                            red_diag_y.push_back(red_diag_y[red_diag_y.size()-1]);
                            red_diag_x.push_back((double)i);
                            red_diag_y.push_back((double)n/10.0);
                        }
                        else {
                            red_diag_x.push_back((double)i);
                            red_diag_y.push_back((double)n/10.0);
                        }
                    }
                    vid_data_t[1].push_back((int)n);
                }
                stream >> header;
                if (header != "seizure_frames") throw 522;
                for (unsigned int i = 0; i < frame_count; i++) {
                    quint32 n;
                    stream >> n;

                    if (i != 0) {
                        if (n != (int)flash_y[flash_y.size()-1]) {
                            flash_x.push_back((double)i);
                            flash_y.push_back(flash_y[flash_y.size()-1]);
                            flash_x.push_back((double)i);
                            flash_y.push_back((double)n);
                        }
                        else {
                            flash_x.push_back((double)i);
                            flash_y.push_back((double)n);
                        }
                    }
                    vid_data_t[2].push_back((int)n);
                }
                stream >> header;
                if (header != "red_seizure_frames") throw 523;
                for (unsigned int i = 0; i < frame_count; i++) {
                    quint32 n;
                    stream >> n;

                    if (i != 0) {
                        if (n != (int)red_flash_y[red_flash_y.size()-1]) {
                            red_flash_x.push_back((double)i);
                            red_flash_y.push_back(red_flash_y[red_flash_y.size()-1]);
                            red_flash_x.push_back((double)i);
                            red_flash_y.push_back((double)n);
                        }
                        else {
                            red_flash_x.push_back((double)i);
                            red_flash_y.push_back((double)n);
                        }
                    }
                    vid_data_t[3].push_back((int)n);
                }
                qDebug() << "success";



                //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

                //Check to see vid_data has valid arrays of data points
                if (vid_data_t.size() < 4) {
                    throw 220;
                }
                else {
                    if (vid_data_t[0].size() != frame_count || vid_data_t[1].size() != frame_count || vid_data_t[2].size() != frame_count || vid_data_t[3].size() != frame_count) throw 230;
                }
                vid_data = vid_data_t;
                vid_data_t.clear();

                //Initialize slider and scrollbar
                ui->horizontalScrollBar->setValue(0);
                ui->slider->setEnabled(true);
                ui->slider->setStyleSheet("QSlider::groove:horizontal { height: 8px; background: qlineargradient(x1:0, y1:0, x2:1, y2:0,stop:0.000000#6d6b6b,stop:1.0#6d6b6b); margin: 2px 0; } QSlider::handle:horizontal { background-color: #8f8f8f; border: 1px solid #5c5c5c; width: 8px; margin: -6px 0; border-radius: 5px; }");
                ui->horizontalSlider->setEnabled(true);
                ui->horizontalSlider->setStyleSheet("QSlider::groove:horizontal { border: 1px solid #999999; height: 5px; background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0.0 #B1B1B1, stop: 1.0 #c4c4c4); margin: 2px 0; } QSlider::handle:horizontal { background: #8f8f8f; border: 1px solid #5c5c5c; width: 8px; margin: -6px 0; border-radius: 3px; }");
                ui->horizontalSlider->setEnabled(true);
                ui->horizontalSlider->setValue(0);
                ui->horizontalScrollBar->setEnabled(true);
                ui->horizontalScrollBar->setRange(0, frame_count);
                connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzScrollBarChanged(int)));
                connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));

                //Update slider
                firstHalfStylesheet = "QSlider::groove:horizontal { height: 8px; background: qlineargradient(x1:0, y1:0, x2:1, y2:0,stop:0.000000#25c660,";
                ui->frame_2->setEnabled(true);
                ui->label_18->setEnabled(true);
                ui->label_19->setText("NONE");
                ui->label_19->setStyleSheet("color: rgb(193, 147, 28);");
                ui->label_16->setText("0");
                ui->label_20->setText("0");
                ui->label_22->setText("0");
                bool isGreen = true;
                bool isRed = false;
                for (int i = 0; i < frame_count; i++) {
                    int current = vid_data[2][i] | vid_data[3][i];
                    int currentLum = vid_data[2][i];
                    int currentRed = vid_data[3][i];

                    if (current && isGreen) {
                        isGreen = false;
                        isRed = true;
                        firstHalfStylesheet = firstHalfStylesheet + "stop:" + to_string((i+1)/(double)frame_count - 0.001) + "#25c660,stop:" + to_string((i+1)/(double)frame_count) + "#c10707,";
                    }
                    else if (!current && isRed) {
                        isGreen = true;
                        isRed = false;
                        firstHalfStylesheet = firstHalfStylesheet + "stop:" + to_string((i+1)/(double)frame_count - 0.001) + "#c10707,stop:" + to_string((i+1)/(double)frame_count) + "#25c660,";
                    }

                    if (i == frame_count-1) {
                        if (isGreen) {
                            firstHalfStylesheet = firstHalfStylesheet + "stop:" + to_string((i+1)/(double)frame_count) + "#25c660";
                        }
                        else if (isRed) {
                            firstHalfStylesheet = firstHalfStylesheet + "stop:" + to_string((i+1)/(double)frame_count) + "#c10707";
                        }
                    }

                    if (currentLum) {
                        ui->label_16->setText(QString::number(ui->label_16->text().toInt() + 1));
                    }
                    if (currentRed) {
                        ui->label_20->setText(QString::number(ui->label_20->text().toInt() + 1));
                    }
                    if (currentRed | currentLum) {
                        ui->label_22->setText(QString::number(ui->label_22->text().toInt() + 1));
                    }

                    /*
                    int previous = vid_data[2][i-2] | vid_data[3][i-2];
                    int current = vid_data[2][i-1] | vid_data[3][i-1];
                    double ind = i-1;
                    firstHalfStylesheet = firstHalfStylesheet.substr(0, firstHalfStylesheet.length()-21);
                    if (previous == 0 && current == 1) {
                        warnings.push_back(i-1);
                        ui->label_6->setText(QString::number(ui->label_6->text().toInt() + 1));
                        firstHalfStylesheet = firstHalfStylesheet + "stop:" + to_string(round(1000.0*((ind-1)/vid_data[4][1]+0.001))/1000.0) + "#c10707,stop:" + to_string(ind/vid_data[4][1]) + "#c10707,stop:" + to_string(round(1000.0*(ind/vid_data[4][1]+0.001))/1000.0) + "#6d6b6b,";
                    }
                    else if (previous == 1 && current == 0) {
                        firstHalfStylesheet = firstHalfStylesheet + "stop:" + to_string(round(1000.0*((ind-1)/vid_data[4][1]+0.001))/1000.0) + "#25c660,stop:" + to_string(ind/vid_data[4][1]) + "#25c660,stop:" + to_string(round(1000.0*(ind/vid_data[4][1]+0.001))/1000.0) + "#6d6b6b,";
                    }
                    else if (previous == 0 && current == 0) {
                        firstHalfStylesheet = firstHalfStylesheet + "stop:" + to_string(round(1000.0*ind/vid_data[4][1])/1000.0) + "#25c660,stop:" + to_string(round(1000.0*(ind/vid_data[4][1]+0.001))/1000.0) + "#6d6b6b,";
                    }
                    else {
                        firstHalfStylesheet = firstHalfStylesheet + "stop:" + to_string(round(1000.0*ind/vid_data[4][1])/1000.0) + "#c10707,stop:" + to_string(round(1000.0*(ind/vid_data[4][1]+0.001))/1000.0) + "#6d6b6b,";
                    }
                    */
                }
                ui->slider->setStyleSheet(QString::fromStdString(firstHalfStylesheet) + "); margin: 2px 0; } QSlider::handle:horizontal { background-color: rgba(143,143,143, 200); border: 1px solid rgb(143,143,143); width: 8px; margin: -6px 0; border-radius: 5px; }");
                ui->slider->repaint();

                //Load video
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
                    qDebug() << lastValue;
                });

                media_first_load = true;
                playlist->clear();
                playlist->addMedia(QUrl::fromLocalFile(filename));
                playlist->setCurrentIndex(1);

                player->setPlaylist(playlist);
                player->setVideoOutput(ui->videoWidget_2);
                ui->videoWidget_2->show();
                ui->label_14->setVisible(false);
                vid_file = filename;
                string s = filename.toStdString();
                auto pos = s.rfind('/');
                if (pos != std::string::npos) {
                    s.erase(0,pos+1);
                }
                ui->label->setText(QString::fromStdString(s));

                ui->timeLabel->setStyleSheet("background-color:#EBFFCF; border-style:solid; border-radius:5px; border-color:#406D00; border-width:2px; color: #406D00;");

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

                //Initialize graphs
                ui->customPlot->clearPlottables();
                ui->customPlot->legend->setVisible(true);
                ui->customPlot->addGraph();
                ui->customPlot->addGraph();
                ui->customPlot->addGraph();
                ui->customPlot->addGraph();
                ui->customPlot->graph(0)->setName("Luminance flash diag.");
                ui->customPlot->graph(1)->setName("Red flash diag.");
                ui->customPlot->graph(2)->setName("Luminance flash");
                ui->customPlot->graph(3)->setName("Red flash");

                ui->customPlot->yAxis->setRange(0.0, 1.05);
                ui->customPlot->xAxis->setLabel("Frame Number");
                ui->customPlot->yAxis->setVisible(false);
                section->bottomRight->setCoords(frame_count, 0.0); // the y value is now in axis rect ratios, so 1.1 is "barely below" the bottom axis rect border
                ui->customPlot->replot();

                //Graph 1 style
                QPen solid;
                solid.setStyle(Qt::DotLine);
                solid.setWidthF(1.5);
                solid.setColor(Qt::black);
                ui->customPlot->graph(0)->setPen(solid);
                ui->customPlot->graph(0)->setLineStyle(QCPGraph::lsLine);
                //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

                //Graph 2 style
                QPen solidRed;
                solidRed.setStyle(Qt::DotLine);
                solidRed.setWidthF(1.5);
                solidRed.setColor(Qt::red);
                ui->customPlot->graph(1)->setPen(solidRed);
                ui->customPlot->graph(1)->setLineStyle(QCPGraph::lsLine);
                //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

                //Graph 3 style
                QPen dotted;
                dotted.setStyle(Qt::SolidLine);
                dotted.setWidthF(3);
                dotted.setColor(Qt::white);
                ui->customPlot->graph(2)->setPen(dotted);
                ui->customPlot->graph(2)->setLineStyle(QCPGraph::lsLine);
                //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

                //Graph 4 style
                QPen dottedRed;
                dottedRed.setStyle(Qt::SolidLine);
                dottedRed.setWidthF(3);
                dottedRed.setColor(Qt::red);
                ui->customPlot->graph(3)->setPen(dottedRed);
                ui->customPlot->graph(3)->setLineStyle(QCPGraph::lsLine);
                ui->customPlot->replot();
                for (int i = 0; i < ui->customPlot->graphCount(); i++) {
                    ui->customPlot->graph(i)->data()->clear();
                }

                //Plot data HERE
                ui->customPlot->legend->setVisible(true);
                for (int i = 0; i < diag_y.size(); i++) {
                    qDebug() << diag_x[i] << ": " << diag_y[i];
                }
                ui->customPlot->graph(0)->setData(diag_x, diag_y);
                ui->customPlot->graph(1)->setData(red_diag_x, red_diag_y);
                ui->customPlot->graph(2)->setData(flash_x, flash_y);
                ui->customPlot->graph(3)->setData(red_flash_x, red_flash_y);

                ui->customPlot->graph(0)->addData(frame_count, 0);
                ui->customPlot->graph(1)->addData(frame_count, 0);
                ui->customPlot->graph(2)->addData(frame_count, 0);
                ui->customPlot->graph(3)->addData(frame_count, 0);

                ui->customPlot->replot();

                if (ui->label_16->text() != "0" || ui->label_20->text() != "0" || ui->label_22->text() != "0") {
                    ui->label_19->setText("FAIL");
                    ui->label_19->setStyleSheet("color: #c10707;");
                } else {
                    ui->label_19->setText("PASS");
                    ui->label_19->setStyleSheet("color: #25c660;");
                }

                nFrame = frame_count;
                ui->label_8->setText(QString::number(frame_count));
                ui->label_9->setText(QString::number(fps));
                ui->label_10->setText("...");
                ui->label_11->setText(dimensions);
                ui->label_12->setText(QString::fromStdString(fourcc_str));
                ui->label_13->setText(date);

                player->setVolume(5);
                ui->playButton->setStyleSheet("#playButton { background-image: url(:/images/Res/PlayUp.png); border-image: url(:/images/Res/PlayUp.png); } #playButton:hover { border-image: url(:/images/Res/PlayDownHilite.png); } #playButton:pressed { border-image: url(:/images/Res/PlayPressed.png); }");
                ui->playButton->setEnabled(true);

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
                index = 0;
                // ADDED
                ui->actionShow_all_graphs->setEnabled(true);
                ui->actionHide_All_Graphs->setEnabled(true);
                ui->actionHide_Selected_Graph->setEnabled(true);
                ui->actionLuminance_diag_graph->setEnabled(true);
                ui->actionRed_Flash_Diag_Graph->setEnabled(true);
                ui->actionLuminance_Flash_Graph->setEnabled(true);
                ui->actionRed_Flash_Graph->setEnabled(true);
                ui->actionPlot_Tooltips->setEnabled(true);

                vidLabel->setText("Done");
                file.close();
            }
        }
        catch (int e) {
            //TO-DO: Complete errors
            QString error;
            switch(e)
            {
                case 600:
                    error = "The report file could not be read. Please check if the video at " + vidFile + " exists and try again.";
                    break;
                case 514:
                    error = "The loaded report file is not valid. Please re-analyze the video and try again.";
                    break;
                case 520:
                    error = "The loaded report file is not valid. Please re-analyze the video and try again.";
                    break;
                case 521:
                    error = "The loaded report file is not valid. Please re-analyze the video and try again.";
                    break;
                case 522:
                    error = "The loaded report file is not valid. Please re-analyze the video and try again.";
                    break;
                case 523:
                    error = "The loaded report file is not valid. Please re-analyze the video and try again.";
                    break;
                default:
                    error = "The report file could not be read. Please ensure that the video at " + vidFile + " is valid and is the same video analyzed in this report.";
            }
            vidLabel->setText("Error");
            for (int i = 0; i < ui->customPlot->graphCount(); i++) {
                ui->customPlot->graph(i)->data()->clear();
            }
            QMessageBox mb;
            mb.critical(this, "Error: " + QString::number(e), error);
            mb.setFixedSize(600,400);
        }
    }
}


void mainFrame::on_forwardWarning_clicked()
{
    double conv = (double)player->duration() / ui->label_8->text().toDouble();
    if (warnings.size() > 0)
    {
        if (forwardWarningJustPressed == false)
        {
            ui->slider->setValue(warnings[index]*conv);
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
                ui->slider->setValue(warnings[index]*conv);
            }
            else {
                index++;
                ui->slider->setValue(warnings[index]*conv);
            }
        }
    }
}

void mainFrame::on_backWarning_clicked()
{
    double conv = (double)player->duration() / ui->label_8->text().toDouble();
    if (warnings.size() > 0)
    {
        if (index == 0)
        {
            index = warnings.size() - 1;
            ui->slider->setValue(warnings[index]*conv);
        }
        else {
            index -= 1;
            ui->slider->setValue(warnings[index]*conv);
        }
    }
}

void mainFrame::openProTool()
{
    player->pause();
    if (vid_data.size() < 5) {
        QMessageBox::critical(this, "Error", "A valid report was not loaded.");
        return;
    }
    else if (vid_data[0].size() != vid_data[4][1]) {
        QMessageBox::critical(this, "Error", "A valid report was not loaded.");
        return;
    }
    rkbprotool = new rkbProTool(vid_data, vid_file, this);
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
        skipFrameRightFunc();
    }
    if(event->key() == Qt::Key_Left)
    {
        skipFrameLeftFunc();
    }
    if(event->key() == Qt::Key_L)
    {
        skipRightFunc();
    }
    if(event->key() == Qt::Key_J)
    {
        skipLeftFunc();
    }
    if(event->key() == Qt::Key_Space)
    {
        playPauseFunc();
    }
}

void mainFrame::skipLeftFunc()
{
    if (ui->actionBack_5_Seconds->isEnabled()) {
        qint64 position = player->position();
        position -= 5000;
        player->setPosition(position);
    }
}
void mainFrame::skipRightFunc()
{
    if (ui->actionForward_5_Seconds->isEnabled()) {
        qint64 position = player->position();
        position += 5000;
        player->setPosition(position);
    }
}
void mainFrame::playPauseFunc()
{
    if (player->state() == 1)
    {
        qDebug() << "playing";
        player->pause();
    }
    else if (player->state() == 2 || player->state() == 0)
    {
        qDebug() << "paused ";
        player->play();
    }
}
void mainFrame::skipFrameRightFunc()
{
    if (ui->actionForward_30_Frames->isEnabled()) {
        double f_count = ui->label_8->text().toDouble();
        double position = (double)(player->position()) / double(player->duration());
        position += (1.0/f_count);
        player->setPosition(position*player->duration());
    }
}
void mainFrame::skipFrameLeftFunc()
{
    if (ui->actionBack_30_Frames->isEnabled()) {
        double f_count = ui->label_8->text().toDouble();
        double position = (double)(player->position()) / double(player->duration());
        position -= (1.0/f_count);
        player->setPosition(position*player->duration());
    }
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
        textLabel->setColor(QColor(190, 212, 221, 255));
        textLabel2->setColor(QColor(190, 212, 221, 255));
        section->setBrush(QBrush(QColor(170,192,201,255)));
        cautionLayer->setBrush(QBrush(QColor(160,182,191,255)));
        ui->customPlot->replot();
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
        textLabel->setColor(QColor(216, 201, 184, 255));
        textLabel2->setColor(QColor(216, 201, 184, 255));
        section->setBrush(QBrush(QColor(196,181,164,255)));
        cautionLayer->setBrush(QBrush(QColor(186,171,154,255)));
        ui->customPlot->replot();
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
        textLabel->setColor(QColor(247, 243, 239, 255));
        textLabel2->setColor(QColor(247, 243, 239, 255));
        section->setBrush(QBrush(QColor(227,223,219,255)));
        cautionLayer->setBrush(QBrush(QColor(217,213,209,255)));
        ui->customPlot->replot();
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
        textLabel->setColor(QColor(192, 219, 217, 255));
        textLabel2->setColor(QColor(192, 219, 217, 255));
        section->setBrush(QBrush(QColor(172,199,197,255)));
        cautionLayer->setBrush(QBrush(QColor(162,189,187,255)));
        ui->customPlot->replot();
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
        textLabel->setColor(QColor(140, 144, 145, 255));
        textLabel2->setColor(QColor(140, 144, 145, 255));
        section->setBrush(QBrush(QColor(120,124,125,255)));
        cautionLayer->setBrush(QBrush(QColor(110,114,115,255)));
        ui->customPlot->replot();
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
        textLabel->setColor(QColor(214,214,214,255));
        textLabel2->setColor(QColor(214,214,214,255));
        section->setBrush(QBrush(QColor(194,194,194,255)));
        cautionLayer->setBrush(QBrush(QColor(184,184,184,255)));
        ui->customPlot->replot();
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
        textLabel->setColor(QColor(252,252,252,255));
        textLabel2->setColor(QColor(252,252,252,255));
        section->setBrush(QBrush(QColor(232,232,232,255)));
        cautionLayer->setBrush(QBrush(QColor(222,222,222,255)));
        ui->customPlot->replot();
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

    QPrintDialog *dialog = new QPrintDialog(&printer);
    dialog->setWindowTitle("Print Document");

    if (dialog->exec() != QDialog::Accepted)
        return;

    QPainter painter;
    painter.begin(&printer);

    painter.drawText(100, 100, 500, 500, Qt::AlignHCenter|Qt::AlignTop, text);

    painter.end();
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
            QString filename = vid_file;
            // Check if filename is a valid path
            if (!QFile::exists(filename)) throw 610;

            QString date = ui->label_13->text();
            if (date.isEmpty()) throw 605;

            stream << (quint32)0xA46BF100 << filename << (quint32)vid_data[4][0] << (quint32)vid_data[4][1] << date;
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
                    error = "The report file could not be written. Please try again and check the video directory is valid.";
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
            vidLabel->setText("Error");
        }
    }
    vidLabel->setText("Saved");
    return saved;
}

void mainFrame::no_report_loaded() {
    vid_data.clear();
    saved = true;
    ui->customPlot->clearPlottables();
    ui->customPlot->legend->setVisible(false);
    ui->customPlot->replot();
    warnings.clear();
    reset_slider();
    ui->label_6->setText("0");
    ui->backWarning->setEnabled(false);
    ui->forwardWarning->setEnabled(false);
    ui->actionPrint_Report->setEnabled(false);
    ui->actionSave_Report->setEnabled(false);
    ui->actionRun_Prophylactic_Tool->setEnabled(false);
    // ADDED
    ui->actionShow_all_graphs->setEnabled(false);
    ui->actionHide_All_Graphs->setEnabled(false);
    ui->actionHide_Selected_Graph->setEnabled(false);
    ui->actionLuminance_diag_graph->setEnabled(false);
    ui->actionRed_Flash_Diag_Graph->setEnabled(false);
    ui->actionLuminance_Flash_Graph->setEnabled(false);
    ui->actionRed_Flash_Graph->setEnabled(false);
    ui->actionPlot_Tooltips->setEnabled(false);

    ui->label_13->setText("");
    ui->frame_2->setEnabled(false);
    ui->label_18->setEnabled(false);
    ui->label_19->setText("NONE");
    ui->label_19->setStyleSheet("color: rgb(193, 147, 28);");
    ui->label_16->setText("0");
    ui->label_20->setText("0");
    ui->label_22->setText("0");

    ui->horizontalScrollBar->setEnabled(false);
    ui->horizontalScrollBar->setRange(0, 100);
    ui->horizontalScrollBar->setValue(0);
    disconnect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzScrollBarChanged(int)));
    disconnect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));

    ui->slider->setEnabled(true);
    ui->slider->setStyleSheet("QSlider::groove:horizontal { height: 8px; background: qlineargradient(x1:0, y1:0, x2:1, y2:0,stop:0.000000#6d6b6b,stop:1.0#6d6b6b); margin: 2px 0; } QSlider::handle:horizontal { background-color: #8f8f8f; border: 1px solid #5c5c5c; width: 8px; margin: -6px 0; border-radius: 5px; }");
    ui->horizontalSlider->setEnabled(true);
    ui->horizontalSlider->setStyleSheet("QSlider::groove:horizontal { border: 1px solid #999999; height: 5px; background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0.0 #B1B1B1, stop: 1.0 #c4c4c4); margin: 2px 0; } QSlider::handle:horizontal { background: #8f8f8f; border: 1px solid #5c5c5c; width: 8px; margin: -6px 0; border-radius: 3px; }");
    index = 0;
}

void mainFrame::reset_slider() {
    firstHalfStylesheet = "QSlider::groove:horizontal { height: 8px; background: qlineargradient(x1:0, y1:0, x2:1, y2:0,stop:0.000000#6d6b6b,";
    secondHalfStylesheet = "stop:1.0#6d6b6b); margin: 2px 0; } QSlider::handle:horizontal { background-color: #8f8f8f; border: 1px solid #5c5c5c; width: 8px; margin: -6px 0; border-radius: 5px; }";
    ui->slider->setStyleSheet(QString::fromStdString(firstHalfStylesheet) + secondHalfStylesheet);
}

/*
void mainFrame::on_label_17_mousePressed()
{
    videoWidgetRect = ui->videoWidget_2->geometry();
    plotRect = ui->customPlot->geometry();
    horizontalScrollRect = ui->horizontalScrollBar->geometry();
    label_14Rect = ui->label_14->geometry();
    label_2Rect = ui->label_2->geometry();
    horizontalSliderRect = ui->horizontalSlider->geometry();
    frameRect = ui->frame_2->geometry();
}

// DISABLED
void mainFrame::on_label_17_mouseMoved()
{
    //qDebug() << ui->customPlot->axisRect()->center();
    int diff = ui->label_17->x_new_pos - ui->label_17->x_old_pos;
    int origPos = ui->label_17->orig_x_pos;

    //ui->line_2->setGeometry(259+diff, 0, 7, 383);
    ui->label_17->setGeometry(origPos+diff, 0, 16, 394);
    ui->line_2->setGeometry(origPos+5+diff, 0, 7, 396);

    ui->customPlot->setGeometry(QRect(plotRect.x()+diff, plotRect.y(), plotRect.width()-diff, plotRect.height()));
    ui->line_5->setGeometry((plotRect.width()-diff)/2, 0, 3, 302);
    ui->horizontalScrollBar->setGeometry(QRect(horizontalScrollRect.x()+diff, horizontalScrollRect.y(), horizontalScrollRect.width()-diff, horizontalScrollRect.height()));
    ui->horizontalSlider->setGeometry(QRect(horizontalSliderRect.x()+diff, horizontalSliderRect.y(), horizontalSliderRect.width()-diff, horizontalSliderRect.height()));
    QPainterPath path;
    path.addRoundedRect(ui->customPlot->rect(), 10, 10);
    QRegion mask = QRegion(path.toFillPolygon().toPolygon());
    ui->customPlot->setMask(mask);

    ui->videoWidget_2->setGeometry(QRect(videoWidgetRect.x(), videoWidgetRect.y()-0.75*diff, videoWidgetRect.width()+0.75*diff, videoWidgetRect.height()+0.75*diff));
    ui->label_14->setGeometry(QRect(label_14Rect.x(), label_14Rect.y()-0.75*diff, label_14Rect.width()+0.75*diff, label_14Rect.height()+0.75*diff));
    ui->label_2->setGeometry(QRect(label_2Rect.x(), label_2Rect.y(), label_2Rect.width()-0.5*diff, label_2Rect.height()-0.5*diff));
    ui->frame_2->setGeometry(QRect(frameRect.x(), frameRect.y()-diff, frameRect.width()-0.5*diff, frameRect.height()-0.5*diff));
    this->update();

    //ui->label_2
    //ui->label_4
}
*/
bool mainFrame::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Enter) {
        QWidget *w = qobject_cast<QWidget*>(obj);
        QString b = w->isEnabled() ? "" : " (disabled)";
        descriptionLabel->setText(w->toolTip() + b);
        if (obj != ui->customPlot) QApplication::setOverrideCursor(Qt::ArrowCursor);
    }
    else if (event->type() == QEvent::Leave) {
        descriptionLabel->setText("");
    }
    else if (event->type() == QEvent::ToolTip) {
        return true;
    }

    return QMainWindow::eventFilter(obj, event);
}

void mainFrame::plotTooltip(QMouseEvent *event) {
    vector<int> visibleGraphs;
    for (int i = 0; i < ui->customPlot->graphCount(); i++) {
        if (ui->customPlot->graph(i)->visible()) {
            visibleGraphs.push_back(i);
        }
    }
    if (visibleGraphs.size() > 0) {
        int x = ui->customPlot->xAxis->pixelToCoord(event->pos().x());
        //descriptionLabel->setText(QString::number(x) + ", " + QString::number(y));
        if (x <= nFrame && x >= 0) {
            double y = ui->customPlot->yAxis->pixelToCoord(event->pos().y());
            if (y > 0) placeholderLabel->setText("Cursor: " + QString::number(x) + ", " + QString::number((int)(100*y)/100.0));
            /*
            QSharedPointer<QCPGraphDataContainer> dataMap1 = ui->customPlot->graph(0)->data();
            QSharedPointer<QCPGraphDataContainer> dataMap2 = ui->customPlot->graph(1)->data();
            QSharedPointer<QCPGraphDataContainer> dataMap3 = ui->customPlot->graph(2)->data();
            QSharedPointer<QCPGraphDataContainer> dataMap4 = ui->customPlot->graph(3)->data();
            */

            double min = 2.0;
            int min_index = -1;
            double min_val = -1.0;
            for (auto itr = visibleGraphs.begin(); itr != visibleGraphs.end(); itr++) {
                QCPGraphDataContainer::const_iterator ptr = ui->customPlot->graph(*itr)->data().data()->findBegin(x);
                double val = ptr->value-(double)y;
                val = val > 0 ? val : -1.0*val;
                if (val < min) {
                    min = val;
                    min_index = *itr;
                    min_val = ptr->value;
                }
            }
            if (min < 0.10) {
                descriptionLabel->setText("Graph: " + QString::number(x) + ", " + QString::number(ui->customPlot->graph(min_index)->data().data()->findBegin(x)->value));
                phaseTracer->setGraph(ui->customPlot->graph(min_index));
                phaseTracer->setGraphKey(x);
                phaseTracer->setVisible(true);
                ui->customPlot->replot();
                QApplication::setOverrideCursor(Qt::ArrowCursor);
                QString status;
                if (min_index == 2 || min_index == 3) {
                    status = (int)min_val == 1 ? "FAIL" : "PASS";
                }
                int dur = player->duration()*(double)x/(double)nFrame;
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
                QString time = hours + ":" + minutes +":" + seconds + ":" + milliseconds;

                switch(min_index) {
                    case 0:
                        QToolTip::showText(event->globalPos(), "Luminance flash diag.\nFrame: " + QString::number(x) + "\nTime: " + time + "\nFlash #: " + QString::number(min_val*10));
                    break;
                    case 1:
                        QToolTip::showText(event->globalPos(), "Red flash diag.\nFrame: " + QString::number(x) + "\nTime: " + time + "\nFlash #: " + QString::number(min_val*10));
                    break;
                    case 2:
                        QToolTip::showText(event->globalPos(), "Luminance flash\nFrame: " + QString::number(x) + "\nTime: " + time + "\nStatus: " + status);
                    break;
                    case 3:
                        QToolTip::showText(event->globalPos(), "Red flash.\nFrame: " + QString::number(x) + "\nTime: " + time + "\nStatus: " + status);
                    break;
                }
            }
            else {
                descriptionLabel->setText("");
                phaseTracer->setVisible(false);
                QToolTip::hideText();
                QApplication::setOverrideCursor(Qt::ArrowCursor);
            }

            if (min < 0.05) {
                QApplication::setOverrideCursor(Qt::PointingHandCursor);
            }

        }
        else {
            descriptionLabel->setText("");
            phaseTracer->setVisible(false);
            QToolTip::hideText();
            QApplication::setOverrideCursor(Qt::ArrowCursor);
        }
    }
}

// ADDED
void mainFrame::contextMenuRequest(QPoint pos)
{
  QMenu *menu = new QMenu(this);
  menu->setAttribute(Qt::WA_DeleteOnClose);
   
  if (ui->customPlot->legend->selectTest(pos, false) >= 0) // context menu on legend requested
  {
    menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
    menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignHCenter));
    menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
    menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
    menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
  } else  // general context menu on graphs requested
  {
      if (ui->customPlot->selectedGraphs().size() > 0)
          menu->addAction("Hide selected plot", this, SLOT(hideSelectedGraph()));
          menu->addSeparator();
      if (ui->customPlot->graphCount() > 0) {
          menu->addAction("Hide all plots", this, SLOT(hideAllGraphs()));
          menu->addAction("Show all plots", this, SLOT(showAllGraphs()));
      }
  }
  menu->popup(ui->customPlot->mapToGlobal(pos));
}

void mainFrame::moveLegend()
{
  if (QAction* contextAction = qobject_cast<QAction*>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
  {
    bool ok;
    int dataInt = contextAction->data().toInt(&ok);
    if (ok)
    {
      ui->customPlot->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
      ui->customPlot->replot();
    }
  }
}

void mainFrame::hideSelectedGraph() {
    foreach(QCPGraph *gr, ui->customPlot->selectedGraphs()) {
        gr->setVisible(false);
    }
    phaseTracer->setVisible(false);
    ui->customPlot->replot();
}

void mainFrame::hideAllGraphs() {
    for (int i = 0; i < ui->customPlot->graphCount(); i++) {
        ui->customPlot->graph(i)->setVisible(false);
        qDebug() << ui->customPlot->graph(i)->visible();
    }
    phaseTracer->setVisible(false);
    ui->customPlot->replot();
}

void mainFrame::showAllGraphs() {
    for (int i = 0; i < ui->customPlot->graphCount(); i++) {
        ui->customPlot->graph(i)->setVisible(true);
    }
    ui->customPlot->replot();
}

// ADDED: Graph actions
void mainFrame::on_actionShow_all_graphs_triggered()
{
    showAllGraphs();
}

void mainFrame::on_actionHide_All_Graphs_triggered() {
    hideAllGraphs();
}

void mainFrame::on_actionHide_Selected_Graph_triggered() {
    hideSelectedGraph();
}

void mainFrame::on_actionLuminance_diag_graph_triggered() {
    if (ui->actionLuminance_diag_graph->isChecked() == true) {
        ui->customPlot->graph(0)->setVisible(true);
    } else {
        ui->customPlot->graph(0)->setVisible(false);
    }
    ui->customPlot->replot();
}

void mainFrame::on_actionRed_Flash_Diag_Graph_triggered() {
    if (ui->actionRed_Flash_Diag_Graph->isChecked() == true) {
        ui->customPlot->graph(1)->setVisible(true);
    } else {
        ui->customPlot->graph(1)->setVisible(false);
    }
    ui->customPlot->replot();
}

void mainFrame::on_actionLuminance_Flash_Graph_triggered() {
    if (ui->actionLuminance_Flash_Graph->isChecked() == true) {
        ui->customPlot->graph(2)->setVisible(true);
    } else {
        ui->customPlot->graph(2)->setVisible(false);
    }
    ui->customPlot->replot();
}

void mainFrame::on_actionRed_Flash_Graph_triggered() {
    if (ui->actionRed_Flash_Graph->isChecked() == true) {
        ui->customPlot->graph(3)->setVisible(true);
    } else {
        ui->customPlot->graph(3)->setVisible(false);
    }
    ui->customPlot->replot();
}


// *printing*,
// *UI plans*, *resizing*, *previews*, *screen capture*
// *fix reloading of plots*, *sensitivity*, final testing!

void mainFrame::on_actionPlot_Tooltips_triggered()
{
    if (ui->actionPlot_Tooltips->isChecked() == true) {
        connect(ui->customPlot, &QCustomPlot::mouseMove, this, &mainFrame::plotTooltip);
        phaseTracer->setVisible(true);
    } else {
        disconnect(ui->customPlot, &QCustomPlot::mouseMove, this, &mainFrame::plotTooltip);
        phaseTracer->setVisible(false);
    }
    ui->customPlot->replot();
}

