#include "Entities/Items/FireFlower.h"
#include <cmath> // std::sin

// ============================================================
// Constructor
// FireFlower đứng yên, kích thước 32x32
// ============================================================
FireFlower::FireFlower(float x, float y) : Item(x, y) {
    size  = {32.f, 32.f};
    baseY = y; // Lưu vị trí Y gốc cho animation nhấp nhô
}

// ============================================================
// update – Logic mỗi frame
//   1. Nếu đang emerge: nhô lên từ gạch
//   2. Nếu đã emerge xong: đứng yên + bob animation (sin wave)
// ============================================================
void FireFlower::update(float dt) {
    if (!active || collected) return;

    if (emerging) {
        // Nhô lên từ gạch chấm hỏi
        emergeDistance += emergeSpeed * dt;
        position.y -= emergeSpeed * dt;
        if (emergeDistance >= emergeTarget) {
            emerging = false;
            baseY = position.y; // Lưu vị trí Y mới sau khi emerge
        }
    } else {
        // Bob animation: nhấp nhô nhẹ bằng sin wave
        bobTimer += dt;
        position.y = baseY + bobAmplitude * std::sin(bobSpeed * bobTimer * 2.f * 3.14159f);
    }

    sprite.setPosition(position);
}

// ============================================================
// render – Vẽ hoa lửa
//   Sprite nếu có texture, fallback: bông hoa cam/đỏ/vàng
// ============================================================
void FireFlower::render(sf::RenderWindow& window) const {
    if (!active || collected) return;

    if (sprite.getTexture() != nullptr) {
        window.draw(sprite);
        return;
    }

    // --- Fallback: vẽ hoa lửa bằng shapes ---
    // Thân hoa (xanh lá)
    float stemW = 6.f;
    float stemH = size.y * 0.5f;
    sf::RectangleShape stem(sf::Vector2f(stemW, stemH));
    stem.setPosition(position.x + (size.x - stemW) / 2.f,
                     position.y + size.y - stemH);
    stem.setFillColor(sf::Color(0, 140, 0));
    stem.setOutlineColor(sf::Color(0, 80, 0));
    stem.setOutlineThickness(1.f);
    window.draw(stem);

    // Nhụy hoa (tròn vàng ở giữa)
    float centerR = 6.f;
    sf::CircleShape center(centerR);
    center.setFillColor(sf::Color(255, 220, 50)); // Vàng
    center.setPosition(position.x + size.x / 2.f - centerR,
                       position.y + size.y * 0.15f);
    window.draw(center);

    // Cánh hoa (4 hình tròn cam/đỏ xung quanh nhụy)
    float petalR = 5.f;
    sf::Color petalColors[] = {
        sf::Color(255, 100, 20),  // Cam
        sf::Color(255, 50, 10),   // Đỏ
        sf::Color(255, 120, 30),  // Cam nhạt
        sf::Color(255, 60, 20)    // Đỏ cam
    };
    float cx = position.x + size.x / 2.f;
    float cy = position.y + size.y * 0.15f + centerR;
    float offsets[][2] = {{-8.f, -2.f}, {8.f, -2.f}, {0.f, -9.f}, {0.f, 7.f}};

    for (int i = 0; i < 4; ++i) {
        sf::CircleShape petal(petalR);
        petal.setFillColor(petalColors[i]);
        petal.setPosition(cx + offsets[i][0] - petalR,
                          cy + offsets[i][1] - petalR);
        window.draw(petal);
    }

    // Vẽ lại nhụy lên trên cánh hoa
    window.draw(center);
}

// ============================================================
// onCollect – Mario ăn hoa lửa
// ============================================================
void FireFlower::onCollect() {
    if (!collected) {
        collected = true;
        active = false;
    }
}

void FireFlower::startEmerge() {
    emerging = true;
    emergeDistance = 0.f;
}

bool FireFlower::isEmerging() const {
    return emerging;
}
