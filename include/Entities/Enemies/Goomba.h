#pragma once

#include "Entities/Enemies/Enemy.h"

class Goomba : public Enemy {
private:
  float squishDuration{0.5f}; // Thời gian hiển thị dạng bẹp (0.5s)
  float squishTimer{0.f};
  sf::Vector2f size{16.f, 16.f}; // Kích thước chuẩn 1 tile (16x16 pixels)

public:
  Goomba(float x = 0.f, float y = 0.f);
  ~Goomba() override = default;

  void update(float dt) override;
  void render(sf::RenderWindow &window) const override;
  void onStomped() override;

  float getSquishTimer() const;
  sf::FloatRect getBounds() const override;
};
