add_library(ui_mainwindow STATIC
    mainwindow.h
    mainwindow.cpp
    functionalitywidget.h
    functionalitywidget.cpp
    widgets/fileselectorwidget.h
    widgets/fileselectorwidget.cpp
    widgets/progresswidget.h
    widgets/progresswidget.cpp
    widgets/parsecontrolwidget.h
    widgets/parsecontrolwidget.cpp
)

target_include_directories(ui_mainwindow
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
)

target_link_libraries(ui_mainwindow
    PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::WebEngineWidgets
    core_ai
    core_database
    core_services
    ui_dialogs_aiconfigdialog
    ui_dialogs_aboutdialog
    ui_markdown
    common_logger
)
