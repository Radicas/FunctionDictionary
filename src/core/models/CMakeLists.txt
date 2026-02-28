add_library(core_models INTERFACE)
target_include_directories(core_models INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(core_models INTERFACE Qt${QT_VERSION_MAJOR}::Core)
