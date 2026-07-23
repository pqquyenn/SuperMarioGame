#include "Entities/Character.h"

void Character::moveLeft(float dt) {
    velocity.x = -moveSpeed;
}

void Character::moveRight(float dt) {
    velocity.x = moveSpeed;
}

void Character::jump() {
    if (isGrounded) {
        velocity.y = -jumpForce;
        isGrounded = false;
    }
}

void Character::shootFireball() {
    // Base shoot action
}
