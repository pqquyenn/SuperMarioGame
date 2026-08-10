#include "Entities/Enemies/RedParatroopa.h"
#include "Core/AssetManager.h"

RedParatroopa::RedParatroopa(float x, float y, const TileMap* map, float rangeUp, float rangeDown)
    : RedKoopa(x, y, map), startY(y), flyMinY(y - rangeUp), flyMaxY(y + rangeDown) {
    speed = 50.f;
    setTexture(AssetManager::getInstance().getTexture("RedParatroopa_Walk1"));
}

void RedParatroopa::update(float dt) {
    if (!active) return;

    if (hasWings) {
        // Vertical flight
        velocity.x = 0.f;
        velocity.y = static_cast<float>(flyDir) * FlySpeed;

        integrateVelocity(dt);

        if (position.y <= flyMinY) {
            position.y = flyMinY;
            flyDir = 1;
        } else if (position.y >= flyMaxY) {
            position.y = flyMaxY;
            flyDir = -1;
        }

        walkAnimTimer += dt;
        if (walkAnimTimer >= walkAnimInterval) {
            walkAnimTimer -= walkAnimInterval;
            walkFrame = 1 - walkFrame;
            auto& assets = AssetManager::getInstance();
            if (walkFrame == 0) {
                setTexture(assets.getTexture("RedParatroopa_Walk1"));
            } else {
                setTexture(assets.getTexture("RedParatroopa_Walk2"));
            }
        }
    } else {
        RedKoopa::update(dt);
    }
}

void RedParatroopa::onStomped() {
    if (hasWings) {
        hasWings = false;
        velocity = {0.f, 0.f};
        setTexture(AssetManager::getInstance().getTexture("RedKoopa_Walk1"));
    } else {
        RedKoopa::onStomped();
    }
}
