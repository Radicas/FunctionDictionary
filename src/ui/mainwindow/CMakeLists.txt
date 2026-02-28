add_library(ui_mainwindow STATIC
    mainwindow.h
    MainWindow.cpp
)

target_include_directories(ui_mainwindow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_SOURCE_DIR}/../../common/logger)
target_link_libraries(ui_mainwindow
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    core_database
    ui_dialogs_aiconfigdialog
    common_logger
)
