#-------------------------------------------------
#
# Project created by QtCreator 2016-06-12T23:30:57
#
#-------------------------------------------------

QT       += core gui xml
CONFIG	+= qt warn_on console c++11
DESTDIR  = ../../bin

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ATPTestShell
TEMPLATE = app
DEPENDPATH += . debug forms
MOC_DIR += moc
UI_DIR += ui
OBJECTS_DIR = ../../obj

INCLUDEPATH += . ./ui ./moc ./..

SOURCES += main.cpp\
        mainwindow.cpp \
    TaskContainer.cpp

HEADERS  += mainwindow.h \
    TaskContainer.h

FORMS    += mainwindow.ui

RESOURCES += ../AdvThreadPool/images.qrc

LIBS += -L./../../lib -lAdvThreadPool
#LIBS += -L./../../lib/CHIEF -lboost_serialization
