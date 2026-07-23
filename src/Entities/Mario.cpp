#include "Entities/Mario.h"
#include "PlayerStates/SmallState.h"

Mario::Mario(float x, float y) : Character(x, y) {
    moveSpeed = 160.f;
    jumpForce = 360.f;
    currentState = std::make_unique<SmallState>();
}

void Mario::update(float dt) {
    // Apply gravity
    velocity.y += 980.f * dt;

    position += velocity * dt;
    sprite.setPosition(position);

    // Reset horizontal speed decay
    velocity.x *= 0.8f;
}

void Mario::changeState(std::unique_ptr<PlayerState> state) {
    currentState = std::move(state);
}
