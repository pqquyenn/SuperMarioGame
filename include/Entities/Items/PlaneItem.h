#pragma once

#include "Entities/Items/Item.h"

class PlaneItem : public Item {
private:
    float fallSpeed{60.f};
    float floatTimer{0.f};

public:
    PlaneItem(float x = 0.f, float y = 0.f);
    ~PlaneItem() override = default;

    void update(float dt) override;
    void render(sf::RenderWindow& window) const override;
    void onCollect() override;
    bool tryCollect(Character& character) override;
};
