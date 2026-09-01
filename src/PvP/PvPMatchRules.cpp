#include "PvP/PvPMatchRules.h"

#include <algorithm>

int PvPMatchRules::friendlyDeathScore(int currentScore) {
    const int score = std::max(0, currentScore);
    if (score < 100) {
        return 0;
    }
    if (score <= 1000) {
        return 5 * (score - 100) / 6;
    }
    return score - 250;
}

PvPWinner PvPMatchRules::determineWinner(
    PvPMatchType type,
    int playerOneLives,
    int playerTwoLives,
    int playerOneScore,
    int playerTwoScore,
    float timeRemaining) {
    if (type == PvPMatchType::Friendly) {
        if (timeRemaining > 0.f) {
            return PvPWinner::None;
        }
        if (playerOneScore == playerTwoScore) {
            return PvPWinner::Draw;
        }
        return playerOneScore > playerTwoScore
            ? PvPWinner::PlayerOne
            : PvPWinner::PlayerTwo;
    }

    if (playerOneLives > 0 && playerTwoLives > 0) {
        return PvPWinner::None;
    }
    if (playerOneLives <= 0 && playerTwoLives <= 0) {
        return PvPWinner::Draw;
    }
    return playerOneLives <= 0
        ? PvPWinner::PlayerTwo
        : PvPWinner::PlayerOne;
}
