#include "Level/TileMap.h"
#include "Level/Tile.h"
#include "Level/TileType.h"
#include "Core/AssetManager.h"
#include <cmath>
#include <sstream>

TileMap::TileMap() : m_tileSize(16), m_needsRedraw(true) {
    initFlyweights();
}

TileMap::~TileMap() = default;

void TileMap::initFlyweights() {
    AssetManager& assets = AssetManager::getInstance();
    assets.loadLevelAssets();
    const sf::Texture& sheet = assets.getTexture("BlockTileSheet");

    auto add = [&](const std::string& key, const sf::Texture* tex, int left, int top, int width, int height, bool solid, bool warp = false, int dir = 0) {
        auto t = std::make_shared<TileType>();
        t->texture = tex;
        t->textureRect = sf::IntRect(left, top, width, height);
        t->isSolid = solid;
        t->isWarpPipe = warp;
        t->warpDirection = dir;
        m_tileRegistry[key] = t;
    };

    // 1. Ground Block (Overworld)
    const sf::Texture* groundTex = &assets.getTexture("GroundBlock");
    add("X", groundTex, 0, 0, 16, 16, true);

    // 2. Stair / Hard Block (solid non-destructible block for stairs/pyramids & flagpole base)
    const sf::Texture* hardBlockTex = &assets.getTexture("HardBlock");
    add("B", hardBlockTex, 0, 0, 16, 16, true);

    // 3. Destructible Brick Block ('S')
    const sf::Texture* brickTex = &assets.getTexture("Brick");
    add("S", brickTex, 0, 0, 16, 16, true);
    m_tileRegistry["S"]->isBrick = true;

    // 4. Question Block ('?', 'Q') and Empty Block ('E')
    const sf::Texture* mysteryTex = &assets.getTexture("MysteryBlock");
    add("?", mysteryTex, 0, 0, 16, 16, true);
    m_tileRegistry["?"]->isQuestionBlock = true;
    m_tileRegistry["?"]->isAnimated = true;

    add("Q", mysteryTex, 0, 0, 16, 16, true);
    m_tileRegistry["Q"]->isQuestionBlock = true;
    m_tileRegistry["Q"]->isAnimated = true;

    const sf::Texture* emptyTex = &assets.getTexture("EmptyBlock");
    add("E", emptyTex, 0, 0, 16, 16, true);

    // 5. Seamless Pipe parts (<, >, [, ])
    const sf::Texture* pipeTopTex = &assets.getTexture("PipeTop");
    const sf::Texture* pipeBottomTex = &assets.getTexture("PipeBottom");
    add("<", pipeTopTex, 0, 0, 16, 16, true, true, 1);
    add(">", pipeTopTex, 16, 0, 16, 16, true, true, 1);
    add("[", pipeBottomTex, 0, 0, 16, 16, true, true, 1);
    add("]", pipeBottomTex, 16, 0, 16, 16, true, true, 1);
    const sf::Texture* pipeConnTex = &assets.getTexture("PipeConnection");
    add("1", pipeConnTex, 0, 16, 16, 16, true, true, 1);
    add("2",pipeConnTex,16,16,16,16,true,true,1);
    add("3",pipeConnTex,0,32,16,16,true,true,1);
    add("4",pipeConnTex,16,32,16,16,true,true,1);
    add("5",pipeConnTex,32,0,16,16,true,true,1);
    add("6",pipeConnTex,32,16,16,16,true,true,1);
    add("7",pipeConnTex,32,32,16,16,true,true,1);
    add("0",pipeConnTex,48,0,16,16,true,true,1);

    // 'a' = horizontal auto-entry warp pipe (used for Pipe A and Pipe C1 in 1-2)
    // Visually identical to '<' but triggers warp on horizontal contact, not Down key.
    add("a", pipeTopTex, 0, 0, 16, 16, true, true, 2);
    m_tileRegistry["a"]->isHorizontalWarpPipe = true;
    

    // 5. End-Level Elements (Castle, FlagPole, Flag)
    const sf::Texture* castleTex = &assets.getTexture("Castle");
    const sf::Texture* largeCastleTex = &assets.getTexture("LargeCastle");
    const sf::Texture* flagPoleTex = &assets.getTexture("FlagPole");
    const sf::Texture* flagTex = &assets.getTexture("Flag");
    add("j", castleTex, 0, 0, 80, 80, false);
    add("C", largeCastleTex, 0, 0, 148, 176, false);
    add("P", flagPoleTex, 0, 0, 16, 16, false);
    add("|", flagPoleTex, 0, 16, 16, 16, false);
    add("F", flagTex, 0, 0, 16, 16, false);

    // 6. Underground Specific Tiles (UndergroundBlock, UndergroundBrick, Coin_Underground)
    const sf::Texture* ugBlockTex = &assets.getTexture("UndergroundBlock");
    const sf::Texture* ugBrickTex = &assets.getTexture("UndergroundBrick");
    const sf::Texture* ugCoinTex = &assets.getTexture("Coin_Underground");
    const sf::Texture* upHardBlockTex= &assets.getTexture("UndergroundHardBlock");
    add("u", ugBlockTex, 0, 0, 16, 16, true);
    add("r", ugBrickTex, 0, 0, 16, 16, true);
    m_tileRegistry["r"]->isBrick = true;
    add("c", ugCoinTex, 0, 0, 16, 16, false);
    add("g",upHardBlockTex,0,0,16,16,true);
    m_tileRegistry["c"]->isCoinTile = true;
    
    // Background Black Tile for 1-2
    add("*", &assets.getTexture("BlackTile"), 0, 0, 16, 16, false);
    
    // Warp Text (mapped as solid bricks for layout, or hard blocks)
    add("W", &assets.getTexture("Brick"), 0, 0, 16, 16, true); // Placeholder for Warp Text

    // 7. Auto-generated mappings for large scenery objects
    auto addMulti = [&](const std::string& prefix, const sf::Texture* tex, int cols, int rows, bool isHill1 = false) {
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                // 1-based indexing for the characters
                std::string key = prefix + std::to_string(r * cols + c + 1);
                int h = (isHill1 && r == 1) ? 8 : 16;
                add(key, tex, c * 16, r * 16, 16, h, false);
            }
        }
    };

    addMulti("bu", &assets.getTexture("Bush1"), 2, 2);
    addMulti("Bu", &assets.getTexture("Bush2"), 3, 2);
    addMulti("bU", &assets.getTexture("Bush3"), 4, 2);
    addMulti("cl", &assets.getTexture("Cloud1"), 2, 2);
    addMulti("Cl", &assets.getTexture("Cloud2"), 3, 2);
    addMulti("cL", &assets.getTexture("Cloud3"), 4, 2);
    addMulti("h", &assets.getTexture("Hill1"), 3, 2, true);
    addMulti("H", &assets.getTexture("Hill2"), 5, 3);
    addMulti("isl", &assets.getTexture("SpriteIsland"), 4, 3);
    
    add("uh", &assets.getTexture("UndergroundHardBlock"), 0, 0, 16, 16, true);
    
    const sf::Texture* platformTex = &assets.getTexture("Platform");
    add("D", platformTex, 0, 0, 16, 8, true);
    add("M", platformTex, 16, 0, 16, 8, true);
    add("N", platformTex, 32, 0, 16, 8, true);
    
    // Athletic Level Specific (1-3)
    add("T", &assets.getTexture("SpriteIsland"), 16, 32, 16, 16, true); // Tree Trunk (solid)
    add("8", &assets.getTexture("SpriteIsland"), 0, 0, 16, 16, true);  // Green Cap
    add("G", &assets.getTexture("SpriteIsland"), 16, 0, 16, 16, true);
    add("9", &assets.getTexture("SpriteIsland"), 48, 0, 16, 16, true);
    add("O", platformTex, 16, 0, 16, 8, true);                          // Orange Wood Platform
}

bool TileMap::readFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "TileMap: Failed to open " << filepath << std::endl;
        return false;
    }

    m_grid.clear();
    std::string line;

    // Check if first line contains numeric dimensions header (e.g. "15 16")
    std::streampos pos = file.tellg();
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        int rows, cols;
        if (ss >> rows >> cols && rows > 0 && cols > 0 && line.find_first_not_of("0123456789 \t\r\n") == std::string::npos) {
            // Valid header line, leave consumed
        } else {
            // Not a header line, rewind to start of file so row 0 is parsed as map data
            file.clear();
            file.seekg(pos);
        }
    }

    int y = 0;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::vector<std::unique_ptr<Tile>> row;

        // Any line containing spaces uses space-separated tokenization (std::stringstream)
        bool isSpaceSeparated = (line.find(' ') != std::string::npos);

        if (isSpaceSeparated) {
            std::stringstream ss(line);
            std::string token;
            int x = 0;
            while (ss >> token) {
                if (token == "A" || token == "." || token == "-") {
                    row.push_back(nullptr);
                } else {
                    auto it = m_tileRegistry.find(token);
                    if (it != m_tileRegistry.end()) {
                        // Skip static tile rendering for Platform entities
                        if (token == "O") {
                            row.push_back(nullptr);
                        } else {
                            float yOff = m_tileOffset.y;
                            if (token[0] == 'h' || (token.length() >= 2 && (token.substr(0, 2) == "bu" || token.substr(0, 2) == "Bu" || token.substr(0, 2) == "bU"))) {
                                yOff -= 4.f;
                            }
                            if (token[0] == 'H') yOff += 4.f;           // Hill1 moves down 4.f
                            
                            row.push_back(std::make_unique<Tile>(
                                it->second.get(),
                                sf::Vector2f(float(x * m_tileSize) + m_tileOffset.x, float(y * m_tileSize) + yOff)));
                        }
                    } else {
                        std::shared_ptr<TileType> typeToUse = nullptr;
                        if (token == "ground1") typeToUse = m_tileRegistry["u"];
                        else if (token == "brick2") typeToUse = m_tileRegistry["r"];
                        else if (token == "coin1") typeToUse = m_tileRegistry["c"];
                        else if (token.rfind("P", 0) == 0) typeToUse = m_tileRegistry["["];

                        if (typeToUse) {
                            row.push_back(std::make_unique<Tile>(
                                typeToUse.get(),
                                sf::Vector2f(float(x * m_tileSize) + m_tileOffset.x, float(y * m_tileSize) + m_tileOffset.y)));
                        } else {
                            std::cerr << "TileMap Warning: Unregistered token '" << token << "' at row " << y << ", col " << x << " in " << filepath << std::endl;
                            row.push_back(nullptr);
                        }
                    }
                }
                ++x;
            }
        } else {
            for (size_t x = 0; x < line.size(); ++x) {
                char c = line[x];
                if (c == '-' || c == ' ' || c == '.') {
                    row.push_back(nullptr);
                    continue;
                }
                // 'O' tiles are moving platform spawn points — skip static tile, record position
                if (c == 'O') {
                    m_platformSpawnPoints.push_back(sf::Vector2f(
                        float(x * m_tileSize) + m_tileOffset.x,
                        float(y * m_tileSize) + m_tileOffset.y));
                    row.push_back(nullptr);
                    continue;
                }
                auto it = m_tileRegistry.find(std::string(1, c));
                if (it != m_tileRegistry.end()) {
                    row.push_back(std::make_unique<Tile>(
                        it->second.get(),
                        sf::Vector2f(float(x * m_tileSize) + m_tileOffset.x, float(y * m_tileSize) + m_tileOffset.y)));
                } else {
                    if (c != 'e' && c != 'E' && c != 'o') {
                        std::cerr << "TileMap Warning: Unregistered char '" << c << "' at row " << y << ", col " << x << " in " << filepath << std::endl;
                    }
                    row.push_back(nullptr);
                }
            }
        }
        m_grid.push_back(std::move(row));
        y++;
    }
    m_needsRedraw = true;
    m_frontBuffer.create(320, 240);
    m_backBuffer.create(320, 240);
    m_frontBuffer.setSmooth(false);
    m_backBuffer.setSmooth(false);

    std::cout << "TileMap: Loaded " << m_grid.size() << " rows from " << filepath << std::endl;
    return !m_grid.empty();
}

void TileMap::updateBuffer(const Camera& camera) {
    const sf::View& view = camera.getView();
    sf::Vector2f sz = view.getSize();
    sf::Vector2f ctr = view.getCenter();

    unsigned bw = (unsigned)std::ceil(sz.x);
    unsigned bh = (unsigned)std::ceil(sz.y);
    if (m_backBuffer.getSize().x != bw || m_backBuffer.getSize().y != bh) {
        m_backBuffer.create(bw, bh);
        m_frontBuffer.create(bw, bh);
        m_backBuffer.setSmooth(false);
        m_frontBuffer.setSmooth(false);
    }

    sf::FloatRect bounds(ctr.x - sz.x * 0.5f, ctr.y - sz.y * 0.5f, sz.x, sz.y);

    m_backBuffer.clear(sf::Color::Transparent);
    sf::View bufView;
    bufView.setSize(sz);
    bufView.setCenter(ctr);
    m_backBuffer.setView(bufView);

    for (Tile* t : getTilesInBounds(bounds))
        if (t) t->render(m_backBuffer);

    m_backBuffer.display();

    m_frontBuffer.clear(sf::Color::Transparent);
    m_frontBuffer.setView(m_frontBuffer.getDefaultView());
    m_frontBuffer.draw(sf::Sprite(m_backBuffer.getTexture()));
    m_frontBuffer.display();

    m_needsRedraw = false;
}

void TileMap::render(sf::RenderTarget& target, const Camera& camera) {
    updateBuffer(camera);
    const sf::View& view = camera.getView();
    sf::Vector2f sz = view.getSize();
    sf::Vector2f ctr = view.getCenter();
    sf::Sprite spr(m_frontBuffer.getTexture());
    spr.setPosition(ctr.x - sz.x * 0.5f, ctr.y - sz.y * 0.5f);
    target.draw(spr);
}

std::vector<Tile*> TileMap::getTilesInBounds(const sf::FloatRect& bounds) const {
    std::vector<Tile*> out;
    if (m_grid.empty()) return out;
    int x0 = std::max(0, (int)std::floor(bounds.left / m_tileSize) - 1);
    int y0 = std::max(0, (int)std::floor(bounds.top  / m_tileSize) - 1);
    int x1 = (int)std::ceil((bounds.left + bounds.width)  / m_tileSize) + 1;
    int y1 = (int)std::ceil((bounds.top  + bounds.height) / m_tileSize) + 1;
    int rows = (int)m_grid.size();
    for (int y = y0; y < y1 && y < rows; ++y) {
        if (y < 0) continue;
        int cols = (int)m_grid[y].size();
        for (int x = x0; x < x1 && x < cols; ++x)
            if (x >= 0 && m_grid[y][x]) out.push_back(m_grid[y][x].get());
    }
    return out;
}

void TileMap::setNeedsRedraw(bool v) { m_needsRedraw = v; }

void TileMap::hitTile(Tile* tile) {
    if (!tile) return;
    auto it = m_tileRegistry.find("E");
    if (it != m_tileRegistry.end()) {
        tile->setType(it->second.get());
        m_needsRedraw = true;
    }
}

void TileMap::removeTile(Tile* tile) {
    if (!tile) return;
    for (auto& row : m_grid) {
        for (auto& t : row) {
            if (t.get() == tile) {
                t.reset();
                m_needsRedraw = true;
                return;
            }
        }
    }
}

void TileMap::update(float dt) {
    for (auto& row : m_grid) {
        for (auto& tile : row) {
            if (tile) {
                tile->update(dt);
            }
        }
    }
}

void TileMap::breakBrick(Tile* tile) {
    if (!tile) return;
    
    sf::FloatRect bounds = tile->getBounds();
    float cx = bounds.left + bounds.width * 0.5f;
    float cy = bounds.top + bounds.height * 0.5f;
    
    // Spawn 4 debris pieces flying in different directions
    // Top-left piece
    m_debris.push_back({{bounds.left, bounds.top}, {-60.f, -250.f}, 0.f, 400.f, 0.f, true});
    // Top-right piece
    m_debris.push_back({{bounds.left + 8.f, bounds.top}, {60.f, -250.f}, 0.f, -350.f, 0.f, true});
    // Bottom-left piece
    m_debris.push_back({{bounds.left, bounds.top + 8.f}, {-50.f, -180.f}, 0.f, 300.f, 0.f, true});
    // Bottom-right piece
    m_debris.push_back({{bounds.left + 8.f, bounds.top + 8.f}, {50.f, -180.f}, 0.f, -280.f, 0.f, true});
    
    removeTile(tile);
}

void TileMap::updateDebris(float dt) {
    const float gravity = 980.f;
    
    for (auto& d : m_debris) {
        if (!d.active) continue;
        
        d.velocity.y += gravity * dt;
        d.position += d.velocity * dt;
        d.rotation += d.rotationSpeed * dt;
        d.lifetime += dt;
        
        // Remove after 2 seconds or if fallen off screen
        if (d.lifetime > 2.f || d.position.y > 500.f) {
            d.active = false;
        }
    }
    
    // Cleanup inactive debris
    m_debris.erase(
        std::remove_if(m_debris.begin(), m_debris.end(),
            [](const BrickDebris& d) { return !d.active; }),
        m_debris.end()
    );
}

void TileMap::renderDebris(sf::RenderTarget& target) const {
    AssetManager& assets = AssetManager::getInstance();
    const sf::Texture& brickTex = assets.getTexture("Brick");
    
    for (const auto& d : m_debris) {
        if (!d.active) continue;
        
        // Each debris piece is an 8x8 quarter of the brick texture
        sf::Sprite sprite(brickTex, sf::IntRect(0, 0, 8, 8));
        sprite.setOrigin(4.f, 4.f);
        sprite.setPosition(d.position);
        sprite.setRotation(d.rotation);
        target.draw(sprite);
    }
}