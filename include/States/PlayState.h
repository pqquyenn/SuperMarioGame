#pragma once

#include "States/GameState.h"
#include "States/GameStateManager.h"
#include "Level/Level.h"
#include "Entities/Character.h"
#include "Entities/Fireball.h"
#include "Input/InputHandler.h"
#include "AdminControl/AdminDebugView.h"
#include "UI/HUD.h"
#include <memory>
#include <string>

class PlayState : public GameState {
private:
    Level level;
    std::unique_ptr<Character> player;
    InputHandler inputHandler;
    std::string initialMapPath;
    sf::Vector2f playerSpawnPoint{0.f, 0.f};
    HUD hud;
    ObserverConnection hudObserverConnection;
    ObserverConnection achievementObserverConnection;
    AdminDebugView adminDebugView;
    std::vector<std::unique_ptr<Fireball>> fireballs;

    bool isFreeCameraMode = false;
    float freeCamSpeed = 900.f;
    sf::Text freeCamText;
    sf::Font freeCamFont;
    bool freeCamFontLoaded = false;
    bool levelWon = false;
    float skyDropTimer = 5.f;

    void spawnFireball(const ProjectileRequest& request);
    void refreshPlayerSpawnPoint();
    void centerCameraOnPlayerSpawn();
    void constrainPlayerHorizontally();
    bool isMap4() const;

public:
    PlayState(const std::string& mapPath = "1.1/1-1.level");

    void onEnter() override;
    void onExit() override;
    void handleInput(sf::Event& event, sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

    Character* getPlayer() { return player.get(); }
    const Character* getPlayer() const { return player.get(); }
    Level& getLevel() { return level; }
    const Level& getLevel() const { return level; }
    HUD& getHUD() { return hud; }
    const HUD& getHUD() const { return hud; }
    const std::string& getInitialMapPath() const { return initialMapPath; }
    bool isLevelWon() const { return levelWon; }
};

