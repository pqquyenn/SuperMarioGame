#pragma once

#include "Entities/Entity.h"

// ============================================================
// Enemy – Lớp cơ sở cho tất cả kẻ địch trong game
// Kế thừa từ Entity. Cung cấp:
//   - speed, direction: di chuyển ngang
//   - GRAVITY, TERMINAL_VELOCITY: vật lý trọng lực
//   - applyGravity(dt), applyPhysics(dt): helper cập nhật vật lý
//   - isAlive, squished: trạng thái sống/chết
// Subclass: Goomba, Koopa, PiranhaPlant
// ============================================================
class Enemy : public Entity {
protected:
  float speed{50.f};
  int direction{-1}; // -1: Left, 1: Right
  bool isAlive{true};
  bool squished{false};
  bool activated{false};
  int scoreValue{100};

  // ---- Vật lý trọng lực ----
  static constexpr float GRAVITY = 980.f;          // Gia tốc trọng lực (px/s²)
  static constexpr float TERMINAL_VELOCITY = 500.f; // Vận tốc rơi tối đa (px/s)

  // Helper: áp dụng trọng lực lên velocity.y
  void applyGravity(float dt);

  // Helper: set velocity.x từ direction * speed, áp gravity, tích phân vị trí
  void applyPhysics(float dt);

public:
  Enemy(float x = 0.f, float y = 0.f);
  virtual ~Enemy() = default;

  virtual void onStomped() = 0;
  virtual bool canBeStomped() const { return true; }
  virtual bool bouncesPlayerOnStompAttempt() const { return false; }
  virtual void onFireball();
  virtual void onFellIntoVoid();
  virtual void reverseDirection();
  void onWallCollision() override;

  int getDirection() const;
  void setDirection(int dir);

  float getSpeed() const;
  void setSpeed(float spd);

  int getScoreValue() const;
  void setScoreValue(int score);

  bool isSquished() const;
  bool isEnemyAlive() const;

  bool isActivated() const;
  void setActivated(bool act);
};
