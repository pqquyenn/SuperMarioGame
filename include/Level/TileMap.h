#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>

#include "Core/AssetManager.h"
#include "Level/TileType.h"
#include "Level/Tile.h"
#include "Level/Camera.h"

struct BrickDebris {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float rotation = 0.f;
    float rotationSpeed = 0.f;
    float lifetime = 0.f;
    bool active = true;
};

class TileMap {
public:
    TileMap();
    ~TileMap();

    TileMap(const TileMap&) = delete;
    TileMap& operator=(const TileMap&) = delete;
    TileMap(TileMap&&) = delete;
    TileMap& operator=(TileMap&&) = delete;

    bool readFromFile(const std::string& filepath);
    void update(float dt);
    void breakBrick(Tile* tile);
    void updateDebris(float dt);
    void renderDebris(sf::RenderTarget& target) const;
    void updateBuffer(const Camera& camera);
    void render(sf::RenderTarget& target, const Camera& camera);
    bool isSolidAt(float worldX, float worldY) const;
    std::vector<Tile*> getTilesInBounds(const sf::FloatRect& bounds) const;
    void setNeedsRedraw(bool needsRedraw);
    void setTileOffset(const sf::Vector2f& offset) { m_tileOffset = offset; }
    void hitTile(Tile* tile);
    void removeTile(Tile* tile);
    
    int getWidth() const {
        int maxW = 0;
        for (const auto& row : m_grid) {
            if ((int)row.size() > maxW) maxW = (int)row.size();
        }
        return maxW;
    }
    int getHeight() const { return (int)m_grid.size(); }

    // Named aliases used by Level and PlayState
    int getMapWidth()  const { return getWidth(); }
    int getMapHeight() const { return getHeight(); }

private:
    void initFlyweights();

private:
    std::unordered_map<std::string, std::shared_ptr<TileType>> m_tileRegistry;
    std::vector<std::vector<std::unique_ptr<Tile>>> m_grid;
    int m_tileSize;
    sf::RenderTexture m_frontBuffer;
    sf::RenderTexture m_backBuffer;
    bool m_needsRedraw;
    sf::Vector2f m_tileOffset{0.f, 0.f};
    std::vector<BrickDebris> m_debris;
};