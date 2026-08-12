#pragma once

#include "Level/LevelDefinition.h"
#include <string>

class Character;

class PortalSystem {
public:
    bool tryActivate(
        const LevelDefinition& definition,
        std::string& currentArea,
        Character& character,
        const sf::FloatRect& contactedTile,
        PortalActivation activation) const;
};
