#-------------------------------------------------
#
# Project created by QtCreator 2013-08-12T20:09:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Comm_part
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    b9printercomm.cpp

include(qextserialport-1.2beta2/src/qextserialport.pri)

HEADERS  += mainwindow.h \
    b9printercomm.h

FORMS    += mainwindow.ui
