#pragma once

#include "States/GameState.h"
#include "States/GameStateManager.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>
#include <optional>

class PlayState;

class WinState : public GameState {
public:
    enum class Phase {
        FlagSlide,
        AutoWalk,
        ScoreTally,
        Done
    };

private:
    PlayState* playState = nullptr;
    std::string mapPath;
    std::string nextStage;

    Phase currentPhase = Phase::FlagSlide;
    float phaseTimer = 0.f;

    sf::FloatRect flagpoleBounds{0.f, 0.f, 0.f, 0.f};
    sf::Vector2f castleDoorPos{0.f, 0.f};
    sf::Vector2f flagPos{0.f, 0.f};
    float flagpoleBottomY = 0.f;
    float flagBottomY = 0.f;
    bool hasFlag = false;
    bool marioInside = false;

    sf::Sprite flagSprite;
    int totalTimeBonus = 0;
    float tickSoundTimer = 0.f;

    void initSequence();
    void updateFlagSlide(float dt);
    void updateAutoWalk(float dt);
    void updateScoreTally(float dt);

public:
    WinState(
        PlayState* play,
        const std::string& path,
        const std::string& nextStage = {});

    void onEnter() override;
    void onExit() override;
    void handleInput(sf::Event& event, sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

    bool isTransparent() const override { return true; }
    bool isMarioInside() const { return marioInside; }
    const sf::Sprite* getCustomFlagSprite() const { return hasFlag ? &flagSprite : nullptr; }
};
