#pragma once

#include "Entities/Enemies/Enemy.h"

class Koopa : public Enemy {
protected:
  bool inShell = false;
  bool shellSpinning = false;
  sf::Vector2f size{16.f, 32.f}; // Kích thước chuẩn 1x2 tiles (16x32 pixels)

  // Walk animation
  float walkAnimTimer = 0.f;
  float walkAnimInterval = 0.2f; // Chuyển frame mỗi 0.2 giây
  int walkFrame = 0;             // 0 = Walk1, 1 = Walk2

public:
  Koopa(float x = 0.f, float y = 0.f);
  ~Koopa() override = default;

  void update(float dt) override;
  void render(sf::RenderWindow& window) const override;
  void onStomped() override;
  void onFireball() override;

  sf::FloatRect getBounds() const override;

  bool isInShell() const;
  bool isShellSpinning() const;
};
