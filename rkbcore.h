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
    void proTool(QString dir, QString report, int decision, double alpha);
    vector<vector<int > > rkbcore(string filename);

private:
    void makeFrameGradient(int lowerBound, int upperBound, string gradientFolder, string vidDir);
    void lowerContrast(int lowerBound, int upperBound, string gradientFolder, double alpha);
    void overlayFilter(int lowerBound, int upperBound, string gradientFolder, double alpha);
    void blurFilter(int lowerBound, int upperBound, string gradientFolder, double alpha);
    void writeVideo(QString ndir, QString nreport);
    float relative_luminance(float R, float G, float B);
    float saturation(float R, float G, float B);
    float red_saturation(float R, float G, float B);

signals:
    void updateUI(vector<QVector<double> >,vector<QVector<double> >);

};



#endif // RKBCORE_H
