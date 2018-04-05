#-------------------------------------------------
#
# Project created by QtCreator 2018-03-12T14:07:01
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FlashyCard
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    datamanager.cpp \
    deckmanager.cpp \
    card.cpp \
    koreancard.cpp \
    studysession.cpp \
    cardlist.cpp \
    studyclock.cpp \
    grammarpuller.cpp \
    statinfo.cpp \
    graph.cpp

HEADERS  += mainwindow.h \
    datamanager.h \
    deckmanager.h \
    globalincludes.h \
    card.h \
    koreancard.h \
    studysession.h \
    cardlist.h \
    studyclock.h \
    grammarpuller.h \
    statinfo.h \
    graph.h

FORMS    += mainwindow.ui \
    graph.ui
