#pragma once

#include "Duo/DuoTypes.h"
#include "States/GameState.h"

#include <SFML/Graphics.hpp>

class DuoLevelCompleteState : public GameState {
private:
    DuoLevelResult result;
    sf::Font font;
    bool fontLoaded{false};
    int selectedIndex{0};
    float blinkTimer{0.f};
    bool showSelector{true};

    void loadFont();

public:
    explicit DuoLevelCompleteState(DuoLevelResult levelResult);

    void onEnter() override;
    void onExit() override;
    void handleInput(sf::Event& event, sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
