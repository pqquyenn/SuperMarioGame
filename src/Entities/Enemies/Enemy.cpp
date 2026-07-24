#include "Entities/Enemies/Enemy.h"

Enemy::Enemy(float x, float y)
    : Entity(x, y), speed(50.f), direction(-1), isAlive(true), squished(false) {}

void Enemy::reverseDirection() {
    direction = -direction;
}

int Enemy::getDirection() const {
    return direction;
}

void Enemy::setDirection(int dir) {
    direction = dir;
}

float Enemy::getSpeed() const {
    return speed;
}

void Enemy::setSpeed(float spd) {
    speed = spd;
}

bool Enemy::isSquished() const {
    return squished;
}

bool Enemy::isEnemyAlive() const {
    return isAlive;
}

