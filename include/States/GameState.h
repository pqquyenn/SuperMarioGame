#pragma once

#include <SFML/Graphics.hpp>

// Forward declaration: de state co the goi stateManager ma khong bi vong lap include
class GameStateManager;

class GameState {
protected:
    // Moi state giu con tro toi GameStateManager
    // de co the tu chuyen man hinh (VD: MenuState -> pushState(PlayState))
    GameStateManager* stateManager = nullptr;

public:
    virtual ~GameState() = default;

    // === Lifecycle: goi khi state duoc push/pop ===
    virtual void onEnter() {}   // Goi 1 lan khi state duoc push vao stack
    virtual void onExit() {}    // Goi 1 lan khi state bi pop ra khoi stack
    virtual void onPause() {}   // Goi khi mot state khac duoc push len tren
    virtual void onResume() {}  // Goi khi state phia tren bi pop

    // Overlay states (for example PauseState) allow states below to render.
    virtual bool isTransparent() const { return false; }

    // === Core loop: goi moi frame ===
    virtual void handleInput(sf::Event& event, sf::RenderWindow& window) = 0;  // Xu ly tung event rieng le
    virtual void update(float dt) = 0;
    virtual void render(sf::RenderWindow& window) = 0;

    // === Setter: GameStateManager goi khi pushState() ===
    void setStateManager(GameStateManager* manager) { stateManager = manager; }
};
