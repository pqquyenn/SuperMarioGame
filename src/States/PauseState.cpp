#include "States/PauseState.h"
#include "States/MenuState.h"
#include <iostream>
#include <memory>

void PauseState::onEnter() {
    std::cout << "[PauseState] onEnter - Game da tam dung" << std::endl;

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

    // --- Overlay den ban trong suot ---
    overlay.setSize(sf::Vector2f(800.f, 600.f));
    overlay.setFillColor(sf::Color(0, 0, 0, 140));  // Den ~55% opacity

    // --- Text "PAUSED" ---
    if (fontLoaded) {
        pausedText.setFont(font);
        pausedText.setString("PAUSED");
        pausedText.setCharacterSize(36);
        pausedText.setFillColor(sf::Color::White);
        pausedText.setOutlineColor(sf::Color::Black);
        pausedText.setOutlineThickness(2.f);
        sf::FloatRect b = pausedText.getLocalBounds();
        pausedText.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
        pausedText.setPosition(400.f, 200.f);

        // --- "RESUME" ---
        resumeText.setFont(font);
        resumeText.setString("RESUME");
        resumeText.setCharacterSize(16);
        resumeText.setFillColor(sf::Color::White);
        b = resumeText.getLocalBounds();
        resumeText.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
        resumeText.setPosition(400.f, 320.f);

        // --- "QUIT TO MENU" ---
        quitText.setFont(font);
        quitText.setString("QUIT TO MENU");
        quitText.setCharacterSize(16);
        quitText.setFillColor(sf::Color::White);
        b = quitText.getLocalBounds();
        quitText.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
        quitText.setPosition(400.f, 370.f);

        // --- Selector ">" ---
        selectorText.setFont(font);
        selectorText.setString(">");
        selectorText.setCharacterSize(16);
        selectorText.setFillColor(sf::Color::White);
    }

    selectedIndex = 0;
    updateSelectorPosition();
}

void PauseState::onExit() {
    std::cout << "[PauseState] onExit - Tiep tuc game" << std::endl;
}

void PauseState::handleInput(sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            // Nhan Escape -> pop PauseState -> quay ve PlayState
            if (stateManager) {
                stateManager->popState();
            }
        }
        else if (event.key.code == sf::Keyboard::Q) {
            // Nhan Q -> quay ve Menu (thay the toan bo stack)
            if (stateManager) {
                stateManager->changeState(std::make_unique<MenuState>());
            }
        }
    }
}

void PauseState::update(float dt) {
    // Pause state: khong update game logic
    // TODO: Update animation pause menu (neu co)
}

void PauseState::render(sf::RenderWindow& window) {
    // TODO: Ve overlay ban trong suot len tren PlayState
    // TODO: Ve text "PAUSED", "Press ESC to Resume", "Press Q to Quit"
}
