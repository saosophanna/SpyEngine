set(SPY_RENDERER_SOURCES "")

file(GLOB _renderer_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
list(APPEND SPY_RENDERER_SOURCES ${_renderer_sources})

foreach(module IN ITEMS rhi resources materials sprites camera lighting batching postprocess)
    file(GLOB_RECURSE _module_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/${module}/*.cpp")
    list(APPEND SPY_RENDERER_SOURCES ${_module_sources})
endforeach()

if(SPY_ENABLE_VULKAN AND Vulkan_FOUND)
    file(GLOB_RECURSE _vulkan_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/vulkan/*.cpp")
    list(APPEND SPY_RENDERER_SOURCES ${_vulkan_sources})
endif()

if(SPY_ENABLE_OPENGL)
    file(GLOB_RECURSE _opengl_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/opengl/*.cpp")
    list(APPEND SPY_RENDERER_SOURCES ${_opengl_sources})
endif()

spy_add_library(SpyRenderer ${SPY_RENDERER_SOURCES})
add_library(SpyEngine::Renderer ALIAS SpyRenderer)

target_include_directories(SpyRenderer PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/engine")

target_link_libraries(SpyRenderer PRIVATE SpyLogging)

if(SPY_ENABLE_VULKAN AND Vulkan_FOUND)
    target_link_libraries(SpyRenderer PUBLIC Vulkan::Vulkan)
    target_compile_definitions(SpyRenderer PUBLIC SPY_RENDERER_VULKAN=1)

    if(WIN32)
        target_compile_definitions(SpyRenderer PRIVATE VK_USE_PLATFORM_WIN32_KHR)
    endif()
endif()

if(SPY_ENABLE_OPENGL)
    target_compile_definitions(SpyRenderer PUBLIC SPY_RENDERER_OPENGL=1)
endif()
