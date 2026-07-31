#pragma once

#include "States/GameState.h"
#include "States/GameStateManager.h"
#include <SFML/Graphics.hpp>

class MenuState : public GameState {
private:
    // === Font ===
    sf::Font font;
    bool fontLoaded = false;

    // === Texts ===
    sf::Text titleText;         // "SUPER MARIO BROS"
    sf::Text subtitleText;      // "© Nintendo / Student Project"
    sf::Text startText;         // "START GAME"
    sf::Text exitText;          // "EXIT"
    sf::Text selectorText;      // ">" ky tu chi muc dang chon

    // === Menu Navigation ===
    int selectedIndex = 0;      // 0 = START GAME, 1 = EXIT
    static const int MENU_ITEMS = 2;

    // === Animation ===
    float blinkTimer = 0.f;
    bool showSelector = true;

    // === Background ===
    sf::RectangleShape groundBlock;   // Gia lap dat nen phia duoi

    // === Helpers ===
    void updateSelectorPosition();

public:
    MenuState() = default;

    void onEnter() override;
    void onExit() override;
    void handleInput(sf::Event& event, sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
