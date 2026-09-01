#pragma once

#include "Duo/DuoTypes.h"
#include "Input/InputHandler.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

struct DuoTetherResult {
    InputPermissions playerOne;
    InputPermissions playerTwo;
    bool playerOneIsLeft{true};
    float separation{0.f};
};

class DuoRules {
public:
    static DuoTetherResult calculateTether(
        const sf::FloatRect& playerOne,
        const sf::FloatRect& playerTwo,
        float maximumSeparation
    );

    static sf::Vector2f calculateCameraFocus(
        const sf::FloatRect& playerOne,
        const sf::FloatRect& playerTwo
    );

    static DuoMvpResult determineMvp(
        const DuoPlayerStats& playerOne,
        const DuoPlayerStats& playerTwo,
        bool useFlagHeight
    );
};
