#include "Entities/Enemies/Goomba.h"

Goomba::Goomba(float x, float y) : Enemy(x, y) {
  speed = 40.f;
  direction = -1; // Mặc định di chuyển sang trái
}

void Goomba::update(float dt) {
  if (!active)
    return;

  if (squished) {
    velocity.x = 0.f;
    velocity.y = 0.f;
    squishTimer += dt;
    if (squishTimer >= squishDuration) {
      active = false;
      isAlive = false;
    }
  } else {
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

void Goomba::render(sf::RenderWindow &window) const {
  if (!active)
    return;

  float currentHeight = squished ? 8.f : size.y;
  float yOffset = squished ? (size.y - currentHeight) : 0.f;

  if (sprite.getTexture() != nullptr) {
    sf::Sprite drawSprite = sprite;
    drawSprite.setPosition(position.x, position.y + yOffset);
    sf::Vector2u texSize = sprite.getTexture()->getSize();
    if (texSize.x > 0 && texSize.y > 0) {
      drawSprite.setScale(size.x / static_cast<float>(texSize.x), currentHeight / static_cast<float>(texSize.y));
    }
    window.draw(drawSprite);
  } else {
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
  float currentHeight = squished ? 8.f : size.y;
  float yOffset = squished ? (size.y - currentHeight) : 0.f;
  return sf::FloatRect(position.x, position.y + yOffset, size.x, currentHeight);
}
