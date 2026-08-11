#include "Entities/Items/OneUpMushroom.h"
#include "Entities/Character.h"
#include "Observer/Event.h"

// ============================================================
// Constructor
// Khởi tạo vị trí thông qua Mushroom
// ============================================================
OneUpMushroom::OneUpMushroom(float x, float y) : Mushroom(x, y) {}

// ============================================================
// tryCollect – Khi Mario chạm vào nấm xanh 1UP
// Tăng 1 mạng cho Mario (phát sự kiện LIFE_GAINED)
// ============================================================
bool OneUpMushroom::tryCollect(Character &character) {
  if (!active || collected)
    return false;

  // Phát sự kiện tăng 1 mạng
  character.notify(GameEvent{GameEventType::LIFE_GAINED, 1});

  onCollect();
  return true;
}

// ============================================================
// render – Vẽ Nấm xanh 1UP
// Nếu có Texture (1upMushroom.png) → vẽ sprite
// Fallback: vẽ hình nấm bằng Shape với mũ màu Xanh Lá
// ============================================================
void OneUpMushroom::render(sf::RenderWindow &window) const {
  if (!active || collected)
    return;

  if (sprite.getTexture() != nullptr) {
    window.draw(sprite);
    return;
  }

  // --- Fallback: vẽ nấm 1UP bằng shapes với mũ Xanh Lá ---
  // Chân nấm (thân trắng kem)
  float stemW = size.x * 0.5f;
  float stemH = size.y * 0.45f;
  sf::RectangleShape stem(sf::Vector2f(stemW, stemH));
  stem.setPosition(position.x + (size.x - stemW) / 2.f,
                   position.y + size.y - stemH);
  stem.setFillColor(sf::Color(255, 230, 200));
  stem.setOutlineColor(sf::Color(180, 150, 100));
  stem.setOutlineThickness(1.f);
  window.draw(stem);

  // Mũ nấm (Màu Xanh Lá - Green 1UP)
  float capW = size.x;
  float capH = size.y * 0.6f;
  sf::RectangleShape cap(sf::Vector2f(capW, capH));
  cap.setPosition(position.x, position.y);
  cap.setFillColor(sf::Color(30, 180, 30)); // Xanh lá tươi
  cap.setOutlineColor(sf::Color(10, 120, 10));
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
