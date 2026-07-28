#pragma once
#include <SFML/Graphics.hpp>

/**
 * @brief Flyweight Intrinsic State for Tiles
 * 
 * This struct represents the shared, intrinsic properties of a tile type.
 * By storing shared textures and properties here, we save significant memory 
 * rather than copying them for every single Tile in the grid.
 */
struct TileType {
    const sf::Texture* texture = nullptr;
    sf::IntRect textureRect;
    
    // Physics properties
    bool isSolid = false;
    
    // Skill Specific properties: Warp Pipes
    bool isWarpPipe = false;
    int warpDirection = 0; // 0: None, 1: Down (Underworld), 2: Up (Overworld)
    
    /**
     * @brief Render the intrinsic texture at an extrinsic position
     * @param target Render target (e.g., window or render texture)
     * @param position Extrinsic position from the Tile object
     */
    void render(sf::RenderTarget& target, const sf::Vector2f& position) const {
        if (!texture) return;
        sf::Sprite sprite(*texture, textureRect);
        sprite.setPosition(position);
        target.draw(sprite);
    }
};
