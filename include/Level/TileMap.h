#pragma once

#include "Level/Tile.h"
#include <vector>
#include <string>

class TileMap {
private:
    int width = 0;
    int height = 0;
    float tileSize = 32.f;
    std::vector<std::vector<Tile>> grid;

public:
    TileMap() = default;
    bool loadFromFile(const std::string& filepath);
    void render(sf::RenderWindow& window);

    const std::vector<std::vector<Tile>>& getGrid() const { return grid; }
    float getTileSize() const { return tileSize; }
};
