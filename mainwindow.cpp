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
    this->setFixedSize(QSize(398,532));

    QLabel *p1 = new QLabel("Trace Center Photosensitive Epilepsy Analysis Tool (PEAT) Version 2.0\n\nThis flash analysis tool is being provided free of charge by the\nTrace R&D Center.  It is provided to facilitate the analysis of web and computer content to help identify those materials which are a risk for causing seizures in individuals with photosensitive seizure disorders including epilepsy. This tool is based on and uses the analysis engine from the Harding Flash and Pattern Analysis Tool for television programming developed by Cambridge Research Systems in conjunction with Dr. Graham Harding.  In this Trace Center tool the underlying engine has been adapted by the Trace Center in conjunction with Dr. Harding and Cambridge Research Systems so that it is applicable to computer software and web content.\n\nThis tool is designed to provide a software and web site analysis tool for use by web and computer software designers, evaluators and users to test for sequences considered provocative to photosensitive individuals.  This tool may be used freely for these purposes.  This tool may NOT be used to assess material commercially produced for the television broadcast, film, or home entertainment or gaming industries.  Individuals interested in a tool for testing these applications should contact Cambridge Research Systems at www.hardingfpa.co.uk.\n\nThe Photosensitive Epilepsy Analysis Tool (PEAT) is copyright Trace R&D Center.  The PSEe engine is copyright Cambridge Research Systems Ltd.\n\nFunding for the development of this tool was provided by the National Institute on Disability and Rehabilitation Research, US Dept. of Education under grant numbers H133E980008, H133E030012, and H133E080022.");
    p1->setWordWrap(true);
    p1->setMargin(5);
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

