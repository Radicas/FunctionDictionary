add_library(ui_dialogs_aboutdialog STATIC
    AboutDialog.cpp
    aboutdialog.h
)

target_include_directories(ui_dialogs_aboutdialog
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ui_dialogs_aboutdialog
    PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    common_utils
)
