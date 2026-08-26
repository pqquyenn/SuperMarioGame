#include "States/PvPState.h"

#include "Core/AssetManager.h"
#include "Core/SoundManager.h"
#include "Entities/Character.h"
#include "Entities/Enemies/Enemy.h"
#include "Entities/Fireball.h"
#include "Entities/Items/FireFlower.h"
#include "Entities/Luigi.h"
#include "Entities/Mario.h"
#include "Factories/EntityFactory.h"
#include "Physics/CollisionManager.h"
#include "PlayerEffects/DamageInvincibilityEffect.h"
#include "PlayerStates/FireState.h"
#include "PlayerStates/SuperState.h"
#include "PvP/PvPCombatResolver.h"
#include "States/GameStateManager.h"
#include "States/MenuState.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

namespace {
constexpr float ArenaViewWidth = 400.f;
constexpr float ArenaViewHeight = 225.f;
constexpr float PvPMovementScale = 0.82f;
constexpr float SpawnProtectionDuration = 1.5f;
constexpr float FireballCooldown = 0.75f;
constexpr float StompBounceVelocity = -250.f;
constexpr float ContactPushVelocity = 135.f;
constexpr float FriendlyRespawnInterval = 15.f;
constexpr float UiWidth = 800.f;
constexpr float UiHeight = 600.f;

KeyBinding only(sf::Keyboard::Key key) {
    return KeyBinding{key, sf::Keyboard::Unknown, sf::Keyboard::Unknown};
}

float bottom(const sf::FloatRect& bounds) {
    return bounds.top + bounds.height;
}

float centerX(const sf::FloatRect& bounds) {
    return bounds.left + bounds.width * 0.5f;
}

void centerText(sf::Text& text, float x, float y) {
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width * 0.5f,
                   bounds.top + bounds.height * 0.5f);
    text.setPosition(x, y);
}

const char* characterName(CharacterChoice choice) {
    return choice == CharacterChoice::Luigi ? "LUIGI" : "MARIO";
}

std::unique_ptr<Character> makeCharacter(CharacterChoice choice) {
    if (choice == CharacterChoice::Luigi) {
        return std::make_unique<Luigi>();
    }
    return std::make_unique<Mario>();
}
}

PvPState::PlayerSlot::PlayerSlot(
    PlayerId playerId,
    CharacterChoice choice,
    const InputBindings& bindings
)
    : id{playerId}, characterChoice{choice}, input{bindings, false} {}

void PvPState::PlayerScore::onNotify(const GameEvent& event) {
    if (event.type == GameEventType::COIN_COLLECTED) {
        ++coins;
        score += event.value;
    } else if (event.type == GameEventType::ENEMY_DEFEATED) {
        score += event.value;
    }
}

InputBindings PvPState::makePlayerOneBindings() {
    InputBindings bindings;
    bindings.moveLeft = only(sf::Keyboard::A);
    bindings.moveRight = only(sf::Keyboard::D);
    bindings.jump = only(sf::Keyboard::W);
    bindings.crouch = only(sf::Keyboard::S);
    bindings.action = only(sf::Keyboard::Z);
    bindings.run = only(sf::Keyboard::LShift);
    return bindings;
}

InputBindings PvPState::makePlayerTwoBindings() {
    InputBindings bindings;
    bindings.moveLeft = only(sf::Keyboard::Left);
    bindings.moveRight = only(sf::Keyboard::Right);
    bindings.jump = only(sf::Keyboard::Up);
    bindings.crouch = only(sf::Keyboard::Down);
    bindings.action = only(sf::Keyboard::J);
    bindings.run = only(sf::Keyboard::RShift);
    return bindings;
}

PvPState::PvPState(
    PvPMatchType type,
    std::string mapPath,
    CharacterChoice playerOneChoice,
    CharacterChoice playerTwoChoice
)
    : matchType{type},
      arenaMapPath{mapPath.empty()
          ? (type == PvPMatchType::Small
                 ? "pvp/small-arena.level"
                 : "pvp/super-arena.level")
          : std::move(mapPath)},
      playerOne{PlayerId::One, playerOneChoice, makePlayerOneBindings()},
      playerTwo{PlayerId::Two, playerTwoChoice, makePlayerTwoBindings()},
      randomEngine{std::random_device{}()} {}

PvPState::~PvPState() = default;

void PvPState::onEnter() {
    AssetManager::getInstance().loadLevelAssets();
    arenaLoadFailed = !level.loadLevel(arenaMapPath);
    if (arenaLoadFailed) {
        std::cerr << "[PvPState] Failed to load arena " << arenaMapPath
                  << "; returning to the main menu." << std::endl;
        loadFont();
        return;
    }

    createPlayers();
    configureArenaCamera();
    loadFont();

    cacheFireFlowerSpawns();
    matchTimeRemaining = isFriendlyMatch()
        ? static_cast<float>(level.getTimeLimit())
        : 0.f;
    friendlyRespawnTimer = FriendlyRespawnInterval;
    flowerSpawnTimer = 3.f;
}

void PvPState::cacheFireFlowerSpawns() {
    fireFlowerSpawns.clear();
    const float tileSize = level.getDefinition().tileSize;
    for (const auto& anchor : level.getDefinition().anchors) {
        if (anchor.id.find("fire_spawn_") == 0) {
            fireFlowerSpawns.push_back({anchor.tilePosition.x * tileSize,
                                        anchor.tilePosition.y * tileSize});
        }
    }
}

void PvPState::respawnFriendlyArena() {
    fireballs.clear();
    fireFlower.reset();
    if (!level.loadLevel(arenaMapPath)) {
        arenaLoadFailed = true;
        return;
    }
    configureArenaCamera();
    cacheFireFlowerSpawns();
    flowerSpawnTimer = 3.f;
    friendlyRespawnTimer = FriendlyRespawnInterval;
}

void PvPState::onExit() {
    fireballs.clear();
    fireFlower.reset();
}

sf::Vector2f PvPState::findAnchor(
    const std::string& id,
    sf::Vector2f fallback
) const {
    if (!level.isDataDriven()) {
        return fallback;
    }
    const float tileSize = level.getDefinition().tileSize;
    for (const auto& anchor : level.getDefinition().anchors) {
        if (anchor.id == id) {
            return {anchor.tilePosition.x * tileSize,
                    anchor.tilePosition.y * tileSize};
        }
    }
    return fallback;
}

void PvPState::createPlayers() {
    playerOne.character = makeCharacter(playerOne.characterChoice);
    playerTwo.character = makeCharacter(playerTwo.characterChoice);
    playerOne.palette = PlayerPalette::Primary;
    playerTwo.palette = PlayerPalette::Primary;
    sameCharacterMatch =
        playerOne.characterChoice == playerTwo.characterChoice;

    AssetManager& assets = AssetManager::getInstance();
    auto& playerTexture =
        assets.getPlayerTexture(PlayerPalette::Primary);
    playerOne.character->setTexture(playerTexture);
    if (sameCharacterMatch) {
        sf::Texture& secondary =
            assets.getPlayerTexture(PlayerPalette::Secondary);
        if (&secondary != &playerTexture) {
            playerTwo.palette = PlayerPalette::Secondary;
        }
        playerTwo.character->setTexture(secondary);
    } else {
        playerTwo.character->setTexture(playerTexture);
    }
    playerOne.character->setHorizontalMovementScale(PvPMovementScale);
    playerTwo.character->setHorizontalMovementScale(PvPMovementScale);
    playerOne.scoreConnection = playerOne.character->addObserver(
        &playerOne.score);
    playerTwo.scoreConnection = playerTwo.character->addObserver(
        &playerTwo.score);

    playerOne.spawnPoint = findAnchor("p1_spawn", {48.f, 192.f});
    playerTwo.spawnPoint = findAnchor("p2_spawn", {336.f, 192.f});
    playerOne.character->setPosition(playerOne.spawnPoint);
    playerTwo.character->setPosition(playerTwo.spawnPoint);

    if (matchType != PvPMatchType::Small) {
        playerOne.character->receivePowerUp(std::make_unique<SuperState>());
        playerTwo.character->receivePowerUp(std::make_unique<SuperState>());
    }

    playerOne.character->setProjectileRequestHandler(
        [this](const ProjectileRequest& request) {
            requestFireball(playerOne, request);
        });
    playerTwo.character->setProjectileRequestHandler(
        [this](const ProjectileRequest& request) {
            requestFireball(playerTwo, request);
        });

    playerOne.spawnProtection = SpawnProtectionDuration;
    playerTwo.spawnProtection = SpawnProtectionDuration;
    playerOne.previousBounds = playerOne.character->getBounds();
    playerTwo.previousBounds = playerTwo.character->getBounds();
}

void PvPState::configureArenaCamera() {
    Camera& camera = level.getCamera();
    const float tileSize = level.getDefinition().tileSize;
    const float worldWidth = level.getTileMap().getMapWidth() * tileSize;
    const float worldHeight = level.getTileMap().getMapHeight() * tileSize;
    const float viewWidth = std::max(ArenaViewWidth, worldWidth);
    // The final terrain row is a backing row below the playable floor. Keep
    // it outside the view while allowing taller custom arenas to show their
    // actual ground instead of clipping it at the bottom edge.
    const float viewHeight = std::max(
        ArenaViewHeight, worldHeight - tileSize);

    camera.setSize(viewWidth, viewHeight);
    camera.setLevelBounds(worldWidth, worldHeight);
    camera.setCenter(viewWidth * 0.5f, viewHeight * 0.5f);
}

void PvPState::handleInput(sf::Event& event, sf::RenderWindow&) {
    if (arenaLoadFailed) {
        return;
    }

    if (event.type != sf::Event::KeyPressed) {
        return;
    }

    if (event.key.code == sf::Keyboard::T) {
        debugVisible = !debugVisible;
        return;
    }

    if (debugVisible && event.key.code == sf::Keyboard::Y &&
        playerOne.character && playerTwo.character) {
        playerOneTrail.start(*playerOne.character);
        playerTwoTrail.start(*playerTwo.character);
        return;
    }

    if (matchOver && event.key.code == sf::Keyboard::Enter) {
        if (stateManager) {
            stateManager->clearAndPushState(
                std::make_unique<PvPState>(
                    matchType,
                    arenaMapPath,
                    playerOne.characterChoice,
                    playerTwo.characterChoice));
        }
        return;
    }

    if (event.key.code == sf::Keyboard::Escape && stateManager) {
        stateManager->clearAndPushState(
            std::make_unique<MenuState>(MenuState::Page::PvP));
    }
}

void PvPState::updatePlayer(PlayerSlot& slot, float dt) {
    Character& character = *slot.character;
    slot.previousBounds = character.getBounds();
    slot.spawnProtection = std::max(0.f, slot.spawnProtection - dt);
    slot.fireCooldown = std::max(0.f, slot.fireCooldown - dt);

    slot.input.handleInput(character, dt);
    character.update(dt);

    if (!character.isActive()) {
        respawnIfReady(slot);
        return;
    }
    if (character.isDying()) {
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
    constrainToArena(slot);

    DebugMovementTrail& trail = slot.id == PlayerId::One
        ? playerOneTrail : playerTwoTrail;
    trail.update(character, dt);

    if (character.getPosition().y > level.getKillPlaneY()) {
        applyDamage(slot, PvPDamageSource::Void);
    }
}

void PvPState::respawnIfReady(PlayerSlot& slot) {
    if (slot.lives <= 0 || slot.character->isActive()) {
        return;
    }

    slot.character->respawn(slot.spawnPoint.x, slot.spawnPoint.y);
    if (matchType != PvPMatchType::Small) {
        slot.character->receivePowerUp(std::make_unique<SuperState>());
    }
    slot.character->addEffect(
        std::make_unique<DamageInvincibilityEffect>(
            SpawnProtectionDuration));
    slot.spawnProtection = SpawnProtectionDuration;
    slot.fireTimeRemaining = 0.f;
    slot.previousBounds = slot.character->getBounds();
}

bool PvPState::applyDamage(
    PlayerSlot& slot,
    PvPDamageSource source
) {
    Character& character = *slot.character;
    if (!character.isActive() || character.isDying() ||
        (source != PvPDamageSource::Void && slot.spawnProtection > 0.f)) {
        return false;
    }

    const std::string before{character.getCurrentFormName()};
    if (source == PvPDamageSource::Void) {
        character.die(DeathCause::Void);
    } else {
        character.takeDamage();
        if (matchType != PvPMatchType::Small &&
            before == "Super" &&
            character.getCurrentFormName() == "Small" &&
            !character.isDying()) {
            character.die(DeathCause::PvP);
        }
    }

    if (character.isDying()) {
        if (isFriendlyMatch()) {
            const int score = slot.score.score;
            if (score < 100) {
                slot.score.score = 0;
            } else if (score <= 1000) {
                slot.score.score = 5 * (score - 100) / 6;
            } else {
                slot.score.score = score - 250;
            }
        } else {
            slot.lives = std::max(0, slot.lives - 1);
        }
        slot.fireTimeRemaining = 0.f;
        SoundManager::getInstance().playSound("death");
        return true;
    }

    const bool changed = before != character.getCurrentFormName();
    if (before == "Fire" &&
        character.getCurrentFormName() == "Super") {
        slot.fireTimeRemaining = 0.f;
    }
    return changed;
}

void PvPState::resolvePlayerContact() {
    Character& one = *playerOne.character;
    Character& two = *playerTwo.character;
    if (!one.isActive() || one.isDying() ||
        !two.isActive() || two.isDying()) {
        return;
    }

    const PvPBodyFrame oneFrame{
        playerOne.previousBounds, one.getBounds(), one.getVelocity()};
    const PvPBodyFrame twoFrame{
        playerTwo.previousBounds, two.getBounds(), two.getVelocity()};
    const PvPContactOutcome outcome =
        PvPCombatResolver::classifyPlayerContact(oneFrame, twoFrame);

    if (outcome == PvPContactOutcome::None) {
        return;
    }

    if (isFriendlyMatch()) {
        pushPlayersApart();
        return;
    }

    if (outcome == PvPContactOutcome::PlayerOneStomps) {
        const sf::FloatRect target = two.getBounds();
        const sf::FloatRect attacker = one.getBounds();
        one.setPosition(one.getPosition().x,
                        target.top - attacker.height);
        one.setVelocity(one.getVelocity().x, StompBounceVelocity);
        applyDamage(playerTwo, PvPDamageSource::Stomp);
        if (!two.isDying()) {
            pushPlayersApart();
        }
        return;
    }

    if (outcome == PvPContactOutcome::PlayerTwoStomps) {
        const sf::FloatRect target = one.getBounds();
        const sf::FloatRect attacker = two.getBounds();
        two.setPosition(two.getPosition().x,
                        target.top - attacker.height);
        two.setVelocity(two.getVelocity().x, StompBounceVelocity);
        applyDamage(playerOne, PvPDamageSource::Stomp);
        if (!one.isDying()) {
            pushPlayersApart();
        }
        return;
    }

    pushPlayersApart();
}

void PvPState::pushPlayersApart() {
    Character& one = *playerOne.character;
    Character& two = *playerTwo.character;
    sf::FloatRect overlap;
    if (!CollisionManager::checkAABB(one.getBounds(), two.getBounds(),
                                     overlap)) {
        return;
    }

    const PvPPushDistribution push =
        PvPCombatResolver::calculatePushDistribution(
            {playerOne.previousBounds, one.getBounds(), one.getVelocity()},
            {playerTwo.previousBounds, two.getBounds(), two.getVelocity()});
    const float totalSeparation = overlap.width + 1.f;
    one.move(push.playerOneIsLeft
                 ? -totalSeparation * push.playerOneShare
                 : totalSeparation * push.playerOneShare,
             0.f);
    two.move(push.playerOneIsLeft
                 ? totalSeparation * push.playerTwoShare
                 : -totalSeparation * push.playerTwoShare,
             0.f);

    // Equal shares preserve the old recoil speed. A faster attacker receives
    // less recoil, while the slower/stationary player receives more.
    const float oneRecoil = ContactPushVelocity *
                            (0.5f + push.playerOneShare);
    const float twoRecoil = ContactPushVelocity *
                            (0.5f + push.playerTwoShare);
    one.setVelocity(push.playerOneIsLeft ? -oneRecoil : oneRecoil,
                    one.getVelocity().y);
    two.setVelocity(push.playerOneIsLeft ? twoRecoil : -twoRecoil,
                    two.getVelocity().y);
    constrainToArena(playerOne);
    constrainToArena(playerTwo);
}

void PvPState::resolveEnemyContacts(PlayerSlot& slot) {
    Character& character = *slot.character;
    if (!character.isActive() || character.isDying()) {
        return;
    }

    for (auto& enemy : level.getEnemies()) {
        if (!enemy || !enemy->isActive() || enemy->isSquished()) {
            continue;
        }
        sf::FloatRect overlap;
        if (!CollisionManager::checkAABB(character.getBounds(),
                                         enemy->getBounds(), overlap)) {
            continue;
        }

        const bool crossedTop =
            character.getVelocity().y > 25.f &&
            bottom(slot.previousBounds) <= enemy->getBounds().top + 2.f &&
            bottom(character.getBounds()) >= enemy->getBounds().top;
        if (enemy->canBeStomped() && crossedTop) {
            character.setPosition(
                character.getPosition().x,
                enemy->getBounds().top - character.getBounds().height);
            character.setVelocity(character.getVelocity().x,
                                  StompBounceVelocity);
            enemy->onStomped();
            character.notify(
                GameEvent::enemyDefeated(enemy->getScoreValue()));
        } else {
            applyDamage(slot, PvPDamageSource::Enemy);
        }
    }
}

void PvPState::requestFireball(
    PlayerSlot& owner,
    const ProjectileRequest& request
) {
    if (matchType == PvPMatchType::Small || matchOver ||
        request.type != ProjectileType::Fireball ||
        owner.fireCooldown > 0.f ||
        owner.character->getCurrentFormName() != "Fire" ||
        activeProjectileCount(owner.id) >= 1) {
        return;
    }

    fireballs.push_back(OwnedFireball{
        owner.id,
        std::make_unique<Fireball>(
            request.position.x,
            request.position.y,
            request.facingRight,
            AssetManager::getInstance().getTexture("BlockTileSheet"))});
    owner.fireCooldown = FireballCooldown;
    SoundManager::getInstance().playSound("fireball");
}

std::size_t PvPState::activeProjectileCount(PlayerId owner) const {
    return static_cast<std::size_t>(std::count_if(
        fireballs.begin(), fireballs.end(),
        [owner](const OwnedFireball& fireball) {
            return fireball.owner == owner && fireball.projectile &&
                   fireball.projectile->isActive();
        }));
}

void PvPState::updateProjectiles(float dt) {
    const sf::FloatRect arena = level.getCamera().getViewBounds();
    for (auto& owned : fireballs) {
        Fireball& fireball = *owned.projectile;
        if (!fireball.isActive()) {
            continue;
        }
        fireball.update(dt);
        CollisionManager::resolveTileCollisions(
            fireball, level.getTileMap(), &level);
        if (!fireball.isActive() || fireball.isExploding()) {
            continue;
        }

        PlayerSlot& target = owned.owner == PlayerId::One
            ? playerTwo : playerOne;
        sf::FloatRect overlap;
        if (!isFriendlyMatch() && target.character->isActive() &&
            !target.character->isDying() &&
            CollisionManager::checkAABB(fireball.getBounds(),
                                         target.character->getBounds(),
                                         overlap)) {
            applyDamage(target, PvPDamageSource::Fireball);
            fireball.explode();
            continue;
        }

        for (auto& enemy : level.getEnemies()) {
            if (enemy && enemy->isActive() &&
                CollisionManager::checkAABB(fireball.getBounds(),
                                             enemy->getBounds(), overlap)) {
                enemy->onFireball();
                PlayerSlot& owner = owned.owner == PlayerId::One
                    ? playerOne : playerTwo;
                owner.character->notify(
                    GameEvent::enemyDefeated(enemy->getScoreValue()));
                fireball.explode();
                break;
            }
        }

        if (fireball.isActive() &&
            !fireball.getBounds().intersects(arena)) {
            fireball.explode();
        }
    }

    fireballs.erase(
        std::remove_if(fireballs.begin(), fireballs.end(),
                       [](const OwnedFireball& owned) {
                           return !owned.projectile ||
                                  !owned.projectile->isActive();
                       }),
        fireballs.end());
}

float PvPState::randomSeconds(float minimum, float maximum) {
    return std::uniform_real_distribution<float>{minimum, maximum}(
        randomEngine);
}

void PvPState::spawnFireFlower() {
    if (fireFlowerSpawns.empty()) {
        return;
    }

    const std::size_t index = std::uniform_int_distribution<std::size_t>{
        0, fireFlowerSpawns.size() - 1}(randomEngine);
    auto entity = EntityFactory::getInstance().create(
        "FireFlower", fireFlowerSpawns[index]);
    if (auto* flower = dynamic_cast<FireFlower*>(entity.get())) {
        entity.release();
        fireFlower.reset(flower);
    }
}

void PvPState::updateFireFlower(float dt) {
    if (matchType == PvPMatchType::Small) {
        return;
    }

    auto updateFireTimer = [this, dt](PlayerSlot& slot) {
        if (slot.fireTimeRemaining <= 0.f ||
            !slot.character->isActive() || slot.character->isDying()) {
            return;
        }
        slot.fireTimeRemaining =
            std::max(0.f, slot.fireTimeRemaining - dt);
        if (slot.fireTimeRemaining <= 0.f &&
            slot.character->getCurrentFormName() == "Fire") {
            slot.character->receivePowerUp(std::make_unique<SuperState>());
        }
    };
    updateFireTimer(playerOne);
    updateFireTimer(playerTwo);

    if (!fireFlower) {
        flowerSpawnTimer -= dt;
        if (flowerSpawnTimer <= 0.f) {
            spawnFireFlower();
        }
        return;
    }

    fireFlower->update(dt);
    auto tryCollect = [this](PlayerSlot& slot) {
        if (!fireFlower || !fireFlower->isActive() ||
            !slot.character->isActive() || slot.character->isDying()) {
            return;
        }
        sf::FloatRect overlap;
        if (CollisionManager::checkAABB(fireFlower->getBounds(),
                                         slot.character->getBounds(),
                                         overlap) &&
            fireFlower->tryCollect(*slot.character)) {
            slot.fireTimeRemaining = randomSeconds(8.f, 15.f);
            SoundManager::getInstance().playSound("powerupcollect");
        }
    };
    tryCollect(playerOne);
    tryCollect(playerTwo);

    if (!fireFlower->isActive()) {
        fireFlower.reset();
        flowerSpawnTimer = randomSeconds(7.f, 12.f);
    }
}

void PvPState::constrainToArena(PlayerSlot& slot) {
    Character& character = *slot.character;
    const sf::FloatRect bounds = character.getBounds();
    const float minimum = level.getLeftBoundaryX();
    const float maximum = level.getRightBoundaryX(bounds.width);
    const float clamped = std::clamp(character.getPosition().x,
                                     minimum, maximum);
    if (clamped != character.getPosition().x) {
        character.setPosition(clamped, character.getPosition().y);
        character.setVelocity(0.f, character.getVelocity().y);
    }
}

void PvPState::evaluateWinner() {
    if (isFriendlyMatch()) {
        if (matchOver || matchTimeRemaining > 0.f) {
            return;
        }
        matchOver = true;
        if (playerOne.score.score == playerTwo.score.score) {
            resultText = "DRAW";
        } else if (playerOne.score.score > playerTwo.score.score) {
            resultText = std::string{"PLAYER 1 - "} +
                         characterName(playerOne.characterChoice) + " WINS";
        } else {
            resultText = std::string{"PLAYER 2 - "} +
                         characterName(playerTwo.characterChoice) + " WINS";
        }
        return;
    }
    if (matchOver || (playerOne.lives > 0 && playerTwo.lives > 0)) {
        return;
    }
    matchOver = true;
    if (playerOne.lives <= 0 && playerTwo.lives <= 0) {
        resultText = "DRAW";
    } else if (playerOne.lives <= 0) {
        resultText = std::string{"PLAYER 2 - "} +
                     characterName(playerTwo.characterChoice) + " WINS";
    } else {
        resultText = std::string{"PLAYER 1 - "} +
                     characterName(playerOne.characterChoice) + " WINS";
    }
}

void PvPState::update(float dt) {
    if (arenaLoadFailed) {
        if (stateManager) {
            stateManager->clearAndPushState(
                std::make_unique<MenuState>(MenuState::Page::PvP));
        }
        return;
    }

    if (matchOver) {
        return;
    }

    if (isFriendlyMatch()) {
        matchTimeRemaining = std::max(0.f, matchTimeRemaining - dt);
        friendlyRespawnTimer -= dt;
        if (matchTimeRemaining > 0.f && friendlyRespawnTimer <= 0.f) {
            respawnFriendlyArena();
            if (arenaLoadFailed) {
                return;
            }
        }
    }

    // Level entities move before contact resolution, while the fixed camera
    // remains independent of either player.
    level.update(dt);
    updatePlayer(playerOne, dt);
    updatePlayer(playerTwo, dt);
    resolvePlayerContact();
    resolveEnemyContacts(playerOne);
    resolveEnemyContacts(playerTwo);
    for (auto& item : level.getItems()) {
        if (!item || !item->isActive()) {
            continue;
        }
        CollisionManager::resolveEntityCollisions(
            *playerOne.character, *item);
        if (item->isActive()) {
            CollisionManager::resolveEntityCollisions(
                *playerTwo.character, *item);
        }
    }
    updateProjectiles(dt);
    updateFireFlower(dt);
    evaluateWinner();
}

void PvPState::loadFont() {
    const std::string candidates[] = {
        "assets/fonts/press-start-2p.ttf",
        "../assets/fonts/press-start-2p.ttf",
        "../../assets/fonts/press-start-2p.ttf"
    };
    for (const auto& path : candidates) {
        if (std::filesystem::exists(path) && font.loadFromFile(path)) {
            fontLoaded = true;
            return;
        }
    }
}

void PvPState::renderHud(sf::RenderWindow& window) {
    if (!fontLoaded) {
        return;
    }

    const sf::View previous = window.getView();
    window.setView(sf::View{sf::FloatRect{0.f, 0.f, UiWidth, UiHeight}});

    sf::RectangleShape header{{UiWidth, 72.f}};
    header.setFillColor(sf::Color{0, 0, 0, 190});
    window.draw(header);

    auto playerLabel = [this](const PlayerSlot& slot,
                              const std::string& name) {
        std::ostringstream text;
        text << (slot.id == PlayerId::One ? "P1 " : "P2 ") << name;
        if (isFriendlyMatch()) {
            text << "  SCORE " << std::setw(6) << std::setfill('0')
                 << slot.score.score << "  COIN " << slot.score.coins;
        } else {
            text << "  x" << slot.lives << "  "
                 << slot.character->getCurrentFormName();
        }
        if (slot.fireTimeRemaining > 0.f) {
            text << "  FIRE " << std::fixed << std::setprecision(1)
                 << slot.fireTimeRemaining << "s";
        }
        return text.str();
    };

    sf::Text left{
        playerLabel(playerOne, characterName(playerOne.characterChoice)),
        font,
        13};
    left.setPosition(22.f, 18.f);
    left.setFillColor(sf::Color{255, 110, 90});
    window.draw(left);

    sf::Text right{
        playerLabel(playerTwo, characterName(playerTwo.characterChoice)),
        font,
        13};
    const sf::FloatRect rightBounds = right.getLocalBounds();
    right.setPosition(UiWidth - rightBounds.width - 24.f, 18.f);
    right.setFillColor(sf::Color{110, 255, 130});
    window.draw(right);

    std::ostringstream modeLabel;
    modeLabel << pvpMatchName(matchType);
    if (isFriendlyMatch()) {
        modeLabel << "  TIME " << static_cast<int>(std::ceil(matchTimeRemaining))
                  << "  RESPAWN "
                  << static_cast<int>(std::ceil(friendlyRespawnTimer));
    }
    sf::Text mode{modeLabel.str(), font, 10};
    centerText(mode, UiWidth * 0.5f, 55.f);
    mode.setFillColor(sf::Color::Yellow);
    window.draw(mode);

    sf::RectangleShape controlsBackground{{UiWidth, 28.f}};
    controlsBackground.setPosition(0.f, UiHeight - 28.f);
    controlsBackground.setFillColor(sf::Color{0, 0, 0, 175});
    window.draw(controlsBackground);
    sf::Text controls{
        "P1: A/D MOVE  W JUMP  S CRAWL  LSHIFT RUN  Z FIRE"
        "     P2: ARROWS MOVE/JUMP/CRAWL  RSHIFT RUN  J FIRE",
        font,
        7};
    centerText(controls, UiWidth * 0.5f, UiHeight - 14.f);
    window.draw(controls);

    if (matchOver) {
        sf::RectangleShape overlay{{UiWidth, UiHeight}};
        overlay.setFillColor(sf::Color{0, 0, 0, 175});
        window.draw(overlay);
        sf::Text result{resultText, font, 22};
        result.setFillColor(sf::Color::Yellow);
        centerText(result, UiWidth * 0.5f, 270.f);
        window.draw(result);
        sf::Text prompt{"ENTER: REMATCH     ESC: PVP MENU", font, 10};
        centerText(prompt, UiWidth * 0.5f, 330.f);
        window.draw(prompt);
    }

    window.setView(previous);
}

void PvPState::renderPlayerMarkers(sf::RenderWindow& window) {
    if (!sameCharacterMatch || !fontLoaded) {
        return;
    }

    auto marker = [this, &window](const PlayerSlot& slot,
                                  const char* label,
                                  sf::Color color) {
        if (!slot.character || !slot.character->isActive()) {
            return;
        }
        const sf::FloatRect bounds = slot.character->getBounds();
        sf::Text text{label, font, 6};
        text.setFillColor(color);
        text.setOutlineColor(sf::Color::Black);
        text.setOutlineThickness(0.75f);
        centerText(text,
                   bounds.left + bounds.width * 0.5f,
                   bounds.top - 6.f);
        window.draw(text);
    };

    marker(playerOne, "P1", sf::Color{255, 100, 80});
    marker(playerTwo, "P2", sf::Color{80, 225, 255});
}

void PvPState::renderDebug(sf::RenderWindow& window) {
    if (!debugVisible) {
        return;
    }

    playerOneTrail.render(window, sf::Color{255, 90, 90});
    playerTwoTrail.render(window, sf::Color{90, 255, 120});

    auto outline = [&window](const Character& character,
                             sf::Color color) {
        sf::RectangleShape rectangle;
        const sf::FloatRect bounds = character.getBounds();
        rectangle.setPosition(bounds.left, bounds.top);
        rectangle.setSize({bounds.width, bounds.height});
        rectangle.setFillColor(sf::Color::Transparent);
        rectangle.setOutlineColor(color);
        rectangle.setOutlineThickness(1.f);
        window.draw(rectangle);
    };
    if (playerOne.character->isActive()) {
        outline(*playerOne.character, sf::Color::Red);
    }
    if (playerTwo.character->isActive()) {
        outline(*playerTwo.character, sf::Color::Green);
    }

    if (!fontLoaded) {
        return;
    }

    const sf::View previous = window.getView();
    window.setView(sf::View{sf::FloatRect{0.f, 0.f, UiWidth, UiHeight}});
    sf::RectangleShape panel{{560.f, 120.f}};
    panel.setPosition(8.f, 80.f);
    panel.setFillColor(sf::Color{0, 0, 0, 215});
    panel.setOutlineColor(sf::Color::Yellow);
    panel.setOutlineThickness(1.f);
    window.draw(panel);

    auto line = [](const PlayerSlot& slot, const char* label) {
        std::ostringstream text;
        const auto& c = *slot.character;
        text << label << " " << c.getCharacterType() << " "
             << c.getCurrentFormName() << "  POS "
             << static_cast<int>(c.getPosition().x) << ","
             << static_cast<int>(c.getPosition().y) << "  VEL "
             << static_cast<int>(c.getVelocity().x) << ","
             << static_cast<int>(c.getVelocity().y) << "  LIVES "
             << slot.lives;
        return text.str();
    };
    sf::Text debug{
        "ADMIN [T]  Y: 8s TRAIL  PVP: I/K/L DISABLED\n" +
        line(playerOne, "P1") + "\n" +
        line(playerTwo, "P2") + "\nCAMERA LOCKED  MOVE SCALE 0.82",
        font,
        9};
    debug.setPosition(18.f, 92.f);
    debug.setFillColor(sf::Color::White);
    window.draw(debug);
    window.setView(previous);
}

void PvPState::render(sf::RenderWindow& window) {
    if (arenaLoadFailed) {
        window.setView(sf::View{sf::FloatRect{0.f, 0.f, UiWidth, UiHeight}});
        window.clear(sf::Color::Black);
        if (fontLoaded) {
            sf::Text error{"FAILED TO LOAD PVP ARENA", font, 16};
            error.setFillColor(sf::Color::Red);
            centerText(error, UiWidth * 0.5f, UiHeight * 0.5f);
            window.draw(error);
        }
        return;
    }

    level.getCamera().applyTo(window);
    const sf::Color background = isFriendlyMatch()
        ? sf::Color{118, 196, 235}
        : (level.usesDarkBackground()
               ? sf::Color::Black
               : sf::Color{92, 148, 252});
    window.clear(background);
    level.render(window);
    if (fireFlower && fireFlower->isActive()) {
        fireFlower->render(window);
    }
    if (playerOne.character && playerOne.character->isActive()) {
        playerOne.character->render(window);
    }
    if (playerTwo.character && playerTwo.character->isActive()) {
        playerTwo.character->render(window);
    }
    renderPlayerMarkers(window);
    for (const auto& owned : fireballs) {
        if (owned.projectile && owned.projectile->isActive()) {
            owned.projectile->render(window);
        }
    }
    renderDebug(window);
    renderHud(window);
}
