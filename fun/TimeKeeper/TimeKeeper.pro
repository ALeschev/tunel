#-------------------------------------------------
#
# Project created by QtCreator 2018-07-25T20:16:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TimeKeeper
TEMPLATE = app


SOURCES += main.cpp\
        timekeeper.cpp \
    task.cpp

HEADERS  += timekeeper.h \
    task.h

FORMS    += timekeeper.ui
