cmake_minimum_required(VERSION 3.28)
project(SFML-Mario LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Try finding installed SFML first, otherwise fetch it automatically
find_package(SFML 2.5 COMPONENTS graphics audio QUIET)

if(NOT SFML_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.1
    )
    set(SFML_BUILD_AUDIO ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_GRAPHICS ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_WINDOW ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SFML)
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

add_executable(mario ${SOURCES} ${HEADERS})

target_include_directories(mario
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if(TARGET sfml-graphics AND TARGET sfml-audio)
    target_link_libraries(mario PRIVATE sfml-graphics sfml-audio)
elseif(TARGET SFML::Graphics AND TARGET SFML::Audio)
    target_link_libraries(mario PRIVATE SFML::Graphics SFML::Audio)
else()
    target_link_libraries(mario PRIVATE sfml-graphics sfml-audio)
endif()

# Copy assets to executable folder post-build
add_custom_command(TARGET mario POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets"
    COMMENT "Copying assets to output directory"
)
