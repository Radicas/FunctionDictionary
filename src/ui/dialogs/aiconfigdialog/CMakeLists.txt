add_library(ui_dialogs_aiconfigdialog STATIC
    aiconfigdialog.h
    AIConfigDialog.cpp
)

target_include_directories(ui_dialogs_aiconfigdialog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ui_dialogs_aiconfigdialog
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    core_ai
    common_logger
)
