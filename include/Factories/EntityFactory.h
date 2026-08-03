#pragma once
#include <SFML/System/Vector2.hpp>
#include <unordered_map>
#include <functional>
#include <memory>
#include <string>

// Forward Declaration: Team members' classes are NOT #include'd here
class Entity;

/**
 * @brief Entity Factory using the Registry (Map) Pattern.
 * 
 * Eliminates switch-case/if-else logic. Entities register their own creator lambdas 
 * (e.g. at startup) which this Factory maps to a string ID (like "Goomba" or "Coin").
 */
class EntityFactory {
public:
    // The signature for creation functions (lambdas)
    using CreatorFunc = std::function<std::unique_ptr<Entity>(const sf::Vector2f&)>;

    // Singleton access
    static EntityFactory& getInstance();

    // Prevent copying and assignment
    EntityFactory(const EntityFactory&) = delete;
    EntityFactory& operator=(const EntityFactory&) = delete;
    
    // Prevent incomplete type errors if moved
    EntityFactory(EntityFactory&&) = delete;
    EntityFactory& operator=(EntityFactory&&) = delete;

    /**
     * @brief Register a new entity type
     * @param typeName The string ID for the entity (e.g., "Mario")
     * @param creator The lambda/function to instantiate the entity
     */
    void registerType(const std::string& typeName, CreatorFunc creator);

    /**
     * @brief Register default game entities (Goomba, Koopa, etc.)
     */
    void registerDefaultEntities();

    /**
     * @brief Create an entity dynamically
     * @param typeName The string ID to lookup in the registry
     * @param position The position to spawn the entity
     * @return unique_ptr to the new Entity (or nullptr if not found)
     */
    [[nodiscard]] std::unique_ptr<Entity> create(const std::string& typeName, const sf::Vector2f& position);

private:
    EntityFactory() = default;
    
    // The Map Registry that holds the creator functions
    std::unordered_map<std::string, CreatorFunc> m_registry;
};
