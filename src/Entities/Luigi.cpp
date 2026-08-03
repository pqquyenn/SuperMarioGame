#include "Entities/Luigi.h"

namespace {
CharacterProfile makeLuigiProfile() {
    CharacterProfile profile;
    profile.moveAcceleration = 850.f;
    profile.walkSpeed = 150.f;
    profile.runSpeed = 220.f;
    profile.jumpForce = 400.f;
    profile.jumpHoldAcceleration = 1000.f;
    profile.maxJumpHoldTime = 0.22f;
    return profile;
}
}

Luigi::Luigi(float x, float y)
    : Character{x, y, makeLuigiProfile()} {}
