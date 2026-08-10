#pragma once

#include "States/GameState.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class MenuState : public GameState {
private:
    enum class Page {
        GameMode,
        Solo,
        Play,
        Character,
        Settings,
        KeyBindings
    };

    struct MenuEntry {
        std::string label;
        bool enabled{true};
    };

    sf::Font font;
    bool fontLoaded{false};

    sf::Text titleText;
    sf::Text pageTitleText;
    sf::Text statusText;
    sf::Text footerText;
    sf::Text selectorText;
    sf::Text keyBindingsText;
    std::vector<sf::Text> entryTexts;

    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    bool bgLoaded{false};
    sf::RectangleShape groundBlock;
    sf::RectangleShape panel;

    Page page{Page::GameMode};
    std::vector<MenuEntry> entries;
    int selectedIndex{0};

    float blinkTimer{0.f};
    bool showSelector{true};

    void setPage(Page newPage);
    void rebuildEntries();
    void updateVisuals();
    void moveSelection(int direction);
    void activateSelection(sf::RenderWindow& window);
    void goBack();
    void adjustVolume(float delta);
    bool loadBackground();

public:
    MenuState() = default;

    void onEnter() override;
    void onExit() override;
    void handleInput(sf::Event& event, sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
