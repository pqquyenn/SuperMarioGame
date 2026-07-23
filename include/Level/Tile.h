#pragma once

#include <SFML/Graphics.hpp>

enum class TileType {
    AIR,
    SOLID_GROUND,
    BRICK,
    QUESTION_BLOCK,
    PIPE,
    COIN_BLOCK
};

class Tile {
private:
    sf::Sprite sprite;
    TileType type;
    sf::Vector2f position;

public:
    Tile(TileType t = TileType::AIR, float x = 0.f, float y = 0.f);
    void setTexture(const sf::Texture& texture);
    void render(sf::RenderWindow& window);

    TileType getType() const { return type; }
    sf::FloatRect getBounds() const;
};
