#pragma once

#include "PvP/PvPTypes.h"

enum class PvPWinner {
    None,
    PlayerOne,
    PlayerTwo,
    Draw
};

class PvPMatchRules {
public:
    static int friendlyDeathScore(int currentScore);
    static PvPWinner determineWinner(
        PvPMatchType type,
        int playerOneLives,
        int playerTwoLives,
        int playerOneScore,
        int playerTwoScore,
        float timeRemaining);
};
