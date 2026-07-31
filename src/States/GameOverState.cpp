#include "States/GameOverState.h"
#include "States/MenuState.h"
#include <iostream>
#include <memory>

void GameOverState::onEnter() {
    std::cout << "[GameOverState] onEnter - GAME OVER" << std::endl;

    // --- Load font ---
    const std::string fontPaths[] = {
        "assets/fonts/press-start-2p.ttf",
        "../assets/fonts/press-start-2p.ttf",
        "../../assets/fonts/press-start-2p.ttf",
        "../../../assets/fonts/press-start-2p.ttf"
    };
    fontLoaded = false;
    for (const auto& path : fontPaths) {
        if (font.loadFromFile(path)) {
            fontLoaded = true;
            break;
        }
    }

    if (!fontLoaded) return;

    // --- "GAME OVER" ---
    gameOverText.setFont(font);
    gameOverText.setString("GAME OVER");
    gameOverText.setCharacterSize(36);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setOutlineColor(sf::Color::Black);
    gameOverText.setOutlineThickness(2.f);
    sf::FloatRect b = gameOverText.getLocalBounds();
    gameOverText.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    gameOverText.setPosition(400.f, 170.f);

    // --- Score ---
    scoreText.setFont(font);
    scoreText.setString("SCORE: " + std::to_string(finalScore));
    scoreText.setCharacterSize(16);
    scoreText.setFillColor(sf::Color::White);
    b = scoreText.getLocalBounds();
    scoreText.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    scoreText.setPosition(400.f, 260.f);

    // --- "TRY AGAIN" ---
    retryText.setFont(font);
    retryText.setString("TRY AGAIN");
    retryText.setCharacterSize(16);
    retryText.setFillColor(sf::Color::White);
    b = retryText.getLocalBounds();
    retryText.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    retryText.setPosition(400.f, 360.f);

    // --- "MAIN MENU" ---
    menuText.setFont(font);
    menuText.setString("MAIN MENU");
    menuText.setCharacterSize(16);
    menuText.setFillColor(sf::Color::White);
    b = menuText.getLocalBounds();
    menuText.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    menuText.setPosition(400.f, 410.f);

    // --- Selector ---
    selectorText.setFont(font);
    selectorText.setString(">");
    selectorText.setCharacterSize(16);
    selectorText.setFillColor(sf::Color::White);

    selectedIndex = 0;
    updateSelectorPosition();
}

void GameOverState::onExit() {
    std::cout << "[GameOverState] onExit - Roi khoi man hinh Game Over" << std::endl;
}

void GameOverState::handleInput(sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Enter) {
            // Nhan Enter -> quay ve Menu
            if (stateManager) {
                stateManager->changeState(std::make_unique<MenuState>());
            }
        }
    }
}

void GameOverState::update(float dt) {
    // TODO: Update animation Game Over (neu co)
}

void GameOverState::render(sf::RenderWindow& window) {
    // TODO: Ve man hinh Game Over, diem so, "Press Enter to return to Menu"
}
