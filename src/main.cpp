#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "Level/TileType.h"
#include "Level/Tile.h"
#include "Level/TileMap.h"
#include "Level/Camera.h"
#include "Core/AssetManager.h"

int main() {
    std::ofstream logFile("debug_log.txt");
    logFile << "Starting Game..." << std::endl;
    logFile << "CWD: " << std::filesystem::current_path().string() << std::endl;

    // 1. Create a 800x600 window
    logFile << "Creating Window..." << std::endl;
    sf::RenderWindow window(sf::VideoMode(800, 600), "TileMap Sandbox - World 1-1");
    window.setFramerateLimit(60);

    // 2. Initialize Camera (NES style retro resolution: 320x240)
    // This perfectly fits 15 rows of 16px blocks = 240px tall map!
    logFile << "Creating Camera..." << std::endl;
    Camera camera(320.f, 240.f);
    camera.setCenter(160.f, 120.f); 

    // 0. Pre-load all game assets
    logFile << "Loading Level Assets..." << std::endl;
    AssetManager::getInstance()->loadLevelAssets();

    // 3. Initialize and Load TileMap
    logFile << "Creating TileMap..." << std::endl;
    TileMap map;
    
    // Try all possible CWD locations: project root, build/, build/bin/, build/bin/Debug/
    const std::string mapFile = "assets/maps/1.1/1-1.txt";
    bool mapLoaded = map.readFromFile(mapFile)
                  || map.readFromFile("../" + mapFile)
                  || map.readFromFile("../../" + mapFile)
                  || map.readFromFile("../../../" + mapFile);
    if (!mapLoaded) {
        logFile << "FAILED to load map from any known path!" << std::endl;
        return -1;
    }
    
    logFile << "Map loaded successfully! Entering Main Loop..." << std::endl;

    // 4. Main Sandbox Loop
    const float cameraSpeed = 8.0f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // --- FREE CAMERA CONTROLS ---
        sf::Vector2f moveOffset(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            moveOffset.x += cameraSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            moveOffset.x -= cameraSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            moveOffset.y += cameraSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            moveOffset.y -= cameraSpeed;
        }

        // Apply movement to our actual Camera class!
        if (moveOffset.x != 0 || moveOffset.y != 0) {
            camera.move(moveOffset.x, moveOffset.y);
            map.setNeedsRedraw(true); // Tell the double buffer to redraw the new chunks
        }

        // --- RENDER PHASE ---
        window.clear(sf::Color(92, 148, 252)); // Sky Blue

        // Set the view
        window.setView(camera.getView());
        
        // Render our Map
        map.render(window, camera);

        window.display();
    }

    return 0;
}
