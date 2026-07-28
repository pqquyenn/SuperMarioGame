#pragma once

#include "Entities/Enemies/Enemy.h"

class Goomba : public Enemy {
private:
  float squishDuration{
      0.5f}; // Thời gian hiển thị dạng bẹp trước khi de-active (0.5s)
  float squishTimer{0.f};
  sf::Vector2f size{32.f, 32.f}; // Kích thước mặc định

public:
  Goomba(float x = 0.f, float y = 0.f);
  ~Goomba() override = default;

  void update(float dt) override;
  void render(sf::RenderWindow &window) const override;
  void onStomped() override;

  float getSquishTimer() const;
  sf::FloatRect getBounds() const override;
};
