file(GLOB_RECURSE SPY_LOGGING_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

find_package(Threads REQUIRED)

spy_add_library(SpyLogging ${SPY_LOGGING_SOURCES})
add_library(SpyEngine::Logging ALIAS SpyLogging)

target_include_directories(SpyLogging PUBLIC "${CMAKE_SOURCE_DIR}/engine")
target_link_libraries(SpyLogging PUBLIC Threads::Threads)
