#pragma once

#include "Entities/Items/Item.h"

// ============================================================
// Mushroom – Nấm tăng sức mạnh (Super Mushroom)
// Hành vi: nhô lên từ gạch chấm hỏi, sau đó trượt sang phải
//          trên mặt đất. Mario ăn → chuyển sang SuperState.
// ============================================================
class Mushroom : public Item {
private:
    float moveSpeed{60.f};       // Tốc độ trượt ngang (pixels/sec)
    int moveDirection{1};        // 1: phải, -1: trái
    float gravity{600.f};        // Gia tốc trọng lực (pixels/sec²)
    float vertVelocity{0.f};     // Vận tốc dọc hiện tại

    // Animation nhô lên từ gạch (emerge)
    bool emerging{false};        // Đang nhô lên khỏi gạch?
    float emergeDistance{0.f};   // Khoảng cách đã nhô
    float emergeTarget{32.f};    // Nhô lên 1 tile (32px)
    float emergeSpeed{40.f};     // Tốc độ nhô lên

public:
    Mushroom(float x = 0.f, float y = 0.f);
    ~Mushroom() override = default;

    void update(float dt) override;
    void render(sf::RenderWindow& window) const override;
    void onCollect() override;
    bool tryCollect(Character& character) override;

    // Bắt đầu animation nhô lên từ gạch
    void startEmerge();
    bool isEmerging() const;

    // Đảo chiều di chuyển (khi chạm tường)
    void reverseDirection();
    int getMoveDirection() const;
};
