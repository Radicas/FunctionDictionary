cmake_minimum_required(VERSION 3.16)

project(FunctionDB VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Sql)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/addfunctiondialog.cpp
    src/databasemanager.cpp
    src/logger.cpp
)

set(HEADERS
    src/mainwindow.h
    src/addfunctiondialog.h
    src/databasemanager.h
    src/logger.h
)

add_executable(FunctionDB ${SOURCES} ${HEADERS})

target_link_libraries(FunctionDB Qt5::Core Qt5::Widgets Qt5::Sql)

if(APPLE)
    set_target_properties(FunctionDB PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "FunctionDB"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
endif()
