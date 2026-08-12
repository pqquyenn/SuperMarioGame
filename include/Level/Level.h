#pragma once

#include "Level/TileMap.h"
#include "Level/Camera.h"
#include "Level/LevelDefinition.h"
#include "Entities/MovingPlatform.h"
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include <memory>
#include <unordered_set>

class Enemy;
class Item;
class Character;

struct EnemyRuntimeStats {
    std::size_t active{0};
    std::size_t inactive{0};
    std::size_t removed{0};
};

class Level {
private:
    int levelId = 1;
    TileMap map;
    TileMap bgMap;
    Camera camera;
    bool isUnderground = false;
    bool isInBonusRoom = false;    // true while the player is in the hidden vault
    bool hasDefinition = false;
    LevelDefinition definition;
    std::string currentArea{"overworld"};
    sf::Vector2f levelStartHint{0.f, 0.f};
    std::optional<sf::Vector2f> levelEndHint;

    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Item>> items;
    std::vector<std::unique_ptr<MovingPlatform>> movingPlatforms;
    std::size_t removedEnemyCount{0};
    std::unordered_set<std::string> activatedCheckpoints;

    bool spawnEntitiesFromMap();
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
        float y,
        const Character* character = nullptr
    ) const;
    bool hasBlockContent(float x, float y) const;
    bool tryActivatePortal(
        Character& character,
        const sf::FloatRect& contactedTile,
        PortalActivation activation
    );
    void updateCameraFor(const sf::Vector2f& playerPosition);
    bool usesDarkBackground() const;
    float getKillPlaneY() const;
    float getLeftBoundaryX() const;
    float getRightBoundaryX(float entityWidth = 0.f) const;
    std::optional<sf::Vector2f> activateCheckpoint(
        const sf::FloatRect& characterBounds,
        const sf::Vector2f& characterSize);

    // Deprecated compatibility entry points. Data-driven portals do not call
    // these methods; they remain temporarily for the old hidden-map hotkeys.
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
    bool getIsUnderground() const { return isUnderground; }
    bool getIsInBonusRoom() const { return isInBonusRoom; }
    const std::string& getCurrentArea() const { return currentArea; }
    const LevelDefinition& getDefinition() const { return definition; }
    bool isDataDriven() const { return hasDefinition; }
    const std::string& getDisplayName() const { return definition.name; }
    const std::string& getNextStage() const { return definition.nextStage; }
    int getTimeLimit() const { return definition.timeLimit; }
    sf::Vector2f getStartPosition(
        const sf::Vector2f& characterSize
    ) const;
    const std::optional<sf::Vector2f>& getEndPosition() const {
        return levelEndHint;
    }
    bool hasReachedEnd(const sf::FloatRect& characterBounds) const;
    EnemyRuntimeStats getEnemyRuntimeStats() const;

    std::vector<std::unique_ptr<Enemy>>& getEnemies() { return enemies; }
    const std::vector<std::unique_ptr<Enemy>>& getEnemies() const { return enemies; }

    std::vector<std::unique_ptr<Item>>& getItems() { return items; }
    const std::vector<std::unique_ptr<Item>>& getItems() const { return items; }

    std::vector<std::unique_ptr<MovingPlatform>>& getMovingPlatforms() { return movingPlatforms; }
};
