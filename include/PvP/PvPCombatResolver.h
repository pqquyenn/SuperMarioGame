#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

enum class PvPContactOutcome {
    None,
    PlayerOneStomps,
    PlayerTwoStomps,
    PushApart
};
struct PvPBodyFrame {
    sf::FloatRect previousBounds;
    sf::FloatRect currentBounds;
    sf::Vector2f velocity;
};

class PvPCombatResolver {
public:
    static PvPContactOutcome classifyPlayerContact(
        const PvPBodyFrame& playerOne,
        const PvPBodyFrame& playerTwo
    );

private:
    static bool isTrueStomp(
        const PvPBodyFrame& attacker,
        const PvPBodyFrame& target
    );
};
