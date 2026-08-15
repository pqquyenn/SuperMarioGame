#include "Entities/Items/StarItem.h"
#include "Entities/Character.h"
#include "Observer/Event.h"
#include "PlayerEffects/StarEffect.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>

namespace {
constexpr int StarRowLeft = 2;
constexpr int StarRowTop = 35;
constexpr int StarFrameStep = 17;
constexpr int StarFrameWidth = 13;
constexpr int StarFrameHeight = 15;

sf::IntRect starFrameAt(int index) {
    return sf::IntRect{
        StarRowLeft + index * StarFrameStep,
        StarRowTop,
        StarFrameWidth,
        StarFrameHeight
    };
}
}

// ============================================================
// Constructor
// Kích thước Star: 16x16 pixels
// ============================================================
StarItem::StarItem(float x, float y)
    : Item(x, y),
      clip{buildAnimationClip()} {
    size = {16.f, 16.f};
    animator.play(clip);
    if (const AnimationFrame* frame = animator.getCurrentFrame()) {
        sprite.setTextureRect(frame->textureRect);
    }
    sprite.setPosition(position);
}

AnimationClip StarItem::buildAnimationClip() const {
    return AnimationClip{
        {
            starFrameAt(0), // (2, 35, 13, 15)
            starFrameAt(1), // (19, 35, 13, 15)
            starFrameAt(2), // (36, 35, 13, 15)
            starFrameAt(3)  // (53, 35, 13, 15)
        },
        0.12f,
        true
    };
}

// ============================================================
// update – Logic di chuyển mỗi frame
// ============================================================
void StarItem::update(float dt) {
    if (!active || collected) return;

    animator.update(dt);
    if (const AnimationFrame* frame = animator.getCurrentFrame()) {
        sprite.setTextureRect(frame->textureRect);
    }

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
        character.notify(GameEvent::powerupCollected(1000));
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

void StarItem::onLanded() {
    notifyGrounded();
}

void StarItem::notifyGrounded() {
    if (!emerging) {
        velocity.y = bounceForce;
    }
}


