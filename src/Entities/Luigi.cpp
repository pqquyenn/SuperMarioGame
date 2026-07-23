#include "Entities/Luigi.h"

Luigi::Luigi(float x, float y) : Character(x, y) {
    moveSpeed = 150.f;  // Runs slightly slower
    jumpForce = 400.f;  // Jumps higher!
}

void Luigi::update(float dt) {
    velocity.y += 980.f * dt;
    position += velocity * dt;
    sprite.setPosition(position);
    velocity.x *= 0.8f;
}
