#include "Level/Level.h"

bool Level::loadLevel(const std::string& levelFile) {
    return map.readFromFile(levelFile);
}

void Level::update(float dt) {}

void Level::render(sf::RenderWindow& window) {
    map.render(window, camera);
}
