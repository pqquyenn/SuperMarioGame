#pragma once

#include "Animation/SpriteAnimator.h"
#include "Entities/Entity.h"
#include "Entities/PlayerAnimation.h"
#include "Observer/Subject.h"
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class PlayerEffect;
class PlayerState;
enum class PlayerAbility;
enum class FormTier;

enum class DeathCause {
    NormalDamage,
    Void,
    TimeOut,
    Crushed,
    PvP
};

enum class ProjectileType {
    Fireball,
    Iceball
};

struct ProjectileRequest {
    ProjectileType type;
    sf::Vector2f position;
    bool facingRight;
};

struct CharacterProfile {
    float moveAcceleration{900.f};
    float walkSpeed{150.f};
    float crawlSpeed{70.f};
    float runSpeed{230.f};
    float groundDeceleration{1000.f};
    float crawlDeceleration{1000.f};

    float gravity{980.f};
    float maxFallSpeed{900.f};
    float jumpForce{350.f};
    // Holding jump reduces gravity briefly; releasing early increases it.
    // Both multipliers preserve a continuous vertical-velocity curve.
    float jumpHoldGravityMultiplier{0.45f};
    float jumpReleaseGravityMultiplier{2.5f};
    float maxJumpHoldTime{0.18f};
    float damageInvincibilityDuration{2.f};

    // Collision body matches the 16x16 Small player sprite and tile grid.
    // Powered forms retain this width and scale the height to 16x32.
    float bodyWidth{16.f};
    float smallBodyHeight{16.f};

    bool canJump{true};
    bool canRun{true};
    bool canReceivePowerUps{true};
    bool canUseSpecialAbility{true};
};

class Character : public Entity, public Subject {
protected:
    CharacterProfile profile;

    float jumpHoldTime{0.f}; // Lift time consumed during the current jump.

    bool grounded{false};                 // True while standing on a solid surface.
    bool running{false};                  // Selects running speed instead of walking speed.
    bool facingRight{true};               // True for right-facing; false for left-facing.
    bool horizontalInputThisFrame{false}; // Prevents friction while movement input is active.
    bool jumpHeldThisFrame{false};        // True when jump input was received this frame.
    float shootTimer{0.f};                // Timer for shoot pose animation.
    float releaseDeceleration{1000.f};    // Braking rate selected by the last movement mode.
    float horizontalMovementScale{1.f};   // Ruleset multiplier; 1 keeps solo physics unchanged.

    void applyHorizontalDeceleration(float dt);
    void applyGravity(float dt);
    void updateCharacterPhysics(float dt);
    void updatePlayerEffects(float dt);

    float getMoveSpeedMultiplier() const;
    float getJumpForceMultiplier() const;
    void setPlayerAnimationProfile(PlayerAnimationProfile playerAnimationProfile);

    explicit Character(float x = 0.f, float y = 0.f);
    Character(
        float x,
        float y,
        const CharacterProfile& characterProfile,
        PlayerAnimationProfile playerAnimationProfile
    );

public:
    ~Character() override;

    void update(float dt) override;
    void render(sf::RenderWindow& window) const override;

    virtual void moveLeft(float dt);
    virtual void moveRight(float dt);
    virtual void jump();
    void setJumpHeld(bool status);
    virtual void shootFireball();
    void useSpecialAbility();

    bool receivePowerUp(std::unique_ptr<PlayerState> state);
    bool hasAbility(PlayerAbility ability) const;
    void takeDamage();
    // Applies one damage transition even when a shield or invincibility effect
    // is active. Used by deterministic tests and admin controls.
    void takeDamageIgnoringProtection();
    void die(DeathCause cause);
    void respawn(float x, float y);

    bool addEffect(std::unique_ptr<PlayerEffect> effect);
    void clearEffects();
    bool defeatsEnemiesOnContact() const;
    bool isStarInvincible() const;

    void setRunning(bool status);
    bool isRunning() const;
    void setHorizontalMovementScale(float scale);
    float getHorizontalMovementScale() const;

    // Powered forms can crouch while grounded. Crouching keeps the feet fixed,
    // uses the Small-height collision body, and remains active until gameplay
    // confirms that the additional standing headroom is clear.
    void setCrouchRequested(bool status);
    bool isCrouchRequested() const;
    bool isCrouching() const;
    sf::FloatRect getStandingBounds() const;
    sf::FloatRect getStandingHeadroomBounds() const;
    // Called after all ground contacts are known for the frame. The physics
    // layer supplies only whether terrain blocks the standing headroom.
    void resolveCrouchState(bool headroomBlocked);
    void standUp();

    void setGrounded(bool status);
    bool isGrounded() const;

    bool isFacingRight() const;
    bool isDying() const;
    sf::FloatRect getBounds() const override;
    virtual std::string_view getCharacterType() const;

    void applyForm(std::string_view formName, float heightMultiplier);
    std::string_view getCurrentFormName() const;

    void setProjectileRequestHandler(
        std::function<void(const ProjectileRequest&)> handler
    );
    void requestProjectile(ProjectileType type);

private:
    bool dying{false};
    float deathTimer{0.f};
    bool deathHopStarted{false};
    float starColorTimer{0.f};
    bool crouchRequested{false};
    bool crouching{false};

    std::unique_ptr<PlayerState> currentState;
    std::vector<std::unique_ptr<PlayerEffect>> activeEffects;
    std::vector<std::unique_ptr<PlayerEffect>> pendingEffects;

    PlayerAnimationProfile animationProfile;
    SpriteAnimator animator;
    sf::Vector2f collisionSize;
    std::string currentFormName{"Small"};
    std::function<void(const ProjectileRequest&)> projectileRequestHandler;

    bool updatingEffects{false};
    bool clearEffectsRequested{false};

    void changeState(std::unique_ptr<PlayerState> state);
    void applyDamage(bool ignoreProtection);
    void insertEffect(std::unique_ptr<PlayerEffect> effect);
    PlayerMotion choosePlayerMotion() const;
    void updatePlayerAnimation(float dt);
};
