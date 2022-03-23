TEMPLATE = lib
TARGET = AdvThreadPool
MOC_DIR += moc
UI_DIR += ui
OBJECTS_DIR = ../../obj
INCLUDEPATH += . ./ui ./moc

DESTDIR  = ../../lib

QT     += core widgets xml

#CONFIG += staticlib
CONFIG	+= qt warn_on debug c++17

#DEFINES += BUILD_STATIC
DEFINES += BUILD_DLL

FORMS += forms/threadpoolshell.ui

# Input
HEADERS +=  Runnable.h \
            AdvThreadPool.h \
            AdvPoolGUI.h \
            AdvThread.h \
            CheckCoreWidget.h \
            AdvMacros.h \
            ThreadPoolSettings.h \
            ServiceStructures.h \
            AdvPoolEmitter.h \
            QObjectSerializer.h \
            WarningJournal.h

SOURCES +=  AdvThreadPool.cpp \
            AdvPoolGUI.cpp \
            AdvThread.cpp \
            CheckCoreWidget.cpp \
            ThreadPoolSettings.cpp \
            QObjectSerializer.cpp \
            WarningJournal.cpp

#RESOURCES += images.qrc
