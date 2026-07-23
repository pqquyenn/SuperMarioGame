#include "Level/Tile.h"

Tile::Tile(TileType t, float x, float y) : type(t), position(x, y) {
    sprite.setPosition(position);
}

void Tile::setTexture(const sf::Texture& texture) {
    sprite.setTexture(texture);
}

void Tile::render(sf::RenderWindow& window) {
    if (type != TileType::AIR) {
        window.draw(sprite);
    }
}

sf::FloatRect Tile::getBounds() const {
    return sprite.getGlobalBounds();
}
