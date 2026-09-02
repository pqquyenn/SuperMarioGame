#include "Entities/YellowLaser.h"
#include "Core/SoundManager.h"
#include <cmath>

YellowLaser::YellowLaser(
    float x,
    float y,
    bool movingRight,
    const sf::Texture& texture
)
    : Entity(x, y),
      movingRight(movingRight) {
    sprite.setTexture(texture);
    laserSize = sf::Vector2f(
        static_cast<float>(texture.getSize().x),
        static_cast<float>(texture.getSize().y)
    );
    if (laserSize.x <= 0.f || laserSize.y <= 0.f) {
        laserSize = sf::Vector2f(26.f, 10.f);
    }
    sprite.setOrigin(0.f, laserSize.y * 0.5f);
    sprite.setScale(movingRight ? 1.f : -1.f, 1.f);
    velocity.x = movingRight ? LaserSpeed : -LaserSpeed;
    velocity.y = 0.f;
    sprite.setPosition(position.x, position.y);
    active = true;
}

void YellowLaser::update(float dt) {
    if (!active) return;

    lifetime += dt;
    if (lifetime > 3.0f) {
        active = false;
        return;
    }

    position.x += velocity.x * dt;
    sprite.setPosition(position.x, position.y);
}

void YellowLaser::render(sf::RenderWindow& window) const {
    if (!active) return;
    window.draw(sprite);
}

sf::FloatRect YellowLaser::getBounds() const {
    float left = movingRight ? position.x : position.x - laserSize.x;
    return sf::FloatRect(left, position.y - laserSize.y * 0.5f, laserSize.x, laserSize.y);
}

void YellowLaser::explode() {
    active = false;
    exploding = true;
    SoundManager::getInstance().playSound("stomp");
}
