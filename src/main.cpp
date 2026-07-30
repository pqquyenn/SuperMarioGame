#include <SFML/Graphics.hpp>
#include <iostream>
#include "Level/Level.h"

int main() {
    std::cout << "Starting Super Mario Bros - Level Sandbox..." << std::endl;

    sf::RenderWindow window(sf::VideoMode(800, 600), "Super Mario Bros - World 1-1 Sandbox");
    window.setFramerateLimit(60);

    Level level(1);
<<<<<<< HEAD
    if (!level.loadLevel("assets/maps/1.1/1-1.txt")) {
        std::cerr << "Failed to load level file assets/maps/1.1/1-1.txt!" << std::endl;
=======
    if (!level.loadLevel("1.1/1-1.txt")) {
        std::cerr << "Failed to load level file 1.1/1-1.txt!" << std::endl;
>>>>>>> 9235567ce23e317ad442de89eb83d2facd74d664
        return -1;
    }

    std::cout << "Level loaded successfully!" << std::endl;
    std::cout << "Controls: Arrow Keys or WASD to move camera." << std::endl;
    std::cout << "Press U or H: Enter Underground Secret Map (underground.txt)." << std::endl;
    std::cout << "Press M or 1: Return to Main Overworld Map." << std::endl;
    std::cout << "Press ESC: Quit." << std::endl;

    Camera& cam = level.getCamera();
    cam.setSize(400.f, 225.f);

    const float levelPixelW = 244.f * 16.f; // 3904px
    const float levelPixelH = 16.f * 16.f;  // 256px
    cam.setLevelBounds(levelPixelW, levelPixelH);
    cam.setCenter(200.f, 112.f);

    const float speed = 18.0f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
                else if (event.key.code == sf::Keyboard::U || event.key.code == sf::Keyboard::H) {
                    std::cout << "Transitioning to Underground Secret Map (underground.txt)..." << std::endl;
                    if (level.loadHiddenMap("underground.txt")) {
                        cam.setCenter(160.f, 120.f);
                        level.getTileMap().setNeedsRedraw(true);
                    }
                }
                else if (event.key.code == sf::Keyboard::M || event.key.code == sf::Keyboard::Num1) {
                    std::cout << "Returning to Main Overworld Map (1.1/1-1.txt)..." << std::endl;
                    if (level.loadLevel("1.1/1-1.txt")) {
                        cam.setCenter(200.f, 112.f);
                        level.getTileMap().setNeedsRedraw(true);
                    }
                }
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
        float camX = cam.getView().getCenter().x;
        float camY = cam.getView().getCenter().y;
        bool isUndergroundArea = level.getIsUnderground() || camY >= 240.f || camX > 3280.f;
        sf::Color bgColor = isUndergroundArea ? sf::Color::Black : sf::Color(92, 148, 252);
        window.clear(bgColor);
        level.render(window);
        window.display();
    }

    return 0;
}
