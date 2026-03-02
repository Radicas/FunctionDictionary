add_library(theme
    STATIC
    ThemeManager.cpp
    thememanager.h
    resources.qrc
)

target_link_libraries(theme
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    common_logger
)

target_include_directories(theme
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(theme
    PRIVATE
    ${PROJECT_COMMON_DIR}/logger
)
