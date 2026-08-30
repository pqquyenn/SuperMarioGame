#pragma once

#include "Duo/DuoTypes.h"
#include "Entities/PlayerPalette.h"
#include "Input/InputHandler.h"
#include "Level/Level.h"
#include "Observer/Observer.h"
#include "Observer/Subject.h"
#include "States/GameState.h"
#include "UI/DuoHUD.h"

#include <SFML/Graphics.hpp>

#include <memory>
#include <string>
#include <vector>

class Character;
class Fireball;

class DuoState : public GameState {
private:
    struct PlayerEvents final : Observer {
        DuoPlayerStats stats;
        int lives{3};
        bool deathRequested{false};

        explicit PlayerEvents(int startingLives = 3)
            : lives{startingLives} {}
        void onNotify(const GameEvent& event) override;
    };

    struct PlayerSlot {
        DuoPlayerId id;
        CharacterChoice characterChoice;
        PlayerPalette palette{PlayerPalette::Primary};
        std::unique_ptr<Character> character;
        InputHandler input;
        PlayerEvents events;
        ObserverConnection eventConnection;
        sf::Vector2f spawnPoint{0.f, 0.f};
        sf::FloatRect previousBounds;
        DuoLifeState lifeState{DuoLifeState::Active};
        float deathTransitionTimer{0.f};
        sf::Vector2f bubblePosition{0.f, 0.f};
        float bubbleTimer{0.f};
        float bubblePhase{0.f};
        float boostCooldown{0.f};
        bool finished{false};

        PlayerSlot(
            DuoPlayerId playerId,
            CharacterChoice choice,
            const InputBindings& bindings,
            int startingLives);
    };

    struct OwnedFireball {
        DuoPlayerId owner;
        std::unique_ptr<Fireball> projectile;
    };

    struct PortalSequence {
        bool active{false};
        PortalTransition transition;
        PortalActivation activation{PortalActivation::Down};
        DuoPlayerId initiator{DuoPlayerId::One};
        float timer{0.f};
    };

    DuoSessionConfig session;
    Level level;
    PlayerSlot playerOne;
    PlayerSlot playerTwo;
    std::vector<OwnedFireball> fireballs;
    DuoHUD hud;

    bool levelLoadFailed{false};
    bool teamWipe{false};
    float teamWipeTimer{0.f};
    bool finishPending{false};
    bool completedByFlag{false};
    float finishTimer{0.f};
    bool transitionQueued{false};
    float timeRemaining{400.f};
    bool timeFrozen{false};
    std::string stageName{"1-1"};
    PortalSequence portalSequence;
    float portalCooldown{0.f};

    static InputBindings makePlayerOneBindings();
    static InputBindings makePlayerTwoBindings();

    void createPlayers();
    void configureCamera();
    void startStageMusic();
    void updateBossTargets(float dt);
    void updatePlayer(
        PlayerSlot& slot,
        float dt,
        const InputPermissions& permissions);
    void resolveEnemyContacts(PlayerSlot& slot);
    void resolveItemContacts();
    void resolveBoostJump();
    void enforceTether();
    void constrainToLevel(PlayerSlot& slot);
    float maximumPlayerSeparation() const;
    void updateCamera();

    void requestFireball(
        PlayerSlot& owner,
        const struct ProjectileRequest& request);
    void updateProjectiles(float dt);
    std::size_t activeProjectileCount(DuoPlayerId owner) const;

    void processDeathEvents();
    void updateDownedPlayers(float dt);
    void updateBubbles(float dt);
    void rescuePlayer(PlayerSlot& bubble, PlayerSlot& rescuer);
    void beginTeamWipe();
    bool hasActivePlayer() const;

    bool startPortal(
        PlayerSlot& initiator,
        PortalActivation activation);
    void updatePortalSequence(float dt);
    void checkHeldPortalInput();

    void checkLevelCompletion(float dt);
    void completeLevel();
    bool touchesFlagpole(const PlayerSlot& slot, float& height) const;

    PlayerSlot& otherPlayer(PlayerSlot& slot);
    const PlayerSlot& otherPlayer(const PlayerSlot& slot) const;
    PlayerSlot& slotFor(DuoPlayerId id);
    const PlayerSlot& slotFor(DuoPlayerId id) const;
    DuoHudPlayerData makeHudData(
        const PlayerSlot& slot,
        const char* label) const;
    void renderBubble(
        sf::RenderWindow& window,
        const PlayerSlot& slot) const;

public:
    explicit DuoState(DuoSessionConfig config = {});
    ~DuoState() override;

    void onEnter() override;
    void onExit() override;
    void handleInput(sf::Event& event, sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

    const DuoSessionConfig& getSessionConfig() const { return session; }
};
