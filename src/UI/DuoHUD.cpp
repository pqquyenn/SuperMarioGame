#include "UI/DuoHUD.h"

#include "Entities/Character.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace {
constexpr float HudWidth = 400.f;
constexpr float HudHeight = 225.f;

std::string formatWorldNumber(const std::string& raw) {
    std::string s = raw;
    while (!s.empty() && s.front() == ' ') s.erase(s.begin());
    if (s.rfind("WORLD ", 0) == 0 || s.rfind("world ", 0) == 0 ||
        s.rfind("World ", 0) == 0) {
        s = s.substr(6);
    }
    while (!s.empty() && s.front() == ' ') s.erase(s.begin());
    return s.empty() ? "1-1" : s;
}

void centerText(sf::Text& text, float x, float y) {
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(std::round(bounds.left + bounds.width * 0.5f),
                   std::round(bounds.top + bounds.height * 0.5f));
    text.setPosition(std::round(x), std::round(y));
}

sf::Color avatarColor(const std::string& name) {
    return name == "LUIGI"
        ? sf::Color{35, 190, 75}
        : sf::Color{225, 55, 45};
}

char avatarLetter(const std::string& name) {
    return name == "LUIGI" ? 'L' : 'M';
}
}

DuoHUD::DuoHUD() {
    const std::string candidates[] = {
        "assets/fonts/press-start-2p.ttf",
        "../assets/fonts/press-start-2p.ttf",
        "../../assets/fonts/press-start-2p.ttf",
        "../../../assets/fonts/press-start-2p.ttf"
    };
    for (const auto& path : candidates) {
        if (std::filesystem::exists(path) && font.loadFromFile(path)) {
            fontLoaded = true;
            const_cast<sf::Texture&>(font.getTexture(8)).setSmooth(false);
            break;
        }
    }
}

const char* DuoHUD::lifeStateName(DuoLifeState state) {
    switch (state) {
        case DuoLifeState::Active: return "";
        case DuoLifeState::Dying: return "DOWN";
        case DuoLifeState::Bubble: return "BUBBLE";
        case DuoLifeState::Out: return "OUT";
    }
    return "";
}

void DuoHUD::render(
    sf::RenderWindow& window,
    const DuoHudPlayerData& playerOne,
    const DuoHudPlayerData& playerTwo,
    const std::string& worldName,
    float timeRemaining) const {
    if (!fontLoaded) {
        return;
    }

    const sf::View previous = window.getView();
    window.setView(sf::View{sf::FloatRect{0.f, 0.f, HudWidth, HudHeight}});

    sf::RectangleShape background{{HudWidth, 34.f}};
    background.setFillColor(sf::Color{0, 0, 0, 185});
    window.draw(background);

    const_cast<sf::Texture&>(font.getTexture(8)).setSmooth(false);

    auto drawPlayer = [this, &window](
        const DuoHudPlayerData& data,
        bool rightAligned) {
        const float avatarX = rightAligned ? 388.f : 12.f;
        sf::CircleShape avatar{8.f};
        avatar.setOrigin(8.f, 8.f);
        avatar.setPosition(avatarX, 17.f);
        avatar.setFillColor(avatarColor(data.characterName));
        avatar.setOutlineColor(sf::Color::White);
        avatar.setOutlineThickness(1.f);
        window.draw(avatar);

        sf::Text letter{
            std::string(1, avatarLetter(data.characterName)), font, 8};
        centerText(letter, avatarX, 17.f);
        letter.setFillColor(sf::Color::White);
        window.draw(letter);

        std::ostringstream line;
        line << data.label << " " << data.characterName << " "
             << std::setw(6) << std::setfill('0') << data.score << "\n"
             << "x" << data.lives << " COIN " << data.coins << " "
             << data.formName;
        const char* state = lifeStateName(data.lifeState);
        if (state[0] != '\0') {
            line << " " << state;
        }

        sf::Text text{line.str(), font, 8};
        text.setLineSpacing(1.1f);
        text.setFillColor(sf::Color::White);
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setPosition(
            std::round(rightAligned ? 376.f - bounds.width : 24.f),
            6.f);
        window.draw(text);
    };

    drawPlayer(playerOne, false);
    drawPlayer(playerTwo, true);

    std::ostringstream center;
    center << "WORLD\n " << formatWorldNumber(worldName) << "\nTIME "
           << std::setw(3) << std::setfill(' ')
           << static_cast<int>(std::ceil(std::max(0.f, timeRemaining)));
    sf::Text world{center.str(), font, 8};
    world.setLineSpacing(0.95f);
    world.setFillColor(sf::Color{255, 235, 80});
    centerText(world, HudWidth * 0.5f, 17.f);
    window.draw(world);

    window.setView(previous);
}

void DuoHUD::renderPlayerMarker(
    sf::RenderWindow& window,
    const Character& character,
    const std::string& label) const {
    if (!fontLoaded || !character.isActive() || character.isDying()) {
        return;
    }
    const sf::Vector2f velocity = character.getVelocity();
    if (std::abs(velocity.x) < 8.f && std::abs(velocity.y) < 8.f) {
        return;
    }

    const sf::FloatRect bounds = character.getBounds();
    const float x = std::round(bounds.left + bounds.width * 0.5f);
    const float tipY = std::round(bounds.top - 2.f);

    sf::ConvexShape arrow;
    arrow.setPointCount(3);
    arrow.setPoint(0, {x - 4.f, tipY - 6.f});
    arrow.setPoint(1, {x + 4.f, tipY - 6.f});
    arrow.setPoint(2, {x, tipY});
    arrow.setFillColor(sf::Color::White);
    arrow.setOutlineColor(sf::Color::Black);
    arrow.setOutlineThickness(1.f);
    window.draw(arrow);

    const_cast<sf::Texture&>(font.getTexture(8)).setSmooth(false);
    sf::Text text{label, font, 8};
    text.setFillColor(sf::Color::White);
    text.setOutlineColor(sf::Color::Black);
    text.setOutlineThickness(1.f);
    centerText(text, x, tipY - 12.f);
    window.draw(text);
}
