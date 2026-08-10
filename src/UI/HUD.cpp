#include "UI/HUD.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>

static sf::Font hudFont;
static bool fontLoaded = false;

HUD::HUD() {
    if (!fontLoaded) {
        std::vector<std::string> fontPaths = {
            "assets/fonts/press-start-2p.ttf",
            "../assets/fonts/press-start-2p.ttf",
            "../../assets/fonts/press-start-2p.ttf",
            "../../../assets/fonts/press-start-2p.ttf"
        };
        for (auto& path : fontPaths) {
            if (hudFont.loadFromFile(path)) {
                fontLoaded = true;
                std::cout << "[HUD] Font loaded: " << path << std::endl;
                break;
            }
        }
        if (!fontLoaded) {
            std::cerr << "[HUD] WARNING: Could not load font!" << std::endl;
        }
    }

    unsigned int fontSize = 8;

    // MARIO  score
    scoreText.setFont(hudFont);
    scoreText.setCharacterSize(fontSize);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(16.f, 8.f);

    // Coins
    coinsText.setFont(hudFont);
    coinsText.setCharacterSize(fontSize);
    coinsText.setFillColor(sf::Color::White);
    coinsText.setPosition(130.f, 8.f);

    // World
    worldText.setFont(hudFont);
    worldText.setCharacterSize(fontSize);
    worldText.setFillColor(sf::Color::White);
    worldText.setPosition(200.f, 8.f);

    // Lives
    livesText.setFont(hudFont);
    livesText.setCharacterSize(fontSize);
    livesText.setFillColor(sf::Color::White);
    livesText.setPosition(280.f, 8.f);

    // Time
    timeText.setFont(hudFont);
    timeText.setCharacterSize(fontSize);
    timeText.setFillColor(sf::Color::White);
    timeText.setPosition(345.f, 8.f);
}

void HUD::onNotify(const GameEvent& event) {
    switch (event.type) {
        case GameEventType::COIN_COLLECTED:
            coins += 1;
            score += 200;
            break;
        case GameEventType::ENEMY_DEFEATED:
            score += 100;
            break;
        case GameEventType::PLAYER_DIED:
            lives -= 1;
            break;
        default:
            break;
    }
}

void HUD::update(float dt) {
    if (timeRemaining > 0) {
        timeRemaining -= dt;
    }

    // Update text strings
    {
        std::ostringstream ss;
        ss << "MARIO\n" << std::setw(6) << std::setfill('0') << score;
        scoreText.setString(ss.str());
    }

    {
        std::ostringstream ss;
        ss << " x" << std::setw(2) << std::setfill('0') << coins;
        coinsText.setString(ss.str());
    }

    {
        std::ostringstream ss;
        ss << "WORLD\n 1-1";
        worldText.setString(ss.str());
    }

    {
        std::ostringstream ss;
        ss << "LIVES\n  x" << lives;
        livesText.setString(ss.str());
    }

    {
        std::ostringstream ss;
        ss << "TIME\n " << std::setw(3) << std::setfill(' ') << static_cast<int>(timeRemaining);
        timeText.setString(ss.str());
    }
}

void HUD::render(sf::RenderWindow& window) {
    if (!fontLoaded) return;

    // Save current view and switch to a fixed HUD view
    sf::View oldView = window.getView();
    sf::View hudView(sf::FloatRect({0.f, 0.f}, {400.f, 225.f}));
    window.setView(hudView);

    window.draw(scoreText);
    window.draw(coinsText);
    window.draw(worldText);
    window.draw(livesText);
    window.draw(timeText);

    // Restore game camera view
    window.setView(oldView);
}
