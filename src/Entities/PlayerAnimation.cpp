#include "Entities/PlayerAnimation.h"

#include <utility>

namespace {
constexpr int FrameLeft = 1;
constexpr int FrameStep = 16;
constexpr int FrameWidth = 16;

// Fire Mario strip: 7 frames of 16x32 starting at (1, 153) laid out
// horizontally on PlayerSpriteSheet.png with 16px step.
constexpr int FireRowLeft = 1;
constexpr int FireRowTop = 153;
constexpr int FireFrameStep = 16;
constexpr int FireFrameWidth = 16;
constexpr int FireFrameHeight = 32;

sf::IntRect frameAt(int index, int rowY, int height) {
    return sf::IntRect{
        FrameLeft + index * FrameStep,
        rowY,
        FrameWidth,
        height
    };
}

sf::IntRect fireFrameAt(int index) {
    return sf::IntRect{
        FireRowLeft + index * FireFrameStep,
        FireRowTop,
        FireFrameWidth,
        FireFrameHeight
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
    // Frame 1 is the death pose in the Small row, but the dedicated crouch
    // pose in every powered row. Small characters never select this motion.
    set.crouching = staticClip(frameAt(1, rowY, frameHeight));
    set.dead = staticClip(deathFrame, 0.15f, false);
    set.shooting = staticClip(frameAt(0, rowY, frameHeight));
    return set;
}

PlayerAnimationSet makeFireAnimationSet(const sf::IntRect& deathFrame) {
    PlayerAnimationSet set;
    set.idle = staticClip(fireFrameAt(0));
    // Keep the 32px frame above the sprite sheet's white separator rows,
    // which begin at y=186. Starting at 155 included one separator row.
    set.shooting = staticClip(sf::IntRect{241, FireRowTop, 16, 32});
    set.running = AnimationClip{
        {
            fireFrameAt(2),
            fireFrameAt(3),
            fireFrameAt(4)
        },
        0.09f,
        true
    };
    set.jumping = staticClip(fireFrameAt(6));
    set.sliding = staticClip(fireFrameAt(5));
    set.crouching = staticClip(fireFrameAt(1));
    set.dead = staticClip(deathFrame, 0.15f, false);
    return set;
}

// Star Mario frames:
// Small Mario: (3, 217), size (11, 15), step 16 -> (3,217), (19,217), (35,217), (51,217)
constexpr int StarSmallLeft = 3;
constexpr int StarSmallTop = 217;
constexpr int StarSmallStep = 16;
constexpr int StarSmallWidth = 11;
constexpr int StarSmallHeight = 15;

// Super Mario: (1, 233), size (15, 31), step 16 -> (1,233), (17,233), (33,233), (49,233)
constexpr int StarSuperLeft = 1;
constexpr int StarSuperTop = 233;
constexpr int StarSuperStep = 16;
constexpr int StarSuperWidth = 15;
constexpr int StarSuperHeight = 31;

sf::IntRect starSmallFrameAt(int index) {
    return sf::IntRect{
        StarSmallLeft + index * StarSmallStep,
        StarSmallTop,
        StarSmallWidth,
        StarSmallHeight
    };
}

sf::IntRect starSuperFrameAt(int index) {
    return sf::IntRect{
        StarSuperLeft + index * StarSuperStep,
        StarSuperTop,
        StarSuperWidth,
        StarSuperHeight
    };
}

PlayerAnimationSet makeStarSmallAnimationSet(const sf::IntRect& deathFrame) {
    PlayerAnimationSet set;
    AnimationClip starClip{
        {
            starSmallFrameAt(0),
            starSmallFrameAt(1),
            starSmallFrameAt(2),
            starSmallFrameAt(3)
        },
        0.08f,
        true
    };
    set.idle = starClip;
    set.running = starClip;
    set.jumping = starClip;
    set.sliding = starClip;
    set.crouching = starClip;
    set.dead = staticClip(deathFrame, 0.15f, false);
    set.shooting = starClip;
    return set;
}

PlayerAnimationSet makeStarSuperAnimationSet(const sf::IntRect& deathFrame) {
    PlayerAnimationSet set;
    AnimationClip starClip{
        {
            starSuperFrameAt(0),
            starSuperFrameAt(1),
            starSuperFrameAt(2),
            starSuperFrameAt(3)
        },
        0.08f,
        true
    };
    set.idle = starClip;
    set.running = starClip;
    set.jumping = starClip;
    set.sliding = starClip;
    set.crouching = starClip;
    set.dead = staticClip(deathFrame, 0.15f, false);
    set.shooting = starClip;
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
        case PlayerMotion::Crouching:
            return &crouching;
        case PlayerMotion::Dead:
            return &dead;
        case PlayerMotion::Shooting:
            return &shooting;
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
    int /*firePoweredRowY*/
) {
    PlayerAnimationProfile profile;
    const sf::IntRect deathFrame{18, normalSmallRowY, 16, 16};

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
        makeFireAnimationSet(deathFrame)
    );
    profile.registerForm(
        "StarSmall",
        makeStarSmallAnimationSet(deathFrame)
    );
    profile.registerForm(
        "StarSuper",
        makeStarSuperAnimationSet(deathFrame)
    );

    return profile;
}

PlayerAnimationProfile makeLuigiAnimationProfile() {
    constexpr int LuigiSmallRowY = 73;
    constexpr int LuigiPoweredRowY = 89;
    constexpr int FirePoweredRowY = 153;
    return makeClassicPlayerAnimationProfile(
        LuigiSmallRowY,
        LuigiPoweredRowY,
        FirePoweredRowY
    );
}

