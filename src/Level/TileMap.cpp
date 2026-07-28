#include "Level/TileMap.h"
#include "Level/Tile.h"
#include "Level/TileType.h"
#include "Level/Camera.h" 
#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include "Core/AssetManager.h"

TileMap::TileMap() : m_tileSize(16), m_needsRedraw(true) {
    // 1. Initialize Double Buffering sf::RenderTexture system
    // Using 320x240 to perfectly match our retro 4:3 Camera view
    if (!m_frontBuffer.create(320, 240) || !m_backBuffer.create(320, 240)) {
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

    // Helper to safely load textures regardless of CWD.
    // Probes each candidate path with std::filesystem::exists first so
    // SFML doesn't print spurious errors. Logs result to debug_log.txt.
    static std::ofstream* pLog = nullptr;
    std::ofstream assetLog("asset_log.txt", std::ios::trunc);
    pLog = &assetLog;

    auto safeLoadTexture = [](const std::string& name, const std::string& path) {
        const std::string sub = "assets/maps/Mario Game Assets/" + path;
        const std::vector<std::string> candidates = {
            sub,
            "../" + sub,
            "../../" + sub,
            "../../../" + sub
        };
        for (const auto& candidate : candidates) {
            if (std::filesystem::exists(candidate)) {
                bool ok = AssetManager::getInstance()->loadTexture(name, candidate);
                if (pLog) *pLog << (ok ? "[OK]  " : "[ERR] ") << name << " -> " << candidate << "\n";
                return;
            }
        }
        if (pLog) *pLog << "[MISS] " << name << " -> " << path << " (not found in any candidate path)\n";
        std::cerr << "TileMap: Could not locate asset: " << path << std::endl;
    };

    auto ground = std::make_shared<TileType>();
    ground->isSolid = true;
    ground->textureRect = sf::IntRect(0, 0, 16, 16);
    safeLoadTexture("Ground", "GroundBlock.png");
    ground->texture = &AssetManager::getInstance()->getTexture("Ground");
    m_tileRegistry["ground"] = ground;

    auto brick = std::make_shared<TileType>();
    brick->isSolid = true;
    brick->textureRect = sf::IntRect(0, 0, 16, 16);
    safeLoadTexture("Brick", "Brick.png");
    brick->texture = &AssetManager::getInstance()->getTexture("Brick");
    m_tileRegistry["brick"] = brick;
    
    auto warpDown = std::make_shared<TileType>();
    warpDown->isSolid = true;
    warpDown->isWarpPipe = true;
    warpDown->warpDirection = 1; 
    warpDown->textureRect = sf::IntRect(0, 0, 32, 16); 
    safeLoadTexture("PipeTop", "PipeTop.png");
    warpDown->texture = &AssetManager::getInstance()->getTexture("PipeTop");
    m_tileRegistry["pipetop"] = warpDown;

    auto pipeBottom = std::make_shared<TileType>();
    pipeBottom->isSolid = true;
    pipeBottom->textureRect = sf::IntRect(0, 0, 32, 16); 
    safeLoadTexture("PipeBottom", "PipeBottom.png");
    pipeBottom->texture = &AssetManager::getInstance()->getTexture("PipeBottom");
    m_tileRegistry["pipebottom"] = pipeBottom;

    auto mystery = std::make_shared<TileType>();
    mystery->isSolid = true;
    mystery->textureRect = sf::IntRect(0, 0, 16, 16);
    safeLoadTexture("MysteryBlock", "MysteryBlock.png");
    mystery->texture = &AssetManager::getInstance()->getTexture("MysteryBlock");
    m_tileRegistry["question"] = mystery;

    auto hardBlock = std::make_shared<TileType>();
    hardBlock->isSolid = true;
    hardBlock->textureRect = sf::IntRect(0, 0, 16, 16);
    safeLoadTexture("HardBlock", "HardBlock.png");
    hardBlock->texture = &AssetManager::getInstance()->getTexture("HardBlock");
    m_tileRegistry["hardblock"] = hardBlock;

    auto castle = std::make_shared<TileType>();
    castle->textureRect = sf::IntRect(0, 0, 80, 80);
    safeLoadTexture("Castle", "Castle.png");
    castle->texture = &AssetManager::getInstance()->getTexture("Castle");
    m_tileRegistry["castle"] = castle;

    auto flagpole = std::make_shared<TileType>();
    flagpole->textureRect = sf::IntRect(0, 0, 16, 160);
    safeLoadTexture("FlagPole", "FlagPole.png");
    flagpole->texture = &AssetManager::getInstance()->getTexture("FlagPole");
    m_tileRegistry["flagpole"] = flagpole;

    auto cloud1 = std::make_shared<TileType>();
    cloud1->textureRect = sf::IntRect(0, 0, 32, 32);
    safeLoadTexture("Cloud1", "Cloud1.png");
    cloud1->texture = &AssetManager::getInstance()->getTexture("Cloud1");
    m_tileRegistry["cloud"] = cloud1;

    auto hill1 = std::make_shared<TileType>();
    hill1->textureRect = sf::IntRect(0, 0, 48, 24);
    safeLoadTexture("Hill1", "Hill1.png");
    hill1->texture = &AssetManager::getInstance()->getTexture("Hill1");
    m_tileRegistry["hill"] = hill1;

    auto bush1 = std::make_shared<TileType>();
    bush1->textureRect = sf::IntRect(0, 0, 32, 32);
    safeLoadTexture("Bush1", "Bush1.png");
    bush1->texture = &AssetManager::getInstance()->getTexture("Bush1");
    m_tileRegistry["bush"] = bush1;
    
    // As per user prompt, let me also map "coin"
    auto coin = std::make_shared<TileType>();
    coin->textureRect = sf::IntRect(0, 0, 16, 16);
    safeLoadTexture("Coin", "Coin.png");
    coin->texture = &AssetManager::getInstance()->getTexture("Coin");
    m_tileRegistry["coin"] = coin;

    auto undergroundBlock = std::make_shared<TileType>();
    undergroundBlock->isSolid = true;
    undergroundBlock->textureRect = sf::IntRect(0, 0, 16, 16);
    safeLoadTexture("UndergroundBlock", "UndergroundBlock.png");
    undergroundBlock->texture = &AssetManager::getInstance()->getTexture("UndergroundBlock");
    m_tileRegistry["underworld_ground"] = undergroundBlock;
    m_tileRegistry["undergroundblock"] = undergroundBlock;

    auto undergroundBrick = std::make_shared<TileType>();
    undergroundBrick->isSolid = true;
    undergroundBrick->textureRect = sf::IntRect(0, 0, 16, 16);
    safeLoadTexture("UndergroundBrick", "UndergroundBrick.png");
    undergroundBrick->texture = &AssetManager::getInstance()->getTexture("UndergroundBrick");
    m_tileRegistry["undergroundbrick"] = undergroundBrick;

    auto emptyBlock = std::make_shared<TileType>();
    emptyBlock->isSolid = true;
    emptyBlock->textureRect = sf::IntRect(0, 0, 16, 16);
    safeLoadTexture("EmptyBlock", "EmptyBlock.png");
    emptyBlock->texture = &AssetManager::getInstance()->getTexture("EmptyBlock");
    m_tileRegistry["emptyblock"] = emptyBlock;

    auto flag = std::make_shared<TileType>();
    flag->textureRect = sf::IntRect(0, 0, 16, 16);
    safeLoadTexture("Flag", "Flag.png");
    flag->texture = &AssetManager::getInstance()->getTexture("Flag");
    m_tileRegistry["flag"] = flag;

    auto pipeConnection = std::make_shared<TileType>();
    pipeConnection->textureRect = sf::IntRect(0, 0, 32, 16);
    safeLoadTexture("PipeConnection", "PipeConnection.png");
    pipeConnection->texture = &AssetManager::getInstance()->getTexture("PipeConnection");
    m_tileRegistry["pipeconnection"] = pipeConnection;

    auto coinUnderground = std::make_shared<TileType>();
    coinUnderground->textureRect = sf::IntRect(0, 0, 16, 16);
    safeLoadTexture("Coin_Underground", "Coin_Underground.png");
    coinUnderground->texture = &AssetManager::getInstance()->getTexture("Coin_Underground");
    m_tileRegistry["coin_underground"] = coinUnderground;

    auto cloud2 = std::make_shared<TileType>();
    cloud2->textureRect = sf::IntRect(0, 0, 48, 32);
    safeLoadTexture("Cloud2", "Cloud2.png");
    cloud2->texture = &AssetManager::getInstance()->getTexture("Cloud2");
    m_tileRegistry["cloud2"] = cloud2;

    auto cloud3 = std::make_shared<TileType>();
    cloud3->textureRect = sf::IntRect(0, 0, 64, 32);
    safeLoadTexture("Cloud3", "Cloud3.png");
    cloud3->texture = &AssetManager::getInstance()->getTexture("Cloud3");
    m_tileRegistry["cloud3"] = cloud3;

    auto hill2 = std::make_shared<TileType>();
    hill2->textureRect = sf::IntRect(0, 0, 80, 35);
    safeLoadTexture("Hill2", "Hill2.png");
    hill2->texture = &AssetManager::getInstance()->getTexture("Hill2");
    m_tileRegistry["hill2"] = hill2;

    auto bush2 = std::make_shared<TileType>();
    bush2->textureRect = sf::IntRect(0, 0, 48, 32);
    safeLoadTexture("Bush2", "Bush2.png");
    bush2->texture = &AssetManager::getInstance()->getTexture("Bush2");
    m_tileRegistry["bush2"] = bush2;

    auto bush3 = std::make_shared<TileType>();
    bush3->textureRect = sf::IntRect(0, 0, 64, 32);
    safeLoadTexture("Bush3", "Bush3.png");
    bush3->texture = &AssetManager::getInstance()->getTexture("Bush3");
    m_tileRegistry["bush3"] = bush3;
}

bool TileMap::readFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "TileMap: Could not open " << filepath << "\n";
        return false;
    }

    int height = 0, width = 0;
    if (!(file >> height >> width)) {
        std::cerr << "TileMap: Failed to read Height and Width from " << filepath << "\n";
        return false;
    }

    m_grid.clear();
    m_grid.reserve(height);

    for (int y = 0; y < height; ++y) {
        std::vector<std::unique_ptr<Tile>> row;
        row.reserve(width);
        
        for (int x = 0; x < width; ++x) {
            std::string blockName;
            if (!(file >> blockName)) {
                std::cerr << "TileMap: Unexpected EOF at x=" << x << " y=" << y << "\n";
                break;
            }
            
            auto it = m_tileRegistry.find(blockName);
            if (blockName != "A" && it != m_tileRegistry.end()) {
                sf::Vector2f position(static_cast<float>(x * m_tileSize), static_cast<float>(y * m_tileSize));
                row.push_back(std::make_unique<Tile>(it->second.get(), position));
            } else {
                row.push_back(nullptr); // Empty space (Air)
            }
        }
        m_grid.push_back(std::move(row));
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

    sf::View view = camera.getView();
    
    // IMPORTANT: Set the back buffer to use the exact same view as the camera
    // so world coordinates are properly drawn to the texture!
    m_backBuffer.setView(view);
    m_backBuffer.clear(sf::Color::Transparent);
    
    // Calculate view bounds from the Camera's sf::View
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
