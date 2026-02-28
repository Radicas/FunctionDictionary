add_library(common_utils STATIC
)

target_include_directories(common_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common_utils Qt${QT_VERSION_MAJOR}::Core)
