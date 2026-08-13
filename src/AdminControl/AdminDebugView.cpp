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
#include <vector>

namespace {
constexpr unsigned int PanelFontSize = 12;
constexpr unsigned int AnnotationFontSize = 10;

struct WorldAnnotation {
    sf::FloatRect bounds;
    std::string label;
    sf::Color color;
};

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

void drawWorldOutline(
    sf::RenderWindow& window,
    const sf::FloatRect& bounds,
    const sf::Color& color) {
    sf::RectangleShape outline({bounds.width, bounds.height});
    outline.setPosition(bounds.left, bounds.top);
    outline.setFillColor(sf::Color::Transparent);
    outline.setOutlineColor(color);
    outline.setOutlineThickness(1.f);
    window.draw(outline);
}

void drawScreenLabel(
    sf::RenderWindow& window,
    const sf::Font& font,
    const WorldAnnotation& annotation,
    const sf::View& worldView,
    const sf::Vector2u& windowSize) {
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(AnnotationFontSize);
    text.setString(annotation.label);
    text.setFillColor(annotation.color);

    const sf::Vector2i anchor = window.mapCoordsToPixel(
        {annotation.bounds.left, annotation.bounds.top}, worldView);
    const sf::FloatRect localBounds = text.getLocalBounds();
    float textX = static_cast<float>(anchor.x);
    float textY = static_cast<float>(anchor.y) -
                  std::ceil(localBounds.top + localBounds.height) - 4.f;

    if (textY < 2.f) {
        textY = static_cast<float>(anchor.y) + 2.f;
    }
    text.setPosition(std::round(textX), std::round(textY));

    sf::FloatRect textBounds = text.getGlobalBounds();
    if (textBounds.left + textBounds.width + 2.f > windowSize.x) {
        text.move(
            static_cast<float>(windowSize.x) -
                (textBounds.left + textBounds.width + 2.f),
            0.f);
        textBounds = text.getGlobalBounds();
    }
    if (textBounds.left < 2.f) {
        text.move(2.f - textBounds.left, 0.f);
        textBounds = text.getGlobalBounds();
    }

    sf::RectangleShape labelBackground({
        std::ceil(textBounds.width) + 4.f,
        std::ceil(textBounds.height) + 4.f
    });
    labelBackground.setPosition(
        std::floor(textBounds.left) - 2.f,
        std::floor(textBounds.top) - 2.f);
    labelBackground.setFillColor(sf::Color(0, 0, 0, 235));
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
        informationText.setPosition(14.f, 12.f);
    }

    panel.setPosition(8.f, 8.f);
    panel.setSize({290.f, 92.f});
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
    const sf::View worldView = window.getView();
    std::vector<WorldAnnotation> annotations;

    for (const auto& enemy : level.getEnemies()) {
        if (enemy && enemy->isActive() && enemy->isActivated()) {
            const sf::FloatRect bounds = enemy->getBounds();
            if (intersects(bounds, viewBounds)) {
                const sf::Color color(255, 90, 90);
                drawWorldOutline(window, bounds, color);
                annotations.push_back({bounds, enemyTypeName(*enemy), color});
            }
        }
    }

    for (const auto& item : level.getItems()) {
        if (item && item->isActive()) {
            const sf::FloatRect bounds = item->getBounds();
            if (intersects(bounds, viewBounds)) {
                const sf::Color color(80, 230, 255);
                drawWorldOutline(window, bounds, color);
                annotations.push_back({bounds, itemTypeName(*item), color});
            }
        }
    }

    const TileMap& map = level.getTileMap();
    for (const TileHandle& handle : map.getTilesInBounds(viewBounds)) {
        const Tile* tile = map.getTile(handle);
        if (!tile) continue;

        const sf::FloatRect bounds = tile->getBounds();
        if (!intersects(bounds, viewBounds)) continue;

        std::string label;
        sf::Color color;
        if (tile->isQuestionBlock()) {
            label = "? " + level.getBlockItemType(bounds.left, &character);
            color = sf::Color(255, 220, 60);
        } else if (tile->isBrick()) {
            const std::string itemType = level.getBrickItemType(bounds.left);
            if (itemType.empty()) continue;
            label = "BRICK " + itemType;
            color = sf::Color(255, 140, 220);
        } else {
            continue;
        }

        drawWorldOutline(window, bounds, color);
        annotations.push_back({bounds, label, color});
    }

    for (const WarpZoneInfo& warp : level.getWarpZones()) {
        if (!intersects(warp.bounds, viewBounds)) continue;
        const sf::Color color(190, 120, 255);
        drawWorldOutline(window, warp.bounds, color);
        annotations.push_back({warp.bounds, warp.label, color});
    }

    const sf::Vector2u windowSize = window.getSize();
    if (windowSize.x == 0 || windowSize.y == 0) {
        return;
    }

    const sf::View pixelView(sf::FloatRect(
        0.f, 0.f,
        static_cast<float>(windowSize.x),
        static_cast<float>(windowSize.y)));
    window.setView(pixelView);
    for (const auto& annotation : annotations) {
        drawScreenLabel(
            window, font, annotation, worldView, windowSize);
    }
    window.setView(worldView);
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
                << "ADMIN [T]  I:STAR  K:UP  L:HIT\n"
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
    const sf::Vector2u windowSize = window.getSize();
    if (windowSize.x == 0 || windowSize.y == 0) {
        return;
    }
    window.setView(sf::View(sf::FloatRect(
        0.f, 0.f,
        static_cast<float>(windowSize.x),
        static_cast<float>(windowSize.y))));
    window.draw(panel);
    window.draw(informationText);
    window.setView(previousView);
}
