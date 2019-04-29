#ifndef MAINFRAME_H
#define MAINFRAME_H

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
#include "waitdialog.h"
#include "qcustomplot.h"
#include <QMenuBar>
#include <QStatusBar>
#include "rkbprotool.h"

using namespace std;

namespace Ui {
class mainFrame;
}

class mainFrame : public QMainWindow
{
    Q_OBJECT

public:
    explicit mainFrame(QWidget *parent = nullptr);
    ~mainFrame();
    double x = 1.0;
    qreal rate = x;
    vector<int> warnings;
    QAction *generateReport;
    QAction *skipLeft;
    QAction *skipRight;
    QAction *playPause;
    QAction *skipFrameLeft;
    QAction *skipFrameRight;
    bool forwardWarningJustPressed = false;
    int lastValue = 0;
    bool firstOpen = true;

private slots:
    void on_folderButton_clicked();

    void on_playButton_clicked();

    void on_pauseButton_clicked();

    void on_forwardButton_clicked();

    void on_rewindButton_clicked();

    void on_reportButton_clicked();

    void horzScrollBarChanged(int value);

    void xAxisChanged(QCPRange range);

    void openReport();

    void openProTool();

    void on_forwardWarning_clicked();

    void on_backWarning_clicked();

    vector<int> frameSplit(string filename, string location);

    void skipLeftFunc();

    void skipRightFunc();

    void playPauseFunc();

    void skipFrameRightFunc();

    void skipFrameLeftFunc();

    void on_restartButton_clicked();

    void peatHelp();

    void aboutPEAT();

    void traceHome();

    string execute(const char * cmd);

    string replaceChar(string str, char ch1, char ch2);

    void on_actionLight_Blue_triggered();

    void on_actionBeige_triggered();

    void on_actionPale_triggered();

    void on_actionTurquoise_triggered();

    void on_actionCharcoal_Grey_triggered();

    void on_actionLight_Grey_triggered();

    void on_actionWhite_triggered();

    void on_actionInvert_UI_Colour_triggered();

private:
    Ui::mainFrame *ui;

    void keyPressEvent(QKeyEvent * event);

    QMediaPlayer* player;
    QVideoWidget* vw;
    QProgressBar* bar;
    QSlider* slider;
    QMediaPlaylist *playlist;
    waitDialog *waitdialog;
    QMenuBar *menubar;
    QStatusBar *statusbar;
    int index = 0;
    rkbProTool *rkbprotool;
    QCPItemRect *section;
};


#endif // MAINFRAME_H
