#include "Duo/DuoRules.h"

#include <algorithm>
#include <cmath>
#include <tuple>

namespace {
float centerX(const sf::FloatRect& bounds) {
    return bounds.left + bounds.width * 0.5f;
}

sf::Vector2f center(const sf::FloatRect& bounds) {
    return {bounds.left + bounds.width * 0.5f,
            bounds.top + bounds.height * 0.5f};
}
}

DuoTetherResult DuoRules::calculateTether(
    const sf::FloatRect& playerOne,
    const sf::FloatRect& playerTwo,
    float maximumSeparation
) {
    DuoTetherResult result;
    const float oneX = centerX(playerOne);
    const float twoX = centerX(playerTwo);
    result.playerOneIsLeft = oneX <= twoX;
    result.separation = std::abs(oneX - twoX);

    if (result.separation + 0.01f < std::max(0.f, maximumSeparation)) {
        return result;
    }

    if (result.playerOneIsLeft) {
        result.playerOne.allowMoveLeft = false;
        result.playerTwo.allowMoveRight = false;
    } else {
        result.playerOne.allowMoveRight = false;
        result.playerTwo.allowMoveLeft = false;
    }
    return result;
}

sf::Vector2f DuoRules::calculateCameraFocus(
    const sf::FloatRect& playerOne,
    const sf::FloatRect& playerTwo
) {
    const sf::Vector2f one = center(playerOne);
    const sf::Vector2f two = center(playerTwo);
    return {(one.x + two.x) * 0.5f, (one.y + two.y) * 0.5f};
}

DuoMvpResult DuoRules::determineMvp(
    const DuoPlayerStats& playerOne,
    const DuoPlayerStats& playerTwo,
    bool useFlagHeight
) {
    if (useFlagHeight &&
        std::abs(playerOne.flagHeight - playerTwo.flagHeight) > 0.01f) {
        return playerOne.flagHeight > playerTwo.flagHeight
            ? DuoMvpResult::PlayerOne
            : DuoMvpResult::PlayerTwo;
    }

    const auto one = std::make_tuple(
        playerOne.score,
        playerOne.enemiesDefeated,
        playerOne.coins,
        playerOne.rescuesPerformed);
    const auto two = std::make_tuple(
        playerTwo.score,
        playerTwo.enemiesDefeated,
        playerTwo.coins,
        playerTwo.rescuesPerformed);
    if (one == two) {
        return DuoMvpResult::Tie;
    }
    return one > two ? DuoMvpResult::PlayerOne : DuoMvpResult::PlayerTwo;
}
