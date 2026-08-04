#include "Entities/Enemies/Koopa.h"

Koopa::Koopa(float x, float y) : Enemy(x, y) {
    speed = 50.f;
}

// ============================================================
// update – Logic Koopa mỗi frame
//   Di chuyển ngang + trọng lực (velocity-based từ Enemy::applyPhysics)
//   Khi ở trạng thái shell (inShell, speed=0): đứng yên nhưng vẫn chịu trọng lực
//   Khi shell spinning (speed=300): trượt nhanh trên mặt đất
// ============================================================
void Koopa::update(float dt) {
    if (!active)
        return;

    // Áp dụng vật lý: velocity.x = direction * speed, gravity, tích phân vị trí
    applyPhysics(dt);
}

// ============================================================
// onStomped – Chuyển trạng thái khi bị dẫm
//   Walking → Shell (dừng lại)
//   Shell idle → Shell spinning (tốc độ 300)
//   Shell spinning → inactive (biến mất)
// ============================================================
void Koopa::onStomped() {
    if (!inShell) {
        // Walking → Shell: dừng lại, co vào vỏ
        inShell = true;
        speed = 0.f;
    } else if (!shellSpinning) {
        // Shell idle → Shell spinning: bắn vỏ rùa
        shellSpinning = true;
        speed = 300.f;
    } else {
        // Shell spinning → chết
        active = false;
    }
}

// ============================================================
// render – Vẽ Koopa
//   Sprite nếu có texture, fallback: hình dạng rùa bằng shapes
//   - Walking: thân xanh lá + mai rùa + đầu + mắt
//   - Shell: hình chữ nhật xanh lá nhỏ hơn
//   - Shell spinning: hình chữ nhật xanh lá xoay
// ============================================================
void Koopa::render(sf::RenderWindow& window) const {
    if (!active)
        return;

    if (sprite.getTexture() != nullptr) {
        window.draw(sprite);
        return;
    }

    // --- Fallback: vẽ Koopa bằng shapes ---
    if (inShell) {
        // Trạng thái Shell: hình chữ nhật nhỏ (vỏ rùa)
        float shellH = 24.f;
        float yOffset = size.y - shellH; // Đẩy xuống đáy

        sf::RectangleShape shell(sf::Vector2f(size.x, shellH));
        shell.setPosition(position.x, position.y + yOffset);
        shell.setFillColor(shellSpinning ? sf::Color(0, 200, 0)    // Xanh sáng khi spinning
                                         : sf::Color(0, 140, 0));  // Xanh tối khi idle
        shell.setOutlineColor(sf::Color(0, 80, 0));
        shell.setOutlineThickness(1.5f);
        window.draw(shell);

        // Vân mai rùa (đường ngang)
        float lineY = position.y + yOffset + shellH / 2.f;
        sf::RectangleShape stripe(sf::Vector2f(size.x - 4.f, 2.f));
        stripe.setPosition(position.x + 2.f, lineY);
        stripe.setFillColor(sf::Color(0, 100, 0));
        window.draw(stripe);
    } else {
        // Trạng thái Walking: thân + mai + đầu + mắt

        // Mai rùa (phần dưới, chiếm 2/3 chiều cao)
        float shellH = size.y * 0.65f;
        sf::RectangleShape shell(sf::Vector2f(size.x, shellH));
        shell.setPosition(position.x, position.y + size.y - shellH);
        shell.setFillColor(sf::Color(0, 160, 0)); // Xanh lá
        shell.setOutlineColor(sf::Color(0, 80, 0));
        shell.setOutlineThickness(1.f);
        window.draw(shell);

        // Đầu (phần trên, hình tròn/bầu dục)
        float headR = size.x * 0.3f;
        sf::CircleShape head(headR);
        head.setPosition(position.x + size.x / 2.f - headR + (direction * 4.f),
                         position.y);
        head.setFillColor(sf::Color(255, 220, 100)); // Vàng
        head.setOutlineColor(sf::Color(200, 170, 50));
        head.setOutlineThickness(1.f);
        window.draw(head);

        // Mắt
        float eyeR = 3.f;
        sf::CircleShape eye(eyeR);
        eye.setFillColor(sf::Color::Black);
        eye.setPosition(position.x + size.x / 2.f + (direction * 6.f) - eyeR,
                        position.y + headR * 0.5f);
        window.draw(eye);
    }
}

// ============================================================
// getBounds – Collision box thay đổi theo trạng thái
//   Walking: 32x48 (đầy đủ)
//   Shell: 32x24 (thấp hơn, đẩy xuống đáy)
// ============================================================
sf::FloatRect Koopa::getBounds() const {
    if (inShell) {
        float shellH = 24.f;
        float yOffset = size.y - shellH;
        return sf::FloatRect(position.x, position.y + yOffset, size.x, shellH);
    }
    return sf::FloatRect(position.x, position.y, size.x, size.y);
}

bool Koopa::isInShell() const { return inShell; }

bool Koopa::isShellSpinning() const { return shellSpinning; }
