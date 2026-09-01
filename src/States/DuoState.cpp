#include "States/DuoState.h"

#include "Core/AchievementSystem.h"
#include "Core/AssetManager.h"
#include "Core/SoundManager.h"
#include "Duo/DuoRules.h"
#include "Entities/Character.h"
#include "Entities/Enemies/DragonLugia.h"
#include "Entities/Enemies/Enemy.h"
#include "Entities/Fireball.h"
#include "Entities/Items/Item.h"
#include "Entities/Luigi.h"
#include "Entities/Mario.h"
#include "Factories/EntityFactory.h"
#include "Physics/CollisionManager.h"
#include "PlayerEffects/DamageInvincibilityEffect.h"
#include "PvP/PvPCombatResolver.h"
#include "States/DuoLevelCompleteState.h"
#include "States/GameOverState.h"
#include "States/GameStateManager.h"
#include "States/MenuState.h"
#include "States/PauseState.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <limits>
#include <utility>

namespace {
constexpr float CameraWidth = 400.f;
constexpr float CameraHeight = 225.f;
constexpr float CameraPlayerMargin = 40.f;
constexpr float BoostJumpVelocity = -430.f;
constexpr float BoostCooldown = 0.18f;
constexpr float BubbleDelay = 0.65f;
constexpr float BubbleAutoRescueTime = 5.f;
constexpr float BubbleFollowSpeed = 3.4f;
constexpr float RespawnProtection = 2.f;
constexpr float PortalEnterDuration = 0.35f;
constexpr float PortalReentryCooldown = 0.65f;
constexpr float TeamWipeDelay = 1.4f;
constexpr float FlagJoinWindow = 2.f;
constexpr float BossFinishDelay = 1.2f;

const char* characterName(CharacterChoice choice) {
    return choice == CharacterChoice::Luigi ? "LUIGI" : "MARIO";
}

std::unique_ptr<Character> makeCharacter(CharacterChoice choice) {
    if (choice == CharacterChoice::Luigi) {
        return std::make_unique<Luigi>();
    }
    return std::make_unique<Mario>();
}

float centerX(const sf::FloatRect& bounds) {
    return bounds.left + bounds.width * 0.5f;
}

float squaredDistance(sf::Vector2f a, sf::Vector2f b) {
    const float x = a.x - b.x;
    const float y = a.y - b.y;
    return x * x + y * y;
}

InputPermissions disabledInput() {
    InputPermissions permissions;
    permissions.allowMoveLeft = false;
    permissions.allowMoveRight = false;
    permissions.allowJump = false;
    permissions.allowCrouch = false;
    permissions.allowAction = false;
    permissions.allowRun = false;
    return permissions;
}
}

void DuoState::PlayerEvents::onNotify(const GameEvent& event) {
    switch (event.type) {
        case GameEventType::COIN_COLLECTED:
            ++stats.coins;
            stats.score += event.value;
            break;
        case GameEventType::ENEMY_DEFEATED:
            ++stats.enemiesDefeated;
            stats.score += event.value;
            break;
        case GameEventType::POWERUP_COLLECTED:
            stats.score += event.value;
            break;
        case GameEventType::LIFE_GAINED:
            lives += event.value > 0 ? event.value : 1;
            stats.score += event.scoreDelta;
            break;
        case GameEventType::PLAYER_DIED:
            if (!deathRequested) {
                deathRequested = true;
                ++stats.deaths;
                lives = std::max(0, lives - 1);
            }
            break;
        default:
            break;
    }
}

DuoState::PlayerSlot::PlayerSlot(
    DuoPlayerId playerId,
    CharacterChoice choice,
    BindingTarget bindingTarget,
    int startingLives)
    : id{playerId},
      characterChoice{choice},
      input{bindingTarget},
      events{std::max(1, startingLives)} {}

DuoState::DuoState(DuoSessionConfig config)
    : session{std::move(config)},
      playerOne{
          DuoPlayerId::One,
          session.playerOneChoice,
          BindingTarget::DuoPlayerOne,
          session.startingLives},
      playerTwo{
          DuoPlayerId::Two,
          session.playerTwoChoice,
          BindingTarget::DuoPlayerTwo,
          session.startingLives} {
    if (session.mapPath.empty()) {
        session.mapPath = "1.1/1-1.level";
    }
}

DuoState::~DuoState() = default;

void DuoState::onEnter() {
    AssetManager::getInstance().loadLevelAssets();
    levelLoadFailed = !level.loadLevel(session.mapPath);
    if (levelLoadFailed) {
        std::cerr << "[DuoState] Failed to load " << session.mapPath
                  << std::endl;
        return;
    }

    stageName = level.getDefinition().name.empty()
        ? std::filesystem::path(session.mapPath).stem().string()
        : level.getDefinition().name;
    if (stageName.empty()) {
        stageName = "1-1";
    }
    timeRemaining = static_cast<float>(level.getTimeLimit());
    level.setPowerupSpawnMultiplier(2);
    skyDropTimer = 5.f;
    createPlayers();
    configureCamera();
    startStageMusic();
}

void DuoState::onExit() {
    playerOne.eventConnection.disconnect();
    playerTwo.eventConnection.disconnect();
    fireballs.clear();
    SoundManager::getInstance().stopBGM();
}

void DuoState::createPlayers() {
    playerOne.character = makeCharacter(playerOne.characterChoice);
    playerTwo.character = makeCharacter(playerTwo.characterChoice);

    AssetManager& assets = AssetManager::getInstance();
    sf::Texture& primary = assets.getPlayerTexture(PlayerPalette::Primary);
    playerOne.character->setTexture(primary);
    playerOne.palette = PlayerPalette::Primary;

    const bool sameCharacter =
        playerOne.characterChoice == playerTwo.characterChoice;
    if (sameCharacter) {
        sf::Texture& secondary =
            assets.getPlayerTexture(PlayerPalette::Secondary);
        playerTwo.character->setTexture(secondary);
        playerTwo.palette = &secondary == &primary
            ? PlayerPalette::Primary
            : PlayerPalette::Secondary;
    } else {
        playerTwo.character->setTexture(primary);
        playerTwo.palette = PlayerPalette::Primary;
    }

    playerOne.character->setProjectileRequestHandler(
        [this](const ProjectileRequest& request) {
            requestFireball(playerOne, request);
        });
    playerTwo.character->setProjectileRequestHandler(
        [this](const ProjectileRequest& request) {
            requestFireball(playerTwo, request);
        });

    playerOne.eventConnection =
        playerOne.character->addObserver(&playerOne.events);
    playerTwo.eventConnection =
        playerTwo.character->addObserver(&playerTwo.events);

    playerOne.character->update(0.f);
    playerTwo.character->update(0.f);
    const sf::FloatRect oneBounds = playerOne.character->getBounds();
    const sf::FloatRect twoBounds = playerTwo.character->getBounds();
    playerOne.spawnPoint = level.getStartPosition({
        std::max(1.f, oneBounds.width),
        std::max(1.f, oneBounds.height)});
    playerTwo.spawnPoint = level.findSafeSpawnNear(
        playerOne.spawnPoint,
        {std::max(1.f, twoBounds.width),
         std::max(1.f, twoBounds.height)});
    playerOne.character->setPosition(playerOne.spawnPoint);
    playerTwo.character->setPosition(playerTwo.spawnPoint);
    playerOne.previousBounds = playerOne.character->getBounds();
    playerTwo.previousBounds = playerTwo.character->getBounds();
}

void DuoState::configureCamera() {
    Camera& camera = level.getCamera();
    camera.setSize(CameraWidth, CameraHeight);
    const float tileSize = level.isDataDriven()
        ? level.getDefinition().tileSize
        : 16.f;
    camera.setLevelBounds(
        level.getTileMap().getMapWidth() * tileSize,
        level.getTileMap().getMapHeight() * tileSize);
    updateCamera();
}

void DuoState::startStageMusic() {
    bool hasBoss = false;
    for (const auto& enemy : level.getEnemies()) {
        if (dynamic_cast<DragonLugia*>(enemy.get())) {
            hasBoss = true;
            break;
        }
    }

    std::string lower = stageName + " " + session.mapPath;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char value) {
                       return static_cast<char>(std::tolower(value));
                   });
    if (hasBoss || lower.find("castle") != std::string::npos) {
        SoundManager::getInstance().playBGM(
            "assets/audio/music/castle.wav");
    } else if (level.usesDarkBackground() ||
               lower.find("underground") != std::string::npos ||
               lower.find("1-2") != std::string::npos) {
        SoundManager::getInstance().playBGM(
            "assets/audio/music/underground.wav");
    } else if (lower.find("underwater") != std::string::npos) {
        SoundManager::getInstance().playBGM(
            "assets/audio/music/underwater.wav");
    } else {
        SoundManager::getInstance().playBGM(
            "assets/audio/music/overworld.wav");
    }
}

void DuoState::handleInput(sf::Event& event, sf::RenderWindow&) {
    if (levelLoadFailed || event.type != sf::Event::KeyPressed ||
        transitionQueued) {
        return;
    }

    if (event.key.code == sf::Keyboard::Escape && stateManager) {
        stateManager->pushState(std::make_unique<PauseState>(session));
        return;
    }
    if (portalSequence.active || teamWipe || finishPending) {
        return;
    }

    const sf::Keyboard::Key key = event.key.code;
    if (playerOne.input.matches(InputAction::Crouch, key)) {
        startPortal(playerOne, PortalActivation::Down);
    } else if (playerTwo.input.matches(InputAction::Crouch, key)) {
        startPortal(playerTwo, PortalActivation::Down);
    } else if (playerOne.input.matches(InputAction::MoveRight, key)) {
        startPortal(playerOne, PortalActivation::Right);
    } else if (playerTwo.input.matches(InputAction::MoveRight, key)) {
        startPortal(playerTwo, PortalActivation::Right);
    }
}

void DuoState::updateBossTargets(float dt) {
    std::vector<Character*> targets;
    if (playerOne.lifeState == DuoLifeState::Active &&
        !playerOne.finished) {
        targets.push_back(playerOne.character.get());
    }
    if (playerTwo.lifeState == DuoLifeState::Active &&
        !playerTwo.finished) {
        targets.push_back(playerTwo.character.get());
    }
    for (auto& enemy : level.getEnemies()) {
        if (auto* dragon = dynamic_cast<DragonLugia*>(enemy.get())) {
            dragon->updateWithPlayers(dt, targets, &level.getTileMap());
        }
    }
}

void DuoState::updatePlayer(
    PlayerSlot& slot,
    float dt,
    const InputPermissions& permissions) {
    Character& character = *slot.character;
    slot.previousBounds = character.getBounds();
    slot.boostCooldown = std::max(0.f, slot.boostCooldown - dt);

    if (slot.lifeState == DuoLifeState::Dying) {
        character.update(dt);
        return;
    }
    if (slot.lifeState != DuoLifeState::Active || !character.isActive()) {
        return;
    }

    slot.input.handleInput(
        character,
        dt,
        slot.finished ? disabledInput() : permissions);
    if (slot.finished) {
        character.setVelocity(0.f, 0.f);
    }
    character.update(dt);
    if (!character.isActive() || character.isDying()) {
        return;
    }

    CollisionManager::resolveTileCollisions(
        character, level.getTileMap(), &level);
    for (auto& platform : level.getMovingPlatforms()) {
        if (platform && platform->isActive()) {
            CollisionManager::resolveMovingPlatform(character, *platform);
        }
    }
    CollisionManager::tryStandUp(character, level.getTileMap());
    constrainToLevel(slot);

    if (character.getPosition().y > level.getKillPlaneY()) {
        character.die(DeathCause::Void);
    }
}

void DuoState::resolveEnemyContacts(PlayerSlot& slot) {
    if (slot.lifeState != DuoLifeState::Active || slot.finished ||
        !slot.character->isActive() || slot.character->isDying()) {
        return;
    }
    for (auto& enemy : level.getEnemies()) {
        if (enemy && enemy->isActive()) {
            CollisionManager::resolveEntityCollisions(
                *slot.character, *enemy);
        }
    }
}

void DuoState::resolveItemContacts() {
    for (auto& item : level.getItems()) {
        if (!item || !item->isActive()) {
            continue;
        }

        PlayerSlot* candidates[2] = {nullptr, nullptr};
        std::size_t count = 0;
        for (PlayerSlot* slot : {&playerOne, &playerTwo}) {
            if (slot->lifeState != DuoLifeState::Active || slot->finished ||
                !slot->character->isActive() || slot->character->isDying()) {
                continue;
            }
            sf::FloatRect overlap;
            if (CollisionManager::checkAABB(
                    slot->character->getBounds(),
                    item->getBounds(),
                    overlap)) {
                candidates[count++] = slot;
            }
        }
        if (count == 0) {
            continue;
        }
        PlayerSlot* collector = candidates[0];
        if (count == 2) {
            const sf::Vector2f itemCenter{
                item->getBounds().left + item->getBounds().width * 0.5f,
                item->getBounds().top + item->getBounds().height * 0.5f};
            const sf::Vector2f oneCenter{
                centerX(candidates[0]->character->getBounds()),
                candidates[0]->character->getBounds().top};
            const sf::Vector2f twoCenter{
                centerX(candidates[1]->character->getBounds()),
                candidates[1]->character->getBounds().top};
            if (squaredDistance(twoCenter, itemCenter) <
                squaredDistance(oneCenter, itemCenter)) {
                collector = candidates[1];
            }
        }
        CollisionManager::resolveEntityCollisions(
            *collector->character, *item);
    }
}

void DuoState::resolveBoostJump() {
    if (portalSequence.active || finishPending ||
        playerOne.lifeState != DuoLifeState::Active ||
        playerTwo.lifeState != DuoLifeState::Active ||
        playerOne.finished || playerTwo.finished) {
        return;
    }

    Character& one = *playerOne.character;
    Character& two = *playerTwo.character;
    if (!one.isActive() || one.isDying() ||
        !two.isActive() || two.isDying()) {
        return;
    }

    const PvPContactOutcome outcome =
        PvPCombatResolver::classifyPlayerContact(
            {playerOne.previousBounds, one.getBounds(), one.getVelocity()},
            {playerTwo.previousBounds, two.getBounds(), two.getVelocity()});

    auto bounce = [](PlayerSlot& attacker, PlayerSlot& target) {
        if (attacker.boostCooldown > 0.f) {
            return;
        }
        const sf::FloatRect attackerBounds = attacker.character->getBounds();
        const sf::FloatRect targetBounds = target.character->getBounds();
        attacker.character->setPosition(
            attacker.character->getPosition().x,
            targetBounds.top - attackerBounds.height);
        attacker.character->setVelocity(
            attacker.character->getVelocity().x,
            BoostJumpVelocity);
        attacker.character->setGrounded(false);
        attacker.boostCooldown = BoostCooldown;
        SoundManager::getInstance().playSound("stomp");
    };

    if (outcome == PvPContactOutcome::PlayerOneStomps) {
        bounce(playerOne, playerTwo);
    } else if (outcome == PvPContactOutcome::PlayerTwoStomps) {
        bounce(playerTwo, playerOne);
    }
}

float DuoState::maximumPlayerSeparation() const {
    return std::max(
        64.f,
        level.getCamera().getView().getSize().x - CameraPlayerMargin * 2.f);
}

void DuoState::enforceTether() {
    if (portalSequence.active ||
        playerOne.lifeState != DuoLifeState::Active ||
        playerTwo.lifeState != DuoLifeState::Active ||
        playerOne.finished || playerTwo.finished) {
        return;
    }

    PlayerSlot* left = &playerOne;
    PlayerSlot* right = &playerTwo;
    if (centerX(playerOne.character->getBounds()) >
        centerX(playerTwo.character->getBounds())) {
        std::swap(left, right);
    }

    const float separation =
        centerX(right->character->getBounds()) -
        centerX(left->character->getBounds());
    const float excess = separation - maximumPlayerSeparation();
    if (excess <= 0.f) {
        return;
    }

    const float leftOutward =
        std::max(0.f, -left->character->getVelocity().x);
    const float rightOutward =
        std::max(0.f, right->character->getVelocity().x);
    const float total = leftOutward + rightOutward;
    const float leftShare = total > 0.01f
        ? leftOutward / total
        : 0.5f;
    const float rightShare = 1.f - leftShare;
    left->character->move(excess * leftShare, 0.f);
    right->character->move(-excess * rightShare, 0.f);

    if (left->character->getVelocity().x < 0.f) {
        left->character->setVelocity(
            0.f, left->character->getVelocity().y);
    }
    if (right->character->getVelocity().x > 0.f) {
        right->character->setVelocity(
            0.f, right->character->getVelocity().y);
    }
    constrainToLevel(*left);
    constrainToLevel(*right);
}

void DuoState::constrainToLevel(PlayerSlot& slot) {
    if (slot.lifeState != DuoLifeState::Active ||
        !slot.character->isActive() || slot.character->isDying()) {
        return;
    }
    Character& character = *slot.character;
    const sf::FloatRect bounds = character.getBounds();
    const float x = std::clamp(
        character.getPosition().x,
        level.getLeftBoundaryX(),
        level.getRightBoundaryX(bounds.width));
    if (x != character.getPosition().x) {
        character.setPosition(x, character.getPosition().y);
        character.setVelocity(0.f, character.getVelocity().y);
    }
}

void DuoState::updateCamera() {
    const bool oneActive =
        playerOne.lifeState == DuoLifeState::Active &&
        playerOne.character->isActive();
    const bool twoActive =
        playerTwo.lifeState == DuoLifeState::Active &&
        playerTwo.character->isActive();
    if (oneActive && twoActive) {
        level.updateCameraFor(DuoRules::calculateCameraFocus(
            playerOne.character->getBounds(),
            playerTwo.character->getBounds()));
    } else if (oneActive) {
        level.updateCameraFor(playerOne.character->getPosition());
    } else if (twoActive) {
        level.updateCameraFor(playerTwo.character->getPosition());
    }
}

std::size_t DuoState::activeProjectileCount(DuoPlayerId owner) const {
    return static_cast<std::size_t>(std::count_if(
        fireballs.begin(), fireballs.end(),
        [owner](const OwnedFireball& fireball) {
            return fireball.owner == owner && fireball.projectile &&
                   fireball.projectile->isActive();
        }));
}

void DuoState::requestFireball(
    PlayerSlot& owner,
    const ProjectileRequest& request) {
    if (request.type != ProjectileType::Fireball ||
        owner.lifeState != DuoLifeState::Active || owner.finished ||
        activeProjectileCount(owner.id) >= 2) {
        return;
    }
    sf::Texture& sheet =
        AssetManager::getInstance().getTexture("BlockTileSheet");
    if (sheet.getSize().x == 0) {
        return;
    }
    fireballs.push_back({
        owner.id,
        std::make_unique<Fireball>(
            request.position.x,
            request.position.y - 4.f,
            request.facingRight,
            sheet,
            8.f)});
}

void DuoState::updateProjectiles(float dt) {
    for (auto& owned : fireballs) {
        if (!owned.projectile || !owned.projectile->isActive()) {
            continue;
        }
        Fireball& fireball = *owned.projectile;
        fireball.update(dt);
        CollisionManager::resolveTileCollisions(
            fireball, level.getTileMap(), &level);
        if (!fireball.isActive() || fireball.isExploding()) {
            continue;
        }

        for (auto& enemy : level.getEnemies()) {
            if (!enemy || !enemy->isActive()) {
                continue;
            }
            sf::FloatRect overlap;
            if (CollisionManager::checkAABB(
                    fireball.getBounds(), enemy->getBounds(), overlap)) {
                enemy->onFireball();
                slotFor(owned.owner).character->notify(
                    GameEvent::enemyDefeated(enemy->getScoreValue()));
                fireball.explode();
                break;
            }
        }

        if (fireball.isActive()) {
            const sf::FloatRect camera = level.getCamera().getViewBounds();
            const sf::FloatRect active{
                camera.left - 64.f,
                camera.top - 64.f,
                camera.width + 128.f,
                camera.height + 128.f};
            if (!fireball.getBounds().intersects(active)) {
                fireball.explode();
            }
        }
    }
    fireballs.erase(
        std::remove_if(
            fireballs.begin(), fireballs.end(),
            [](const OwnedFireball& fireball) {
                return !fireball.projectile ||
                       !fireball.projectile->isActive();
            }),
        fireballs.end());
}

void DuoState::processDeathEvents() {
    for (PlayerSlot* slot : {&playerOne, &playerTwo}) {
        if (!slot->events.deathRequested) {
            continue;
        }
        slot->events.deathRequested = false;
        slot->lifeState = DuoLifeState::Dying;
        slot->deathTransitionTimer = 0.f;
        slot->finished = false;
    }
    if (!hasActivePlayer() && !finishPending) {
        beginTeamWipe();
    }
}

void DuoState::updateDownedPlayers(float dt) {
    if (teamWipe) {
        return;
    }
    for (PlayerSlot* slot : {&playerOne, &playerTwo}) {
        if (slot->lifeState != DuoLifeState::Dying) {
            continue;
        }
        slot->deathTransitionTimer += dt;
        if (slot->deathTransitionTimer < BubbleDelay) {
            continue;
        }

        PlayerSlot& survivor = otherPlayer(*slot);
        slot->character->setActive(false);
        if (slot->events.lives > 0 &&
            survivor.lifeState == DuoLifeState::Active &&
            survivor.character->isActive() &&
            !survivor.character->isDying()) {
            slot->lifeState = DuoLifeState::Bubble;
            slot->bubbleTimer = 0.f;
            slot->bubblePhase = 0.f;
            slot->bubblePosition = survivor.character->getPosition() +
                sf::Vector2f{
                    slot->id == DuoPlayerId::One ? -26.f : 26.f,
                    -34.f};
        } else {
            slot->lifeState = DuoLifeState::Out;
        }
    }
}

void DuoState::updateBubbles(float dt) {
    for (PlayerSlot* slot : {&playerOne, &playerTwo}) {
        if (slot->lifeState != DuoLifeState::Bubble) {
            continue;
        }
        PlayerSlot& survivor = otherPlayer(*slot);
        if (survivor.lifeState != DuoLifeState::Active ||
            !survivor.character->isActive() ||
            survivor.character->isDying()) {
            continue;
        }

        slot->bubbleTimer += dt;
        slot->bubblePhase += dt * 3.5f;
        const sf::Vector2f target = survivor.character->getPosition() +
            sf::Vector2f{
                slot->id == DuoPlayerId::One ? -26.f : 26.f,
                -34.f + std::sin(slot->bubblePhase) * 5.f};
        const float follow = std::min(1.f, dt * BubbleFollowSpeed);
        slot->bubblePosition +=
            (target - slot->bubblePosition) * follow;

        const sf::FloatRect bubbleBounds{
            slot->bubblePosition.x - 13.f,
            slot->bubblePosition.y - 13.f,
            26.f,
            26.f};
        sf::FloatRect overlap;
        const bool airborneTouch = !survivor.character->isGrounded() &&
            CollisionManager::checkAABB(
                survivor.character->getBounds(), bubbleBounds, overlap);
        if (airborneTouch || slot->bubbleTimer >= BubbleAutoRescueTime) {
            rescuePlayer(*slot, survivor);
        }
    }
}

void DuoState::rescuePlayer(
    PlayerSlot& bubble,
    PlayerSlot& rescuer) {
    const sf::FloatRect oldBounds = bubble.character->getBounds();
    const sf::Vector2f requested =
        rescuer.character->getPosition() + sf::Vector2f{0.f, -24.f};
    const sf::Vector2f spawn = level.findSafeSpawnNear(
        requested,
        {std::max(1.f, oldBounds.width),
         std::max(1.f, oldBounds.height)});
    bubble.character->respawn(spawn.x, spawn.y);
    bubble.character->addEffect(
        std::make_unique<DamageInvincibilityEffect>(RespawnProtection));
    bubble.lifeState = DuoLifeState::Active;
    bubble.deathTransitionTimer = 0.f;
    bubble.bubbleTimer = 0.f;
    bubble.previousBounds = bubble.character->getBounds();
    ++bubble.events.stats.rescuesReceived;
    ++rescuer.events.stats.rescuesPerformed;
    SoundManager::getInstance().playSound("powerupcollect");
}

bool DuoState::hasActivePlayer() const {
    return (playerOne.lifeState == DuoLifeState::Active &&
            playerOne.character && playerOne.character->isActive() &&
            !playerOne.character->isDying()) ||
           (playerTwo.lifeState == DuoLifeState::Active &&
            playerTwo.character && playerTwo.character->isActive() &&
            !playerTwo.character->isDying());
}

void DuoState::beginTeamWipe() {
    if (teamWipe || finishPending) {
        return;
    }
    teamWipe = true;
    teamWipeTimer = 0.f;
    timeFrozen = true;
    SoundManager::getInstance().stopBGM();
}

bool DuoState::startPortal(
    PlayerSlot& initiator,
    PortalActivation activation) {
    if (portalSequence.active || portalCooldown > 0.f || teamWipe ||
        finishPending || initiator.lifeState != DuoLifeState::Active ||
        initiator.finished || !initiator.character->isActive() ||
        initiator.character->isDying()) {
        return false;
    }
    const auto transition =
        level.queryPortalForInput(*initiator.character, activation);
    if (!transition) {
        return false;
    }
    portalSequence.active = true;
    portalSequence.transition = *transition;
    portalSequence.activation = activation;
    portalSequence.initiator = initiator.id;
    portalSequence.timer = 0.f;
    for (PlayerSlot* slot : {&playerOne, &playerTwo}) {
        if (slot->lifeState == DuoLifeState::Active) {
            slot->character->setRunning(false);
            slot->character->setJumpHeld(false);
            slot->character->setVelocity(0.f, 0.f);
        }
    }
    return true;
}

void DuoState::updatePortalSequence(float dt) {
    if (!portalSequence.active) {
        return;
    }
    portalSequence.timer += dt;
    PlayerSlot& initiator = slotFor(portalSequence.initiator);
    if (initiator.lifeState == DuoLifeState::Active &&
        initiator.character->isActive()) {
        const float movement = 30.f * dt;
        if (portalSequence.activation == PortalActivation::Down) {
            initiator.character->move(0.f, movement);
        } else {
            initiator.character->move(movement, 0.f);
        }
    }
    if (portalSequence.timer < PortalEnterDuration) {
        return;
    }

    std::vector<Character*> party;
    // Keep the initiator at the exact anchor and place the companion through
    // Level's safe-offset search.
    if (initiator.lifeState == DuoLifeState::Active) {
        party.push_back(initiator.character.get());
    }
    PlayerSlot& companion = otherPlayer(initiator);
    if (companion.lifeState == DuoLifeState::Active) {
        party.push_back(companion.character.get());
    }
    level.activatePortalForParty(portalSequence.transition, party);

    for (PlayerSlot* slot : {&playerOne, &playerTwo}) {
        if (slot->lifeState == DuoLifeState::Bubble && !party.empty()) {
            slot->bubblePosition = party.front()->getPosition() +
                sf::Vector2f{0.f, -32.f};
        }
    }
    portalSequence.active = false;
    portalSequence.timer = 0.f;
    portalCooldown = PortalReentryCooldown;
    updateCamera();
}

void DuoState::checkHeldPortalInput() {
    if (portalSequence.active || portalCooldown > 0.f || finishPending ||
        teamWipe) {
        return;
    }
    if (playerOne.input.isHeld(InputAction::Crouch) &&
        startPortal(playerOne, PortalActivation::Down)) {
        return;
    }
    if (playerTwo.input.isHeld(InputAction::Crouch) &&
        startPortal(playerTwo, PortalActivation::Down)) {
        return;
    }
    if (playerOne.input.isHeld(InputAction::MoveRight) &&
        startPortal(playerOne, PortalActivation::Right)) {
        return;
    }
    if (playerTwo.input.isHeld(InputAction::MoveRight)) {
        startPortal(playerTwo, PortalActivation::Right);
    }
}

bool DuoState::touchesFlagpole(
    const PlayerSlot& slot,
    float& height) const {
    if (slot.lifeState != DuoLifeState::Active || slot.finished ||
        !slot.character->isActive() || slot.character->isDying()) {
        return false;
    }
    const auto flagpole = level.findFlagpoleBounds();
    if (!flagpole ||
        !slot.character->getBounds().intersects(*flagpole)) {
        return false;
    }
    const sf::FloatRect bounds = slot.character->getBounds();
    const float base = flagpole->top + flagpole->height;
    height = std::max(0.f, base - (bounds.top + bounds.height));
    return true;
}

void DuoState::checkLevelCompletion(float dt) {
    if (transitionQueued || teamWipe) {
        return;
    }

    bool newFlagTouch = false;
    for (PlayerSlot* slot : {&playerOne, &playerTwo}) {
        float height = -1.f;
        if (!touchesFlagpole(*slot, height)) {
            continue;
        }
        slot->events.stats.flagHeight = height;
        slot->finished = true;
        slot->character->setVelocity(0.f, 0.f);
        newFlagTouch = true;
    }
    if (newFlagTouch && !finishPending) {
        finishPending = true;
        completedByFlag = true;
        finishTimer = 0.f;
        timeFrozen = true;
        SoundManager::getInstance().stopBGM();
        SoundManager::getInstance().playSound("flagraise");
    }

    if (!finishPending) {
        for (const auto& enemy : level.getEnemies()) {
            const auto* dragon = dynamic_cast<const DragonLugia*>(enemy.get());
            if (dragon && dragon->isBossDefeated()) {
                finishPending = true;
                completedByFlag = false;
                finishTimer = 0.f;
                timeFrozen = true;
                for (PlayerSlot* slot : {&playerOne, &playerTwo}) {
                    if (slot->lifeState == DuoLifeState::Active) {
                        slot->finished = true;
                        slot->character->setVelocity(0.f, 0.f);
                    }
                }
                SoundManager::getInstance().stopBGM();
                SoundManager::getInstance().playSound("castleclear");
                break;
            }
        }
    }

    if (!finishPending) {
        return;
    }
    finishTimer += dt;
    const bool bothFinished =
        (playerOne.finished ||
         playerOne.lifeState != DuoLifeState::Active) &&
        (playerTwo.finished ||
         playerTwo.lifeState != DuoLifeState::Active);
    const float requiredDelay = completedByFlag
        ? FlagJoinWindow
        : BossFinishDelay;
    if ((completedByFlag && bothFinished) ||
        finishTimer >= requiredDelay) {
        completeLevel();
    }
}

void DuoState::completeLevel() {
    if (transitionQueued || !stateManager) {
        return;
    }
    transitionQueued = true;
    DuoLevelResult result;
    result.session = session;
    result.stageName = stageName;
    result.nextStage = level.getNextStage();
    result.playerOne = playerOne.events.stats;
    result.playerTwo = playerTwo.events.stats;
    result.playerOneLives = playerOne.events.lives;
    result.playerTwoLives = playerTwo.events.lives;
    result.timeBonus =
        static_cast<int>(std::max(0.f, timeRemaining) * 50.f);
    result.completedByFlag = completedByFlag;

    const int teamScore = result.playerOne.score +
                          result.playerTwo.score +
                          result.timeBonus;
    AchievementSystem::getInstance().completeLevel(
        level.getLevelId(), teamScore);
    stateManager->changeState(
        std::make_unique<DuoLevelCompleteState>(std::move(result)));
}

void DuoState::update(float dt) {
    if (levelLoadFailed) {
        if (stateManager && !transitionQueued) {
            transitionQueued = true;
            stateManager->clearAndPushState(
                std::make_unique<MenuState>(MenuState::Page::DuoPlay));
        }
        return;
    }
    if (transitionQueued) {
        return;
    }

    portalCooldown = std::max(0.f, portalCooldown - dt);
    if (teamWipe) {
        for (PlayerSlot* slot : {&playerOne, &playerTwo}) {
            if (slot->lifeState == DuoLifeState::Dying &&
                slot->character->isActive()) {
                slot->character->update(dt);
            }
        }
        teamWipeTimer += dt;
        if (teamWipeTimer >= TeamWipeDelay && stateManager) {
            transitionQueued = true;
            const int score = playerOne.events.stats.score +
                              playerTwo.events.stats.score;
            stateManager->changeState(
                std::make_unique<GameOverState>(score, session));
        }
        return;
    }

    if (portalSequence.active) {
        updatePortalSequence(dt);
        return;
    }

    if (!timeFrozen) {
        timeRemaining = std::max(0.f, timeRemaining - dt);
        if (timeRemaining <= 0.f) {
            for (PlayerSlot* slot : {&playerOne, &playerTwo}) {
                if (slot->lifeState == DuoLifeState::Active &&
                    !slot->character->isDying()) {
                    slot->character->die(DeathCause::TimeOut);
                }
            }
        }
    }

    updateBossTargets(dt);
    level.update(dt);

    // Boss support drops (Doubled for Duo mode)
    const DragonLugia* stageBoss = nullptr;
    for (const auto& enemy : level.getEnemies()) {
        if (const auto* dragon = dynamic_cast<const DragonLugia*>(enemy.get())) {
            stageBoss = dragon;
            break;
        }
    }

    if (!finishPending && !teamWipe && stageBoss && !stageBoss->isBossDefeated() && hasActivePlayer()) {
        skyDropTimer -= dt;
        if (skyDropTimer <= 0.f) {
            skyDropTimer = 6.5f + static_cast<float>(rand() % 40) / 10.f; // 6.5s - 10.5s

            int dropType = rand() % 100;
            if (dropType < 55) {
                // 55% chance: Powerup Items (Mushroom, FireFlower, StarItem) - DOUBLED for Duo mode!
                struct SurfaceSpot {
                    float x;
                    float y;
                };
                static const SurfaceSpot surfaces[] = {
                    {80.f, 192.f},   // Ground Left
                    {200.f, 192.f},  // Ground Center
                    {310.f, 192.f},  // Ground Right
                    {88.f, 128.f},   // Left Blue Platform (row 9)
                    {184.f, 80.f},   // Middle Blue Platform (row 6)
                    {300.f, 112.f}   // Right Blue Platform (row 8)
                };

                static const char* itemTypes[] = {"Mushroom", "FireFlower", "StarItem"};
                int itemIdx = rand() % 3;
                std::string selectedItem = itemTypes[itemIdx];

                // If at least one active player is not Small (e.g. Super, Fire), spawn 2 FireFlowers instead of Mushrooms
                bool hasBigPlayer = false;
                for (const PlayerSlot* slot : {&playerOne, &playerTwo}) {
                    if (slot->lifeState == DuoLifeState::Active && slot->character &&
                        slot->character->getCurrentFormName() != "Small") {
                        hasBigPlayer = true;
                        break;
                    }
                }
                if (selectedItem == "Mushroom" && hasBigPlayer) {
                    selectedItem = "FireFlower";
                }

                // In Duo mode, spawn 2 items at 2 distinct surface locations
                int spotIdx1 = rand() % 6;
                int spotIdx2 = (spotIdx1 + 1 + rand() % 5) % 6;

                sf::Vector2f pos1(surfaces[spotIdx1].x + static_cast<float>(rand() % 16 - 8), surfaces[spotIdx1].y);
                sf::Vector2f pos2(surfaces[spotIdx2].x + static_cast<float>(rand() % 16 - 8), surfaces[spotIdx2].y);

                bool spawned = false;
                if (auto itemEnt1 = EntityFactory::getInstance().create(selectedItem, pos1)) {
                    if (auto* item1 = dynamic_cast<Item*>(itemEnt1.get())) {
                        itemEnt1.release();
                        level.getItems().push_back(std::unique_ptr<Item>(item1));
                        spawned = true;
                    }
                }
                if (auto itemEnt2 = EntityFactory::getInstance().create(selectedItem, pos2)) {
                    if (auto* item2 = dynamic_cast<Item*>(itemEnt2.get())) {
                        itemEnt2.release();
                        level.getItems().push_back(std::unique_ptr<Item>(item2));
                        spawned = true;
                    }
                }
                if (spawned) {
                    SoundManager::getInstance().playSound("powerupappear");
                }
            } else {
                // 45% chance: Goomba drops from ceiling (2 Goombas for Duo mode)
                for (int i = 0; i < 2; ++i) {
                    float spawnX = 60.f + static_cast<float>(rand() % 280);
                    float spawnY = 20.f + static_cast<float>(rand() % 10);
                    if (auto enemyEnt = EntityFactory::getInstance().create("Goomba", {spawnX, spawnY})) {
                        if (auto* enemy = dynamic_cast<Enemy*>(enemyEnt.get())) {
                            enemy->setActivated(true);
                            enemyEnt.release();
                            level.getEnemies().push_back(std::unique_ptr<Enemy>(enemy));
                        }
                    }
                }
            }
        }
    }

    InputPermissions onePermissions;
    InputPermissions twoPermissions;
    if (playerOne.lifeState == DuoLifeState::Active &&
        playerTwo.lifeState == DuoLifeState::Active &&
        !playerOne.finished && !playerTwo.finished) {
        const DuoTetherResult tether = DuoRules::calculateTether(
            playerOne.character->getBounds(),
            playerTwo.character->getBounds(),
            maximumPlayerSeparation());
        onePermissions = tether.playerOne;
        twoPermissions = tether.playerTwo;
    }

    updatePlayer(playerOne, dt, onePermissions);
    updatePlayer(playerTwo, dt, twoPermissions);
    resolveEnemyContacts(playerOne);
    resolveEnemyContacts(playerTwo);
    resolveItemContacts();
    resolveBoostJump();
    enforceTether();
    updateProjectiles(dt);
    processDeathEvents();
    updateDownedPlayers(dt);
    updateBubbles(dt);

    if (!teamWipe) {
        checkHeldPortalInput();
        checkLevelCompletion(dt);
        updateCamera();
    }
}

DuoState::PlayerSlot& DuoState::otherPlayer(PlayerSlot& slot) {
    return slot.id == DuoPlayerId::One ? playerTwo : playerOne;
}

const DuoState::PlayerSlot& DuoState::otherPlayer(
    const PlayerSlot& slot) const {
    return slot.id == DuoPlayerId::One ? playerTwo : playerOne;
}

DuoState::PlayerSlot& DuoState::slotFor(DuoPlayerId id) {
    return id == DuoPlayerId::One ? playerOne : playerTwo;
}

const DuoState::PlayerSlot& DuoState::slotFor(DuoPlayerId id) const {
    return id == DuoPlayerId::One ? playerOne : playerTwo;
}

DuoHudPlayerData DuoState::makeHudData(
    const PlayerSlot& slot,
    const char* label) const {
    return {
        label,
        characterName(slot.characterChoice),
        slot.character
            ? std::string{slot.character->getCurrentFormName()}
            : std::string{"SMALL"},
        slot.events.stats.score,
        slot.events.stats.coins,
        slot.events.lives,
        slot.lifeState};
}

void DuoState::renderBubble(
    sf::RenderWindow& window,
    const PlayerSlot& slot) const {
    if (slot.lifeState != DuoLifeState::Bubble) {
        return;
    }
    sf::CircleShape bubble{13.f};
    bubble.setOrigin(13.f, 13.f);
    bubble.setPosition(slot.bubblePosition);
    bubble.setFillColor(sf::Color{210, 240, 255, 100});
    bubble.setOutlineColor(sf::Color::White);
    bubble.setOutlineThickness(1.5f);
    window.draw(bubble);

    sf::CircleShape core{5.f};
    core.setOrigin(5.f, 5.f);
    core.setPosition(slot.bubblePosition);
    core.setFillColor(
        slot.id == DuoPlayerId::One
            ? sf::Color{235, 70, 55, 220}
            : sf::Color{45, 205, 90, 220});
    window.draw(core);

    sf::CircleShape shine{2.5f};
    shine.setPosition(
        slot.bubblePosition.x - 7.f,
        slot.bubblePosition.y - 8.f);
    shine.setFillColor(sf::Color{255, 255, 255, 210});
    window.draw(shine);
}

void DuoState::render(sf::RenderWindow& window) {
    if (levelLoadFailed) {
        window.clear(sf::Color::Black);
        return;
    }

    level.getCamera().applyTo(window);
    window.clear(level.usesDarkBackground()
                     ? sf::Color::Black
                     : sf::Color{92, 148, 252});
    level.render(window);

    if (playerOne.character && playerOne.character->isActive()) {
        playerOne.character->render(window);
    }
    if (playerTwo.character && playerTwo.character->isActive()) {
        playerTwo.character->render(window);
    }
    renderBubble(window, playerOne);
    renderBubble(window, playerTwo);

    for (const auto& fireball : fireballs) {
        if (fireball.projectile && fireball.projectile->isActive()) {
            fireball.projectile->render(window);
        }
    }

    if (playerOne.lifeState == DuoLifeState::Active) {
        hud.renderPlayerMarker(window, *playerOne.character, "P1");
    }
    if (playerTwo.lifeState == DuoLifeState::Active) {
        hud.renderPlayerMarker(window, *playerTwo.character, "P2");
    }
    hud.render(
        window,
        makeHudData(playerOne, "P1"),
        makeHudData(playerTwo, "P2"),
        stageName,
        timeRemaining);
}
