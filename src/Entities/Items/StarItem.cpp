#include "Entities/Items/StarItem.h"
#include "Entities/Character.h"
#include "PlayerEffects/StarEffect.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>

// ============================================================
// Constructor
// Kích thước Star: 16x16 pixels
// ============================================================
StarItem::StarItem(float x, float y) : Item(x, y) {
    size = {16.f, 16.f};
}

// ============================================================
// update – Logic di chuyển mỗi frame
// ============================================================
void StarItem::update(float dt) {
    if (!active || collected) return;

    if (emerging) {
        emergeDistance += emergeSpeed * dt;
        position.y -= emergeSpeed * dt;
        if (emergeDistance >= emergeTarget) {
            emerging = false;
        }
    } else {
        velocity.x = static_cast<float>(moveDirection) * moveSpeed;
        velocity.y += gravity * dt;
        integrateVelocity(dt);
    }

    sprite.setPosition(position);
}

// ============================================================
// render – Vẽ Ngôi Sao
// ============================================================
void StarItem::render(sf::RenderWindow& window) const {
    if (!active || collected) return;

    if (sprite.getTexture() != nullptr) {
        window.draw(sprite);
        return;
    }

    // --- Fallback: Vẽ Ngôi Sao vàng bằng hình 5 cánh nếu chưa load được texture ---
    sf::ConvexShape starShape(10);
    starShape.setPoint(0, sf::Vector2f(8.f, 0.f));
    starShape.setPoint(1, sf::Vector2f(10.5f, 5.f));
    starShape.setPoint(2, sf::Vector2f(16.f, 5.8f));
    starShape.setPoint(3, sf::Vector2f(12.f, 9.8f));
    starShape.setPoint(4, sf::Vector2f(13.f, 15.3f));
    starShape.setPoint(5, sf::Vector2f(8.f, 12.5f));
    starShape.setPoint(6, sf::Vector2f(3.f, 15.3f));
    starShape.setPoint(7, sf::Vector2f(4.f, 9.8f));
    starShape.setPoint(8, sf::Vector2f(0.f, 5.8f));
    starShape.setPoint(9, sf::Vector2f(5.5f, 5.f));
    starShape.setPosition(position);
    starShape.setFillColor(sf::Color(255, 220, 0));
    starShape.setOutlineColor(sf::Color(200, 150, 0));
    starShape.setOutlineThickness(1.f);
    window.draw(starShape);
}

// ============================================================
// onCollect & tryCollect
// ============================================================
void StarItem::onCollect() {
    if (!collected) {
        collected = true;
        active = false;
    }
}

bool StarItem::tryCollect(Character& character) {
    if (!active || collected) return false;

    // Gắn StarEffect 10 giây bất tử + tăng tốc cho Mario
    if (character.addEffect(std::make_unique<StarEffect>(10.f))) {
        onCollect();
        return true;
    }
    return false;
}

void StarItem::startEmerge() {
    emerging = true;
    emergeDistance = 0.f;
}

bool StarItem::isEmerging() const {
    return emerging;
}

void StarItem::reverseDirection() {
    moveDirection = -moveDirection;
}

int StarItem::getMoveDirection() const {
    return moveDirection;
}

void StarItem::notifyGrounded() {
    if (!emerging) {
        velocity.y = bounceForce;
    }
}
