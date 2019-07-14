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
    c_label.cpp

HEADERS += \
    mainwindow.h \
    rkbcore.h \
    waitdialog.h \
    qcustomplot.h \
    rkbprotool.h \
    mainframe.h \
    c_label.h

FORMS += \
        mainwindow.ui \
    waitdialog.ui \
    rkbprotool.ui \
    mainframe.ui

RESOURCES += \
    resources.qrc

QMAKE_CXXFLAGS += -D__STDC_CONSTANT_MACROS

INCLUDEPATH += C:\opencv-build\install\include

LIBS += -L C:\\opencv-build\\bin \
    libopencv_core331 \
    libopencv_highgui331 \
    libopencv_imgproc331 \
    libopencv_features2d331 \
    libopencv_calib3d331 \
    libopencv_videoio331 \
    libopencv_imgcodecs331 \
    libopencv_ffmpeg331 \

DISTFILES += \
    README
