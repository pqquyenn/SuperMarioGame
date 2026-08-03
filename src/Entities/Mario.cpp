#include "Entities/Mario.h"

namespace {
CharacterProfile makeMarioProfile() {
    CharacterProfile profile;
    profile.moveAcceleration = 1000.f;
    profile.walkSpeed = 170.f;
    profile.runSpeed = 260.f;
    profile.jumpForce = 350.f;
    profile.jumpHoldGravityMultiplier = 0.45f;
    profile.jumpReleaseGravityMultiplier = 2.5f;
    profile.maxJumpHoldTime = 0.16f;
    return profile;
}
}

Mario::Mario(float x, float y)
    : Character{
          x,
          y,
          makeMarioProfile(),
          makeClassicPlayerAnimationProfile(9, 25, 153)
      } {}
