#include "Factories/EntityFactory.h"
#include "Entities/Entity.h"
#include "Entities/Enemies/Goomba.h"
#include "Entities/Enemies/Koopa.h"
#include "Entities/Enemies/PiranhaPlant.h"
#include "Entities/Items/Coin.h"
#include "Entities/Items/Mushroom.h"
#include "Entities/Items/FireFlower.h"
#include "Entities/Items/StarItem.h"
#include "Core/AssetManager.h"

void EntityFactory::registerDefaultEntities() {
    static bool initialized = false;
    if (initialized) return;

    registerType("Goomba", [](const sf::Vector2f& pos) {
        auto entity = std::make_unique<Goomba>(pos.x, pos.y);
        entity->setTexture(AssetManager::getInstance().getTexture("Goomba"));
        return entity;
    });
    registerType("Koopa", [](const sf::Vector2f& pos) {
        auto entity = std::make_unique<Koopa>(pos.x, pos.y);
        entity->setTexture(AssetManager::getInstance().getTexture("Koopa"));
        return entity;
    });
    registerType("PiranhaPlant", [](const sf::Vector2f& pos) {
        auto entity = std::make_unique<PiranhaPlant>(pos.x, pos.y);
        entity->setTexture(AssetManager::getInstance().getTexture("PiranhaPlant"));
        return entity;
    });
    registerType("Coin", [](const sf::Vector2f& pos) {
        auto entity = std::make_unique<Coin>(pos.x, pos.y);
        entity->setTexture(AssetManager::getInstance().getTexture("Coin"));
        return entity;
    });
    registerType("Mushroom", [](const sf::Vector2f& pos) {
        auto entity = std::make_unique<Mushroom>(pos.x, pos.y);
        entity->setTexture(AssetManager::getInstance().getTexture("Mushroom"));
        return entity;
    });
    registerType("FireFlower", [](const sf::Vector2f& pos) {
        auto entity = std::make_unique<FireFlower>(pos.x, pos.y);
        auto& assets = AssetManager::getInstance();
        sf::Texture& sheet = assets.getTexture("BlockTileSheet");
        if (sheet.getSize().x > 0) {
            entity->setTexture(sheet);
        } else {
            entity->setTexture(assets.getTexture("FireFlower"));
        }
        return entity;
    });
    auto starCreator = [](const sf::Vector2f& pos) {
        auto entity = std::make_unique<StarItem>(pos.x, pos.y);
        entity->setTexture(AssetManager::getInstance().getTexture("Starman"));
        return entity;
    };
    registerType("StarItem", starCreator);
    registerType("Star", starCreator);

    initialized = true;
}
EntityFactory& EntityFactory::getInstance() {
    static EntityFactory instance;
    return instance;
}

void EntityFactory::registerType(const std::string& typeName, CreatorFunc creator) {
    // Stores the creator lambda in the map using the type string as the key.
    // E.g. "Goomba" -> [] (const sf::Vector2f& p) { return std::make_unique<Goomba>(p); }
    m_registry[typeName] = creator;
}

std::unique_ptr<Entity> EntityFactory::create(const std::string& typeName, const sf::Vector2f& position) {
    // Look up the creator function in O(1) time
    auto it = m_registry.find(typeName);
    
    if (it != m_registry.end()) {
        // We found the lambda! Call it to instantiate the dynamic Entity.
        // Pure Factory Pattern - absolutely no if-else or switch-case checks.
        return it->second(position);
    }
    
    // Handle gracefully if an unrecognized ID is parsed from the map.
    return nullptr;
}
