#include "Entities/Fireball.h"

namespace {
constexpr int FireballRowLeft = 600;
constexpr int FireballRowTop = 209;
constexpr int FireballFrameStep = 17;
constexpr int FireballFrameWidth = 7;
constexpr int FireballFrameHeight = 7;

constexpr int ExplosionRowLeft = 613;
constexpr int ExplosionRowTop = 222;
constexpr int ExplosionFrameStep = 17;
constexpr int ExplosionFrameWidth = 16;
constexpr int ExplosionFrameHeight = 16;

sf::IntRect fireballFrameAt(int index) {
    return sf::IntRect{
        FireballRowLeft + index * FireballFrameStep,
        FireballRowTop,
        FireballFrameWidth,
        FireballFrameHeight
    };
}

sf::IntRect fireballExplosionFrameAt(int index) {
    return sf::IntRect{
        ExplosionRowLeft + index * ExplosionFrameStep,
        ExplosionRowTop,
        ExplosionFrameWidth,
        ExplosionFrameHeight
    };
}
}

Fireball::Fireball(
    float x,
    float y,
    bool movingRightDirection,
    const sf::Texture& spriteSheet,
    float display
)
    : Entity{x, y},
      movingRight{movingRightDirection},
      displaySize{display},
      clip{buildAnimationClip()},
      explosionClip{buildExplosionClip()} {
    setTexture(spriteSheet);
    velocity.x = movingRight ? HorizontalSpeed : -HorizontalSpeed;
    velocity.y = 0.f;

    animator.play(clip);
    if (const AnimationFrame* frame = animator.getCurrentFrame()) {
        sprite.setTextureRect(frame->textureRect);
        const float frameWidth = static_cast<float>(std::abs(frame->textureRect.width));
        sprite.setScale(displaySize / frameWidth, displaySize / frameWidth);
    }
}

AnimationClip Fireball::buildAnimationClip() const {
    return AnimationClip{
        {
            fireballFrameAt(0),
            fireballFrameAt(1),
            fireballFrameAt(2),
            fireballFrameAt(3)
        },
        0.07f,
        true
    };
}

AnimationClip Fireball::buildExplosionClip() const {
    return AnimationClip{
        {
            fireballExplosionFrameAt(0),
            fireballExplosionFrameAt(1),
            fireballExplosionFrameAt(2)
        },
        0.05f,
        false
    };
}

void Fireball::explode() {
    if (exploding || !active) return;
    exploding = true;
    velocity = {0.f, 0.f};
    explosionTimer = 0.15f;
    animator.play(explosionClip);
    if (const AnimationFrame* frame = animator.getCurrentFrame()) {
        sprite.setTextureRect(frame->textureRect);
        sprite.setScale(1.f, 1.f);
        sprite.setPosition(position.x - 4.f, position.y - 4.f);
    }
}

void Fireball::update(float dt) {
    if (!active) {
        return;
    }

    if (exploding) {
        explosionTimer -= dt;
        animator.update(dt);
        if (const AnimationFrame* frame = animator.getCurrentFrame()) {
            sprite.setTextureRect(frame->textureRect);
            sprite.setScale(1.f, 1.f);
            sprite.setPosition(position.x - 4.f, position.y - 4.f);
        }
        if (explosionTimer <= 0.f) {
            active = false;
        }
        return;
    }

    // Gravity for bounce behaviour
    velocity.y += Gravity * dt;
    if (velocity.y > MaxFallSpeed) {
        velocity.y = MaxFallSpeed;
    }

    integrateVelocity(dt);

    animator.update(dt);
    if (const AnimationFrame* frame = animator.getCurrentFrame()) {
        sprite.setTextureRect(frame->textureRect);
    }
}

sf::FloatRect Fireball::getBounds() const {
    if (exploding) return sf::FloatRect{0.f, 0.f, 0.f, 0.f};
    return sf::FloatRect{position.x, position.y, displaySize, displaySize};
}

void Fireball::bounce() {
    if (!exploding) {
        velocity.y = BounceVelocity;
    }
}

void Fireball::onLanded() {
    bounce();
}

void Fireball::onWallCollision() {
    explode();
}
