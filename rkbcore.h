#ifndef RKBCORE_H
#define RKBCORE_H

#include <QMainWindow>
#include "mainframe.h"
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <opencv2/imgproc/imgproc.hpp>
#include <dirent.h>
#include <cmath>
#include <fstream>
#include "waitdialog.h"
#include <QObject>

using namespace std;
using namespace cv;

class rObject : public QObject
{
    Q_OBJECT

public:
    explicit rObject(QObject *parent = Q_NULLPTR);
    ~rObject();
    QString proTool(vector<vector<int> > vidData, QString dir, QString video, int decision, double alpha);

private:
    void makeFrameGradient(int lowerBound, int upperBound, string gradientFolder, string vidDir);
    Mat lowerContrast(Mat frame, double alpha);
    Mat overlayFilter(Mat frame, double alpha);
    Mat blurFilter(Mat frame, double alpha);
    void writeVideo(QString ndir, QString nreport);
    float relative_luminance(float R, float G, float B);
    float saturation(float R, float G, float B);
    float red_saturation(float R, float G, float B);

public slots:
    void rkbcore(string filename);

signals:
    void updateUI(vector<QVector<double > >,vector<QVector<double > >);
    void progressCount(int, int);
    void error();
    void finished();
    void fileReceived();
};



#endif // RKBCORE_H
