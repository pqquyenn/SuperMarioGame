#include "Physics/CollisionGeometry.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

struct TestRunner {
    int total{0};
    int failed{0};

    void expect(bool condition, const std::string& message) {
        ++total;
        if (!condition) {
            ++failed;
            std::cerr << "FAILED: " << message << '\n';
        }
    }
};

bool nearlyEqual(float a, float b) {
    return std::abs(a - b) < 0.001f;
}

void testIntersection(TestRunner& runner) {
    sf::FloatRect overlap;
    runner.expect(
        CollisionGeometry::intersects(
            {10.f, 10.f, 16.f, 16.f},
            {20.f, 20.f, 16.f, 16.f},
            overlap),
        "overlapping rectangles intersect");
    runner.expect(
        nearlyEqual(overlap.left, 20.f) &&
            nearlyEqual(overlap.top, 20.f) &&
            nearlyEqual(overlap.width, 6.f) &&
            nearlyEqual(overlap.height, 6.f),
        "intersection rectangle is calculated correctly");

    runner.expect(
        !CollisionGeometry::intersects(
            {0.f, 0.f, 16.f, 16.f},
            {16.f, 0.f, 16.f, 16.f},
            overlap),
        "edge contact without penetration is not an overlap");
}

void expectContact(
    TestRunner& runner,
    const sf::FloatRect& moving,
    const sf::FloatRect& obstacle,
    CollisionSide expectedSide,
    const sf::Vector2f& expectedNormal,
    const std::string& name) {
    const auto contact = CollisionGeometry::findContact(moving, obstacle);
    runner.expect(contact.has_value(), name + " produces a contact");
    if (!contact) {
        return;
    }

    runner.expect(contact->side == expectedSide, name + " reports its side");
    runner.expect(
        nearlyEqual(contact->normal.x, expectedNormal.x) &&
            nearlyEqual(contact->normal.y, expectedNormal.y),
        name + " reports its normal");
}

void testContactSides(TestRunner& runner) {
    const sf::FloatRect obstacle{16.f, 16.f, 16.f, 16.f};
    expectContact(
        runner,
        {16.f, 8.f, 16.f, 16.f},
        obstacle,
        CollisionSide::Top,
        {0.f, -1.f},
        "top contact");
    expectContact(
        runner,
        {16.f, 24.f, 16.f, 16.f},
        obstacle,
        CollisionSide::Bottom,
        {0.f, 1.f},
        "bottom contact");
    expectContact(
        runner,
        {8.f, 16.f, 16.f, 16.f},
        obstacle,
        CollisionSide::Left,
        {-1.f, 0.f},
        "left contact");
    expectContact(
        runner,
        {24.f, 16.f, 16.f, 16.f},
        obstacle,
        CollisionSide::Right,
        {1.f, 0.f},
        "right contact");

    runner.expect(
        !CollisionGeometry::findContact(
             {0.f, 0.f, 8.f, 8.f}, obstacle).has_value(),
        "separated rectangles produce no contact");
}

} // namespace

int main() {
    TestRunner runner;
    testIntersection(runner);
    testContactSides(runner);

    if (runner.failed == 0) {
        std::cout << "SOLID-04 collision geometry tests passed ("
                  << runner.total << " checks)\n";
    }
    return runner.failed == 0 ? 0 : 1;
}
