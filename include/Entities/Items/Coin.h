#pragma once

#include "Entities/Items/Item.h"

// ============================================================
// Coin – Đồng xu Mario thu thập để tăng điểm
// 2 loại coin:
//   1. Floating coin: lơ lửng trên không, xoay liên tục (trong level)
//   2. Block coin: bắn lên khi đập gạch chấm hỏi (pop animation)
// ============================================================
class Coin : public Item {
private:
    static constexpr int DefaultScoreValue = 200;

    // Animation xoay (scale oscillation giả lập xoay 3D)
    float spinTimer{0.f};
    float spinSpeed{4.0f};      // Tốc độ xoay (Hz)

    // Pop animation (khi coin bắn ra từ gạch)
    bool popping{false};         // Đang trong animation pop?
    float popVelocity{-300.f};   // Vận tốc lên ban đầu khi pop
    float popGravity{800.f};     // Gia tốc rơi xuống
    float currentPopVelocity{0.f};
    float popStartY{0.f};       // Vị trí Y bắt đầu pop
    float popMaxHeight{80.f};   // Chiều cao tối đa bay lên

    int scoreValue{DefaultScoreValue}; // Điểm khi thu thập

public:
    Coin(float x = 0.f, float y = 0.f);
    ~Coin() override = default;

    void update(float dt) override;
    void render(sf::RenderWindow& window) const override;
    void onCollect() override;
    bool tryCollect(Character& character) override;

    // Bắt đầu animation pop (bắn ra từ gạch)
    void startPop();
    bool isPopping() const;

    int getScoreValue() const;
    void setScoreValue(int score);

    static constexpr int defaultScoreValue() {
        return DefaultScoreValue;
    }
};
