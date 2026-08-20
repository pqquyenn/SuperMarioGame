#include "AdminControl/DebugMovementTrail.h"

#include "Entities/Character.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float MinimumSampleDistance = 0.75f;
constexpr float WallApproachSpeed = 20.f;
constexpr float StoppedSpeed = 0.5f;

float squaredDistance(sf::Vector2f first, sf::Vector2f second) {
    const float x = first.x - second.x;
    const float y = first.y - second.y;
    return x * x + y * y;
}
}
sf::Vector2f DebugMovementTrail::bodyCenter(
    const Character& character
) {
    const sf::FloatRect bounds = character.getBounds();
    return {bounds.left + bounds.width * 0.5f,
            bounds.top + bounds.height * 0.5f};
}

void DebugMovementTrail::append(
    const Character& character,
    DebugTrailEvent event
) {
    const sf::Vector2f center = bodyCenter(character);
    if (event == DebugTrailEvent::None && !samples.empty() &&
        squaredDistance(samples.back().center, center) <
            MinimumSampleDistance * MinimumSampleDistance) {
        return;
    }
    samples.push_back({center, event});
}

void DebugMovementTrail::start(const Character& character) {
    samples.clear();
    remainingSeconds = DurationSeconds;
    active = true;
    hasPreviousFrame = true;
    previousGrounded = character.isGrounded();
    previousVelocity = character.getVelocity();
    append(character, DebugTrailEvent::None);
}

void DebugMovementTrail::update(
    const Character& character,
    float dt
) {
    if (!active) {
        return;
    }

    remainingSeconds = std::max(0.f, remainingSeconds - dt);
    if (remainingSeconds <= 0.f) {
        active = false;
        return;
    }

    const bool grounded = character.isGrounded();
    const sf::Vector2f velocity = character.getVelocity();
    DebugTrailEvent event = DebugTrailEvent::None;

    if (hasPreviousFrame) {
        if (previousGrounded && !grounded && velocity.y < 0.f) {
            event = DebugTrailEvent::Takeoff;
        } else if (!previousGrounded && grounded) {
            event = DebugTrailEvent::Landing;
        } else if (!previousGrounded && !grounded &&
                   std::abs(previousVelocity.x) > WallApproachSpeed &&
                   std::abs(velocity.x) <= StoppedSpeed) {
            event = DebugTrailEvent::WallImpact;
        }
    }

    append(character, event);
    hasPreviousFrame = true;
    previousGrounded = grounded;
    previousVelocity = velocity;
}

void DebugMovementTrail::render(
    sf::RenderWindow& window,
    sf::Color trailColor
) const {
    if (!active || samples.empty()) {
        return;
    }

    sf::VertexArray line{sf::LineStrip, samples.size()};
    const float denominator = static_cast<float>(
        std::max<std::size_t>(1, samples.size() - 1));
    for (std::size_t index = 0; index < samples.size(); ++index) {
        sf::Color color = trailColor;
        const float progress = static_cast<float>(index) / denominator;
        color.a = static_cast<sf::Uint8>(45.f + progress * 210.f);
        line[index] = sf::Vertex{samples[index].center, color};
    }
    window.draw(line);

    for (const Sample& sample : samples) {
        if (sample.event == DebugTrailEvent::None) {
            continue;
        }

        if (sample.event == DebugTrailEvent::Takeoff) {
            sf::CircleShape marker{3.f};
            marker.setOrigin(3.f, 3.f);
            marker.setPosition(sample.center);
            marker.setFillColor(sf::Color::Transparent);
            marker.setOutlineColor(sf::Color::Yellow);
            marker.setOutlineThickness(1.f);
            window.draw(marker);
        } else if (sample.event == DebugTrailEvent::Landing) {
            sf::RectangleShape marker{{6.f, 4.f}};
            marker.setOrigin(3.f, 2.f);
            marker.setPosition(sample.center);
            marker.setFillColor(sf::Color{80, 255, 120});
            window.draw(marker);
        } else {
            sf::ConvexShape marker;
            marker.setPointCount(4);
            marker.setPoint(0, {0.f, -4.f});
            marker.setPoint(1, {4.f, 0.f});
            marker.setPoint(2, {0.f, 4.f});
            marker.setPoint(3, {-4.f, 0.f});
            marker.setPosition(sample.center);
            marker.setFillColor(sf::Color{255, 100, 240});
            window.draw(marker);
        }
    }
}

void DebugMovementTrail::clear() {
    samples.clear();
    remainingSeconds = 0.f;
    active = false;
    hasPreviousFrame = false;
}

std::size_t DebugMovementTrail::getEventCount(
    DebugTrailEvent event
) const {
    return static_cast<std::size_t>(std::count_if(
        samples.begin(), samples.end(),
        [event](const Sample& sample) { return sample.event == event; }));
}
