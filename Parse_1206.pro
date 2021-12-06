#-------------------------------------------------
#
# Project created by QtCreator 2021-12-06T08:51:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Parse_1206
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    xinshengparse.cpp \
    qaesencryption.cpp

HEADERS  += widget.h \
    xinsheng.h \
    xinshengparse.h \
    qaesencryption.h

FORMS    += widget.ui

CONFIG += C++11
