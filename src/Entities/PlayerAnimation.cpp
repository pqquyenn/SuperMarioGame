#include "Entities/PlayerAnimation.h"

#include <utility>

namespace {
constexpr int FrameLeft = 1;
constexpr int FrameWidth = 16;

sf::IntRect frameAt(int index, int rowY, int height) {
    return sf::IntRect{
        FrameLeft + index * FrameWidth,
        rowY,
        FrameWidth,
        height
    };
}

AnimationClip staticClip(
    const sf::IntRect& frame,
    float duration = 0.1f,
    bool looping = true
) {
    return AnimationClip{{frame}, duration, looping};
}

PlayerAnimationSet makeAnimationSet(
    int rowY,
    int frameHeight,
    const sf::IntRect& deathFrame
) {
    PlayerAnimationSet set;
    set.idle = staticClip(frameAt(0, rowY, frameHeight));
    set.running = AnimationClip{
        {
            frameAt(2, rowY, frameHeight),
            frameAt(3, rowY, frameHeight),
            frameAt(4, rowY, frameHeight)
        },
        0.09f,
        true
    };
    set.jumping = staticClip(frameAt(6, rowY, frameHeight));
    set.sliding = staticClip(frameAt(5, rowY, frameHeight));
    set.dead = staticClip(deathFrame, 0.15f, false);
    return set;
}
}

const AnimationClip* PlayerAnimationSet::findClip(PlayerMotion motion) const {
    switch (motion) {
        case PlayerMotion::Idle:
            return &idle;
        case PlayerMotion::Running:
            return &running;
        case PlayerMotion::Jumping:
            return &jumping;
        case PlayerMotion::Sliding:
            return &sliding;
        case PlayerMotion::Dead:
            return &dead;
    }

    return nullptr;
}

void PlayerAnimationProfile::registerForm(
    std::string formName,
    PlayerAnimationSet animationSet
) {
    forms.insert_or_assign(
        std::move(formName),
        std::move(animationSet)
    );
}

const AnimationClip* PlayerAnimationProfile::findClip(
    std::string_view formName,
    PlayerMotion motion
) const {
    const auto form = forms.find(std::string{formName});
    if (form == forms.end()) {
        return nullptr;
    }

    const AnimationClip* clip = form->second.findClip(motion);
    if (clip && clip->isValid()) {
        return clip;
    }

    const AnimationClip* idle =
        form->second.findClip(PlayerMotion::Idle);
    return idle && idle->isValid() ? idle : nullptr;
}

PlayerAnimationProfile makeClassicPlayerAnimationProfile(
    int normalSmallRowY,
    int normalPoweredRowY,
    int firePoweredRowY
) {
    PlayerAnimationProfile profile;
    const sf::IntRect deathFrame =
        frameAt(1, normalSmallRowY, 16);

    profile.registerForm(
        "Small",
        makeAnimationSet(normalSmallRowY, 16, deathFrame)
    );
    profile.registerForm(
        "Super",
        makeAnimationSet(normalPoweredRowY, 32, deathFrame)
    );
    profile.registerForm(
        "Fire",
        makeAnimationSet(firePoweredRowY, 32, deathFrame)
    );

    return profile;
}
