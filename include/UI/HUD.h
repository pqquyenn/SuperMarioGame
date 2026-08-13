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
    bool timeFrozen = false;
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
    void addScore(int points) { score += points; }
    void setTimeRemaining(float time) { timeRemaining = time; }
    void setTimeFrozen(bool frozen) { timeFrozen = frozen; }
    bool isTimeFrozen() const { return timeFrozen; }

    int getLives() const { return lives; }
    int getScore() const { return score; }
    int getCoins() const { return coins; }
    float getTimeRemaining() const { return timeRemaining; }
    const std::string& getLevelName() const { return levelName; }
    const std::string& getPlayerName() const { return playerName; }
};
