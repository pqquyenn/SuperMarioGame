#pragma once

#include "Entities/Items/Item.h"

class FireFlower : public Item {
public:
    FireFlower(float x = 0.f, float y = 0.f);
    void update(float dt) override;
    void onCollect() override;
};
