#include "Entities/Items/Coin.h"

Coin::Coin(float x, float y) : Item(x, y) {}

void Coin::update(float dt) {}

void Coin::onCollect() {
    active = false;
}
