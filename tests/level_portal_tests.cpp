#include "Entities/Character.h"
#include "Level/Level.h"

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

class TestCharacter final : public Character {
public:
    TestCharacter(float x = 0.f, float y = 0.f)
        : Character{x, y} {}
};

bool nearlyEqual(float actual, float expected) {
    return std::abs(actual - expected) < 0.01f;
}

void configureCamera(Level& level) {
    const float tileSize = level.getDefinition().tileSize;
    level.getCamera().setSize(320.f, 240.f);
    level.getCamera().setLevelBounds(
        level.getTileMap().getMapWidth() * tileSize,
        level.getTileMap().getMapHeight() * tileSize);
}

void expectPosition(
    TestRunner& runner,
    const Character& character,
    float x,
    float y,
    const std::string& message) {
    const sf::Vector2f position = character.getPosition();
    runner.expect(nearlyEqual(position.x, x) && nearlyEqual(position.y, y),
                  message);
}

void expectVelocity(
    TestRunner& runner,
    const Character& character,
    float x,
    float y,
    const std::string& message) {
    const sf::Vector2f velocity = character.getVelocity();
    runner.expect(nearlyEqual(velocity.x, x) && nearlyEqual(velocity.y, y),
                  message);
}

void testRuntimeRequiresManifest(TestRunner& runner) {
    Level level;
    runner.expect(!level.loadLevel("1.1/1-1.txt"),
                  "runtime rejects a txt stage entry");
    runner.expect(level.loadLevel("1.1/1-1.level"),
                  "runtime accepts a level manifest");
    runner.expect(level.isDataDriven(),
                  "successful runtime level is manifest-driven");
}

void testOneOnePortalAndFixedCamera(TestRunner& runner) {
    Level level;
    runner.expect(level.loadLevel("1.1/1-1.level"),
                  "1-1 runtime level loads");
    configureCamera(level);

    TestCharacter character;
    const sf::FloatRect entrance{57.f * 16.f, 9.f * 16.f, 32.f, 16.f};
    runner.expect(!level.tryActivatePortal(
                      character, entrance, PortalActivation::Right),
                  "wrong portal direction is rejected");
    runner.expect(level.getCurrentArea() == "overworld",
                  "wrong direction keeps the current area");

    runner.expect(level.tryActivatePortal(
                      character, entrance, PortalActivation::Down),
                  "1-1 manifest entrance activates");
    runner.expect(level.getCurrentArea() == "bonus",
                  "1-1 entrance selects the bonus area");
    expectPosition(runner, character, 3736.f, 32.f,
                   "1-1 entrance uses the manifest anchor");
    expectVelocity(runner, character, 0.f, 0.f,
                   "1-1 entrance uses the manifest exit velocity");
    runner.expect(level.usesDarkBackground(),
                  "bonus camera selects its dark background rule");
    runner.expect(!level.tryActivatePortal(
                      character, entrance, PortalActivation::Down),
                  "portal cannot activate from the wrong current area");

    const sf::Vector2f fixedCenter = level.getCamera().getView().getCenter();
    level.updateCameraFor({3900.f, 100.f});
    const sf::Vector2f movedPlayerCenter =
        level.getCamera().getView().getCenter();
    runner.expect(nearlyEqual(fixedCenter.x, movedPlayerCenter.x) &&
                      nearlyEqual(fixedCenter.y, movedPlayerCenter.y),
                  "fixed bonus camera does not follow the player");

    const sf::FloatRect exit{245.f * 16.f, 11.f * 16.f, 48.f, 32.f};
    runner.expect(level.tryActivatePortal(
                      character, exit, PortalActivation::Right),
                  "1-1 manifest exit activates");
    runner.expect(level.getCurrentArea() == "overworld",
                  "1-1 exit returns to overworld");
    expectPosition(runner, character, 2872.f, 144.f,
                   "1-1 exit uses the manifest anchor");
    expectVelocity(runner, character, 0.f, -100.f,
                   "1-1 exit uses the manifest velocity");
}

void testInputAndDebugPortalActivation(TestRunner& runner) {
    Level level;
    runner.expect(level.loadLevel("1.1/1-1.level"),
                  "1-1 loads for input portal test");
    configureCamera(level);

    TestCharacter inputCharacter{57.f * 16.f, 128.f};
    runner.expect(level.tryActivatePortalForInput(
                      inputCharacter, PortalActivation::Down),
                  "down input activates a portal while standing on its top");
    runner.expect(level.getCurrentArea() == "bonus",
                  "input activation changes area through manifest data");

    level.resetToInitialArea();
    TestCharacter debugCharacter;
    runner.expect(level.tryActivateFirstPortalFromCurrentArea(debugCharacter),
                  "debug navigation activates the first area portal");
    runner.expect(level.getCurrentArea() == "bonus",
                  "debug navigation uses the portal target area");

    level.resetToInitialArea();
    runner.expect(level.getCurrentArea() == "overworld",
                  "respawn reset uses initial_area from the manifest");
}

void testOneTwoPortalChain(TestRunner& runner) {
    Level level;
    runner.expect(level.loadLevel("1.2/1-2.level"),
                  "1-2 runtime level loads");
    configureCamera(level);
    TestCharacter character{240.f, 160.f};

    runner.expect(level.tryActivatePortalForInput(
                      character, PortalActivation::Right),
                  "1-2 overworld entrance activates");
    runner.expect(level.getCurrentArea() == "underground",
                  "1-2 entrance selects underground area");
    expectPosition(runner, character, 48.f, 256.f,
                   "1-2 underground entry uses anchor position");
    expectVelocity(runner, character, 30.f, 0.f,
                   "1-2 underground entry uses anchor velocity");
    runner.expect(level.usesDarkBackground(),
                  "underground camera is dark");

    const float firstUndergroundCameraX =
        level.getCamera().getView().getCenter().x;
    level.updateCameraFor({800.f, 400.f});
    runner.expect(level.getCamera().getView().getCenter().x >
                      firstUndergroundCameraX,
                  "underground camera follows the player on X");

    character.setPosition(116.f * 16.f, 384.f);
    runner.expect(level.tryActivatePortalForInput(
                      character, PortalActivation::Down),
                  "1-2 bonus entrance activates");
    runner.expect(level.getCurrentArea() == "bonus",
                  "1-2 bonus entrance changes area");
    expectPosition(runner, character, 32.f, 560.f,
                   "1-2 bonus entry uses anchor position");

    character.setPosition(38.f * 16.f, 41.f * 16.f);
    runner.expect(level.tryActivatePortalForInput(
                      character, PortalActivation::Right),
                  "1-2 bonus exit activates");
    runner.expect(level.getCurrentArea() == "underground",
                  "1-2 bonus exit returns underground");
    expectPosition(runner, character, 2000.f, 400.f,
                   "1-2 return uses anchor position");
    expectVelocity(runner, character, 30.f, -80.f,
                   "1-2 return uses anchor velocity");

    character.setPosition(188.f * 16.f, 18.f * 16.f);
    runner.expect(level.tryActivatePortalForInput(
                      character, PortalActivation::Right),
                  "1-2 underground exit activates");
    runner.expect(level.getCurrentArea() == "overworld",
                  "1-2 underground exit returns overworld");
    expectPosition(runner, character, 2872.f, 144.f,
                   "1-2 overworld exit uses anchor position");
}

void testNextStageRuntimeChain(TestRunner& runner) {
    Level level;
    runner.expect(level.loadLevel("1.3/1-3.level"),
                  "1-3 runtime level loads");
    runner.expect(level.getNextStage() == "1.4/1-4.level",
                  "1-3 next stage comes from its manifest");

    Level next;
    runner.expect(next.loadLevel(level.getNextStage()),
                  "manifest next stage is loadable at runtime");
    runner.expect(next.getDefinition().id == "world-1-4",
                  "runtime transition reaches 1-4");
    runner.expect(next.getNextStage().empty(),
                  "empty next_stage marks the final stage");
}

} // namespace

int main() {
    TestRunner runner;
    testRuntimeRequiresManifest(runner);
    testOneOnePortalAndFixedCamera(runner);
    testInputAndDebugPortalActivation(runner);
    testOneTwoPortalChain(runner);
    testNextStageRuntimeChain(runner);

    if (runner.failed == 0) {
        std::cout << "SOLID-03 portal runtime tests passed ("
                  << runner.total << " checks)\n";
    }
    return runner.failed == 0 ? 0 : 1;
}
