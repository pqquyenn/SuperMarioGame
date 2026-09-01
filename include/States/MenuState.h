#pragma once

#include "Core/GameSettings.h"
#include "Input/KeyBinding.h"
#include "PvP/PvPTypes.h"
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
        Duo,
        DuoPlay,
        DuoCharacter,
        PvP,
        PvPMap,
        PvPCharacter,
        Play,
        Character,
        Achievements,
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
    sf::Text achievementsText;
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
    int pvpSelectionStage{1};
    CharacterChoice pvpPlayerOneChoice{CharacterChoice::Mario};
    CharacterChoice pvpPlayerTwoChoice{CharacterChoice::Luigi};
    PvPMatchType pendingPvPMatchType{PvPMatchType::Small};
    std::string pendingPvPMapPath;
    int duoSelectionStage{1};
    CharacterChoice duoPlayerOneChoice{CharacterChoice::Mario};
    CharacterChoice duoPlayerTwoChoice{CharacterChoice::Luigi};
    std::string pendingDuoMapPath{"1.1/1-1.level"};

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
    Page entryPage{Page::GameMode};
    bool enterMenuDirectly{false};
    std::vector<MenuEntry> entries;
    int selectedIndex{0};
    BindingTarget bindingTarget{BindingTarget::Solo};
    bool bindingCaptureActive{false};
    std::string bindingStatusMessage;

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
    void cycleBindingTarget(int direction);
    void beginBindingCapture();
    void captureBindingKey(sf::Keyboard::Key key);
    bool isCharacterSelectionPage() const;
    void beginPvPCharacterSelection(
        PvPMatchType type,
        std::string mapPath
    );
    void launchPendingPvPMatch();
    void beginDuoCharacterSelection(std::string mapPath);
    void launchPendingDuoGame();
    bool loadTextures();
    void loadFonts();
    void initClouds();
    void renderCharacterSelect(sf::RenderWindow& window);

public:
    MenuState() = default;
    explicit MenuState(Page initialPage);

    void onEnter() override;
    void onExit() override;
    void handleInput(sf::Event& event, sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
