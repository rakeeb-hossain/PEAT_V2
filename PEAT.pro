#-------------------------------------------------
#
# Project created by QtCreator 2018-06-15T17:07:00
#
#-------------------------------------------------

QT       += core gui multimedia multimediawidgets printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PEAT
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
    mainwindow.cpp \
    rkbcore.cpp \
    waitdialog.cpp \
    qcustomplot.cpp \
    rkbprotool.cpp \
    mainframe.cpp

HEADERS += \
    mainwindow.h \
    rkbcore.h \
    waitdialog.h \
    qcustomplot.h \
    rkbprotool.h \
    mainframe.h

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
