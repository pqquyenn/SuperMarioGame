#pragma once

#include "Animation/AnimationClip.h"
#include <string>
#include <string_view>
#include <unordered_map>

// Motions currently supported by both gameplay and PlayerSpriteSheet.png.
enum class PlayerMotion {
    Idle,
    Running,
    Jumping,
    Sliding,
    Dead
};

struct PlayerAnimationSet {
    AnimationClip idle;
    AnimationClip running;
    AnimationClip jumping;
    AnimationClip sliding;
    AnimationClip dead;

    const AnimationClip* findClip(PlayerMotion motion) const;
};

// Maps a PlayerState form name (Small/Super/Fire) to its visual clips.
// New forms can register another set without changing SpriteAnimator.
class PlayerAnimationProfile {
private:
    std::unordered_map<std::string, PlayerAnimationSet> forms;

public:
    void registerForm(
        std::string formName,
        PlayerAnimationSet animationSet
    );

    const AnimationClip* findClip(
        std::string_view formName,
        PlayerMotion motion
    ) const;
};

// Builds a profile for the current 403x266 PlayerSpriteSheet.png. Each player
// supplies its own normal Small/Big row; Fire uses the shared Fire row.
PlayerAnimationProfile makeClassicPlayerAnimationProfile(
    int normalSmallRowY,
    int normalPoweredRowY,
    int firePoweredRowY
);
