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

struct PvPPushDistribution {
    bool playerOneIsLeft{true};
    // Fraction of total separation/recoil received by each player.
    float playerOneShare{0.5f};
    float playerTwoShare{0.5f};
};

class PvPCombatResolver {
public:
    static PvPContactOutcome classifyPlayerContact(
        const PvPBodyFrame& playerOne,
        const PvPBodyFrame& playerTwo
    );
    static PvPPushDistribution calculatePushDistribution(
        const PvPBodyFrame& playerOne,
        const PvPBodyFrame& playerTwo
    );

private:
    static bool isTrueStomp(
        const PvPBodyFrame& attacker,
        const PvPBodyFrame& target
    );
};
