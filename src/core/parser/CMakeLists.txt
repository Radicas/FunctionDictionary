add_library(core_parser STATIC
    functionparser.h
    FunctionParser.cpp
    aicodeparser.h
    aicodeparser.cpp
    batchcodeparser.h
    batchcodeparser.cpp
)

target_include_directories(core_parser
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core_parser
    PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    core_models
    core_ai
    core_database
    common_logger
)
