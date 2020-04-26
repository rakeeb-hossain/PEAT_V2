#-------------------------------------------------
#
# Project created by QtCreator 2018-06-15T17:07:00
#
#-------------------------------------------------

QT       += core gui multimedia multimediawidgets printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PEAT
TEMPLATE = app

QT += opengl
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += resources_big

SOURCES += \
        main.cpp \
    mainwindow.cpp \
    rkbcore.cpp \
    waitdialog.cpp \
    qcustomplot.cpp \
    rkbprotool.cpp \
    mainframe.cpp \
    c_label.cpp \
    colordialog.cpp \
    rkbaxisticker.cpp

HEADERS += \
    mainwindow.h \
    rkbcore.h \
    waitdialog.h \
    qcustomplot.h \
    rkbprotool.h \
    mainframe.h \
    c_label.h \
    colordialog.h \
    rkbaxisticker.h

FORMS += \
        mainwindow.ui \
    waitdialog.ui \
    rkbprotool.ui \
    mainframe.ui \
    colordialog.ui \
    colordialog.ui

RESOURCES += \
    resources.qrc

QMAKE_CXXFLAGS += -D__STDC_CONSTANT_MACROS

win32 {
    INCLUDEPATH += C:\opencv-4.1.1-build\install\include
    INCLUDEPATH += $$PWD\Lib

    LIBS += -L C:\\opencv-4.1.1-build\\bin \
        libopencv_core411 \
        libopencv_highgui411 \
        libopencv_imgproc411 \
        libopencv_features2d411 \
        libopencv_calib3d411 \
        libopencv_videoio411 \
        libopencv_imgcodecs411 \
        libopencv_videoio_ffmpeg411 \
}

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv4
}

DISTFILES += \
    README
