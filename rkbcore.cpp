//
//  main.cpp
//  GradientTest
//
//  Created by Hossain, Rakeeb on 2018-01-02.
//  Copyright © 2018 Hossain, Rakeeb. All rights reserved.
//
#include "mainframe.h"
#include "ui_mainframe.h"
#include <QDialog>
#include <QProgressBar>
#include <iostream>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <opencv2/imgproc/imgproc.hpp>
#include <dirent.h>
#include <cmath>
#include <fstream>
#include <QFile>
#include <QProgressDialog>
#include <algorithm>
#include <set>

using namespace std;
using namespace cv;

enum SeverityScale
{
    MODERATETOLOW = 6,
    MODERATE,
    MODERATETOHIGH,
    HIGH,
    HIGHER,
    HIGHEST,
};


void makeFrameGradient(int lowerBound, int upperBound, string gradientFolder, string vidDir) {
    string lowerBoundGradientPath = gradientFolder + to_string(lowerBound) + ".jpg";
    string nextGradientPath = gradientFolder + to_string(lowerBound + 1) + ".jpg";
    string upperBoundGradientPath = gradientFolder + to_string(upperBound) + ".jpg";

    Mat frame1 = imread(lowerBoundGradientPath);
    Mat frame2 = imread(nextGradientPath);
    Mat frameFinal = imread(upperBoundGradientPath);
    Mat newFrame = frame1;
    bool seizureBool = true;
    bool continueBool = false;
    int thresholdInt = 135;
    for (int count = 1; count <= 8; count++)
    {
        string pointPath = vidDir + to_string(lowerBound + count) + ".jpg";
        for(int x = 0; x < frame1.rows; x++)
        {
            for(int y = 0; y < frame1.cols; y++)
            {
                Vec3b color1 = frame1.at<Vec3b>(x,y);
                Vec3b color2 = frame2.at<Vec3b>(x,y);
                Vec3b newColorFrame = newFrame.at<Vec3b>(x,y);

                if (color1[0] - color2[0] > thresholdInt)
                {
                    newColorFrame[0] = color1[0] - 10;
                }
                else if (color2[0] - color1[0] > thresholdInt)
                {
                    newColorFrame[0] = color1[0] + thresholdInt;
                }
                else if (color1[1] - color2[1] > thresholdInt)
                {
                    newColorFrame[1] = color1[1] - 10;

                }
                else if (color2[1] - color1[1] > thresholdInt)
                {
                    newColorFrame[1] = color1[1] + 10;

                }
                else if (color1[2] - color2[2] > thresholdInt)
                {
                    newColorFrame[2] = color1[2] - 10;
                }
                else if (color2[2] - color1[2] > thresholdInt)
                {
                    newColorFrame[2] = color1[2] + 10;
                }
                else
                {
                    continueBool = true;
                }
                newFrame.at<Vec3b>(x,y) = newColorFrame;
            }
        }
        if (continueBool == true)
        {
            frame2 = imread(pointPath);
        }

        frame1 = newFrame;
        imwrite(pointPath, newFrame);

        if (count == 8 && seizureBool == true)
        {
            lowerBound = lowerBound + count;
            lowerBoundGradientPath = gradientFolder + to_string(lowerBound) + ".jpg";
            nextGradientPath = gradientFolder + to_string(lowerBound + 1) + ".jpg";
            frame1 = imread(lowerBoundGradientPath);
            frame2 = imread(nextGradientPath);
            count = 0;
        }
        if (lowerBound + count == upperBound)
        {
            break;
        }
    }
}

void lowerContrast(int lowerBound, int upperBound, string gradientFolder, double alpha)
{

    for (int count = 0; count <= (upperBound - lowerBound); count++)
    {
        string lowerBoundGradientPath = gradientFolder + to_string(lowerBound + count) + ".jpg";
        Mat frame1 = imread(lowerBoundGradientPath);
        Mat updatedFrame = Mat::zeros( frame1.size(), frame1.type() );

        for(int x = 0; x < frame1.rows; x++)
        {
            for(int y = 0; y < frame1.cols; y++)
            {
                for(int c = 0; c < 3; c++)
                {
                    updatedFrame.at<Vec3b>(x,y)[c] = saturate_cast<uchar>((1.0 - alpha)*(frame1.at<Vec3b>(x,y)[c]) + 0);
                }
            }
        }
        imwrite(lowerBoundGradientPath, updatedFrame);
    }
}

void overlayFilter(int lowerBound, int upperBound, string gradientFolder, double alpha)
{
    for (int count = 0; count <= (upperBound - lowerBound); count++)
    {
        string lowerBoundGradientPath = gradientFolder + to_string(lowerBound + count) + ".jpg";
        Mat frame1 = imread(lowerBoundGradientPath);
        Mat overlay;
        frame1.copyTo(overlay);
        cv::rectangle(overlay, cv::Rect(0, 0, frame1.cols, frame1.rows), cv::Scalar(50, 50, 50), -1);

        cv::addWeighted(overlay, alpha, frame1, 1.0 - alpha, 0.0, frame1);

        imwrite(lowerBoundGradientPath, frame1);
    }
}

void blurFilter(int lowerBound, int upperBound, string gradientFolder, double alpha)
{
    int kernal = alpha*225;
    if (kernal % 2 == 0)
    {
        kernal++;
    }
    for (int count = 0; count <= (upperBound - lowerBound); count++)
    {
        string lowerBoundGradientPath = gradientFolder + to_string(lowerBound + count) + ".jpg";
        Mat frame1 = imread(lowerBoundGradientPath);
        if (alpha != 0.0)
        {
            blur(frame1, frame1, Size(kernal, kernal));
            imwrite(lowerBoundGradientPath, frame1);
        }
    }
}

void writeVideo(QString ndir, QString nreport)
{
    std::string dir = ndir.toUtf8().constData();
    std::string oldDir = ndir.toUtf8().constData();
    dir = dir + "/frames/";
    QFile reportFile(nreport);
    int fileNum;

    if (!reportFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(0, "Error", "Could not open the report");
    }
    else {
        QTextStream in(&reportFile);
        QString line;
        int counter = 0;

        bool ok;

        while (!in.atEnd() && counter == 0)
        {
            line = in.readLine();
            fileNum = line.toInt();
            counter++;
        }
    }
    string gradientFolder = dir;

    string lowerBoundGradientPath = gradientFolder + "1.jpg";
    Mat frame1 = imread(lowerBoundGradientPath);
    int frame_width = frame1.cols;
    int frame_height = frame1.rows;
    string videoName = oldDir + "/video.mp4";
    VideoWriter video(videoName, CV_FOURCC('X','V','I','D'), 30, Size(frame_width,frame_height));

    QCoreApplication::processEvents();
    QProgressDialog progress("Writing video...", "Abort", 0, fileNum);
    progress.setWindowModality(Qt::WindowModal);
    progress.setFixedSize(500, 100);
    progress.show();
    QCoreApplication::processEvents();

    for (int count = 0; count <= fileNum; count++)
    {
        lowerBoundGradientPath = gradientFolder + to_string(count) + ".jpg";
        frame1 = imread(lowerBoundGradientPath);

        video.write(frame1);

        progress.setValue(count);
        QCoreApplication::processEvents();
    }
    progress.close();
    video.release();
}
vector< vector<int> > relativeLuminance(Mat colorFrame)
{
    vector<vector<int> > brightnessMatrix;
    brightnessMatrix.resize(colorFrame.rows);
    for (int x = 0; colorFrame.rows > x; x++)
    {
        brightnessMatrix[x].resize(colorFrame.cols);

        for (int y = 0; colorFrame.cols > y; y++)
        {
            int pixelValue = (int)colorFrame.at<uchar>(x,y);
            brightnessMatrix[x][y] = pixelValue;
        }
    }
    return brightnessMatrix;
}
vector< vector< vector<double> > > saturationValues(Mat colorFrame)
{
    vector<vector<vector<double> > > brightnessMatrix;
    brightnessMatrix.resize(floor(colorFrame.rows/2));
    for (int x = 0; floor(colorFrame.rows/2) > x; x++)
    {
        brightnessMatrix[x].resize(floor(colorFrame.cols/2));

        for (int y = 0; floor(colorFrame.cols/2) > y; y++)
        {
            brightnessMatrix[x][y].resize(3);
            Vec3b bgrValues = colorFrame.at<Vec3b>(x,y);
            double sBlue = bgrValues[0]/255.0;
            double sGreen = bgrValues[1]/255.0;
            double sRed = bgrValues[2]/255.0;
            double blue;
            double green;
            double red;
            if (sBlue <= 0.0398)
            {
                blue = sBlue/12.92;
            }
            else {
                blue = pow(((sBlue + 0.055)/1.055), 2.4);
            }
            if (sGreen <= 0.0398)
            {
                green = sGreen/12.92;
            }
            else {
                green = pow(((sGreen + 0.055)/1.055), 2.4);
            }
            if (sRed <= 0.03)
            {
                red = sRed/12.92;
            }
            else {
                red = pow(((sRed + 0.055)/1.055), 2.4);
            }

            brightnessMatrix[x][y][0] = blue;
            brightnessMatrix[x][y][1] = green;
            brightnessMatrix[x][y][2] = red;
        }
    }
    return brightnessMatrix;
}

vector<vector<int > > seizureDetection(string path, QString reportName, vector<int> properties)
{
    QString Ufilename = reportName + "/USER" + reportName + ".txt";
    QString filename = reportName + "/" + reportName + ".txt";

    QFile file(Ufilename);
    QFile aFile(filename);

    int numOfFrames = properties[0];
    int n = 0;
    int seizureCount = 0;
    int redSeizureCount = 0;
    int arb = 0;
    int aarb = 0;
    int rate = round(properties[1]);

    QString newPath = QString::fromStdString(path);
    QDir dir(newPath);
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    numOfFrames = dir.count();

    vector<vector<int > > seizureBoundaries;
    seizureBoundaries.push_back({0,0});
    vector<vector<int > > lumDiagFrames;
    vector<vector<int > > redDiagFrames;
    vector<vector<int > > redSeizureBoundaries;
    redSeizureBoundaries.push_back({0,0});
    vector<double> index = {0};
    vector<double> redIndex = {0};
    int num = 0;
    QApplication::processEvents();
    bool updatingBounds = false;
    QProgressDialog progress("Analyzing frames...", "Abort", 0, floor(numOfFrames/30));
    progress.setWindowModality(Qt::WindowModal);
    progress.setFixedSize(500, 100);
    progress.show();
    int diagInt = 0;
    int redDiagInt = 0;
    bool wasSeizure = false;
    bool wasRedSeizure = false;
    bool adjustN = false;

    int count = 0;
    QApplication::processEvents();

    QString labelText;
    while(n + 30 <= numOfFrames)
    {
        QApplication::processEvents();
        labelText = "Analyzing frames... " + QString::number(n) + "/" + QString::number(numOfFrames);
        progress.setLabelText(labelText);
        progress.setValue(round(n/30));
        qDebug() << "Level: " << n << endl;

        string path1 = path + to_string(n+1) + ".jpg";
        string path2 = path + to_string(n+2) + ".jpg";
        string path3 = path + to_string(n+3) + ".jpg";
        string path4 = path + to_string(n+4) + ".jpg";
        string path5 = path + to_string(n+5) + ".jpg";
        string path6 = path + to_string(n+6) + ".jpg";
        string path7 = path + to_string(n+7) + ".jpg";
        string path8 = path + to_string(n+8) + ".jpg";
        string path9 = path + to_string(n+9) + ".jpg";
        string path10 = path + to_string(n+10) + ".jpg";
        string path11 = path + to_string(n+11) + ".jpg";
        string path12 = path + to_string(n+12) + ".jpg";
        string path13 = path + to_string(n+13) + ".jpg";
        string path14 = path + to_string(n+14) + ".jpg";
        string path15 = path + to_string(n+15) + ".jpg";
        string path16 = path + to_string(n+16) + ".jpg";
        string path17 = path + to_string(n+17) + ".jpg";
        string path18 = path + to_string(n+18) + ".jpg";
        string path19 = path + to_string(n+19) + ".jpg";
        string path20 = path + to_string(n+20) + ".jpg";
        string path21 = path + to_string(n+21) + ".jpg";
        string path22 = path + to_string(n+22) + ".jpg";
        string path23 = path + to_string(n+23) + ".jpg";
        string path24 = path + to_string(n+24) + ".jpg";
        string path25 = path + to_string(n+25) + ".jpg";
        string path26 = path + to_string(n+26) + ".jpg";
        string path27 = path + to_string(n+27) + ".jpg";
        string path28 = path + to_string(n+28) + ".jpg";
        string path29 = path + to_string(n+29) + ".jpg";
        string path30 = path + to_string(n+30) + ".jpg";
        QApplication::processEvents();

        Mat frame1 = imread(path1);
        Mat frame2 = imread(path2);
        Mat frame3 = imread(path3);
        Mat frame4 = imread(path4);
        Mat frame5 = imread(path5);
        Mat frame6 = imread(path6);
        Mat frame7 = imread(path7);
        Mat frame8 = imread(path8);
        Mat frame9 = imread(path9);
        Mat frame10 = imread(path10);
        Mat frame11 = imread(path11);
        Mat frame12 = imread(path12);
        Mat frame13 = imread(path13);
        Mat frame14 = imread(path14);
        Mat frame15 = imread(path15);
        Mat frame16 = imread(path16);
        Mat frame17 = imread(path17);
        Mat frame18 = imread(path18);
        Mat frame19 = imread(path19);
        Mat frame20 = imread(path20);
        Mat frame21 = imread(path21);
        Mat frame22 = imread(path22);
        Mat frame23 = imread(path23);
        Mat frame24 = imread(path24);
        Mat frame25 = imread(path25);
        Mat frame26 = imread(path26);
        Mat frame27 = imread(path27);
        Mat frame28 = imread(path28);
        Mat frame29 = imread(path29);
        Mat frame30 = imread(path30);

        Mat frame1R = imread(path1);
        Mat frame2R = imread(path2);
        Mat frame3R = imread(path3);
        Mat frame4R = imread(path4);
        Mat frame5R = imread(path5);
        Mat frame6R = imread(path6);
        Mat frame7R = imread(path7);
        Mat frame8R = imread(path8);
        Mat frame9R = imread(path9);
        Mat frame10R = imread(path10);
        Mat frame11R = imread(path11);
        Mat frame12R = imread(path12);
        Mat frame13R = imread(path13);
        Mat frame14R = imread(path14);
        Mat frame15R = imread(path15);
        Mat frame16R = imread(path16);
        Mat frame17R = imread(path17);
        Mat frame18R = imread(path18);
        Mat frame19R = imread(path19);
        Mat frame20R = imread(path20);
        Mat frame21R = imread(path21);
        Mat frame22R = imread(path22);
        Mat frame23R = imread(path23);
        Mat frame24R = imread(path24);
        Mat frame25R = imread(path25);
        Mat frame26R = imread(path26);
        Mat frame27R = imread(path27);
        Mat frame28R = imread(path28);
        Mat frame29R = imread(path29);
        Mat frame30R = imread(path30);

        cvtColor(frame1, frame1, CV_BGR2GRAY);
        cvtColor(frame2, frame2, CV_BGR2GRAY);
        cvtColor(frame3, frame3, CV_BGR2GRAY);
        cvtColor(frame4, frame4, CV_BGR2GRAY);
        cvtColor(frame5, frame5, CV_BGR2GRAY);
        cvtColor(frame6, frame6, CV_BGR2GRAY);
        cvtColor(frame7, frame7, CV_BGR2GRAY);
        cvtColor(frame8, frame8, CV_BGR2GRAY);
        cvtColor(frame9, frame9, CV_BGR2GRAY);
        cvtColor(frame10, frame10, CV_BGR2GRAY);
        cvtColor(frame11, frame11, CV_BGR2GRAY);
        cvtColor(frame12, frame12, CV_BGR2GRAY);
        cvtColor(frame13, frame13, CV_BGR2GRAY);
        cvtColor(frame14, frame14, CV_BGR2GRAY);
        cvtColor(frame15, frame15, CV_BGR2GRAY);
        cvtColor(frame16, frame16, CV_BGR2GRAY);
        cvtColor(frame17, frame17, CV_BGR2GRAY);
        cvtColor(frame18, frame18, CV_BGR2GRAY);
        cvtColor(frame19, frame19, CV_BGR2GRAY);
        cvtColor(frame20, frame20, CV_BGR2GRAY);
        cvtColor(frame21, frame21, CV_BGR2GRAY);
        cvtColor(frame22, frame22, CV_BGR2GRAY);
        cvtColor(frame23, frame23, CV_BGR2GRAY);
        cvtColor(frame24, frame24, CV_BGR2GRAY);
        cvtColor(frame25, frame25, CV_BGR2GRAY);
        cvtColor(frame26, frame26, CV_BGR2GRAY);
        cvtColor(frame27, frame27, CV_BGR2GRAY);
        cvtColor(frame28, frame28, CV_BGR2GRAY);
        cvtColor(frame29, frame29, CV_BGR2GRAY);
        cvtColor(frame30, frame30, CV_BGR2GRAY);

        vector < vector<int> > frame1Luminance = relativeLuminance(frame1);
        vector < vector<int> > frame2Luminance = relativeLuminance(frame2);
        vector < vector<int> > frame3Luminance = relativeLuminance(frame3);
        vector < vector<int> > frame4Luminance = relativeLuminance(frame4);
        vector < vector<int> > frame5Luminance = relativeLuminance(frame5);
        vector < vector<int> > frame6Luminance = relativeLuminance(frame6);
        vector < vector<int> > frame7Luminance = relativeLuminance(frame7);
        vector < vector<int> > frame8Luminance = relativeLuminance(frame8);
        vector < vector<int> > frame9Luminance = relativeLuminance(frame9);
        vector < vector<int> > frame10Luminance = relativeLuminance(frame10);
        vector < vector<int> > frame11Luminance = relativeLuminance(frame11);
        vector < vector<int> > frame12Luminance = relativeLuminance(frame12);
        vector < vector<int> > frame13Luminance = relativeLuminance(frame13);
        vector < vector<int> > frame14Luminance = relativeLuminance(frame14);
        vector < vector<int> > frame15Luminance = relativeLuminance(frame15);
        vector < vector<int> > frame16Luminance = relativeLuminance(frame16);
        vector < vector<int> > frame17Luminance = relativeLuminance(frame17);
        vector < vector<int> > frame18Luminance = relativeLuminance(frame18);
        vector < vector<int> > frame19Luminance = relativeLuminance(frame19);
        vector < vector<int> > frame20Luminance = relativeLuminance(frame20);
        vector < vector<int> > frame21Luminance = relativeLuminance(frame21);
        vector < vector<int> > frame22Luminance = relativeLuminance(frame22);
        vector < vector<int> > frame23Luminance = relativeLuminance(frame23);
        vector < vector<int> > frame24Luminance = relativeLuminance(frame24);
        vector < vector<int> > frame25Luminance = relativeLuminance(frame25);
        vector < vector<int> > frame26Luminance = relativeLuminance(frame26);
        vector < vector<int> > frame27Luminance = relativeLuminance(frame27);
        vector < vector<int> > frame28Luminance = relativeLuminance(frame28);
        vector < vector<int> > frame29Luminance = relativeLuminance(frame29);
        vector < vector<int> > frame30Luminance = relativeLuminance(frame30);
        vector<vector<vector<int> > > luminanceMatrix {frame1Luminance, frame2Luminance, frame3Luminance, frame4Luminance, frame5Luminance, frame6Luminance, frame7Luminance, frame8Luminance, frame9Luminance, frame10Luminance, frame11Luminance, frame12Luminance, frame13Luminance, frame14Luminance, frame15Luminance, frame16Luminance, frame17Luminance, frame18Luminance, frame19Luminance, frame20Luminance, frame21Luminance, frame22Luminance, frame23Luminance, frame24Luminance, frame25Luminance, frame26Luminance, frame27Luminance, frame28Luminance, frame29Luminance, frame30Luminance};

        vector < vector< vector<double> > > frame1RLuminance = saturationValues(frame1R);
        vector < vector< vector<double> > > frame2RLuminance = saturationValues(frame2R);
        vector < vector< vector<double> > > frame3RLuminance = saturationValues(frame3R);
        vector < vector< vector<double> > > frame4RLuminance = saturationValues(frame4R);
        vector < vector< vector<double> > > frame5RLuminance = saturationValues(frame5R);
        vector < vector< vector<double> > > frame6RLuminance = saturationValues(frame6R);
        vector < vector< vector<double> > > frame7RLuminance = saturationValues(frame7R);
        vector < vector< vector<double> > > frame8RLuminance = saturationValues(frame8R);
        vector < vector< vector<double> > > frame9RLuminance = saturationValues(frame9R);
        vector < vector< vector<double> > > frame10RLuminance = saturationValues(frame10R);
        vector < vector< vector<double> > > frame11RLuminance = saturationValues(frame11R);
        vector < vector< vector<double> > > frame12RLuminance = saturationValues(frame12R);
        vector < vector< vector<double> > > frame13RLuminance = saturationValues(frame13R);
        vector < vector< vector<double> > > frame14RLuminance = saturationValues(frame14R);
        vector < vector< vector<double> > > frame15RLuminance = saturationValues(frame15R);
        vector < vector< vector<double> > > frame16RLuminance = saturationValues(frame16R);
        vector < vector< vector<double> > > frame17RLuminance = saturationValues(frame17R);
        vector < vector< vector<double> > > frame18RLuminance = saturationValues(frame18R);
        vector < vector< vector<double> > > frame19RLuminance = saturationValues(frame19R);
        vector < vector< vector<double> > > frame20RLuminance = saturationValues(frame20R);
        vector < vector< vector<double> > > frame21RLuminance = saturationValues(frame21R);
        vector < vector< vector<double> > > frame22RLuminance = saturationValues(frame22R);
        vector < vector< vector<double> > > frame23RLuminance = saturationValues(frame23R);
        vector < vector< vector<double> > > frame24RLuminance = saturationValues(frame24R);
        vector < vector< vector<double> > > frame25RLuminance = saturationValues(frame25R);
        vector < vector< vector<double> > > frame26RLuminance = saturationValues(frame26R);
        vector < vector< vector<double> > > frame27RLuminance = saturationValues(frame27R);
        vector < vector< vector<double> > > frame28RLuminance = saturationValues(frame28R);
        vector < vector< vector<double> > > frame29RLuminance = saturationValues(frame29R);
        vector < vector< vector<double> > > frame30RLuminance = saturationValues(frame30R);
        vector<vector<vector<vector<double > > > > redMatrix {frame1RLuminance, frame2RLuminance, frame3RLuminance, frame4RLuminance, frame5RLuminance, frame6RLuminance, frame7RLuminance, frame8RLuminance, frame9RLuminance, frame10RLuminance, frame11RLuminance, frame12RLuminance, frame13RLuminance, frame14RLuminance, frame15RLuminance, frame16RLuminance, frame17RLuminance, frame18RLuminance, frame19RLuminance, frame20RLuminance, frame21RLuminance, frame22RLuminance, frame23RLuminance, frame24RLuminance, frame25RLuminance, frame26RLuminance, frame27RLuminance, frame28RLuminance, frame29RLuminance, frame30RLuminance};

        vector<double> avgLumMatrix(30);
        vector<double> avgRedMatrix(30);
        double avgLum = 0.0;
        double avgSaturation = 0.0;
        double avgBlue = 0.0;
        double avgGreen = 0.0;
        double avgRed = 0.0;
        int a = 256;
        int b = 341;

        int currentLumArrSize = lumDiagFrames.size();

        for(int j = 0; j < frame1.rows - 256; j += 30)
        {
            for(int k = 0; k < frame1.cols - 341; k += 30)
            {
                diagInt = 0;
                redDiagInt = 0;
                int currentLumSizeSecond = lumDiagFrames.size();
                wasSeizure = false;
                avgLum = 0.0;
                avgSaturation = 0.0;
                avgBlue = 0.0;
                avgGreen = 0.0;
                avgRed = 0.0;
                avgLumMatrix.empty();
                avgRedMatrix.empty();
                index.empty();
                redIndex.empty();

                for(int i = 0; i < 30; i++)
                {
                    for (int x = 0; x < a; x++)
                    {
                        for (int y = 0; y < b; y++)
                        {
                            avgLum += luminanceMatrix[i][x + j][y + k];
                        }
                    }
                    for (int c = 0; c < floor(frame1.rows/4); c++)
                    {
                        for (int d = 0; d < floor(frame1.cols/4); d++)
                        {
                            avgSaturation += (redMatrix[i][c][d][2])/(redMatrix[i][c][d][0] + redMatrix[i][c][d][1] + redMatrix[i][c][d][2]);
                            avgBlue += redMatrix[i][c][d][0];
                            avgGreen += redMatrix[i][c][d][1];
                            avgRed += redMatrix[i][c][d][2];
                        }
                    }
                    avgLum = avgLum/(static_cast<double>(a*b));
                    avgLumMatrix[i] = avgLum;
                    avgLum = 0;

                    avgSaturation = avgSaturation/(static_cast<double>(a*b*4));
                    avgBlue = avgBlue/(static_cast<double>(a*b*4));
                    avgGreen = avgGreen/(static_cast<double>(a*b*4));
                    avgRed = avgRed/(static_cast<double>(a*b*4));
                    qDebug() << avgSaturation;

                    if (avgSaturation >= 0.02)
                    {
                        qDebug() << n << " : SATURATION";
                        avgRedMatrix[i] = (avgRed - avgGreen - avgBlue)*320.0;
                    }
                    else{
                        avgRedMatrix[i] = 0.0;
                    }
                    avgSaturation = 0;
                }

                vector<double> amp = avgLumMatrix;
                vector<double> peaks;
                index = {0};
                // vector<double>::iterator it;
                const int NOISE = -1;               // Level up to and including which peaks will be excluded
                int wideStart = -1;                 // The start of any current wide peak
                int grad = -1;                      // Sign of gradient (almost)
                //    =  1 for increasing
                //    =  0 for level AND PREVIOUSLY INCREASING (so potential wide peak)
                //    = -1 for decreasing OR level, but previously decreasing
                // A sharp peak is identified by grad=1 -> grad=-1
                // A wide  peak is identified by grad=0 -> grad=-1

                for ( int i = 0; i < amp.size(); i++ )
                {
                    if ( amp[i+1] < amp[i])         // Only possibility of a peak
                    {
                        if ( grad == 1 && amp[i] > NOISE )
                        {
                            peaks.push_back(amp[i]);
                            index.push_back(i);
                        }
                        else if ( grad == 0 && amp[i] > NOISE )
                        {
                        }
                        grad = -1;
                    }
                    else if ( amp[i+1] == amp[i] )   // Check for start of a wide peak
                    {
                        if ( grad == 1 )
                        {
                            wideStart = i;
                            grad = 0;
                        }
                    }
                    else
                    {
                        grad = 1;
                    }
                }

                vector<double> peaksRed;
                redIndex = {0};
                // vector<double>::iterator it;
                grad = -1;                      // Sign of gradient (almost)

                for ( int i = 0; i < avgRedMatrix.size(); i++ )
                {
                    if ( avgRedMatrix[i+1] < avgRedMatrix[i])         // Only possibility of a peak
                    {
                        if ( grad == 1 && avgRedMatrix[i] > NOISE )
                        {
                            peaksRed.push_back(avgRedMatrix[i]);
                            redIndex.push_back(i);
                        }
                        else if ( grad == 0 && avgRedMatrix[i] > NOISE )
                        {
                        }
                        grad = -1;
                    }
                    else if ( avgRedMatrix[i+1] == avgRedMatrix[i] )   // Check for start of a wide peak
                    {
                        if ( grad == 1 )
                        {
                            wideStart = i;
                            grad = 0;
                        }
                    }
                    else
                    {
                        grad = 1;
                    }
                }



                vector<int> peakIndex = {};

                for (num = 0; num < peaks.size(); num++)
                {
                    vector<double> betweenPeaksMin;
                    vector<double> betweenPeaksMax;
                    double min = 0.0, otherMin = 0.0, deviation1 = 0.0, deviation2 = 0.0;

                    for (int abc = 0; abc < (index[num + 1] - index[num] - 1); abc++)
                    {
                        betweenPeaksMin.push_back(amp[abc + index[num] + 1]);
                        min = *min_element(betweenPeaksMin.begin(), betweenPeaksMin.end());
                    }
                    if (num + 2 < index.size())
                    {
                        for (int abc = 0; abc < (index[num + 2] - index[num + 1] - 1); abc++)
                        {
                            betweenPeaksMax.push_back(amp[abc + index[num + 1] + 1]);
                            otherMin = *min_element(betweenPeaksMax.begin(), betweenPeaksMax.end());
                        }
                    }
                    else
                    {
                        for (int abc = 0; abc < (30 - index[num + 1] - 1); abc++)
                        {
                            betweenPeaksMax.push_back(amp[abc + index[num + 1] + 1]);
                            otherMin = *min_element(betweenPeaksMax.begin(), betweenPeaksMax.end());
                        }
                    }

                    if ((peaks[num] - min)/(min) > 0.07 && (peaks[num] - otherMin)/(otherMin) > 0.07)
                    {
                        if (index[num] + 1 != 1)
                        {
                            vector<int> frameData;
                            diagInt++;
                            frameData.push_back(index[num] + n + 1);
                            frameData.push_back(diagInt);

                            lumDiagFrames.push_back(frameData);

                            peakIndex.push_back(index[num] + 1);
                        }
                    }
                }

                if (lumDiagFrames.size() != 0)
                {
                    if (diagInt >= 3)
                    {
                        seizureCount++;
                    }
                }

                vector<int> redPeakIndex = {};

                for (num = 0; num < peaksRed.size(); num++)
                {
                    vector<double> betweenPeaksMin;
                    vector<double> betweenPeaksMax;
                    double min = 0.0, otherMin = 0.0, deviation1 = 0.0, deviation2 = 0.0;

                    for (int abc = 0; abc < (redIndex[num + 1] - redIndex[num] - 1); abc++)
                    {
                        betweenPeaksMin.push_back(avgRedMatrix[abc + redIndex[num] + 1]);
                        min = *min_element(betweenPeaksMin.begin(), betweenPeaksMin.end());
                    }
                    if (num + 2 < redIndex.size())
                    {
                        for (int abc = 0; abc < (redIndex[num + 2] - redIndex[num + 1] - 1); abc++)
                        {
                            betweenPeaksMax.push_back(avgRedMatrix[abc + redIndex[num + 1] + 1]);
                            otherMin = *min_element(betweenPeaksMax.begin(), betweenPeaksMax.end());
                        }
                    }
                    else
                    {
                        for (int abc = 0; abc < (30 - redIndex[num + 1] - 1); abc++)
                        {
                            betweenPeaksMax.push_back(avgRedMatrix[abc + redIndex[num + 1] + 1]);
                            otherMin = *min_element(betweenPeaksMax.begin(), betweenPeaksMax.end());
                        }
                    }
                    qDebug() << "Check: " << (peaksRed[num] - min);
                    if ((peaksRed[num] - min) > 4.0 && (peaksRed[num] - otherMin) > 4.0)
                    {
                        if (redIndex[num] + 1 != 1)
                        {
                            vector<int> frameData;
                            redDiagInt++;
                            frameData.push_back(redIndex[num] + n + 1);
                            frameData.push_back(redDiagInt);

                            redDiagFrames.push_back(frameData);

                            redPeakIndex.push_back(redIndex[num] + 1);
                            if (adjustN == true)
                            {
                                qDebug() << redDiagInt;
                            }
                        }
                    }
                }

                if (redDiagFrames.size() != 0)
                {
                    if (redDiagInt >= 3)
                    {
                        redSeizureCount++;
                    }
                }


                vector<int> redBounds(2);
                redBounds = {};

                if (redSeizureCount != 0)
                {
                    redBounds.push_back(redDiagFrames[redDiagFrames.size() - redDiagFrames[redDiagFrames.size() - 1][1]][0]);
                    redBounds.push_back(redDiagFrames[redDiagFrames.size() - 1][0]);

                    qDebug() << "BROPred: " << redDiagFrames[redDiagFrames.size() - redDiagFrames[redDiagFrames.size() - 1][1]][0];
                    qDebug() << redBounds[0] << "___" << redBounds[1];
                    if (redDiagFrames[redDiagFrames.size() - redDiagFrames[redDiagFrames.size() - 1][1]][0] > redSeizureBoundaries[redSeizureBoundaries.size() - 1][1])
                    {
                        cout << redDiagFrames[redDiagFrames.size()-1][1];
                        redSeizureBoundaries.push_back(redBounds);



                        qDebug() << redDiagFrames[redDiagFrames.size() - 1][1];
                        //currentLumSizeSecond = 297 for some reason
                        //redDiagFrames.erase(redDiagFrames.begin() + currentLumSizeSecond - 1, redDiagFrames.begin() + redDiagFrames.size() - 1 - redDiagFrames[redDiagFrames.size() - 1][1]);
                    }
                    else {
                        qDebug() << "bokemonronto!";
                        redSeizureBoundaries[redSeizureBoundaries.size() - 1][1] = redDiagFrames[redDiagFrames.size() - 1][0];
                    }
                    qDebug() << "toronto!";
                    if (peakIndex.size() != 0)
                    {
                        if (n > n - peakIndex[0] + redPeakIndex[0])
                        {
                            n = n - peakIndex[0] + redPeakIndex[0];
                        }
                        else{
                            n += peakIndex[0];
                        }
                    }

                    qDebug() << "got eem!";
                    wasRedSeizure = true;
                    j = frame1.rows;
                    k = frame1.cols;

                    redSeizureCount = 0;
                    redDiagInt = 1;
                    if (wasSeizure == false)
                    {
                        n += redPeakIndex[0];
                    }

                }
                vector<int> bounds(2);
                bounds = {};

                if (seizureCount != 0)
                {
                    bounds.push_back(lumDiagFrames[lumDiagFrames.size() - lumDiagFrames[lumDiagFrames.size() - 1][1]][0]);
                    bounds.push_back(lumDiagFrames[lumDiagFrames.size() - 1][0]);

                    qDebug() << "BROP: " << lumDiagFrames[lumDiagFrames.size() - lumDiagFrames[lumDiagFrames.size() - 1][1]][0];
                    if (lumDiagFrames[lumDiagFrames.size() - lumDiagFrames[lumDiagFrames.size() - 1][1]][0] > seizureBoundaries[seizureBoundaries.size() - 1][1])
                    {
                        cout << lumDiagFrames[lumDiagFrames.size()-1][1];
                        seizureBoundaries.push_back(bounds);


                        qDebug() << currentLumSizeSecond;
                        qDebug() << lumDiagFrames[lumDiagFrames.size() - 1][1];
                        //currentLumSizeSecond = 297 for some reason
                        lumDiagFrames.erase(lumDiagFrames.begin() + currentLumSizeSecond - 1, lumDiagFrames.begin() + lumDiagFrames.size() - 1 - lumDiagFrames[lumDiagFrames.size() - 1][1]);
                    }
                    else {
                        seizureBoundaries[seizureBoundaries.size() - 1][1] = lumDiagFrames[lumDiagFrames.size() - 1][0];
                    }
                    n += peakIndex[0];
                    wasSeizure = true;
                    j = frame1.rows;
                    k = frame1.cols;

                    seizureCount = 0;
                    diagInt = 1;

                }
            }
        }

        if (wasSeizure == false)
        {

            qDebug() << "Test1";
            if (lumDiagFrames.size() != currentLumArrSize)
            {
                qDebug() << lumDiagFrames.size();
                qDebug() << currentLumArrSize;

                vector<int> newDiagFrames;
                for (int a = 0; a < lumDiagFrames.size() - currentLumArrSize; a++)
                {
                    qDebug() << "Test1.5";
                    newDiagFrames.push_back(lumDiagFrames[a + currentLumArrSize][0]);
                    qDebug() << newDiagFrames[a];
                }
                for (int y = 0; y < lumDiagFrames.size() - currentLumArrSize; y++)
                {
                    qDebug() << "Test2.5";
                    lumDiagFrames.pop_back();
                }

                sort(newDiagFrames.begin(), newDiagFrames.end());
                newDiagFrames.erase(unique(newDiagFrames.begin(), newDiagFrames.end()), newDiagFrames.end());

                for (int i = 0; i < newDiagFrames.size(); i++)
                {
                    qDebug() << "Test4: " << newDiagFrames[i];

                    if (i != 0)
                    {
                        lumDiagFrames.push_back({newDiagFrames[i], 2});
                    }
                    else {
                        lumDiagFrames.push_back({newDiagFrames[i], i + 1});
                    }
                }
                for (int x = 0; x < newDiagFrames.size(); x++)
                {
                    qDebug() << "__:" << newDiagFrames[x];
                }
                n = lumDiagFrames[lumDiagFrames.size() - lumDiagFrames[lumDiagFrames.size() - 1][1]][0];
                qDebug() << n;
                newDiagFrames.empty();
                diagInt = 0;
            }
            else {
                diagInt = 0;
                n += 30;
            }
            if (n + 30 > numOfFrames && adjustN == false)
            {
                n = numOfFrames - 30;
                adjustN = true;
            }
        }
    }

    vector< vector<int>> newLumInfo = {};

    //Loop to edit lumDiagFrames
    for (int i = 0; i < lumDiagFrames.size(); i++)
    {
        bool foundRepeat = false;

        for (int x = 0; x < newLumInfo.size(); x++)
        {
            if (lumDiagFrames[i][0] == newLumInfo[x][0])
            {
                foundRepeat = true;
                break;
            }
        }
        if (foundRepeat == false)
        {
            newLumInfo.push_back(lumDiagFrames[i]);
        }
    }

    vector< vector<int>> newRedLumInfo = {};

    //Loop to edit lumDiagFrames
    for (int i = 0; i < redDiagFrames.size(); i++)
    {
        bool foundRepeat = false;

        for (int x = 0; x < newRedLumInfo.size(); x++)
        {
            if (redDiagFrames[i][0] == newRedLumInfo[x][0])
            {
                foundRepeat = true;
                break;
            }
        }
        if (foundRepeat == false)
        {
            newRedLumInfo.push_back(redDiagFrames[i]);
        }
    }
    //
    if (seizureBoundaries.size() > 1) {
        if ( file.open(QIODevice::ReadWrite|QIODevice::Text) )
        {
            QTextStream stream( &file );
            for (int x = 1; x < seizureBoundaries.size(); x++)
            {
                stream << "Seizure at frames: " << seizureBoundaries[x][0] << " to " << seizureBoundaries[x][1] << endl;
            }
            for (int x = 1; x < redSeizureBoundaries.size(); x++)
            {
                stream << "Saturated red seizure at frames: " << redSeizureBoundaries[x][0] << " to " << redSeizureBoundaries[x][1] << endl;
            }
        }
    }
    else {
        file.open(QIODevice::ReadWrite|QIODevice::Text);
        if (file.pos() == 0)
        {
            QTextStream stream( &file );
            stream << "This video does not contain seizure-inducing frames." << endl;
        }
    }
    if (aarb == 0)
    {
        if ( aFile.open(QIODevice::ReadWrite|QIODevice::Text) )
        {
            QTextStream stream( &aFile );
            stream << numOfFrames << endl;
            for (int x = 1; x < seizureBoundaries.size(); x++)
            {
                stream << seizureBoundaries[x][0] << endl;
                stream << seizureBoundaries[x][1] << endl;
            }
            stream << "RedSeizure" << endl;
            for (int x = 1; x < redSeizureBoundaries.size(); x++)
            {
                stream << redSeizureBoundaries[x][0] << endl;
                stream << redSeizureBoundaries[x][1] << endl;
            }
        }
    }
    else {
        if ( aFile.open(QIODevice::ReadWrite|QIODevice::Text) )
        {
            QTextStream stream( &aFile );
            for (int x = 1; x < seizureBoundaries.size(); x++)
            {
                stream << seizureBoundaries[x][0] << endl;
                stream << seizureBoundaries[x][1] << endl;
            }
        }
    }

    for (int x = 0; x < seizureBoundaries.size(); x++)
    {
        cout << "Seizure: " << seizureBoundaries[x][0] << " to " << seizureBoundaries[x][1] << endl;
    }
    aFile.open(QIODevice::ReadWrite|QIODevice::Text);
    QFile in(&aFile);
    QTextStream stream(&aFile);
    QString line;
    if(in.atEnd())
    {
        stream << "Diagnostic" << endl;
        for (int x = 0; x < newLumInfo.size(); x++)
        {
            if (x == 0)
            {
                stream << 0 << endl;
                stream << 0 << endl;
            }
            stream << newLumInfo[x][0] << endl;
            stream << newLumInfo[x][1] << endl;
            if (x == newLumInfo.size() - 1)
            {
                stream << numOfFrames << endl;
                stream << 0 << endl;
            }
        }
        stream << "RedDiagnostic" << endl;
        for (int x = 0; x < newRedLumInfo.size(); x++)
        {
            if (x == 0)
            {
                stream << 0 << endl;
                stream << 0 << endl;
            }
            stream << newRedLumInfo[x][0] << endl;
            stream << newRedLumInfo[x][1] << endl;
            if (x == newRedLumInfo.size() - 1)
            {
                stream << numOfFrames << endl;
                stream << 0 << endl;
            }
        }
    }

    progress.close();
    return seizureBoundaries;

    // Make this function only work for areas of pixels that take up more than 10% of the viewport
}


void rkbcore(string vidDir, QString report, vector<int> props)
{

    vector<vector<int > > randMatrix = seizureDetection(vidDir, report, props);

    /*
    int decision;
    cout << "Choose a filter method: 1. Color-shift 2. Change Contrast 3. Overlay 4. Blur" << endl;
    cin >> decision;
    double alpha;

    */
}


void proTool(QString ndir, QString report, int decision, double alpha)
{
    std::string dir = ndir.toUtf8().constData();
    dir = dir + "/frames/";
    bool folderValid;
    bool valid = true;

    //Check if fields are valid
    string testDir = dir + "1.jpg";
    Mat testImg = imread(testDir);
    if (testImg.empty())
    {
        folderValid = false;
        QMessageBox::critical(0, "Error", "This is not a valid project folder. The project folder should contain the folder 'frames' and should be generated by an analysis. Please try again.");
    }
    else
    {
        folderValid = true;
        QFile reportFile(report);
        vector<int> warn;
        int count = 0;

        //Check suffix
        QFileInfo info(report);
        QString ext = info.suffix();

        if (!reportFile.open(QIODevice::ReadOnly) || ext != "txt")
        {
            QMessageBox::critical(0, "Error", "Could not open the report");
        }
        else {

            QTextStream in(&reportFile);
            QString line;
            int counter = 0;
            int ynum;
            vector<double> y;
            int start = 0;
            int end = 0;
            bool atStart = true;
            bool reachedDiagnostic = false;
            bool reachedRed = false;
            int n = 0;
            int otherCounter = 0;

            while (!in.atEnd())
            {
                line = in.readLine();
                std::string newLine = line.toUtf8().constData();
                ynum = line.toInt();
                if (line == "RedSeizure")
                {
                    reachedRed = true;
                }
                if (line == "Diagnostic")
                {
                    reachedDiagnostic = true;
                }
                if (newLine.find_first_not_of("0123456789.") != std::string::npos && reachedDiagnostic == false && reachedRed == false)
                {
                    valid = false;
                }
                if (counter > 0 && valid == true && reachedDiagnostic == false && reachedRed == false)
                {
                    y.push_back(ynum);
                }
                else if (counter > 0 && valid == true && reachedDiagnostic == false && reachedRed == true && otherCounter == 0)
                {
                    otherCounter++;
                }
                else if (counter > 0 && valid == true && reachedDiagnostic == false && reachedRed == true && otherCounter > 0)
                {
                    y.push_back(ynum);
                }

                counter++;
            }
            if (valid == true)
            {
                for (int a = 0; a < y.size(); a++)
                {
                    warn.push_back(y[a]);
                }
                if( decision == 1){
                    QCoreApplication::processEvents();
                    QProgressDialog progress("Running prophylactic tool...", "Abort", 0, warn.size());
                    progress.setWindowModality(Qt::WindowModal);
                    progress.setFixedSize(500, 100);
                    progress.show();
                    QCoreApplication::processEvents();
                    int sampleRate = 0;
                    int extraFrames = 0;
                    for (int i = 0; i < warn.size(); i += 2)
                    {
                        if (warn.size() != 0 && warn[i] != 0)
                        {
                            sampleRate = floor((warn[i+1] - warn[i])/30);
                            extraFrames = (warn[i+1] - warn[i]) % 30;
                            if(sampleRate > 0)
                            {
                                for(int b = 0; b < sampleRate; b++)
                                {
                                    makeFrameGradient(warn[i] + 1 + 30*(b), warn[i] + 30*(b+1), dir, dir);
                                }
                                makeFrameGradient(warn[i] + 16*sampleRate + 1, warn[i] + 30*sampleRate + extraFrames, dir, dir);
                            }
                            else {
                                makeFrameGradient(warn[i] + 1, (warn[i+1]), dir, dir);
                            }
                            count += 2;
                            progress.setValue(count);
                        }
                    }
                    progress.close();
                    count = 0;
                }
                else if (decision == 2){
                    QCoreApplication::processEvents();
                    QProgressDialog progress("Running prophylactic tool...", "Abort", 0, warn.size());
                    progress.setWindowModality(Qt::WindowModal);
                    progress.setFixedSize(500, 100);
                    progress.show();
                    QCoreApplication::processEvents();
                    for (int i = 0; i < warn.size(); i += 2)
                    {
                        if (warn.size() != 0)
                        {
                            cout << warn[i] << endl;
                            overlayFilter(warn[i], (warn[i+1]), dir, alpha);
                            count++;
                            progress.setValue(count);
                        }
                    }
                    progress.close();
                    count = 0;
                }
                else if (decision == 3){
                    QCoreApplication::processEvents();
                    QProgressDialog progress("Running prophylactic tool...", "Abort", 0, warn.size());
                    progress.setWindowModality(Qt::WindowModal);
                    progress.setFixedSize(500, 100);
                    progress.show();
                    QCoreApplication::processEvents();
                    for (int i = 0; i < warn.size(); i++)
                    {
                        if (warn.size() != 0)
                        {
                            lowerContrast(warn[i]+1, (warn[i]+30), dir, alpha);
                            count++;
                            progress.setValue(count);
                        }
                    }
                    progress.close();
                    count = 0;
                }
                else if (decision == 4){
                    QCoreApplication::processEvents();
                    QProgressDialog progress("Running prophylactic tool...", "Abort", 0, warn.size());
                    progress.setWindowModality(Qt::WindowModal);
                    progress.setFixedSize(500, 100);
                    progress.show();
                    QCoreApplication::processEvents();
                    for (int i = 0; i < warn.size(); i++)
                    {
                        if (warn.size() != 0)
                        {
                            blurFilter(warn[i]+1, (warn[i]+30), dir, alpha);
                            count++;
                            progress.setValue(count);
                        }
                    }
                    progress.close();
                    count = 0;
                }
            }
            else {
                QMessageBox::critical(0, "Error", "That is not a valid non-user project report. Ensure that the report you selected does not have the 'USER' flag in its name and that the content is not corrupted.");
            }
        }
    }
    if (folderValid = true && valid == true)
    {
        QMessageBox messageBox;
        QMessageBox::StandardButton reply;
        reply = messageBox.question(0, "Write video", "Write the edited video?", QMessageBox::Yes|QMessageBox::No);
        messageBox.setFixedSize(600,400);

        if (reply == QMessageBox::Yes)
        {
            writeVideo(ndir, report);
            QMessageBox messageBox3;
            messageBox3.information(0, "Success!", "The prophylactic tool has completed running. Your edited video is available in your project directory if the codecs (XviD) were properly installed on your system.", QMessageBox::NoButton);
            messageBox3.setFixedSize(600,400);
        }
    }
    // makeFrameGradient(1, 30, vidDir);
    // TO PUT TOGETHER VIDEO USE: ffmpeg -framerate 60 -i %d.jpg video.mp4
    /*
    std::string dir = ndir.toUtf8().constData();
    dir = dir + "/frames/";
    bool folderValid;
    bool valid = true;

    //Check if fields are valid
    string testDir = dir + "1.jpg";
    Mat testImg = imread(testDir);
    if (testImg.empty())
    {
        folderValid = false;
        QMessageBox::critical(0, "Error", "This is not a valid project folder. The project folder should contain the folder 'frames' and should be generated by an analysis. Please try again.");
    }
    else
    {
        folderValid = true;
        QFile reportFile(report);
        vector<int> warn;
        int count = 0;

        //Check suffix
        QFileInfo info(report);
        QString ext = info.suffix();

        if (!reportFile.open(QIODevice::ReadOnly) || ext != "txt")
        {
            QMessageBox::critical(0, "Error", "Could not open the report");
        }
        else {

            QTextStream in(&reportFile);
            QString line;
            int counter = 0;
            double ynum;
            vector<double> y;

            bool ok;
            int n = 0;

            while (!in.atEnd())
            {
                line = in.readLine();
                std::string newLine = line.toUtf8().constData();
                ynum = line.toDouble();

                if (newLine.find_first_not_of("0123456789.") != std::string::npos)
                {
                    valid = false;
                }
                if (counter > 0 && valid == true)
                {
                    y.push_back(ynum);
                }

                counter++;
            }
            if (valid == true)
            {
                for (int a = 0; a < y.size(); a++)
                {
                    if (y[a] >= 0.25)
                    {
                        warn.push_back(30*a);
                    }
                }
                if( decision == 1){
                    QCoreApplication::processEvents();
                    QProgressDialog progress("Running prophylactic tool...", "Abort", 0, warn.size());
                    progress.setWindowModality(Qt::WindowModal);
                    progress.setFixedSize(500, 100);
                    progress.show();
                    QCoreApplication::processEvents();
                    for (int i = 0; i < warn.size(); i++)
                    {
                        if (warn.size() != 0)
                        {
                            makeFrameGradient(warn[i]+1, (warn[i]+30), dir, dir);
                            count++;
                            progress.setValue(count);
                        }
                    }
                    progress.close();
                    count = 0;
                }
                else if (decision == 2){
                    QCoreApplication::processEvents();
                    QProgressDialog progress("Running prophylactic tool...", "Abort", 0, warn.size());
                    progress.setWindowModality(Qt::WindowModal);
                    progress.setFixedSize(500, 100);
                    progress.show();
                    QCoreApplication::processEvents();
                    for (int i = 0; i < warn.size(); i++)
                    {
                        if (warn.size() != 0)
                        {
                            overlayFilter(warn[i]+1, (warn[i]+30), dir, alpha);
                            count++;
                            progress.setValue(count);
                        }
                    }
                    progress.close();
                    count = 0;
                }
                else if (decision == 3){
                    QCoreApplication::processEvents();
                    QProgressDialog progress("Running prophylactic tool...", "Abort", 0, warn.size());
                    progress.setWindowModality(Qt::WindowModal);
                    progress.setFixedSize(500, 100);
                    progress.show();
                    QCoreApplication::processEvents();
                    for (int i = 0; i < warn.size(); i++)
                    {
                        if (warn.size() != 0)
                        {
                            lowerContrast(warn[i]+1, (warn[i]+30), dir, alpha);
                            count++;
                            progress.setValue(count);
                        }
                    }
                    progress.close();
                    count = 0;
                }
                else if (decision == 4){
                    QCoreApplication::processEvents();
                    QProgressDialog progress("Running prophylactic tool...", "Abort", 0, warn.size());
                    progress.setWindowModality(Qt::WindowModal);
                    progress.setFixedSize(500, 100);
                    progress.show();
                    QCoreApplication::processEvents();
                    for (int i = 0; i < warn.size(); i++)
                    {
                        if (warn.size() != 0)
                        {
                            blurFilter(warn[i]+1, (warn[i]+30), dir, alpha);
                            count++;
                            progress.setValue(count);
                        }
                    }
                    progress.close();
                    count = 0;
                }
            }
            else {
                QMessageBox::critical(0, "Error", "That is not a valid non-user project report. Ensure that the report you selected does not have the 'USER' flag in its name and that the content is not corrupted.");
            }
        }
    }
    if (folderValid = true && valid == true)
    {
        QMessageBox messageBox;
        QMessageBox::StandardButton reply;
        reply = messageBox.question(0, "Write video", "Write the edited video?", QMessageBox::Yes|QMessageBox::No);
        messageBox.setFixedSize(600,400);

        if (reply == QMessageBox::Yes)
        {
            writeVideo(ndir, report);
            QMessageBox messageBox3;
            messageBox3.information(0, "Success!", "The prophylactic tool has completed running. Your edited video is available in your project directory if the codecs (XviD) were properly installed on your system.", QMessageBox::NoButton);
            messageBox3.setFixedSize(600,400);
        }
    }
    // makeFrameGradient(1, 30, vidDir);
    // TO PUT TOGETHER VIDEO USE: ffmpeg -framerate 60 -i %d.jpg video.mp4
    */
}
