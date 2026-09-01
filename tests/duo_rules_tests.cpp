#include "Duo/DuoRules.h"

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

bool nearlyEqual(float actual, float expected) {
    return std::abs(actual - expected) < 0.001f;
}

void testTetherInsideRange(TestRunner& runner) {
    const DuoTetherResult result = DuoRules::calculateTether(
        {100.f, 50.f, 16.f, 24.f},
        {250.f, 50.f, 16.f, 24.f},
        200.f);

    runner.expect(result.playerOneIsLeft,
                  "tether identifies the left player");
    runner.expect(result.playerOne.allowMoveLeft &&
                      result.playerTwo.allowMoveRight,
                  "outward movement remains enabled inside the tether");
    runner.expect(result.playerOne.allowMoveRight &&
                      result.playerTwo.allowMoveLeft,
                  "inward movement remains enabled inside the tether");
}

void testTetherAtLimit(TestRunner& runner) {
    const DuoTetherResult normal = DuoRules::calculateTether(
        {100.f, 50.f, 16.f, 24.f},
        {300.f, 50.f, 16.f, 24.f},
        200.f);
    runner.expect(!normal.playerOne.allowMoveLeft,
                  "left player cannot move farther left at the limit");
    runner.expect(!normal.playerTwo.allowMoveRight,
                  "right player cannot move farther right at the limit");
    runner.expect(normal.playerOne.allowMoveRight &&
                      normal.playerTwo.allowMoveLeft,
                  "both players can still move toward each other");

    const DuoTetherResult reversed = DuoRules::calculateTether(
        {300.f, 50.f, 16.f, 24.f},
        {100.f, 50.f, 16.f, 24.f},
        200.f);
    runner.expect(!reversed.playerOne.allowMoveRight &&
                      !reversed.playerTwo.allowMoveLeft,
                  "tether follows player ordering after they cross");
}

void testSharedCameraFocus(TestRunner& runner) {
    const sf::Vector2f focus = DuoRules::calculateCameraFocus(
        {100.f, 40.f, 20.f, 20.f},
        {200.f, 80.f, 20.f, 20.f});
    runner.expect(nearlyEqual(focus.x, 160.f) &&
                      nearlyEqual(focus.y, 70.f),
                  "shared camera focuses on the midpoint of both players");
}

void testMvpRules(TestRunner& runner) {
    DuoPlayerStats playerOne;
    DuoPlayerStats playerTwo;
    playerOne.score = 5000;
    playerTwo.score = 1000;
    playerOne.flagHeight = 20.f;
    playerTwo.flagHeight = 80.f;

    runner.expect(
        DuoRules::determineMvp(playerOne, playerTwo, true) ==
            DuoMvpResult::PlayerTwo,
        "higher flag touch wins Top Jumper regardless of score");

    playerOne.flagHeight = -1.f;
    playerTwo.flagHeight = -1.f;
    runner.expect(
        DuoRules::determineMvp(playerOne, playerTwo, false) ==
            DuoMvpResult::PlayerOne,
        "score leads the non-flag MVP comparison");

    playerTwo.score = playerOne.score;
    runner.expect(
        DuoRules::determineMvp(playerOne, playerTwo, false) ==
            DuoMvpResult::Tie,
        "identical co-op statistics produce a shared MVP");
}

} // namespace

int main() {
    TestRunner runner;
    testTetherInsideRange(runner);
    testTetherAtLimit(runner);
    testSharedCameraFocus(runner);
    testMvpRules(runner);

    if (runner.failed == 0) {
        std::cout << "Duo gameplay rule tests passed ("
                  << runner.total << " checks)\n";
    }
    return runner.failed == 0 ? 0 : 1;
}
