#include "States/GameOverState.h"
#include "Core/AchievementSystem.h"
#include "States/PlayState.h"
#include "States/DuoState.h"
#include "States/MenuState.h"
#include "Core/SoundManager.h"
#include <iostream>
#include <memory>
#include <filesystem>

GameOverState::GameOverState(int score, const std::string& mapPath)
    : finalScore(score), currentMapPath(mapPath) {}

GameOverState::GameOverState(
    int score,
    const DuoSessionConfig& config)
    : finalScore{score},
      currentMapPath{config.mapPath},
      duoMode{true},
      duoSession{config} {}

void GameOverState::onEnter() {
    std::cout << "[GameOverState] onEnter - GAME OVER (Score: " << finalScore << ")" << std::endl;
    SoundManager::getInstance().stopBGM();
    SoundManager::getInstance().playSound("gameover");
    AchievementSystem::getInstance().recordScore(finalScore);

    // --- Load font ---
    const std::string fontPaths[] = {
        "assets/fonts/press-start-2p.ttf",
        "../assets/fonts/press-start-2p.ttf",
        "../../assets/fonts/press-start-2p.ttf",
        "../../../assets/fonts/press-start-2p.ttf"
    };
    fontLoaded = false;
    for (const auto& path : fontPaths) {
        if (std::filesystem::exists(path)) {
            if (font.loadFromFile(path)) {
                fontLoaded = true;
                break;
            }
        }
    }

    if (!fontLoaded) {
        for (const auto& path : fontPaths) {
            if (font.loadFromFile(path)) {
                fontLoaded = true;
                break;
            }
        }
    }

    if (!fontLoaded) return;

    // --- "GAME OVER" ---
    gameOverText.setFont(font);
    gameOverText.setString("GAME OVER");
    gameOverText.setCharacterSize(36);
    gameOverText.setFillColor(sf::Color(228, 50, 50));
    gameOverText.setOutlineColor(sf::Color::Black);
    gameOverText.setOutlineThickness(3.f);
    sf::FloatRect b = gameOverText.getLocalBounds();
    gameOverText.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    gameOverText.setPosition(400.f, 170.f);

    // --- Score ---
    scoreText.setFont(font);
    scoreText.setString("SCORE: " + std::to_string(finalScore));
    scoreText.setCharacterSize(18);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setOutlineColor(sf::Color::Black);
    scoreText.setOutlineThickness(2.f);
    b = scoreText.getLocalBounds();
    scoreText.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    scoreText.setPosition(400.f, 260.f);

    // --- "TRY AGAIN" ---
    retryText.setFont(font);
    retryText.setString("TRY AGAIN");
    retryText.setCharacterSize(16);
    retryText.setFillColor(sf::Color::White);
    retryText.setOutlineColor(sf::Color::Black);
    retryText.setOutlineThickness(2.f);
    b = retryText.getLocalBounds();
    retryText.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    retryText.setPosition(400.f, 360.f);

    // --- "MAIN MENU" ---
    menuText.setFont(font);
    menuText.setString("MAIN MENU");
    menuText.setCharacterSize(16);
    menuText.setFillColor(sf::Color::White);
    menuText.setOutlineColor(sf::Color::Black);
    menuText.setOutlineThickness(2.f);
    b = menuText.getLocalBounds();
    menuText.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    menuText.setPosition(400.f, 410.f);

    // --- Selector ---
    selectorText.setFont(font);
    selectorText.setString(">");
    selectorText.setCharacterSize(16);
    selectorText.setFillColor(sf::Color(228, 166, 61));
    selectorText.setOutlineColor(sf::Color::Black);
    selectorText.setOutlineThickness(2.f);

    selectedIndex = 0;
    blinkTimer = 0.f;
    showSelector = true;
    updateSelectorPosition();
}

void GameOverState::onExit() {
    std::cout << "[GameOverState] onExit - Roi khoi man hinh Game Over" << std::endl;
}

void GameOverState::handleInput(sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Up:
            case sf::Keyboard::W:
                SoundManager::getInstance().playSound("stomp");
                selectedIndex--;
                if (selectedIndex < 0) selectedIndex = MENU_ITEMS - 1;
                updateSelectorPosition();
                break;

            case sf::Keyboard::Down:
            case sf::Keyboard::S:
                SoundManager::getInstance().playSound("stomp");
                selectedIndex++;
                if (selectedIndex >= MENU_ITEMS) selectedIndex = 0;
                updateSelectorPosition();
                break;

            case sf::Keyboard::Enter:
            case sf::Keyboard::Space:
                SoundManager::getInstance().playSound("coin");
                if (selectedIndex == 0) {
                    if (stateManager) {
                        if (duoMode) {
                            stateManager->changeState(
                                std::make_unique<DuoState>(duoSession));
                        } else {
                            stateManager->changeState(
                                std::make_unique<PlayState>(currentMapPath));
                        }
                    }
                } else if (selectedIndex == 1) {
                    if (stateManager) {
                        stateManager->changeState(
                            std::make_unique<MenuState>(
                                duoMode
                                    ? MenuState::Page::DuoPlay
                                    : MenuState::Page::Play));
                    }
                }
                break;

            default:
                break;
        }
    }
}

void GameOverState::update(float dt) {
    blinkTimer += dt;
    if (blinkTimer >= 0.4f) {
        showSelector = !showSelector;
        blinkTimer = 0.f;
    }
}

void GameOverState::render(sf::RenderWindow& window) {
    // Reset view to default 800x600 window coordinates
    window.setView(window.getDefaultView());
    window.clear(sf::Color::Black);

    if (!fontLoaded) return;

    window.draw(gameOverText);
    window.draw(scoreText);
    window.draw(retryText);
    window.draw(menuText);
    if (showSelector) {
        window.draw(selectorText);
    }
}

void GameOverState::updateSelectorPosition() {
    float yPositions[] = { 360.f, 410.f };
    sf::FloatRect b = selectorText.getLocalBounds();
    selectorText.setOrigin(b.left, b.top + b.height / 2.f);
    selectorText.setPosition(270.f, yPositions[selectedIndex]);

    retryText.setFillColor(selectedIndex == 0 ? sf::Color(228, 166, 61) : sf::Color::White);
    menuText.setFillColor(selectedIndex == 1 ? sf::Color(228, 166, 61) : sf::Color::White);
}
