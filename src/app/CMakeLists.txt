add_executable(FunctionDB
    main.cpp
)

target_link_libraries(FunctionDB
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
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
    
    get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
    
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET FunctionDB POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Running windeployqt..."
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-translations
                --no-system-d3d-compiler
                --no-angle
                --no-opengl-sw
                "$<TARGET_FILE:FunctionDB>"
            COMMENT "Deploying Qt libraries and plugins for Windows"
        )
    else()
        message(WARNING "windeployqt not found. You may need to manually deploy Qt plugins.")
    endif()
endif()

if(APPLE)
    set_target_properties(FunctionDB PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "FunctionDB"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
endif()
