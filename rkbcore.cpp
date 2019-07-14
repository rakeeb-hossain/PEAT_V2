//
//  main.cpp
//  GradientTest
//
//  Created by Hossain, Rakeeb on 2018-01-02.
//  Copyright © 2018 Hossain, Rakeeb. All rights reserved.
//
#include "mainframe.h"
#include "rkbcore.h"
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

const int CHUNK = 32;
const int SCALE = 4;
const int N = CHUNK/SCALE;
const int AREA = N*N;
const double ST = 0.80;
const int RTT = 20;
const double FT = 0.10;
const double DT = 0.80;
const double LT = 0.10;
const double ATH = 0.05; // High sensitivity = 0.05, low = 0.10
const double RTH = 0.10;

rObject::rObject(QObject *parent) : QObject(parent) {
}

rObject::~rObject() {}

void rObject::makeFrameGradient(int lowerBound, int upperBound, string gradientFolder, string vidDir) {
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

Mat rObject::lowerContrast(Mat frame, double alpha)
{
    Mat updatedFrame = Mat::zeros( frame.size(), frame.type() );


    for(int j = 0; j < frame.rows; j++)
    {
        const Vec3b* framei = frame.ptr<Vec3b>(j);
        for(int i = 0; i < frame.cols; i++) {
            updatedFrame.at<Vec3b>(j,i)[0] = saturate_cast<uchar>((1.0 - alpha)*(frame.at<Vec3b>(j,i)[0]) + 0);
            updatedFrame.at<Vec3b>(j,i)[1] = saturate_cast<uchar>((1.0 - alpha)*(frame.at<Vec3b>(j,i)[1]) + 0);
            updatedFrame.at<Vec3b>(j,i)[2] = saturate_cast<uchar>((1.0 - alpha)*(frame.at<Vec3b>(j,i)[2]) + 0);
        }
    }
    /*
    parallel_for_(Range(0, frame.rows*frame.cols), [&](const Range& range) {
        for (int r = range.start; r < range.end; r++) {
            int x = r % frame.rows;
            int y = r / frame.rows;
            updatedFrame.at<Vec3b>(x,y)[0] = saturate_cast<uchar>((1.0 - alpha)*(frame.at<Vec3b>(x,y)[0]) + 0);
            updatedFrame.at<Vec3b>(x,y)[1] = saturate_cast<uchar>((1.0 - alpha)*(frame.at<Vec3b>(x,y)[1]) + 0);
            updatedFrame.at<Vec3b>(x,y)[2] = saturate_cast<uchar>((1.0 - alpha)*(frame.at<Vec3b>(x,y)[2]) + 0);
        }
    });
    */
    return updatedFrame;
}

Mat rObject::reduceGreen(Mat frame, double alpha)
{
    Mat updatedFrame = Mat::zeros( frame.size(), frame.type() );


    for(int j = 0; j < frame.rows; j++)
    {
        const Vec3b* framei = frame.ptr<Vec3b>(j);
        for(int i = 0; i < frame.cols; i++) {
            updatedFrame.at<Vec3b>(j,i)[0] = saturate_cast<uchar>((1.0)*(frame.at<Vec3b>(j,i)[0]) + 0);
            updatedFrame.at<Vec3b>(j,i)[1] = saturate_cast<uchar>((1.0 - alpha)*(frame.at<Vec3b>(j,i)[1]) + 0);
            updatedFrame.at<Vec3b>(j,i)[2] = saturate_cast<uchar>((1.0 - alpha)*(frame.at<Vec3b>(j,i)[2]) + 0);
        }
    }
    /*
    parallel_for_(Range(0, frame.rows*frame.cols), [&](const Range& range) {
        for (int r = range.start; r < range.end; r++) {
            int x = r % frame.rows;
            int y = r / frame.rows;
            updatedFrame.at<Vec3b>(x,y)[0] = saturate_cast<uchar>((1.0 - alpha)*(frame.at<Vec3b>(x,y)[0]) + 0);
            updatedFrame.at<Vec3b>(x,y)[1] = saturate_cast<uchar>((1.0 - alpha)*(frame.at<Vec3b>(x,y)[1]) + 0);
            updatedFrame.at<Vec3b>(x,y)[2] = saturate_cast<uchar>((1.0 - alpha)*(frame.at<Vec3b>(x,y)[2]) + 0);
        }
    });
    */
    return updatedFrame;
}

Mat rObject::overlayFilter(Mat frame, double alpha)
{
    qDebug() << "INT";
    Mat overlay;
    frame.copyTo(overlay);
    cv::rectangle(overlay, cv::Rect(0, 0, frame.cols, frame.rows), cv::Scalar(50, 50, 50), -1);
    cv::addWeighted(overlay, alpha, frame, 1.0 - alpha, 0.0, frame);
    return frame;
}

Mat rObject::blurFilter(Mat frame, double alpha)
{
    int kernal = alpha*225;
    if (kernal % 2 == 0)
    {
        kernal++;
    }
    if (alpha != 0.0)
    {
        blur(frame, frame, Size(kernal, kernal));
    }
    return frame;
}

Mat rObject::swapChannels(Mat frame) {
    Mat updatedFrame = Mat::zeros( frame.size(), frame.type() );

    for(int j = 0; j < frame.rows; j++)
    {
        const Vec3b* framei = frame.ptr<Vec3b>(j);
        for(int i = 0; i < frame.cols; i++) {
            updatedFrame.at<Vec3b>(j,i)[0] = saturate_cast<uchar>((1.0)*(frame.at<Vec3b>(j,i)[2]) + 0);
            updatedFrame.at<Vec3b>(j,i)[1] = saturate_cast<uchar>((1.0)*(frame.at<Vec3b>(j,i)[1]) + 0);
            updatedFrame.at<Vec3b>(j,i)[2] = saturate_cast<uchar>((1.0)*(frame.at<Vec3b>(j,i)[0]) + 0);
        }
    }
    return updatedFrame;
}

void rObject::writeVideo(QString ndir, QString nreport)
{
    std::string dir = ndir.toUtf8().constData();
    std::string oldDir = ndir.toUtf8().constData();
    dir = dir + "/frames/";
    QFile reportFile(nreport);
    int fileNum = 0;

    if (!reportFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(0, "Error", "Could not open the report");
    }
    else {
        QTextStream in(&reportFile);
        QString line;
        int counter = 0;

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


float rObject::relative_luminance(float R, float G, float B) {
    return (0.2126*R+0.7152*G+0.0722*B);
}

float rObject::saturation(float R, float G, float B) {
    return (R/(R+G+B));
}

float rObject::red_saturation(float R, float G, float B) {
    float red = (R-G-B)*320.f;
    return (red >= 0 ? red : 0);
}
void rObject::stopLoop() {
    threadStopped = true;
}

void rObject::rkbcore(string filename) {
    int count = 1;
    VideoCapture cap(filename);

    if (!cap.isOpened()) {emit error(); return;};

    //Load first frame
    Mat frame;
    cap >> frame;
    int frame_count = cap.get(CAP_PROP_FRAME_COUNT);
    double fps = cap.get(CAP_PROP_FPS);
    if (frame.empty()) {emit error(); return;};

    //Resize first frame
    auto scale = 1.0 / SCALE;
    resize(frame, frame, Size(), scale, scale);

    //Round dimensions for analysis based on chunk size (N)
    int length = frame.cols - (frame.cols % N);
    int width = frame.rows - (frame.rows % N);

    //Declare arrays of avg. lum. values & red. sat. values of chunks
    int num_chunks = length*width/AREA;
    vector<float> lum_first(num_chunks);
    vector<float> lum_next(num_chunks);
    vector<float> sat_first(num_chunks);
    vector<float> sat_next(num_chunks);
    vector<float> red_first(num_chunks);
    vector<float> red_next(num_chunks);

    //Declare arrays of data points
    vector<int> lum_diag = {0};
    vector<int> red_diag = {0};
    int seizure_frames = 0;
    int red_seizure_frames = 0;

    parallel_for_(Range(0, length*width), [&](const Range& range) {
        for (int r = range.start; r < range.end; r++) {
            int c_num = r / AREA;
            int x = N*(c_num % (length/N)) + r % N;
            int y = N*(c_num / (length/N)) + (r % AREA) / N;

            float sB = frame.at<Vec3b>(y,x)[0]/255.f;
            float sG = frame.at<Vec3b>(y,x)[1]/255.f;
            float sR = frame.at<Vec3b>(y,x)[2]/255.f;
            float B = (sB <= 0.03928 ? sB/12.92 : pow((sB+0.055)/1.055, 2.4));
            float G = (sG <= 0.03928 ? sG/12.92 : pow((sG+0.055)/1.055, 2.4));
            float R = (sR <= 0.03928 ? sR/12.92 : pow((sR+0.055)/1.055, 2.4));

            float lum = relative_luminance(R,G,B);
            float sat = saturation(R,G,B);
            float red_val = red_saturation(R,G,B);

            lum_first[c_num] += lum;
            sat_first[c_num] += sat;
            red_first[c_num] += red_val;
        }
    });

    parallel_for_(Range(0, num_chunks), [&](const Range& range) {
        for (int r = range.start; r < range.end; r++) {
            lum_first[r] = lum_first[r] / (float) AREA;
            sat_first[r] = ((sat_first[r] / (float) AREA) >= ST ? sat_first[r] / (float) AREA : 0);
            red_first[r] = (sat_first[r] != 0 ? red_next[r] / (float) AREA : 0);
        }
    });

    bool just_flashed = false; //True if flash just occurred (now looking for decrease in luminance)
    bool just_red_flashed = false;

    //Loop through frames, loading two at a time
    while(count < frame_count && !threadStopped) {
        vector<QVector<double > > points_x(4);
        vector<QVector<double > > points_y(4);
        int seizure_count = 0;
        int red_count = 0;
        int flash_count = 0;
        int red_flash_count = 0;

        //Load new frame and resize
        Mat next_frame;
        cap >> next_frame;
        if (frame.empty() || next_frame.empty()) {emit error(); return;};
        resize(next_frame, next_frame, Size(), scale, scale);

        //Parallel loop through pixels in NxN chunks to get lum. and red sat. values
        parallel_for_(Range(0, length*width), [&](const Range& range) {
            for (int r = range.start; r < range.end; r++) {
                int c_num = r / AREA;
                int x = N*(c_num % (length/N)) + r % N;
                int y = N*(c_num / (length/N)) + (r % AREA) / N;

                //Inverse gamma correction for linear RGB
                float sB = frame.at<Vec3b>(y,x)[0]/255.f;
                float sG = frame.at<Vec3b>(y,x)[1]/255.f;
                float sR = frame.at<Vec3b>(y,x)[2]/255.f;
                float B = (sB <= 0.03928 ? sB/12.92 : pow((sB+0.055)/1.055, 2.4));
                float G = (sG <= 0.03928 ? sG/12.92 : pow((sG+0.055)/1.055, 2.4));
                float R = (sR <= 0.03928 ? sR/12.92 : pow((sR+0.055)/1.055, 2.4));

                float lum = relative_luminance(R,G,B);
                float sat = saturation(R,G,B);
                float red_val = red_saturation(R,G,B);

                lum_next[c_num] += lum;
                sat_next[c_num] += sat;
                red_next[c_num] += red_val;
            }
        });
        //Parallel loop to calculate and compare chunk averages between the frames
        parallel_for_(Range(0, num_chunks), [&](const Range& range) {
            for (int r = range.start; r < range.end; r++) {
                lum_next[r] = lum_next[r] / (float) AREA;
                sat_next[r] = ((sat_next[r] / (float) AREA) >= ST ? sat_next[r] / (float) AREA : 0);
                red_next[r] = (sat_next[r] != 0 ? red_next[r] / (float) AREA : 0);

                if (just_flashed == false) {
                    if (lum_next[r] - lum_first[r] > FT && lum_first[r] < DT) {
                        seizure_count++;
                    }
                }
                else {
                    if (lum_first[r] - lum_next[r] > FT && lum_next[r] < DT) {
                        seizure_count++;
                    }
                }

                //CONSIDER: checking only if abs. diff. of red values is > 20
                if (just_red_flashed == false) {
                    //INVESTIGATE: WCAG 2.0 Guidelines say that both states should be saturated
                    //if (sat_first[r] != 0 && sat_next[r] != 0)
                    if (sat_next[r] != 0) {
                        if (red_next[r] > RTT) {
                            red_count++;
                        }
                    }
                }
                else {
                    if (sat_first[r] != 0) {
                        if (red_first[r] > RTT ) {
                            red_count++;
                        }
                    }
                }

            }
        });

        //Check # of increases (flashes) in diag frames in past 1 second and set flash counts to that
        if (count > fps) {
            for (int i = count-fps; i < count-1; i++) {
                if (lum_diag[i] < lum_diag[i+1]) flash_count++;
                if (red_diag[i] < red_diag[i+1]) red_flash_count++;
            }
        }
        else {
            for (int i = 0; i < count-1; i++) {
                if (lum_diag[i] < lum_diag[i+1]) flash_count++;
                if (red_diag[i] < red_diag[i+1]) red_flash_count++;
            }
        }

        //Increment flash counts if a valid flash occurs in the transition to the next frame
        if (seizure_count*AREA >= ATH*frame.cols*frame.rows) {
            if (just_flashed == false) {
                flash_count++;
                just_flashed = true;
            }
            else {
                //flash_count++;
                just_flashed = false;
            }
        }

        if (red_count*AREA >= RTH*frame.cols*frame.rows) {
            if (just_red_flashed == false) {
                red_flash_count++;
                just_red_flashed = true;
            }
            else {
                //red_flash_count++;
                just_red_flashed = false;
            }
        }

        //Update data points
        //FIX: sloped transition between points i.e. (0,0) -> (1,1) instead of (0,0) -> (1,0) & (1,1)
        if (flash_count == lum_diag[count-1]) {
            //Call plot function that only plots next point
            points_x[0].push_back((double)count);
            points_y[0].push_back((double)flash_count/10.0);
            lum_diag.push_back(flash_count);
        }
        else {
            points_x[0].push_back((double)count);
            points_y[0].push_back((double)lum_diag[count-1]/10.0);
            points_x[0].push_back((double)count);
            points_y[0].push_back((double)flash_count/10.0);
            //Call different plot function that plots past point at new x-value and new point (to stop sloping)
            lum_diag.push_back(flash_count);
        }

        if (red_flash_count == red_diag[count-1]) {
            //Call plot function that only plots next point
            points_x[1].push_back((double)count);
            points_y[1].push_back((double)red_flash_count/10.0);
            red_diag.push_back(red_flash_count);
        }
        else {
            //Call different plot function that plots past point at new x-value and new point (to stop sloping)
            points_x[1].push_back((double)count);
            points_y[1].push_back((double)red_diag[count-1]/10.0);
            points_x[1].push_back((double)count);
            points_y[1].push_back((double)red_flash_count/10.0);
            red_diag.push_back(red_flash_count);
        }

        int is_seizure = (flash_count >= 3);
        int is_red_sat = (red_flash_count >= 3);
        if (is_seizure == seizure_frames) {
            //Call plot function that only plots next point
            points_x[2].push_back((double)count);
            points_y[2].push_back((double)is_seizure);
            seizure_frames = is_seizure;
        }
        else {
            points_x[2].push_back((double)count);
            points_y[2].push_back((double)seizure_frames);
            points_x[2].push_back((double)count);
            points_y[2].push_back((double)is_seizure);
            //Call different plot function that plots past point at new x-value and new point (to stop sloping)
            seizure_frames = is_seizure;
        }

        if (is_red_sat == red_seizure_frames) {
            //Call plot function that only plots next point
            points_x[3].push_back((double)count);
            points_y[3].push_back((double)is_red_sat);
            red_seizure_frames = is_red_sat;
        }
        else {
            //Call different plot function that plots past point at new x-value and new point (to stop sloping)
            points_x[3].push_back((double)count);
            points_y[3].push_back((double)red_seizure_frames);
            points_x[3].push_back((double)count);
            points_y[3].push_back((double)is_red_sat);
            red_seizure_frames = is_red_sat;
        }
        //cout << "Count: " << count << endl;
        //Plot points and update signals
        emit updateUI(points_x, points_y);

        if (count % 4 == 0) {
            emit progressCount(count, round(frame_count));
        }

        //Set frame to next_frame
        frame = next_frame.clone();
        lum_first = lum_next;
        sat_first = sat_next;
        red_first = red_next;
        count++;
    }
    if (threadStopped) emit stopped();

    emit finished();
}

QString rObject::proTool(vector<vector<int > > vidData, QString ndir, QString origVidFile, int decision, double alpha)
{
    if (ndir == origVidFile) return "You cannot replace the original video used for the analysis. Please select a different output video path.";

    if (vidData.size() < 5) {
        return "A valid report was not loaded.";
    }
    else if (vidData[0].size() != vidData[4][1]) {
        return "A valid report was not loaded.";
    }

    string dir = ndir.toUtf8().constData();
    string vidFile = origVidFile.toUtf8().constData();

    vector<int> warn;

    VideoCapture cap(vidFile);
    if (!cap.isOpened()) return "Original video frames could not be decoded.";
    Mat frame;
    cap >> frame;
    if (frame.empty()) return "A blank video frame was encountered during the video writing process. Video stream could not be decoded.";
    int frame_width = frame.cols;
    int frame_height = frame.rows;
    int fps = round(vidData[4][0]);
    VideoWriter video(dir, CV_FOURCC('X','V','I','D'), fps, Size(frame_width,frame_height));

    waitDialog *progress = new waitDialog;
    progress->setProgressBarRange(0, 0);
    progress->setImg(QPixmap::fromImage(QImage((unsigned char*) frame.data, frame.cols, frame.rows, QImage::Format_RGB888)));
    progress->show();

    int range = vidData[4][1];
    if (decision == 6) {
        int speed = (int)alpha;
        for (int i = 0; i < vidData[2].size(); i++) {
            if (vidData[2][i] == 1 || vidData[3][i] == 1) {
                range = range + alpha - 1;
            }
            QCoreApplication::processEvents();
        }
    } else if (decision == 7) {
        for (int i = 0; i < vidData[2].size(); i++) {
            if (vidData[2][i] == 1 || vidData[3][i] == 1) {
                range -= 1;
            }
            QCoreApplication::processEvents();
        }
    }

    for (int i = 0; i < vidData[2].size()-1; i++) {
        QCoreApplication::processEvents();
        if (vidData[2][i+1] == 1 && vidData[2][i] == 0) {
            int j = i-1;
            bool found_two = false;
            while (!found_two) {
                vidData[2][j] = 1;
                if (vidData[0][j] == 1) found_two = true;
                j--;
            }
        }
        if (vidData[3][i+1] == 1 && vidData[3][i] == 0) {
            int j = i-1;
            bool found_two = false;
            while (!found_two) {
                vidData[3][j] = 1;
                if (vidData[1][j] == 1) found_two = true;
                j--;
            }
        }
    }

    progress->setProgressBarRange(0, range);

    if( decision == 1){
        /*
        int sampleRate = 0;
        int extraFrames = 0;
        for (unsigned int i = 0; i < warn.size(); i += 2)
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
            }
        }
        */
        for (unsigned int i = 0; i < vidData[2].size(); i++)
        {
            if (frame.empty()) {
                return "A blank video frame was encountered during the video writing process. Video stream could not be decoded.";
            }
            if ((int)vidData[2][i] == 1 || (int)vidData[3][i] == 1)
            {
                Mat new_frame = reduceGreen(frame, alpha);
                video.write(new_frame);
                cvtColor(new_frame, new_frame, CV_BGR2RGB);
                progress->setImg(QPixmap::fromImage(QImage((unsigned char*) new_frame.data, new_frame.cols, new_frame.rows, new_frame.step, QImage::Format_RGB888)));
            }
            else {
                video.write(frame);
                cvtColor(frame, frame, CV_BGR2RGB);
                progress->setImg(QPixmap::fromImage(QImage((unsigned char*) frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888)));
            }
            progress->setProgressBarValue(i+1);
            QCoreApplication::processEvents();
            if (i != vidData[2].size()-1) cap >> frame;
        }
    }
    else if (decision == 2){
        for (unsigned int i = 0; i < vidData[2].size(); i++)
        {
            if (frame.empty()) {
                return "A blank video frame was encountered during the video writing process. Video stream could not be decoded.";
            }
            if ((int)vidData[2][i] == 1 || (int)vidData[3][i] == 1)
            {
                Mat new_frame = overlayFilter(frame, alpha);
                video.write(new_frame);
                cvtColor(new_frame, new_frame, CV_BGR2RGB);
                progress->setImg(QPixmap::fromImage(QImage((unsigned char*) new_frame.data, new_frame.cols, new_frame.rows, new_frame.step, QImage::Format_RGB888)));
            }
            else {
                video.write(frame);
                cvtColor(frame, frame, CV_BGR2RGB);
                progress->setImg(QPixmap::fromImage(QImage((unsigned char*) frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888)));
            }
            progress->setProgressBarValue(i+1);
            QCoreApplication::processEvents();
            if (i != vidData[2].size()-1) cap >> frame;
        }
    }
    else if (decision == 3){
        QTime time;
        time.start();
        for (unsigned int i = 0; i < vidData[2].size(); i++)
        {
            if (frame.empty()) {
                return "A blank video frame was encountered during the video writing process. Video stream could not be decoded.";
            }
            if ((int)vidData[2][i] == 1 || (int)vidData[3][i] == 1)
            {
                Mat new_frame = lowerContrast(frame, alpha);
                video.write(new_frame);
                cvtColor(new_frame, new_frame, CV_BGR2RGB);
                progress->setImg(QPixmap::fromImage(QImage((unsigned char*) new_frame.data, new_frame.cols, new_frame.rows, new_frame.step, QImage::Format_RGB888)));
            }
            else {
                video.write(frame);
                cvtColor(frame, frame, CV_BGR2RGB);
                progress->setImg(QPixmap::fromImage(QImage((unsigned char*) frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888)));
            }
            progress->setProgressBarValue(i+1);
            QCoreApplication::processEvents();
            if (i != vidData[2].size()-1) cap >> frame;
        }
        qDebug() << time.elapsed();
    }
    else if (decision == 4){
        for (unsigned int i = 0; i < vidData[2].size(); i++)
        {
            if (frame.empty()) {
                return "A blank video frame was encountered during the video writing process. Video stream could not be decoded.";
            }
            if ((int)vidData[2][i] == 1 || (int)vidData[3][i] == 1)
            {
                Mat new_frame = blurFilter(frame, alpha);
                video.write(new_frame);
                cvtColor(new_frame, new_frame, CV_BGR2RGB);
                progress->setImg(QPixmap::fromImage(QImage((unsigned char*) new_frame.data, new_frame.cols, new_frame.rows, new_frame.step, QImage::Format_RGB888)));
            }
            else {
                video.write(frame);
                cvtColor(frame, frame, CV_BGR2RGB);
                progress->setImg(QPixmap::fromImage(QImage((unsigned char*) frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888)));
            }
            progress->setProgressBarValue(i+1);
            QCoreApplication::processEvents();
            if (i != vidData[2].size()-1) cap >> frame;
        }
    }
    else if (decision == 5){
        for (unsigned int i = 0; i < vidData[2].size(); i++)
        {
            if (frame.empty()) {
                return "A blank video frame was encountered during the video writing process. Video stream could not be decoded.";
            }
            if ((int)vidData[3][i] == 1)
            {
                Mat new_frame = swapChannels(frame);
                video.write(new_frame);
                cvtColor(new_frame, new_frame, CV_BGR2RGB);
                progress->setImg(QPixmap::fromImage(QImage((unsigned char*) new_frame.data, new_frame.cols, new_frame.rows, new_frame.step, QImage::Format_RGB888)));
            }
            else {
                video.write(frame);
                cvtColor(frame, frame, CV_BGR2RGB);
                progress->setImg(QPixmap::fromImage(QImage((unsigned char*) frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888)));
            }
            progress->setProgressBarValue(i+1);
            QCoreApplication::processEvents();
            if (i != vidData[2].size()-1) cap >> frame;
        }
    }
    else if (decision == 6){
        int speed = (int)alpha;
        int count = 0;
        for (unsigned int i = 0; i < vidData[2].size(); i++)
        {
            if (frame.empty()) {
                return "A blank video frame was encountered during the video writing process. Video stream could not be decoded.";
            }
            if ((int)vidData[2][i] == 1 || (int)vidData[3][i] == 1)
            {
                for (int j = 0; j < speed; j++) {
                    video.write(frame);
                    cvtColor(frame, frame, CV_BGR2RGB);
                    progress->setImg(QPixmap::fromImage(QImage((unsigned char*) frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888)));
                    QCoreApplication::processEvents();
                    progress->setProgressBarValue(count+1);
                    count++;
                }
            }
            else {
                video.write(frame);
                cvtColor(frame, frame, CV_BGR2RGB);
                progress->setImg(QPixmap::fromImage(QImage((unsigned char*) frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888)));
                progress->setProgressBarValue(count+1);
            }
            QCoreApplication::processEvents();
            if (i != vidData[2].size()-1) cap >> frame;
        }
    }
    else if (decision == 7){
        for (unsigned int i = 0; i < vidData[2].size(); i++)
        {
            if (frame.empty()) {
                return "A blank video frame was encountered during the video writing process. Video stream could not be decoded.";
            }
            if ((int)vidData[2][i] != 1 && (int)vidData[3][i] != 1)
            {
                video.write(frame);
                cvtColor(frame, frame, CV_BGR2RGB);
                progress->setImg(QPixmap::fromImage(QImage((unsigned char*) frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888)));
                progress->setProgressBarValue(i+1);
                QCoreApplication::processEvents();
            }
            if (i != vidData[2].size()-1) cap >> frame;
        }
    }
    progress->close();
    cap.release();
    video.release();

    return "Success";
}
