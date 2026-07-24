#include "Entities/Mario.h"

namespace {
CharacterProfile makeMarioProfile() {
    CharacterProfile profile;
    profile.moveAcceleration = 1000.f;
    profile.walkSpeed = 170.f;
    profile.runSpeed = 260.f;
    profile.jumpForce = 350.f;
    profile.jumpHoldAcceleration = 850.f;
    profile.maxJumpHoldTime = 0.16f;
    return profile;
}
}

Mario::Mario(float x, float y)
    : Character{x, y, makeMarioProfile()} {}
