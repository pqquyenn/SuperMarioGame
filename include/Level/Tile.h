#pragma once
#include <SFML/Graphics.hpp>
#include "Level/TileType.h"

/**
 * @brief Extrinsic State for a Tile
 * 
 * Represents a specific tile on the map. It holds its position in the world 
 * and a pointer to the shared Flyweight (TileType) for its appearance and rules.
 */
class Tile {
public:
    /**
     * @brief Construct a new Tile object
     * @param type Pointer to the shared Flyweight state
     * @param position Extrinsic position in the world
     */
    Tile(const TileType* type, const sf::Vector2f& position);

    /**
     * @brief Renders the tile by delegating to the Flyweight type
     */
    void render(sf::RenderTarget& target) const;

    /**
     * @brief Returns the AABB boundary for collision detection
     */
    sf::FloatRect getBounds() const;
    
    // --- Delegates to the Flyweight type ---
    bool isSolid() const;
    bool isWarpPipe() const;
    int getWarpDirection() const;

private:
    const TileType* m_type;   // Pointer to shared intrinsic state
    sf::Vector2f m_position;  // Extrinsic world position
};
