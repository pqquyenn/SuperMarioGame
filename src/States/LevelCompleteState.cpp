#include "States/LevelCompleteState.h"
#include "Core/AchievementSystem.h"
#include "States/PlayState.h"
#include "States/MenuState.h"
#include <iostream>
#include <memory>
#include <filesystem>

LevelCompleteState::LevelCompleteState(
    int id,
    const std::string& mapPath,
    const std::string& nextStage,
    int score,
    int coins,
    int bonus)
    : levelId(id),
      completedMapPath(mapPath),
      nextStagePath(nextStage),
      finalScore(score),
      coinsCollected(coins),
      timeBonus(bonus),
      isLastLevel(nextStage.empty()) {
}

std::string LevelCompleteState::getNextLevelPath() const {
    return nextStagePath.empty() ? completedMapPath : nextStagePath;
}

void LevelCompleteState::onEnter() {
    std::cout << "[LevelCompleteState] onEnter - Level " << levelId
              << " Completed! Score: " << finalScore << " Coins: " << coinsCollected
              << " TimeBonus: " << timeBonus << std::endl;
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
        if (std::filesystem::exists(path) && font.loadFromFile(path)) {
            fontLoaded = true;
            break;
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

    // --- Title ---
    clearTitleText.setFont(font);
    if (isLastLevel) {
        clearTitleText.setString("CONGRATULATIONS!");
        clearTitleText.setCharacterSize(24);
        clearTitleText.setFillColor(sf::Color(255, 215, 0)); // Gold
    } else {
        std::string worldName = (levelId == 1 ? "1-1" : (levelId == 2 ? "1-2" : "1-3"));
        clearTitleText.setString("WORLD " + worldName + " CLEAR!");
        clearTitleText.setCharacterSize(28);
        clearTitleText.setFillColor(sf::Color(92, 228, 50)); // Bright Green
    }
    clearTitleText.setOutlineColor(sf::Color::Black);
    clearTitleText.setOutlineThickness(3.f);
    sf::FloatRect b = clearTitleText.getLocalBounds();
    clearTitleText.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    clearTitleText.setPosition(400.f, 130.f);

    // --- Subtitle (if last level) ---
    if (isLastLevel) {
        subTitleText.setFont(font);
        subTitleText.setString("ALL WORLDS COMPLETED!");
        subTitleText.setCharacterSize(14);
        subTitleText.setFillColor(sf::Color::White);
        subTitleText.setOutlineColor(sf::Color::Black);
        subTitleText.setOutlineThickness(2.f);
        sf::FloatRect sb = subTitleText.getLocalBounds();
        subTitleText.setOrigin(sb.left + sb.width / 2.f, sb.top + sb.height / 2.f);
        subTitleText.setPosition(400.f, 175.f);
    }

    // --- Score ---
    scoreText.setFont(font);
    scoreText.setString("TOTAL SCORE: " + std::to_string(finalScore));
    scoreText.setCharacterSize(16);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setOutlineColor(sf::Color::Black);
    scoreText.setOutlineThickness(2.f);
    b = scoreText.getLocalBounds();
    scoreText.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    scoreText.setPosition(400.f, 225.f);

    // --- Coins ---
    coinsText.setFont(font);
    coinsText.setString("COINS COLLECTED: " + std::to_string(coinsCollected));
    coinsText.setCharacterSize(14);
    coinsText.setFillColor(sf::Color(255, 230, 100));
    coinsText.setOutlineColor(sf::Color::Black);
    coinsText.setOutlineThickness(2.f);
    b = coinsText.getLocalBounds();
    coinsText.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    coinsText.setPosition(400.f, 265.f);

    // --- Time Bonus ---
    timeBonusText.setFont(font);
    timeBonusText.setString("TIME BONUS: +" + std::to_string(timeBonus));
    timeBonusText.setCharacterSize(14);
    timeBonusText.setFillColor(sf::Color(100, 220, 255));
    timeBonusText.setOutlineColor(sf::Color::Black);
    timeBonusText.setOutlineThickness(2.f);
    b = timeBonusText.getLocalBounds();
    timeBonusText.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    timeBonusText.setPosition(400.f, 305.f);

    // --- Option 1: "NEXT LEVEL" or "PLAY AGAIN" ---
    option1Text.setFont(font);
    option1Text.setString(isLastLevel ? "PLAY AGAIN" : "NEXT LEVEL");
    option1Text.setCharacterSize(16);
    option1Text.setFillColor(sf::Color::White);
    option1Text.setOutlineColor(sf::Color::Black);
    option1Text.setOutlineThickness(2.f);
    b = option1Text.getLocalBounds();
    option1Text.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    option1Text.setPosition(400.f, 380.f);

    // --- Option 2: "MAIN MENU" ---
    option2Text.setFont(font);
    option2Text.setString("MAIN MENU");
    option2Text.setCharacterSize(16);
    option2Text.setFillColor(sf::Color::White);
    option2Text.setOutlineColor(sf::Color::Black);
    option2Text.setOutlineThickness(2.f);
    b = option2Text.getLocalBounds();
    option2Text.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    option2Text.setPosition(400.f, 430.f);

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

void LevelCompleteState::onExit() {
    std::cout << "[LevelCompleteState] Leaving level complete screen" << std::endl;
}

void LevelCompleteState::handleInput(sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Up:
            case sf::Keyboard::W:
                selectedIndex--;
                if (selectedIndex < 0) selectedIndex = MENU_ITEMS - 1;
                updateSelectorPosition();
                break;

            case sf::Keyboard::Down:
            case sf::Keyboard::S:
                selectedIndex++;
                if (selectedIndex >= MENU_ITEMS) selectedIndex = 0;
                updateSelectorPosition();
                break;

            case sf::Keyboard::Enter:
            case sf::Keyboard::Space:
                if (selectedIndex == 0) {
                    // NEXT LEVEL (or PLAY AGAIN from 1-1)
                    if (stateManager) {
                        std::string nextMap = getNextLevelPath();
                        std::cout << "[LevelCompleteState] Starting next level: " << nextMap << std::endl;
                        stateManager->changeState(std::make_unique<PlayState>(nextMap));
                    }
                } else if (selectedIndex == 1) {
                    // MAIN MENU -> Return to MenuState
                    if (stateManager) {
                        stateManager->changeState(std::make_unique<MenuState>());
                    }
                }
                break;

            default:
                break;
        }
    }
}

void LevelCompleteState::update(float dt) {
    blinkTimer += dt;
    if (blinkTimer >= 0.4f) {
        showSelector = !showSelector;
        blinkTimer = 0.f;
    }
}

void LevelCompleteState::render(sf::RenderWindow& window) {
    window.setView(window.getDefaultView());
    window.clear(sf::Color(20, 24, 32)); // Sleek dark slate background

    if (!fontLoaded) return;

    window.draw(clearTitleText);
    if (isLastLevel) {
        window.draw(subTitleText);
    }
    window.draw(scoreText);
    window.draw(coinsText);
    window.draw(timeBonusText);
    window.draw(option1Text);
    window.draw(option2Text);
    if (showSelector) {
        window.draw(selectorText);
    }
}

void LevelCompleteState::updateSelectorPosition() {
    float yPositions[] = { 380.f, 430.f };
    sf::FloatRect b = selectorText.getLocalBounds();
    selectorText.setOrigin(b.left, b.top + b.height / 2.f);
    selectorText.setPosition(250.f, yPositions[selectedIndex]);

    option1Text.setFillColor(selectedIndex == 0 ? sf::Color(228, 166, 61) : sf::Color::White);
    option2Text.setFillColor(selectedIndex == 1 ? sf::Color(228, 166, 61) : sf::Color::White);
}
