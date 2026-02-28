add_library(core_ai STATIC
    aiconfigmanager.h
    AIConfigManager.cpp
    aiservicemanager.h
    AIServiceManager.cpp
)

target_include_directories(core_ai PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(core_ai
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    core_models
    common_logger
)
