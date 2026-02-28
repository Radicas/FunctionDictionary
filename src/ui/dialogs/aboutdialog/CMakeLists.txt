# AboutDialog CMakeLists.txt

set(ABOUT_DIALOG_SOURCES
    AboutDialog.cpp
)

set(ABOUT_DIALOG_HEADERS
    aboutdialog.h
)

add_library(ui_dialogs_aboutdialog STATIC
    ${ABOUT_DIALOG_SOURCES}
    ${ABOUT_DIALOG_HEADERS}
)

target_include_directories(ui_dialogs_aboutdialog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ui_dialogs_aboutdialog Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets common_utils)
