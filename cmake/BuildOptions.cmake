option(SPY_BUILD_RUNTIME      "Build the standalone game runtime"   ON)
option(SPY_BUILD_EDITOR       "Build the editor"                    ON)
option(SPY_BUILD_GAMES        "Build bundled game projects"         ON)
option(SPY_BUILD_TESTS        "Build unit tests"                    ON)
option(SPY_BUILD_TOOLS        "Build development tools"             ON)
option(SPY_WARNINGS_AS_ERRORS "Treat compiler warnings as errors"   OFF)

option(SPY_ENABLE_VULKAN      "Enable the Vulkan backend"            ON)
option(SPY_ENABLE_OPENGL      "Enable the OpenGL backend"            OFF)
option(SPY_ENABLE_VALIDATION  "Enable Vulkan validation layers"      ON)

set(SPY_ARCH "" CACHE STRING "Target architecture (x64, arm64)")
set_property(CACHE SPY_ARCH PROPERTY STRINGS "" x64 arm64)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
