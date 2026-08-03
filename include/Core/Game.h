#pragma once

#include <SFML/Graphics.hpp>
#include "States/GameStateManager.h"

class Game {
private:
    // === Window ===
    sf::RenderWindow window;

    // === Game States ===
    GameStateManager stateManager;

    // === Timing / Game Loop ===
    sf::Clock clock;

    // FPS capping: 60 FPS -> moi frame toi da 1/60 giay
    static constexpr float TARGET_FPS = 60.0f;
    static constexpr float TIME_PER_FRAME = 1.0f / TARGET_FPS;

    // Fixed timestep accumulator: tich luy thoi gian
    // de dam bao physics update chay deu tren moi may
    float accumulator;

    // === Window & Fullscreen ===
    bool isFullscreen = false;
    void toggleFullscreen();

    // === Initialization ===
    void initWindow();
    void initStates();


    // === Core Loop Steps ===
    void processEvents();               // Xu ly input/event cua SFML
    void update(float dt);              // Update logic theo deltaTime (variable)
    void fixedUpdate(float fixedDt);    // Update physics voi buoc thoi gian co dinh
    void render();                      // Ve len man hinh

public:
    Game();
    ~Game() = default;

    void run();  // Game Loop chinh
};
