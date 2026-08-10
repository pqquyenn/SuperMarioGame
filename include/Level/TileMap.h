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
#include <cstdint>

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

struct TileHandle {
    std::size_t row{0};
    std::size_t column{0};
    std::uint64_t mapRevision{0};
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
    void breakBrick(const TileHandle& handle);
    void updateDebris(float dt);
    void renderDebris(sf::RenderTarget& target) const;
    void updateBuffer(const Camera& camera);
    void render(sf::RenderTarget& target, const Camera& camera);
    std::vector<TileHandle> getTilesInBounds(const sf::FloatRect& bounds) const;
    Tile* getTile(const TileHandle& handle);
    const Tile* getTile(const TileHandle& handle) const;
    void setNeedsRedraw(bool needsRedraw);
    void setTileOffset(const sf::Vector2f& offset) { m_tileOffset = offset; }
    void hitTile(const TileHandle& handle);
    void removeTile(const TileHandle& handle);
    
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
    std::uint64_t m_mapRevision{0};
    int m_tileSize;
    sf::RenderTexture m_frontBuffer;
    sf::RenderTexture m_backBuffer;
    bool m_needsRedraw;
    sf::Vector2f m_tileOffset{0.f, 0.f};
    std::vector<BrickDebris> m_debris;
};
