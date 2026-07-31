#pragma once

#include "States/GameState.h"
#include "States/GameStateManager.h"
#include <SFML/Graphics.hpp>

class GameOverState : public GameState {
private:
    // === Font ===
    sf::Font font;
    bool fontLoaded = false;

    // === UI Elements ===
    sf::Text gameOverText;      // "GAME OVER"
    sf::Text scoreText;         // "SCORE: XXXXX"
    sf::Text retryText;         // "TRY AGAIN"
    sf::Text menuText;          // "MAIN MENU"
    sf::Text selectorText;      // ">"

    // === Data ===
    int finalScore = 0;

    // === Menu Navigation ===
    int selectedIndex = 0;      // 0 = TRY AGAIN, 1 = MAIN MENU
    static const int MENU_ITEMS = 2;

    // === Animation ===
    float blinkTimer = 0.f;
    bool showSelector = true;

    // === Helper ===
    void updateSelectorPosition();

public:
    GameOverState() = default;
    explicit GameOverState(int score);

    void onEnter() override;
    void onExit() override;
    void handleInput(sf::Event& event, sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
