add_library(core_batch STATIC
    batchprocessmanager.h
    BatchProcessManager.cpp
)

target_include_directories(core_batch PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(core_batch
    Qt${QT_VERSION_MAJOR}::Core
    core_models
    core_parser
    core_ai
    core_database
    common_logger
)
