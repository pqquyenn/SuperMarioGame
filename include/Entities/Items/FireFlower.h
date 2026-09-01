#pragma once

#include "Animation/AnimationClip.h"
#include "Animation/SpriteAnimator.h"
#include "Entities/Items/Item.h"

// ============================================================
// FireFlower – Hoa lửa cho Mario biến thành Fire Mario
// Hành vi: nhô lên từ gạch chấm hỏi, sau đó đứng yên tại chỗ
//          với animation lấp lánh 4 frame lấy từ BlockTileSheet
//          vùng bắt đầu (76,127), mỗi frame 16x16 tịnh tiến 16px.
// ============================================================
class FireFlower : public Item {
private:
    // Emerge animation state
    bool emerging{false};
    float emergeDistance{0.f};
    float emergeTarget{16.f};
    float emergeSpeed{40.f};

    SpriteAnimator animator;
    AnimationClip clip;

    AnimationClip buildAnimationClip() const;

public:
    FireFlower(float x = 0.f, float y = 0.f);
    ~FireFlower() override = default;

    void update(float dt) override;
    void render(sf::RenderWindow& window) const override;
    void onCollect() override;
    bool tryCollect(Character& character) override;

    void startEmerge();
    bool isEmerging() const override;
    bool shouldSkipTileCollision() const override { return emerging; }
};
