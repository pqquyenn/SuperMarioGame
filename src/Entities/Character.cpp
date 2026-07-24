#include "Entities/Character.h"
#include "PlayerEffects/DamageInvincibilityEffect.h"
#include "PlayerEffects/PlayerEffect.h"
#include "PlayerStates/PlayerState.h"
#include "PlayerStates/SmallState.h"
#include "PlayerStates/SuperState.h"

#include <algorithm>
#include <utility>

Character::Character(float x, float y)
    : Character{x, y, CharacterProfile{}} {}

Character::Character(
    float x,
    float y,
    const CharacterProfile& characterProfile
)
    : Entity{x, y},
      profile{characterProfile},
      collisionSize{
          characterProfile.bodyWidth,
          characterProfile.smallBodyHeight
      } {
    changeState(std::make_unique<SmallState>());
}

Character::~Character() = default;

void Character::update(float dt) {
    if (!isActive()) {
        return;
    }

    updatePlayerEffects(dt);

    if (isActive()) {
        updateCharacterPhysics(dt);
    }
}

void Character::render(sf::RenderWindow& window) const {
    if (!isActive()) {
        return;
    }

    for (const auto& effect : activeEffects) {
        if (!effect->isCharacterVisible()) {
            return;
        }
    }

    Entity::render(window);
}

void Character::moveLeft(float dt) {
    horizontalInputThisFrame = true;
    facingRight = false;

    const float baseSpeed = running ? profile.runSpeed : profile.walkSpeed;
    const float maximumSpeed = baseSpeed * getMoveSpeedMultiplier();

    velocity.x = std::max(
        velocity.x - profile.moveAcceleration * dt,
        -maximumSpeed
    );
}

void Character::moveRight(float dt) {
    horizontalInputThisFrame = true;
    facingRight = true;

    const float baseSpeed = running ? profile.runSpeed : profile.walkSpeed;
    const float maximumSpeed = baseSpeed * getMoveSpeedMultiplier();

    velocity.x = std::min(
        velocity.x + profile.moveAcceleration * dt,
        maximumSpeed
    );
}

void Character::jump() {
    if (!profile.canJump) {
        return;
    }

    jumpHeldThisFrame = true;

    if (grounded) {
        velocity.y = -profile.jumpForce * getJumpForceMultiplier();
        grounded = false;
        jumpHoldTime = 0.f;
    }
}

void Character::shootFireball() {
    if (!hasAbility(PlayerAbility::ShootFireballs)) {
        return;
    }

    requestProjectile(ProjectileType::Fireball);
}

void Character::useSpecialAbility() {
    if (!isActive() ||
        !profile.canUseSpecialAbility ||
        !currentState) {
        return;
    }

    currentState->useSpecialAbility(*this);
}

void Character::applyHorizontalDeceleration(float dt) {
    const float deceleration = profile.groundDeceleration * dt;

    if (velocity.x > 0.f) {
        velocity.x = std::max(0.f, velocity.x - deceleration);
    } else if (velocity.x < 0.f) {
        velocity.x = std::min(0.f, velocity.x + deceleration);
    }
}

void Character::applyGravity(float dt) {
    if (jumpHeldThisFrame &&
        velocity.y < 0.f &&
        jumpHoldTime < profile.maxJumpHoldTime) {
        velocity.y -= profile.jumpHoldAcceleration *
                      getJumpForceMultiplier() * dt;
        jumpHoldTime += dt;
    } else if (!jumpHeldThisFrame &&
               velocity.y < 0.f &&
               jumpHoldTime < profile.maxJumpHoldTime) {
        // Releasing jump early shortens the jump.
        velocity.y *= 0.5f;
        jumpHoldTime = profile.maxJumpHoldTime;
    }

    velocity.y = std::min(
        velocity.y + profile.gravity * dt,
        profile.maxFallSpeed
    );
}

void Character::updateCharacterPhysics(float dt) {
    if (!horizontalInputThisFrame && grounded) {
        applyHorizontalDeceleration(dt);
    }

    applyGravity(dt);
    integrateVelocity(dt);

    horizontalInputThisFrame = false;
    jumpHeldThisFrame = false;
}

void Character::updatePlayerEffects(float dt) {
    updatingEffects = true;

    for (auto& effect : activeEffects) {
        effect->update(*this, dt);
    }

    updatingEffects = false;

    auto effect = activeEffects.begin();
    while (effect != activeEffects.end()) {
        if ((*effect)->hasExpired()) {
            (*effect)->onRemove(*this);
            effect = activeEffects.erase(effect);
        } else {
            ++effect;
        }
    }

    if (clearEffectsRequested) {
        clearEffectsRequested = false;
        clearEffects();
        pendingEffects.clear();
        return;
    }

    auto queuedEffects = std::move(pendingEffects);
    pendingEffects.clear();

    for (auto& queuedEffect : queuedEffects) {
        insertEffect(std::move(queuedEffect));
    }
}

void Character::changeState(std::unique_ptr<PlayerState> newState) {
    if (!newState) {
        return;
    }

    if (currentState) {
        currentState->onExit(*this);
    }

    currentState = std::move(newState);
    currentState->onEnter(*this);
}

bool Character::receivePowerUp(std::unique_ptr<PlayerState> newState) {
    if (!isActive() ||
        !profile.canReceivePowerUps ||
        !currentState ||
        !newState) {
        return false;
    }

    if (currentState->getFormTier() == FormTier::Small &&
        newState->getFormTier() == FormTier::Powered) {
        changeState(std::make_unique<SuperState>());
    } else {
        changeState(std::move(newState));
    }

    notify(GameEvent{GameEventType::POWERUP_COLLECTED});
    return true;
}

bool Character::hasAbility(PlayerAbility ability) const {
    return currentState && currentState->hasAbility(ability);
}

void Character::takeDamage() {
    if (!isActive() || !currentState) {
        return;
    }

    for (auto& effect : activeEffects) {
        if (effect->tryAbsorbDamage(*this)) {
            return;
        }
    }

    notify(GameEvent{GameEventType::PLAYER_HIT});

    std::unique_ptr<PlayerState> nextState = currentState->takeDamage();
    if (!nextState) {
        die(DeathCause::NormalDamage);
        return;
    }

    changeState(std::move(nextState));

    addEffect(std::make_unique<DamageInvincibilityEffect>(
        profile.damageInvincibilityDuration
    ));
}

void Character::die(DeathCause cause) {
    if (!isActive()) {
        return;
    }

    clearEffects();
    setVelocity(0.f, 0.f);
    grounded = false;
    running = false;
    horizontalInputThisFrame = false;
    jumpHeldThisFrame = false;
    setActive(false);

    notify(GameEvent{
        GameEventType::PLAYER_DIED,
        static_cast<int>(cause)
    });
}

void Character::respawn(float x, float y) {
    clearEffects();
    setPosition(x, y);
    setVelocity(0.f, 0.f);

    grounded = false;
    running = false;
    horizontalInputThisFrame = false;
    jumpHeldThisFrame = false;
    jumpHoldTime = 0.f;
    setActive(true);

    changeState(std::make_unique<SmallState>());
}

bool Character::addEffect(std::unique_ptr<PlayerEffect> effect) {
    if (!isActive() || !effect) {
        return false;
    }

    if (updatingEffects) {
        pendingEffects.push_back(std::move(effect));
        return true;
    }

    insertEffect(std::move(effect));
    return true;
}

void Character::insertEffect(std::unique_ptr<PlayerEffect> effect) {
    effect->onApply(*this);
    activeEffects.push_back(std::move(effect));

    std::stable_sort(
        activeEffects.begin(),
        activeEffects.end(),
        [](const auto& left, const auto& right) {
            return left->getDamagePriority() >
                   right->getDamagePriority();
        }
    );
}

void Character::clearEffects() {
    if (updatingEffects) {
        clearEffectsRequested = true;
        pendingEffects.clear();
        return;
    }

    for (auto& effect : activeEffects) {
        effect->onRemove(*this);
    }

    activeEffects.clear();
    pendingEffects.clear();
}

bool Character::defeatsEnemiesOnContact() const {
    return std::any_of(
        activeEffects.begin(),
        activeEffects.end(),
        [](const auto& effect) {
            return effect->defeatsEnemiesOnContact();
        }
    );
}

float Character::getMoveSpeedMultiplier() const {
    float multiplier = 1.f;

    for (const auto& effect : activeEffects) {
        multiplier *= effect->getMoveSpeedMultiplier();
    }

    return multiplier;
}

float Character::getJumpForceMultiplier() const {
    float multiplier = 1.f;

    for (const auto& effect : activeEffects) {
        multiplier *= effect->getJumpForceMultiplier();
    }

    return multiplier;
}

void Character::setRunning(bool status) {
    running = profile.canRun && status;
}

bool Character::isRunning() const {
    return running;
}

void Character::setGrounded(bool status) {
    grounded = status;

    if (grounded) {
        jumpHoldTime = 0.f;

        if (velocity.y > 0.f) {
            velocity.y = 0.f;
        }
    }
}

bool Character::isGrounded() const {
    return grounded;
}

bool Character::isFacingRight() const {
    return facingRight;
}

sf::FloatRect Character::getBounds() const {
    return sf::FloatRect{
        position.x,
        position.y,
        collisionSize.x,
        collisionSize.y
    };
}

void Character::applyForm(
    std::string_view formName,
    float heightMultiplier
) {
    const float oldBottom = position.y + collisionSize.y;
    const float safeMultiplier = std::max(0.1f, heightMultiplier);

    collisionSize.x = profile.bodyWidth;
    collisionSize.y = profile.smallBodyHeight * safeMultiplier;
    currentFormName = std::string{formName};

    position.y = oldBottom - collisionSize.y;
    syncSpritePosition();
}

std::string_view Character::getCurrentFormName() const {
    return currentFormName;
}

void Character::setProjectileRequestHandler(
    std::function<void(const ProjectileRequest&)> handler
) {
    projectileRequestHandler = std::move(handler);
}

void Character::requestProjectile(ProjectileType type) {
    if (!isActive() || !projectileRequestHandler) {
        return;
    }

    const sf::FloatRect bounds = getBounds();
    const float projectileX =
        facingRight ? bounds.left + bounds.width : bounds.left;

    projectileRequestHandler(ProjectileRequest{
        type,
        sf::Vector2f{
            projectileX,
            bounds.top + bounds.height * 0.5f
        },
        facingRight
    });
}
