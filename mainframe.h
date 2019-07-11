#ifndef MAINFRAME_H
#define MAINFRAME_H

#define QCUSTOMPLOT_USE_OPENGL
#define QT_FATAL_WARNINGS

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
#include <QPrintDialog>
#include <QPrinter>
#include <QThread>

using namespace std;

namespace Ui {
class mainFrame;
}

class mainFrame : public QMainWindow
{
    Q_OBJECT
    QThread workerThread;

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
    vector<vector<int > > vid_data;
    bool saved = true;
    QString vid_file;
    int nFrame;

signals:
    void received();
    void stopped();

public slots:
    void updatePlot(vector<QVector<double > > points_x, vector<QVector<double > > points_y);
    void updateSlider(int moved, int frameNum);

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

    void on_actionPrint_Report_triggered();

    bool on_actionSave_Report_triggered();

    void closeEvent(QCloseEvent *event);

    void no_report_loaded();

    void reset_slider();

    void on_label_17_mouseMoved();

    bool eventFilter(QObject *obj, QEvent *event);

    void plotTooltip(QMouseEvent *event);

    void contextMenuRequest(QPoint pos);

    void hideSelectedGraph();

    void showAllGraphs();

    void hideAllGraphs();
    
    void on_actionShow_all_graphs_triggered();

    void on_actionHide_All_Graphs_triggered();

    void on_actionHide_Selected_Graph_triggered();

    void on_actionLuminance_diag_graph_triggered();

    void on_actionRed_Flash_Diag_Graph_triggered();

    void on_actionLuminance_Flash_Graph_triggered();

    void on_actionRed_Flash_Graph_triggered();

    void moveLegend();

    void on_actionPlot_Tooltips_triggered();

private:
    Ui::mainFrame *ui;

    void keyPressEvent(QKeyEvent * event);
    QLabel *vidLabel;
    QLabel *descriptionLabel;
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
    QCPItemRect *cautionLayer;
    string firstHalfStylesheet = "QSlider::groove:horizontal { height: 8px; background: qlineargradient(x1:0, y1:0, x2:1, y2:0,stop:0.000000#6d6b6b,";
    QString secondHalfStylesheet = "stop:1.0#6d6b6b); margin: 2px 0; } QSlider::handle:horizontal { background-color: rgba(143,143,143, 255); border: 1px solid rgb(143,143,143); width: 8px; margin: -6px 0; border-radius: 5px; }";
    bool media_first_load = true;
    QCPItemTracer *phaseTracer;
    QCPItemText *textLabel;
    QCPItemText *textLabel2;
    QLabel *placeholderLabel;
};


#endif // MAINFRAME_H
