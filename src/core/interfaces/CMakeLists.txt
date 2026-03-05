add_library(core_interfaces STATIC
    iparseservice.h
)

target_include_directories(core_interfaces
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core_interfaces
    PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    core_models
)
