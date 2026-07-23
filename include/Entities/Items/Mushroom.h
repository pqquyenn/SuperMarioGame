#pragma once

#include "Entities/Items/Item.h"

class Mushroom : public Item {
public:
    Mushroom(float x = 0.f, float y = 0.f);
    void update(float dt) override;
    void onCollect() override;
};
