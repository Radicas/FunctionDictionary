add_library(ui_dialogs_addfunctiondialog STATIC
    addfunctiondialog.h
    AddFunctionDialog.cpp
)

target_include_directories(ui_dialogs_addfunctiondialog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ui_dialogs_addfunctiondialog
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    common_logger
)
