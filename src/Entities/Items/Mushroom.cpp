#include "Entities/Items/Mushroom.h"

Mushroom::Mushroom(float x, float y) : Item(x, y) {}

void Mushroom::update(float dt) {
    if (!active) return;
    position.x += 40.f * dt;
    sprite.setPosition(position);
}

void Mushroom::onCollect() {
    active = false;
}
