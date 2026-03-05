add_library(ui_dialogs_addprojectdialog STATIC
    addprojectdialog.h
    addprojectdialog.cpp
)

target_include_directories(ui_dialogs_addprojectdialog
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ui_dialogs_addprojectdialog
    PUBLIC
    Qt${QT_VERSION_MAJOR}::Widgets
    core_models
    common_logger
)
