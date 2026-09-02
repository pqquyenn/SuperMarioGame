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

add_executable(pvp_rules_tests
    tests/pvp_rules_tests.cpp
    src/PvP/PvPCameraPolicy.cpp
    src/PvP/PvPMatchRules.cpp
    src/PvP/PvPPlayerSession.cpp
    src/PvP/PvPRuleset.cpp
)

target_include_directories(pvp_rules_tests
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(TARGET SFML::System)
    target_link_libraries(pvp_rules_tests PRIVATE SFML::System)
elseif(TARGET sfml-system)
    target_link_libraries(pvp_rules_tests PRIVATE sfml-system)
endif()

add_test(
    NAME pvp_rules_tests
    COMMAND pvp_rules_tests
)
set_tests_properties(
    pvp_rules_tests
    PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_executable(key_binding_tests
    tests/key_binding_tests.cpp
    src/Input/KeyBinding.cpp
    src/Input/KeyBindingService.cpp
)

target_include_directories(key_binding_tests
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(TARGET SFML::Window)
    target_link_libraries(key_binding_tests PRIVATE SFML::Window SFML::System)
elseif(TARGET sfml-window)
    target_link_libraries(key_binding_tests PRIVATE sfml-window sfml-system)
endif()

add_test(
    NAME key_binding_tests
    COMMAND key_binding_tests
)
set_tests_properties(
    key_binding_tests
    PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

set(SOLID03_RUNTIME_SOURCES ${SOURCES})
list(FILTER SOLID03_RUNTIME_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")

add_executable(solid03_portal_tests
    tests/level_portal_tests.cpp
    ${SOLID03_RUNTIME_SOURCES}
)

target_include_directories(solid03_portal_tests
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(TARGET SFML::Graphics)
    target_link_libraries(solid03_portal_tests PRIVATE SFML::Graphics SFML::Audio SFML::Window SFML::System)
elseif(TARGET sfml-graphics)
    target_link_libraries(solid03_portal_tests PRIVATE sfml-graphics sfml-audio sfml-window sfml-system)
else()
    target_link_libraries(solid03_portal_tests PRIVATE sfml-graphics sfml-audio sfml-window sfml-system)
endif()

add_test(
    NAME solid03_portal_runtime_tests
    COMMAND solid03_portal_tests
)
set_tests_properties(
    solid03_portal_runtime_tests
    PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        ENVIRONMENT "PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY};$ENV{PATH}"
)

add_executable(solid04_tests
    tests/collision_tests.cpp
    src/Physics/CollisionGeometry.cpp
)

target_include_directories(solid04_tests
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(TARGET SFML::System)
    target_link_libraries(solid04_tests PRIVATE SFML::System)
elseif(TARGET sfml-system)
    target_link_libraries(solid04_tests PRIVATE sfml-system)
endif()

add_test(
    NAME solid04_collision_tests
    COMMAND solid04_tests
)
set_tests_properties(
    solid04_collision_tests
    PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_executable(duo_rules_tests
    tests/duo_rules_tests.cpp
    src/Duo/DuoRules.cpp
)

target_include_directories(duo_rules_tests
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(TARGET SFML::System)
    target_link_libraries(duo_rules_tests PRIVATE SFML::System)
elseif(TARGET sfml-system)
    target_link_libraries(duo_rules_tests PRIVATE sfml-system)
endif()

add_test(
    NAME duo_rules_tests
    COMMAND duo_rules_tests
)
set_tests_properties(
    duo_rules_tests
    PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_executable(solid04_tile_tests
    tests/tile_collision_tests.cpp
    src/Physics/CollisionManager.cpp
    src/Physics/CollisionGeometry.cpp
    src/Level/Tile.cpp
    src/Level/TileMap.cpp
    src/Level/Camera.cpp
    src/Entities/Entity.cpp
    src/Entities/Character.cpp
    src/Entities/MovingPlatform.cpp
    src/Entities/PlayerAnimation.cpp
    src/Entities/PlayerPalette.cpp
    src/Entities/Enemies/Enemy.cpp
    src/Animation/SpriteAnimator.cpp
    src/Observer/Subject.cpp
    src/PlayerStates/PlayerState.cpp
    src/PlayerStates/SmallState.cpp
    src/PlayerStates/SuperState.cpp
    src/PlayerStates/FireState.cpp
    src/PlayerStates/PlaneState.cpp
    src/PlayerEffects/PlayerEffect.cpp
    src/PlayerEffects/DamageInvincibilityEffect.cpp
    src/PlayerEffects/StarEffect.cpp
    src/PlayerEffects/ShieldEffect.cpp
    src/Core/AssetManager.cpp
    src/Core/SoundManager.cpp
)

target_include_directories(solid04_tile_tests
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(TARGET SFML::Graphics)
    target_link_libraries(solid04_tile_tests PRIVATE SFML::Graphics SFML::Audio SFML::Window SFML::System)
elseif(TARGET sfml-graphics)
    target_link_libraries(solid04_tile_tests PRIVATE sfml-graphics sfml-audio sfml-window sfml-system)
else()
    target_link_libraries(solid04_tile_tests PRIVATE sfml-graphics sfml-audio sfml-window sfml-system)
endif()

add_test(
    NAME solid04_tile_collision_tests
    COMMAND solid04_tile_tests
)
set_tests_properties(
    solid04_tile_collision_tests
    PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        ENVIRONMENT "PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY};$ENV{PATH}"
)

add_executable(solid07_factory_tests
    tests/entity_factory_tests.cpp
    ${SOLID03_RUNTIME_SOURCES}
)

target_include_directories(solid07_factory_tests
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(TARGET SFML::Graphics)
    target_link_libraries(solid07_factory_tests PRIVATE SFML::Graphics SFML::Audio SFML::Window SFML::System)
elseif(TARGET sfml-graphics)
    target_link_libraries(solid07_factory_tests PRIVATE sfml-graphics sfml-audio sfml-window sfml-system)
else()
    target_link_libraries(solid07_factory_tests PRIVATE sfml-graphics sfml-audio sfml-window sfml-system)
endif()

add_test(
    NAME solid07_factory_tests
    COMMAND solid07_factory_tests
)
set_tests_properties(
    solid07_factory_tests
    PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        ENVIRONMENT "PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY};$ENV{PATH}"
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
    src/PlayerStates/PlaneState.cpp
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
    src/Entities/Items/PlaneItem.cpp
    src/Entities/Items/StarItem.cpp
    src/Entities/Items/OneUpMushroom.cpp
    src/Entities/YellowLaser.cpp
    src/Core/AssetManager.cpp
    src/Core/SoundManager.cpp
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
