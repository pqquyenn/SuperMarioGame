#pragma once

#include "PvP/PvPTypes.h"

struct PvPRuleset {
    int startingLives{3};
    bool playersCanDamageEachOther{true};
    bool startsPowered{false};
    bool fireFlowersEnabled{false};
    bool timedMatch{false};
    bool refreshArena{false};
    float arenaRefreshInterval{0.f};

    static PvPRuleset forMatch(PvPMatchType type);
};
