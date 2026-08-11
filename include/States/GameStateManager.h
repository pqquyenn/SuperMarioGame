#pragma once

#include "States/GameState.h"
#include <memory>
#include <vector>

class GameStateManager {
private:
    std::vector<std::unique_ptr<GameState>> states;

    // Danh sach hanh dong cho doi xu ly (tranh thay doi stack giua vong lap)
    enum class Action { Push, Pop, Change, ClearAndPush };
    struct PendingAction {
        Action action;
        std::unique_ptr<GameState> state;
    };
    std::vector<PendingAction> pendingActions;

public:
    // === Quan ly state ===
    void pushState(std::unique_ptr<GameState> state);    // Day state moi len stack (VD: Pause de len Play)
    void popState();                                      // Xoa state tren cung (VD: thoat Pause -> quay ve Play)
    void changeState(std::unique_ptr<GameState> state);  // Thay the state hien tai (VD: Menu -> Play)
    void clearAndPushState(std::unique_ptr<GameState> state);

    // === Xu ly pending actions (goi moi dau frame) ===
    void processPendingActions();

    // === Core loop: delegate cho state tren cung ===
    void handleInput(sf::Event& event, sf::RenderWindow& window);
    void update(float dt);
    void render(sf::RenderWindow& window);

    // === Tien ich ===
    bool isEmpty() const { return states.empty(); }
    bool hasPendingTransition() const { return !pendingActions.empty(); }
    GameState* getCurrentState() const;
};
