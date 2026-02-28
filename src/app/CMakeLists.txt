add_executable(FunctionDB
    main.cpp
)

target_link_libraries(FunctionDB
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    ui_mainwindow
    ui_dialogs_addfunctiondialog
    core_database
    core_ai
    api
    common_logger
)

if(MSVC)
    target_compile_options(FunctionDB PRIVATE /utf-8)
    set_target_properties(FunctionDB PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    target_compile_definitions(FunctionDB PRIVATE NOMINMAX)
    target_compile_options(FunctionDB PRIVATE /W4)
endif()

if(APPLE)
    set_target_properties(FunctionDB PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "FunctionDB"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
endif()
