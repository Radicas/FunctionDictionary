add_library(core_models STATIC
    functiondata.h
    extractedfunction.h
    aiconfig.h
    batchconfig.h
    processerror.h
    errorhandler.h
    ErrorHandler.cpp
    ratelimiter.h
    RateLimiter.cpp
    concurrencycontroller.h
    ConcurrencyController.cpp
    parseresult.h
)

target_include_directories(core_models
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core_models
    PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    common_logger
)
