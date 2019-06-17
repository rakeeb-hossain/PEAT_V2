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
    QApplication a(argc, argv);

    //av_register_all();

    a.setWindowIcon(QIcon(":/images/Res/PEAT.ico"));

    QApplication::setQuitOnLastWindowClosed(false);

    qRegisterMetaType<vector<QVector<double > >>("vector<QVector<double > >");
    qRegisterMetaType<int>("int");

    MainWindow w;

    w.show();

    return a.exec();
}
