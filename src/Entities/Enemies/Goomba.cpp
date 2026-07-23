#include "Entities/Enemies/Goomba.h"

Goomba::Goomba(float x, float y) : Enemy(x, y) {
    speed = 40.f;
}

void Goomba::update(float dt) {
    if (!active) return;
    position.x += direction * speed * dt;
    sprite.setPosition(position);
}

void Goomba::onStomped() {
    active = false; // Flatten and deactivate
}
