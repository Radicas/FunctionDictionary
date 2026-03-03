add_library(ui_markdown STATIC
    markdownview.h
    markdownview.cpp
    markdown.qrc
)

target_include_directories(ui_markdown
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ui_markdown
    PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::WebEngineWidgets
    common_logger
)
