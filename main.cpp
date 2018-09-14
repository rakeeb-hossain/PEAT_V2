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

    a.setWindowIcon(QIcon(":/Res/PEAT.ico"));

    QApplication::setQuitOnLastWindowClosed(false);

    MainWindow w;

    w.show();

    return a.exec();
}
