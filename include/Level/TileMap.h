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

class TileMap {
public:
    TileMap();
    ~TileMap();

    TileMap(const TileMap&) = delete;
    TileMap& operator=(const TileMap&) = delete;
    TileMap(TileMap&&) = delete;
    TileMap& operator=(TileMap&&) = delete;

    bool readFromFile(const std::string& filepath);
    void updateBuffer(const Camera& camera);
    void render(sf::RenderTarget& target, const Camera& camera);
    std::vector<Tile*> getTilesInBounds(const sf::FloatRect& bounds) const;
    void setNeedsRedraw(bool needsRedraw);

private:
    void initFlyweights();

private:
    std::unordered_map<std::string, std::shared_ptr<TileType>> m_tileRegistry;
    std::vector<std::vector<std::unique_ptr<Tile>>> m_grid;
    int m_tileSize;
    sf::RenderTexture m_frontBuffer;
    sf::RenderTexture m_backBuffer;
    bool m_needsRedraw;
};