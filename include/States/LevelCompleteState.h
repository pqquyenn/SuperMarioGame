#pragma once

#include "States/GameState.h"
#include "States/GameStateManager.h"
#include <SFML/Graphics.hpp>
#include <string>

class LevelCompleteState : public GameState {
private:
    // === Font ===
    sf::Font font;
    bool fontLoaded = false;

    // === UI Elements ===
    sf::Text clearTitleText;    // "WORLD 1-1 CLEAR!" or "CONGRATULATIONS!"
    sf::Text subTitleText;      // "ALL WORLDS COMPLETED!" (if last level)
    sf::Text scoreText;         // "SCORE: XXXXX"
    sf::Text coinsText;         // "COINS: XX"
    sf::Text timeBonusText;     // "TIME BONUS: XXXX"
    sf::Text option1Text;       // "NEXT LEVEL" or "PLAY AGAIN"
    sf::Text option2Text;       // "MAIN MENU"
    sf::Text selectorText;      // ">"

    // === Data ===
    int levelId = 1;
    std::string completedMapPath;
    std::string nextStagePath;
    int finalScore = 0;
    int coinsCollected = 0;
    int timeBonus = 0;
    bool isLastLevel = false;

    // === Menu Navigation ===
    int selectedIndex = 0;      // 0 = Option 1, 1 = Option 2
    static constexpr int MENU_ITEMS = 2;

    // === Animation ===
    float blinkTimer = 0.f;
    bool showSelector = true;

    // === Helpers ===
    void updateSelectorPosition();
    std::string getNextLevelPath() const;

public:
    LevelCompleteState(
        int levelId,
        const std::string& mapPath,
        const std::string& nextStage,
        int score,
        int coins,
        int timeBonus);

    void onEnter() override;
    void onExit() override;
    void handleInput(sf::Event& event, sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
