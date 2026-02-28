add_library(api STATIC
    function_dict_c_api.h
    function_dict_c_api.cpp
)

target_include_directories(api PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(api
    Qt${QT_VERSION_MAJOR}::Core
    core_database
    common_logger
)
