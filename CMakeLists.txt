cmake_minimum_required(VERSION 3.20)
project(SFML-Mario LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Try finding installed SFML first, otherwise fetch it automatically via FetchContent
find_package(SFML 2.5 COMPONENTS graphics audio window system QUIET)

if(NOT SFML_FOUND)
    set(SFML_BUILD_AUDIO ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_GRAPHICS ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_WINDOW ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
    if(EXISTS "${CMAKE_BINARY_DIR}/_deps/sfml-src/CMakeLists.txt")
        add_subdirectory("${CMAKE_BINARY_DIR}/_deps/sfml-src" "${CMAKE_BINARY_DIR}/_deps/sfml-build")
    else()
        include(FetchContent)
        FetchContent_Declare(
            SFML
            GIT_REPOSITORY https://github.com/SFML/SFML.git
            GIT_TAG 2.6.1
        )
        FetchContent_MakeAvailable(SFML)
    endif()
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h" 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_executable(mario ${SOURCES} ${HEADERS})

target_include_directories(mario
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(TARGET SFML::Graphics)
    target_link_libraries(mario PRIVATE SFML::Graphics SFML::Audio SFML::Window SFML::System)
elseif(TARGET sfml-graphics)
    target_link_libraries(mario PRIVATE sfml-graphics sfml-audio sfml-window sfml-system)
else()
    target_link_libraries(mario PRIVATE sfml-graphics sfml-audio sfml-window sfml-system)
endif()

enable_testing()

add_executable(solid03_tests
    tests/level_definition_tests.cpp
    src/Level/EntitySymbolCatalog.cpp
    src/Level/LevelDefinitionLoader.cpp
    src/PvP/PvPCombatResolver.cpp
)

target_include_directories(solid03_tests
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(TARGET SFML::System)
    target_link_libraries(solid03_tests PRIVATE SFML::System)
elseif(TARGET sfml-system)
    target_link_libraries(solid03_tests PRIVATE sfml-system)
endif()

add_test(
    NAME solid03_level_definition_tests
    COMMAND solid03_tests
)
set_tests_properties(
    solid03_level_definition_tests
    PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_executable(solid11_tests
    tests/observer_and_entity_tests.cpp
    src/AdminControl/DebugMovementTrail.cpp
    src/Observer/Subject.cpp
    src/UI/HUD.cpp
    src/Entities/Entity.cpp
    src/Entities/Character.cpp
    src/Entities/Mario.cpp
    src/Entities/Luigi.cpp
    src/Entities/PlayerPalette.cpp
    src/Entities/PlayerAnimation.cpp
    src/Animation/SpriteAnimator.cpp
    src/PlayerStates/PlayerState.cpp
    src/PlayerStates/SmallState.cpp
    src/PlayerStates/SuperState.cpp
    src/PlayerStates/FireState.cpp
    src/PlayerEffects/PlayerEffect.cpp
    src/PlayerEffects/DamageInvincibilityEffect.cpp
    src/PlayerEffects/StarEffect.cpp
    src/PlayerEffects/ShieldEffect.cpp
    src/Commands/CrawlCommand.cpp
    src/Entities/Enemies/Enemy.cpp
    src/Entities/Enemies/Goomba.cpp
    src/Entities/Enemies/Koopa.cpp
    src/Entities/Enemies/PiranhaPlant.cpp
    src/Entities/Enemies/GreenParatroopa.cpp
    src/Entities/Items/Item.cpp
    src/Entities/Items/Coin.cpp
    src/Entities/Items/Mushroom.cpp
    src/Entities/Items/FireFlower.cpp
    src/Entities/Items/StarItem.cpp
    src/Entities/Items/OneUpMushroom.cpp
    src/Core/AssetManager.cpp
)

target_include_directories(solid11_tests
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(TARGET SFML::Graphics)
    target_link_libraries(solid11_tests PRIVATE SFML::Graphics SFML::Audio SFML::Window SFML::System)
elseif(TARGET sfml-graphics)
    target_link_libraries(solid11_tests PRIVATE sfml-graphics sfml-audio sfml-window sfml-system)
else()
    target_link_libraries(solid11_tests PRIVATE sfml-graphics sfml-audio sfml-window sfml-system)
endif()

add_test(
    NAME solid11_observer_and_entity_tests
    COMMAND solid11_tests
)
set_tests_properties(
    solid11_observer_and_entity_tests
    PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        ENVIRONMENT "PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY};$ENV{PATH}"
)

# Copy assets to executable folder post-build
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET mario POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets"
        COMMENT "Copying assets to output directory"
    )
endif()

# Copy SFML DLLs on Windows post-build if fetched
if(WIN32 AND TARGET sfml-graphics)
    add_custom_command(TARGET mario POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:sfml-graphics>
            $<TARGET_FILE:sfml-audio>
            $<TARGET_FILE:sfml-window>
            $<TARGET_FILE:sfml-system>
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMENT "Copying SFML DLLs to runtime directory"
    )

    # Copy OpenAL DLL (required by sfml-audio) - without this the game crashes silently on startup
    if(EXISTS "${CMAKE_BINARY_DIR}/_deps/sfml-src/extlibs/bin/x64/openal32.dll")
        set(OPENAL_DLL "${CMAKE_BINARY_DIR}/_deps/sfml-src/extlibs/bin/x64/openal32.dll")
    elseif(EXISTS "${sfml_SOURCE_DIR}/extlibs/bin/x64/openal32.dll")
        set(OPENAL_DLL "${sfml_SOURCE_DIR}/extlibs/bin/x64/openal32.dll")
    endif()

    if(DEFINED OPENAL_DLL AND EXISTS "${OPENAL_DLL}")
        add_custom_command(TARGET mario POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${OPENAL_DLL}"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
            COMMENT "Copying openal32.dll to runtime directory"
        )
    endif()
endif()
