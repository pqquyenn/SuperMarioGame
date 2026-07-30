#pragma once

#include "Level/TileMap.h"
#include "Level/Camera.h"
#include <string>

class Level {
private:
    int levelId = 1;
    TileMap map;
    Camera camera;
    bool isUnderground = false;

public:
    Level(int id = 1) : levelId(id) {}
    bool loadLevel(const std::string& levelFile);
    bool loadHiddenMap(const std::string& hiddenFile = "underground.txt");
    bool loadMap(const std::string& mapFile);
    void update(float dt);
    void render(sf::RenderWindow& window);

    TileMap& getTileMap() { return map; }
    Camera& getCamera() { return camera; }
    bool getIsUnderground() const { return isUnderground; }
};