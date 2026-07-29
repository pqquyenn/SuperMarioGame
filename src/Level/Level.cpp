#include "Level/Level.h"

bool Level::loadLevel(const std::string& levelFile) {
    if (map.readFromFile(levelFile)) return true;
    if (map.readFromFile("../" + levelFile)) return true;
    if (map.readFromFile("../../" + levelFile)) return true;
    if (map.readFromFile("../../../" + levelFile)) return true;
    return false;
}

void Level::update(float dt) {}

void Level::render(sf::RenderWindow& window) {
    map.render(window, camera);
}