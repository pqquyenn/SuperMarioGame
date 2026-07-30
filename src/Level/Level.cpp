#include "Level/Level.h"

bool Level::loadLevel(const std::string& levelFile) {
    isUnderground = false;
    std::string paths[] = {
        levelFile,
        "assets/maps/" + levelFile,
        "assets/maps/1.1/" + levelFile,
        "../assets/maps/" + levelFile,
        "../assets/maps/1.1/" + levelFile,
        "../../assets/maps/" + levelFile,
        "../" + levelFile,
        "../../" + levelFile
    };
    for (const auto& p : paths) {
        if (map.readFromFile(p)) return true;
    }
    return false;
}

bool Level::loadHiddenMap(const std::string& hiddenFile) {
    isUnderground = true;
    std::string paths[] = {
        hiddenFile,
        "assets/maps/" + hiddenFile,
        "assets/maps/1.1/" + hiddenFile,
        "assets/maps/Mario Game Assets/" + hiddenFile,
        "../assets/maps/" + hiddenFile,
        "../assets/maps/1.1/" + hiddenFile,
        "../assets/maps/Mario Game Assets/" + hiddenFile,
        "../../assets/maps/" + hiddenFile,
        "../" + hiddenFile,
        "../../" + hiddenFile
    };
    for (const auto& p : paths) {
        if (map.readFromFile(p)) return true;
    }
    return false;
}

bool Level::loadMap(const std::string& mapFile) {
    if (mapFile.find("underground") != std::string::npos || mapFile.find("hidden") != std::string::npos) {
        return loadHiddenMap(mapFile);
    }
    return loadLevel(mapFile);
}

void Level::update(float dt) {}

void Level::render(sf::RenderWindow& window) {
    map.render(window, camera);
}