#include "Entities/Enemies/Koopa.h"

Koopa::Koopa(float x, float y) : Enemy(x, y) {
    speed = 50.f;
}

void Koopa::update(float dt) {
    if (!active) return;
    position.x += direction * speed * dt;
    sprite.setPosition(position);
}

void Koopa::onStomped() {
    if (!inShell) {
        inShell = true;
        speed = 0.f;
    } else if (!shellSpinning) {
        shellSpinning = true;
        speed = 300.f; // Shell spins fast!
    } else {
        active = false;
    }
}
