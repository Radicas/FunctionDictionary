add_library(ui_mainwindow STATIC
    mainwindow.h
    MainWindow.cpp
    FunctionalityWidget.h
    FunctionalityWidget.cpp
)

target_include_directories(ui_mainwindow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ui_mainwindow
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    core_ai
    core_database
    ui_dialogs_aiconfigdialog
    ui_dialogs_aboutdialog
    common_logger
)
