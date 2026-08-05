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
    
    void render(sf::RenderTarget& target, const sf::Vector2f& position) const {
        if (!texture) return;
        sf::Sprite sprite(*texture, textureRect);
        sprite.setPosition(position);
        target.draw(sprite);
    }
};