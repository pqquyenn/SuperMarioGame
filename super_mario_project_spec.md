# Super Mario Game Project Specification & Rubric

## 1. Overview & References

### Sample Super Mario Games
* **Super Mario Bros. (1985)** [(Wiki)](https://en.wikipedia.org/wiki/Super_Mario_Bros.)  
  *Reference:* Super Mario Bros. (1985) Full Walkthrough NES Gameplay [Nostalgia]
* **New Super Mario Bros.** [(Wiki)](https://en.wikipedia.org/wiki/New_Super_Mario_Bros.)  
  *Reference:* New Super Mario Bros. DS HD - Full Game 100% Walkthrough

---

## 2. Project Description

### Objectives
In this project, you will develop a 2D Mario-style game using C++ with an emphasis on Object-Oriented Programming (OOP) principles. You are required to implement inheritance, encapsulation, polymorphism, and abstraction. Additionally, you must incorporate at least **5 design patterns** (Factory, Singleton, Observer, etc.) to create a modular, extensible, and well-structured game.

---

## 3. Requirements

### A. Object-Oriented Programming Principles
* **Inheritance & Polymorphism:**
  * **Character System:**
    * Base class `Character` defining common attributes and methods (e.g., position, movement, jump).
    * Derived classes: `Mario`, `Luigi`, `Enemy`, etc.
    * Unique behaviors and attributes per derived class (e.g., Mario grows with power-ups, enemies have distinct movement patterns).
  * **Items & Power-ups:**
    * Base class `Item` with core methods like `activate()` and `collect()`.
    * Derived item classes: `Mushroom`, `Coin`, `FireFlower`, etc.
    * Specific interaction logic with player characters (affecting size, speed, or special abilities).
  * **Polymorphism in Action:**
    * Use containers of base class pointers (e.g., `std::vector<Character*>`, `std::vector<Item*>`) to manage game objects in a unified pipeline.

### B. Game Design Patterns (At least 5 required)
* **Factory Pattern:** Dynamically instantiate characters and items based on level configuration without tight coupling.
* **Singleton Pattern:** Ensure single instances for core controllers (e.g., `GameManager`, `SoundController`).
* **Observer Pattern / Command Pattern / State Pattern / etc.:** Choose 3 additional design patterns to handle event dispatching, player inputs, or game states.

### C. Game Engine & Libraries
Choose one of the following C++ game development libraries/engines:
* **SFML:** [https://www.sfml-dev.org/](https://www.sfml-dev.org/)
* **SDL:** [https://www.libsdl.org/](https://www.libsdl.org/)
* **raylib:** [https://www.raylib.com/](https://www.raylib.com/)
* **Box2D:** [https://github.com/erincatto/box2d](https://github.com/erincatto/box2d)

### D. Core Features
* **Levels:**
  * Must feature at least **3 levels** with progressively increasing difficulty.
* **User Input & Interaction:**
  * Capture keyboard or controller input for movement (walking, running, jumping, crouching).
  * Precise collision detection with terrain, enemies, items, and level boundaries.
* **Environment, Graphics & Sound:**
  * 2D / 3D graphics rendering.
  * Audio playback for background music (BGM) and action sound effects (jumping, item collection, enemy defeat).
* **Game State Management:**
  * Manage transitions between Start Screen, Gameplay, Pause Menu, and Game Over.
  * Track and update progress metrics (score, lives, level, timer).
* **Save/Load System:**
  * File handling in C++ to save player progress and reload saved game states.

---

## 4. Advanced & Bonus Features

* **AI for Enemies:** Basic AI for characters like Goombas or Koopas (patrolling, proximity-based chasing/turning).
* **Multiple Player Characters:** Select/switch between Mario, Luigi, or others with distinct stats (e.g., Luigi jumps higher but runs slower). Character selection screen before/during levels.
* **Level Editor (Bonus):** Allow players to build, save, and load custom levels using custom file serialization.

---

## 5. Deliverables

1. **Source Code:** Well-documented C++ source code adhering to modern OOP best practices.
2. **Design Documentation:**
   * Class diagrams (UML).
   * Sequence diagrams illustrating design pattern usage.
   * Explanations of OOP principles and design pattern applications.
3. **Demo Video:** A short video demonstrating gameplay, features, and level mechanics.

---

## 6. Evaluation Rubric

| Category | Requirement / Description | Max Points |
| :--- | :--- | :---: |
| **Functionality** | **Total Functionality Points** | **65** |
| | Player Inputs, Movement, and Collision Detection | 20 |
| | Enemy Behavior & AI | 10 |
| | Power-Ups and Items | 10 |
| | 3 Level Completion & Progression | 15 |
| | Sound Effects & Background Music | 10 |
| **Design & Architecture** | **Total Design Points** | **35** |
| | Object-Oriented Design (OOP Principles) | 10 |
| | Implementation of 5 Design Patterns | 25 |
| **Additional Requirements** | **Extra / Advanced Features** | **15** |
| | Enemy AI Mechanics | 5 |
| | Multiple Playable Characters | 5 |
| | 3D Game Implementation | 5 |

* **Google Sheet Reference:** [Super Mario Rubric Sheet](https://docs.google.com/spreadsheets/d/11P1AgDmA_io1BZS3azFMiLdnxrxHj0iNQif_vrhir5o/edit?usp=sharing)

---

## 7. Disclaimer

> *This game is a non-commercial, educational project created solely for learning and teaching purposes. All intellectual property rights of the original game and its assets belong to their respective owners. This project does not aim to infringe on any copyrights and is not intended for distribution or sale. No financial profit is being made from this project. If you are the copyright holder and have concerns about this use, please contact us to address them promptly.*
