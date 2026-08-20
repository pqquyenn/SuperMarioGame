#pragma once

#include "Level/TileMap.h"
#include "Level/Camera.h"
#include "Level/LevelDefinition.h"
#include "Entities/MovingPlatform.h"
#include "Physics/TileCollisionHandler.h"
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include <memory>

class Enemy;
class Item;
class Character;

struct EnemyRuntimeStats {
    std::size_t active{0};
    std::size_t inactive{0};
    std::size_t removed{0};
};

struct WarpZoneInfo {
    sf::FloatRect bounds;
    std::string label;
};

class Level : public TileCollisionHandler {
private:
    TileMap map;
    TileMap bgMap;
    Camera camera;
    bool hasDefinition = false;
    LevelDefinition definition;
    std::string currentArea{"overworld"};
    sf::Vector2f levelStartHint{0.f, 0.f};

    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Item>> items;
    std::vector<std::unique_ptr<MovingPlatform>> movingPlatforms;
    std::size_t removedEnemyCount{0};

    bool spawnEntitiesFromMap();
    bool loadManifest(const std::string& filename);
    void setCurrentArea(const std::string& area);
    const AnchorDefinition* findAnchor(const std::string& anchorId) const;
    bool activatePortal(
        Character& character,
        const PortalDefinition& portal);
    sf::Vector2f findGroundedSpawn(
        const sf::Vector2f& requestedPosition,
        const sf::Vector2f& characterSize
    ) const;

public:
    Level();
    ~Level();
    bool loadLevel(const std::string& levelFile);
    void update(float dt);
    void render(sf::RenderWindow& window);
    void spawnItemFromBlock(float x, float y);
    void spawnItemFromBlock(float x, float y, Character* character);
    std::string getBlockItemType(
        float x,
        float y,
        const Character* character = nullptr
    ) const;
    // Returns an empty string for ordinary bricks without hidden contents.
    std::string getBrickItemType(float x, float y) const;

    bool tryActivatePortal(
        Character& character,
        const sf::FloatRect& contact,
        PortalActivation activation);
    bool tryActivatePortalForInput(
        Character& character,
        PortalActivation activation);
    bool tryActivateFirstPortalFromCurrentArea(Character& character);
    void resetToInitialArea();
    void onTileCeilingContact(
        Entity& entity,
        TileMap& map,
        Tile& tile,
        const TileHandle& handle,
        const CollisionContact& contact) override;
    void onTileOverlap(
        Entity& entity,
        TileMap& map,
        Tile& tile,
        const TileHandle& handle,
        const CollisionContact& contact) override;
    void updateCameraFor(const sf::Vector2f& playerPosition);
    bool usesDarkBackground() const;
    float getKillPlaneY() const;
    float getLeftBoundaryX() const;
    float getRightBoundaryX(float entityWidth = 0.f) const;

    TileMap& getTileMap() { return map; }
    const TileMap& getTileMap() const { return map; }
    TileMap& getBgMap() { return bgMap; }
    Camera& getCamera() { return camera; }
    const Camera& getCamera() const { return camera; }
    const std::string& getCurrentArea() const { return currentArea; }
    const LevelDefinition& getDefinition() const { return definition; }
    bool isDataDriven() const { return hasDefinition; }
    const std::string& getNextStage() const { return definition.nextStage; }
    int getTimeLimit() const { return definition.timeLimit; }

    std::optional<sf::FloatRect> findFlagpoleBounds() const { return map.findFlagpoleBounds(); }
    std::optional<sf::Vector2f> findCastleDoor() const { return map.findCastleDoor(); }
    std::optional<sf::Vector2f> findFlagPosition() const { return map.findFlagPosition(); }
    std::optional<sf::Vector2f> takeFlagPosition() { return map.takeFlagPosition(); }
    sf::Vector2f getStartPosition(
        const sf::Vector2f& characterSize
    ) const;
    EnemyRuntimeStats getEnemyRuntimeStats() const;
    std::vector<WarpZoneInfo> getWarpZones() const;

    std::vector<std::unique_ptr<Enemy>>& getEnemies() { return enemies; }
    const std::vector<std::unique_ptr<Enemy>>& getEnemies() const { return enemies; }

    std::vector<std::unique_ptr<Item>>& getItems() { return items; }
    const std::vector<std::unique_ptr<Item>>& getItems() const { return items; }

    std::vector<std::unique_ptr<MovingPlatform>>& getMovingPlatforms() { return movingPlatforms; }
};
