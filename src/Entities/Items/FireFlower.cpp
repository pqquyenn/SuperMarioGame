#include "Entities/Items/FireFlower.h"

namespace {
constexpr int FireFlowerRowLeft = 1;
constexpr int FireFlowerRowTop = 18;
constexpr int FireFlowerFrameStep = 17;
constexpr int FireFlowerFrameWidth = 16;

sf::IntRect fireFlowerFrameAt(int index) {
    return sf::IntRect{
        FireFlowerRowLeft + index * FireFlowerFrameStep,
        FireFlowerRowTop,
        FireFlowerFrameWidth,
        FireFlowerFrameWidth
    };
}
}

// ============================================================
// Constructor
// FireFlower đứng yên, kích thước logic 16x16 (kế thừa từ Item)
// ============================================================
FireFlower::FireFlower(float x, float y)
    : Item(x, y),
      clip{buildAnimationClip()} {
    size = {16.f, 16.f};
    animator.play(clip);
    if (const sf::IntRect* frame = animator.getCurrentFrame()) {
        sprite.setTextureRect(*frame);
    }
    // Đồng bộ sprite ngay để tránh lệch 1 frame khi vừa spawn
    sprite.setPosition(position);
}

AnimationClip FireFlower::buildAnimationClip() const {
    return AnimationClip{
        {
            fireFlowerFrameAt(0),
            fireFlowerFrameAt(1),
            fireFlowerFrameAt(2),
            fireFlowerFrameAt(3)
        },
        0.12f,
        true
    };
}

// ============================================================
// update – Logic mỗi frame
//   1. Nếu đang emerge: nhô lên từ gạch
//   2. Nếu đã emerge xong: đứng yên + shimmer 4 frame
// ============================================================
void FireFlower::update(float dt) {
    if (!active || collected) return;

    if (emerging) {
        emergeDistance += emergeSpeed * dt;
        position.y -= emergeSpeed * dt;
        if (emergeDistance >= emergeTarget) {
            emerging = false;
        }
    }

    animator.update(dt);
    if (const sf::IntRect* frame = animator.getCurrentFrame()) {
        sprite.setTextureRect(*frame);
    }
    sprite.setPosition(position);
}

// ============================================================
// render – Vẽ hoa lửa (Sprite nếu có texture, fallback shape)
// ============================================================
void FireFlower::render(sf::RenderWindow& window) const {
    if (!active || collected) return;

    if (sprite.getTexture() != nullptr) {
        window.draw(sprite);
        return;
    }

    // --- Fallback: vẽ hoa lửa bằng shapes khi texture lỗi ---
    float stemW = 4.f;
    float stemH = size.y * 0.5f;
    sf::RectangleShape stem(sf::Vector2f(stemW, stemH));
    stem.setPosition(sf::Vector2f(position.x + (size.x - stemW) / 2.f,
                     position.y + size.y - stemH));
    stem.setFillColor(sf::Color(0, 140, 0));
    window.draw(stem);

    float centerR = 4.f;
    sf::CircleShape center(centerR);
    center.setFillColor(sf::Color(255, 220, 50));
    center.setPosition(position.x + size.x / 2.f - centerR,
                       position.y + size.y * 0.15f);
    window.draw(center);
}

// ============================================================
// onCollect – Mario ăn hoa lửa
// ============================================================
#include "Entities/Character.h"
#include "PlayerStates/FireState.h"

void FireFlower::onCollect() {
    if (!collected) {
        collected = true;
        active = false;
    }
}

bool FireFlower::tryCollect(Character& character) {
    if (!active || collected) return false;
    if (character.receivePowerUp(std::make_unique<FireState>())) {
        onCollect();
        return true;
    }
    return false;
}

void FireFlower::startEmerge() {
    emerging = true;
    emergeDistance = 0.f;
}

bool FireFlower::isEmerging() const {
    return emerging;
}
