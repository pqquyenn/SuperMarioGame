#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

#include <optional>
#include <string>
#include <vector>

enum class PortalActivation {
    Down,
    Right
};

enum class PlatformMotion {
    OscillateVertical,
    OscillateHorizontal,
    LoopDown,
    LoopUp
};

struct EntitySpawnDefinition {
    std::string id;
    std::string symbol;
    std::string resolvedType;
    std::string area{"overworld"};
    sf::Vector2f tilePosition{0.f, 0.f};
    int direction{-1};
    float speed{-1.f};
};

struct PlatformDefinition {
    std::string id;
    std::string area{"overworld"};
    sf::Vector2f tilePosition{0.f, 0.f};
    float widthTiles{3.f};
    float minimumTile{0.f};
    float maximumTile{0.f};
    float speed{50.f};
    PlatformMotion motion{PlatformMotion::OscillateVertical};
};

struct BlockContentDefinition {
    std::string area{"overworld"};
    sf::Vector2i tilePosition{0, 0};
    std::string content{"Coin"};
};

struct AnchorDefinition {
    std::string id;
    std::string area{"overworld"};
    sf::Vector2f tilePosition{0.f, 0.f};
    sf::Vector2f exitVelocity{0.f, 0.f};
};

struct PortalDefinition {
    std::string id;
    std::string sourceArea{"overworld"};
    PortalActivation activation{PortalActivation::Down};
    sf::FloatRect triggerTiles{0.f, 0.f, 1.f, 1.f};
    std::string targetAnchor;
};

struct CameraZoneDefinition {
    std::string id;
    std::string area{"overworld"};
    sf::FloatRect boundsTiles{0.f, 0.f, 1.f, 1.f};
    bool followX{true};
    bool followY{false};
    float centerYTiles{7.f};
    bool darkBackground{false};
};

struct StageRules {
    float killPlaneTile{-1.f};
    float leftBoundaryTile{0.f};
    float rightBoundaryTile{-1.f};
    float enemyVoidMarginTiles{4.f};
};

struct LevelDefinition {
    int version{1};
    std::string sourcePath;
    std::string id;
    std::string name;
    std::string terrainPath;
    std::string backgroundPath;
    std::string nextStage;
    std::string initialArea{"overworld"};
    int timeLimit{400};
    float tileSize{16.f};
    StageRules rules;
    std::optional<sf::Vector2f> playerStartTile;
    std::vector<EntitySpawnDefinition> entities;
    std::vector<EntitySpawnDefinition> items;
    std::vector<PlatformDefinition> platforms;
    std::vector<BlockContentDefinition> blockContents;
    std::vector<AnchorDefinition> anchors;
    std::vector<PortalDefinition> portals;
    std::vector<CameraZoneDefinition> cameraZones;
};

