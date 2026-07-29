#include "Level/TileMap.h"
#include "Level/Tile.h"
#include "Level/TileType.h"
#include "Core/AssetManager.h"
#include <cmath>

TileMap::TileMap() : m_tileSize(16), m_needsRedraw(true) {
    initFlyweights();
}

TileMap::~TileMap() = default;

void TileMap::initFlyweights() {
    AssetManager* assets = AssetManager::getInstance();
    assets->loadLevelAssets();
    const sf::Texture& sheet = assets->getTexture("BlockTileSheet");

    auto add = [&](const std::string& key, int left, int top, bool solid,
                   bool warp = false, int dir = 0) {
        auto t = std::make_shared<TileType>();
        t->texture = &sheet;
        t->textureRect = sf::IntRect(left, top, 16, 16);
        t->isSolid = solid;
        t->isWarpPipe = warp;
        t->warpDirection = dir;
        m_tileRegistry[key] = t;
    };

    // Ground / stair
    add("X", 1, 137, true);
    add("x", 1, 137, true);
    add("B", 1, 137, true);
    add("D", 1, 137, true);
    add("b", 1, 137, true);
    add("d", 1, 137, true);

    // Brick
    add("S", 35, 103, true);
    add("s", 35, 103, true);

    // Question
    add("?", 1, 69, true);
    add("Q", 1, 69, true);
    add("q", 1, 69, true);

    // Pipe top / body
    add("<", 103, 52, true, true, 1);
    add(">", 119, 52, true, true, 1);
    add("[", 103, 69, true, true, 1);
    add("]", 119, 69, true, true, 1);

    // Cloud 6 tiles (3x2) from Overworld
    add("(", 232, 72, false);
    add(")", 248, 72, false);
    add("*", 264, 72, false);
    add("{", 232, 80, false);
    add("_", 248, 80, false);
    add("}", 264, 80, false);
    add("C", 248, 72, false);
    add("c", 248, 72, false);

    // Bush 4 tiles
    add("a", 64, 96, false);
    add("b", 80, 96, false);
    add("c", 96, 96, false);
    add("d", 112, 96, false);
    add("U", 80, 96, false);
    add("u", 80, 96, false);

    // Hill 9 tiles
    //   . 1 .
    //  2 3 4
    // 5 6 7 8 9
    add("1", 176, 48, false);
    add("2", 160, 80, false);
    add("3", 176, 80, false);
    add("4", 192, 80, false);
    add("5", 144, 96, false);
    add("6", 160, 96, false);
    add("7", 176, 96, false);
    add("8", 192, 96, false);
    add("9", 192, 96, false);
    add("H", 176, 96, false);
    add("h", 176, 96, false);
    add("L", 160, 96, false);
    add("M", 176, 96, false);
    add("R", 192, 96, false);

    // Flag / pole
    add("F", 480, 40, false);
    add("f", 480, 40, false);
    add("|", 103, 69, true);
}

bool TileMap::readFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "TileMap: Failed to open " << filepath << std::endl;
        return false;
    }

    m_grid.clear();
    std::string line;
    int y = 0;

    while (std::getline(file, line)) {
        std::vector<std::unique_ptr<Tile>> row;
        for (size_t x = 0; x < line.size(); ++x) {
            char c = line[x];
            if (c == '-') {
                row.push_back(nullptr);
                continue;
            }
            auto it = m_tileRegistry.find(std::string(1, c));
            if (it != m_tileRegistry.end()) {
                row.push_back(std::make_unique<Tile>(
                    it->second.get(),
                    sf::Vector2f(float(x * m_tileSize), float(y * m_tileSize))));
            } else {
                row.push_back(nullptr);
            }
        }
        m_grid.push_back(std::move(row));
        ++y;
    }

    m_needsRedraw = true;
    m_frontBuffer.create(320, 240);
    m_backBuffer.create(320, 240);
    m_frontBuffer.setSmooth(false);
    m_backBuffer.setSmooth(false);

    std::cout << "TileMap: Loaded " << m_grid.size() << " rows\n";
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