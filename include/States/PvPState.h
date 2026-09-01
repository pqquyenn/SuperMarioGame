#pragma once

#include "AdminControl/DebugMovementTrail.h"
#include "Core/GameSettings.h"
#include "Entities/PlayerPalette.h"
#include "Input/InputHandler.h"
#include "Level/Level.h"
#include "Observer/Observer.h"
#include "Observer/Subject.h"
#include "PvP/PvPTypes.h"
#include "PvP/PvPPlayerSession.h"
#include "PvP/PvPRuleset.h"
#include "States/GameState.h"

#include <SFML/Graphics.hpp>

#include <memory>
#include <random>
#include <string>
#include <vector>

class Character;
class FireFlower;
class Fireball;

class PvPState : public GameState {
private:
    struct PlayerSlot {
        CharacterChoice characterChoice;
        PlayerPalette palette{PlayerPalette::Primary};
        std::unique_ptr<Character> character;
        InputHandler input;
        PvPPlayerSession session;
        sf::Vector2f spawnPoint{0.f, 0.f};
        sf::FloatRect previousBounds;
        ObserverConnection scoreConnection;

        PlayerSlot(PlayerId playerId,
                   CharacterChoice choice,
                   BindingTarget bindingTarget,
                   int startingLives);
    };

    struct OwnedFireball {
        PlayerId owner;
        std::unique_ptr<Fireball> projectile;
    };

    PvPMatchType matchType;
    PvPRuleset ruleset;
    // The ruleset and arena are independent: callers may reuse Small or
    // Super PvP rules with any compatible data-driven level manifest.
    std::string arenaMapPath;
    Level level;
    PlayerSlot playerOne;
    PlayerSlot playerTwo;
    std::vector<OwnedFireball> fireballs;
    std::unique_ptr<FireFlower> fireFlower;
    std::vector<sf::Vector2f> fireFlowerSpawns;
    std::mt19937 randomEngine;
    bool sameCharacterMatch{false};

    sf::Font font;
    bool fontLoaded{false};
    bool arenaLoadFailed{false};
    bool debugVisible{false};
    DebugMovementTrail playerOneTrail;
    DebugMovementTrail playerTwoTrail;
    bool matchOver{false};
    std::string resultText;
    float flowerSpawnTimer{0.f};
    float matchTimeRemaining{0.f};
    float friendlyRespawnTimer{0.f};

    void createPlayers();
    sf::Vector2f findAnchor(const std::string& id,
                            sf::Vector2f fallback) const;
    void configureArenaCamera();
    void updateArenaViewport(const sf::Vector2u& windowSize);
    void updatePlayer(PlayerSlot& slot, float dt);
    void respawnIfReady(PlayerSlot& slot);
    bool applyDamage(PlayerSlot& slot, PvPDamageSource source);
    void evaluateWinner();
    void resolvePlayerContact();
    void pushPlayersApart();
    void resolveEnemyContacts(PlayerSlot& slot);
    void updateProjectiles(float dt);
    void requestFireball(PlayerSlot& owner,
                         const struct ProjectileRequest& request);
    std::size_t activeProjectileCount(PlayerId owner) const;
    void updateFireFlower(float dt);
    void spawnFireFlower();
    void cacheFireFlowerSpawns();
    void respawnFriendlyArena();
    float randomSeconds(float minimum, float maximum);
    void constrainToArena(PlayerSlot& slot);
    void loadFont();
    void renderHud(sf::RenderWindow& window);
    void renderPlayerMarkers(sf::RenderWindow& window);
    void renderDebug(sf::RenderWindow& window);
    bool isFriendlyMatch() const {
        return matchType == PvPMatchType::Friendly;
    }

public:
    explicit PvPState(
        PvPMatchType type,
        std::string mapPath = {},
        CharacterChoice playerOneChoice = CharacterChoice::Mario,
        CharacterChoice playerTwoChoice = CharacterChoice::Luigi
    );
    ~PvPState() override;

    void onEnter() override;
    void onExit() override;
    void handleInput(sf::Event& event, sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
