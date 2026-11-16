set(SPY_RENDERER_SOURCES "")

foreach(module IN ITEMS rhi resources materials sprites camera lighting batching postprocess)
    file(GLOB_RECURSE _src CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/${module}/*.cpp")
    list(APPEND SPY_RENDERER_SOURCES ${_src})
endforeach()

if(SPY_ENABLE_VULKAN)
    file(GLOB_RECURSE _vk CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/vulkan/*.cpp")
    list(APPEND SPY_RENDERER_SOURCES ${_vk})
endif()

if(SPY_ENABLE_OPENGL)
    file(GLOB_RECURSE _gl CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/opengl/*.cpp")
    list(APPEND SPY_RENDERER_SOURCES ${_gl})
endif()

spy_add_library(SpyRenderer ${SPY_RENDERER_SOURCES})
add_library(SpyEngine::Renderer ALIAS SpyRenderer)

target_include_directories(SpyRenderer PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/engine")

if(SPY_ENABLE_VULKAN AND Vulkan_FOUND)
    target_link_libraries(SpyRenderer PUBLIC Vulkan::Vulkan)
    target_compile_definitions(SpyRenderer PUBLIC SPY_RENDERER_VULKAN=1)
endif()

if(SPY_ENABLE_OPENGL)
    target_compile_definitions(SpyRenderer PUBLIC SPY_RENDERER_OPENGL=1)
endif()
