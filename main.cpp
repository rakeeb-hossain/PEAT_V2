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

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect geometry = screen->availableGeometry();
    if (geometry.height() < 650 || geometry.width() < 1000) {
        QMessageBox::information(0, "Screen Resolution", "Please adjust your screen resolution to at least a height of 650 and width of 1000");
        exit(0);
    }

    MainWindow w;

    w.show();

    return a.exec();
}
