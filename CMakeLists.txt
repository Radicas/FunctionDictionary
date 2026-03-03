cmake_minimum_required(VERSION 3.16)

project(FunctionDictionary VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Widgets 
    Sql 
    Network 
    Concurrent
    Positioning
    Quick
    Qml
    WebChannel
)

# WebEngine模块单独查找（Qt 6.10.2+ WebEngine作为插件分发）
# 需要先找到依赖模块：Positioning, Quick, Qml, WebChannel
# 跨平台支持：Windows/macOS/Linux
find_package(Qt6WebEngineWidgets QUIET)
if(NOT Qt6WebEngineWidgets_FOUND)
    # 尝试在Qt安装目录的lib/cmake下查找
    if(WIN32)
        set(Qt6WebEngineWidgets_DIR "${Qt6_DIR}/../Qt6WebEngineWidgets")
    elseif(APPLE)
        set(Qt6WebEngineWidgets_DIR "${Qt6_DIR}/../Qt6WebEngineWidgets")
    elseif(UNIX)
        set(Qt6WebEngineWidgets_DIR "${Qt6_DIR}/../Qt6WebEngineWidgets")
    endif()
    find_package(Qt6WebEngineWidgets REQUIRED)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

set(PROJECT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PROJECT_COMMON_DIR ${PROJECT_SRC_DIR}/common)
set(PROJECT_CORE_DIR ${PROJECT_SRC_DIR}/core)
set(PROJECT_UI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/ui)

if(MSVC)
    add_compile_options(/utf-8)
    add_definitions(-D_UNICODE -DUNICODE)
endif()

add_subdirectory(src)
