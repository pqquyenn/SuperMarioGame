#include "Core/Game.h"
#include "States/MenuState.h"
#include <memory>

void Game::initWindow() {
    window.create(sf::VideoMode(800, 600), "Super Mario Bros (C++ SFML 2.6.1)", sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(60);
}

void Game::initStates() {
    stateManager.pushState(std::make_unique<MenuState>());
}

Game::Game() {
    initWindow();
    initStates();
}

void Game::run() {
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        stateManager.handleInput(window);
        stateManager.update(dt);

        window.clear(sf::Color(107, 140, 255)); // Classic Mario sky blue
        stateManager.render(window);
        window.display();
    }
}
