file(GLOB_RECURSE SPY_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp")

if(NOT SPY_TEST_SOURCES)
    message(STATUS "SpyEngine: no tests found")
    return()
endif()

foreach(test_source IN LISTS SPY_TEST_SOURCES)
    get_filename_component(test_name ${test_source} NAME_WE)

    add_executable(${test_name} ${test_source})
    spy_target_defaults(${test_name})
    target_link_libraries(${test_name} PRIVATE SpyEngine::Engine)
    target_include_directories(${test_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_definitions(${test_name} PRIVATE
        SPY_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
        SPY_SHADER_DIR="${CMAKE_BINARY_DIR}/shaders")

    if(TARGET spy_shaders)
        add_dependencies(${test_name} spy_shaders)
    endif()

    if(test_name STREQUAL "fruit_spin_logic_test")
        target_sources(${test_name} PRIVATE
            "${CMAKE_SOURCE_DIR}/games/fruit_spin/src/gameplay/fruit_symbols.cpp"
            "${CMAKE_SOURCE_DIR}/games/fruit_spin/src/gameplay/game_state.cpp")
        target_include_directories(${test_name} PRIVATE
            "${CMAKE_SOURCE_DIR}/games/fruit_spin/src")
    endif()

    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
