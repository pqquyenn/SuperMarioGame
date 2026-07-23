cmake_minimum_required(VERSION 3.28)
project(SFML-Mario LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable(mario

)

target_compile_features(mario PRIVATE cxx_std_17)
target_link_libraries(mario PRIVATE SFML::Graphics SFML::Audio)

target_include_directories(mario
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/json/include
)


add_custom_command(TARGET mario POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
)
