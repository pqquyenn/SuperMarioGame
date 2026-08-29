#include "States/PauseState.h"
#include "States/MenuState.h"
#include "States/PlayState.h"
#include "Core/SoundManager.h"
#include <iostream>
#include <memory>

PauseState::PauseState(const std::string& mapPath)
    : currentMapPath(mapPath) {}

void PauseState::onEnter() {
    std::cout << "[PauseState] onEnter - Game da tam dung" << std::endl;
    SoundManager::getInstance().playSound("pause");

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

        // --- "RESTART STAGE" ---
        restartText.setFont(font);
        restartText.setString("RESTART");
        restartText.setCharacterSize(16);
        restartText.setFillColor(sf::Color::White);
        b = restartText.getLocalBounds();
        restartText.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
        restartText.setPosition(400.f, 370.f);

        // --- "QUIT TO MENU" ---
        quitText.setFont(font);
        quitText.setString("QUIT TO MENU");
        quitText.setCharacterSize(16);
        quitText.setFillColor(sf::Color::White);
        b = quitText.getLocalBounds();
        quitText.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
        quitText.setPosition(400.f, 420.f);

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
    std::cout << "[PauseState] Leaving pause menu" << std::endl;
}

void PauseState::handleInput(sf::Event& event, sf::RenderWindow& window) {
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
                if (stateManager) {
                    if (selectedIndex == 0) {
                        // RESUME -> pop PauseState, quay ve PlayState
                        SoundManager::getInstance().playSound("pause");
                        stateManager->popState();
                    } else if (selectedIndex == 1) {
                        // RESTART STAGE -> tao PlayState moi tu map hien tai
                        SoundManager::getInstance().playSound("coin");
                        stateManager->clearAndPushState(
                            std::make_unique<PlayState>(currentMapPath));
                    } else if (selectedIndex == 2) {
                        // QUIT TO MENU -> quay ve trang chon map Solo
                        SoundManager::getInstance().playSound("coin");
                        stateManager->clearAndPushState(
                            std::make_unique<MenuState>(MenuState::Page::Play));
                    }
                }
                break;

            case sf::Keyboard::Escape:
                // Phim tat: Escape luon = Resume
                SoundManager::getInstance().playSound("pause");
                if (stateManager) {
                    stateManager->popState();
                }
                break;

            default:
                break;
        }
    }
}

void PauseState::update(float dt) {
    // Game logic KHONG update (dang pause)
    // Chi update animation nhap nhay selector
    blinkTimer += dt;
    if (blinkTimer >= 0.4f) {
        showSelector = !showSelector;
        blinkTimer = 0.f;
    }
}

void PauseState::render(sf::RenderWindow& window) {
    const sf::View previousView = window.getView();
    const sf::View uiView(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
    window.setView(uiView);

    // 1. Ve overlay den ban trong suot
    window.draw(overlay);

    if (!fontLoaded) {
        window.setView(previousView);
        return;
    }

    // 2. Ve texts
    window.draw(pausedText);
    window.draw(resumeText);
    window.draw(restartText);
    window.draw(quitText);

    // 3. Ve selector nhap nhay
    if (showSelector) {
        window.draw(selectorText);
    }

    window.setView(previousView);
}

void PauseState::updateSelectorPosition() {
    float yPositions[] = { 320.f, 370.f, 420.f };
    selectorText.setPosition(260.f, yPositions[selectedIndex]);

    // Highlight muc dang chon = vang, muc khac = trang
    resumeText.setFillColor(selectedIndex == 0 ? sf::Color(228, 166, 61) : sf::Color::White);
    restartText.setFillColor(selectedIndex == 1 ? sf::Color(228, 166, 61) : sf::Color::White);
    quitText.setFillColor(selectedIndex == 2 ? sf::Color(228, 166, 61) : sf::Color::White);
}
