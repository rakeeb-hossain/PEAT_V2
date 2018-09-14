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

using namespace std;
using namespace cv;

void makeFrameGradient(int lowerBound, int upperBound, string gradientFolder, string vidDir);
void lowerContrast(int lowerBound, int upperBound, string gradientFolder, double alpha);
void overlayFilter(int lowerBound, int upperBound, string gradientFolder, double alpha);
void blurFilter(int lowerBound, int upperBound, string gradientFolder, double alpha);
vector< vector<int> > relativeLuminance(Mat colorFrame);
vector<vector<int > > seizureDetection(string path, QString reportName, vector<int> properties);
void rkbcore(string vidDir, QString reportName, vector<int> props);
void proTool(QString dir, QString report, int decision, double alpha);
void writeVideo(QString ndir, QString nreport);


#endif // RKBCORE_H
