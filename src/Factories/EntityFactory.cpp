#include "Factories/EntityFactory.h"
#include "Entities/Entity.h"
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
