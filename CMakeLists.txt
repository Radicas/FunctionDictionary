cmake_minimum_required(VERSION 3.16)

project(FunctionDB VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 支持Qt5和Qt6
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Sql Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Sql Network)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/AddFunctionDialog.cpp
    src/DatabaseManager.cpp
    src/Logger.cpp
    src/function_dict_c_api.cpp
    src/AIConfigManager.cpp
    src/AIConfigDialog.cpp
    src/AIServiceManager.cpp
)

set(HEADERS
    src/mainwindow.h
    src/addfunctiondialog.h
    src/databasemanager.h
    src/logger.h
    src/function_dict_c_api.h
    src/aiconfigmanager.h
    src/aiconfigdialog.h
    src/aiservicemanager.h
)

add_executable(FunctionDB ${SOURCES} ${HEADERS})

target_link_libraries(FunctionDB Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Network)

# Windows MSVC特定配置
if(MSVC)
    # 使用UTF-8编码
    target_compile_options(FunctionDB PRIVATE /utf-8)
    
    # 设置Windows子系统为GUI（而非控制台）
    set_target_properties(FunctionDB PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # 避免使用min和max宏定义
    target_compile_definitions(FunctionDB PRIVATE NOMINMAX)
    
    # 警告级别
    target_compile_options(FunctionDB PRIVATE /W4)
endif()

if(APPLE)
    set_target_properties(FunctionDB PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "FunctionDB"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
endif()
