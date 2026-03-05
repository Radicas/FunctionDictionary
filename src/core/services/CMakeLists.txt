add_library(core_services STATIC
    parseservice.h
    parseservice.cpp
)

target_include_directories(core_services
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core_services
    PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    core_interfaces
    core_parser
    core_database
    common_logger
)
