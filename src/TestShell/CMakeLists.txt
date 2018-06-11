cmake_minimum_required(VERSION 2.8)

project(TestShell)
#set(CMAKE_CXX_STANDARD 11)
add_definitions(-std=c++11)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set (PROJECTNAME_ARCHITECTURE "x86")
# Tell CMake to run moc when necessary:
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
#set(AUTORCC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package (Qt5Core)
find_package (Qt5Widgets)
find_package (Qt5Xml)
include_directories(${Qt5Widgets_INCLUDE_DIRS} )
include_directories(${Qt5Core_INCLUDE_DIRS} )
include_directories(${Qt5Xml_INCLUDE_DIRS} )
include_directories(./../../src )

set(SOURCE main.cpp
        mainwindow.cpp
    	TaskContainer.cpp)

set(HEADERS mainwindow.h TaskContainer.h)

set(FORMS mainwindow.ui)
QT5_WRAP_CPP(HEADERS_MOC ${HEADERS})
QT5_WRAP_UI(HEADERS_FORM ${FORMS})

set(MY_LIBS_DIR ${CMAKE_BINARY_DIR}/lib)
link_directories( ${MY_LIBS_DIR} )
set(LIBS_NAMES AdvThreadPool)

set(RESOURCE ${CMAKE_SOURCE_DIR}/AdvThreadPool/images.qrc)
qt5_add_resources(RESOURCE_ADDED ${RESOURCE})

add_executable(TestShell ${SOURCE} ${HEADERS} ${HEADERS_FORM} ${HEADERS_MOC} ${RESOURCE_ADDED})
target_link_libraries(TestShell ${LIBS_NAMES} ${Qt5Core_QTMAIN_LIBRARIES}
                                            ${Qt5Widgets_LIBRARIES}
                                            ${Qt5Xml_LIBRARIES})
