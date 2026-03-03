add_library(core_parser STATIC
    functionparser.h
    FunctionParser.cpp
)

target_include_directories(core_parser
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core_parser
    PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    core_models
    common_logger
)
