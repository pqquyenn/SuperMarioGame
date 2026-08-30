#include "Entities/Enemies/DragonLugia.h"
#include "Entities/Character.h"
#include "Level/TileMap.h"
#include "Core/SoundManager.h"
#include "Core/AssetManager.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace {
constexpr int FRAME_SIZE = 208;
constexpr float SPRITE_SCALE = 0.38f;
constexpr float ORIGIN_X = 104.f;
constexpr float ORIGIN_Y = 175.f;

AnimationClip makeClip(int row, int frameCount, float duration, bool loop) {
    AnimationClip clip;
    clip.frameDuration = duration;
    clip.looping = loop;
    for (int col = 0; col < frameCount; ++col) {
        clip.frames.emplace_back(sf::IntRect(col * FRAME_SIZE, row * FRAME_SIZE, FRAME_SIZE, FRAME_SIZE));
    }
    return clip;
}
} // namespace

DragonLugia::DragonLugia(float x, float y)
    : Enemy(x, y), maxHp(5), currentHp(5) {
    direction = -1;
    speed = 50.f;
    scoreValue = 5000;
    groundY = (y > 0.f) ? y : 192.f;

    initAnimations();
    animator.play(animRun);

    // Flame graphic
    flameShape.setRadius(7.f);
    flameShape.setOrigin(7.f, 7.f);
    flameShape.setFillColor(sf::Color(70, 255, 120)); // Green flame
    flameShape.setOutlineColor(sf::Color(20, 100, 40));
    flameShape.setOutlineThickness(1.5f);

    // HP Bar UI
    hpBarBack.setSize(sf::Vector2f(80.f, 8.f));
    hpBarBack.setFillColor(sf::Color(40, 20, 20, 200));
    hpBarBack.setOutlineColor(sf::Color(255, 215, 60));
    hpBarBack.setOutlineThickness(1.5f);

    hpBarFront.setSize(sf::Vector2f(80.f, 8.f));
    hpBarFront.setFillColor(sf::Color(50, 220, 90));
}

void DragonLugia::initAnimations() {
    animIdle = makeClip(0, 4, 0.15f, true);
    animRun = makeClip(1, 8, 0.09f, true);
    animFireGround = makeClip(2, 10, 0.08f, false);
    animFly = makeClip(3, 6, 0.10f, true);
    animFireFly = makeClip(4, 10, 0.08f, false);
    animAttackGround = makeClip(5, 9, 0.08f, false);
    animAttackFly = makeClip(6, 8, 0.08f, false);
    animHurt = makeClip(7, 3, 0.15f, false);
    animDeath = makeClip(8, 6, 0.18f, false);
}

void DragonLugia::setTexture(const sf::Texture& texture, bool resetRect) {
    Entity::setTexture(texture, resetRect);
    sprite.setOrigin(ORIGIN_X, ORIGIN_Y);
    sprite.setScale(static_cast<float>(direction) * -SPRITE_SCALE, SPRITE_SCALE);

    if (auto* frame = animator.getCurrentFrame()) {
        sprite.setTextureRect(frame->textureRect);
    }

    auto& assets = AssetManager::getInstance();
    if (assets.getTexture("DragonFlameProjectile").getSize().x > 0) {
        flameSprite.setTexture(assets.getTexture("DragonFlameProjectile"));
        auto bounds = flameSprite.getLocalBounds();
        flameSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    }
    if (assets.getTexture("DragonFlameBurstSheet").getSize().x > 0) {
        burstSprite.setTexture(assets.getTexture("DragonFlameBurstSheet"));
    }
}

void DragonLugia::changeBossState(State newState) {
    state = newState;
    stateTimer = 0.f;
    hasFiredInAttack = false;

    switch (state) {
        case State::GroundedWalk:
            animator.play(animRun);
            break;
        case State::GroundedFire:
            animator.play(animFireGround, true);
            SoundManager::getInstance().playSound("bowserfire");
            break;
        case State::Takeoff:
            animator.play(animFly);
            break;
        case State::SwoopTowardsPlayer:
            animator.play(animFly);
            break;
        case State::AerialFire:
            animator.play(animFireFly, true);
            SoundManager::getInstance().playSound("bowserfire");
            break;
        case State::Landing:
            animator.play(animFly);
            break;
        case State::Hurt:
            animator.play(animHurt, true);
            hurtTimer = 0.7f;
            break;
        case State::Dying:
            animator.play(animDeath, true);
            if (!deathSoundPlayed) {
                deathSoundPlayed = true;
                SoundManager::getInstance().playSound("bowserfall");
            }
            break;
        case State::Defeated:
            bossDefeatedFlag = true;
            break;
    }
}

void DragonLugia::update(float dt) {
    if (!isActive()) return;

    stateTimer += dt;
    animator.update(dt);

    if (hurtTimer > 0.f) {
        hurtTimer -= dt;
        int flash = static_cast<int>(hurtTimer * 20.f) % 2;
        sprite.setColor(flash == 0 ? sf::Color(255, 100, 100, 240) : sf::Color(255, 255, 255, 255));
    } else {
        sprite.setColor(sf::Color::White);
    }

    // Always face Mario, but prevent direction flipping when clamped against an arena wall.
    // Without this guard the direction oscillates every frame when the player is beyond the wall,
    // which causes the flame spawn offset to push outside the arena and makes the dragon freeze.
    if (state != State::Dying && state != State::Defeated) {
        if (targetPlayerPos.x < position.x - 10.f && position.x > arenaMinX + 1.f) {
            direction = -1;
        } else if (targetPlayerPos.x > position.x + 10.f && position.x < arenaMaxX - 1.f) {
            direction = 1;
        }
    }

    if (const auto* frame = animator.getCurrentFrame()) {
        sprite.setTextureRect(frame->textureRect);
    }

    sprite.setScale(static_cast<float>(direction) * -SPRITE_SCALE, SPRITE_SCALE);

    // Boss State Machine with Targeted Pursuit AI
    switch (state) {
        case State::GroundedWalk: {
            position.y = groundY;
            float dx = targetPlayerPos.x - position.x;
            float moveDir = (dx < 0.f) ? -1.f : 1.f;
            position.x += moveDir * speed * dt;

            position.x = std::clamp(position.x, arenaMinX, arenaMaxX);

            // Require a minimum walk time (0.8s) before allowing distance-based transition.
            // This prevents rapid state cycling when the dragon is clamped at a wall boundary.
            if (stateTimer >= 2.2f || (stateTimer >= 0.8f && std::abs(dx) < 80.f)) {
                walkCycle++;
                if (walkCycle % 2 == 0) {
                    changeBossState(State::Takeoff);
                } else {
                    changeBossState(State::GroundedFire);
                }
            }
            break;
        }

        case State::GroundedFire: {
            position.y = groundY;
            if (!hasFiredInAttack && stateTimer >= 0.35f) {
                hasFiredInAttack = true;
                shootFlameAtTarget(targetPlayerPos, 100.f);
            }
            if (animator.isFinished() || stateTimer >= 0.8f) {
                changeBossState(State::Takeoff);
            }
            break;
        }

        case State::Takeoff: {
            float targetAltitude = std::clamp(targetPlayerPos.y - 50.f, 50.f, 130.f);
            position.y -= 110.f * dt;
            float dx = targetPlayerPos.x - position.x;
            position.x += ((dx < 0.f) ? -1.f : 1.f) * (speed * 0.8f) * dt;
            position.x = std::clamp(position.x, arenaMinX, arenaMaxX);

            if (position.y <= targetAltitude) {
                position.y = targetAltitude;
                changeBossState(State::SwoopTowardsPlayer);
            }
            break;
        }

        case State::SwoopTowardsPlayer: {
            hoverTime += dt;
            // Clamp target within arena bounds so the dragon doesn't perpetually
            // chase a position it can never reach (e.g. player against a wall).
            float targetX = std::clamp(targetPlayerPos.x, arenaMinX, arenaMaxX);
            float targetY = std::clamp(targetPlayerPos.y - 45.f, 45.f, 125.f);

            float dx = targetX - position.x;
            float dy = targetY - position.y;
            float dist = std::hypot(dx, dy);

            // Fly directly towards Mario
            if (dist > 10.f) {
                position.x += (dx / dist) * (speed * 1.7f) * dt;
                position.y += (dy / dist) * (speed * 1.3f) * dt;
            }

            position.x = std::clamp(position.x, arenaMinX, arenaMaxX);
            position.y = std::clamp(position.y, 40.f, groundY - 20.f);

            // Recalculate distance after clamping for accurate transition check
            float clampedDist = std::hypot(targetX - position.x, targetY - position.y);

            // Once close or after swooping, unleash aerial fire
            if (clampedDist < 65.f || stateTimer >= 2.0f) {
                changeBossState(State::AerialFire);
            }
            break;
        }

        case State::AerialFire: {
            if (!hasFiredInAttack && stateTimer >= 0.35f) {
                hasFiredInAttack = true;
                // Shoot targeted flame blast at Mario's position with balanced speed
                shootFlameAtTarget(targetPlayerPos, 110.f);
                shootFlameAtTarget(targetPlayerPos + sf::Vector2f(static_cast<float>(direction) * 25.f, -10.f), 95.f);
            }
            if (animator.isFinished() || stateTimer >= 0.8f) {
                changeBossState(State::Landing);
            }
            break;
        }

        case State::Landing: {
            position.y += 115.f * dt;
            float dx = targetPlayerPos.x - position.x;
            position.x += ((dx < 0.f) ? -1.f : 1.f) * (speed * 0.6f) * dt;
            position.x = std::clamp(position.x, arenaMinX, arenaMaxX);

            if (position.y >= groundY) {
                position.y = groundY;
                changeBossState(State::GroundedWalk);
            }
            break;
        }

        case State::Hurt: {
            // Recoil away
            position.x -= static_cast<float>(direction) * 40.f * dt;
            position.x = std::clamp(position.x, arenaMinX, arenaMaxX);

            if (stateTimer >= 0.5f) {
                if (currentHp <= 0) {
                    changeBossState(State::Dying);
                } else {
                    changeBossState(State::Takeoff);
                }
            }
            break;
        }

        case State::Dying: {
            position.y += 30.f * dt;
            if (animator.isFinished() || stateTimer >= 1.5f) {
                changeBossState(State::Defeated);
            }
            break;
        }

        case State::Defeated: {
            break;
        }
    }

    sprite.setPosition(position);
    updateFlames(dt, nullptr);
}

void DragonLugia::updateWithPlayer(float dt, Character* player, const TileMap* tileMap) {
    if (player && player->isActive() && !player->isDying()) {
        targetPlayerPos = player->getPosition();
    }

    // Check collision between flames and player
    if (player && player->isActive() && !player->isDying()) {
        if (state != State::Dying && state != State::Defeated) {
            const sf::FloatRect pBounds = player->getBounds();
            for (auto& flame : flames) {
                if (!flame.active) continue;
                sf::FloatRect fBounds(flame.position.x - 8.f, flame.position.y - 8.f, 16.f, 16.f);
                if (pBounds.intersects(fBounds)) {
                    player->takeDamage();
                    flame.active = false;
                }
            }
        }
    }
}

void DragonLugia::triggerFlameImpact(sf::Vector2f pos) {
    DragonFlameImpact impact;
    impact.position = pos;
    impact.timer = 0.f;
    impact.maxDuration = 0.32f;
    impact.active = true;
    impacts.push_back(impact);
}

void DragonLugia::shootFlameAtTarget(sf::Vector2f targetPos, float flameSpeed) {
    // Determine flame spawn offset toward the target, not just based on facing direction,
    // so flames aren't spawned outside arena walls when the dragon is near a wall.
    float dirToTarget = (targetPos.x >= position.x) ? 1.f : -1.f;
    sf::Vector2f spawnPos(
        position.x + dirToTarget * 26.f,
        position.y - 30.f
    );

    // Clamp spawn position inside the arena walls to prevent instant despawn
    spawnPos.x = std::clamp(spawnPos.x, 18.f, 382.f);

    sf::Vector2f dir = targetPos - spawnPos;
    float length = std::hypot(dir.x, dir.y);
    if (length > 0.001f) {
        dir.x /= length;
        dir.y /= length;
    } else {
        dir = sf::Vector2f(dirToTarget, 0.f);
    }

    DragonFlame flame;
    flame.position = spawnPos;
    flame.velocity = dir * flameSpeed;
    flame.lifetime = 3.5f;
    flame.active = true;
    flames.push_back(flame);
}

void DragonLugia::updateFlames(float dt, const TileMap* tileMap) {
    for (auto& flame : flames) {
        if (!flame.active) continue;
        flame.position += flame.velocity * dt;
        flame.lifetime -= dt;
        flame.animTimer += dt;

        // 1. Chạm đất màu nâu ở sàn dưới -> Kích hoạt hiệu ứng nổ tại mặt đất
        // Uses flameGroundY (actual brown floor at row 13) instead of groundY (dragon walk height)
        // so flames pass through blue castle blocks and only explode on the brown ground.
        if (flame.position.y >= flameGroundY - 2.f) {
            triggerFlameImpact(sf::Vector2f(flame.position.x, flameGroundY));
            flame.active = false;
            continue;
        }

        // 2. Chạm tường màu nâu bên trái -> Kích hoạt hiệu ứng nổ tại tường trái
        if (flame.position.x <= 16.f) {
            triggerFlameImpact(sf::Vector2f(16.f, std::min(flame.position.y, flameGroundY)));
            flame.active = false;
            continue;
        }

        // 3. Chạm tường màu nâu bên phải -> Kích hoạt hiệu ứng nổ tại tường phải
        if (flame.position.x >= 384.f) {
            triggerFlameImpact(sf::Vector2f(384.f, std::min(flame.position.y, flameGroundY)));
            flame.active = false;
            continue;
        }

        // 4. Hết thời gian tồn tại hoặc bay ra ngoài biên màn hình (phía trên / dưới quá mức)
        if (flame.lifetime <= 0.f || flame.position.y < 0.f || flame.position.y > flameGroundY + 20.f) {
            flame.active = false;
        }
    }

    flames.erase(
        std::remove_if(flames.begin(), flames.end(), [](const DragonFlame& f) { return !f.active; }),
        flames.end()
    );

    // Update impact burst animations
    for (auto& impact : impacts) {
        if (!impact.active) continue;
        impact.timer += dt;
        if (impact.timer >= impact.maxDuration) {
            impact.active = false;
        }
    }

    impacts.erase(
        std::remove_if(impacts.begin(), impacts.end(), [](const DragonFlameImpact& imp) { return !imp.active; }),
        impacts.end()
    );
}

void DragonLugia::onStomped() {
    // DragonLugia boss cannot be stomped
}

bool DragonLugia::canBeStomped() const {
    return false;
}

void DragonLugia::onFireball() {
    if (hurtTimer > 0.f || state == State::Dying || state == State::Defeated) return;

    currentHp--;
    SoundManager::getInstance().playSound("kick");
    std::cout << "[DragonLugia] Hit by fireball! HP: " << currentHp << "/" << maxHp << std::endl;

    if (currentHp <= 0) {
        changeBossState(State::Dying);
    } else {
        changeBossState(State::Hurt);
    }
}

sf::FloatRect DragonLugia::getBounds() const {
    if (state == State::Dying || state == State::Defeated) {
        return sf::FloatRect(0.f, 0.f, 0.f, 0.f);
    }
    // Snug hitbox accurately matching the dragon's visual sprite
    return sf::FloatRect(position.x - 14.f, position.y - 42.f, 28.f, 34.f);
}

void DragonLugia::render(sf::RenderWindow& window) const {
    if (state == State::Defeated) return;

    // Draw active flying flame projectiles
    bool hasFlameTexture = (flameSprite.getTexture() != nullptr);
    for (const auto& flame : flames) {
        if (!flame.active) continue;
        if (hasFlameTexture) {
            flameSprite.setPosition(flame.position);
            float pulse = 0.22f * (1.0f + 0.12f * std::sin(flame.animTimer * 18.f));
            flameSprite.setScale(pulse, pulse);

            // Orient the flame towards movement direction
            float angle = std::atan2(flame.velocity.y, flame.velocity.x) * 180.f / 3.14159265f;
            flameSprite.setRotation(angle - 90.f);

            window.draw(flameSprite);
        } else {
            flameShape.setPosition(flame.position);
            float pulse = 1.0f + 0.2f * std::sin(flame.animTimer * 15.f);
            flameShape.setScale(pulse, pulse);
            window.draw(flameShape);
        }
    }

    // Draw flame ground impact bursts
    bool hasBurstTexture = (burstSprite.getTexture() != nullptr);
    for (const auto& impact : impacts) {
        if (!impact.active) continue;
        if (hasBurstTexture) {
            float progress = std::clamp(impact.timer / impact.maxDuration, 0.f, 0.999f);
            int frameIdx = std::clamp(static_cast<int>(progress * 5.0f), 0, 4);
            burstSprite.setTextureRect(sf::IntRect(frameIdx * 64, 0, 64, 48));
            burstSprite.setOrigin(32.f, 44.f);
            burstSprite.setPosition(impact.position);
            burstSprite.setScale(0.55f, 0.55f);
            window.draw(burstSprite);
        }
    }

    // Draw Boss Dragon Sprite
    window.draw(sprite);

    // Draw Boss Health Bar snug above boss head
    if (state != State::Dying && state != State::Defeated && currentHp > 0) {
        float barW = 44.f;
        float barH = 5.f;
        float barX = position.x - barW / 2.f;
        float barY = position.y - 48.f;

        hpBarBack.setSize(sf::Vector2f(barW, barH));
        hpBarBack.setPosition(barX, barY);

        float healthPct = std::clamp(static_cast<float>(currentHp) / static_cast<float>(maxHp), 0.f, 1.f);
        hpBarFront.setSize(sf::Vector2f(barW * healthPct, barH));
        hpBarFront.setPosition(barX, barY);
        hpBarFront.setFillColor(healthPct > 0.4f ? sf::Color(50, 230, 80) : sf::Color(230, 50, 50));

        window.draw(hpBarBack);
        window.draw(hpBarFront);
    }
}
