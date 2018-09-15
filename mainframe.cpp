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
// PEAT App Window

using namespace std;
using namespace cv;

mainFrame::mainFrame(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainFrame)

{
    ui->setupUi(this);
    this->setWindowTitle("UW Trace Center Photosensitive Epilepsy Analysis Tool (PEAT)");
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    this->resize(QSize(870,591));
    this->setFixedSize(QSize(870,591));

    //QVBoxLayout *layout = new QVBoxLayout;
    //this->setLayout(layout);

    menubar = new QMenuBar(this);
    statusbar = new QStatusBar(this);

    QAction *quit = new QAction("&Quit...                  Alt+F4", this);
    QAction *openVideo = new QAction("&Open Video...", this);
    QAction *openReport = new QAction("&Open Report...", this);
    generateReport = new QAction("&Generate Seizure Report...", this);
    generateReport->setEnabled(false);
    QAction *remove = new QAction("Run Prophylactic Tool...", this);

    QAction *peatHelp = new QAction("PEAT Help", this);
    QAction *aboutPEAT = new QAction("About PEAT", this);
    QAction *traceHome = new QAction("Trace Center Homepage", this);

    QMenu *file;
    QMenu *analysis;
    QMenu *controls;
    QMenu *help;

    skipLeft = new QAction("&Forward 5 seconds...       L", this);
    skipRight = new QAction("&Back 5 seconds...             J", this);
    playPause = new QAction("&Play/Pause...                    Space", this);
    skipFrameRight = new QAction("&Forward 30 frames...       Right", this);
    skipFrameLeft = new QAction("&Back 30 frames...             Left", this);
    skipLeft->setEnabled(false);
    skipRight->setEnabled(false);
    playPause->setEnabled(false);
    skipFrameRight->setEnabled(false);
    skipFrameLeft->setEnabled(false);

    file = menubar->addMenu("&File");
    analysis = menubar->addMenu("&Analysis");
    controls = menubar->addMenu("&Controls");
    help = menubar->addMenu("&Help");

    file->addAction(openVideo);
    file->addAction(openReport);
    file->addAction(quit);
    analysis->addAction(remove);
    analysis->addAction(generateReport);
    controls->addAction(skipLeft);
    controls->addAction(skipRight);
    controls->addAction(playPause);
    controls->addAction(skipFrameRight);
    controls->addAction(skipFrameLeft);
    help->addAction(peatHelp);
    help->addAction(aboutPEAT);
    help->addAction(traceHome);
    //layout->setMenuBar(menubar);
    connect(openVideo, &QAction::triggered, this, &mainFrame::on_folderButton_clicked);
    connect(openReport, &QAction::triggered, this, &mainFrame::openReport);
    connect(generateReport, &QAction::triggered, this, &mainFrame::on_reportButton_clicked);
    connect(quit, &QAction::triggered, qApp, QApplication::quit);
    connect(remove, &QAction::triggered, this, &mainFrame::openProTool);

    connect(skipLeft, &QAction::triggered, this, &mainFrame::skipLeftFunc);
    connect(skipRight, &QAction::triggered, this, &mainFrame::skipRightFunc);
    connect(playPause, &QAction::triggered, this,& mainFrame::playPauseFunc);
    connect(skipFrameRight, &QAction::triggered, this, &mainFrame::skipFrameRightFunc);
    connect(skipFrameLeft, &QAction::triggered, this, &mainFrame::skipFrameLeftFunc);

    connect(peatHelp, &QAction::triggered, this, &mainFrame::peatHelp);
    connect(aboutPEAT, &QAction::triggered, this, &mainFrame::aboutPEAT);
    connect(traceHome, &QAction::triggered, this, &mainFrame::traceHome);

    ui->customPlot->yAxis->setRange(0,1);
    ui->customPlot->xAxis->setLabel("Frame Number");
    ui->customPlot->setBackground(QBrush(QColor("#bbc0c9")));
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
    connect(ui->slider, &QSlider::sliderMoved, player, &QMediaPlayer::setPosition);

    connect(ui->slider, &QSlider::valueChanged, ui->horizontalScrollBar, [&](qint64 move) {
        qint64 sliderMax = ui->slider->maximum();
        qint64 scrollMax = ui->horizontalScrollBar->maximum();
        double percent = move/(double)(sliderMax);
        ui->horizontalScrollBar->setValue(percent*(double)scrollMax);
    });
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

void mainFrame::on_folderButton_clicked()
{

    QString filename = QFileDialog::getOpenFileName(this, "Open a Video File", "", "Video File (*.mp4; *.avi; *.flv; *.mpeg)");
    QFileInfo fi(filename);
    QString ext = fi.suffix();
    bool didSelect = false;
    bool isVideo = false;

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
                playlist->clear();
                playlist->addMedia(QUrl::fromLocalFile(filename));
                playlist->setCurrentIndex(1);

                player->setPlaylist(playlist);
                player->setVideoOutput(ui->videoWidget);
                ui->videoWidget->show();
                ui->label_4->setEnabled(false);
                ui->label_4->setText("Press play");
                ui->label->setText(filename);
                ui->timeLabel->setStyleSheet("background-color:rgb(210, 255, 189); border-style:solid; border-color:black; border-width:1px;");
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

                this->generateReport->setEnabled(true);
                this->skipLeft->setEnabled(true);
                this->skipRight->setEnabled(true);
                this->playPause->setEnabled(true);
                this->skipFrameLeft->setEnabled(true);
                this->skipFrameRight->setEnabled(true);

                player->play();
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
vector<int> mainFrame::frameSplit(string filename, string location)
{

    //Extract + Save Frames
    //THIS STUFF USED TO EXIST
    QString QcurrentDir = QDir::currentPath();
    string currentDir = QcurrentDir.toUtf8().constData();

    string locationDir = currentDir + "/" + location + "/frames/";

    string fpsCMD = "RKBVidCore.exe -i " + filename + " -hide_banner 2>&1";

    const char * f = fpsCMD.c_str();
    string output = execute(f);

    string fileCMD = "RKBVidCore.exe -i " + filename + " -r 30 " + locationDir + "%01d.jpg";
    const char * c = fileCMD.c_str();
    system(c);
    int count = 0;
    string fps = output;
    string duration = output;
    cout << "A1" << endl;
    cout << output << endl;

    size_t pos = output.find("Duration: ");
    if (pos != string::npos)
    {
        duration = output.substr(pos);
        duration = duration.substr(10,11);
        cout << "Time: " + duration << endl;
    }
    else {
        QMessageBox::critical(0, "Error", "Error in decoding video. Please try again or try another video.");
        count++;
    }
    cout << "A2" << endl;

    pos = output.find("Video:");

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
        else {
            QMessageBox::critical(0, "Error", "Error in decoding video. Please try again or try another video.");
            count++;
        }
    }
    else{
        QMessageBox::critical(0, "Error", "Error in decoding video. Please try again or try another video.");
        count++;
    }

    auto fpsS = atof(fps.c_str());
    cout << fpsS << endl;

    int d, h, m, s= 0;
    double secs = 0.0;

    if (sscanf(duration.c_str(), "%d:%d:%d:%d", &d, &h, &m, &s) >= 2)
    {
      secs = d * 3600 + h *60 + m + s * 0.01;
    }
    int numOfFrames = floor(secs*fpsS);
    cout << numOfFrames << endl;
    cout << "A4" << endl;

        Mat dimensions = imread(locationDir + "1.jpg");
        int width = dimensions.cols;
        int height = dimensions.rows;

        vector<int> props;
        if (count != 0)
        {
            props = {};
        }
        else {
            props.push_back(numOfFrames);
            props.push_back(round(fpsS));
            props.push_back(width);
            props.push_back(height);
        }

        qDebug() << props[0];

        return props;
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
void mainFrame::on_reportButton_clicked()
{
    QMessageBox messageBox;
    QMessageBox::StandardButton reply;
    reply = messageBox.question(this, "Begin analysis", "Are you sure you would like to begin the analysis? \nThis may take some time.", QMessageBox::Yes|QMessageBox::No);
    messageBox.setFixedSize(600,400);

    if (reply == QMessageBox::Yes)
    {

        QString filename = ui->label->text();
        std::string stringedFile = filename.toLocal8Bit().constData();
        player->pause();

        // Get valid folder name
        bool ok;
        QString folderName = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                                 tr("Set a name for this report"), QLineEdit::Normal,
                                                 QDir::home().dirName(), &ok);
        QMessageBox messageBox2;

        std::string stringedFolder = folderName.toLocal8Bit().constData();

        if (stringedFolder.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_") != std::string::npos)
        {
            ok = false;
        }

        if (QDir(folderName).exists() == true)
        {
            ok = false;
        }

        if (ok && !folderName.isEmpty())
        {

            // Split video into frames
            QDir().mkdir(folderName);
            QDir().mkdir(folderName + "/frames");
            vector<int> properties = frameSplit(stringedFile, stringedFolder);
            QApplication::processEvents();
            qDebug() << "bs0.5";
            qDebug() << properties.size();
            if (properties.size() != 0)
            {
                // Run RKBCore analysis

                //waitdialog = new waitDialog(this);
                //waitdialog->show();
                QApplication::processEvents();
                cout << "properties: " << endl;

                cout << properties[0] << endl;
                cout << properties[1] << endl;
                cout << properties[2] << endl;
                cout << properties[3] << endl;
                qDebug() << "bs0.75";

                rkbcore(stringedFolder + "/frames/", folderName, properties);
                //waitdialog->hide();
                QMessageBox messageBox3;
                messageBox3.information(this, "Success!", "The analysis was completed successfully and your report is available in the project directory.", QMessageBox::NoButton);
                messageBox3.setFixedSize(600,400);

                //Initialize plot and scrolling
                ui->horizontalScrollBar->setEnabled(true);
                ui->horizontalScrollBar->setRange(0, properties[0]);
                ui->customPlot->yAxis->setRange(0.0, 1.0);
                ui->customPlot->xAxis->setLabel("Frame Number");
                connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzScrollBarChanged(int)));
                connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));
                ui->customPlot->replot();

                QFile reportFile(folderName + "/" + folderName + ".txt");
                if (!reportFile.open(QIODevice::ReadOnly))
                    QMessageBox::critical(0, "Error", "Error opening report. Please try generating the report again.");
                else {
                    /*
                    QVector<double> x(101), y(101); // initialize with entries 0..100
                    for (int i=0; i<101; ++i)
                    {
                      x[i] = i/50.0 - 1; // x goes from -1 to 1
                      y[i] = x[i]*x[i]; // let's plot a quadratic function
                    }
                    // create graph and assign data to it:
                    ui->customPlot->addGraph();
                    ui->customPlot->graph(0)->setData(x, y);
                    // give the axes some labels:
                    ui->customPlot->xAxis->setLabel("x");
                    ui->customPlot->yAxis->setLabel("y");
                    // set axes ranges, so we see all data:
                    ui->customPlot->xAxis->setRange(-1, 1);
                    ui->customPlot->yAxis->setRange(0, 1);
                    ui->customPlot->replot();
                    */



                    QTextStream in(&reportFile);
                    QString line;
                    int counter = 0;
                    bool atStart = true;
                    int start = 0;
                    int end = 0;
                    int frameN = 0;
                    int frameDiagValue = 0;
                    //double ynum;
                    int ynum;
                    vector<double> y;
                    vector<double> x;

                    vector<double> yDiag;
                    vector<double> xDiag;
                    int64 frameNum = 0;

                    bool isDiagnostic = false;
                    int n = 0;
                    while (!in.atEnd())
                    {
                        line = in.readLine();
                        ynum = line.toInt();
                        if (line == "Diagnostic")
                        {
                            isDiagnostic = true;
                        }
                        if (counter > 0)
                        {
                            if (isDiagnostic == false)
                            {
                                if(atStart == true)
                                {
                                    start = ynum;
                                    for (int i = end; i <= start; i++)
                                    {

                                        y.push_back(0.0);
                                        x.push_back(static_cast<double>(i));
                                    }
                                    atStart = !atStart;
                                }
                                else {
                                    end = ynum;
                                    for (int i = start; i <= end; i++)
                                    {

                                        y.push_back(1.0);
                                        x.push_back(static_cast<double>(i));
                                    }
                                    atStart = !atStart;
                                }
                            }
                        }
                        else {
                            frameNum = line.toInt();
                        }

                        if(isDiagnostic == true && n > 0)
                        {
                            if(atStart == true)
                            {
                                for(int a = frameN; a < ynum; a++)
                                {
                                    if(ynum - frameN < 30)
                                    {
                                        xDiag.push_back(static_cast<double>(a));
                                        yDiag.push_back(frameDiagValue/10.0);
                                    }

                                    else {
                                        xDiag.push_back(static_cast<double>(a));
                                        yDiag.push_back(0.0);
                                    }
                                }
                                frameN = ynum;
                                xDiag.push_back(frameN/1.0);
                                atStart = !atStart;
                            }
                            else{
                                frameDiagValue = ynum;
                                if (frameDiagValue > 10)
                                {
                                    frameDiagValue = 10;
                                }
                                yDiag.push_back(frameDiagValue/10.0);
                                atStart = !atStart;
                            }
                        }
                        else if(isDiagnostic == true && n == 0)
                        {
                            atStart = true;
                            n++;
                        }
                        counter++;
                    }

                    /*
                    while (!in.atEnd())
                    {
                        line = in.readLine();
                        ynum = line.toDouble();
                        if (counter > 0)
                        {
                            for (int i = 0 + n; i < 30 + n; i++)
                            {

                                y.push_back(ynum);
                                x.push_back(i);


                            }
                            n += 30;
                        }
                        else {
                            frameNum = line.toInt();
                        }
                        counter++;
                    }
                    if(in.atEnd())
                    {
                        for (int a = 1; a <= (frameNum % 30); a++)
                        {
                            y.push_back(0.15);
                            x.push_back(floor(frameNum/30) * 30 + a);
                        }
                    }
                    */
                    QVector<double> xx = QVector<double>::fromStdVector(x);
                    QVector<double> yy = QVector<double>::fromStdVector(y);

                    QVector<double> xxDiag = QVector<double>::fromStdVector(xDiag);
                    QVector<double> yyDiag = QVector<double>::fromStdVector(yDiag);

                    qDebug() << xx.size();

                    ui->customPlot->addGraph();
                    QPen dotted;
                    dotted.setStyle(Qt::SolidLine);
                    dotted.setWidthF(4);
                    dotted.setColor(Qt::white);
                    ui->customPlot->graph(0)->setPen(dotted);
                    ui->customPlot->graph(0)->setLineStyle(QCPGraph::lsLine);
                    //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
                    ui->customPlot->graph(0)->setData(xx, yy);
                    ui->customPlot->xAxis->scaleRange(5.0, ui->customPlot->xAxis->range().center());
                    //ADDED
                    ui->customPlot->addGraph();
                    QPen solid;
                    dotted.setStyle(Qt::DotLine);
                    dotted.setWidthF(2);
                    dotted.setColor(Qt::white);
                    ui->customPlot->graph(1)->setPen(solid);
                    ui->customPlot->graph(1)->setLineStyle(QCPGraph::lsLine);
                    //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
                    ui->customPlot->graph(1)->setData(xxDiag, yyDiag);
                    ui->customPlot->xAxis->scaleRange(5.0, ui->customPlot->xAxis->range().center());

                    ui->customPlot->replot();

                    // Load warnings
                    ui->backWarning->setStyleSheet("#backWarning { background-image: url(:/images/Res/PrevWarningUp.png); border-image: url(:/images/Res/PrevWarningUp.png); } #backWarning:hover { border-image: url(:/images/Res/PrevWarningDownHilite.png); } #backWarning:pressed { border-image: url(:/images/Res/PrevWarningPressed.png); }");
                    ui->forwardWarning->setStyleSheet("#forwardWarning { background-image: url(:/images/Res/NextvWarningUp.png); border-image: url(:/images/Res/NextWarningUp.png); } #forwardWarning:hover { border-image: url(:/images/Res/NextWarningDownHilite.png); } #forwardWarning:pressed { border-image: url(:/images/Res/NextWarningPressed.png); }");
                    ui->backWarning->setEnabled(true);
                    ui->forwardWarning->setEnabled(true);
                    warnings.clear();

                    for (int a = 0; a < y.size(); a += 30)
                    {
                        if (y[a] >= 0.25)
                        {
                            warnings.push_back(a);
                        }
                    }
                    ui->label_6->setText(QString::number(warnings.size()));
                }
            }
            else {
                QMessageBox mb;
                mb.critical(this, "Error", "There was an error in decoding the video stream. Please try and install the appropriate codecs and try again.");
                mb.setFixedSize(600,400);
            }
        }
        else if (folderName.isEmpty() || ok == false)
        {
            messageBox2.information(this, "Failed", "Please input a valid folder name", QMessageBox::NoButton);
            messageBox2.setFixedSize(600,400);
        }
    }
}

void mainFrame::openReport()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open a report file", "", ".txt (*.txt)");
    ui->label->setText(filename);
    if(!filename.isEmpty())
    {

        QFile reportFile(filename);
        if (!reportFile.open(QIODevice::ReadOnly))
            QMessageBox::critical(0, "Error", "Error opening report. Please try generating the report again.");
        else {
            // REMOVED
            /*
            QVector<double> x(101), y(101); // initialize with entries 0..100
            for (int i=0; i<101; ++i)
            {
              x[i] = i/50.0 - 1; // x goes from -1 to 1
              y[i] = x[i]*x[i]; // let's plot a quadratic function
            }
            // create graph and assign data to it:
            ui->customPlot->addGraph();
            ui->customPlot->graph(0)->setData(x, y);
            // give the axes some labels:
            ui->customPlot->xAxis->setLabel("x");
            ui->customPlot->yAxis->setLabel("y");
            // set axes ranges, so we see all data:
            ui->customPlot->xAxis->setRange(-1, 1);
            ui->customPlot->yAxis->setRange(0, 1);
            ui->customPlot->replot();
            */



            QTextStream in(&reportFile);
            QString line;
            int counter = 0;
            bool atStart = true;
            int start = 0;
            int end = 0;
            int frameN = 0;
            int frameDiagValue = 0;
            //double ynum;
            int ynum;
            vector<double> y;
            vector<double> x;

            vector<double> yDiag;
            vector<double> xDiag;
            int64 frameNum = 0;
            double startOfRed = 0.0;
            double endOfRed = 0.0;
            vector<double> redPortions = {};
            //QString stylesheet = "QSlider::groove:horizontal { border: 1px solid #999999; height: 10px; background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0.0 #B1B1B1, stop: 1.0 #c4c4c4); margin: 2px 0; } QSlider::handle:horizontal { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); border: 1px solid #5c5c5c; width: 8px; margin: -6px 0; border-radius: 3px; }";
            QString firstHalfStylesheet = "QSlider::groove:horizontal { border: 1px solid #999999; height: 10px; background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0.0 #25c660, ";
            QString secondHalfStylesheet = "stop: 1.0 #25c660); margin: 2px 0; } QSlider::handle:horizontal { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); border: 1px solid #5c5c5c; width: 8px; margin: -6px 0; border-radius: 3px; }";
            bool isDiagnostic = false;
            int n = 0;

            while (!in.atEnd())
            {
                line = in.readLine();
                ynum = line.toInt();
                if (line == "Diagnostic")
                {
                    isDiagnostic = true;
                }
                if (counter > 0)
                {
                    if (isDiagnostic == false)
                    {
                        if(atStart == true)
                        {
                            start = ynum;
                            for (int i = end; i <= start; i++)
                            {

                                y.push_back(0.0);
                                x.push_back(static_cast<double>(i));
                            }
                            atStart = !atStart;
                        }
                        else {
                            end = ynum;
                            startOfRed = static_cast<double>(start)/frameNum;
                            endOfRed = static_cast<double>(end)/frameNum;
                            redPortions.push_back(startOfRed - 0.0001);
                            redPortions.push_back(startOfRed);
                            redPortions.push_back(endOfRed);
                            redPortions.push_back(endOfRed + 0.0001);
                            for (int i = start; i <= end; i++)
                            {
                                y.push_back(1.0);
                                x.push_back(static_cast<double>(i));
                            }
                            atStart = !atStart;
                        }
                    }
                }
                else {
                    frameNum = line.toInt();
                }

                if(isDiagnostic == true && n > 0)
                {
                    if(atStart == true)
                    {
                        for(int a = frameN; a < ynum; a++)
                        {
                            if(ynum - frameN < 30)
                            {
                                xDiag.push_back(static_cast<double>(a));
                                yDiag.push_back(frameDiagValue/10.0);
                            }

                            else {
                                xDiag.push_back(static_cast<double>(a));
                                yDiag.push_back(0.0);
                            }
                        }
                        frameN = ynum;
                        xDiag.push_back(frameN/1.0);
                        atStart = !atStart;
                    }
                    else{
                        frameDiagValue = ynum;
                        if (frameDiagValue > 10)
                        {
                            frameDiagValue = 10;
                        }
                        yDiag.push_back(frameDiagValue/10.0);
                        atStart = !atStart;
                    }
                }
                else if(isDiagnostic == true && n == 0)
                {
                    atStart = true;
                    n++;
                }
                counter++;
            }
            for (int x = 0; x < redPortions.size(); x += 4)
            {
                firstHalfStylesheet += "stop: " + QString::number(redPortions[x]) + " #25c660, ";
                firstHalfStylesheet += "stop: " + QString::number(redPortions[x+1]) + " #c10707, ";
                firstHalfStylesheet += "stop: " + QString::number(redPortions[x+2]) + " #c10707, ";
                firstHalfStylesheet += "stop: " + QString::number(redPortions[x+3]) + " #25c660, ";
            }
            QString stylesheet = firstHalfStylesheet + secondHalfStylesheet;
            ui->slider->setStyleSheet(stylesheet);
            /*
            while (!in.atEnd())
            {
                line = in.readLine();
                ynum = line.toDouble();
                if (counter > 0)
                {
                    for (int i = 0 + n; i < 30 + n; i++)
                    {

                        y.push_back(ynum);
                        x.push_back(i);


                    }
                    n += 30;
                }
                else {
                    frameNum = line.toInt();
                }
                counter++;
            }
            if(in.atEnd())
            {
                for (int a = 1; a <= (frameNum % 30); a++)
                {
                    y.push_back(0.15);
                    x.push_back(floor(frameNum/30) * 30 + a);
                }
            }
            */
            ui->horizontalScrollBar->setEnabled(true);
            ui->horizontalScrollBar->setRange(0, frameNum);
            ui->customPlot->yAxis->setRange(0.0, 1.0);
            ui->customPlot->xAxis->setLabel("Frame Number");
            connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzScrollBarChanged(int)));
            connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));
            ui->customPlot->replot();

            QVector<double> xx = QVector<double>::fromStdVector(x);
            QVector<double> yy = QVector<double>::fromStdVector(y);

            QVector<double> xxDiag = QVector<double>::fromStdVector(xDiag);
            QVector<double> yyDiag = QVector<double>::fromStdVector(yDiag);

            qDebug() << xx.size();

            ui->customPlot->addGraph();
            QPen dotted;
            dotted.setStyle(Qt::SolidLine);
            dotted.setWidthF(4);
            dotted.setColor(Qt::white);
            ui->customPlot->graph(0)->setPen(dotted);
            ui->customPlot->graph(0)->setLineStyle(QCPGraph::lsLine);
            //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
            ui->customPlot->graph(0)->setData(xx, yy);
            ui->customPlot->xAxis->scaleRange(8.0, ui->customPlot->xAxis->range().center());

            //ADDED
            ui->customPlot->addGraph();
            QPen solid;
            dotted.setStyle(Qt::DotLine);
            dotted.setWidthF(2);
            dotted.setColor(Qt::white);
            ui->customPlot->graph(1)->setPen(solid);
            ui->customPlot->graph(1)->setLineStyle(QCPGraph::lsLine);
            //ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
            ui->customPlot->graph(1)->setData(xxDiag, yyDiag);

            ui->customPlot->replot();
            connect(ui->horizontalSlider, &QSlider::valueChanged, this, [&](int moved)
            {
                //FIX THIS
                int alpha;
                int delta = moved - lastValue;
                if (delta > 0)
                {
                    for (int x = lastValue + 1; x < moved + 1; x++)
                    {
                        alpha *= 1.0 + (x)*0.01;
                    }
                }
                else {
                    for (int x = lastValue - 1; x > moved - 1; x--)
                    {
                        alpha *= 1/(1.0 + (abs(x))*0.01);
                    }
                }
                qDebug() << "Delta: " << moved - lastValue;
                qDebug() << "Moved: " << moved;
                qDebug() << "Last: " << lastValue;

                ui->customPlot->xAxis->scaleRange(alpha, ui->customPlot->xAxis->range().center());
                ui->customPlot->replot();
                lastValue = moved;
            });


            for (int i = 0; i < yyDiag.size(); i++)
            {
                qDebug() << yyDiag[i];
            }
            // Load warnings
            ui->backWarning->setStyleSheet("#backWarning { background-image: url(:/images/Res/PrevWarningUp.png); border-image: url(:/images/Res/PrevWarningUp.png); } #backWarning:hover { border-image: url(:/images/Res/PrevWarningDownHilite.png); } #backWarning:pressed { border-image: url(:/images/Res/PrevWarningPressed.png); }");
            ui->forwardWarning->setStyleSheet("#forwardWarning { background-image: url(:/images/Res/NextvWarningUp.png); border-image: url(:/images/Res/NextWarningUp.png); } #forwardWarning:hover { border-image: url(:/images/Res/NextWarningDownHilite.png); } #forwardWarning:pressed { border-image: url(:/images/Res/NextWarningPressed.png); }");
            ui->backWarning->setEnabled(true);
            ui->forwardWarning->setEnabled(true);
            warnings.clear();

            for (int a = 0; a < y.size(); a += 30)
            {
                if (y[a] >= 0.25)
                {
                    warnings.push_back(a);
                }
            }
            ui->label_6->setText(QString::number(warnings.size()));


        }
    }
}


void mainFrame::on_forwardWarning_clicked()
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

void mainFrame::on_backWarning_clicked()
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

