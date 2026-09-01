#include "States/MenuState.h"

#include "Core/AchievementSystem.h"
#include "Core/GameSettings.h"
#include "Core/SoundManager.h"
#include "Core/AssetManager.h"
#include "Input/KeyBindingService.h"
#include "States/GameStateManager.h"
#include "States/PlayState.h"
#include "States/PvPState.h"
#include "States/DuoState.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <memory>
#include <utility>

namespace {
constexpr float UiWidth = 800.f;
constexpr float UiHeight = 600.f;
constexpr float EntryStartY = 255.f;
constexpr float EntrySpacing = 42.f;

const sf::Color AccentGold{255, 225, 60};
const sf::Color TextWhite{255, 255, 255};
const sf::Color TextDisabled{140, 140, 140};

void centerText(sf::Text& text, float x, float y) {
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(std::round(bounds.left + bounds.width / 2.f),
                   std::round(bounds.top + bounds.height / 2.f));
    text.setPosition(x, y);
}

const char* characterName(CharacterChoice choice) {
    return choice == CharacterChoice::Luigi ? "LUIGI" : "MARIO";
}

const char* achievementStatus(const AchievementSystem& achievements,
                              AchievementId id) {
    return achievements.isUnlocked(id) ? "[UNLOCKED]" : "[LOCKED]";
}
}

MenuState::MenuState(Page initialPage)
    : entryPage{initialPage}, enterMenuDirectly{true} {}

void MenuState::onEnter() {
    std::cout << "[MenuState] Entered main menu (NSMBU Deluxe Style)" << std::endl;

    loadFonts();
    loadTextures();
    AssetManager::getInstance().loadSoundAssets();
    initClouds();

    const sf::Font& retroFont = fontRetroLoaded ? fontRetro : fontClean;
    const sf::Font& cleanFont = fontCleanLoaded ? fontClean : retroFont;

    // 1. "Press ENTER to start" prompt
    promptText.setFont(retroFont);
    promptText.setString("Press ENTER to start");
    promptText.setCharacterSize(15);
    promptText.setFillColor(AccentGold);
    promptText.setOutlineColor(sf::Color(30, 20, 10));
    promptText.setOutlineThickness(2.5f);
    centerText(promptText, UiWidth / 2.f, 345.f);

    // 2. Copyright and Version
    copyrightText.setFont(cleanFont);
    copyrightText.setString(L"\u00A9 2012-2018 Nintendo.");
    copyrightText.setCharacterSize(14);
    copyrightText.setFillColor(sf::Color(240, 240, 240, 230));
    copyrightText.setOutlineColor(sf::Color(10, 10, 10, 180));
    copyrightText.setOutlineThickness(1.5f);
    centerText(copyrightText, UiWidth / 2.f, 578.f);

    versionText.setFont(cleanFont);
    versionText.setString("Ver.1.0.2");
    versionText.setCharacterSize(14);
    versionText.setFillColor(sf::Color(240, 240, 240, 230));
    versionText.setOutlineColor(sf::Color(10, 10, 10, 180));
    versionText.setOutlineThickness(1.5f);
    centerText(versionText, 735.f, 578.f);

    // 3. Menu UI Elements
    pageTitleText.setFont(cleanFont);
    pageTitleText.setCharacterSize(16);
    pageTitleText.setFillColor(sf::Color(40, 25, 5));
    pageTitleText.setStyle(sf::Text::Bold);

    statusText.setFont(retroFont);
    statusText.setCharacterSize(10);
    statusText.setFillColor(AccentGold);
    statusText.setOutlineColor(sf::Color::Black);
    statusText.setOutlineThickness(1.5f);

    footerText.setFont(retroFont);
    footerText.setCharacterSize(8);
    footerText.setFillColor(sf::Color(220, 220, 220));
    footerText.setOutlineColor(sf::Color(10, 10, 10, 200));
    footerText.setOutlineThickness(1.5f);
    footerText.setString("ARROWS / WASD: MOVE     ENTER: SELECT     ESC: BACK");
    centerText(footerText, UiWidth / 2.f, 578.f);

    selectorText.setFont(retroFont);
    selectorText.setString(">");
    selectorText.setCharacterSize(16);
    selectorText.setFillColor(AccentGold);
    selectorText.setOutlineColor(sf::Color::Black);
    selectorText.setOutlineThickness(2.f);

    keyBindingsText.setFont(cleanFont);
    keyBindingsText.setCharacterSize(10);
    keyBindingsText.setFillColor(sf::Color::White);
    keyBindingsText.setLineSpacing(1.4f);
    keyBindingsText.setPosition(170.f, 225.f);

    achievementsText.setFont(cleanFont);
    achievementsText.setCharacterSize(14);
    achievementsText.setFillColor(sf::Color::White);
    achievementsText.setLineSpacing(1.35f);
    achievementsText.setPosition(190.f, 235.f);

    // 4. Menu Card Container
    menuCard.setSize({520.f, 320.f});
    menuCard.setPosition(140.f, 185.f);
    menuCard.setFillColor(sf::Color(15, 20, 35, 220));
    menuCard.setOutlineColor(sf::Color(255, 215, 60, 200));
    menuCard.setOutlineThickness(2.5f);

    menuCardHeader.setSize({520.f, 36.f});
    menuCardHeader.setPosition(140.f, 185.f);
    menuCardHeader.setFillColor(sf::Color(255, 200, 40, 240));

    selectionGlow.setSize({460.f, 34.f});
    selectionGlow.setFillColor(sf::Color(255, 215, 60, 50));
    selectionGlow.setOutlineColor(sf::Color(255, 230, 80, 200));
    selectionGlow.setOutlineThickness(1.5f);

    // 5. Character Select Specific UI
    charChooseTitle.setFont(cleanFont);
    charChooseTitle.setString("Choose a character!");
    charChooseTitle.setCharacterSize(26);
    charChooseTitle.setFillColor(sf::Color::White);
    charChooseTitle.setStyle(sf::Text::Bold);
    charChooseTitle.setOutlineColor(sf::Color(20, 20, 20, 220));
    charChooseTitle.setOutlineThickness(3.f);
    centerText(charChooseTitle, UiWidth / 2.f, 85.f);

    charChoosePrompt.setFont(retroFont);
    charChoosePrompt.setCharacterSize(9);
    charChoosePrompt.setFillColor(AccentGold);
    charChoosePrompt.setOutlineColor(sf::Color::Black);
    charChoosePrompt.setOutlineThickness(1.5f);

    marioCardBadge.setFont(retroFont);
    marioCardBadge.setCharacterSize(10);
    marioCardBadge.setOutlineThickness(1.5f);

    luigiCardBadge.setFont(retroFont);
    luigiCardBadge.setCharacterSize(10);
    luigiCardBadge.setOutlineThickness(1.5f);

    marioCardGlow.setSize({230.f, 305.f});
    marioCardGlow.setOrigin(115.f, 152.5f);
    marioCardGlow.setFillColor(sf::Color(255, 50, 50, 45));
    marioCardGlow.setOutlineColor(sf::Color(255, 220, 60, 230));
    marioCardGlow.setOutlineThickness(3.5f);

    luigiCardGlow.setSize({230.f, 305.f});
    luigiCardGlow.setOrigin(115.f, 152.5f);
    luigiCardGlow.setFillColor(sf::Color(50, 220, 80, 45));
    luigiCardGlow.setOutlineColor(sf::Color(255, 220, 60, 230));
    luigiCardGlow.setOutlineThickness(3.5f);

    arrowLeft.setPointCount(3);
    arrowLeft.setPoint(0, sf::Vector2f(0.f, 15.f));
    arrowLeft.setPoint(1, sf::Vector2f(22.f, 0.f));
    arrowLeft.setPoint(2, sf::Vector2f(22.f, 30.f));
    arrowLeft.setOrigin(11.f, 15.f);
    arrowLeft.setFillColor(sf::Color(255, 215, 30));
    arrowLeft.setOutlineColor(sf::Color(20, 20, 20));
    arrowLeft.setOutlineThickness(2.f);

    arrowRight.setPointCount(3);
    arrowRight.setPoint(0, sf::Vector2f(22.f, 15.f));
    arrowRight.setPoint(1, sf::Vector2f(0.f, 0.f));
    arrowRight.setPoint(2, sf::Vector2f(0.f, 30.f));
    arrowRight.setOrigin(11.f, 15.f);
    arrowRight.setFillColor(sf::Color(255, 215, 30));
    arrowRight.setOutlineColor(sf::Color(20, 20, 20));
    arrowRight.setOutlineThickness(2.f);

    setDisplayMode(enterMenuDirectly
                       ? DisplayMode::InMenu
                       : DisplayMode::TitleScreen);
    setPage(entryPage);
}

void MenuState::onExit() {
    std::cout << "[MenuState] Leaving main menu" << std::endl;
}

void MenuState::loadFonts() {
    const std::string retroPaths[] = {
        "assets/fonts/press-start-2p.ttf",
        "../assets/fonts/press-start-2p.ttf",
        "../../assets/fonts/press-start-2p.ttf"
    };
    for (const auto& path : retroPaths) {
        if (std::filesystem::exists(path) && fontRetro.loadFromFile(path)) {
            fontRetroLoaded = true;
            break;
        }
    }

    const std::string cleanPaths[] = {
        "assets/fonts/RobotoFont.ttf",
        "../assets/fonts/RobotoFont.ttf",
        "../../assets/fonts/RobotoFont.ttf"
    };
    for (const auto& path : cleanPaths) {
        if (std::filesystem::exists(path) && fontClean.loadFromFile(path)) {
            fontCleanLoaded = true;
            break;
        }
    }
}

bool MenuState::loadTextures() {
    const std::string basePrefixes[] = {"", "../", "../../"};

    for (const auto& p : basePrefixes) {
        std::string path = p + "assets/state/MenuGameBackGround.jpg";
        if (std::filesystem::exists(path) && bgTexture.loadFromFile(path)) {
            bgLoaded = true;
            bgTexture.setSmooth(true);
            bgSprite.setTexture(bgTexture);
            const sf::Vector2u sz = bgTexture.getSize();
            if (sz.x > 0 && sz.y > 0) {
                bgSprite.setScale(UiWidth / static_cast<float>(sz.x),
                                  UiHeight / static_cast<float>(sz.y));
            }
            break;
        }
    }

    auto loadKeyedTexture = [](const std::string& path, sf::Texture& tex, sf::Sprite& spr, float& baseScale, float targetW) -> bool {
        sf::Image img;
        if (!std::filesystem::exists(path) || !img.loadFromFile(path)) return false;
        const unsigned int w = img.getSize().x;
        const unsigned int h = img.getSize().y;
        for (unsigned int y = 0; y < h; ++y) {
            for (unsigned int x = 0; x < w; ++x) {
                sf::Color c = img.getPixel(x, y);
                int brightness = c.r + c.g + c.b;
                if (brightness < 30) {
                    img.setPixel(x, y, sf::Color(0, 0, 0, 0));
                } else if (brightness < 90) {
                    sf::Uint8 alpha = static_cast<sf::Uint8>((brightness - 30) * 255 / 60);
                    img.setPixel(x, y, sf::Color(c.r, c.g, c.b, alpha));
                }
            }
        }
        tex.loadFromImage(img);
        tex.setSmooth(true);
        spr.setTexture(tex);
        spr.setOrigin(w * 0.5f, h * 0.5f);
        baseScale = targetW / static_cast<float>(w);
        spr.setScale(baseScale, baseScale);
        return true;
    };

    for (const auto& p : basePrefixes) {
        std::string path = p + "assets/state/MenuLogo.jpg";
        if (loadKeyedTexture(path, logoTexture, logoSprite, baseLogoScale, 460.f)) {
            logoLoaded = true;
            break;
        }
    }

    for (const auto& p : basePrefixes) {
        std::string path = p + "assets/state/MenuCharacters.jpg";
        if (loadKeyedTexture(path, charTexture, charSprite, baseCharScale, 540.f)) {
            charLoaded = true;
            break;
        }
    }

    for (const auto& p : basePrefixes) {
        std::string path = p + "assets/state/MarioCard.jpg";
        if (std::filesystem::exists(path) && marioCardTexture.loadFromFile(path)) {
            marioCardTexture.setSmooth(true);
            marioCardLoaded = true;
            marioCardSprite.setTexture(marioCardTexture);
            const sf::Vector2u sz = marioCardTexture.getSize();
            marioCardSprite.setOrigin(sz.x * 0.5f, sz.y * 0.5f);
            baseMarioCardScale = 210.f / static_cast<float>(sz.x);
            marioCardSprite.setScale(baseMarioCardScale, baseMarioCardScale);
            break;
        }
    }

    for (const auto& p : basePrefixes) {
        std::string path = p + "assets/state/LuigiCard.jpg";
        if (std::filesystem::exists(path) && luigiCardTexture.loadFromFile(path)) {
            luigiCardTexture.setSmooth(true);
            luigiCardLoaded = true;
            luigiCardSprite.setTexture(luigiCardTexture);
            const sf::Vector2u sz = luigiCardTexture.getSize();
            luigiCardSprite.setOrigin(sz.x * 0.5f, sz.y * 0.5f);
            baseLuigiCardScale = 210.f / static_cast<float>(sz.x);
            luigiCardSprite.setScale(baseLuigiCardScale, baseLuigiCardScale);
            break;
        }
    }

    AssetManager& assets = AssetManager::getInstance();
    assets.loadLevelAssets();
    const sf::Texture& c1 = assets.getTexture("Cloud1");
    const sf::Texture& c2 = assets.getTexture("Cloud2");
    const sf::Texture& c3 = assets.getTexture("Cloud3");
    if (c1.getSize().x > 0) {
        cloudSprite1.setTexture(c1);
        cloudSprite2.setTexture(c2);
        cloudSprite3.setTexture(c3);
        cloudsLoaded = true;
    }

    return bgLoaded;
}

void MenuState::initClouds() {
    clouds.clear();
    clouds.push_back({{80.f, 40.f}, 12.f, 1.3f, 1});
    clouds.push_back({{380.f, 65.f}, 8.f, 1.1f, 2});
    clouds.push_back({{650.f, 35.f}, 15.f, 1.4f, 3});
    clouds.push_back({{-120.f, 80.f}, 10.f, 1.2f, 1});
}

void MenuState::setDisplayMode(DisplayMode newMode) {
    displayMode = newMode;
    blinkTimer = 0.f;
    showSelector = true;
    updateVisuals();
}

bool MenuState::isCharacterSelectionPage() const {
    return page == Page::Character || page == Page::PvPCharacter ||
           page == Page::DuoCharacter;
}

void MenuState::beginPvPCharacterSelection(
    PvPMatchType type,
    std::string mapPath
) {
    pendingPvPMatchType = type;
    pendingPvPMapPath = std::move(mapPath);
    pvpSelectionStage = 1;
    setPage(Page::PvPCharacter);
}

void MenuState::launchPendingPvPMatch() {
    if (!stateManager) {
        return;
    }

    stateManager->clearAndPushState(std::make_unique<PvPState>(
        pendingPvPMatchType,
        pendingPvPMapPath,
        pvpPlayerOneChoice,
        pvpPlayerTwoChoice));
}

void MenuState::beginDuoCharacterSelection(std::string mapPath) {
    pendingDuoMapPath = std::move(mapPath);
    duoSelectionStage = 1;
    setPage(Page::DuoCharacter);
}

void MenuState::launchPendingDuoGame() {
    if (!stateManager) {
        return;
    }
    DuoSessionConfig config;
    config.mapPath = pendingDuoMapPath;
    config.playerOneChoice = duoPlayerOneChoice;
    config.playerTwoChoice = duoPlayerTwoChoice;
    stateManager->clearAndPushState(
        std::make_unique<DuoState>(std::move(config)));
}

void MenuState::setPage(Page newPage) {
    if (newPage != Page::KeyBindings) {
        bindingCaptureActive = false;
        bindingStatusMessage.clear();
    }
    page = newPage;
    selectedIndex = 0;
    showSelector = true;
    blinkTimer = 0.f;
    isCharacterConfirming = false;
    characterConfirmTimer = 0.f;
    characterFlashTimer = 0.f;

    if (isCharacterSelectionPage()) {
        CharacterChoice choice =
            GameSettings::getInstance().getCharacterChoice();
        if (page == Page::PvPCharacter) {
            choice = pvpSelectionStage == 1
                ? pvpPlayerOneChoice
                : pvpPlayerTwoChoice;
        } else if (page == Page::DuoCharacter) {
            choice = duoSelectionStage == 1
                ? duoPlayerOneChoice
                : duoPlayerTwoChoice;
        }
        characterCardSelection = choice == CharacterChoice::Luigi ? 1 : 0;
        marioCurrentScale = (characterCardSelection == 0) ? (baseMarioCardScale * 1.14f) : (baseMarioCardScale * 0.88f);
        luigiCurrentScale = (characterCardSelection == 1) ? (baseLuigiCardScale * 1.14f) : (baseLuigiCardScale * 0.88f);
    }

    rebuildEntries();
    if (!entries.empty() && !entries[selectedIndex].enabled) {
        moveSelection(1);
    }
    updateVisuals();
}

void MenuState::rebuildEntries() {
    entries.clear();
    switch (page) {
        case Page::GameMode:
            pageTitleText.setString("GAME MODE");
            entries = {{"SOLO"}, {"DUO"}, {"PVP"}, {"SETTINGS"}, {"BACK TO TITLE"}};
            break;
        case Page::Solo:
            pageTitleText.setString("SOLO");
            entries = {{"PLAY"}, {"CHARACTER"}, {"ACHIEVEMENTS"}, {"BACK"}};
            break;
        case Page::Duo:
            pageTitleText.setString("DUO CO-OP");
            entries = {{"PLAY"}, {"BACK"}};
            break;
        case Page::DuoPlay:
            pageTitleText.setString("DUO - SELECT WORLD");
            entries = {{"WORLD 1-1"}, {"WORLD 1-2"}, {"WORLD 1-3"}, {"WORLD 1-4"}, {"BACK"}};
            break;
        case Page::DuoCharacter:
            break;
        case Page::PvP:
            pageTitleText.setString("PVP MATCH-UP");
            entries = {{"SMALL MATCH"},
                       {"SUPER MATCH"},
                       {"FRIENDLY MATCH"},
                       {"BACK"}};
            break;
        case Page::PvPMap:
            pageTitleText.setString("SELECT PVP MAP");
            if (pendingPvPMatchType == PvPMatchType::Small) {
                entries = {{"SMALL ARENA"}, {"BACK"}};
            } else if (pendingPvPMatchType == PvPMatchType::Super) {
                entries = {{"SUPER ARENA 1"},
                           {"SUPER ARENA 2"},
                           {"BACK"}};
            } else {
                entries = {{"FRIENDLY ARENA"}, {"BACK"}};
            }
            break;
        case Page::PvPCharacter:
            break;
        case Page::Play:
            pageTitleText.setString("SELECT WORLD");
            entries = {{"WORLD 1-1"}, {"WORLD 1-2"}, {"WORLD 1-3"}, {"WORLD 1-4"}, {"BACK"}};
            break;
        case Page::Character:
            break;
        case Page::Achievements: {
            pageTitleText.setString("ACHIEVEMENTS");
            const AchievementSystem& achievements =
                AchievementSystem::getInstance();
            achievementsText.setString(
                "HIGHEST SCORE       " +
                std::to_string(achievements.getHighestScore()) + "\n\n" +
                "CLEAR WORLD 1-1     " +
                achievementStatus(achievements, AchievementId::ClearWorld11) + "\n" +
                "CLEAR WORLD 1-2     " +
                achievementStatus(achievements, AchievementId::ClearWorld12) + "\n" +
                "CLEAR WORLD 1-3     " +
                achievementStatus(achievements, AchievementId::ClearWorld13) + "\n" +
                "SMALL IS ENOUGH     " +
                achievementStatus(achievements, AchievementId::SmallIsEnough) + "\n" +
                "HARDCORE            " +
                achievementStatus(achievements, AchievementId::Hardcore) + "\n" +
                "FRIENDLY            " +
                achievementStatus(achievements, AchievementId::Friendly) + "\n" +
                "I HATE MYSTERY      " +
                achievementStatus(achievements, AchievementId::IHateMystery));
            entries = {{"BACK"}};
            break;
        }
        case Page::Settings: {
            pageTitleText.setString("SETTINGS");
            const int volume = static_cast<int>(std::round(SoundManager::getInstance().getMasterVolume()));
            entries = {{"KEY BINDINGS"}, {"MASTER VOLUME  < " + std::to_string(volume) + "% >"}, {"BACK"}};
            break;
        }
        case Page::KeyBindings: {
            pageTitleText.setString("KEY BINDINGS");
            const auto& bindings = KeyBindingService::getInstance();
            keyBindingsText.setString(
                std::string{"PROFILE:  < "} + bindingTargetName(bindingTarget) +
                " >\nLEFT / RIGHT: CHANGE PROFILE");
            for (InputAction action : allInputActions()) {
                entries.push_back({
                    inputActionName(action) + std::string{"    < "} +
                    keyDisplayName(bindings.getKey(bindingTarget, action)) +
                    " >"});
            }
            entries.push_back({"RESET PROFILE DEFAULT"});
            entries.push_back({"RESET ALL DEFAULTS"});
            entries.push_back({"BACK"});
            break;
        }
    }

    const sf::Font& retroFont = fontRetroLoaded ? fontRetro : fontClean;
    entryTexts.clear();
    entryTexts.reserve(entries.size());
    for (const auto& entry : entries) {
        sf::Text text;
        text.setFont(retroFont);
        text.setString(entry.label);
        text.setCharacterSize(entry.label.size() > 24 ? 10 : 13);
        text.setFillColor(entry.enabled ? TextWhite : TextDisabled);
        text.setOutlineColor(sf::Color(10, 10, 10, 200));
        text.setOutlineThickness(1.5f);
        entryTexts.push_back(text);
    }
}

void MenuState::moveSelection(int direction) {
    if (isCharacterSelectionPage()) {
        characterCardSelection = (characterCardSelection == 0) ? 1 : 0;
        return;
    }
    if (entries.empty()) return;
    const int count = static_cast<int>(entries.size());
    for (int attempt = 0; attempt < count; ++attempt) {
        selectedIndex = (selectedIndex + direction + count) % count;
        if (entries[selectedIndex].enabled) break;
    }
    showSelector = true;
    blinkTimer = 0.f;
    updateVisuals();
}

void MenuState::updateVisuals() {
    if (isCharacterSelectionPage()) {
        if (page == Page::PvPCharacter || page == Page::DuoCharacter) {
            const int selectionStage = page == Page::PvPCharacter
                ? pvpSelectionStage
                : duoSelectionStage;
            const CharacterChoice firstChoice = page == Page::PvPCharacter
                ? pvpPlayerOneChoice
                : duoPlayerOneChoice;
            const std::string player = selectionStage == 1
                ? "PLAYER 1" : "PLAYER 2";
            charChooseTitle.setString(player + ": Choose a character!");
            centerText(charChooseTitle, UiWidth / 2.f, 85.f);

            const bool marioSelected = characterCardSelection == 0;
            marioCardBadge.setString(marioSelected
                ? "[" + player + ": SELECT]" : "MARIO");
            luigiCardBadge.setString(!marioSelected
                ? "[" + player + ": SELECT]" : "LUIGI");
            marioCardBadge.setFillColor(
                marioSelected ? AccentGold : sf::Color{230, 230, 230});
            luigiCardBadge.setFillColor(
                !marioSelected ? AccentGold : sf::Color{230, 230, 230});
            marioCardBadge.setOutlineColor(sf::Color{20, 20, 20});
            luigiCardBadge.setOutlineColor(sf::Color{20, 20, 20});
            const std::string priorChoice = selectionStage == 2
                ? std::string{"P1: "} + characterName(firstChoice) +
                      "     "
                : std::string{};
            charChoosePrompt.setString(
                priorChoice +
                "ARROWS / WASD: CHOOSE     ENTER: CONFIRM     ESC: BACK");
            centerText(charChoosePrompt, UiWidth / 2.f, 565.f);
            return;
        }

        charChooseTitle.setString("Choose a character!");
        centerText(charChooseTitle, UiWidth / 2.f, 85.f);
        const CharacterChoice activeChoice = GameSettings::getInstance().getCharacterChoice();
        if (activeChoice == CharacterChoice::Mario) {
            marioCardBadge.setString("[ACTIVE]");
            marioCardBadge.setFillColor(sf::Color(255, 220, 50));
            marioCardBadge.setOutlineColor(sf::Color(30, 20, 10));
        } else {
            marioCardBadge.setString("ENTER: SELECT");
            marioCardBadge.setFillColor(sf::Color(230, 230, 230));
            marioCardBadge.setOutlineColor(sf::Color(20, 20, 20));
        }
        if (activeChoice == CharacterChoice::Luigi) {
            luigiCardBadge.setString("[ACTIVE]");
            luigiCardBadge.setFillColor(sf::Color(255, 220, 50));
            luigiCardBadge.setOutlineColor(sf::Color(30, 20, 10));
        } else {
            luigiCardBadge.setString("ENTER: SELECT");
            luigiCardBadge.setFillColor(sf::Color(230, 230, 230));
            luigiCardBadge.setOutlineColor(sf::Color(20, 20, 20));
        }
        charChoosePrompt.setString("LEFT / RIGHT / WASD: CHOOSE     ENTER: CONFIRM     ESC: BACK");
        centerText(charChoosePrompt, UiWidth / 2.f, 565.f);
        return;
    }

    centerText(pageTitleText, UiWidth / 2.f, 203.f);
    for (std::size_t i = 0; i < entryTexts.size(); ++i) {
        const float firstEntryY = page == Page::KeyBindings ? 272.f :
            page == Page::Achievements ? 490.f : EntryStartY;
        const float spacing = page == Page::KeyBindings ? 23.f : EntrySpacing;
        const float y = firstEntryY + static_cast<float>(i) * spacing;
        centerText(entryTexts[i], UiWidth / 2.f, y);
        if (!entries[i].enabled) {
            entryTexts[i].setFillColor(TextDisabled);
        } else {
            entryTexts[i].setFillColor(static_cast<int>(i) == selectedIndex ? AccentGold : TextWhite);
        }
    }
    if (!entryTexts.empty() && selectedIndex >= 0 && selectedIndex < static_cast<int>(entryTexts.size())) {
        const sf::FloatRect bounds = entryTexts[selectedIndex].getGlobalBounds();
        selectionGlow.setSize({460.f, page == Page::KeyBindings ? 20.f : 34.f});
        selectorText.setPosition(bounds.left - 24.f, bounds.top + bounds.height / 2.f - 10.f);
        const float glowOffset = page == Page::KeyBindings ? 9.f : 16.f;
        selectionGlow.setPosition(UiWidth / 2.f - 230.f,
                                  bounds.top + bounds.height / 2.f - glowOffset);
    }
    if (page == Page::Settings && selectedIndex == 1) {
        statusText.setString("LEFT / RIGHT: ADJUST VOLUME");
    } else if (page == Page::KeyBindings) {
        statusText.setString(bindingStatusMessage.empty()
            ? "ENTER: REBIND     ESC: BACK"
            : bindingStatusMessage);
    } else {
        statusText.setString("");
    }
    centerText(statusText, UiWidth / 2.f,
               page == Page::KeyBindings ? 515.f : 485.f);
}

void MenuState::activateSelection(sf::RenderWindow& window) {
    if (isCharacterSelectionPage()) {
        if (isCharacterConfirming) return;
        CharacterChoice choice = (characterCardSelection == 0) ? CharacterChoice::Mario : CharacterChoice::Luigi;
        if (page == Page::Character) {
            GameSettings::getInstance().setCharacterChoice(choice);
        } else if (page == Page::PvPCharacter) {
            if (pvpSelectionStage == 1) {
                pvpPlayerOneChoice = choice;
            } else {
                pvpPlayerTwoChoice = choice;
            }
        } else if (duoSelectionStage == 1) {
            duoPlayerOneChoice = choice;
        } else {
            duoPlayerTwoChoice = choice;
        }
        SoundManager::getInstance().playSound("powerupcollect");
        isCharacterConfirming = true;
        characterConfirmTimer = 0.f;
        characterFlashTimer = 0.f;
        updateVisuals();
        return;
    }
    if (entries.empty() || !entries[selectedIndex].enabled) return;
    switch (page) {
        case Page::GameMode:
            if (selectedIndex == 0) setPage(Page::Solo);
            else if (selectedIndex == 1) setPage(Page::Duo);
            else if (selectedIndex == 2) setPage(Page::PvP);
            else if (selectedIndex == 3) setPage(Page::Settings);
            else if (selectedIndex == 4) setDisplayMode(DisplayMode::TitleScreen);
            break;
        case Page::Duo:
            if (selectedIndex == 0) setPage(Page::DuoPlay);
            else setPage(Page::GameMode);
            break;
        case Page::DuoPlay: {
            if (selectedIndex == 4) {
                setPage(Page::Duo);
                break;
            }
            static const char* maps[] = {
                "1.1/1-1.level",
                "1.2/1-2.level",
                "1.3/1-3.level",
                "1.4/1-4.level"};
            beginDuoCharacterSelection(maps[selectedIndex]);
            break;
        }
        case Page::DuoCharacter:
            break;
        case Page::PvP:
            if (selectedIndex == 3) {
                setPage(Page::GameMode);
            } else {
                static const PvPMatchType matchTypes[] = {
                    PvPMatchType::Small,
                    PvPMatchType::Super,
                    PvPMatchType::Friendly
                };
                pendingPvPMatchType = matchTypes[selectedIndex];
                setPage(Page::PvPMap);
            }
            break;
        case Page::PvPMap: {
            const int backIndex = pendingPvPMatchType == PvPMatchType::Super
                ? 2 : 1;
            if (selectedIndex == backIndex) {
                setPage(Page::PvP);
            } else {
                std::string mapPath;
                if (pendingPvPMatchType == PvPMatchType::Small) {
                    mapPath = "pvp/small-arena.level";
                } else if (pendingPvPMatchType == PvPMatchType::Super) {
                    mapPath = selectedIndex == 0
                        ? "pvp/super-arena.level"
                        : "pvp/super-arena1.level";
                } else {
                    mapPath = "pvp/friendly-arena.level";
                }
                beginPvPCharacterSelection(
                    pendingPvPMatchType, std::move(mapPath));
            }
            break;
        }
        case Page::Solo:
            if (selectedIndex == 0) setPage(Page::Play);
            else if (selectedIndex == 1) setPage(Page::Character);
            else if (selectedIndex == 2) setPage(Page::Achievements);
            else if (selectedIndex == 3) setPage(Page::GameMode);
            break;
        case Page::Play: {
            if (selectedIndex == 4) { setPage(Page::Solo); break; }
            static const char* maps[] = {"1.1/1-1.level", "1.2/1-2.level", "1.3/1-3.level", "1.4/1-4.level"};
            if (stateManager) {
                stateManager->clearAndPushState(std::make_unique<PlayState>(maps[selectedIndex]));
            }
            break;
        }
        case Page::Character: break;
        case Page::PvPCharacter: break;
        case Page::Achievements:
            setPage(Page::Solo);
            break;
        case Page::Settings:
            if (selectedIndex == 0) setPage(Page::KeyBindings);
            else if (selectedIndex == 1) adjustVolume(10.f);
            else setPage(Page::GameMode);
            break;
        case Page::KeyBindings:
            if (selectedIndex < static_cast<int>(InputActionCount)) {
                beginBindingCapture();
            } else if (selectedIndex == static_cast<int>(InputActionCount)) {
                const BindingUpdateResult result =
                    KeyBindingService::getInstance().resetProfile(bindingTarget);
                if (result.status == BindingUpdateStatus::Applied) {
                    bindingStatusMessage = "PROFILE RESTORED TO DEFAULT";
                    rebuildEntries();
                } else {
                    bindingStatusMessage = result.issues.empty()
                        ? "COULD NOT RESET PROFILE"
                        : result.issues.front().message;
                }
                updateVisuals();
            } else if (selectedIndex == static_cast<int>(InputActionCount) + 1) {
                const BindingUpdateResult result =
                    KeyBindingService::getInstance().resetAll();
                if (result.status == BindingUpdateStatus::Applied) {
                    bindingStatusMessage = "ALL PROFILES RESTORED TO DEFAULT";
                    rebuildEntries();
                } else {
                    bindingStatusMessage = result.issues.empty()
                        ? "COULD NOT RESET BINDINGS"
                        : result.issues.front().message;
                }
                updateVisuals();
            } else {
                setPage(Page::Settings);
            }
            break;
    }
}

void MenuState::adjustVolume(float delta) {
    SoundManager& sound = SoundManager::getInstance();
    sound.setMasterVolume(sound.getMasterVolume() + delta);
    rebuildEntries();
    selectedIndex = 1;
    updateVisuals();
}

void MenuState::cycleBindingTarget(int direction) {
    const int count = static_cast<int>(BindingTargetCount);
    const int current = static_cast<int>(bindingTarget);
    bindingTarget = static_cast<BindingTarget>(
        (current + direction + count) % count);
    bindingStatusMessage.clear();
    const int previousSelection = selectedIndex;
    rebuildEntries();
    selectedIndex = std::min(previousSelection,
                             static_cast<int>(entries.size()) - 1);
    updateVisuals();
}

void MenuState::beginBindingCapture() {
    if (selectedIndex < 0 ||
        selectedIndex >= static_cast<int>(InputActionCount)) {
        return;
    }
    bindingCaptureActive = true;
    const InputAction action = allInputActions()[selectedIndex];
    bindingStatusMessage =
        std::string{"PRESS A KEY FOR "} + inputActionName(action) +
        " (ESC: CANCEL)";
    updateVisuals();
}

void MenuState::captureBindingKey(sf::Keyboard::Key key) {
    const InputAction action = allInputActions()[selectedIndex];
    const BindingUpdateResult result = KeyBindingService::getInstance().tryUpdate(
        {bindingTarget, action, key});
    if (result.status == BindingUpdateStatus::Applied ||
        result.status == BindingUpdateStatus::Unchanged) {
        bindingCaptureActive = false;
        bindingStatusMessage = result.status == BindingUpdateStatus::Applied
            ? std::string{inputActionName(action)} + " SET TO " +
                  keyDisplayName(key)
            : "KEY IS ALREADY ASSIGNED";
        rebuildEntries();
    } else if (!result.issues.empty()) {
        bindingStatusMessage = result.issues.front().message +
            " (ESC: CANCEL)";
    } else {
        bindingStatusMessage = "COULD NOT SAVE KEY BINDING (ESC: CANCEL)";
    }
    updateVisuals();
}

void MenuState::goBack() {
    switch (page) {
        case Page::GameMode: setDisplayMode(DisplayMode::TitleScreen); break;
        case Page::Solo: setPage(Page::GameMode); break;
        case Page::Duo: setPage(Page::GameMode); break;
        case Page::DuoPlay: setPage(Page::Duo); break;
        case Page::DuoCharacter:
            if (duoSelectionStage == 2) {
                duoSelectionStage = 1;
                setPage(Page::DuoCharacter);
            } else {
                setPage(Page::DuoPlay);
            }
            break;
        case Page::PvP: setPage(Page::GameMode); break;
        case Page::PvPMap: setPage(Page::PvP); break;
        case Page::PvPCharacter:
            if (pvpSelectionStage == 2) {
                pvpSelectionStage = 1;
                setPage(Page::PvPCharacter);
            } else {
                setPage(Page::PvP);
            }
            break;
        case Page::Play:
        case Page::Character: setPage(Page::Solo); break;
        case Page::Achievements: setPage(Page::Solo); break;
        case Page::Settings: setPage(Page::GameMode); break;
        case Page::KeyBindings: setPage(Page::Settings); break;
    }
}

void MenuState::handleInput(sf::Event& event, sf::RenderWindow& window) {
    if (displayMode == DisplayMode::TitleScreen) {
        if (event.type == sf::Event::KeyPressed &&
            (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return)) {
            SoundManager::getInstance().playSound("pause");
            setDisplayMode(DisplayMode::InMenu);
            setPage(Page::GameMode);
        }
        return;
    }
    if (event.type != sf::Event::KeyPressed) return;

    if (page == Page::KeyBindings) {
        if (bindingCaptureActive) {
            if (event.key.code == sf::Keyboard::Escape) {
                bindingCaptureActive = false;
                bindingStatusMessage = "REBIND CANCELLED";
                updateVisuals();
            } else {
                captureBindingKey(event.key.code);
            }
            return;
        }

        switch (event.key.code) {
            case sf::Keyboard::Up: case sf::Keyboard::W:
                SoundManager::getInstance().playSound("stomp");
                moveSelection(-1);
                break;
            case sf::Keyboard::Down: case sf::Keyboard::S:
                SoundManager::getInstance().playSound("stomp");
                moveSelection(1);
                break;
            case sf::Keyboard::Left: case sf::Keyboard::A:
                SoundManager::getInstance().playSound("stomp");
                cycleBindingTarget(-1);
                break;
            case sf::Keyboard::Right: case sf::Keyboard::D:
                SoundManager::getInstance().playSound("stomp");
                cycleBindingTarget(1);
                break;
            case sf::Keyboard::Enter: case sf::Keyboard::Space:
                SoundManager::getInstance().playSound("coin");
                activateSelection(window);
                break;
            case sf::Keyboard::Escape:
                SoundManager::getInstance().playSound("pipe");
                goBack();
                break;
            default:
                break;
        }
        return;
    }

    if (isCharacterSelectionPage()) {
        if (isCharacterConfirming) return;
        switch (event.key.code) {
            case sf::Keyboard::Left: case sf::Keyboard::A:
            case sf::Keyboard::Right: case sf::Keyboard::D:
            case sf::Keyboard::Up: case sf::Keyboard::W:
            case sf::Keyboard::Down: case sf::Keyboard::S:
                characterCardSelection = (characterCardSelection == 0) ? 1 : 0;
                SoundManager::getInstance().playSound("kick");
                updateVisuals();
                break;
            case sf::Keyboard::Enter: case sf::Keyboard::Space:
                activateSelection(window); break;
            case sf::Keyboard::Escape:
                SoundManager::getInstance().playSound("pipe");
                goBack(); break;
            default: break;
        }
        return;
    }

    switch (event.key.code) {
        case sf::Keyboard::Up: case sf::Keyboard::W:
            SoundManager::getInstance().playSound("stomp"); moveSelection(-1); break;
        case sf::Keyboard::Down: case sf::Keyboard::S:
            SoundManager::getInstance().playSound("stomp"); moveSelection(1); break;
        case sf::Keyboard::Left: case sf::Keyboard::A:
            if (page == Page::Settings && selectedIndex == 1) {
                adjustVolume(-10.f);
                SoundManager::getInstance().playSound("stomp");
            } break;
        case sf::Keyboard::Right: case sf::Keyboard::D:
            if (page == Page::Settings && selectedIndex == 1) {
                adjustVolume(10.f);
                SoundManager::getInstance().playSound("stomp");
            } break;
        case sf::Keyboard::Enter: case sf::Keyboard::Space:
            SoundManager::getInstance().playSound("coin"); activateSelection(window); break;
        case sf::Keyboard::Escape:
            SoundManager::getInstance().playSound("pipe"); goBack(); break;
        default: break;
    }
}

void MenuState::update(float dt) {
    globalTime += dt;

    for (auto& c : clouds) {
        c.position.x += c.speed * dt;
        if (c.position.x > UiWidth + 100.f) c.position.x = -150.f;
    }

    blinkTimer += dt;
    if (blinkTimer >= 0.4f) { showSelector = !showSelector; blinkTimer = 0.f; }

    if (displayMode == DisplayMode::TitleScreen) {
        if (logoLoaded) {
            float logoY = 185.f + std::sin(globalTime * 2.2f) * 6.f;
            float logoScale = baseLogoScale * (1.0f + 0.015f * std::sin(globalTime * 2.2f));
            logoSprite.setPosition(UiWidth / 2.f, logoY);
            logoSprite.setScale(logoScale, logoScale);
        }
        if (charLoaded) {
            float charBounce = -std::abs(std::sin(globalTime * 5.0f)) * 4.f;
            charSprite.setPosition(UiWidth / 2.f, 485.f + charBounce);
            charSprite.setScale(baseCharScale, baseCharScale);
        }
        float pulse = (std::sin(globalTime * 4.0f) + 1.f) * 0.5f;
        promptText.setScale(1.0f + pulse * 0.08f, 1.0f + pulse * 0.08f);
        promptText.setFillColor(sf::Color(255, static_cast<sf::Uint8>(215 + pulse * 35), static_cast<sf::Uint8>(40 + pulse * 40)));
        centerText(promptText, UiWidth / 2.f, 345.f);
    } else {
        if (logoLoaded && !isCharacterSelectionPage()) {
            float logoY = 100.f + std::sin(globalTime * 1.5f) * 3.f;
            float logoScale = baseLogoScale * 0.55f;
            logoSprite.setPosition(UiWidth / 2.f, logoY);
            logoSprite.setScale(logoScale, logoScale);
        }
        if (charLoaded && !isCharacterSelectionPage()) {
            float charBounce = -std::abs(std::sin(globalTime * 4.0f)) * 2.5f;
            charSprite.setPosition(UiWidth / 2.f, 520.f + charBounce);
            charSprite.setScale(baseCharScale * 0.85f, baseCharScale * 0.85f);
        }
        if (isCharacterSelectionPage()) {
            if (isCharacterConfirming) {
                characterConfirmTimer += dt;
                characterFlashTimer += dt;
                const float popScaleBonus = 1.20f;
                if (characterCardSelection == 0) {
                    marioCurrentScale += (baseMarioCardScale * popScaleBonus - marioCurrentScale) * std::min(1.f, dt * 15.f);
                } else {
                    luigiCurrentScale += (baseLuigiCardScale * popScaleBonus - luigiCurrentScale) * std::min(1.f, dt * 15.f);
                }
                if (characterConfirmTimer >= 0.7f) {
                    isCharacterConfirming = false;
                    characterConfirmTimer = 0.f;
                    if (page == Page::Character) {
                        setPage(Page::Solo);
                    } else if (page == Page::PvPCharacter) {
                        if (pvpSelectionStage == 1) {
                            pvpSelectionStage = 2;
                            setPage(Page::PvPCharacter);
                        } else {
                            launchPendingPvPMatch();
                        }
                    } else if (duoSelectionStage == 1) {
                        duoSelectionStage = 2;
                        setPage(Page::DuoCharacter);
                    } else {
                        launchPendingDuoGame();
                    }
                }
            } else {
                float marioTarget = (characterCardSelection == 0) ? (baseMarioCardScale * 1.15f) : (baseMarioCardScale * 0.88f);
                float luigiTarget = (characterCardSelection == 1) ? (baseLuigiCardScale * 1.15f) : (baseLuigiCardScale * 0.88f);
                marioCurrentScale += (marioTarget - marioCurrentScale) * std::min(1.f, dt * 10.f);
                luigiCurrentScale += (luigiTarget - luigiCurrentScale) * std::min(1.f, dt * 10.f);
            }
        }
    }
}

void MenuState::renderCharacterSelect(sf::RenderWindow& window) {
    window.draw(charChooseTitle);
    const float marioX = 270.f, luigiX = 530.f, baseCardY = 310.f;
    const float bob = isCharacterConfirming ? 0.f : (std::sin(globalTime * 3.5f) * 4.f);
    const float marioY = baseCardY + (characterCardSelection == 0 ? bob : 0.f);
    const float luigiY = baseCardY + (characterCardSelection == 1 ? bob : 0.f);
    const bool flashOn = !isCharacterConfirming || (static_cast<int>(characterFlashTimer * 16.f) % 2 == 0);

    if (marioCardLoaded) {
        if (characterCardSelection == 0) {
            if (isCharacterConfirming) {
                marioCardGlow.setOutlineColor(flashOn ? sf::Color(255,255,255,255) : sf::Color(255,220,60,160));
                marioCardGlow.setFillColor(flashOn ? sf::Color(255,255,255,100) : sf::Color(255,50,50,60));
                marioCardSprite.setColor(flashOn ? sf::Color(255,255,255) : sf::Color(255,220,180));
            } else {
                marioCardGlow.setOutlineColor(sf::Color(255,220,60,230));
                marioCardGlow.setFillColor(sf::Color(255,50,50,45));
                marioCardSprite.setColor(sf::Color::White);
            }
            marioCardGlow.setPosition(marioX, marioY);
            marioCardGlow.setScale(marioCurrentScale / baseMarioCardScale, marioCurrentScale / baseMarioCardScale);
            window.draw(marioCardGlow);
        } else {
            marioCardSprite.setColor(sf::Color(170,170,170,210));
        }
        marioCardSprite.setPosition(marioX, marioY);
        marioCardSprite.setScale(marioCurrentScale, marioCurrentScale);
        window.draw(marioCardSprite);
        const float badgeY = marioY + 160.f * (marioCurrentScale / baseMarioCardScale);
        if (isCharacterConfirming && characterCardSelection == 0) {
            marioCardBadge.setString(flashOn ? "[★ CONFIRMED! ★]" : "[              ]");
            marioCardBadge.setFillColor(sf::Color(255,255,100));
        }
        centerText(marioCardBadge, marioX, badgeY);
        window.draw(marioCardBadge);
    }

    if (luigiCardLoaded) {
        if (characterCardSelection == 1) {
            if (isCharacterConfirming) {
                luigiCardGlow.setOutlineColor(flashOn ? sf::Color(255,255,255,255) : sf::Color(255,220,60,160));
                luigiCardGlow.setFillColor(flashOn ? sf::Color(255,255,255,100) : sf::Color(50,220,80,60));
                luigiCardSprite.setColor(flashOn ? sf::Color(255,255,255) : sf::Color(200,255,200));
            } else {
                luigiCardGlow.setOutlineColor(sf::Color(255,220,60,230));
                luigiCardGlow.setFillColor(sf::Color(50,220,80,45));
                luigiCardSprite.setColor(sf::Color::White);
            }
            luigiCardGlow.setPosition(luigiX, luigiY);
            luigiCardGlow.setScale(luigiCurrentScale / baseLuigiCardScale, luigiCurrentScale / baseLuigiCardScale);
            window.draw(luigiCardGlow);
        } else {
            luigiCardSprite.setColor(sf::Color(170,170,170,210));
        }
        luigiCardSprite.setPosition(luigiX, luigiY);
        luigiCardSprite.setScale(luigiCurrentScale, luigiCurrentScale);
        window.draw(luigiCardSprite);
        const float badgeY = luigiY + 160.f * (luigiCurrentScale / baseLuigiCardScale);
        if (isCharacterConfirming && characterCardSelection == 1) {
            luigiCardBadge.setString(flashOn ? "[★ CONFIRMED! ★]" : "[              ]");
            luigiCardBadge.setFillColor(sf::Color(255,255,100));
        }
        centerText(luigiCardBadge, luigiX, badgeY);
        window.draw(luigiCardBadge);
    }

    if (!isCharacterConfirming) {
        const float arrowPulse = std::sin(globalTime * 6.f) * 4.f;
        if (characterCardSelection == 0) {
            arrowLeft.setPosition(marioX - 140.f + arrowPulse, marioY);
            arrowRight.setPosition(marioX + 140.f - arrowPulse, marioY);
        } else {
            arrowLeft.setPosition(luigiX - 140.f + arrowPulse, luigiY);
            arrowRight.setPosition(luigiX + 140.f - arrowPulse, luigiY);
        }
        window.draw(arrowLeft);
        window.draw(arrowRight);
    }
    window.draw(charChoosePrompt);
}

void MenuState::render(sf::RenderWindow& window) {
    const sf::View previousView = window.getView();
    const sf::View uiView(sf::FloatRect(0.f, 0.f, UiWidth, UiHeight));
    window.setView(uiView);

    if (bgLoaded) {
        window.draw(bgSprite);
    } else {
        window.clear(sf::Color(92, 148, 252));
    }

    if (cloudsLoaded) {
        for (const auto& c : clouds) {
            sf::Sprite* spr = (c.textureIndex == 1) ? &cloudSprite1 : (c.textureIndex == 2 ? &cloudSprite2 : &cloudSprite3);
            if (spr) {
                spr->setPosition(c.position);
                spr->setScale(c.scale, c.scale);
                window.draw(*spr);
            }
        }
    }

    if (charLoaded && !isCharacterSelectionPage()) {
        window.draw(charSprite);
    }

    if (displayMode == DisplayMode::TitleScreen) {
        if (logoLoaded) window.draw(logoSprite);
        window.draw(promptText);
        window.draw(copyrightText);
        window.draw(versionText);
    } else if (isCharacterSelectionPage()) {
        renderCharacterSelect(window);
    } else {
        if (logoLoaded) window.draw(logoSprite);
        window.draw(menuCard);
        window.draw(menuCardHeader);
        window.draw(pageTitleText);
        if (!entryTexts.empty() && selectedIndex >= 0 && selectedIndex < static_cast<int>(entryTexts.size()) && entries[selectedIndex].enabled) {
            window.draw(selectionGlow);
        }
        for (const auto& text : entryTexts) window.draw(text);
        if (page == Page::KeyBindings) window.draw(keyBindingsText);
        if (page == Page::Achievements) window.draw(achievementsText);
        window.draw(statusText);
        window.draw(footerText);
        if (showSelector && !entryTexts.empty()) window.draw(selectorText);
    }

    window.setView(previousView);
}
