#include "AdminControl/AdminDebugView.h"

#include "Entities/Character.h"
#include "Entities/Enemies/Goomba.h"
#include "Entities/Enemies/GreenParatroopa.h"
#include "Entities/Enemies/Koopa.h"
#include "Entities/Enemies/PiranhaPlant.h"
#include "Entities/Enemies/RedKoopa.h"
#include "Entities/Enemies/RedParatroopa.h"
#include "Entities/Items/Coin.h"
#include "Entities/Items/FireFlower.h"
#include "Entities/Items/Mushroom.h"
#include "Entities/Items/OneUpMushroom.h"
#include "Entities/Items/StarItem.h"
#include "Level/Level.h"
#include "Level/Tile.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace {
constexpr unsigned int PanelFontSize = 6;
constexpr unsigned int AnnotationFontSize = 5;

std::string enemyTypeName(const Enemy& enemy) {
    if (dynamic_cast<const RedParatroopa*>(&enemy)) return "RedParatroopa";
    if (dynamic_cast<const GreenParatroopa*>(&enemy)) return "GreenParatroopa";
    if (dynamic_cast<const RedKoopa*>(&enemy)) return "RedKoopa";
    if (dynamic_cast<const Koopa*>(&enemy)) return "Koopa";
    if (dynamic_cast<const Goomba*>(&enemy)) return "Goomba";
    if (dynamic_cast<const PiranhaPlant*>(&enemy)) return "PiranhaPlant";
    return "Enemy";
}

std::string itemTypeName(const Item& item) {
    if (dynamic_cast<const OneUpMushroom*>(&item)) return "1UpMushroom";
    if (dynamic_cast<const Mushroom*>(&item)) return "Mushroom";
    if (dynamic_cast<const FireFlower*>(&item)) return "FireFlower";
    if (dynamic_cast<const StarItem*>(&item)) return "Star";
    if (dynamic_cast<const Coin*>(&item)) return "Coin";
    return "Item";
}

bool intersects(const sf::FloatRect& first, const sf::FloatRect& second) {
    return first.intersects(second);
}

void drawAnnotation(
    sf::RenderWindow& window,
    const sf::Font& font,
    const sf::FloatRect& bounds,
    const std::string& label,
    const sf::Color& color,
    const sf::FloatRect& viewBounds) {
    if (!intersects(bounds, viewBounds)) return;

    sf::RectangleShape outline({bounds.width, bounds.height});
    outline.setPosition(bounds.left, bounds.top);
    outline.setFillColor(sf::Color::Transparent);
    outline.setOutlineColor(color);
    outline.setOutlineThickness(1.f);
    window.draw(outline);

    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(AnnotationFontSize);
    text.setString(label);
    text.setFillColor(color);

    const float labelOffset = static_cast<float>(AnnotationFontSize) + 3.f;
    const float labelY = bounds.top - labelOffset >= viewBounds.top
        ? bounds.top - labelOffset
        : bounds.top + 1.f;
    text.setPosition(std::floor(bounds.left), std::floor(labelY));

    const sf::FloatRect textBounds = text.getGlobalBounds();
    sf::RectangleShape labelBackground({
        std::ceil(textBounds.width) + 2.f,
        std::ceil(textBounds.height) + 2.f
    });
    labelBackground.setPosition(
        std::floor(textBounds.left) - 1.f,
        std::floor(textBounds.top) - 1.f);
    labelBackground.setFillColor(sf::Color(0, 0, 0, 225));
    window.draw(labelBackground);
    window.draw(text);
}
}

AdminDebugView::AdminDebugView() {
    const std::string fontPaths[] = {
        "assets/fonts/RobotoFont.ttf",
        "../assets/fonts/RobotoFont.ttf",
        "../../assets/fonts/RobotoFont.ttf",
        "../../../assets/fonts/RobotoFont.ttf"
    };

    for (const auto& path : fontPaths) {
        if (font.loadFromFile(path)) {
            fontLoaded = true;
            break;
        }
    }

    if (!fontLoaded) {
        std::cerr << "[AdminDebugView] WARNING: Could not load font!"
                  << std::endl;
    } else {
        informationText.setFont(font);
        informationText.setCharacterSize(PanelFontSize);
        informationText.setFillColor(sf::Color(210, 255, 220));
        informationText.setPosition(7.f, 7.f);
    }

    panel.setPosition(4.f, 4.f);
    panel.setSize({158.f, 46.f});
    panel.setFillColor(sf::Color(0, 0, 0, 240));
    panel.setOutlineColor(sf::Color(255, 210, 70));
    panel.setOutlineThickness(1.f);
}

void AdminDebugView::toggle() {
    visible = !visible;
}

bool AdminDebugView::isVisible() const {
    return visible;
}

void AdminDebugView::renderWorldAnnotations(
    sf::RenderWindow& window,
    const Character& character,
    const Level& level) const {
    const sf::FloatRect viewBounds = level.getCamera().getViewBounds();

    for (const auto& enemy : level.getEnemies()) {
        if (enemy && enemy->isActive() && enemy->isActivated()) {
            drawAnnotation(
                window, font, enemy->getBounds(), enemyTypeName(*enemy),
                sf::Color(255, 90, 90), viewBounds);
        }
    }

    for (const auto& item : level.getItems()) {
        if (item && item->isActive()) {
            drawAnnotation(
                window, font, item->getBounds(), itemTypeName(*item),
                sf::Color(80, 230, 255), viewBounds);
        }
    }

    const TileMap& map = level.getTileMap();
    for (const TileHandle& handle : map.getTilesInBounds(viewBounds)) {
        const Tile* tile = map.getTile(handle);
        if (!tile || !tile->isQuestionBlock()) continue;

        const sf::FloatRect bounds = tile->getBounds();
        drawAnnotation(
            window, font, bounds,
            "? " + level.getBlockItemType(bounds.left, &character),
            sf::Color(255, 220, 60), viewBounds);
    }
}

void AdminDebugView::render(
    sf::RenderWindow& window,
    const Character& character,
    const Level& level) {
    if (!visible || !fontLoaded) {
        return;
    }

    const sf::Vector2f position = character.getPosition();
    const sf::Vector2f velocity = character.getVelocity();
    const float speed = std::sqrt(
        velocity.x * velocity.x + velocity.y * velocity.y);
    const EnemyRuntimeStats enemyStats = level.getEnemyRuntimeStats();

    std::ostringstream information;
    information << std::fixed << std::setprecision(1)
                << "ADMIN [T]\n"
                << "CHAR " << character.getCharacterType() << " | "
                << (character.isDying()
                        ? std::string_view{"Dying"}
                        : character.getCurrentFormName())
                << "\n"
                << "SPD " << speed << " | VEL "
                << velocity.x << "," << velocity.y << "\n"
                << "POS " << position.x << "," << position.y << "\n"
                << "ENEMY A " << enemyStats.active
                << " | I " << enemyStats.inactive
                << " | R " << enemyStats.removed;
    informationText.setString(information.str());

    const sf::View previousView = window.getView();
    renderWorldAnnotations(window, character, level);
    constexpr float AdminViewWidth = 400.f;
    const sf::Vector2f gameViewSize = previousView.getSize();
    const float adminViewHeight = gameViewSize.x > 0.f
        ? AdminViewWidth * gameViewSize.y / gameViewSize.x
        : 300.f;
    window.setView(sf::View(
        sf::FloatRect(0.f, 0.f, AdminViewWidth, adminViewHeight)));
    window.draw(panel);
    window.draw(informationText);
    window.setView(previousView);
}
