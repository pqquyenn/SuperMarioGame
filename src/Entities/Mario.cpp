#include "Entities/Mario.h"

#include <utility>

namespace {
constexpr int SmallWidth = 16;
constexpr int SmallHeight = 16;
constexpr int SuperWidth = 16;
constexpr int SuperHeight = 32;

CharacterProfile makeMarioProfile() {
    CharacterProfile profile;
    profile.moveAcceleration = 1000.f;
    profile.walkSpeed = 150.f;
    profile.crawlSpeed = 70.f;
    profile.runSpeed = 210.f;
    profile.groundDeceleration = profile.runSpeed / 0.14f;
    profile.crawlDeceleration = profile.crawlSpeed / 0.05f;
    profile.jumpForce = 350.f;
    profile.jumpHoldGravityMultiplier = 0.45f;
    profile.jumpReleaseGravityMultiplier = 2.5f;
    profile.maxJumpHoldTime = 0.16f;
    return profile;
}

bool hasSize(const sf::Texture* texture, unsigned width, unsigned height) {
    return texture && texture->getSize() == sf::Vector2u{width, height};
}

AnimationFrame fullFrame(
    const sf::Texture* texture,
    int width,
    int height
) {
    return AnimationFrame{sf::IntRect{0, 0, width, height}, texture};
}

AnimationClip staticClip(const AnimationFrame& frame) {
    return AnimationClip{{frame}, 0.1f, true};
}

PlayerAnimationSet makeSmallAnimationSet(
    const MarioAnimationTextures& textures
) {
    PlayerAnimationSet set;
    set.idle = staticClip(fullFrame(
        textures.smallIdle,
        SmallWidth,
        SmallHeight
    ));
    set.running = AnimationClip{
        {
            fullFrame(textures.smallRun1, SmallWidth, SmallHeight),
            fullFrame(textures.smallRun2, SmallWidth, SmallHeight),
            fullFrame(textures.smallRun3, SmallWidth, SmallHeight)
        },
        0.09f,
        true
    };
    set.jumping = staticClip(fullFrame(
        textures.smallJump,
        SmallWidth,
        SmallHeight
    ));
    set.sliding = staticClip(fullFrame(
        textures.smallSlide,
        SmallWidth,
        SmallHeight
    ));
    set.crouching = set.idle;
    set.dead = AnimationClip{
        {
            fullFrame(
                textures.smallDeath
                    ? textures.smallDeath
                    : textures.smallIdle,
                SmallWidth,
                SmallHeight
            )
        },
        0.15f,
        false
    };
    return set;
}

PlayerAnimationSet makeSuperAnimationSet(
    const MarioAnimationTextures& textures,
    const AnimationClip& deathClip
) {
    PlayerAnimationSet set;
    set.idle = staticClip(fullFrame(
        textures.superIdle,
        SuperWidth,
        SuperHeight
    ));
    set.running = AnimationClip{
        {
            fullFrame(textures.superRun1, SuperWidth, SuperHeight),
            fullFrame(textures.superRun2, SuperWidth, SuperHeight),
            fullFrame(textures.superRun3, SuperWidth, SuperHeight)
        },
        0.09f,
        true
    };
    set.jumping = staticClip(fullFrame(
        textures.superJump,
        SuperWidth,
        SuperHeight
    ));
    set.sliding = staticClip(fullFrame(
        textures.superSlide,
        SuperWidth,
        SuperHeight
    ));
    // The individual-image asset set has no crouch image. Atlas-backed Mario
    // uses the dedicated crouch frame; this profile falls back to idle.
    set.crouching = set.idle;
    set.dead = deathClip;
    return set;
}

PlayerAnimationProfile makeIndividualMarioAnimationProfile(
    const MarioAnimationTextures& textures
) {
    PlayerAnimationProfile animationProfile;
    PlayerAnimationSet small = makeSmallAnimationSet(textures);
    PlayerAnimationSet super = makeSuperAnimationSet(textures, small.dead);

    animationProfile.registerForm("Small", small);
    animationProfile.registerForm("Super", super);

    // The current asset set has no Fire Mario images. Reusing Super is an
    // explicit visual fallback; PlayerState mechanics remain Fire.
    animationProfile.registerForm("Fire", std::move(super));
    return animationProfile;
}
}

bool MarioAnimationTextures::isValid() const {
    return
        hasSize(smallIdle, SmallWidth, SmallHeight) &&
        hasSize(smallRun1, SmallWidth, SmallHeight) &&
        hasSize(smallRun2, SmallWidth, SmallHeight) &&
        hasSize(smallRun3, SmallWidth, SmallHeight) &&
        hasSize(smallJump, SmallWidth, SmallHeight) &&
        hasSize(smallSlide, SmallWidth, SmallHeight) &&
        (!smallDeath || hasSize(smallDeath, SmallWidth, SmallHeight)) &&
        hasSize(superIdle, SuperWidth, SuperHeight) &&
        hasSize(superRun1, SuperWidth, SuperHeight) &&
        hasSize(superRun2, SuperWidth, SuperHeight) &&
        hasSize(superRun3, SuperWidth, SuperHeight) &&
        hasSize(superJump, SuperWidth, SuperHeight) &&
        hasSize(superSlide, SuperWidth, SuperHeight);
}

Mario::Mario(float x, float y)
    : Character{
          x,
          y,
          makeMarioProfile(),
          makeClassicPlayerAnimationProfile(9, 25, 153)
      } {}

std::string_view Mario::getCharacterType() const {
    return "Mario";
}

bool Mario::setAnimationTextures(const MarioAnimationTextures& textures) {
    if (!textures.isValid()) {
        return false;
    }

    setPlayerAnimationProfile(
        makeIndividualMarioAnimationProfile(textures)
    );
    return true;
}
