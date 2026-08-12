#include "States/VictoryState.h"

#include "States/GameStateManager.h"
#include "Level/StageCatalog.h"
#include "States/MenuState.h"
#include "States/PlayState.h"
#include <filesystem>
#include <iostream>
#include <memory>
#include <utility>

namespace {
constexpr float UiWidth = 800.f;
constexpr float UiHeight = 600.f;

void centerText(sf::Text& text, float x, float y) {
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(
        bounds.left + bounds.width * 0.5f,
        bounds.top + bounds.height * 0.5f);
    text.setPosition(x, y);
}
}

VictoryState::VictoryState(
    std::string completedStage,
    int score,
    std::string nextStage)
    : stageName{std::move(completedStage)},
      nextStageId{std::move(nextStage)},
      finalScore{score} {}

void VictoryState::onEnter() {
    const std::string fontPaths[] = {
        "assets/fonts/press-start-2p.ttf",
        "../assets/fonts/press-start-2p.ttf",
        "../../assets/fonts/press-start-2p.ttf",
        "../../../assets/fonts/press-start-2p.ttf"
    };

    for (const auto& path : fontPaths) {
        if (std::filesystem::exists(path) && font.loadFromFile(path)) {
            fontLoaded = true;
            break;
        }
    }

    if (!fontLoaded) {
        std::cerr << "[VictoryState] Could not load UI font" << std::endl;
        return;
    }

    titleText.setFont(font);
    titleText.setString("STAGE CLEAR");
    titleText.setCharacterSize(38);
    titleText.setFillColor(sf::Color(255, 210, 70));
    titleText.setOutlineColor(sf::Color::Black);
    titleText.setOutlineThickness(3.f);
    centerText(titleText, UiWidth * 0.5f, 180.f);

    stageText.setFont(font);
    stageText.setString(stageName.rfind("WORLD ", 0) == 0
        ? stageName : "WORLD " + stageName);
    stageText.setCharacterSize(20);
    stageText.setFillColor(sf::Color::White);
    centerText(stageText, UiWidth * 0.5f, 280.f);

    scoreText.setFont(font);
    scoreText.setString("SCORE " + std::to_string(finalScore));
    scoreText.setCharacterSize(18);
    scoreText.setFillColor(sf::Color::White);
    centerText(scoreText, UiWidth * 0.5f, 340.f);

    continueText.setFont(font);
    continueText.setString(nextStageId.empty()
        ? "PRESS ENTER FOR MENU" : "PRESS ENTER FOR NEXT STAGE");
    continueText.setCharacterSize(14);
    continueText.setFillColor(sf::Color(210, 255, 220));
    centerText(continueText, UiWidth * 0.5f, 440.f);

    std::cout << "[VictoryState] Completed " << stageName
              << " with score " << finalScore << std::endl;
}

void VictoryState::handleInput(
    sf::Event& event,
    sf::RenderWindow& window) {
    (void)window;
    if (event.type != sf::Event::KeyPressed || !stateManager) {
        return;
    }

    if (event.key.code == sf::Keyboard::Escape) {
        stateManager->changeState(std::make_unique<MenuState>());
    } else if (event.key.code == sf::Keyboard::Enter ||
               event.key.code == sf::Keyboard::Space) {
        const auto next = StageCatalog::findById(nextStageId);
        if (next) {
            stateManager->changeState(
                std::make_unique<PlayState>(next->manifestPath));
        } else {
            stateManager->changeState(std::make_unique<MenuState>());
        }
    }
}

void VictoryState::update(float dt) {
    (void)dt;
}

void VictoryState::render(sf::RenderWindow& window) {
    window.setView(sf::View(sf::FloatRect(0.f, 0.f, UiWidth, UiHeight)));
    window.clear(sf::Color(92, 148, 252));

    if (!fontLoaded) {
        return;
    }

    window.draw(titleText);
    window.draw(stageText);
    window.draw(scoreText);
    window.draw(continueText);
}
