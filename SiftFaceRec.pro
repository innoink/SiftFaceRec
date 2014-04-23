#-------------------------------------------------
#
# Project created by QtCreator 2014-04-15T15:53:56
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SiftFaceRec
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
    ui/fancy_widget/fancytabwidget.cpp \
    ui/fancy_widget/styledbar.cpp \
    ui/fancy_widget/stylehelper.cpp \
    ui/main_window.cpp \
    ui/camera_widget.cpp \
    utils/rqueue.c \
    utils/img_fmt_cvt.cpp \
    face_detector/face_detector.cpp \
    ui/train_widget.cpp \
    face_recognizer/face_recognizer.cpp \
    ui/sift_demo_widget.cpp \
    face_collection/face_collection.cpp \
    ui/collection_widget.cpp

HEADERS  += \
    ui/fancy_widget/fancytabwidget.h \
    ui/fancy_widget/styledbar.h \
    ui/fancy_widget/stylehelper.h \
    ui/main_window.h \
    ui/camera_widget.h \
    utils/rqueue.h \
    utils/img_fmt_cvt.h \
    face_detector/face_detector.h \
    ui/train_widget.h \
    face_recognizer/face_recognizer.h \
    ui/sift_demo_widget.h \
    face_collection/face_collection.h \
    ui/collection_widget.h

unix{
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv
    LIBS += -lvl -lunqlite
}
win32{
    INCLUDEPATH += "C:/opencv/opencv/build/include/opencv" \
                   "C:/opencv/opencv/build/include"

    LIBS += "C:/opencv/lib/libopencv_core241.dll.a" \
            "C:/opencv/lib/libopencv_features2d241.dll.a" \
            "C:/opencv/lib/libopencv_highgui241.dll.a" \
            "C:/opencv/lib/libopencv_video241.dll.a" \
            "C:/opencv/lib/libopencv_imgproc241.dll.a"
}
