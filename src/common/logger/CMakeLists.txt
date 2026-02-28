add_library(common_logger STATIC
    logger.h
    Logger.cpp
)

target_include_directories(common_logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common_logger Qt${QT_VERSION_MAJOR}::Core)
