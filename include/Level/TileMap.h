#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

// Forward Declarations to prevent circular dependencies
class Tile;
struct TileType;
class Camera;

/**
 * @brief Manages the entire Level map, including Flyweight registry and grid rendering.
 * 
 * Implements a Double Buffer rendering strategy to optimize performance 
 * by rendering visible chunks to an sf::RenderTexture.
 */
class TileMap {
public:
    TileMap();
    ~TileMap();

    // Prevent incomplete type errors with std::unique_ptr<Tile> when compiler generates copy/move operations
    TileMap(const TileMap&) = delete;
    TileMap& operator=(const TileMap&) = delete;
    TileMap(TileMap&&) = delete;
    TileMap& operator=(TileMap&&) = delete;

    /**
     * @brief Loads map data from an ASCII .txt file
     * @param filepath Path to the level text file
     * @return true if successful
     */
    bool loadFromFile(const std::string& filepath);

    /**
     * @brief Updates the back buffer if a redraw is needed based on the camera view
     */
    void updateBuffer(const Camera& camera);

    /**
     * @brief Renders the front buffer to the target
     */
    void render(sf::RenderTarget& target, const Camera& camera);

    /**
     * @brief Gets all tiles intersecting a given bounding box (for physics)
     */
    std::vector<Tile*> getTilesInBounds(const sf::FloatRect& bounds) const;

    /**
     * @brief Call this when the camera scrolls significantly to trigger a buffer refresh
     */
    void setNeedsRedraw(bool needsRedraw);

private:
    /**
     * @brief Initializes the Flyweight TileType instances
     */
    void initFlyweights();

private:
    // Flyweight Registry: Maps characters like 'G' (Ground), 'B' (Brick) to their shared types
    std::unordered_map<char, std::shared_ptr<TileType>> m_tileRegistry;
    
    // The 2D grid storing unique Tile objects (which reference Flyweights)
    std::vector<std::vector<std::unique_ptr<Tile>>> m_grid;
    
    int m_tileSize;
    sf::Texture m_spriteSheet; // Shared sprite sheet for all tiles
    
    // --- Double Buffering ---
    sf::RenderTexture m_frontBuffer;
    sf::RenderTexture m_backBuffer;
    bool m_needsRedraw;
};
