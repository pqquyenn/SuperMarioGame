#pragma once

#include "Level/TileMap.h"
#include "Level/Camera.h"
#include "Entities/MovingPlatform.h"
#include <string>
#include <vector>
#include <memory>

class Enemy;
class Item;
class Character;

class Level {
private:
    int levelId = 1;
    TileMap map;
    TileMap bgMap;
    Camera camera;
    bool isUnderground = false;
    bool isInBonusRoom = false;    // true while Mario is in the hidden vault

    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Item>> items;
    std::vector<std::unique_ptr<MovingPlatform>> movingPlatforms;

    void spawnEntitiesFromMap();
    bool loadInternal(const std::string& filename, bool isUnderground);

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
    void warpToUnderground(class Mario* mario = nullptr);
    void warpToUnderground1_2(class Mario* mario = nullptr);
    void warpToOverworldExit(class Mario* mario = nullptr);

    // World 1-2 warp methods
    void warpPipeA_Entry(class Mario* mario = nullptr);   // Overworld intro pipe → underground
    void warpPipeB_Entry(class Mario* mario = nullptr);   // Underground → bonus room vault
    void warpPipeC1_Exit(class Mario* mario = nullptr);   // Bonus room → underground Pipe C2

    TileMap& getTileMap() { return map; }
    TileMap& getBgMap() { return bgMap; }
    Camera& getCamera() { return camera; }
    int  getLevelId() const { return levelId; }
    bool getIsUnderground() const { return isUnderground; }
    bool getIsInBonusRoom() const { return isInBonusRoom; }

    std::vector<std::unique_ptr<Enemy>>& getEnemies() { return enemies; }
    const std::vector<std::unique_ptr<Enemy>>& getEnemies() const { return enemies; }

    std::vector<std::unique_ptr<Item>>& getItems() { return items; }
    const std::vector<std::unique_ptr<Item>>& getItems() const { return items; }

    std::vector<std::unique_ptr<MovingPlatform>>& getMovingPlatforms() { return movingPlatforms; }
};