#-------------------------------------------------
#
# Project created by QtCreator 2018-04-25T14:48:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FlashyTester
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    network.cpp \
    multimanager.cpp \
    server.cpp \
    encryption.cpp \
    readevents.cpp \
    messagebuffer.cpp

HEADERS  += mainwindow.h \
    network.h \
    multimanager.h \
    server.h \
    encryption.h \
    readevents.h \
    messagebuffer.h

FORMS    += mainwindow.ui
