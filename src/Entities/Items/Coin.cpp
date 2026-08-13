#include "Entities/Items/Coin.h"
#include <cmath> // std::sin, std::abs

// ============================================================
// Constructor
// Coin kích thước 16x16 (chuẩn 1 tile)
// ============================================================
Coin::Coin(float x, float y) : Item(x, y) {
    size = {16.f, 16.f};
    position = {x, y};
}

// ============================================================
// update – Logic mỗi frame
//   1. Nếu đang pop: bay lên rồi rơi xuống, khi qua vị trí gốc → biến mất
//   2. Bình thường: chỉ chạy spin timer cho animation xoay
// ============================================================
void Coin::update(float dt) {
    if (!active || collected) return;

    // Spin animation timer (chạy liên tục)
    spinTimer += dt;

    if (popping) {
        // Pop animation: bay lên rồi rơi
        currentPopVelocity += popGravity * dt;
        position.y += currentPopVelocity * dt;

        // Khi rơi qua vị trí bắt đầu → kết thúc animation
        if (position.y >= popStartY) {
            collected = true;
            active = false;
        }
    }

    sprite.setPosition(position);
}

// ============================================================
// render – Vẽ đồng xu
//   Sprite nếu có texture, fallback: hình tròn vàng với effect xoay
//   (scale ngang dao động để giả lập xoay 3D)
// ============================================================
void Coin::render(sf::RenderWindow& window) const {
    if (!active || collected) return;

    if (sprite.getTexture() != nullptr) {
        sf::Sprite drawSprite = sprite;
        drawSprite.setPosition(position);
        sf::Vector2u texSize = sprite.getTexture()->getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            drawSprite.setScale(size.x / static_cast<float>(texSize.x), size.y / static_cast<float>(texSize.y));
        }
        window.draw(drawSprite);
        return;
    }

    // --- Fallback: vẽ coin bằng shapes ---
    // Giả lập xoay 3D bằng scale ngang (sin wave)
    float scaleX = std::abs(std::sin(spinSpeed * spinTimer * 2.f * 3.14159f));
    // Clamp tối thiểu 0.2 để coin không biến mất hoàn toàn
    if (scaleX < 0.2f) scaleX = 0.2f;

    float coinW = size.x * scaleX;
    float coinH = size.y;
    float drawX = position.x + (size.x - coinW) / 2.f; // Canh giữa

    // Thân coin (vàng)
    sf::RectangleShape coinBody(sf::Vector2f(coinW, coinH));
    coinBody.setPosition(drawX, position.y);
    coinBody.setFillColor(sf::Color(255, 215, 0)); // Vàng gold
    coinBody.setOutlineColor(sf::Color(200, 160, 0));
    coinBody.setOutlineThickness(1.5f);
    window.draw(coinBody);

    // Ký hiệu "$" ở giữa coin (chỉ vẽ khi coin đủ rộng)
    if (coinW > 10.f) {
        // Vạch dọc
        float lineW = 2.f;
        float lineH = coinH * 0.5f;
        sf::RectangleShape line(sf::Vector2f(lineW, lineH));
        line.setPosition(drawX + coinW / 2.f - lineW / 2.f,
                         position.y + (coinH - lineH) / 2.f);
        line.setFillColor(sf::Color(180, 140, 0));
        window.draw(line);
    }
}

// ============================================================
// onCollect – Mario thu thập coin
// ============================================================
#include "Entities/Character.h"

void Coin::onCollect() {
    if (!collected) {
        collected = true;
        active = false;
    }
}

bool Coin::tryCollect(Character& character) {
    if (!active || collected) return false;
    onCollect();
    character.notify(GameEvent::coinCollected(scoreValue));
    return true;
}

// ============================================================
// startPop – Bắt đầu animation pop (bắn ra từ gạch chấm hỏi)
// ============================================================
void Coin::startPop() {
    popping = true;
    popStartY = position.y;
    currentPopVelocity = popVelocity; // Bay lên
}

bool Coin::isPopping() const {
    return popping;
}

int Coin::getScoreValue() const {
    return scoreValue;
}

void Coin::setScoreValue(int score) {
    scoreValue = score;
}
