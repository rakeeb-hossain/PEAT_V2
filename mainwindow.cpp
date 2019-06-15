#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScrollArea>
#include <QScrollBar>

#include <string>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>

// PEAT Info
using namespace std;
using namespace cv;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //QPixmap pix("C:/Users/rakee/Documents/PEAT/Res/TraceLogo.bmp");
    //ui->label->setPixmap(pix);
    this->setWindowTitle("UW Trace Center Photosensitive Epilepsy Analysis Tool (PEAT)");
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    this->setFixedSize(QSize(387,519));

    QLabel *p1 = new QLabel("Trace Center Photosensitive Epilepsy Analysis Tool (PEAT)\nVersion 2.0\n\nThis flash analysis tool is being provided free of charge by the\nTrace R&D Center.  It is provided to facilitate the analysis of web\nand computer content to help identify those materials which are a\nrisk for causing seizures in individuals with photosensitive seizure\ndisorders including epilepsy.  This tool is based on and uses the\nanalysis engine from the Harding Flash and Pattern Analysis Tool\nfor television programming developed by Cambridge Research\nSystems in conjunction with Dr. Graham Harding.  In this Trace\nCenter tool the underlying engine has been adapted by the Trace\nCenter in conjunction with Dr. Harding and Cambridge Research\nSystems so that it is applicable to computer software and web\ncontent.\n\nThis tool is designed to provide a software and web site analysis\ntool for use by web and computer software designers, evaluators\nand users to test for sequences considered provocative to\nphotosensitive individuals.  This tool may be used freely for these\npurposes.  This tool may NOT be used to assess material\ncommercially produced for the television broadcast, film, or home\nentertainment or gaming industries.  Individuals interested in a\ntool for testing these applications should contact Cambridge\nResearch Systems at www.hardingfpa.co.uk.\n\nThe Photosensitive Epilepsy Analysis Tool (PEAT) is copyright\nTrace R&D Center.  The PSEe engine is copyright Cambridge\nResearch Systems Ltd.\n\nFunding for the development of this tool was provided by the\nNational Institute on Disability and Rehabilitation Research, US\nDept. of Education under grant numbers H133E980008,\nH133E030012, and H133E080022.\n");
    ui->scrollArea->setWidget(p1);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    hide();
    mainframe = new mainFrame(this);
    mainframe->show();
}

void MainWindow::on_pushButton_2_clicked()
{
    QApplication::quit();
}

