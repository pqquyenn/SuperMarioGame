#pragma once

#include "Entities/Items/Mushroom.h"

// ============================================================
// OneUpMushroom – Nấm xanh tăng mạng (1-UP Mushroom)
// Kế thừa từ Mushroom: tái sử dụng toàn bộ logic nhô lên (emerge),
// trượt ngang, trọng lực, và nảy tường.
// Mario ăn → Tăng 1 mạng (LIFE_GAINED event)
// ============================================================
class OneUpMushroom : public Mushroom {
public:
    OneUpMushroom(float x = 0.f, float y = 0.f);
    ~OneUpMushroom() override = default;

    bool tryCollect(class Character& character) override;
    void render(sf::RenderWindow& window) const override;
};
