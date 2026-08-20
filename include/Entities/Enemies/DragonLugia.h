#pragma once

#include "Entities/Enemies/Enemy.h"
#include "Animation/AnimationClip.h"
#include "Animation/SpriteAnimator.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

class Character;
class TileMap;

struct DragonFlame {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime{4.f};
    bool active{true};
    float animTimer{0.f};
};

class DragonLugia : public Enemy {
public:
    enum class State {
        GroundedWalk,
        GroundedFire,
        Takeoff,
        SwoopTowardsPlayer,
        AerialFire,
        Landing,
        Hurt,
        Dying,
        Defeated
    };

private:
    int maxHp{5};
    int currentHp{5};

    State state{State::GroundedWalk};
    float stateTimer{0.f};
    float hurtTimer{0.f};
    float hoverTime{0.f};
    float groundY{192.f};
    float arenaMinX{36.f};
    float arenaMaxX{364.f};

    sf::Vector2f targetPlayerPos{60.f, 192.f};

    bool hasFiredInAttack{false};
    bool deathSoundPlayed{false};
    bool bossDefeatedFlag{false};

    // Animations
    AnimationClip animIdle;
    AnimationClip animRun;
    AnimationClip animFireGround;
    AnimationClip animFly;
    AnimationClip animFireFly;
    AnimationClip animAttackGround;
    AnimationClip animAttackFly;
    AnimationClip animHurt;
    AnimationClip animDeath;
    SpriteAnimator animator;

    // Projectiles
    std::vector<DragonFlame> flames;
    mutable sf::CircleShape flameShape;

    // UI Health bar
    mutable sf::RectangleShape hpBarBack;
    mutable sf::RectangleShape hpBarFront;

    void initAnimations();
    void shootFlameAtTarget(sf::Vector2f targetPos, float speed);
    void updateFlames(float dt, const TileMap* tileMap);
    void changeBossState(State newState);

public:
    DragonLugia(float x = 0.f, float y = 0.f);

    void setTexture(const sf::Texture& texture, bool resetRect = false) override;
    void update(float dt) override;
    void updateWithPlayer(float dt, Character* player, const TileMap* tileMap);
    void render(sf::RenderWindow& window) const override;

    void onStomped() override;
    bool canBeStomped() const override;
    void onFireball() override;

    sf::FloatRect getBounds() const override;
    int getCurrentHp() const { return currentHp; }
    int getMaxHp() const { return maxHp; }
    bool isBossDefeated() const { return bossDefeatedFlag; }
    bool isBossDying() const { return state == State::Dying || state == State::Defeated; }

    const std::vector<DragonFlame>& getFlames() const { return flames; }
};
