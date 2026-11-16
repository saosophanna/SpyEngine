if(WIN32)
    set(SPY_PLATFORM "windows" CACHE INTERNAL "")
elseif(ANDROID)
    set(SPY_PLATFORM "android" CACHE INTERNAL "")
elseif(APPLE)
    set(SPY_PLATFORM "macos" CACHE INTERNAL "")
elseif(UNIX)
    set(SPY_PLATFORM "linux" CACHE INTERNAL "")
else()
    message(FATAL_ERROR "SpyEngine: unsupported platform")
endif()

message(STATUS "SpyEngine platform: ${SPY_PLATFORM}")

string(TOUPPER "${SPY_PLATFORM}" SPY_PLATFORM_UPPER)

add_library(spy_platform INTERFACE)

target_compile_definitions(spy_platform INTERFACE
    SPY_PLATFORM_${SPY_PLATFORM_UPPER}=1
    $<$<CONFIG:Debug>:SPY_DEBUG=1>
    $<$<CONFIG:Release>:SPY_RELEASE=1>
    $<$<CONFIG:RelWithDebInfo>:SPY_RELEASE=1>)

if(SPY_PLATFORM STREQUAL "windows")
    target_compile_definitions(spy_platform INTERFACE
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
        NOMINMAX)
endif()
