#pragma once
#include <SFML/Graphics/Rect.hpp>

// Forward Declarations
class Entity;
class TileMap;
class Level;
class Character;

/**
 * @brief Handles AABB collision detection and polymorphic resolution.
 * 
 * Decouples collision math from game logic. Post-collision responses are deferred 
 * back to the Entity objects via virtual method calls.
 */
class CollisionManager {
public:
    /**
     * @brief Axis-Aligned Bounding Box (AABB) intersection check
     * @param a Bounds of object A
     * @param b Bounds of object B
     * @param overlap Populated with the intersected rectangle if a collision occurs
     * @return true if overlapping
     */
    static bool checkAABB(const sf::FloatRect& a, const sf::FloatRect& b, sf::FloatRect& overlap);
    
    /**
     * @brief Checks an entity against the nearby grid in TileMap and resolves position
     * @param entity The entity moving through the map
     * @param map The static level geometry
     */
    static void resolveTileCollisions(Entity& entity, TileMap& map, Level* level = nullptr);
    
    /**
     * @brief Resolves collisions between two dynamic entities
     * Uses double-dispatch or simple callback logic on the Entity class.
     */
    static void resolveEntityCollisions(Entity& a, Entity& b);

    /**
     * @brief Attempts to enter a downward warp whose top surface supports the
     * character. Returns true when a teleport occurs.
     */
    static bool tryEnterDownWarp(Character& character, Level& level);

    /**
     * @brief Attempts to enter a right-activated data-driven warp while the
     * character overlaps its trigger. Used by collision-free debug flight.
     */
    static bool tryEnterRightWarp(Character& character, Level& level);

    /**
     * @brief Checks if a character is standing on a MovingPlatform and carries it.
     * Call once per platform per frame after tile collisions are resolved.
     */
    static void resolveMovingPlatform(Character& character, class MovingPlatform& platform);
};
