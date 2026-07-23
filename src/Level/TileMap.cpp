#include "Level/TileMap.h"
#include <iostream>

bool TileMap::loadFromFile(const std::string& filepath) {
    // Read map format (JSON or text grid)
    return true;
}

void TileMap::render(sf::RenderWindow& window) {
    for (auto& row : grid) {
        for (auto& tile : row) {
            tile.render(window);
        }
    }
}
