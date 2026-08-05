#pragma once

#include "Level/TileMap.h"
#include "Level/Camera.h"
#include <string>
#include <vector>
#include <memory>

class Enemy;
class Item;

class Level {
private:
    int levelId = 1;
    TileMap map;
    TileMap bgMap;
    Camera camera;
    bool isUnderground = false;

    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Item>> items;

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
    void warpToUnderground(class Mario* mario = nullptr);
    void warpToOverworldExit(class Mario* mario = nullptr);

    TileMap& getTileMap() { return map; }
    TileMap& getBgMap() { return bgMap; }
    Camera& getCamera() { return camera; }
    bool getIsUnderground() const { return isUnderground; }

    std::vector<std::unique_ptr<Enemy>>& getEnemies() { return enemies; }
    const std::vector<std::unique_ptr<Enemy>>& getEnemies() const { return enemies; }

    std::vector<std::unique_ptr<Item>>& getItems() { return items; }
    const std::vector<std::unique_ptr<Item>>& getItems() const { return items; }
};