file(GLOB_RECURSE SPY_GAME_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(fruit_spin ${SPY_GAME_SOURCES})
spy_target_defaults(fruit_spin)

target_link_libraries(fruit_spin PRIVATE SpyEngine::Engine)
target_include_directories(fruit_spin PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_custom_command(TARGET fruit_spin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:fruit_spin>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/config"
            "$<TARGET_FILE_DIR:fruit_spin>/config")

if(TARGET spy_shaders)
    add_dependencies(fruit_spin spy_shaders)

    add_custom_command(TARGET fruit_spin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_BINARY_DIR}/shaders"
                "$<TARGET_FILE_DIR:fruit_spin>/shaders")
endif()
