#pragma once

#include "Entities/Items/Item.h"

// ============================================================
// StarItem – Ngôi sao bất tử (Starman)
// Hành vi: Nhô lên từ gạch chấm hỏi (emerge), sau đó nảy
//          bouncing lên/xuống và trượt ngang.
//          Mario ăn → nhận StarEffect (bất tử + tăng tốc 10s).
// ============================================================
class StarItem : public Item {
private:
    float moveSpeed{80.f};
    int moveDirection{1};
    float gravity{600.f};
    float bounceForce{-220.f}; // Lực nảy lên khi chạm đất

    // Emerge state
    bool emerging{false};
    float emergeDistance{0.f};
    float emergeTarget{16.f};
    float emergeSpeed{40.f};

public:
    StarItem(float x = 0.f, float y = 0.f);
    ~StarItem() override = default;

    void update(float dt) override;
    void render(sf::RenderWindow& window) const override;
    void onCollect() override;
    bool tryCollect(Character& character) override;

    void startEmerge();
    bool isEmerging() const;

    void reverseDirection();
    int getMoveDirection() const;

    void notifyGrounded();
};
