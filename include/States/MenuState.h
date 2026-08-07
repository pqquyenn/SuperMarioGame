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
    sf::Text startText;         // "PLAY 1-1"
    sf::Text play12Text;        // "PLAY 1-2"
    sf::Text play13Text;        // "PLAY 1-3"
    sf::Text exitText;          // "EXIT"
    sf::Text selectorText;      // ">" ky tu chi muc dang chon

    // === Menu Navigation ===
    int selectedIndex = 0;      // 0 = PLAY 1-1, 1 = PLAY 1-2, 2 = PLAY 1-3, 3 = EXIT
    static const int MENU_ITEMS = 4;

    // === Animation ===
    float blinkTimer = 0.f;
    bool showSelector = true;

    // === Background ===
    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    bool bgLoaded = false;
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
