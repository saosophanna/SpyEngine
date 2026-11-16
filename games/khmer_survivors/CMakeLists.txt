file(GLOB_RECURSE SPY_GAME_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(khmer_survivors ${SPY_GAME_SOURCES})
spy_target_defaults(khmer_survivors)

target_link_libraries(khmer_survivors PRIVATE SpyEngine::Engine)
target_include_directories(khmer_survivors PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_custom_command(TARGET khmer_survivors POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:khmer_survivors>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/config"
            "$<TARGET_FILE_DIR:khmer_survivors>/config")
