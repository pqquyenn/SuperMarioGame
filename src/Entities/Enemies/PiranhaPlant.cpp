#include "Entities/Enemies/PiranhaPlant.h"

PiranhaPlant::PiranhaPlant(float x, float y) : Enemy(x, y) {}

void PiranhaPlant::update(float dt) {
    // Up & down movement inside pipe
}

void PiranhaPlant::onStomped() {
    // Cannot be stomped, only destroyed by fireball or star
}
