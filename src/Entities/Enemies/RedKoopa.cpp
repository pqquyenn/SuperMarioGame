#include "Entities/Enemies/RedKoopa.h"
#include "Level/TileMap.h"
#include "Core/AssetManager.h"

RedKoopa::RedKoopa(float x, float y, const TileMap* map)
    : Koopa(x, y), tileMapRef(map) {
    speed = 50.f;
    setTexture(AssetManager::getInstance().getTexture("RedKoopa_Walk1"));
}

void RedKoopa::setTileMap(const TileMap* map) {
    tileMapRef = map;
}

void RedKoopa::update(float dt) {
    if (!active) return;

    if (!inShell && tileMapRef) {
        // Check ledge ahead
        float checkX = position.x + (direction > 0 ? size.x + 1.f : -1.f);
        float checkY = position.y + size.y + 2.f;
        if (!tileMapRef->isSolidAt(checkX, checkY)) {
            reverseDirection();
        }
    }

    applyPhysics(dt);

    auto& assets = AssetManager::getInstance();

    if (!inShell) {
        walkAnimTimer += dt;
        if (walkAnimTimer >= walkAnimInterval) {
            walkAnimTimer -= walkAnimInterval;
            walkFrame = 1 - walkFrame;
            if (walkFrame == 0) {
                setTexture(assets.getTexture("RedKoopa_Walk1"));
            } else {
                setTexture(assets.getTexture("RedKoopa_Walk2"));
            }
        }
    } else if (shellSpinning) {
        walkAnimTimer += dt;
        if (walkAnimTimer >= 0.1f) {
            walkAnimTimer -= 0.1f;
            walkFrame = 1 - walkFrame;
            if (walkFrame == 0) {
                setTexture(assets.getTexture("RedKoopa_Shell1"));
            } else {
                setTexture(assets.getTexture("RedKoopa_Shell2"));
            }
        }
    }
}

void RedKoopa::onStomped() {
    auto& assets = AssetManager::getInstance();
    if (!inShell) {
        inShell = true;
        speed = 0.f;
        setTexture(assets.getTexture("RedKoopa_Shell"));
    } else if (!shellSpinning) {
        shellSpinning = true;
        speed = 300.f;
        setTexture(assets.getTexture("RedKoopa_Shell1"));
    } else {
        active = false;
    }
}
