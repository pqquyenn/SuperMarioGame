#include "States/MenuState.h"
#include "States/PlayState.h"
#include <iostream>
#include <memory>
#include <cmath>
#include <filesystem>

// ==============================================================
// onEnter() - Goi 1 lan khi MenuState duoc push vao stack
// Khoi tao font, tao cac text hien thi tren man hinh menu
// ==============================================================
void MenuState::onEnter() {
    std::cout << "[MenuState] onEnter - Khoi tao menu chinh" << std::endl;

    // --- Load font ---
    // Thu nhieu duong dan tuong doi de tim font
    const std::string fontPaths[] = {
        "assets/fonts/press-start-2p.ttf",
        "../assets/fonts/press-start-2p.ttf",
        "../../assets/fonts/press-start-2p.ttf",
        "../../../assets/fonts/press-start-2p.ttf"
    };

    fontLoaded = false;
    for (const auto& path : fontPaths) {
        if (std::filesystem::exists(path)) {
            if (font.loadFromFile(path)) {
                fontLoaded = true;
                std::cout << "[MenuState] Font loaded: " << path << std::endl;
                break;
            }
        }
    }

    if (!fontLoaded) {
        std::cerr << "[MenuState] WARNING: Khong tim thay font! Text se khong hien thi." << std::endl;
        return;
    }

    // --- Title: "SUPER MARIO BROS" ---
    titleText.setFont(font);
    titleText.setString("SUPER MARIO BROS");
    titleText.setCharacterSize(28);
    titleText.setFillColor(sf::Color(228, 166, 61));   // Mau vang cam giong Mario classic
    titleText.setOutlineColor(sf::Color::Black);
    titleText.setOutlineThickness(2.f);
    // Can giua theo chieu ngang (window 800px)
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.left + titleBounds.width / 2.f,
                        titleBounds.top + titleBounds.height / 2.f);
    titleText.setPosition(400.f, 160.f);

    // --- Subtitle ---
    subtitleText.setFont(font);
    subtitleText.setString("OOP C++ / SFML Project");
    subtitleText.setCharacterSize(10);
    subtitleText.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect subBounds = subtitleText.getLocalBounds();
    subtitleText.setOrigin(subBounds.left + subBounds.width / 2.f,
                           subBounds.top + subBounds.height / 2.f);
    subtitleText.setPosition(400.f, 210.f);

    // --- Menu item: "START GAME" ---
    startText.setFont(font);
    startText.setString("PLAY 1-1");
    startText.setCharacterSize(16);
    startText.setFillColor(sf::Color::White);
    sf::FloatRect startBounds = startText.getLocalBounds();
    startText.setOrigin(startBounds.left + startBounds.width / 2.f,
                        startBounds.top + startBounds.height / 2.f);
    startText.setPosition(400.f, 310.f);

    // --- Menu item: "PLAY 1-2" ---
    play12Text.setFont(font);
    play12Text.setString("PLAY 1-2");
    play12Text.setCharacterSize(16);
    play12Text.setFillColor(sf::Color::White);
    sf::FloatRect p12Bounds = play12Text.getLocalBounds();
    play12Text.setOrigin(p12Bounds.left + p12Bounds.width / 2.f,
                         p12Bounds.top + p12Bounds.height / 2.f);
    play12Text.setPosition(400.f, 350.f);

    // --- Menu item: "PLAY 1-3" ---
    play13Text.setFont(font);
    play13Text.setString("PLAY 1-3");
    play13Text.setCharacterSize(16);
    play13Text.setFillColor(sf::Color::White);
    sf::FloatRect p13Bounds = play13Text.getLocalBounds();
    play13Text.setOrigin(p13Bounds.left + p13Bounds.width / 2.f,
                         p13Bounds.top + p13Bounds.height / 2.f);
    play13Text.setPosition(400.f, 390.f);

    // --- Menu item: "EXIT" ---
    exitText.setFont(font);
    exitText.setString("EXIT");
    exitText.setCharacterSize(16);
    exitText.setFillColor(sf::Color::White);
    sf::FloatRect exitBounds = exitText.getLocalBounds();
    exitText.setOrigin(exitBounds.left + exitBounds.width / 2.f,
                       exitBounds.top + exitBounds.height / 2.f);
    exitText.setPosition(400.f, 430.f);

    // --- Selector ">" ---
    selectorText.setFont(font);
    selectorText.setString(">");
    selectorText.setCharacterSize(16);
    selectorText.setFillColor(sf::Color::White);

    // --- Ground block decoration ---
    groundBlock.setSize(sf::Vector2f(800.f, 64.f));
    groundBlock.setPosition(0.f, 536.f);
    groundBlock.setFillColor(sf::Color(192, 96, 0));  // Mau nau dat

    // --- Load background image ---
    const std::string bgPaths[] = {
        "assets/state/MenuGameBackGround.jpg",
        "assets/state/MenuGameBackGround.png",
        "assets/state/MenuGameBackGround.jpeg",
        "assets/state/MenuGameBackGround.bmp",
        "../assets/state/MenuGameBackGround.jpg",
        "../assets/state/MenuGameBackGround.png",
        "../assets/state/MenuGameBackGround.jpeg",
        "../../assets/state/MenuGameBackGround.jpg",
        "../../assets/state/MenuGameBackGround.png",
        "../../../assets/state/MenuGameBackGround.jpg",
        "../../../assets/state/MenuGameBackGround.png"
    };

    bgLoaded = false;
    for (const auto& path : bgPaths) {
        if (std::filesystem::exists(path)) {
            if (bgTexture.loadFromFile(path)) {
                bgLoaded = true;
                bgSprite.setTexture(bgTexture);
                sf::Vector2u texSize = bgTexture.getSize();
                if (texSize.x > 0 && texSize.y > 0) {
                    bgSprite.setScale(800.f / static_cast<float>(texSize.x), 600.f / static_cast<float>(texSize.y));
                }
                std::cout << "[MenuState] Background image loaded: " << path << std::endl;
                break;
            }
        }
    }

    // Fallback: Check if there's any image file inside assets/state directory
    if (!bgLoaded) {
        const std::string stateDirs[] = { "assets/state", "../assets/state", "../../assets/state" };
        for (const auto& dir : stateDirs) {
            if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir)) {
                for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                    if (entry.is_regular_file()) {
                        std::string ext = entry.path().extension().string();
                        if (ext == ".jpg" || ext == ".png" || ext == ".jpeg" || ext == ".bmp") {
                            if (bgTexture.loadFromFile(entry.path().string())) {
                                bgLoaded = true;
                                bgSprite.setTexture(bgTexture);
                                sf::Vector2u texSize = bgTexture.getSize();
                                if (texSize.x > 0 && texSize.y > 0) {
                                    bgSprite.setScale(800.f / static_cast<float>(texSize.x), 600.f / static_cast<float>(texSize.y));
                                }
                                std::cout << "[MenuState] Background image loaded from dir scan: " << entry.path().string() << std::endl;
                                break;
                            }
                        }
                    }
                }
            }
            if (bgLoaded) break;
        }
    }


    // --- Mac dinh chon START GAME ---
    selectedIndex = 0;
    updateSelectorPosition();
}

// ==============================================================
// onExit() - Goi khi MenuState bi pop/change
// ==============================================================
void MenuState::onExit() {
    std::cout << "[MenuState] onExit - Roi khoi menu" << std::endl;
}

// ==============================================================
// handleInput() - Xu ly tung event rieng le
// - Up/Down: chuyen lua chon menu
// - Enter: xac nhan lua chon
// ==============================================================
void MenuState::handleInput(sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Up:
            case sf::Keyboard::W:
                // Di chuyen len
                selectedIndex--;
                if (selectedIndex < 0) selectedIndex = MENU_ITEMS - 1;
                updateSelectorPosition();
                break;

            case sf::Keyboard::Down:
            case sf::Keyboard::S:
                // Di chuyen xuong
                selectedIndex++;
                if (selectedIndex >= MENU_ITEMS) selectedIndex = 0;
                updateSelectorPosition();
                break;

            case sf::Keyboard::Enter:
                // Xac nhan lua chon
                if (selectedIndex == 0) {
                    if (stateManager) {
                        stateManager->changeState(std::make_unique<PlayState>("1.1/1-1.txt"));
                    }
                } else if (selectedIndex == 1) {
                    if (stateManager) {
                        stateManager->changeState(std::make_unique<PlayState>("1.2/1-2.txt"));
                    }
                } else if (selectedIndex == 2) {
                    if (stateManager) {
                        stateManager->changeState(std::make_unique<PlayState>("1.3/1-3.txt"));
                    }
                } else if (selectedIndex == 3) {
                    window.close();
                }
                break;

            default:
                break;
        }
    }
}

// ==============================================================
// update() - Goi moi frame
// - Animation nhap nhay selector ">"
// ==============================================================
void MenuState::update(float dt) {
    // Nhap nhay selector moi 0.4 giay
    blinkTimer += dt;
    if (blinkTimer >= 0.4f) {
        showSelector = !showSelector;
        blinkTimer = 0.f;
    }
}

// ==============================================================
// render() - Ve menu len man hinh
// ==============================================================
void MenuState::render(sf::RenderWindow& window) {
    if (bgLoaded) {
        window.draw(bgSprite);
    } else {
        // Fallback: Background xanh troi Mario classic
        window.clear(sf::Color(92, 148, 252));
        window.draw(groundBlock);
    }

    if (!fontLoaded) {
        // Neu khong co font, chi hien thi background
        return;
    }

    // Ve title
    window.draw(titleText);

    // Ve subtitle
    window.draw(subtitleText);

    // Ve menu items
    window.draw(startText);
    window.draw(play12Text);
    window.draw(play13Text);
    window.draw(exitText);

    // Ve selector ">" (nhap nhay)
    if (showSelector) {
        window.draw(selectorText);
    }
}


// ==============================================================
// updateSelectorPosition() - Cap nhat vi tri ">" theo muc dang chon
// ==============================================================
void MenuState::updateSelectorPosition() {
    float yPositions[] = { 310.f, 350.f, 390.f, 430.f };
    selectorText.setPosition(280.f, yPositions[selectedIndex]);

    // Highlight muc dang chon = mau vang, muc khac = trang
    startText.setFillColor(selectedIndex == 0 ? sf::Color(228, 166, 61) : sf::Color::White);
    play12Text.setFillColor(selectedIndex == 1 ? sf::Color(228, 166, 61) : sf::Color::White);
    play13Text.setFillColor(selectedIndex == 2 ? sf::Color(228, 166, 61) : sf::Color::White);
    exitText.setFillColor(selectedIndex == 3 ? sf::Color(228, 166, 61) : sf::Color::White);
}
