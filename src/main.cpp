
// #include "Core/Game.h"
// #include <iostream>

// int main() {
//     try {
//         Game game;
//         game.run();
//     } catch (const std::exception& e) {
//         std::cerr << "Fatal Error: " << e.what() << std::endl;
//         return -1;
//     }
//     return 0;
// }
#include <SFML/Graphics.hpp>
#include <iostream>

// Include your week 1 headers for testing compilation
#include "Level/TileType.h"
#include "Level/Tile.h"
#include "Level/TileMap.h"

// Note: Do NOT include Entity.h or Teammates' code here!

/**
 * @brief SANDBOX MAIN
 * 
 * A standalone environment for Nhật to test TileMap parsing and double buffering 
 * without breaking the teammates' core game loop.
 * 
 * Instruction: 
 * Once you implement `TileMap.cpp` and `Tile.cpp`, you can compile this file 
 * along with them to test your map rendering visually.
 */
int main() {
    std::cout << "Starting TileMap Sandbox..." << std::endl;

    // 1. Create a minimal window
    sf::RenderWindow window(sf::VideoMode(800, 600), "TileMap Sandbox - Week 1");
    window.setFramerateLimit(60);

    // Mock Camera logic (assuming your teammate's Camera class isn't ready)
    sf::View cameraView(sf::FloatRect(0, 0, 800, 600));
    window.setView(cameraView);

    // 2. Initialize TileMap (Uncomment once TileMap.cpp is written)
    // TileMap map;
    // if (!map.loadFromFile("assets/levels/level1.txt")) {
    //     std::cerr << "Failed to load map!" << std::endl;
    //     return -1;
    // }

    // 3. Main Loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            
            // Allow moving the camera for testing double buffering bounds
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Right) cameraView.move(16.f, 0);
                if (event.key.code == sf::Keyboard::Left) cameraView.move(-16.f, 0);
                // map.setNeedsRedraw(true); // Trigger a buffer refresh!
            }
        }

        window.clear(sf::Color(92, 148, 252)); // Sky Blue for Overworld

        window.setView(cameraView);
        
        // Mock Camera instance to pass to TileMap (Uncomment when Camera is integrated)
        // Camera mockCamera; 
        // map.render(window, mockCamera); 

        window.display();
    }

    return 0;
}
