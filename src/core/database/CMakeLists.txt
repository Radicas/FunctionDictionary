add_library(core_database STATIC
    databasemanager.h
    DatabaseManager.cpp
)

target_include_directories(core_database PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(core_database
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql
    core_models
    common_logger
)
