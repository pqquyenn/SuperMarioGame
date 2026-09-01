#include "States/DuoLevelCompleteState.h"

#include "Core/AchievementSystem.h"
#include "Core/SoundManager.h"
#include "Duo/DuoRules.h"
#include "States/DuoState.h"
#include "States/GameStateManager.h"
#include "States/MenuState.h"

#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <sstream>
#include <utility>

namespace {
constexpr float UiWidth = 800.f;
constexpr float UiHeight = 600.f;

void centerText(sf::Text& text, float x, float y) {
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width * 0.5f,
                   bounds.top + bounds.height * 0.5f);
    text.setPosition(x, y);
}

const char* characterName(CharacterChoice choice) {
    return choice == CharacterChoice::Luigi ? "LUIGI" : "MARIO";
}
}

DuoLevelCompleteState::DuoLevelCompleteState(DuoLevelResult levelResult)
    : result{std::move(levelResult)} {}

void DuoLevelCompleteState::loadFont() {
    const std::string candidates[] = {
        "assets/fonts/press-start-2p.ttf",
        "../assets/fonts/press-start-2p.ttf",
        "../../assets/fonts/press-start-2p.ttf",
        "../../../assets/fonts/press-start-2p.ttf"
    };
    for (const auto& path : candidates) {
        if (std::filesystem::exists(path) && font.loadFromFile(path)) {
            fontLoaded = true;
            return;
        }
    }
}

void DuoLevelCompleteState::onEnter() {
    loadFont();
    SoundManager::getInstance().stopBGM();
    if (result.nextStage.empty()) {
        SoundManager::getInstance().playBGM(
            "assets/audio/music/gamewon.wav", false);
    }
    AchievementSystem::getInstance().recordScore(
        result.playerOne.score + result.playerTwo.score + result.timeBonus);
}

void DuoLevelCompleteState::onExit() {
    SoundManager::getInstance().stopBGM();
}

void DuoLevelCompleteState::handleInput(
    sf::Event& event,
    sf::RenderWindow&) {
    if (event.type != sf::Event::KeyPressed) {
        return;
    }
    if (event.key.code == sf::Keyboard::Up ||
        event.key.code == sf::Keyboard::W ||
        event.key.code == sf::Keyboard::Down ||
        event.key.code == sf::Keyboard::S) {
        selectedIndex = selectedIndex == 0 ? 1 : 0;
        SoundManager::getInstance().playSound("stomp");
        return;
    }
    if (event.key.code != sf::Keyboard::Enter &&
        event.key.code != sf::Keyboard::Space) {
        return;
    }

    SoundManager::getInstance().playSound("coin");
    if (!stateManager) {
        return;
    }
    if (selectedIndex == 0) {
        DuoSessionConfig next = result.session;
        next.mapPath = result.nextStage.empty()
            ? result.session.mapPath
            : result.nextStage;
        stateManager->changeState(std::make_unique<DuoState>(std::move(next)));
    } else {
        stateManager->changeState(
            std::make_unique<MenuState>(MenuState::Page::DuoPlay));
    }
}

void DuoLevelCompleteState::update(float dt) {
    blinkTimer += dt;
    if (blinkTimer >= 0.4f) {
        blinkTimer = 0.f;
        showSelector = !showSelector;
    }
}

void DuoLevelCompleteState::render(sf::RenderWindow& window) {
    window.setView(sf::View{sf::FloatRect{0.f, 0.f, UiWidth, UiHeight}});
    window.clear(sf::Color{18, 24, 38});
    if (!fontLoaded) {
        return;
    }

    sf::Text title{
        result.stageName + " DUO CLEAR!", font, 25};
    title.setFillColor(sf::Color{255, 220, 55});
    title.setOutlineColor(sf::Color::Black);
    title.setOutlineThickness(2.f);
    centerText(title, 400.f, 72.f);
    window.draw(title);

    const DuoMvpResult mvp = DuoRules::determineMvp(
        result.playerOne, result.playerTwo, result.completedByFlag);
    std::string award = result.completedByFlag ? "TOP JUMPER: " : "MVP: ";
    if (mvp == DuoMvpResult::Tie) {
        award += "P1 + P2 (CO-MVP)";
    } else if (mvp == DuoMvpResult::PlayerOne) {
        award += "P1 " +
            std::string{characterName(result.session.playerOneChoice)};
    } else {
        award += "P2 " +
            std::string{characterName(result.session.playerTwoChoice)};
    }
    sf::Text awardText{award, font, 12};
    awardText.setFillColor(sf::Color{105, 225, 255});
    centerText(awardText, 400.f, 118.f);
    window.draw(awardText);

    sf::RectangleShape leftPanel{{310.f, 245.f}};
    leftPanel.setPosition(65.f, 150.f);
    leftPanel.setFillColor(sf::Color{120, 25, 25, 180});
    leftPanel.setOutlineColor(sf::Color{255, 110, 90});
    leftPanel.setOutlineThickness(2.f);
    window.draw(leftPanel);

    sf::RectangleShape rightPanel = leftPanel;
    rightPanel.setPosition(425.f, 150.f);
    rightPanel.setFillColor(sf::Color{20, 95, 42, 180});
    rightPanel.setOutlineColor(sf::Color{100, 255, 140});
    window.draw(rightPanel);

    auto statsText = [this](
        const char* label,
        CharacterChoice choice,
        const DuoPlayerStats& stats,
        int lives) {
        std::ostringstream stream;
        stream << label << " " << characterName(choice) << "\n\n"
               << "SCORE       " << stats.score << "\n"
               << "COINS       " << stats.coins << "\n"
               << "ENEMIES     " << stats.enemiesDefeated << "\n"
               << "DEATHS      " << stats.deaths << "\n"
               << "RESCUES     " << stats.rescuesPerformed << "\n"
               << "LIVES       " << lives;
        if (result.completedByFlag) {
            stream << "\nFLAG HEIGHT "
                   << static_cast<int>(std::max(0.f, stats.flagHeight));
        }
        return stream.str();
    };

    sf::Text left{
        statsText("P1", result.session.playerOneChoice,
                  result.playerOne, result.playerOneLives),
        font,
        10};
    left.setPosition(88.f, 172.f);
    left.setLineSpacing(1.25f);
    window.draw(left);

    sf::Text right{
        statsText("P2", result.session.playerTwoChoice,
                  result.playerTwo, result.playerTwoLives),
        font,
        10};
    right.setPosition(448.f, 172.f);
    right.setLineSpacing(1.25f);
    window.draw(right);

    std::ostringstream team;
    team << "TEAM TIME BONUS +" << result.timeBonus
         << "    TOTAL "
         << result.playerOne.score + result.playerTwo.score + result.timeBonus;
    sf::Text teamText{team.str(), font, 10};
    teamText.setFillColor(sf::Color::White);
    centerText(teamText, 400.f, 425.f);
    window.draw(teamText);

    const std::string optionOne = result.nextStage.empty()
        ? "PLAY AGAIN"
        : "NEXT LEVEL";
    sf::Text next{optionOne, font, 13};
    sf::Text menu{"MAIN MENU", font, 13};
    next.setFillColor(selectedIndex == 0
                          ? sf::Color{255, 220, 55}
                          : sf::Color::White);
    menu.setFillColor(selectedIndex == 1
                          ? sf::Color{255, 220, 55}
                          : sf::Color::White);
    centerText(next, 400.f, 485.f);
    centerText(menu, 400.f, 535.f);
    window.draw(next);
    window.draw(menu);

    if (showSelector) {
        sf::Text selector{">", font, 14};
        selector.setFillColor(sf::Color{255, 220, 55});
        selector.setPosition(285.f, selectedIndex == 0 ? 474.f : 524.f);
        window.draw(selector);
    }
}
