#-------------------------------------------------
#
# Project created by QtCreator 2013-08-06T19:04:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Projector_part
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    crushbitmap.cpp \
    b9print.cpp \
    b9terminal.cpp \
    b9projector.cpp \
    dlgprintprep.cpp \
    b9printercomm.cpp

include(qextserialport-1.2beta2/src/qextserialport.pri)

HEADERS  += mainwindow.h \
    crushbitmap.h \
    b9print.h \
    b9terminal.h \
    b9projector.h \
    dlgprintprep.h \
    b9printercomm.h

FORMS    += mainwindow.ui \
    b9print.ui \
    b9terminal.ui \
    dlgprintprep.ui
