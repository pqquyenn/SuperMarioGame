#include "Entities/Enemies/Enemy.h"

Enemy::Enemy(float x, float y)
    : Entity(x, y), speed(50.f), direction(-1), isAlive(true), squished(false) {
}

// ============================================================
// applyGravity – Áp dụng gia tốc trọng lực lên velocity.y
//   velocity.y tăng mỗi frame, giới hạn bởi TERMINAL_VELOCITY
//   Giúp enemy rơi xuống khi không đứng trên đất
// ============================================================
void Enemy::applyGravity(float dt) {
    velocity.y += GRAVITY * dt;
    if (velocity.y > TERMINAL_VELOCITY) {
        velocity.y = TERMINAL_VELOCITY;
    }
}

// ============================================================
// applyPhysics – Cập nhật toàn bộ vật lý mỗi frame
//   1. Set velocity.x = direction * speed (di chuyển ngang)
//   2. Áp trọng lực lên velocity.y (rơi xuống)
//   3. Tích phân: position += velocity * dt (Entity::integrateVelocity)
//   CollisionManager sẽ reset velocity.y = 0 và chỉnh position.y
//   khi enemy đứng trên đất, và gọi reverseDirection() khi chạm tường
// ============================================================
void Enemy::applyPhysics(float dt) {
    // Di chuyển ngang dựa trên direction và speed
    velocity.x = static_cast<float>(direction) * speed;

    // Áp trọng lực
    applyGravity(dt);

    // Tích phân vị trí: position += velocity * dt, đồng bộ sprite
    integrateVelocity(dt);
}

void Enemy::reverseDirection() { direction = -direction; }

int Enemy::getDirection() const { return direction; }

void Enemy::setDirection(int dir) { direction = dir; }

float Enemy::getSpeed() const { return speed; }

void Enemy::setSpeed(float spd) { speed = spd; }

bool Enemy::isSquished() const { return squished; }

bool Enemy::isEnemyAlive() const { return isAlive; }

bool Enemy::isActivated() const { return activated; }

void Enemy::setActivated(bool act) { activated = act; }
