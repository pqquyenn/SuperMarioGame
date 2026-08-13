#include "States/MenuState.h"

#include "Core/GameSettings.h"
#include "Core/SoundManager.h"
#include "States/GameStateManager.h"
#include "States/PlayState.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <memory>

namespace {
constexpr float UiWidth = 800.f;
constexpr float UiHeight = 600.f;
constexpr float EntryStartY = 270.f;
constexpr float EntrySpacing = 46.f;
const sf::Color Accent{228, 166, 61};
const sf::Color Disabled{125, 125, 125};

void centerText(sf::Text& text, float x, float y) {
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.f,
                   bounds.top + bounds.height / 2.f);
    text.setPosition(x, y);
}

const char* characterName(CharacterChoice choice) {
    return choice == CharacterChoice::Luigi ? "LUIGI" : "MARIO";
}
}

void MenuState::onEnter() {
    std::cout << "[MenuState] Entered main menu" << std::endl;

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
        std::cerr << "[MenuState] Could not load menu font" << std::endl;
    }

    titleText.setFont(font);
    titleText.setString("SUPER MARIO BROS");
    titleText.setCharacterSize(28);
    titleText.setFillColor(Accent);
    titleText.setOutlineColor(sf::Color::Black);
    titleText.setOutlineThickness(2.f);
    centerText(titleText, UiWidth / 2.f, 105.f);

    pageTitleText.setFont(font);
    pageTitleText.setCharacterSize(14);
    pageTitleText.setFillColor(sf::Color::White);

    statusText.setFont(font);
    statusText.setCharacterSize(9);
    statusText.setFillColor(Accent);

    footerText.setFont(font);
    footerText.setCharacterSize(8);
    footerText.setFillColor(sf::Color(205, 205, 205));
    footerText.setString("ARROWS / WASD: MOVE     ENTER: SELECT     ESC: BACK");
    centerText(footerText, UiWidth / 2.f, 565.f);

    selectorText.setFont(font);
    selectorText.setString(">");
    selectorText.setCharacterSize(16);
    selectorText.setFillColor(Accent);

    keyBindingsText.setFont(font);
    keyBindingsText.setCharacterSize(10);
    keyBindingsText.setFillColor(sf::Color::White);
    keyBindingsText.setLineSpacing(1.6f);
    keyBindingsText.setString(
        "MOVE LEFT     A / LEFT\n"
        "MOVE RIGHT    D / RIGHT\n"
        "JUMP          SPACE / W / UP\n"
        "ACTION / RUN  Z / J / Q\n"
        "RUN           LEFT SHIFT / RIGHT SHIFT\n\n"
        "KEY REMAPPING IS COMING SOON");
    keyBindingsText.setPosition(175.f, 245.f);

    groundBlock.setSize({UiWidth, 64.f});
    groundBlock.setPosition(0.f, UiHeight - 64.f);
    groundBlock.setFillColor(sf::Color(192, 96, 0));

    panel.setSize({540.f, 350.f});
    panel.setPosition(130.f, 175.f);
    panel.setFillColor(sf::Color(0, 0, 0, 145));
    panel.setOutlineColor(sf::Color(255, 255, 255, 70));
    panel.setOutlineThickness(2.f);

    loadBackground();
    setPage(Page::GameMode);
}

void MenuState::onExit() {
    std::cout << "[MenuState] Leaving main menu" << std::endl;
}

bool MenuState::loadBackground() {
    const std::string paths[] = {
        "assets/state/MenuGameBackGround.jpg",
        "assets/state/MenuGameBackGround.png",
        "../assets/state/MenuGameBackGround.jpg",
        "../assets/state/MenuGameBackGround.png",
        "../../assets/state/MenuGameBackGround.jpg",
        "../../assets/state/MenuGameBackGround.png"
    };

    for (const auto& path : paths) {
        if (std::filesystem::exists(path) && bgTexture.loadFromFile(path)) {
            bgLoaded = true;
            bgSprite.setTexture(bgTexture);
            const sf::Vector2u size = bgTexture.getSize();
            if (size.x > 0 && size.y > 0) {
                bgSprite.setScale(UiWidth / static_cast<float>(size.x),
                                  UiHeight / static_cast<float>(size.y));
            }
            return true;
        }
    }
    return false;
}

void MenuState::setPage(Page newPage) {
    page = newPage;
    selectedIndex = 0;
    showSelector = true;
    blinkTimer = 0.f;
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
            entries = {
                {"SOLO"},
                {"DUO  [COMING SOON]", false},
                {"PVP  [COMING SOON]", false},
                {"SETTINGS"},
                {"EXIT"}
            };
            break;

        case Page::Solo:
            pageTitleText.setString("SOLO");
            entries = {
                {"PLAY"},
                {"CHARACTER"},
                {"ACHIEVEMENTS  [COMING SOON]", false},
                {"BACK"}
            };
            break;

        case Page::Play:
            pageTitleText.setString("SELECT WORLD");
            entries = {
                {"WORLD 1-1"},
                {"WORLD 1-2"},
                {"WORLD 1-3"},
                {"BACK"}
            };
            break;

        case Page::Character: {
            pageTitleText.setString("SELECT CHARACTER");
            const CharacterChoice selected =
                GameSettings::getInstance().getCharacterChoice();
            entries = {
                {std::string("MARIO") +
                 (selected == CharacterChoice::Mario ? "  [SELECTED]" : "")},
                {std::string("LUIGI") +
                 (selected == CharacterChoice::Luigi ? "  [SELECTED]" : "")},
                {"BACK"}
            };
            break;
        }

        case Page::Settings: {
            pageTitleText.setString("SETTINGS");
            const int volume = static_cast<int>(std::round(
                SoundManager::getInstance().getMasterVolume()));
            entries = {
                {"KEY BINDINGS"},
                {"MASTER VOLUME  < " + std::to_string(volume) + "% >"},
                {"BACK"}
            };
            break;
        }

        case Page::KeyBindings:
            pageTitleText.setString("KEY BINDINGS");
            entries = {{"BACK"}};
            break;
    }

    entryTexts.clear();
    entryTexts.reserve(entries.size());
    for (const auto& entry : entries) {
        sf::Text text;
        text.setFont(font);
        text.setString(entry.label);
        text.setCharacterSize(entry.label.size() > 24 ? 11 : 15);
        text.setFillColor(entry.enabled ? sf::Color::White : Disabled);
        entryTexts.push_back(text);
    }
}

void MenuState::moveSelection(int direction) {
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
    centerText(pageTitleText, UiWidth / 2.f, 205.f);

    for (std::size_t i = 0; i < entryTexts.size(); ++i) {
        const float firstEntryY =
            page == Page::KeyBindings ? 445.f : EntryStartY;
        const float y = firstEntryY + static_cast<float>(i) * EntrySpacing;
        centerText(entryTexts[i], UiWidth / 2.f, y);
        if (!entries[i].enabled) {
            entryTexts[i].setFillColor(Disabled);
        } else {
            entryTexts[i].setFillColor(
                static_cast<int>(i) == selectedIndex ? Accent : sf::Color::White);
        }
    }

    if (!entryTexts.empty()) {
        const sf::FloatRect bounds = entryTexts[selectedIndex].getGlobalBounds();
        selectorText.setPosition(bounds.left - 30.f,
                                 bounds.top + bounds.height / 2.f - 8.f);
    }

    if (page == Page::Character) {
        statusText.setString(std::string("CURRENT: ") + characterName(
            GameSettings::getInstance().getCharacterChoice()));
    } else if (page == Page::Settings && selectedIndex == 1) {
        statusText.setString("LEFT / RIGHT CHANGES VOLUME");
    } else if (page == Page::KeyBindings) {
        statusText.setString("DISPLAY ONLY - REMAPPING WILL BE ADDED LATER");
    } else {
        statusText.setString("");
    }
    centerText(statusText, UiWidth / 2.f, 500.f);
}

void MenuState::activateSelection(sf::RenderWindow& window) {
    if (entries.empty() || !entries[selectedIndex].enabled) return;

    switch (page) {
        case Page::GameMode:
            if (selectedIndex == 0) setPage(Page::Solo);
            else if (selectedIndex == 3) setPage(Page::Settings);
            else if (selectedIndex == 4) window.close();
            break;

        case Page::Solo:
            if (selectedIndex == 0) setPage(Page::Play);
            else if (selectedIndex == 1) setPage(Page::Character);
            else if (selectedIndex == 3) setPage(Page::GameMode);
            break;

        case Page::Play: {
            if (selectedIndex == 3) {
                setPage(Page::Solo);
                break;
            }
            static const char* maps[] = {
                "1.1/1-1.level", "1.2/1-2.level", "1.3/1-3.level"
            };
            if (stateManager) {
                stateManager->clearAndPushState(
                    std::make_unique<PlayState>(maps[selectedIndex]));
            }
            break;
        }

        case Page::Character:
            if (selectedIndex == 0) {
                GameSettings::getInstance().setCharacterChoice(
                    CharacterChoice::Mario);
                rebuildEntries();
                updateVisuals();
            } else if (selectedIndex == 1) {
                GameSettings::getInstance().setCharacterChoice(
                    CharacterChoice::Luigi);
                rebuildEntries();
                updateVisuals();
            } else {
                setPage(Page::Solo);
            }
            break;

        case Page::Settings:
            if (selectedIndex == 0) setPage(Page::KeyBindings);
            else if (selectedIndex == 1) adjustVolume(10.f);
            else setPage(Page::GameMode);
            break;

        case Page::KeyBindings:
            setPage(Page::Settings);
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

void MenuState::goBack() {
    switch (page) {
        case Page::GameMode: break;
        case Page::Solo: setPage(Page::GameMode); break;
        case Page::Play:
        case Page::Character: setPage(Page::Solo); break;
        case Page::Settings: setPage(Page::GameMode); break;
        case Page::KeyBindings: setPage(Page::Settings); break;
    }
}

void MenuState::handleInput(sf::Event& event, sf::RenderWindow& window) {
    if (event.type != sf::Event::KeyPressed) return;

    switch (event.key.code) {
        case sf::Keyboard::Up:
        case sf::Keyboard::W:
            moveSelection(-1);
            break;
        case sf::Keyboard::Down:
        case sf::Keyboard::S:
            moveSelection(1);
            break;
        case sf::Keyboard::Left:
        case sf::Keyboard::A:
            if (page == Page::Settings && selectedIndex == 1) {
                adjustVolume(-10.f);
            }
            break;
        case sf::Keyboard::Right:
        case sf::Keyboard::D:
            if (page == Page::Settings && selectedIndex == 1) {
                adjustVolume(10.f);
            }
            break;
        case sf::Keyboard::Enter:
        case sf::Keyboard::Space:
            activateSelection(window);
            break;
        case sf::Keyboard::Escape:
            goBack();
            break;
        default:
            break;
    }
}

void MenuState::update(float dt) {
    blinkTimer += dt;
    if (blinkTimer >= 0.4f) {
        showSelector = !showSelector;
        blinkTimer = 0.f;
    }
}

void MenuState::render(sf::RenderWindow& window) {
    const sf::View previousView = window.getView();
    const sf::View uiView(sf::FloatRect(0.f, 0.f, UiWidth, UiHeight));
    window.setView(uiView);

    if (bgLoaded) {
        window.draw(bgSprite);
    } else {
        window.clear(sf::Color(92, 148, 252));
        window.draw(groundBlock);
    }
    window.draw(panel);

    if (fontLoaded) {
        window.draw(titleText);
        window.draw(pageTitleText);
        for (const auto& text : entryTexts) window.draw(text);

        if (page == Page::KeyBindings) window.draw(keyBindingsText);
        window.draw(statusText);
        window.draw(footerText);
        if (showSelector && !entryTexts.empty()) window.draw(selectorText);
    }

    window.setView(previousView);
}
