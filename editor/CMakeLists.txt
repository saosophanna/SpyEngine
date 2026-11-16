file(GLOB_RECURSE SPY_EDITOR_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(SpyEditor ${SPY_EDITOR_SOURCES})
spy_target_defaults(SpyEditor)

target_link_libraries(SpyEditor PRIVATE SpyEngine::Engine)
target_include_directories(SpyEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
