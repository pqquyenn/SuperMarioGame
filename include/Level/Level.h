#pragma once

#include "Level/TileMap.h"
#include "Level/Camera.h"
#include "Entities/MovingPlatform.h"
#include <cstddef>
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

class Level {
private:
    int levelId = 1;
    TileMap map;
    TileMap bgMap;
    Camera camera;
    bool isUnderground = false;
    bool isInBonusRoom = false;    // true while the player is in the hidden vault
    sf::Vector2f levelStartHint{0.f, 0.f};

    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Item>> items;
    std::vector<std::unique_ptr<MovingPlatform>> movingPlatforms;
    std::size_t removedEnemyCount{0};

    void spawnEntitiesFromMap();
    bool loadInternal(const std::string& filename, bool isUnderground);
    sf::Vector2f findGroundedSpawn(
        const sf::Vector2f& requestedPosition,
        const sf::Vector2f& characterSize
    ) const;

public:
    Level(int id = 1);
    ~Level();
    bool loadLevel(const std::string& levelFile);
    bool loadHiddenMap(const std::string& hiddenFile = "underground.txt");
    bool loadMap(const std::string& mapFile);
    void update(float dt);
    void render(sf::RenderWindow& window);
    void spawnItemFromBlock(float x, float y);
    void spawnItemFromBlock(float x, float y, Character* character);
    std::string getBlockItemType(
        float x,
        const Character* character = nullptr
    ) const;
    // Returns an empty string for ordinary bricks without hidden contents.
    std::string getBrickItemType(float x) const;
    void warpToUnderground(Character* character = nullptr);
    void warpToUnderground1_2(Character* character = nullptr);
    void warpToOverworldExit(Character* character = nullptr);

    // World 1-2 warp methods
    void warpPipeA_Entry(Character* character = nullptr);   // Overworld intro pipe → underground
    void warpPipeB_Entry(Character* character = nullptr);   // Underground → bonus room vault
    void warpPipeC1_Exit(Character* character = nullptr);   // Bonus room → underground Pipe C2

    TileMap& getTileMap() { return map; }
    const TileMap& getTileMap() const { return map; }
    TileMap& getBgMap() { return bgMap; }
    Camera& getCamera() { return camera; }
    const Camera& getCamera() const { return camera; }
    int  getLevelId() const { return levelId; }
    void setLevelId(int id) { levelId = id; }
    bool getIsUnderground() const { return isUnderground; }
    bool getIsInBonusRoom() const { return isInBonusRoom; }
    void setIsUnderground(bool v) { isUnderground = v; }
    void setIsInBonusRoom(bool v) { isInBonusRoom = v; }

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
