#include "Entities/Character.h"
#include "Entities/Enemies/Enemy.h"
#include "Entities/Enemies/DragonLugia.h"
#include "Core/SoundManager.h"
#include "PlayerEffects/DamageInvincibilityEffect.h"
#include "PlayerEffects/PlayerEffect.h"
#include "PlayerStates/PlayerState.h"
#include "PlayerStates/SmallState.h"
#include "PlayerStates/SuperState.h"

#include <algorithm>
#include <cmath>
#include <utility>

Character::Character(float x, float y)
    : Character{
          x,
          y,
          CharacterProfile{},
          makeClassicPlayerAnimationProfile(9, 25, 153)
      } {}

Character::Character(
    float x,
    float y,
    const CharacterProfile& characterProfile,
    PlayerAnimationProfile playerAnimationProfile
)
    : Entity{x, y},
      profile{characterProfile},
      releaseDeceleration{characterProfile.groundDeceleration},
      animationProfile{std::move(playerAnimationProfile)},
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

    if (dying) {
        deathTimer += dt;

        // Classic Mario Death Sequence:
        // 1. Freeze for 0.4s on the death frame (18, 9, 16, 16)
        // 2. Hop upward at 0.4s
        // 3. Fall downward with gravity ignoring collisions
        if (deathTimer >= 0.4f && !deathHopStarted) {
            deathHopStarted = true;
            velocity.y = -360.f;
        }

        if (deathHopStarted) {
            velocity.y += 980.f * dt;
            if (velocity.y > 600.f) {
                velocity.y = 600.f;
            }
            position.y += velocity.y * dt;
            syncSpritePosition();
        }

        updatePlayerAnimation(dt);

        if (deathTimer >= 2.6f) {
            dying = false;
            setActive(false);
        }
        return;
    }

    if (shootTimer > 0.f) {
        shootTimer -= dt;
    }

    updatePlayerEffects(dt);

    if (isActive()) {
        updateCharacterPhysics(dt);
        updatePlayerAnimation(dt);
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

    sf::Sprite displaySprite = sprite;
    if (crouching && currentState) {
        const float standingHeight =
            profile.smallBodyHeight * currentState->getHeightMultiplier();
        displaySprite.move(0.f, -(standingHeight - collisionSize.y));
    }
    window.draw(displaySprite);
}

void Character::onCollision(
    Entity& other,
    const sf::FloatRect& overlap) {
    if (!isActive() || dying) {
        return;
    }

    auto* enemy = dynamic_cast<Enemy*>(&other);
    if (!enemy || !enemy->isActive() || enemy->isSquished()) {
        return;
    }

    // Star power is a player-side interaction rule. The generic collision
    // layer only reports the overlap and does not know this ability.
    if (defeatsEnemiesOnContact()) {
        enemy->onFireball();
        notify(GameEvent::enemyDefeated(enemy->getScoreValue()));
        return;
    }

    const sf::FloatRect characterBounds = getBounds();
    const sf::FloatRect enemyBounds = enemy->getBounds();
    const bool isStomp =
        velocity.y > 0.f &&
        (characterBounds.top + characterBounds.height - overlap.height <=
         enemyBounds.top + 8.f);

    auto* dragon = dynamic_cast<DragonLugia*>(enemy);
    if (dragon) {
        if (isStomp) {
            // Khi Mario nhảy lên đầu boss: Mario nảy lên an toàn, không bị mất máu và boss không bị stomp
            setPosition(position.x, enemyBounds.top - characterBounds.height);
            setVelocity(sf::Vector2f(velocity.x, -250.f));
            return;
        }
        takeDamage();
        return;
    }

    if (!enemy->canBeStomped()) {
        takeDamage();
        return;
    }

    if (isStomp) {
        setPosition(position.x, enemyBounds.top - characterBounds.height);
        enemy->onStomped();
        SoundManager::getInstance().playSound("stomp");
        notify(GameEvent::enemyDefeated(enemy->getScoreValue()));
        setVelocity(sf::Vector2f(velocity.x, -250.f));
        return;
    }

    takeDamage();
}

void Character::beginTileCollision() {
    setGrounded(false);
}

void Character::onLanded() {
    setGrounded(true);
}

void Character::moveLeft(float dt) {
    horizontalInputThisFrame = true;
    facingRight = false;

    const float baseSpeed = crouching
        ? profile.crawlSpeed
        : (running ? profile.runSpeed : profile.walkSpeed);
    const float maximumSpeed =
        baseSpeed * getMoveSpeedMultiplier() * horizontalMovementScale;
    releaseDeceleration = crouching
        ? profile.crawlDeceleration
        : profile.groundDeceleration;

    velocity.x = std::max(
        velocity.x - profile.moveAcceleration * horizontalMovementScale * dt,
        -maximumSpeed
    );
}

void Character::moveRight(float dt) {
    horizontalInputThisFrame = true;
    facingRight = true;

    const float baseSpeed = crouching
        ? profile.crawlSpeed
        : (running ? profile.runSpeed : profile.walkSpeed);
    const float maximumSpeed =
        baseSpeed * getMoveSpeedMultiplier() * horizontalMovementScale;
    releaseDeceleration = crouching
        ? profile.crawlDeceleration
        : profile.groundDeceleration;

    velocity.x = std::min(
        velocity.x + profile.moveAcceleration * horizontalMovementScale * dt,
        maximumSpeed
    );
}

void Character::jump() {
    if (!profile.canJump || !grounded || crouching) {
        return;
    }

    velocity.y = -profile.jumpForce * getJumpForceMultiplier();
    grounded = false;
    jumpHoldTime = 0.f;
    SoundManager::getInstance().playSound("jump");
}

void Character::setJumpHeld(bool status) {
    jumpHeldThisFrame = profile.canJump && status;
}

void Character::shootFireball() {
    if (crouching || !hasAbility(PlayerAbility::ShootFireballs)) {
        return;
    }

    shootTimer = 0.15f;
    SoundManager::getInstance().playSound("fireball");
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
    const float deceleration =
        releaseDeceleration * horizontalMovementScale * dt;

    if (velocity.x > 0.f) {
        velocity.x = std::max(0.f, velocity.x - deceleration);
    } else if (velocity.x < 0.f) {
        velocity.x = std::min(0.f, velocity.x + deceleration);
    }
}

void Character::applyGravity(float dt) {
    float gravityMultiplier = 1.f;

    if (jumpHeldThisFrame &&
        velocity.y < 0.f &&
        jumpHoldTime < profile.maxJumpHoldTime) {
        // Holding jump supplies its strongest lift just after takeoff, then
        // smoothly fades back to normal gravity. This keeps the initial jump
        // impulse and held-jump assistance from feeling like two separate
        // forces while still allowing variable jump height.
        const float holdProgress = std::clamp(
            jumpHoldTime / profile.maxJumpHoldTime,
            0.f,
            1.f
        );
        const float easedProgress =
            holdProgress * holdProgress * (3.f - 2.f * holdProgress);
        gravityMultiplier =
            profile.jumpHoldGravityMultiplier +
            (1.f - profile.jumpHoldGravityMultiplier) * easedProgress;
        jumpHoldTime = std::min(
            profile.maxJumpHoldTime,
            jumpHoldTime + dt
        );
    } else if (!jumpHeldThisFrame && velocity.y < 0.f) {
        // Early release produces a shorter jump without abruptly changing
        // the velocity that was accumulated on previous frames.
        gravityMultiplier = profile.jumpReleaseGravityMultiplier;
    }

    velocity.y = std::min(
        velocity.y + profile.gravity * gravityMultiplier * dt,
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

PlayerMotion Character::choosePlayerMotion() const {
    constexpr float motionThreshold = 1.f;

    if (!isActive() || dying) {
        return PlayerMotion::Dead;
    }

    if (crouching) {
        return PlayerMotion::Crouching;
    }

    if (shootTimer > 0.f) {
        return PlayerMotion::Shooting;
    }

    // The current sheet has one airborne pose, shared by rising and falling.
    if (!grounded) {
        return PlayerMotion::Jumping;
    }

    if (std::abs(velocity.x) < motionThreshold) {
        return PlayerMotion::Idle;
    }

    const bool velocityFacesRight = velocity.x > 0.f;
    if (velocityFacesRight != facingRight) {
        return PlayerMotion::Sliding;
    }

    return PlayerMotion::Running;
}

void Character::updatePlayerAnimation(float dt) {
    const AnimationClip* clip = animationProfile.findClip(
        currentFormName,
        choosePlayerMotion()
    );

    if (!clip) {
        animator.stop();
        return;
    }

    animator.play(*clip);
    animator.update(dt);

    const AnimationFrame* frame = animator.getCurrentFrame();
    if (!frame) {
        return;
    }

    if (frame->texture) {
        sprite.setTexture(*frame->texture);
    }
    sprite.setTextureRect(frame->textureRect);

    // Keep the sprite anchored to the collision body's left edge while using
    // negative X scale to reuse right-facing frames for left-facing movement.
    const float frameWidth = static_cast<float>(
        std::abs(frame->textureRect.width)
    );
    sprite.setOrigin(facingRight ? 0.f : frameWidth, 0.f);
    sprite.setScale(facingRight ? 1.f : -1.f, 1.f);

    // Dynamic Star Color Tinting (Rainbow color cycling every 0.06s)
    if (isStarInvincible() && !dying) {
        starColorTimer += dt;
        static const sf::Color starColors[] = {
            sf::Color(255, 255, 255), // Trắng sáng nguyên bản
            sf::Color(255, 225, 60),  // Vàng Gold phát sáng
            sf::Color(90, 255, 120),  // Xanh lá neon
            sf::Color(255, 100, 70),  // Đỏ cam rực lửa
            sf::Color(80, 230, 255),  // Xanh Cyan
            sf::Color(255, 130, 240)  // Tím hồng
        };
        constexpr int numColors = sizeof(starColors) / sizeof(starColors[0]);
        const int colorIdx = static_cast<int>(starColorTimer / 0.06f) % numColors;
        sprite.setColor(starColors[colorIdx]);
    } else {
        starColorTimer = 0.f;
        sprite.setColor(sf::Color::White);
    }
}

void Character::setPlayerAnimationProfile(
    PlayerAnimationProfile playerAnimationProfile
) {
    animator.stop();
    animationProfile = std::move(playerAnimationProfile);
    updatePlayerAnimation(0.f);
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

    if (currentState->getName() == "Fire" && newState->getName() == "Super") {
        // Keep Fire form when collecting a Super Mushroom
    } else if (currentState->getFormTier() == FormTier::Small &&
        newState->getFormTier() == FormTier::Powered) {
        changeState(std::make_unique<SuperState>());
    } else {
        changeState(std::move(newState));
    }

    SoundManager::getInstance().playSound("powerupcollect");
    notify(GameEvent::powerupCollected());
    return true;
}

bool Character::hasAbility(PlayerAbility ability) const {
    return currentState && currentState->hasAbility(ability);
}

void Character::takeDamage() {
    applyDamage(false);
}

void Character::takeDamageIgnoringProtection() {
    applyDamage(true);
}

void Character::applyDamage(bool ignoreProtection) {
    if (!isActive() || !currentState) {
        return;
    }

    if (!ignoreProtection) {
        for (auto& effect : activeEffects) {
            if (effect->tryAbsorbDamage(*this)) {
                return;
            }
        }
    }

    notify(GameEvent::playerHit());

    std::unique_ptr<PlayerState> nextState = currentState->takeDamage();
    if (!nextState) {
        die(DeathCause::NormalDamage);
        return;
    }

    SoundManager::getInstance().playSound("shrink");
    changeState(std::move(nextState));

    addEffect(std::make_unique<DamageInvincibilityEffect>(
        profile.damageInvincibilityDuration
    ));
}

void Character::die(DeathCause cause) {
    if (dying || !isActive()) {
        return;
    }

    dying = true;
    deathTimer = 0.f;
    deathHopStarted = false;

    SoundManager::getInstance().playSound("death");

    // Reset to Small Mario form so the death frame uses Small Mario's (18, 9, 16, 16)
    currentFormName = "Small";
    changeState(std::make_unique<SmallState>());

    clearEffects();
    starColorTimer = 0.f;
    sprite.setColor(sf::Color::White);
    setVelocity(0.f, 0.f);
    grounded = false;
    running = false;
    horizontalInputThisFrame = false;
    jumpHeldThisFrame = false;
    setActive(true);

    notify(GameEvent::playerDied(static_cast<int>(cause)));
}

void Character::respawn(float x, float y) {
    dying = false;
    deathTimer = 0.f;
    deathHopStarted = false;
    clearEffects();
    starColorTimer = 0.f;
    sprite.setColor(sf::Color::White);
    setVelocity(0.f, 0.f);

    grounded = false;
    running = false;
    horizontalInputThisFrame = false;
    jumpHeldThisFrame = false;
    jumpHoldTime = 0.f;
    setActive(true);

    changeState(std::make_unique<SmallState>());
    setPosition(x, y);
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

bool Character::isStarInvincible() const {
    return defeatsEnemiesOnContact();
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
    running = profile.canRun && !crouching && status;
}

bool Character::isRunning() const {
    return running;
}

void Character::setHorizontalMovementScale(float scale) {
    horizontalMovementScale = std::max(0.f, scale);
}

float Character::getHorizontalMovementScale() const {
    return horizontalMovementScale;
}

void Character::setCrouchRequested(bool status) {
    crouchRequested = status;

    if (!status || crouching || !isActive() || dying || !grounded ||
        !currentState || currentState->getFormTier() != FormTier::Powered) {
        return;
    }

    const float oldBottom = position.y + collisionSize.y;
    collisionSize.y = profile.smallBodyHeight;
    position.y = oldBottom - collisionSize.y;
    crouching = true;
    running = false;
    jumpHeldThisFrame = false;
    syncSpritePosition();
    updatePlayerAnimation(0.f);
}

bool Character::isCrouchRequested() const {
    return crouchRequested;
}

bool Character::isCrouching() const {
    return crouching;
}

sf::FloatRect Character::getStandingBounds() const {
    if (!currentState) {
        return getBounds();
    }

    const float standingHeight =
        profile.smallBodyHeight * currentState->getHeightMultiplier();
    const float bottom = position.y + collisionSize.y;
    return sf::FloatRect{
        position.x,
        bottom - standingHeight,
        profile.bodyWidth,
        standingHeight
    };
}

sf::FloatRect Character::getStandingHeadroomBounds() const {
    if (!crouching) {
        return sf::FloatRect{position.x, position.y, collisionSize.x, 0.f};
    }

    const sf::FloatRect standingBounds = getStandingBounds();
    const float addedHeight = std::max(0.f, position.y - standingBounds.top);
    return sf::FloatRect{
        standingBounds.left,
        standingBounds.top,
        standingBounds.width,
        addedHeight
    };
}

void Character::resolveCrouchState(bool headroomBlocked) {
    if (!crouching || headroomBlocked || (crouchRequested && grounded)) {
        return;
    }

    standUp();
}

void Character::standUp() {
    if (!crouching || !currentState) {
        return;
    }

    const float oldBottom = position.y + collisionSize.y;
    collisionSize.x = profile.bodyWidth;
    collisionSize.y =
        profile.smallBodyHeight * currentState->getHeightMultiplier();
    position.y = oldBottom - collisionSize.y;
    crouching = false;
    syncSpritePosition();
    updatePlayerAnimation(0.f);
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

std::string_view Character::getCharacterType() const {
    return "Character";
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

    const bool remainsCrouched = crouching && safeMultiplier > 1.f;
    collisionSize.x = profile.bodyWidth;
    collisionSize.y = remainsCrouched
        ? profile.smallBodyHeight
        : profile.smallBodyHeight * safeMultiplier;
    crouching = remainsCrouched;
    currentFormName = std::string{formName};

    position.y = oldBottom - collisionSize.y;
    syncSpritePosition();
    updatePlayerAnimation(0.f);
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

bool Character::isDying() const {
    return dying;
}
