#pragma once

#include "Entities/Items/Item.h"

class Coin : public Item {
public:
    Coin(float x = 0.f, float y = 0.f);
    void update(float dt) override;
    void onCollect() override;
};
