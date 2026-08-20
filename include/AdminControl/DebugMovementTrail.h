#pragma once

#include <SFML/Graphics.hpp>

#include <cstddef>
#include <vector>

class Character;

enum class DebugTrailEvent {
    None,
    Takeoff,
    Landing,
    WallImpact
};
class DebugMovementTrail {
private:
    struct Sample {
        sf::Vector2f center;
        DebugTrailEvent event{DebugTrailEvent::None};
    };

    static constexpr float DurationSeconds = 8.f;
    std::vector<Sample> samples;
    float remainingSeconds{0.f};
    bool active{false};
    bool hasPreviousFrame{false};
    bool previousGrounded{false};
    sf::Vector2f previousVelocity{0.f, 0.f};

    static sf::Vector2f bodyCenter(const Character& character);
    void append(const Character& character, DebugTrailEvent event);

public:
    void start(const Character& character);
    void update(const Character& character, float dt);
    void render(sf::RenderWindow& window, sf::Color trailColor) const;
    void clear();

    bool isActive() const { return active; }
    float getRemainingSeconds() const { return remainingSeconds; }
    std::size_t getSampleCount() const { return samples.size(); }
    std::size_t getEventCount(DebugTrailEvent event) const;
};
