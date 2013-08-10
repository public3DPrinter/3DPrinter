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
    dlgprintprep.cpp

HEADERS  += mainwindow.h \
    crushbitmap.h \
    b9print.h \
    b9terminal.h \
    b9projector.h \
    dlgprintprep.h

FORMS    += mainwindow.ui \
    b9print.ui \
    b9terminal.ui \
    dlgprintprep.ui
