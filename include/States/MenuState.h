#pragma once

#include "States/GameState.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class MenuState : public GameState {
public:
    enum class DisplayMode {
        TitleScreen,
        InMenu
    };

    enum class Page {
        GameMode,
        Solo,
        Play,
        Character,
        Settings,
        KeyBindings
    };

private:
    struct MenuEntry {
        std::string label;
        bool enabled{true};
    };

    struct CloudData {
        sf::Vector2f position;
        float speed{10.f};
        float scale{1.f};
        int textureIndex{1};
    };

    sf::Font fontRetro;
    sf::Font fontClean;
    bool fontRetroLoaded{false};
    bool fontCleanLoaded{false};

    // Text elements
    sf::Text promptText;
    sf::Text copyrightText;
    sf::Text versionText;
    sf::Text pageTitleText;
    sf::Text statusText;
    sf::Text footerText;
    sf::Text selectorText;
    sf::Text keyBindingsText;
    std::vector<sf::Text> entryTexts;

    // Visual assets
    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    bool bgLoaded{false};

    sf::Texture logoTexture;
    sf::Sprite logoSprite;
    bool logoLoaded{false};
    float baseLogoScale{1.f};

    sf::Texture charTexture;
    sf::Sprite charSprite;
    bool charLoaded{false};
    float baseCharScale{1.f};

    // Character Selection Cards
    sf::Texture marioCardTexture;
    sf::Sprite marioCardSprite;
    bool marioCardLoaded{false};
    float baseMarioCardScale{1.f};

    sf::Texture luigiCardTexture;
    sf::Sprite luigiCardSprite;
    bool luigiCardLoaded{false};
    float baseLuigiCardScale{1.f};

    float marioCurrentScale{1.f};
    float luigiCurrentScale{1.f};
    int characterCardSelection{0}; // 0 = Mario, 1 = Luigi

    sf::Text charChooseTitle;
    sf::Text charChoosePrompt;
    sf::Text marioCardBadge;
    sf::Text luigiCardBadge;
    sf::RectangleShape marioCardGlow;
    sf::RectangleShape luigiCardGlow;
    sf::ConvexShape arrowLeft;
    sf::ConvexShape arrowRight;

    // Background clouds
    std::vector<CloudData> clouds;
    sf::Sprite cloudSprite1;
    sf::Sprite cloudSprite2;
    sf::Sprite cloudSprite3;
    bool cloudsLoaded{false};

    // Menu Card styling
    sf::RectangleShape menuCard;
    sf::RectangleShape menuCardHeader;
    sf::RectangleShape selectionGlow;

    DisplayMode displayMode{DisplayMode::TitleScreen};
    Page page{Page::GameMode};
    std::vector<MenuEntry> entries;
    int selectedIndex{0};

    // Timers & Animations
    float globalTime{0.f};
    float blinkTimer{0.f};
    bool showSelector{true};

    bool isCharacterConfirming{false};
    float characterConfirmTimer{0.f};
    float characterFlashTimer{0.f};

    void setDisplayMode(DisplayMode newMode);
    void setPage(Page newPage);
    void rebuildEntries();
    void updateVisuals();
    void moveSelection(int direction);
    void activateSelection(sf::RenderWindow& window);
    void goBack();
    void adjustVolume(float delta);
    bool loadTextures();
    void loadFonts();
    void initClouds();
    void renderCharacterSelect(sf::RenderWindow& window);

public:
    MenuState() = default;

    void onEnter() override;
    void onExit() override;
    void handleInput(sf::Event& event, sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
