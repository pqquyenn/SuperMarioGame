#pragma once
#include <SFML/Graphics.hpp>

struct TileType {
    const sf::Texture* texture = nullptr;
    sf::IntRect textureRect;
    
    bool isSolid = false;
    bool isWarpPipe = false;
    int warpDirection = 0;
    bool isQuestionBlock = false;
    bool isCoinTile = false;
    bool isBrick = false;
    bool isAnimated = false;
    bool isHorizontalWarpPipe = false; // auto-entry on horizontal contact (Pipe A, Pipe C1)
    sf::Vector2f placementOffset{0.f, 0.f};
    
    void render(sf::RenderTarget& target, const sf::Vector2f& position) const {
        if (!texture) return;
        sf::Sprite sprite(*texture, textureRect);
        sprite.setPosition(position);
        target.draw(sprite);
    }
};
