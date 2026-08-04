#include "Level/Level.h"
#include "Physics/CollisionManager.h"
#include "Factories/EntityFactory.h"
#include "Entities/Enemies/Enemy.h"
#include "Entities/Items/Item.h"
#include <algorithm>
#include <vector>
#include <filesystem>

Level::Level(int id) : levelId(id) {}
Level::~Level() = default;

struct SpawnData {
    std::string type;
    sf::Vector2f position;
};

void Level::spawnEntitiesFromMap() {
    auto& factory = EntityFactory::getInstance();
    
    // Data-driven spawn positions for each level
    std::vector<SpawnData> enemySpawns;
    std::vector<SpawnData> itemSpawns;

    if (levelId == 1) {
        enemySpawns = {
            {"Goomba", {336.f, 192.f}},
            {"Goomba", {656.f, 192.f}},
            {"Koopa", {1712.f, 176.f}}
        };
        itemSpawns = {
            {"Coin", {256.f, 144.f}},
            {"Mushroom", {336.f, 144.f}}
        };
    } else if (levelId == 2) {
        enemySpawns = {
            {"Goomba", {200.f, 200.f}}
        };
        itemSpawns = {
            {"Coin", {160.f, 160.f}}
        };
    } else if (levelId == 3) {
        enemySpawns = {
            {"PiranhaPlant", {300.f, 150.f}}
        };
    }

    for (const auto& data : enemySpawns) {
        if (auto entity = factory.create(data.type, data.position)) {
            if (auto* enemy = dynamic_cast<Enemy*>(entity.get())) {
                entity.release();
                enemies.push_back(std::unique_ptr<Enemy>(enemy));
            }
        }
    }

    for (const auto& data : itemSpawns) {
        if (auto entity = factory.create(data.type, data.position)) {
            if (auto* item = dynamic_cast<Item*>(entity.get())) {
                entity.release();
                items.push_back(std::unique_ptr<Item>(item));
            }
        }
    }
}

bool Level::loadInternal(const std::string& filename, bool isUndergroundFlag) {
    isUnderground = isUndergroundFlag;
    enemies.clear();
    items.clear();
    
    EntityFactory::getInstance().registerDefaultEntities();

    std::filesystem::path p(filename);
    std::string rawFilename = p.filename().string();

    const std::string candidates[] = {
        filename,
        "assets/maps/" + filename,
        "../assets/maps/" + filename,
        "../../assets/maps/" + filename,
        "assets/maps/1.1/" + rawFilename,
        "assets/maps/1.2/" + rawFilename,
        "assets/maps/1.3/" + rawFilename,
        "../assets/maps/1.1/" + rawFilename,
        "../assets/maps/1.2/" + rawFilename,
        "../assets/maps/1.3/" + rawFilename,
        "../../assets/maps/1.1/" + rawFilename,
        "../../assets/maps/1.2/" + rawFilename,
        "../../assets/maps/1.3/" + rawFilename,
        "../" + filename,
        "../../" + filename
    };

    for (const auto& path : candidates) {
        if (std::filesystem::exists(path)) {
            if (map.readFromFile(path)) {
                spawnEntitiesFromMap();
                return true;
            }
        }
    }
    std::cerr << "[Level] Failed to find level map file: " << filename << std::endl;
    return false;
}

bool Level::loadLevel(const std::string& levelFile) {
    return loadInternal(levelFile, false);
}

bool Level::loadHiddenMap(const std::string& hiddenFile) {
    return loadInternal(hiddenFile, true);
}

bool Level::loadMap(const std::string& mapFile) {
    if (mapFile.find("underground") != std::string::npos || mapFile.find("hidden") != std::string::npos) {
        return loadHiddenMap(mapFile);
    }
    return loadLevel(mapFile);
}

void Level::update(float dt) {
    for (auto& enemy : enemies) {
        if (enemy && enemy->isActive()) {
            enemy->update(dt);
            CollisionManager::resolveTileCollisions(*enemy, map);
        }
    }

    for (auto& item : items) {
        if (item && item->isActive()) {
            item->update(dt);
            CollisionManager::resolveTileCollisions(*item, map);
        }
    }

    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const auto& e) { return !e || !e->isActive(); }),
        enemies.end()
    );

    items.erase(
        std::remove_if(items.begin(), items.end(),
            [](const auto& i) { return !i || !i->isActive(); }),
        items.end()
    );
}

void Level::render(sf::RenderWindow& window) {
    map.render(window, camera);

    for (const auto& enemy : enemies) {
        if (enemy && enemy->isActive()) {
            enemy->render(window);
        }
    }

    for (const auto& item : items) {
        if (item && item->isActive()) {
            item->render(window);
        }
    }
}