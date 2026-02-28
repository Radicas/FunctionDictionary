cmake_minimum_required(VERSION 3.16)

project(FunctionDictionary VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Sql Network Concurrent)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

set(PROJECT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PROJECT_COMMON_DIR ${PROJECT_SRC_DIR}/common)
set(PROJECT_CORE_DIR ${PROJECT_SRC_DIR}/core)
set(PROJECT_UI_DIR ${PROJECT_SRC_DIR}/ui)

if(MSVC)
    add_compile_options(/utf-8)
    add_definitions(-D_UNICODE -DUNICODE)
endif()

add_subdirectory(src)
