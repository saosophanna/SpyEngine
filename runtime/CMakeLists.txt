file(GLOB_RECURSE SPY_RUNTIME_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(SpyRuntime ${SPY_RUNTIME_SOURCES})
spy_target_defaults(SpyRuntime)

target_link_libraries(SpyRuntime PRIVATE SpyEngine::Engine)
target_include_directories(SpyRuntime PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
