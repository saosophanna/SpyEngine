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

    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
