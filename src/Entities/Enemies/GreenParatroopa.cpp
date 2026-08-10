#include "Entities/Enemies/GreenParatroopa.h"
#include "Core/AssetManager.h"

GreenParatroopa::GreenParatroopa(float x, float y) : Koopa(x, y) {
    speed = 50.f;
    setTexture(AssetManager::getInstance().getTexture("GreenParatroopa_Walk1"));
}

void GreenParatroopa::update(float dt) {
    if (!active) return;

    if (hasWings) {
        if (onGround) {
            velocity.y = HopVelocity;
            onGround = false;
        }

        applyPhysics(dt);

        walkAnimTimer += dt;
        if (walkAnimTimer >= walkAnimInterval) {
            walkAnimTimer -= walkAnimInterval;
            walkFrame = 1 - walkFrame;
            auto& assets = AssetManager::getInstance();
            if (walkFrame == 0) {
                setTexture(assets.getTexture("GreenParatroopa_Walk1"));
            } else {
                setTexture(assets.getTexture("GreenParatroopa_Walk2"));
            }
        }
    } else {
        Koopa::update(dt);
    }
}

void GreenParatroopa::onStomped() {
    if (hasWings) {
        hasWings = false;
        velocity.y = 0.f;
        setTexture(AssetManager::getInstance().getTexture("Koopa"));
    } else {
        Koopa::onStomped();
    }
}

void GreenParatroopa::notifyLanded() {
    if (hasWings) {
        onGround = true;
    }
}
