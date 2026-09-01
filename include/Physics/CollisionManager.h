#pragma once

#include <SFML/Graphics/Rect.hpp>

class Character;
class Entity;
class MovingPlatform;
class TileCollisionHandler;
class TileMap;

/**
 * @brief Coordinates generic collision detection and physical separation.
 *
 * Gameplay reactions are delegated to Entity and TileCollisionHandler hooks.
 * This keeps generic physics independent from input, level scripts, scoring,
 * and concrete enemy or item types.
 */
class CollisionManager {
public:
    static bool checkAABB(
        const sf::FloatRect& a,
        const sf::FloatRect& b,
        sf::FloatRect& overlap);

    static void resolveTileCollisions(
        Entity& entity,
        TileMap& map,
        TileCollisionHandler* tileHandler = nullptr);

    static void resolveEntityCollisions(Entity& a, Entity& b);

    /**
     * Finalizes crawl/stand state after static tiles and moving platforms have
     * both contributed their contact state for the frame.
     */
    static bool tryStandUp(Character& character, const TileMap& map);

    static void resolveMovingPlatform(
        Character& character,
        MovingPlatform& platform);
};
