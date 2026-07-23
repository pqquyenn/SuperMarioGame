#pragma once

#include "Level/TileMap.h"
#include <string>

class Level {
private:
    int levelId = 1;
    TileMap map;

public:
    Level(int id = 1) : levelId(id) {}
    bool loadLevel(const std::string& levelFile);
    void update(float dt);
    void render(sf::RenderWindow& window);

    TileMap& getTileMap() { return map; }
};
