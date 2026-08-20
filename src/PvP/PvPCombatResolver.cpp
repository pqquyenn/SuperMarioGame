#include "PvP/PvPCombatResolver.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float TopCrossingTolerance = 2.f;
constexpr float PassiveStackOverlapTolerance = 4.f;
constexpr float MinimumFallSpeed = 25.f;
constexpr float MinimumRelativeFallSpeed = 25.f;
constexpr float MinimumHorizontalOverlap = 3.f;
constexpr float PushSpeedBaseline = 30.f;
constexpr float CenterTieTolerance = 0.01f;

float right(const sf::FloatRect& bounds) {
    return bounds.left + bounds.width;
}
float bottom(const sf::FloatRect& bounds) {
    return bounds.top + bounds.height;
}

float centerX(const sf::FloatRect& bounds) {
    return bounds.left + bounds.width * 0.5f;
}

float horizontalOverlap(
    const sf::FloatRect& left,
    const sf::FloatRect& rightBounds
) {
    return std::max(0.f,
                    std::min(right(left), right(rightBounds)) -
                    std::max(left.left, rightBounds.left));
}

float verticalOverlap(
    const sf::FloatRect& upper,
    const sf::FloatRect& lower
) {
    return std::max(0.f,
                    std::min(bottom(upper), bottom(lower)) -
                    std::max(upper.top, lower.top));
}

bool isPassiveVerticalStack(
    const PvPBodyFrame& upper,
    const PvPBodyFrame& lower
) {
    if (upper.currentBounds.top >= lower.currentBounds.top ||
        horizontalOverlap(upper.currentBounds, lower.currentBounds) <
            MinimumHorizontalOverlap ||
        verticalOverlap(upper.currentBounds, lower.currentBounds) >
            PassiveStackOverlapTolerance) {
        return false;
    }

    const bool upperIsFalling = upper.velocity.y >= MinimumFallSpeed;
    const bool lowerIsJumping = lower.velocity.y <= -MinimumFallSpeed;
    return !upperIsFalling && !lowerIsJumping;
}
}

PvPPushDistribution PvPCombatResolver::calculatePushDistribution(
    const PvPBodyFrame& playerOne,
    const PvPBodyFrame& playerTwo
) {
    const float currentCenterDelta =
        centerX(playerOne.currentBounds) - centerX(playerTwo.currentBounds);
    const float previousCenterDelta =
        centerX(playerOne.previousBounds) - centerX(playerTwo.previousBounds);

    bool oneIsLeft = true;
    if (std::abs(currentCenterDelta) > CenterTieTolerance) {
        oneIsLeft = currentCenterDelta < 0.f;
    } else if (std::abs(previousCenterDelta) > CenterTieTolerance) {
        oneIsLeft = previousCenterDelta < 0.f;
    } else if (std::abs(playerOne.velocity.x - playerTwo.velocity.x) >
               CenterTieTolerance) {
        // At an exact positional tie, positive relative motion means P1 came
        // from the left; negative relative motion means P1 came from right.
        oneIsLeft = playerOne.velocity.x > playerTwo.velocity.x;
    }

    const float oneApproachSpeed = oneIsLeft
        ? std::max(0.f, playerOne.velocity.x)
        : std::max(0.f, -playerOne.velocity.x);
    const float twoApproachSpeed = oneIsLeft
        ? std::max(0.f, -playerTwo.velocity.x)
        : std::max(0.f, playerTwo.velocity.x);

    // Each player's recoil share is driven by the opponent's incoming speed.
    // The baseline retains some recoil for a fast attacker and produces an
    // even split when neither player is approaching.
    const float adjustedOne = oneApproachSpeed + PushSpeedBaseline;
    const float adjustedTwo = twoApproachSpeed + PushSpeedBaseline;
    const float total = adjustedOne + adjustedTwo;
    return {oneIsLeft, adjustedTwo / total, adjustedOne / total};
}

bool PvPCombatResolver::isTrueStomp(
    const PvPBodyFrame& attacker,
    const PvPBodyFrame& target
) {
    // Relative velocity alone is not enough: a stationary upper player must
    // not receive a stomp when the lower player jumps upward into them.
    if (attacker.velocity.y < MinimumFallSpeed) {
        return false;
    }

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

    // A player resting just above another player should not be pushed merely
    // because their bounds touch. Interaction resumes once the upper player
    // falls or the lower player jumps into them.
    if (isPassiveVerticalStack(playerOne, playerTwo) ||
        isPassiveVerticalStack(playerTwo, playerOne)) {
        return PvPContactOutcome::None;
    }

    return PvPContactOutcome::PushApart;
}
