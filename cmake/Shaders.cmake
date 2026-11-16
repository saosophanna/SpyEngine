find_program(SPY_GLSLC_EXECUTABLE
    NAMES glslc
    HINTS "$ENV{VULKAN_SDK}/Bin")

function(spy_add_shader_target target)
    if(NOT SPY_GLSLC_EXECUTABLE)
        message(STATUS "SpyEngine: glslc not found - shaders will not be compiled")
        add_custom_target(${target})
        return()
    endif()

    set(shader_source_root "${CMAKE_SOURCE_DIR}/shaders/vulkan")
    set(shader_output_root "${CMAKE_BINARY_DIR}/shaders")

    file(GLOB_RECURSE shader_sources CONFIGURE_DEPENDS
        "${shader_source_root}/*.vert"
        "${shader_source_root}/*.frag")
    file(GLOB_RECURSE shader_includes CONFIGURE_DEPENDS
        "${shader_source_root}/*.glsl")

    set(shader_outputs "")

    foreach(shader_source IN LISTS shader_sources)
        file(RELATIVE_PATH shader_relative "${shader_source_root}" "${shader_source}")
        set(shader_output "${shader_output_root}/${shader_relative}.spv")
        get_filename_component(shader_output_dir "${shader_output}" DIRECTORY)

        add_custom_command(
            OUTPUT "${shader_output}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${shader_output_dir}"
            COMMAND "${SPY_GLSLC_EXECUTABLE}"
                    --target-env=vulkan1.2
                    -O
                    -o "${shader_output}"
                    "${shader_source}"
            DEPENDS "${shader_source}" ${shader_includes}
            COMMENT "Compiling ${shader_relative}")

        list(APPEND shader_outputs "${shader_output}")
    endforeach()

    add_custom_target(${target} DEPENDS ${shader_outputs})
endfunction()
