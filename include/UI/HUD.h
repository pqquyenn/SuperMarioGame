#pragma once

#include "Observer/Observer.h"
#include <SFML/Graphics.hpp>
#include <string>

class HUD : public Observer {
private:
    int score = 0;
    int coins = 0;
    int lives = 3;
    float timeRemaining = 400.f;
    std::string playerName{"MARIO"};
    std::string levelName{"1-1"};

    sf::Text scoreText;
    sf::Text coinsText;
    sf::Text livesText;
    sf::Text worldText;
    sf::Text timeText;

public:
    HUD();
    void onNotify(const GameEvent& event) override;
    void update(float dt);
    void render(sf::RenderWindow& window);
    void setPlayerName(const std::string& name) { playerName = name; }
    void setLevelName(const std::string& name) { levelName = name; }
    void setTimeLimit(float seconds) { timeRemaining = seconds; }

    int getLives() const { return lives; }
    int getScore() const { return score; }
    float getTimeRemaining() const { return timeRemaining; }
};
