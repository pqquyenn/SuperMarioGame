#include "Level/Level.h"
#include "Physics/CollisionManager.h"
#include "Factories/EntityFactory.h"
#include "Entities/Enemies/Enemy.h"
#include "Entities/Enemies/Goomba.h"
#include "Entities/Enemies/Koopa.h"
#include "Entities/Enemies/PiranhaPlant.h"
#include "Entities/Items/Item.h"
#include "Entities/Items/Coin.h"
#include "Entities/Items/Mushroom.h"
#include "Entities/Items/FireFlower.h"
#include <algorithm>

Level::Level(int id) : levelId(id) {}
Level::~Level() = default;

static bool factoryInitialized = false;
static void initEntityFactory() {
    if (factoryInitialized) return;
    auto& factory = EntityFactory::getInstance();

    factory.registerType("Goomba", [](const sf::Vector2f& pos) {
        return std::make_unique<Goomba>(pos.x, pos.y);
    });
    factory.registerType("Koopa", [](const sf::Vector2f& pos) {
        return std::make_unique<Koopa>(pos.x, pos.y);
    });
    factory.registerType("PiranhaPlant", [](const sf::Vector2f& pos) {
        return std::make_unique<PiranhaPlant>(pos.x, pos.y);
    });

    factory.registerType("Coin", [](const sf::Vector2f& pos) {
        return std::make_unique<Coin>(pos.x, pos.y);
    });
    factory.registerType("Mushroom", [](const sf::Vector2f& pos) {
        return std::make_unique<Mushroom>(pos.x, pos.y);
    });
    factory.registerType("FireFlower", [](const sf::Vector2f& pos) {
        return std::make_unique<FireFlower>(pos.x, pos.y);
    });

    factoryInitialized = true;
}

void Level::spawnEntitiesFromMap() {
    auto& factory = EntityFactory::getInstance();

    if (levelId == 1) {
        auto g1 = factory.create("Goomba", {352.f, 192.f});
        if (g1) enemies.push_back(std::unique_ptr<Enemy>(dynamic_cast<Enemy*>(g1.release())));

        auto g2 = factory.create("Goomba", {640.f, 192.f});
        if (g2) enemies.push_back(std::unique_ptr<Enemy>(dynamic_cast<Enemy*>(g2.release())));

        auto k1 = factory.create("Koopa", {1700.f, 192.f});
        if (k1) enemies.push_back(std::unique_ptr<Enemy>(dynamic_cast<Enemy*>(k1.release())));

        auto c1 = factory.create("Coin", {256.f, 144.f});
        if (c1) items.push_back(std::unique_ptr<Item>(dynamic_cast<Item*>(c1.release())));

        auto m1 = factory.create("Mushroom", {336.f, 144.f});
        if (m1) items.push_back(std::unique_ptr<Item>(dynamic_cast<Item*>(m1.release())));
    }
    else if (levelId == 2) {
        auto g1 = factory.create("Goomba", {200.f, 200.f});
        if (g1) enemies.push_back(std::unique_ptr<Enemy>(dynamic_cast<Enemy*>(g1.release())));

        auto c1 = factory.create("Coin", {160.f, 160.f});
        if (c1) items.push_back(std::unique_ptr<Item>(dynamic_cast<Item*>(c1.release())));
    }
    else if (levelId == 3) {
        auto p1 = factory.create("PiranhaPlant", {300.f, 150.f});
        if (p1) enemies.push_back(std::unique_ptr<Enemy>(dynamic_cast<Enemy*>(p1.release())));
    }
}

bool Level::loadLevel(const std::string& levelFile) {
    isUnderground = false;
    enemies.clear();
    items.clear();
    initEntityFactory();

    std::string paths[] = {
        levelFile,
        "assets/maps/" + levelFile,
        "assets/maps/1.1/" + levelFile,
        "assets/maps/1.2/" + levelFile,
        "assets/maps/1.3/" + levelFile,
        "../assets/maps/" + levelFile,
        "../assets/maps/1.1/" + levelFile,
        "../assets/maps/1.2/" + levelFile,
        "../assets/maps/1.3/" + levelFile,
        "../../assets/maps/" + levelFile,
        "../" + levelFile,
        "../../" + levelFile
    };
    for (const auto& p : paths) {
        if (map.readFromFile(p)) {
            spawnEntitiesFromMap();
            return true;
        }
    }
    return false;
}

bool Level::loadHiddenMap(const std::string& hiddenFile) {
    isUnderground = true;
    enemies.clear();
    items.clear();
    initEntityFactory();

    std::string paths[] = {
        hiddenFile,
        "assets/maps/" + hiddenFile,
        "assets/maps/1.1/" + hiddenFile,
        "assets/maps/1.2/" + hiddenFile,
        "assets/maps/1.3/" + hiddenFile,
        "assets/maps/Mario Game Assets/" + hiddenFile,
        "../assets/maps/" + hiddenFile,
        "../assets/maps/1.1/" + hiddenFile,
        "../assets/maps/1.2/" + hiddenFile,
        "../assets/maps/1.3/" + hiddenFile,
        "../assets/maps/Mario Game Assets/" + hiddenFile,
        "../../assets/maps/" + hiddenFile,
        "../" + hiddenFile,
        "../../" + hiddenFile
    };
    for (const auto& p : paths) {
        if (map.readFromFile(p)) {
            spawnEntitiesFromMap();
            return true;
        }
    }
    return false;
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