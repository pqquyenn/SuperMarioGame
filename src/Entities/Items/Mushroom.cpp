#include "Entities/Items/Mushroom.h"

// ============================================================
// Constructor
// Nấm mặc định kích thước 32x32, trượt sang phải
// ============================================================
Mushroom::Mushroom(float x, float y) : Item(x, y) {
    size = {32.f, 32.f};
}

// ============================================================
// update – Logic mỗi frame
//   1. Nếu đang emerge: nhô lên từ gạch
//   2. Nếu đã emerge xong: di chuyển ngang + trọng lực
// ============================================================
void Mushroom::update(float dt) {
    if (!active || collected) return;

    if (emerging) {
        // Nhô lên từ gạch chấm hỏi
        emergeDistance += emergeSpeed * dt;
        position.y -= emergeSpeed * dt;
        if (emergeDistance >= emergeTarget) {
            emerging = false;
        }
    } else {
        // Di chuyển ngang
        position.x += moveDirection * moveSpeed * dt;

        // Áp trọng lực (nếu không đứng trên đất, sẽ rơi)
        vertVelocity += gravity * dt;
        position.y += vertVelocity * dt;

        // TODO: va chạm với đất sẽ được xử lý bởi CollisionManager
        // CollisionManager sẽ set vertVelocity = 0 và chỉnh position.y
    }

    sprite.setPosition(position);
}

// ============================================================
// render – Vẽ nấm
//   Sprite nếu có texture, fallback: hình nấm đỏ + chấm trắng
// ============================================================
void Mushroom::render(sf::RenderWindow& window) const {
    if (!active || collected) return;

    if (sprite.getTexture() != nullptr) {
        window.draw(sprite);
        return;
    }

    // --- Fallback: vẽ nấm bằng shapes ---
    // Chân nấm (thân trắng kem)
    float stemW = size.x * 0.5f;
    float stemH = size.y * 0.45f;
    sf::RectangleShape stem(sf::Vector2f(stemW, stemH));
    stem.setPosition(position.x + (size.x - stemW) / 2.f,
                     position.y + size.y - stemH);
    stem.setFillColor(sf::Color(255, 230, 200)); // Kem
    stem.setOutlineColor(sf::Color(180, 150, 100));
    stem.setOutlineThickness(1.f);
    window.draw(stem);

    // Mũ nấm (đỏ tròn)
    float capW = size.x;
    float capH = size.y * 0.6f;
    sf::RectangleShape cap(sf::Vector2f(capW, capH));
    cap.setPosition(position.x, position.y);
    cap.setFillColor(sf::Color(220, 20, 20)); // Đỏ
    cap.setOutlineColor(sf::Color(150, 10, 10));
    cap.setOutlineThickness(1.5f);
    window.draw(cap);

    // Chấm trắng trên mũ nấm
    float dotR = 4.f;
    sf::CircleShape dot1(dotR);
    dot1.setFillColor(sf::Color::White);
    dot1.setPosition(position.x + 6.f, position.y + 4.f);
    window.draw(dot1);

    sf::CircleShape dot2(dotR);
    dot2.setFillColor(sf::Color::White);
    dot2.setPosition(position.x + size.x - 14.f, position.y + 4.f);
    window.draw(dot2);

    sf::CircleShape dot3(3.f);
    dot3.setFillColor(sf::Color::White);
    dot3.setPosition(position.x + size.x / 2.f - 3.f, position.y + 2.f);
    window.draw(dot3);
}

// ============================================================
// onCollect – Gọi khi Mario chạm vào nấm
// ============================================================
#include "Entities/Character.h"
#include "PlayerStates/SuperState.h"

void Mushroom::onCollect() {
    if (!collected) {
        collected = true;
        active = false;
    }
}

bool Mushroom::tryCollect(Character& character) {
    if (!active || collected) return false;
    if (character.receivePowerUp(std::make_unique<SuperState>())) {
        onCollect();
        return true;
    }
    return false;
}

void Mushroom::startEmerge() {
    emerging = true;
    emergeDistance = 0.f;
}

bool Mushroom::isEmerging() const {
    return emerging;
}

void Mushroom::reverseDirection() {
    moveDirection = -moveDirection;
}

int Mushroom::getMoveDirection() const {
    return moveDirection;
}
