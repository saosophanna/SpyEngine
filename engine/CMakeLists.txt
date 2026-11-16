set(SPY_ENGINE_MODULES
    core/application
    core/lifecycle
    core/time
    core/events
    core/memory
    core/threading
    core/math
    core/utilities
    platform
    world
    ecs
    physics
    animation
    audio
    ui
    assets
    scripting)

set(SPY_ENGINE_SOURCES "")
foreach(module IN LISTS SPY_ENGINE_MODULES)
    file(GLOB_RECURSE _module_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/${module}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/${module}/*.c")
    list(APPEND SPY_ENGINE_SOURCES ${_module_sources})
endforeach()

add_subdirectory(core/logging)
add_subdirectory(renderer)

spy_add_library(SpyEngine ${SPY_ENGINE_SOURCES})
add_library(SpyEngine::Engine ALIAS SpyEngine)

target_include_directories(SpyEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(SpyEngine PUBLIC ${SPY_ENGINE_DEPS} SpyRenderer SpyLogging)

if(WIN32)
    target_link_libraries(SpyEngine PRIVATE user32)
endif()
