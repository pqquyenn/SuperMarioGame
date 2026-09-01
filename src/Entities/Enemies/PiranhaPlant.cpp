#include "Entities/Enemies/PiranhaPlant.h"
#include "Core/AssetManager.h"
#include <algorithm> // std::min, std::max

// ============================================================
// Constructor
// Vị trí (x, y) = đỉnh ống. PiranhaPlant bắt đầu ẩn hoàn toàn.
// ============================================================
PiranhaPlant::PiranhaPlant(float x, float y) : Enemy(x, y) {
    speed = 0.f;        // PiranhaPlant không di chuyển ngang
    direction = 0;      // Không có hướng trái/phải
    pipeTopY = y;        // Lưu vị trí đỉnh ống
    currentRise = 0.f;   // Bắt đầu ẩn hoàn toàn trong ống
    currentState = State::WAITING_BOT;
    waitTimer = 0.f;
    waitingForInitialRise = true;

    // Đặt position ban đầu = ẩn hoàn toàn (dưới đỉnh ống)
    position.y = pipeTopY;
}

// ============================================================
// setPipeTopY – Gọi sau khi tạo object để đặt vị trí ống
// ============================================================
void PiranhaPlant::setPipeTopY(float y) {
    pipeTopY = y;
    position.y = pipeTopY; // Reset về vị trí ẩn
    currentRise = 0.f;
    currentState = State::WAITING_BOT;
    waitTimer = 0.f;
    waitingForInitialRise = true;
}

void PiranhaPlant::setCycleTiming(float visibleSeconds,
                                  float hiddenSeconds,
                                  float initialDelaySeconds) {
    visibleDuration = std::max(0.f, visibleSeconds);
    hiddenDuration = std::max(0.f, hiddenSeconds);
    initialHiddenDelay = std::max(0.f, initialDelaySeconds);
    position.y = pipeTopY;
    currentRise = 0.f;
    currentState = State::WAITING_BOT;
    waitTimer = 0.f;
    waitingForInitialRise = true;
}

// ============================================================
// update – State machine chính: RISING → WAITING_TOP → DESCENDING → WAITING_BOT → ...
// ============================================================
void PiranhaPlant::update(float dt) {
    if (!active) return;

    switch (currentState) {

        case State::RISING:
            // Nhô lên với riseSpeed (pixels/sec)
            currentRise += riseSpeed * dt;
            if (currentRise >= riseHeight) {
                currentRise = riseHeight;
                currentState = State::WAITING_TOP;
                waitTimer = 0.f;
            }
            // Cập nhật vị trí Y: nhô lên = position.y giảm
            position.y = pipeTopY - currentRise;
            break;

        case State::WAITING_TOP:
            // Chờ ở đỉnh
            waitTimer += dt;
            if (waitTimer >= visibleDuration) {
                currentState = State::DESCENDING;
            }
            break;

        case State::DESCENDING:
            // Thu xuống với riseSpeed
            currentRise -= riseSpeed * dt;
            if (currentRise <= 0.f) {
                currentRise = 0.f;
                currentState = State::WAITING_BOT;
                waitTimer = 0.f;
                waitingForInitialRise = false;
            }
            position.y = pipeTopY - currentRise;
            break;

        case State::WAITING_BOT:
            // Chờ ở đáy (ẩn hoàn toàn trong ống)
            waitTimer += dt;
            const float targetWait = waitingForInitialRise
                ? initialHiddenDelay
                : hiddenDuration;
            if (waitTimer >= targetWait) {
                currentState = State::RISING;
                waitTimer = 0.f;
            }
            break;
    }

    if (currentRise > 0.f) {
        animTimer += dt;
        if (animTimer >= animInterval) {
            animTimer = 0.f;
            currentFrame = 1 - currentFrame;
            auto& assets = AssetManager::getInstance();
            if (currentFrame == 0) {
                sprite.setTexture(assets.getTexture("PiranhaPlant_1"));
            } else {
                sprite.setTexture(assets.getTexture("PiranhaPlant_2"));
            }
        }
    }

    sprite.setPosition(position);
}

// ============================================================
// render – Vẽ PiranhaPlant
//   - Ưu tiên sprite nếu có texture
//   - Fallback: hình chữ nhật xanh lá + "miệng" đỏ
//   - Chỉ vẽ phần nhô lên (clip bởi currentRise)
// ============================================================
void PiranhaPlant::render(sf::RenderWindow& window) const {
    if (!active) return;
    if (currentRise <= 0.f) return; // Ẩn hoàn toàn → không vẽ

    if (sprite.getTexture() != nullptr) {
        sf::Sprite drawSprite = sprite;
        sf::Vector2u texSize = sprite.getTexture()->getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            // Scale sprite to plantSize (no stretching, full texture shown)
            drawSprite.setScale(plantSize.x / static_cast<float>(texSize.x),
                                plantSize.y / static_cast<float>(texSize.y));
            // Vẽ nguyên sprite tại vị trí nhô lên, phần còn trong ống bị tile che tự nhiên
            drawSprite.setPosition(position.x, pipeTopY - currentRise);
        }
        window.draw(drawSprite);
        return;
    }

    // --- Fallback: vẽ shape ---
    // Chỉ vẽ phần nhô lên khỏi ống
    float visibleHeight = std::min(currentRise, plantSize.y);
    float drawY = pipeTopY - currentRise;

    // Thân cây (xanh lá đậm)
    sf::RectangleShape stem(sf::Vector2f(plantSize.x * 0.5f, visibleHeight));
    stem.setPosition(position.x + plantSize.x * 0.25f, drawY);
    stem.setFillColor(sf::Color(0, 120, 0));
    stem.setOutlineColor(sf::Color(0, 70, 0));
    stem.setOutlineThickness(1.f);
    window.draw(stem);

    // Đầu cây ăn thịt (phần trên, rộng hơn thân)
    if (currentRise > plantSize.y * 0.3f) {
        float headH = plantSize.y * 0.4f;
        sf::RectangleShape head(sf::Vector2f(plantSize.x, headH));
        head.setPosition(position.x, drawY);
        head.setFillColor(sf::Color(0, 160, 0));
        head.setOutlineColor(sf::Color(0, 80, 0));
        head.setOutlineThickness(1.5f);
        window.draw(head);

        // Miệng (chấm đỏ/trắng ở giữa đầu)
        float mouthW = plantSize.x * 0.6f;
        float mouthH = headH * 0.35f;
        sf::RectangleShape mouth(sf::Vector2f(mouthW, mouthH));
        mouth.setPosition(position.x + (plantSize.x - mouthW) / 2.f,
                          drawY + headH * 0.3f);
        mouth.setFillColor(sf::Color(200, 30, 30));
        window.draw(mouth);

        // Răng (2 hình tam giác nhỏ = 2 hình chữ nhật trắng nhỏ)
        float toothW = 4.f;
        float toothH = 4.f;
        sf::RectangleShape toothL(sf::Vector2f(toothW, toothH));
        toothL.setPosition(position.x + plantSize.x * 0.3f, drawY + headH * 0.25f);
        toothL.setFillColor(sf::Color::White);

        sf::RectangleShape toothR(sf::Vector2f(toothW, toothH));
        toothR.setPosition(position.x + plantSize.x * 0.6f, drawY + headH * 0.25f);
        toothR.setFillColor(sf::Color::White);

        window.draw(toothL);
        window.draw(toothR);
    }
}

// ============================================================
// onStomped – PiranhaPlant KHÔNG bị dẫm chết
// Chỉ chết bởi fireball hoặc star (xử lý ở CollisionManager)
// ============================================================
void PiranhaPlant::onStomped() {
    // Immune – PiranhaPlant không thể bị dẫm
    // Mario sẽ bị thương nếu chạm vào thay vì giết PiranhaPlant
}

// ============================================================
// getBounds – Collision box chỉ tính phần nhô lên
// ============================================================
sf::FloatRect PiranhaPlant::getBounds() const {
    if (currentRise <= 0.f) {
        // Ẩn hoàn toàn → không có collision box
        return sf::FloatRect(0.f, 0.f, 0.f, 0.f);
    }
    float visibleHeight = std::min(currentRise, plantSize.y);
    return sf::FloatRect(position.x, pipeTopY - currentRise, plantSize.x, visibleHeight);
}

// ============================================================
// Getters
// ============================================================
PiranhaPlant::State PiranhaPlant::getCurrentState() const {
    return currentState;
}

float PiranhaPlant::getCurrentRise() const {
    return currentRise;
}
