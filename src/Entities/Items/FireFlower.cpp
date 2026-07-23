#include "Entities/Items/FireFlower.h"

FireFlower::FireFlower(float x, float y) : Item(x, y) {}

void FireFlower::update(float dt) {}

void FireFlower::onCollect() {
    active = false;
}
