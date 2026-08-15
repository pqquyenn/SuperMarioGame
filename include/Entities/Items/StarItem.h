#pragma once

#include "Animation/AnimationClip.h"
#include "Animation/SpriteAnimator.h"
#include "Entities/Items/Item.h"

// ============================================================
// StarItem – Ngôi sao bất tử (Starman)
// Hành vi: Nhô lên từ gạch (emerge), sau đó nảy
//          bouncing lên/xuống và trượt ngang.
//          Animation 4 khung hình lấp lánh lấy từ BlockTileSheet.
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

    SpriteAnimator animator;
    AnimationClip clip;

    AnimationClip buildAnimationClip() const;

public:
    StarItem(float x = 0.f, float y = 0.f);
    ~StarItem() override = default;

    void update(float dt) override;
    void render(sf::RenderWindow& window) const override;
    void onCollect() override;
    bool tryCollect(Character& character) override;

    void startEmerge();
    bool isEmerging() const;
    bool shouldSkipTileCollision() const override { return emerging; }

    void reverseDirection() override;
    int getMoveDirection() const;

    void onLanded() override;
    void notifyGrounded();
};

