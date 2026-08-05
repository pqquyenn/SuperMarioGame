#pragma once

#include "Observer/Observer.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <sstream>
#include <iomanip>

// ============================================================
// HUD – Heads-Up Display hiển thị trên màn hình
// Phong cách NES Mario: MARIO, WORLD, TIME, COINS, LIVES
// Kế thừa Observer để nhận sự kiện từ hệ thống game
// ============================================================
class HUD : public Observer {
private:
    int score = 0;
    int coins = 0;
    int lives = 3;
    float timeRemaining = 400.f;
    std::string worldName = "1-1";

    sf::Font font;
    bool fontLoaded = false;

    // Nhãn cố định (Labels)
    sf::Text marioLabel;
    sf::Text worldLabel;
    sf::Text timeLabel;

    // Giá trị thay đổi (Values)
    sf::Text scoreText;
    sf::Text coinsText;
    sf::Text livesText;
    sf::Text timeText;
    sf::Text worldText;

    // Helper: format số với padding 0 phía trước
    std::string formatScore() const;
    std::string formatCoins() const;
    std::string formatTime() const;
    std::string formatLives() const;

    // Khởi tạo 1 sf::Text với cấu hình chuẩn NES
    void initText(sf::Text& text, unsigned int charSize, float x, float y);

public:
    HUD();
    void onNotify(const GameEvent& event) override;
    void update(float dt);
    void render(sf::RenderWindow& window);

    // Getters
    int getScore() const { return score; }
    int getCoins() const { return coins; }
    int getLives() const { return lives; }
    float getTimeRemaining() const { return timeRemaining; }

    // Setters
    void setWorldName(const std::string& name) { worldName = name; }
    void resetTime(float t = 400.f) { timeRemaining = t; }
};
