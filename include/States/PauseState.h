#pragma once

#include "States/GameState.h"
#include "States/GameStateManager.h"
#include "Duo/DuoTypes.h"
#include <SFML/Graphics.hpp>
#include <string>

class PauseState : public GameState {
private:
    // === Font ===
    sf::Font font;
    bool fontLoaded = false;

    // === UI Elements ===
    sf::RectangleShape overlay;     // Overlay den ban trong suot
    sf::Text pausedText;            // "PAUSED"
    sf::Text resumeText;            // "RESUME"
    sf::Text restartText;           // "RESTART STAGE"
    sf::Text quitText;              // "QUIT TO MENU"
    sf::Text selectorText;          // ">"

    // === Menu Navigation ===
    int selectedIndex = 0;          // 0 = RESUME, 1 = RESTART, 2 = QUIT
    static const int MENU_ITEMS = 3;
    std::string currentMapPath;
    bool duoMode{false};
    DuoSessionConfig duoSession;

    // === Animation ===
    float blinkTimer = 0.f;
    bool showSelector = true;

    // === Helper ===
    void updateSelectorPosition();

public:
    explicit PauseState(const std::string& mapPath);
    explicit PauseState(const DuoSessionConfig& config);

    void onEnter() override;
    void onExit() override;
    void handleInput(sf::Event& event, sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
    bool isTransparent() const override { return true; }
};
