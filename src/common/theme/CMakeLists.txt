add_library(common_theme STATIC
    ThemeManager.cpp
    thememanager.h
    theme.qrc
)

target_include_directories(common_theme
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(common_theme
    PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    common_logger
)
