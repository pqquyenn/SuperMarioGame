#pragma once

#include "Animation/SpriteAnimator.h"
#include "Animation/AnimationClip.h"
#include "Entities/Entity.h"

// ============================================================
// Fireball – Đạn lửa Fire Mario bắn ra khi người chơi bấm phím
// hành động. Bay thẳng theo hướng, nảy khi chạm đất, biến mất
// khi chạm tường hoặc tiêu diệt kẻ thù. Sprite 8x8 lấy từ
// BlockTileSheet.png vùng bắt đầu (306,183), 4 frame shimmer.
// ============================================================
class Fireball : public Entity {
public:
    // frameWidth/Height: kích thước vùng cắt trong sheet (8x8).
    // displaySize: kích thước vẽ/va chạm in-game (mặc định 8.f).
    Fireball(
        float x,
        float y,
        bool movingRight,
        const sf::Texture& spriteSheet,
        float displaySize = 8.f
    );

    void update(float dt) override;
    sf::FloatRect getBounds() const override;

    bool isMovingRight() const { return movingRight; }
    void explode();
    bool isExploding() const { return exploding; }
    void bounce();

private:
    static constexpr float HorizontalSpeed{240.f};
    static constexpr float BounceVelocity{-180.f};
    static constexpr float Gravity{980.f};
    static constexpr float MaxFallSpeed{500.f};

    AnimationClip buildAnimationClip() const;
    AnimationClip buildExplosionClip() const;

    bool movingRight{true};
    bool exploding{false};
    float explosionTimer{0.f};
    float displaySize{8.f};
    SpriteAnimator animator;
    AnimationClip clip;
    AnimationClip explosionClip;
};
