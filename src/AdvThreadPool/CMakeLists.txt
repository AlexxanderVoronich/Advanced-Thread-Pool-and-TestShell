cmake_minimum_required(VERSION 2.8)

project(AdvThreadPool)
#set(CMAKE_CXX_STANDARD 11)
add_definitions(-std=c++11)
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")

#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
#set(AUTORCC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package (Qt5Widgets)
find_package (Qt5Core)
find_package (Qt5Xml)
include_directories(${Qt5Widgets_INCLUDE_DIRS} )
include_directories(${Qt5Core_INCLUDE_DIRS} )
include_directories(${Qt5Xml_INCLUDE_DIRS} )

set(SOURCE AdvThreadPool.cpp
            AdvPoolGUI.cpp
            AdvThread.cpp
            CheckCoreWidget.cpp
            ThreadPoolSettings.cpp
            QObjectSerializer.cpp
            WarningJournal.cpp)

set(HEADERS Runnable.h
            AdvThreadPool.h
            AdvPoolGUI.h
            AdvThread.h
            CheckCoreWidget.h
            AdvMacros.h
            ThreadPoolSettings.h
            ServiceStructures.h
            AdvPoolEmitter.h
            QObjectSerializer.h
            WarningJournal.h)

set(FORMS forms/threadpoolshell.ui)
QT5_WRAP_CPP(HEADERS_MOC ${HEADERS})
QT5_WRAP_UI(HEADERS_FORM ${FORMS})

add_library(AdvThreadPool STATIC ${SOURCE} ${HEADERS} ${HEADERS_FORM} ${HEADERS_MOC})
