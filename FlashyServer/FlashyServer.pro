#-------------------------------------------------
#
# Project created by QtCreator 2018-04-07T19:05:51
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FlashyServer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    server.cpp \
    network.cpp \
    messagebuffer.cpp \
    readevents.cpp \
    multimanager.cpp \
    session.cpp \
    datamanager.cpp \
    encryption.cpp

HEADERS  += mainwindow.h \
    server.h \
    network.h \
    messagebuffer.h \
    readevents.h \
    multimanager.h \
    multilogic.h \
    session.h \
    datamanager.h \
    outgoingqueue.h \
    encryption.h

FORMS    += mainwindow.ui
