#pragma once

#include "Entities/Enemies/Enemy.h"

class Koopa : public Enemy {
private:
  bool inShell = false;
  bool shellSpinning = false;
  sf::Vector2f size{16.f, 32.f}; // Kích thước chuẩn 1x2 tiles (16x32 pixels)

public:
  Koopa(float x = 0.f, float y = 0.f);
  ~Koopa() override = default;

  void update(float dt) override;
  void render(sf::RenderWindow& window) const override;
  void onStomped() override;

  sf::FloatRect getBounds() const override;

  bool isInShell() const;
  bool isShellSpinning() const;
};
