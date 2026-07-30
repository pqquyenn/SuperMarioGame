#include <SFML/Graphics.hpp>
#include <iostream>
#include "Level/Level.h"

int main() {
    std::cout << "Starting Super Mario Bros - Level Sandbox..." << std::endl;

    sf::RenderWindow window(sf::VideoMode(800, 600), "Super Mario Bros - World 1-1 Sandbox");
    window.setFramerateLimit(60);

    Level level(1);
    if (!level.loadLevel("assets/maps/1.1/1-1.txt")) {
        std::cerr << "Failed to load level file assets/maps/1.1/1-1.txt!" << std::endl;
        return -1;
    }

    std::cout << "Level loaded successfully!" << std::endl;
    std::cout << "Controls: Arrow Keys or WASD to move camera. ESC to quit." << std::endl;

    Camera& cam = level.getCamera();

    cam.setSize(400.f, 225.f);

    const float levelPixelW = 224.f * 16.f; // 3584
    const float levelPixelH = 14.f * 16.f;  // 224
    cam.setLevelBounds(levelPixelW, levelPixelH);

    cam.setCenter(200.f, 112.f);

    const float speed = 6.0f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
        }

        bool moved = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            cam.move(speed, 0.f);
            moved = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            cam.move(-speed, 0.f);
            moved = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            cam.move(0.f, speed);
            moved = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            cam.move(0.f, -speed);
            moved = true;
        }

        if (moved) {
            level.getTileMap().setNeedsRedraw(true);
        }

        cam.applyTo(window);
        window.clear(sf::Color(92, 148, 252));
        level.render(window);
        window.display();
    }

    return 0;
}


