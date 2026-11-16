cmake_minimum_required(VERSION 3.24)

project(SpyEngine
    VERSION 0.1.0
    DESCRIPTION "SpyEngine - a cross-platform 2D/3D game engine"
    LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(BuildOptions)
include(Platform)
include(CompilerOptions)
include(Dependencies)

add_subdirectory(engine)

if(SPY_BUILD_RUNTIME)
    add_subdirectory(runtime)
endif()

if(SPY_BUILD_EDITOR)
    add_subdirectory(editor)
endif()

if(SPY_BUILD_GAMES)
    add_subdirectory(games/khmer_survivors)
endif()

if(SPY_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
