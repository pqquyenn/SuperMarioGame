#include "Level/TileMap.h"
#include "Level/Tile.h"
#include "Level/TileType.h"
#include "Level/Camera.h" 
#include <fstream>
#include <iostream>
#include <algorithm>

TileMap::TileMap() : m_tileSize(16), m_needsRedraw(true) {
    // 1. Initialize Double Buffering sf::RenderTexture system
    // Using 800x600 as the assumed viewport size
    if (!m_frontBuffer.create(800, 600) || !m_backBuffer.create(800, 600)) {
        std::cerr << "TileMap: Failed to initialize double buffers!" << std::endl;
    }
    
    m_frontBuffer.clear(sf::Color::Transparent);
    m_backBuffer.clear(sf::Color::Transparent);

    // 2. Initialize our Flyweights immediately upon creation
    initFlyweights();
}

TileMap::~TileMap() {}

void TileMap::initFlyweights() {
    // This is the core of the Flyweight Pattern! 
    // We instantiate these complex properties ONLY ONCE.

    auto ground = std::make_shared<TileType>();
    ground->isSolid = true;
    ground->textureRect = sf::IntRect(0, 0, 16, 16);
    m_tileRegistry['G'] = ground;

    auto brick = std::make_shared<TileType>();
    brick->isSolid = true;
    brick->textureRect = sf::IntRect(16, 0, 16, 16);
    m_tileRegistry['B'] = brick;
    
    auto warpDown = std::make_shared<TileType>();
    warpDown->isSolid = true;
    warpDown->isWarpPipe = true;
    warpDown->warpDirection = 1; // Down
    warpDown->textureRect = sf::IntRect(32, 0, 16, 16);
    m_tileRegistry['1'] = warpDown;
    
    // Note: To render visually, you would load an sf::Texture here:
    // m_spriteSheet.loadFromFile("assets/tileset.png");
    // ground->texture = &m_spriteSheet; ...
}

bool TileMap::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "TileMap: Could not open " << filepath << "\n";
        return false;
    }

    std::string line;
    int y = 0;
    m_grid.clear();

    while (std::getline(file, line)) {
        std::vector<std::unique_ptr<Tile>> row;
        for (size_t x = 0; x < line.length(); ++x) {
            char c = line[x];
            auto it = m_tileRegistry.find(c);
            
            if (it != m_tileRegistry.end()) {
                // Create Extrinsic Tile -> Store Pointer to Intrinsic Flyweight
                sf::Vector2f position(static_cast<float>(x * m_tileSize), static_cast<float>(y * m_tileSize));
                row.push_back(std::make_unique<Tile>(it->second.get(), position));
            } else {
                row.push_back(nullptr); // Empty space (Air)
            }
        }
        m_grid.push_back(std::move(row));
        y++;
    }

    m_needsRedraw = true; // Force redraw on initial load
    return true;
}

void TileMap::setNeedsRedraw(bool needsRedraw) {
    m_needsRedraw = needsRedraw;
}

std::vector<Tile*> TileMap::getTilesInBounds(const sf::FloatRect& bounds) const {
    std::vector<Tile*> result;
    if (m_grid.empty()) return result;

    // Convert world bounds to grid cell indices
    int startX = std::max(0, static_cast<int>(bounds.left / m_tileSize));
    int startY = std::max(0, static_cast<int>(bounds.top / m_tileSize));
    
    int maxGridY = static_cast<int>(m_grid.size()) - 1;
    int maxGridX = static_cast<int>(m_grid[0].size()) - 1;
    
    int endX = std::min(maxGridX, static_cast<int>((bounds.left + bounds.width) / m_tileSize));
    int endY = std::min(maxGridY, static_cast<int>((bounds.top + bounds.height) / m_tileSize));

    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            if (m_grid[y][x]) {
                result.push_back(m_grid[y][x].get());
            }
        }
    }
    return result;
}

void TileMap::updateBuffer(const Camera& camera) {
    if (!m_needsRedraw) return;

    m_backBuffer.clear(sf::Color::Transparent);
    
    // Calculate view bounds from the Camera's sf::View
    sf::View view = camera.getView();
    sf::FloatRect viewBounds(
        view.getCenter().x - view.getSize().x / 2.0f,
        view.getCenter().y - view.getSize().y / 2.0f,
        view.getSize().x,
        view.getSize().y
    );
    
    std::vector<Tile*> visibleTiles = getTilesInBounds(viewBounds);
    
    for (Tile* tile : visibleTiles) {
        tile->render(m_backBuffer);
    }
    m_backBuffer.display();
    
    // Swap Logic: Copy back buffer to front buffer to act as final render
    m_frontBuffer.clear(sf::Color::Transparent);
    sf::Sprite swapSprite(m_backBuffer.getTexture());
    m_frontBuffer.draw(swapSprite);
    m_frontBuffer.display();

    m_needsRedraw = false;
}

void TileMap::render(sf::RenderTarget& target, const Camera& camera) {
    // Re-render internally ONLY if the camera scrolled heavily or map changed
    updateBuffer(camera);

    // This is the Double Buffer Magic: 
    // Instead of drawing 1000s of Tiles, we draw exactly 1 Sprite!
    sf::Sprite finalSprite(m_frontBuffer.getTexture());
    
    // Shift sprite to follow camera position properly
    sf::View view = camera.getView();
    sf::FloatRect bounds(
        view.getCenter().x - view.getSize().x / 2.0f,
        view.getCenter().y - view.getSize().y / 2.0f,
        view.getSize().x,
        view.getSize().y
    );
    finalSprite.setPosition(bounds.left, bounds.top);
    
    target.draw(finalSprite);
}
