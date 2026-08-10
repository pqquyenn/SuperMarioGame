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
    bool isHorizontalWarpPipe() const;
    int getWarpDirection() const;
    bool isQuestionBlock() const;
    bool isCoinTile() const;
    void update(float dt);
    bool isBrick() const;
    bool isAnimated() const;
    void startBump();
    void setType(const TileType* type) { m_type = type; }
    const TileType* getType() const { return m_type; }

private:
    const TileType* m_type;
    sf::Vector2f m_position;
    float m_animTimer = 0.f;
    int m_animFrame = 0;
    float m_bumpOffset = 0.f;
    bool m_bumping = false;
};