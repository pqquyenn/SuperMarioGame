#include "Entities/Items/PlaneItem.h"
#include "Core/AssetManager.h"
#include "Core/SoundManager.h"
#include "Entities/Character.h"
#include "PlayerStates/PlaneState.h"
#include <cmath>

PlaneItem::PlaneItem(float x, float y)
    : Item(x, y) {
    size = {32.f, 20.f};
    sf::Texture& tex = AssetManager::getInstance().getTexture("PlaneRed");
    if (tex.getSize().x > 0) {
        sprite.setTexture(tex);
        float scaleX = size.x / static_cast<float>(tex.getSize().x);
        float scaleY = size.y / static_cast<float>(tex.getSize().y);
        sprite.setScale(scaleX, scaleY);
    }
    sprite.setPosition(position);
    active = true;
}

void PlaneItem::update(float dt) {
    if (!active || collected) return;

    floatTimer += dt;
    // Gentle floating descent with subtle horizontal sway
    velocity.y = fallSpeed;
    velocity.x = std::sin(floatTimer * 3.f) * 20.f;

    position.x += velocity.x * dt;
    position.y += velocity.y * dt;
    sprite.setPosition(position);
}

void PlaneItem::render(sf::RenderWindow& window) const {
    if (!active || collected) return;

    if (sprite.getTexture() != nullptr && sprite.getTexture()->getSize().x > 0) {
        window.draw(sprite);
    } else {
        // Fallback shape
        sf::RectangleShape box(size);
        box.setPosition(position);
        box.setFillColor(sf::Color::Red);
        box.setOutlineColor(sf::Color::Yellow);
        box.setOutlineThickness(1.f);
        window.draw(box);
    }
}

void PlaneItem::onCollect() {
    if (!collected) {
        collected = true;
        active = false;
        SoundManager::getInstance().playSound("powerupcollect");
    }
}

bool PlaneItem::tryCollect(Character& character) {
    if (!active || collected) return false;
    if (character.receivePowerUp(std::make_unique<PlaneState>())) {
        onCollect();
        return true;
    }
    return false;
}
