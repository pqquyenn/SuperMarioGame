#include "Level/Level.h"
#include "Physics/CollisionManager.h"
#include "Factories/EntityFactory.h"
#include "Entities/Enemies/Enemy.h"
#include "Entities/Items/Item.h"
#include "Entities/Items/Coin.h"
#include "Entities/Items/Mushroom.h"
#include "Entities/Mario.h"
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
                std::cout << "[Level] Spawned Enemy: " << data.type << " at (" << data.position.x << ", " << data.position.y << ")" << std::endl;
            }
        }
    }

    for (const auto& data : itemSpawns) {
        if (auto entity = factory.create(data.type, data.position)) {
            if (auto* item = dynamic_cast<Item*>(entity.get())) {
                entity.release();
                items.push_back(std::unique_ptr<Item>(item));
                std::cout << "[Level] Spawned Item: " << data.type << " at (" << data.position.x << ", " << data.position.y << ")" << std::endl;
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
                std::filesystem::path bgPath = std::filesystem::path(path).parent_path() / "background.txt";
                if (std::filesystem::exists(bgPath)) {
                    bgMap.readFromFile(bgPath.string());
                }
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
    sf::FloatRect camBounds = camera.getViewBounds();
    const float spawnMargin = 80.f;

    for (auto& enemy : enemies) {
        if (!enemy || !enemy->isActive()) continue;

        if (!enemy->isActivated()) {
            if (enemy->getPosition().x <= camBounds.left + camBounds.width + spawnMargin) {
                enemy->setActivated(true);
            }
        }

        if (enemy->isActivated()) {
            enemy->update(dt);
            CollisionManager::resolveTileCollisions(*enemy, map);
        }
    }

    for (auto& item : items) {
        if (item && item->isActive()) {
            item->update(dt);
            
            bool isEthereal = false;
            if (auto* coin = dynamic_cast<Coin*>(item.get())) {
                if (coin->isPopping()) isEthereal = true;
            } else if (auto* shroom = dynamic_cast<Mushroom*>(item.get())) {
                if (shroom->isEmerging()) isEthereal = true;
            }
            
            if (!isEthereal) {
                CollisionManager::resolveTileCollisions(*item, map);
            }
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
    bgMap.render(window, camera);

    for (const auto& item : items) {
        if (item && item->isActive()) {
            item->render(window);
        }
    }

    for (const auto& enemy : enemies) {
        if (enemy && enemy->isActive()) {
            enemy->render(window);
        }
    }

    map.render(window, camera);
}

void Level::spawnItemFromBlock(float x, float y) {
    std::string itemType = "Coin";
    if (std::abs(x - 336.f) < 1.f || std::abs(x - 1248.f) < 1.f || std::abs(x - 1744.f) < 1.f) {
        itemType = "Mushroom";
    }

    sf::Vector2f spawnPos = (itemType == "Coin") ? sf::Vector2f{x, y - 16.f} : sf::Vector2f{x, y};
    if (auto entity = EntityFactory::getInstance().create(itemType, spawnPos)) {
        if (itemType == "Coin") {
            if (auto* coin = dynamic_cast<Coin*>(entity.get())) {
                entity.release();
                coin->startPop();
                items.push_back(std::unique_ptr<Item>(coin));
            }
        } else {
            if (auto* item = dynamic_cast<Item*>(entity.get())) {
                if (auto* shroom = dynamic_cast<Mushroom*>(item)) {
                    shroom->startEmerge();
                }
                entity.release();
                items.push_back(std::unique_ptr<Item>(item));
            }
        }
    }
}

void Level::warpToUnderground(Mario* mario) {
    std::cout << "[Level] Teleporting to hidden underground map area..." << std::endl;
    if (mario) {
        mario->setPosition(3416.f, 32.f);
        mario->setVelocity(sf::Vector2f(0.f, 0.f));
    }
    camera.setCenter(3416.f, 120.f);
}

void Level::warpToOverworldExit(Mario* mario) {
    std::cout << "[Level] Teleporting back to Overworld (5th pipe)..." << std::endl;
    if (mario) {
        mario->setPosition(2608.f, 160.f);
        mario->setVelocity(sf::Vector2f(0.f, -100.f)); // pop out of pipe
    }
    camera.setCenter(2608.f, 120.f);
}