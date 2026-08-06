#include "Entities/Enemies/Koopa.h"
#include "Core/AssetManager.h"

Koopa::Koopa(float x, float y) : Enemy(x, y) { speed = 50.f; }

void Koopa::update(float dt) {
  if (!active)
    return;

  applyPhysics(dt);

  // Walk animation: chuyển đổi giữa Walk1 và Walk2 khi đang đi (không ở trong shell)
  if (!inShell) {
    walkAnimTimer += dt;
    if (walkAnimTimer >= walkAnimInterval) {
      walkAnimTimer -= walkAnimInterval;
      walkFrame = 1 - walkFrame; // Toggle 0 ↔ 1
      auto& assets = AssetManager::getInstance();
      if (walkFrame == 0) {
        setTexture(assets.getTexture("Koopa"));        // Walk1
      } else {
        setTexture(assets.getTexture("Koopa_Walk2"));  // Walk2
      }
    }
  }
}

void Koopa::onStomped() {
  if (!inShell) {
    inShell = true;
    speed = 0.f;
    setTexture(AssetManager::getInstance().getTexture("Koopa_Shell"));
  } else if (!shellSpinning) {
    shellSpinning = true;
    speed = 300.f;
  } else {
    active = false;
  }
}

void Koopa::render(sf::RenderWindow &window) const {
  if (!active)
    return;

  float currentHeight = inShell ? 16.f : size.y;
  float yOffset = size.y - currentHeight;

  if (sprite.getTexture() != nullptr) {
    sf::Sprite drawSprite = sprite;
    drawSprite.setPosition(position.x, position.y + yOffset);
    sf::Vector2u texSize = sprite.getTexture()->getSize();
    if (texSize.x > 0 && texSize.y > 0) {
      drawSprite.setScale(size.x / static_cast<float>(texSize.x),
                          currentHeight / static_cast<float>(texSize.y));
    }
    window.draw(drawSprite);
    return;
  }

  if (inShell) {
    sf::RectangleShape shell(sf::Vector2f(size.x, currentHeight));
    shell.setPosition(position.x, position.y + yOffset);
    shell.setFillColor(shellSpinning ? sf::Color(0, 200, 0)
                                     : sf::Color(0, 140, 0));
    shell.setOutlineColor(sf::Color(0, 80, 0));
    shell.setOutlineThickness(1.f);
    window.draw(shell);
  } else {
    float shellH = size.y * 0.65f;
    sf::RectangleShape shell(sf::Vector2f(size.x, shellH));
    shell.setPosition(position.x, position.y + size.y - shellH);
    shell.setFillColor(sf::Color(0, 160, 0));
    shell.setOutlineColor(sf::Color(0, 80, 0));
    shell.setOutlineThickness(1.f);
    window.draw(shell);

    float headR = size.x * 0.35f;
    sf::CircleShape head(headR);
    head.setPosition(position.x + size.x / 2.f - headR + (direction * 2.f),
                     position.y);
    head.setFillColor(sf::Color(255, 220, 100));
    window.draw(head);
  }
}

sf::FloatRect Koopa::getBounds() const {
  float currentHeight = inShell ? 16.f : size.y;
  float yOffset = size.y - currentHeight;
  return sf::FloatRect(position.x, position.y + yOffset, size.x, currentHeight);
}

bool Koopa::isInShell() const { return inShell; }

bool Koopa::isShellSpinning() const { return shellSpinning; }
