#include "Observer/Subject.h"
#include "Observer/Observer.h"
#include "Observer/Event.h"
#include "UI/HUD.h"
#include "Entities/Character.h"
#include "Entities/Enemies/Enemy.h"
#include "Entities/Enemies/Goomba.h"
#include "Entities/Enemies/Koopa.h"
#include "Entities/Enemies/PiranhaPlant.h"
#include "Entities/Items/Item.h"
#include "Entities/Items/Coin.h"
#include "Entities/Items/Mushroom.h"
#include "Entities/Items/FireFlower.h"
#include "Entities/Items/StarItem.h"
#include "Entities/Items/OneUpMushroom.h"
#include "PlayerStates/SmallState.h"
#include "PlayerStates/SuperState.h"
#include "PlayerStates/FireState.h"
#include "PlayerEffects/StarEffect.h"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>

namespace {

// ============================================================
// Custom C++17 Test Runner
// ============================================================
struct TestRunner {
    int total = 0;
    int passed = 0;
    int failed = 0;

    void expect(bool condition, const std::string& testName, const std::string& message) {
        ++total;
        if (condition) {
            ++passed;
        } else {
            ++failed;
            std::cerr << "  [FAIL] " << testName << " -> " << message << "\n";
        }
    }

    void report(const std::string& suiteName) const {
        std::cout << "[" << suiteName << "] Total: " << total
                  << " | Passed: " << passed
                  << " | Failed: " << failed << "\n";
    }

    int exitCode() const {
        return failed == 0 ? 0 : 1;
    }
};

// ============================================================
// Test Fixtures: RecordingObserver (Test Spy) & TestPlayer
// ============================================================
class RecordingObserver : public Observer {
public:
    std::vector<GameEvent> recordedEvents;

    void onNotify(const GameEvent& event) override {
        recordedEvents.push_back(event);
    }

    void clear() {
        recordedEvents.clear();
    }

    std::size_t count() const {
        return recordedEvents.size();
    }
};

class TestPlayer : public Character {
public:
    TestPlayer(float x = 0.f, float y = 0.f)
        : Character(x, y) {}
};

// ============================================================
// Suite 1: Observer & Subject Tests (SOLID-08)
// ============================================================
void testObserverNormalDispatch(TestRunner& runner) {
    Subject subject;
    RecordingObserver observer1;
    RecordingObserver observer2;

    ObserverConnection conn1 = subject.addObserver(&observer1);
    ObserverConnection conn2 = subject.addObserver(&observer2);

    runner.expect(conn1.isConnected(), "NormalDispatch", "Connection 1 is connected");
    runner.expect(conn2.isConnected(), "NormalDispatch", "Connection 2 is connected");

    subject.notify(GameEvent::coinCollected(200));
    subject.notify(GameEvent::enemyDefeated(100));

    runner.expect(observer1.count() == 2, "NormalDispatch", "Observer 1 received 2 events");
    runner.expect(observer2.count() == 2, "NormalDispatch", "Observer 2 received 2 events");

    if (observer1.count() >= 2) {
        runner.expect(observer1.recordedEvents[0].type == GameEventType::COIN_COLLECTED,
                      "NormalDispatch", "Event 1 is COIN_COLLECTED");
        runner.expect(observer1.recordedEvents[0].value == 200,
                      "NormalDispatch", "Event 1 value is 200");
        runner.expect(observer1.recordedEvents[1].type == GameEventType::ENEMY_DEFEATED,
                      "NormalDispatch", "Event 2 is ENEMY_DEFEATED");
        runner.expect(observer1.recordedEvents[1].value == 100,
                      "NormalDispatch", "Event 2 value is 100");
    }
}

void testObserverDuplicatePrevention(TestRunner& runner) {
    Subject subject;
    RecordingObserver observer;

    ObserverConnection conn1 = subject.addObserver(&observer);
    ObserverConnection conn2 = subject.addObserver(&observer); // Duplicate

    runner.expect(conn1.isConnected(), "DuplicatePrevention", "First connection is connected");
    runner.expect(!conn2.isConnected(), "DuplicatePrevention", "Duplicate connection is empty / not connected");

    subject.notify(GameEvent::enemyDefeated(100));

    runner.expect(observer.count() == 1, "DuplicatePrevention", "Duplicate registration does not double-notify");
}

void testObserverLifetimeSafety(TestRunner& runner) {
    Subject subject;
    ObserverConnection conn;

    {
        RecordingObserver localObserver;
        conn = subject.addObserver(&localObserver);
        runner.expect(conn.isConnected(), "LifetimeSafety", "Connection active while observer lives");
        subject.notify(GameEvent::enemyDefeated(100));
        runner.expect(localObserver.count() == 1, "LifetimeSafety", "Local observer received event");
    } // localObserver destroyed here, but conn still exists in outer scope

    runner.expect(!conn.isConnected(), "LifetimeSafety", "Connection detects dead observer token");

    // Must NOT crash or dereference dangling pointer
    subject.notify(GameEvent::enemyDefeated(200));
    runner.expect(true, "LifetimeSafety", "Notifying after observer destruction does not crash");
}

void testObserverRAIIConnection(TestRunner& runner) {
    Subject subject;
    RecordingObserver observer;

    {
        ObserverConnection conn = subject.addObserver(&observer);
        runner.expect(conn.isConnected(), "RAIIConnection", "Connection is connected");
        subject.notify(GameEvent::coinCollected(200));
        runner.expect(observer.count() == 1, "RAIIConnection", "Observer notified before disconnect");
        conn.disconnect();
        runner.expect(!conn.isConnected(), "RAIIConnection", "Connection disconnected explicitly");
        subject.notify(GameEvent::coinCollected(200));
        runner.expect(observer.count() == 1, "RAIIConnection", "No notification after explicit disconnect");
    }

    // New connection that destructs naturally
    {
        ObserverConnection conn2 = subject.addObserver(&observer);
        runner.expect(conn2.isConnected(), "RAIIConnection", "Connection 2 is connected");
    } // conn2 destructor calls disconnect()

    subject.notify(GameEvent::coinCollected(200));
    runner.expect(observer.count() == 1, "RAIIConnection", "Destructed connection automatically stopped notifications");
}

void testObserverReentrantSafety(TestRunner& runner) {
    Subject subject;

    class SelfDisconnectingObserver : public Observer {
    public:
        ObserverConnection connection;
        int receiveCount = 0;

        void onNotify(const GameEvent& event) override {
            ++receiveCount;
            connection.disconnect();
        }
    };

    SelfDisconnectingObserver selfDiscObserver;
    selfDiscObserver.connection = subject.addObserver(&selfDiscObserver);

    RecordingObserver bystanderObserver;
    ObserverConnection bystanderConn = subject.addObserver(&bystanderObserver);

    // Should cleanly notify both without iterator invalidation crash
    subject.notify(GameEvent::enemyDefeated(100));

    runner.expect(selfDiscObserver.receiveCount == 1, "ReentrantSafety", "Self-disconnecting observer notified once");
    runner.expect(bystanderObserver.count() == 1, "ReentrantSafety", "Bystander observer still received event");

    // Second notify: self-disconnecting observer should no longer receive event
    subject.notify(GameEvent::enemyDefeated(100));
    runner.expect(selfDiscObserver.receiveCount == 1, "ReentrantSafety", "Self-disconnected observer no longer receives event");
    runner.expect(bystanderObserver.count() == 2, "ReentrantSafety", "Bystander observer continues to receive event");
}

// ============================================================
// Suite 2: HUD Score & Event Integration Tests (SOLID-08)
// ============================================================
void testHUDInitialState(TestRunner& runner) {
    HUD hud;
    runner.expect(hud.getScore() == 0, "HUDInitialState", "Initial score is 0");
    runner.expect(hud.getCoins() == 0, "HUDInitialState", "Initial coins is 0");
    runner.expect(hud.getLives() == 3, "HUDInitialState", "Initial lives is 3");
    runner.expect(std::abs(hud.getTimeRemaining() - 400.f) < 0.001f, "HUDInitialState", "Initial time is 400");
    runner.expect(!hud.isTimeFrozen(), "HUDInitialState", "Time is not frozen by default");
}

void testHUDCoinCollection(TestRunner& runner) {
    HUD hud;
    hud.onNotify(GameEvent::coinCollected(200));

    runner.expect(hud.getCoins() == 1, "HUDCoinCollection", "Coin count incremented to 1");
    runner.expect(hud.getScore() == 200, "HUDCoinCollection", "Score increased by 200");

    hud.onNotify(GameEvent::coinCollected(300));
    runner.expect(hud.getCoins() == 2, "HUDCoinCollection", "Coin count incremented to 2");
    runner.expect(hud.getScore() == 500, "HUDCoinCollection", "Score increased by 300 to 500");
}

void testHUDEnemyDefeated(TestRunner& runner) {
    HUD hud;
    hud.onNotify(GameEvent::enemyDefeated(100));
    runner.expect(hud.getScore() == 100, "HUDEnemyDefeated", "Score increased to 100");
    runner.expect(hud.getCoins() == 0, "HUDEnemyDefeated", "Coin count remains 0");

    hud.onNotify(GameEvent::enemyDefeated(400));
    runner.expect(hud.getScore() == 500, "HUDEnemyDefeated", "Score increased to 500");
}

void testHUDLifeGained(TestRunner& runner) {
    HUD hud;
    hud.onNotify(GameEvent::lifeGained(1, 1000));
    runner.expect(hud.getLives() == 4, "HUDLifeGained", "Lives increased from 3 to 4");
    runner.expect(hud.getScore() == 1000, "HUDLifeGained", "Score increased by 1000");

    hud.onNotify(GameEvent::lifeGained(2, 500));
    runner.expect(hud.getLives() == 6, "HUDLifeGained", "Lives increased by 2 to 6");
    runner.expect(hud.getScore() == 1500, "HUDLifeGained", "Score increased by 500 to 1500");
}

void testHUDPlayerDied(TestRunner& runner) {
    HUD hud;
    hud.onNotify(GameEvent::playerDied(0));
    runner.expect(hud.getLives() == 2, "HUDPlayerDied", "Lives decremented from 3 to 2");
    hud.onNotify(GameEvent::playerDied(0));
    runner.expect(hud.getLives() == 1, "HUDPlayerDied", "Lives decremented to 1");
}

void testHUDTimerAndFreeze(TestRunner& runner) {
    HUD hud;
    hud.update(10.f);
    runner.expect(std::abs(hud.getTimeRemaining() - 390.f) < 0.01f, "HUDTimerAndFreeze", "Time decreased by 10s");

    hud.setTimeFrozen(true);
    runner.expect(hud.isTimeFrozen(), "HUDTimerAndFreeze", "Time freeze flag is set");
    hud.update(10.f);
    runner.expect(std::abs(hud.getTimeRemaining() - 390.f) < 0.01f, "HUDTimerAndFreeze", "Time remains frozen");

    hud.setTimeFrozen(false);
    hud.update(5.f);
    runner.expect(std::abs(hud.getTimeRemaining() - 385.f) < 0.01f, "HUDTimerAndFreeze", "Time resumes countdown");
}

// ============================================================
// Suite 3: Enemy State Transitions Tests (SOLID-11)
// ============================================================
void testGoombaStateTransitions(TestRunner& runner) {
    Goomba goomba(100.f, 200.f);
    runner.expect(goomba.isEnemyAlive(), "GoombaState", "Goomba initially alive");
    runner.expect(goomba.isActive(), "GoombaState", "Goomba initially active");
    runner.expect(!goomba.isSquished(), "GoombaState", "Goomba initially not squished");

    goomba.onStomped();
    runner.expect(goomba.isSquished(), "GoombaState", "Goomba is squished after stomp");
    runner.expect(goomba.isActive(), "GoombaState", "Goomba still active during squish animation");
    runner.expect(std::abs(goomba.getSquishTimer() - 0.f) < 0.001f, "GoombaState", "Squish timer reset to 0");

    // Advance squish timer partially (0.25s / 0.5s duration)
    goomba.update(0.25f);
    runner.expect(goomba.isActive(), "GoombaState", "Goomba active at 0.25s squish duration");

    // Advance past 0.5s duration
    goomba.update(0.26f);
    runner.expect(!goomba.isActive(), "GoombaState", "Goomba deactivated after 0.5s squish duration");
    runner.expect(!goomba.isEnemyAlive(), "GoombaState", "Goomba is marked dead");
}

void testGoombaFireballDefeat(TestRunner& runner) {
    Goomba goomba(100.f, 200.f);
    goomba.onFireball();
    runner.expect(!goomba.isActive(), "GoombaFireball", "Goomba deactivated immediately on fireball");
    runner.expect(!goomba.isEnemyAlive(), "GoombaFireball", "Goomba marked dead on fireball");
}

void testKoopaShellCycle(TestRunner& runner) {
    Koopa koopa(100.f, 200.f);
    runner.expect(koopa.isEnemyAlive(), "KoopaShellCycle", "Koopa initially alive");
    runner.expect(koopa.isActive(), "KoopaShellCycle", "Koopa initially active");
    runner.expect(!koopa.isInShell(), "KoopaShellCycle", "Koopa initially walking outside shell");
    runner.expect(koopa.getSpeed() == 50.f, "KoopaShellCycle", "Initial walking speed is 50");

    // Stomp 1: Retract into shell (stationary)
    koopa.onStomped();
    runner.expect(koopa.isInShell(), "KoopaShellCycle", "First stomp puts Koopa in shell");
    runner.expect(!koopa.isShellSpinning(), "KoopaShellCycle", "Shell is not spinning initially");
    runner.expect(koopa.getSpeed() == 0.f, "KoopaShellCycle", "Shell is stationary (speed 0)");
    runner.expect(koopa.isActive(), "KoopaShellCycle", "Shell remains active");

    // Stomp 2: Kick shell (spinning at speed 300)
    koopa.onStomped();
    runner.expect(koopa.isInShell(), "KoopaShellCycle", "Koopa remains in shell");
    runner.expect(koopa.isShellSpinning(), "KoopaShellCycle", "Second stomp spins the shell");
    runner.expect(koopa.getSpeed() == 300.f, "KoopaShellCycle", "Spinning shell speed is 300");
    runner.expect(koopa.isActive(), "KoopaShellCycle", "Spinning shell remains active");

    // Stomp 3: Stop / eliminate spinning shell
    koopa.onStomped();
    runner.expect(!koopa.isActive(), "KoopaShellCycle", "Third stomp deactivates spinning shell");
}

void testPiranhaPlantStateMachineAndTiming(TestRunner& runner) {
    PiranhaPlant plant(100.f, 200.f);
    plant.setPipeTopY(200.f);

    // Initial state: WAITING_BOT (starts hidden in pipe)
    runner.expect(plant.getCurrentState() == PiranhaPlant::State::WAITING_BOT,
                  "PiranhaPlant", "Initial state is WAITING_BOT");
    runner.expect(plant.getCurrentRise() == 0.f, "PiranhaPlant", "Initial rise is 0 (hidden)");

    // Stomp immunity: onStomped should do nothing
    plant.onStomped();
    runner.expect(plant.isActive(), "PiranhaPlant", "PiranhaPlant is immune to stomping");
    runner.expect(plant.isEnemyAlive(), "PiranhaPlant", "PiranhaPlant remains alive after stomp");

    // After setPipeTopY, waitTimer is 0 and waitDuration is 2.0s -> wait 2.0s at bottom to transition to RISING
    plant.update(2.01f);
    runner.expect(plant.getCurrentState() == PiranhaPlant::State::RISING,
                  "PiranhaPlant", "State transitions to RISING after bottom wait");

    // Rises at 60px/s for height 24px (24/60 = 0.4s) -> reaches WAITING_TOP
    plant.update(0.41f);
    runner.expect(plant.getCurrentState() == PiranhaPlant::State::WAITING_TOP,
                  "PiranhaPlant", "State transitions to WAITING_TOP at peak rise");
    runner.expect(plant.getCurrentRise() == 24.f, "PiranhaPlant", "Rise reaches maximum 24px");

    // Wait at top for duration (2.0s) -> transitions to DESCENDING
    plant.update(2.01f);
    runner.expect(plant.getCurrentState() == PiranhaPlant::State::DESCENDING,
                  "PiranhaPlant", "State transitions to DESCENDING after top wait");

    // Descends 24px in 0.4s -> reaches WAITING_BOT
    plant.update(0.41f);
    runner.expect(plant.getCurrentState() == PiranhaPlant::State::WAITING_BOT,
                  "PiranhaPlant", "State transitions to WAITING_BOT after descent");
    runner.expect(plant.getCurrentRise() == 0.f, "PiranhaPlant", "Rise is back to 0 (hidden)");
}

void testEnemyDirectionAndProperties(TestRunner& runner) {
    Goomba enemy(50.f, 50.f);
    runner.expect(enemy.getDirection() == -1, "EnemyDirection", "Default direction is -1 (left)");

    enemy.reverseDirection();
    runner.expect(enemy.getDirection() == 1, "EnemyDirection", "Direction reversed to 1 (right)");

    enemy.setSpeed(75.f);
    runner.expect(enemy.getSpeed() == 75.f, "EnemyDirection", "Speed updated to 75");

    enemy.setScoreValue(300);
    runner.expect(enemy.getScoreValue() == 300, "EnemyDirection", "Score value updated to 300");
}

// ============================================================
// Suite 4: Item Collection & State Tests (SOLID-11)
// ============================================================
void testCoinCollectionAndPop(TestRunner& runner) {
    Coin coin(100.f, 100.f);
    runner.expect(coin.isActive(), "CoinTest", "Coin initially active");
    runner.expect(!coin.isCollected(), "CoinTest", "Coin initially not collected");
    runner.expect(!coin.isPopping(), "CoinTest", "Coin initially not popping");

    TestPlayer player(0.f, 0.f);
    RecordingObserver spy;
    ObserverConnection conn = player.addObserver(&spy);

    bool collected = coin.tryCollect(player);
    runner.expect(collected, "CoinTest", "Coin collected by player");
    runner.expect(coin.isCollected(), "CoinTest", "Coin marked as collected");
    runner.expect(!coin.isActive(), "CoinTest", "Coin is deactivated after collection");
    runner.expect(spy.count() == 1, "CoinTest", "Player emitted 1 event on coin collection");

    if (spy.count() >= 1) {
        runner.expect(spy.recordedEvents[0].type == GameEventType::COIN_COLLECTED,
                      "CoinTest", "Event type is COIN_COLLECTED");
        runner.expect(spy.recordedEvents[0].value == 200,
                      "CoinTest", "Default coin score value is 200");
    }

    // Idempotency: second collection attempt must return false and not re-emit
    bool secondCollect = coin.tryCollect(player);
    runner.expect(!secondCollect, "CoinTest", "Second collect returns false");
    runner.expect(spy.count() == 1, "CoinTest", "No duplicate event on second collect");

    // Pop animation test
    Coin popCoin(50.f, 50.f);
    popCoin.startPop();
    runner.expect(popCoin.isPopping(), "CoinTest", "isPopping is true after startPop()");
}

void testMushroomEmergingAndCollect(TestRunner& runner) {
    Mushroom mushroom(100.f, 100.f);
    mushroom.startEmerge();
    runner.expect(mushroom.isEmerging(), "MushroomTest", "Mushroom is emerging after startEmerge()");

    // emergeTarget = 16.f, emergeSpeed = 40.f -> takes 16/40 = 0.4s (use 0.45s to complete)
    mushroom.update(0.45f);
    runner.expect(!mushroom.isEmerging(), "MushroomTest", "Mushroom finishes emerging after 0.45s");

    TestPlayer player(0.f, 0.f);
    runner.expect(player.getCurrentFormName() == "Small", "MushroomTest", "Player initially Small");

    RecordingObserver spy;
    ObserverConnection conn = player.addObserver(&spy);

    bool collected = mushroom.tryCollect(player);
    runner.expect(collected, "MushroomTest", "Mushroom collected successfully");
    runner.expect(player.getCurrentFormName() == "Super", "MushroomTest", "Player transformed into Super Mario");
    runner.expect(mushroom.isCollected(), "MushroomTest", "Mushroom marked collected");
    runner.expect(spy.count() == 1, "MushroomTest", "Powerup event emitted");
    if (spy.count() >= 1) {
        runner.expect(spy.recordedEvents[0].type == GameEventType::POWERUP_COLLECTED,
                      "MushroomTest", "Event is POWERUP_COLLECTED");
    }
}

void testFireFlowerCollectWithPrecondition(TestRunner& runner) {
    TestPlayer player(0.f, 0.f);
    RecordingObserver spy;
    ObserverConnection conn = player.addObserver(&spy);

    // Precondition: Mario must be Super first before FireFlower can upgrade to Fire
    Mushroom mushroom(0.f, 0.f);
    mushroom.tryCollect(player);
    runner.expect(player.getCurrentFormName() == "Super", "FireFlowerTest", "Player upgraded to Super first");

    FireFlower flower(100.f, 100.f);
    bool collected = flower.tryCollect(player);
    runner.expect(collected, "FireFlowerTest", "FireFlower collected successfully");
    runner.expect(player.getCurrentFormName() == "Fire", "FireFlowerTest", "Player transformed into Fire Mario");
    runner.expect(flower.isCollected(), "FireFlowerTest", "Flower marked collected");
}

void testStarItemBounceAndCollect(TestRunner& runner) {
    StarItem star(100.f, 100.f);
    star.notifyGrounded(); // Triggers bounceForce
    runner.expect(star.getVelocity().y < 0.f, "StarItemTest", "StarItem bounces upward when grounded");

    TestPlayer player(0.f, 0.f);
    RecordingObserver spy;
    ObserverConnection conn = player.addObserver(&spy);

    runner.expect(!player.isStarInvincible(), "StarItemTest", "Player initially not star invincible");

    bool collected = star.tryCollect(player);
    runner.expect(collected, "StarItemTest", "StarItem collected successfully");
    runner.expect(player.isStarInvincible(), "StarItemTest", "Player gained star invincibility");
    runner.expect(spy.count() == 1, "StarItemTest", "Event emitted on star collection");
    if (spy.count() >= 1) {
        runner.expect(spy.recordedEvents[0].type == GameEventType::POWERUP_COLLECTED,
                      "StarItemTest", "Event is POWERUP_COLLECTED");
        runner.expect(spy.recordedEvents[0].value == 1000,
                      "StarItemTest", "Star powerup score payload is 1000");
    }
}

void testOneUpMushroomCollect(TestRunner& runner) {
    OneUpMushroom oneUp(100.f, 100.f);
    TestPlayer player(0.f, 0.f);
    RecordingObserver spy;
    ObserverConnection conn = player.addObserver(&spy);

    bool collected = oneUp.tryCollect(player);
    runner.expect(collected, "OneUpMushroomTest", "1-Up mushroom collected");
    runner.expect(oneUp.isCollected(), "OneUpMushroomTest", "1-Up marked collected");
    runner.expect(spy.count() == 1, "OneUpMushroomTest", "Event emitted on 1-Up collection");
    if (spy.count() >= 1) {
        runner.expect(spy.recordedEvents[0].type == GameEventType::LIFE_GAINED,
                      "OneUpMushroomTest", "Event is LIFE_GAINED");
        runner.expect(spy.recordedEvents[0].value == 1,
                      "OneUpMushroomTest", "Life amount payload is 1");
        runner.expect(spy.recordedEvents[0].scoreDelta == 1000,
                      "OneUpMushroomTest", "Score delta is 1000");
    }
}

} // namespace

// ============================================================
// Main Entry Point
// ============================================================
int main() {
    TestRunner runner;

    std::cout << "====================================================\n";
    std::cout << " Running SOLID-11 Automated Unit Tests (Lương Nhật Minh)\n";
    std::cout << "====================================================\n\n";

    // Suite 1: Observer & Subject
    testObserverNormalDispatch(runner);
    testObserverDuplicatePrevention(runner);
    testObserverLifetimeSafety(runner);
    testObserverRAIIConnection(runner);
    testObserverReentrantSafety(runner);
    runner.report("Suite 1: Observer & Subject");

    // Suite 2: HUD Score & Event Integration
    testHUDInitialState(runner);
    testHUDCoinCollection(runner);
    testHUDEnemyDefeated(runner);
    testHUDLifeGained(runner);
    testHUDPlayerDied(runner);
    testHUDTimerAndFreeze(runner);
    runner.report("Suite 2: HUD Score & Events");

    // Suite 3: Enemy State Transitions
    testGoombaStateTransitions(runner);
    testGoombaFireballDefeat(runner);
    testKoopaShellCycle(runner);
    testPiranhaPlantStateMachineAndTiming(runner);
    testEnemyDirectionAndProperties(runner);
    runner.report("Suite 3: Enemy State Transitions");

    // Suite 4: Item Collection & State
    testCoinCollectionAndPop(runner);
    testMushroomEmergingAndCollect(runner);
    testFireFlowerCollectWithPrecondition(runner);
    testStarItemBounceAndCollect(runner);
    testOneUpMushroomCollect(runner);
    runner.report("Suite 4: Item Collection & States");

    std::cout << "\n====================================================\n";
    if (runner.exitCode() == 0) {
        std::cout << " ALL SOLID-11 TESTS PASSED SUCCESSFULLY! (" << runner.passed << "/" << runner.total << " assertions)\n";
    } else {
        std::cout << " SOME TESTS FAILED! (" << runner.failed << "/" << runner.total << " failures)\n";
    }
    std::cout << "====================================================\n";

    return runner.exitCode();
}
