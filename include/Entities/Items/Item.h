#pragma once

#include "Entities/Entity.h"

class Item : public Entity {
public:
    Item(float x = 0.f, float y = 0.f) : Entity(x, y) {}
    virtual ~Item() = default;

    virtual void onCollect() = 0;
};
