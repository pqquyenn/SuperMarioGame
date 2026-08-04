#pragma once

#include "Entities/Enemies/Enemy.h"

// ============================================================
// Koopa – Rùa 3 trạng thái: Walking → Shell → Spinning
// Hành vi vật lý: di chuyển ngang + trọng lực (velocity-based)
// ============================================================
class Koopa : public Enemy {
private:
  bool inShell = false;
  bool shellSpinning = false;
  sf::Vector2f size{32.f, 48.f}; // Koopa cao hơn Goomba (32x48)

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
