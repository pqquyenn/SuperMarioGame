#include "Entities/Enemies/Goomba.h"

Goomba::Goomba(float x, float y) : Enemy(x, y) {
  speed = 40.f;
  direction = -1; // Mặc định di chuyển sang trái
}

// ============================================================
// update – Logic Goomba mỗi frame
//   Nếu squished: đếm timer rồi biến mất
//   Nếu bình thường: di chuyển ngang + trọng lực (velocity-based)
// ============================================================
void Goomba::update(float dt) {
  if (!active)
    return;

  if (squished) {
    // Khi bị dẫm: dừng lại, đếm timer rồi biến mất
    velocity.x = 0.f;
    velocity.y = 0.f;
    squishTimer += dt;
    if (squishTimer >= squishDuration) {
      active = false;
      isAlive = false;
    }
  } else {
    // Di chuyển ngang + trọng lực (sử dụng applyPhysics từ Enemy base)
    // applyPhysics sẽ: set velocity.x = direction * speed,
    //                   velocity.y += GRAVITY * dt,
    //                   position += velocity * dt
    applyPhysics(dt);
  }
}

void Goomba::onStomped() {
  if (!squished && isAlive) {
    squished = true;
    speed = 0.f;
    squishTimer = 0.f;
  }
}

// ============================================================
// render – Vẽ Goomba
//   Sprite nếu có texture, fallback: hình chữ nhật nâu (bẹp khi bị dẫm)
// ============================================================
void Goomba::render(sf::RenderWindow &window) const {
  if (!active)
    return;

  // Nếu đã gán texture cho sprite thì vẽ sprite
  if (sprite.getTexture() != nullptr) {
    window.draw(sprite);
  } else {
    // Fallback: Vẽ hình chữ nhật màu nâu đại diện cho Goomba (Bẹp xuống khi bị
    // dẫm)
    float currentHeight = squished ? 10.f : size.y;
    float yOffset = squished ? (size.y - currentHeight) : 0.f;

    sf::RectangleShape shape(sf::Vector2f(size.x, currentHeight));
    shape.setPosition(position.x, position.y + yOffset);
    shape.setFillColor(squished ? sf::Color(139, 69, 19)
                                : sf::Color(210, 105, 30));
    shape.setOutlineColor(sf::Color::Black);
    shape.setOutlineThickness(1.f);

    window.draw(shape);
  }
}

float Goomba::getSquishTimer() const { return squishTimer; }

sf::FloatRect Goomba::getBounds() const {
  float currentHeight = squished ? 10.f : size.y;
  float yOffset = squished ? (size.y - currentHeight) : 0.f;
  return sf::FloatRect(position.x, position.y + yOffset, size.x, currentHeight);
}
