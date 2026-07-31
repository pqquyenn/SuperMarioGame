#include <SFML/Graphics.hpp>
#include <iostream>
#include "Level/Level.h"

int main() {
    std::cout << "Starting Super Mario Bros - Level Sandbox..." << std::endl;

    sf::RenderWindow window(sf::VideoMode(800, 600), "Super Mario Bros - World 1-1 Sandbox");
    window.setFramerateLimit(60);

    Level level(1);
    if (!level.loadLevel("1.1/1-1.txt")) {
        std::cerr << "Failed to load level file 1.1/1-1.txt!" << std::endl;
        return -1;
    }

    TileMap bgMap;
    bgMap.setTileOffset(sf::Vector2f(0.f, -8.f));
    
    Camera& cam = level.getCamera();
    cam.setSize(400.f, 225.f);

    auto loadLevelMap = [&](const std::string& levelPath, const std::string& bgPath, float bgOffsetY = -8.f) {
        if (level.loadLevel(levelPath)) {
            bgMap.setTileOffset(sf::Vector2f(0.f, bgOffsetY));
            
            float levelPixelW = level.getTileMap().getWidth() * 16.f;
            float levelPixelH = level.getTileMap().getHeight() * 16.f;
            cam.setLevelBounds(levelPixelW, levelPixelH);
            cam.setCenter(200.f, 112.f);
            level.getTileMap().setNeedsRedraw(true);
            
            std::string fullBgPaths[] = {
                "assets/maps/" + bgPath,
                "../assets/maps/" + bgPath,
                "../../assets/maps/" + bgPath,
                "../../../assets/maps/" + bgPath
            };
            bool bgLoaded = false;
            for (const auto& p : fullBgPaths) {
                if (bgMap.readFromFile(p)) {
                    bgLoaded = true;
                    break;
                }
            }
            if (!bgLoaded) {
                std::cerr << "Failed to load " << bgPath << "!" << std::endl;
            }
            bgMap.setNeedsRedraw(true);
        }
    };

    loadLevelMap("1.1/1-1.txt", "1.1/background.txt", -8.f);

    std::cout << "Level loaded successfully!" << std::endl;
    std::cout << "Controls: Arrow Keys or WASD to move camera." << std::endl;
    std::cout << "Press U or H: Enter Underground Secret Map." << std::endl;
    std::cout << "Press M or 1: Load World 1-1." << std::endl;
    std::cout << "Press 2: Load World 1-2." << std::endl;
    std::cout << "Press 3: Load World 1-3." << std::endl;
    std::cout << "Press ESC: Quit." << std::endl;

    const float speed = 20.0f;

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
                        bgMap.setNeedsRedraw(true);
                    }
                }
                else if (event.key.code == sf::Keyboard::M || event.key.code == sf::Keyboard::Num1) {
                    std::cout << "Loading World 1-1..." << std::endl;
                    loadLevelMap("1.1/1-1.txt", "1.1/background.txt", -8.f);
                }
                else if (event.key.code == sf::Keyboard::Num2) {
                    std::cout << "Loading World 1-2..." << std::endl;
                    loadLevelMap("1.2/1-2.txt", "1.2/background.txt", 0.f);
                }
                else if (event.key.code == sf::Keyboard::Num3) {
                    std::cout << "Loading World 1-3..." << std::endl;
                    loadLevelMap("1.3/1-3.txt", "1.3/background.txt", -8.f);
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
            bgMap.setNeedsRedraw(true);
        }

        cam.applyTo(window);
        window.clear(sf::Color(92, 148, 252));
        
        bgMap.render(window, cam);
        level.render(window);
        window.display();
    }

    return 0;
}
