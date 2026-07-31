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
    Camera camera;
    bool isUnderground = false;

    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Item>> items;

    void spawnEntitiesFromMap();

public:
    Level(int id = 1);
    ~Level();
    bool loadLevel(const std::string& levelFile);
    bool loadHiddenMap(const std::string& hiddenFile = "underground.txt");
    bool loadMap(const std::string& mapFile);
    void update(float dt);
    void render(sf::RenderWindow& window);

    TileMap& getTileMap() { return map; }
    Camera& getCamera() { return camera; }
    bool getIsUnderground() const { return isUnderground; }

    std::vector<std::unique_ptr<Enemy>>& getEnemies() { return enemies; }
    const std::vector<std::unique_ptr<Enemy>>& getEnemies() const { return enemies; }

    std::vector<std::unique_ptr<Item>>& getItems() { return items; }
    const std::vector<std::unique_ptr<Item>>& getItems() const { return items; }
};