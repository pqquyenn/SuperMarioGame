#include "PvP/PvPCombatResolver.h"

#include <algorithm>

namespace {
constexpr float TopCrossingTolerance = 2.f;
constexpr float MinimumRelativeFallSpeed = 25.f;
constexpr float MinimumHorizontalOverlap = 3.f;

float right(const sf::FloatRect& bounds) {
    return bounds.left + bounds.width;
}
float bottom(const sf::FloatRect& bounds) {
    return bounds.top + bounds.height;
}

float horizontalOverlap(
    const sf::FloatRect& left,
    const sf::FloatRect& rightBounds
) {
    return std::max(0.f,
                    std::min(right(left), right(rightBounds)) -
                    std::max(left.left, rightBounds.left));
}
}

bool PvPCombatResolver::isTrueStomp(
    const PvPBodyFrame& attacker,
    const PvPBodyFrame& target
) {
    const float relativeFallSpeed =
        attacker.velocity.y - target.velocity.y;
    if (relativeFallSpeed < MinimumRelativeFallSpeed) {
        return false;
    }

    if (bottom(attacker.previousBounds) >
        target.previousBounds.top + TopCrossingTolerance) {
        return false;
    }

    if (bottom(attacker.currentBounds) < target.currentBounds.top) {
        return false;
    }

    return horizontalOverlap(attacker.currentBounds,
                             target.currentBounds) >=
           MinimumHorizontalOverlap;
}

PvPContactOutcome PvPCombatResolver::classifyPlayerContact(
    const PvPBodyFrame& playerOne,
    const PvPBodyFrame& playerTwo
) {
    if (!playerOne.currentBounds.intersects(playerTwo.currentBounds)) {
        return PvPContactOutcome::None;
    }

    const bool oneStomps = isTrueStomp(playerOne, playerTwo);
    const bool twoStomps = isTrueStomp(playerTwo, playerOne);

    if (oneStomps != twoStomps) {
        return oneStomps
            ? PvPContactOutcome::PlayerOneStomps
            : PvPContactOutcome::PlayerTwoStomps;
    }

    return PvPContactOutcome::PushApart;
}
