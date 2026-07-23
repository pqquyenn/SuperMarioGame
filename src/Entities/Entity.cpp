#include "Entities/Entity.h"

void Entity::render(sf::RenderWindow& window) {
    if (active) {
        sprite.setPosition(position);
        window.draw(sprite);
    }
}

sf::FloatRect Entity::getBounds() const {
    return sprite.getGlobalBounds();
}

void Entity::setPosition(float x, float y) {
    position = sf::Vector2f(x, y);
    sprite.setPosition(position);
}
