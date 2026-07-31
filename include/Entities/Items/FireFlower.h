#pragma once

#include "Entities/Items/Item.h"

// ============================================================
// FireFlower – Hoa lửa cho Mario biến thành Fire Mario
// Hành vi: nhô lên từ gạch chấm hỏi, sau đó đứng yên tại chỗ
//          với animation nhấp nhô nhẹ. Mario ăn → FireState.
// ============================================================
class FireFlower : public Item {
private:
    // Animation nhấp nhô (bob up and down)
    float bobTimer{0.f};       // Timer cho sin wave
    float bobAmplitude{3.f};   // Biên độ nhấp nhô (pixels)
    float bobSpeed{3.0f};      // Tốc độ nhấp nhô (Hz)
    float baseY{0.f};          // Vị trí Y gốc (không nhấp nhô)

    // Animation nhô lên từ gạch (emerge)
    bool emerging{false};
    float emergeDistance{0.f};
    float emergeTarget{32.f};
    float emergeSpeed{40.f};

public:
    FireFlower(float x = 0.f, float y = 0.f);
    ~FireFlower() override = default;

    void update(float dt) override;
    void render(sf::RenderWindow& window) const override;
    void onCollect() override;

    void startEmerge();
    bool isEmerging() const;
};
