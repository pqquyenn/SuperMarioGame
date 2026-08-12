#pragma once

#include "States/GameState.h"
#include <SFML/Graphics.hpp>
#include <string>

class VictoryState final : public GameState {
private:
    std::string stageName;
    std::string nextStageId;
    int finalScore{0};
    sf::Font font;
    bool fontLoaded{false};
    sf::Text titleText;
    sf::Text stageText;
    sf::Text scoreText;
    sf::Text continueText;

public:
    VictoryState(
        std::string completedStage,
        int score,
        std::string nextStage = {});

    void onEnter() override;
    void handleInput(sf::Event& event, sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
