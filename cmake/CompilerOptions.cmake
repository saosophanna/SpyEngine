add_library(spy_compiler_options INTERFACE)

if(MSVC)
    target_compile_options(spy_compiler_options INTERFACE
        /W4
        /permissive-
        /Zc:__cplusplus
        /Zc:preprocessor
        /utf-8
        /MP)
    target_compile_definitions(spy_compiler_options INTERFACE _CRT_SECURE_NO_WARNINGS)

    if(SPY_WARNINGS_AS_ERRORS)
        target_compile_options(spy_compiler_options INTERFACE /WX)
    endif()
else()
    target_compile_options(spy_compiler_options INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wno-unused-parameter)

    if(SPY_WARNINGS_AS_ERRORS)
        target_compile_options(spy_compiler_options INTERFACE -Werror)
    endif()
endif()

function(spy_target_defaults target)
    get_target_property(_type ${target} TYPE)
    if(_type STREQUAL "INTERFACE_LIBRARY")
        target_link_libraries(${target} INTERFACE spy_compiler_options spy_platform)
    else()
        target_link_libraries(${target} PRIVATE spy_compiler_options spy_platform)
        set_target_properties(${target} PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF)
    endif()
endfunction()

function(spy_add_library target)
    if(ARGC GREATER 1)
        add_library(${target} STATIC ${ARGN})
    else()
        add_library(${target} INTERFACE)
    endif()
    spy_target_defaults(${target})
endfunction()
