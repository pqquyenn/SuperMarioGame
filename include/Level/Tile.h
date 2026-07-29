#pragma once
#include <SFML/Graphics.hpp>
#include "Level/TileType.h"

class Tile {
public:
    Tile(const TileType* type, const sf::Vector2f& position);
    void render(sf::RenderTarget& target) const;
    sf::FloatRect getBounds() const;
    bool isSolid() const;
    bool isWarpPipe() const;
    int getWarpDirection() const;

private:
    const TileType* m_type;
    sf::Vector2f m_position;
};