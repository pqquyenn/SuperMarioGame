#pragma once
#include "Entities/Enemies/Koopa.h"

class TileMap;

class RedKoopa : public Koopa {
protected:
    const TileMap* tileMapRef = nullptr;

public:
    RedKoopa(float x = 0.f, float y = 0.f, const TileMap* map = nullptr);
    ~RedKoopa() override = default;

    void update(float dt) override;
    void onStomped() override;

    void setTileMap(const TileMap* map);
};
