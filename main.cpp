//
//  main.cpp
//  PEAT_V2
//
//  Created by Hossain, Rakeeb on 2018-01-02.
//  Copyright © 2018 Hossain, Rakeeb. All rights reserved.
//
#include "mainwindow.h"
#include <QApplication>

/*
extern "C"
    {
        #include <libavformat/avformat.h>
        #include <libavcodec/avcodec.h>
        #include <libavutil/avutil.h>
    }
*/
int main(int argc, char *argv[])
{
    qputenv("QT_SCALE_FACTOR", "1");
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "0");
    qputenv("QT_SCREEN_SCALE_FACTORS", "1");
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);

    //av_register_all();

    a.setWindowIcon(QIcon(":/images/Res/PEAT.ico"));

    QApplication::setQuitOnLastWindowClosed(false);

    qRegisterMetaType<vector<QVector<double > >>("vector<QVector<double > >");
    qRegisterMetaType<int>("int");

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect geometry = screen->availableGeometry();
    if (geometry.height() < 650 || geometry.width() < 1000) {
        QMessageBox::information(0, "Screen Resolution", "Please adjust your screen resolution to at least a height of 650 and width of 1000");
        exit(0);
    }
    //qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    MainWindow w;
    w.show();

    return a.exec();
}
