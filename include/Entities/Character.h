#pragma once

#include "Entities/Entity.h"
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
    Crushed
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
    float runSpeed{230.f};
    float groundDeceleration{1000.f};

    float gravity{980.f};
    float maxFallSpeed{900.f};
    float jumpForce{350.f};
    float jumpHoldAcceleration{900.f};
    float maxJumpHoldTime{0.18f};
    float damageInvincibilityDuration{2.f};

    float bodyWidth{28.f};
    float smallBodyHeight{30.f};

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

    void applyHorizontalDeceleration(float dt);
    void applyGravity(float dt);
    void updateCharacterPhysics(float dt);
    void updatePlayerEffects(float dt);

    float getMoveSpeedMultiplier() const;
    float getJumpForceMultiplier() const;

    explicit Character(float x = 0.f, float y = 0.f);
    Character(float x, float y, const CharacterProfile& characterProfile);

public:
    ~Character() override;

    void update(float dt) override;
    void render(sf::RenderWindow& window) const override;

    virtual void moveLeft(float dt);
    virtual void moveRight(float dt);
    virtual void jump();
    virtual void shootFireball();
    void useSpecialAbility();

    bool receivePowerUp(std::unique_ptr<PlayerState> state);
    bool hasAbility(PlayerAbility ability) const;
    void takeDamage();
    void die(DeathCause cause);
    void respawn(float x, float y);

    bool addEffect(std::unique_ptr<PlayerEffect> effect);
    void clearEffects();
    bool defeatsEnemiesOnContact() const;

    void setRunning(bool status);
    bool isRunning() const;

    void setGrounded(bool status);
    bool isGrounded() const;

    bool isFacingRight() const;
    sf::FloatRect getBounds() const override;

    void applyForm(std::string_view formName, float heightMultiplier);
    std::string_view getCurrentFormName() const;

    void setProjectileRequestHandler(
        std::function<void(const ProjectileRequest&)> handler
    );
    void requestProjectile(ProjectileType type);

private:
    std::unique_ptr<PlayerState> currentState;
    std::vector<std::unique_ptr<PlayerEffect>> activeEffects;
    std::vector<std::unique_ptr<PlayerEffect>> pendingEffects;

    sf::Vector2f collisionSize;
    std::string currentFormName{"Small"};
    std::function<void(const ProjectileRequest&)> projectileRequestHandler;

    bool updatingEffects{false};
    bool clearEffectsRequested{false};

    void changeState(std::unique_ptr<PlayerState> state);
    void insertEffect(std::unique_ptr<PlayerEffect> effect);
};
