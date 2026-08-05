#include "UI/HUD.h"
#include <iostream>
#include <filesystem>

// ============================================================
// Constructor – Load font và khởi tạo các sf::Text
// ============================================================
HUD::HUD() {
    // Tìm font file "press-start-2p.ttf" (retro pixel font)
    const std::string fontCandidates[] = {
        "assets/fonts/press-start-2p.ttf",
        "../assets/fonts/press-start-2p.ttf",
        "../../assets/fonts/press-start-2p.ttf"
    };

    for (const auto& path : fontCandidates) {
        if (std::filesystem::exists(path)) {
            if (font.loadFromFile(path)) {
                fontLoaded = true;
                std::cout << "[HUD] Font loaded: " << path << std::endl;
                break;
            }
        }
    }

    if (!fontLoaded) {
        std::cerr << "[HUD] WARNING: Could not load font! HUD text will not render." << std::endl;
        return;
    }

    // ── Kích thước chữ (pixel font, NES style) ──
    // Camera view = 400x225, dùng charSize nhỏ cho vừa
    const unsigned int labelSize = 8;
    const unsigned int valueSize = 8;

    // ── Hàng 1: Labels (MARIO, WORLD, TIME) ──
    // Chia đều 3 cột trên chiều rộng 400px
    initText(marioLabel, labelSize, 16.f, 4.f);
    marioLabel.setString("MARIO");

    initText(worldLabel, labelSize, 160.f, 4.f);
    worldLabel.setString("WORLD");

    initText(timeLabel, labelSize, 310.f, 4.f);
    timeLabel.setString("TIME");

    // ── Hàng 2: Values (Score, Coins, World, Time, Lives) ──
    initText(scoreText, valueSize, 16.f, 16.f);
    initText(coinsText, valueSize, 100.f, 16.f);    // "x00" sau icon coin
    initText(worldText, valueSize, 164.f, 16.f);
    initText(timeText, valueSize, 314.f, 16.f);
    initText(livesText, valueSize, 240.f, 16.f);     // "Lx3"
}

// ============================================================
// initText – Thiết lập 1 sf::Text chuẩn NES
// ============================================================
void HUD::initText(sf::Text& text, unsigned int charSize, float x, float y) {
    text.setFont(font);
    text.setCharacterSize(charSize);
    text.setFillColor(sf::Color::White);
    text.setPosition(x, y);
}

// ============================================================
// Format helpers
// ============================================================
std::string HUD::formatScore() const {
    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(6) << score;
    return ss.str();
}

std::string HUD::formatCoins() const {
    std::ostringstream ss;
    ss << "x" << std::setfill('0') << std::setw(2) << coins;
    return ss.str();
}

std::string HUD::formatTime() const {
    int t = static_cast<int>(timeRemaining);
    if (t < 0) t = 0;
    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(3) << t;
    return ss.str();
}

std::string HUD::formatLives() const {
    std::ostringstream ss;
    ss << "Lx" << lives;
    return ss.str();
}

// ============================================================
// onNotify – Nhận sự kiện từ Observer Pattern
// ============================================================
void HUD::onNotify(const GameEvent& event) {
    switch (event.type) {
        case GameEventType::COIN_COLLECTED:
            coins += 1;
            score += 200;
            break;
        case GameEventType::ENEMY_DEFEATED:
            score += 100;
            break;
        case GameEventType::PLAYER_DIED:
            lives -= 1;
            break;
        case GameEventType::POWERUP_COLLECTED:
            score += 1000;
            break;
        default:
            break;
    }
}

// ============================================================
// update – Đếm ngược thời gian
// ============================================================
void HUD::update(float dt) {
    if (timeRemaining > 0) {
        timeRemaining -= dt;
    }
}

// ============================================================
// render – Vẽ HUD overlay lên màn hình
// Sử dụng view riêng (HUD view) để HUD không bị cuộn theo camera
// ============================================================
void HUD::render(sf::RenderWindow& window) {
    if (!fontLoaded) return;

    // Lưu view hiện tại (game camera)
    sf::View gameView = window.getView();

    // Tạo HUD view cố định (cùng kích thước với game view, nhưng tọa độ cố định)
    sf::View hudView;
    hudView.setSize(gameView.getSize());
    hudView.setCenter(gameView.getSize().x / 2.f, gameView.getSize().y / 2.f);
    window.setView(hudView);

    // Cập nhật nội dung text
    scoreText.setString(formatScore());
    coinsText.setString(formatCoins());
    worldText.setString(worldName);
    timeText.setString(formatTime());
    livesText.setString(formatLives());

    // Vẽ nền bán trong suốt cho HUD (thanh trên cùng)
    sf::RectangleShape hudBg(sf::Vector2f(gameView.getSize().x, 28.f));
    hudBg.setPosition(0.f, 0.f);
    hudBg.setFillColor(sf::Color(0, 0, 0, 100));
    window.draw(hudBg);

    // Vẽ các labels cố định
    window.draw(marioLabel);
    window.draw(worldLabel);
    window.draw(timeLabel);

    // Vẽ icon coin nhỏ (hình tròn vàng) trước coinsText
    sf::CircleShape coinIcon(3.f);
    coinIcon.setFillColor(sf::Color(255, 215, 0)); // Gold
    coinIcon.setPosition(90.f, 18.f);
    window.draw(coinIcon);

    // Vẽ các giá trị
    window.draw(scoreText);
    window.draw(coinsText);
    window.draw(worldText);
    window.draw(timeText);
    window.draw(livesText);

    // Khôi phục game camera view
    window.setView(gameView);
}
