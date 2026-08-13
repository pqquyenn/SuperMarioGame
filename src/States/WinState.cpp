#include "States/WinState.h"
#include "States/PlayState.h"
#include "States/LevelCompleteState.h"
#include "Core/AssetManager.h"
#include "Physics/CollisionManager.h"
#include <iostream>
#include <algorithm>
#include <cmath>

WinState::WinState(PlayState* play, int id, const std::string& path)
    : playState(play), levelId(id), mapPath(path) {
}

void WinState::onEnter() {
    std::cout << "[WinState] onEnter - Level Win Sequence Started!" << std::endl;
    initSequence();
}

void WinState::initSequence() {
    if (!playState) return;

    Level& level = playState->getLevel();
    Character* player = playState->getPlayer();
    HUD& hud = playState->getHUD();

    hud.setTimeFrozen(true);

    // 1. Find Flagpole Bounds
    if (auto fp = level.findFlagpoleBounds()) {
        flagpoleBounds = *fp;
        flagpoleBottomY = flagpoleBounds.top + flagpoleBounds.height;
    } else {
        // Fallback default
        if (player) {
            flagpoleBounds = sf::FloatRect(player->getPosition().x, 32.f, 16.f, 176.f);
            flagpoleBottomY = flagpoleBounds.top + flagpoleBounds.height;
        }
    }

    // 2. Find Castle Door
    if (auto cd = level.findCastleDoor()) {
        castleDoorPos = *cd;
    } else {
        // Fallback: 160px past flagpole
        castleDoorPos = sf::Vector2f(flagpoleBounds.left + 160.f, flagpoleBottomY);
    }

    // 3. Take Flag Position (removes static flag tile from map grid so it's not rendered twice)
    if (auto fp = level.takeFlagPosition()) {
        flagPos = *fp;
        hasFlag = true;
    } else {
        flagPos = sf::Vector2f(flagpoleBounds.left - 16.f, flagpoleBounds.top);
        hasFlag = true;
    }

    flagBottomY = flagpoleBottomY - 16.f; // Base of flagpole for flag

    // Setup flag sprite
    AssetManager& assets = AssetManager::getInstance();
    const sf::Texture& flagTex = assets.getTexture("Flag");
    if (flagTex.getSize().x > 0) {
        flagSprite.setTexture(flagTex);
        flagSprite.setTextureRect(sf::IntRect(0, 0, 16, 16));
        flagSprite.setPosition(flagPos);
    }

    // Position player on the left side of the flagpole
    if (player) {
        player->setVelocity(0.f, 0.f);
        player->setGrounded(false);
        // Place Mario grabbing the flagpole
        player->setPosition(flagpoleBounds.left - 6.f, player->getPosition().y);
    }

    currentPhase = Phase::FlagSlide;
    phaseTimer = 0.f;
}

void WinState::onExit() {
    std::cout << "[WinState] onExit - Win Sequence Finished" << std::endl;
}

void WinState::handleInput(sf::Event& event, sf::RenderWindow& window) {
    // Gameplay inputs are disabled during victory sequence.
}

void WinState::update(float dt) {
    if (!playState) return;

    switch (currentPhase) {
        case Phase::FlagSlide:
            updateFlagSlide(dt);
            break;
        case Phase::AutoWalk:
            updateAutoWalk(dt);
            break;
        case Phase::ScoreTally:
            updateScoreTally(dt);
            break;
        case Phase::Done:
            break;
    }

    // Always update HUD presentation
    playState->getHUD().update(dt);
}

void WinState::updateFlagSlide(float dt) {
    Character* player = playState->getPlayer();
    if (!player) return;

    constexpr float slideSpeed = 140.f;
    const float playerHeight = player->getBounds().height;
    const float targetPlayerY = flagpoleBottomY - playerHeight;

    // Slide Mario down
    if (player->getPosition().y < targetPlayerY) {
        player->move(0.f, slideSpeed * dt);
        if (player->getPosition().y > targetPlayerY) {
            player->setPosition(player->getPosition().x, targetPlayerY);
        }
    }

    // Slide Flag down concurrently
    if (hasFlag && flagPos.y < flagBottomY) {
        flagPos.y += slideSpeed * dt;
        if (flagPos.y > flagBottomY) {
            flagPos.y = flagBottomY;
        }
        flagSprite.setPosition(flagPos);
    }

    // Check if reached bottom
    if (player->getPosition().y >= targetPlayerY) {
        phaseTimer += dt;
        if (phaseTimer >= 0.35f) {
            // Mario flips to the right side of the flagpole and starts walking to castle
            player->setPosition(flagpoleBounds.left + 12.f, targetPlayerY);
            player->setGrounded(true);
            player->setVelocity(0.f, 0.f);

            // Award flagpole bonus
            playState->getHUD().addScore(1000);

            currentPhase = Phase::AutoWalk;
            phaseTimer = 0.f;
        }
    }
}

void WinState::updateAutoWalk(float dt) {
    Character* player = playState->getPlayer();
    Level& level = playState->getLevel();
    if (!player) return;

    // Walk Mario towards castle door
    player->moveRight(dt);
    player->update(dt);

    // Resolve floor collisions so Mario stays on ground while walking
    CollisionManager::resolveTileCollisions(*player, level.getTileMap(), &level);

    // Keep camera following or centered
    float camX = std::max(200.f, player->getPosition().x);
    level.getCamera().setCenter(camX, level.getCamera().getView().getCenter().y);

    // Check if Mario reached the castle door
    if (player->getPosition().x >= castleDoorPos.x - 4.f) {
        marioInside = true;
        player->setActive(false);
        currentPhase = Phase::ScoreTally;
        phaseTimer = 0.f;
        std::cout << "[WinState] Mario entered the Castle! Starting score tally." << std::endl;
    }
}

void WinState::updateScoreTally(float dt) {
    HUD& hud = playState->getHUD();
    float timeRemaining = hud.getTimeRemaining();

    if (timeRemaining > 0.f) {
        // Tally rate: deduct ~250 time units per second
        float deduct = std::min(timeRemaining, dt * 250.f);
        if (deduct < 1.f && timeRemaining <= 1.f) {
            deduct = timeRemaining;
        }
        
        hud.setTimeRemaining(timeRemaining - deduct);
        int points = static_cast<int>(deduct * 50.f);
        hud.addScore(points);
        totalTimeBonus += points;
    } else {
        hud.setTimeRemaining(0.f);
        phaseTimer += dt;
        // Wait 1.2 seconds before showing level complete screen
        if (phaseTimer >= 1.2f) {
            currentPhase = Phase::Done;
            if (stateManager) {
                stateManager->changeState(std::make_unique<LevelCompleteState>(
                    levelId,
                    mapPath,
                    hud.getScore(),
                    hud.getCoins(),
                    totalTimeBonus
                ));
            }
        }
    }
}

void WinState::render(sf::RenderWindow& window) {
    if (!playState) return;

    Level& level = playState->getLevel();
    Character* player = playState->getPlayer();
    Camera& cam = level.getCamera();
    cam.applyTo(window);

    float camX = cam.getView().getCenter().x;
    bool isUndergroundArea = level.getIsUnderground() ||
                             level.getIsInBonusRoom() || camX > 3600.f;
    sf::Color bgColor =
        isUndergroundArea ? sf::Color::Black : sf::Color(92, 148, 252);

    window.clear(bgColor);
    level.render(window);

    // Draw Flag sliding down if in FlagSlide or later
    if (hasFlag) {
        window.draw(flagSprite);
    }

    // Render player if not inside castle
    if (player && player->isActive() && !marioInside) {
        player->render(window);
    }

    // Render HUD on top
    playState->getHUD().render(window);
}
