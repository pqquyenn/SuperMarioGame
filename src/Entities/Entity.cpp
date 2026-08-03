#include "Entities/Entity.h"

Entity::Entity(float x, float y)
    : position{x, y} {
    syncSpritePosition();
}

void Entity::syncSpritePosition() {
    sprite.setPosition(position);
}

void Entity::integrateVelocity(float dt) {
    position += velocity * dt;
    syncSpritePosition();
}

void Entity::render(sf::RenderWindow& window) const {
    if (active) {
        window.draw(sprite);
    }
}

sf::FloatRect Entity::getBounds() const {
    return sprite.getGlobalBounds();
}

void Entity::setTexture(const sf::Texture& texture, bool resetRect) {
    sprite.setTexture(texture, resetRect);
}

void Entity::setTextureRect(const sf::IntRect& textureRect) {
    sprite.setTextureRect(textureRect);
}

const sf::Vector2f& Entity::getPosition() const {
    return position;
}

void Entity::setPosition(float x, float y) {
    setPosition(sf::Vector2f{x, y});
}

void Entity::setPosition(const sf::Vector2f& newPosition) {
    position = newPosition;
    syncSpritePosition();
}

void Entity::move(float offsetX, float offsetY) {
    move(sf::Vector2f{offsetX, offsetY});
}

void Entity::move(const sf::Vector2f& offset) {
    position += offset;
    syncSpritePosition();
}

const sf::Vector2f& Entity::getVelocity() const {
    return velocity;
}

void Entity::setVelocity(float x, float y) {
    setVelocity(sf::Vector2f{x, y});
}

void Entity::setVelocity(const sf::Vector2f& newVelocity) {
    velocity = newVelocity;
}

void Entity::addVelocity(float x, float y) {
    velocity += sf::Vector2f{x, y};
}

bool Entity::isActive() const {
    return active;
}

void Entity::setActive(bool status) {
    active = status;
}
