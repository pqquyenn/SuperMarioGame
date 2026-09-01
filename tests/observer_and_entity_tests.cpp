#include "AdminControl/DebugMovementTrail.h"
#include "Commands/CrawlCommand.h"
#include "Entities/Character.h"
#include "Entities/Enemies/Enemy.h"
#include "Entities/Enemies/Goomba.h"
#include "Entities/Enemies/GreenParatroopa.h"
#include "Entities/Enemies/Koopa.h"
#include "Entities/Enemies/PiranhaPlant.h"
#include "Entities/Items/Coin.h"
#include "Entities/Items/FireFlower.h"
#include "Entities/Items/Item.h"
#include "Entities/Items/Mushroom.h"
#include "Entities/Items/OneUpMushroom.h"
#include "Entities/Items/PlaneItem.h"
#include "Entities/Items/StarItem.h"
#include "Entities/Luigi.h"
#include "Entities/Mario.h"
#include "Entities/PlayerPalette.h"
#include "Entities/YellowLaser.h"
#include "Observer/Event.h"
#include "Observer/Observer.h"
#include "Observer/Subject.h"
#include "PlayerEffects/DamageInvincibilityEffect.h"
#include "PlayerEffects/StarEffect.h"
#include "PlayerStates/FireState.h"
#include "PlayerStates/PlaneState.h"
#include "PlayerStates/SmallState.h"
#include "PlayerStates/SuperState.h"
#include "UI/HUD.h"


#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>


namespace {

// ============================================================
// Custom C++17 Test Runner
// ============================================================
struct TestRunner {
  int total = 0;
  int passed = 0;
  int failed = 0;

  void expect(bool condition, const std::string &testName,
              const std::string &message) {
    ++total;
    if (condition) {
      ++passed;
    } else {
      ++failed;
      std::cerr << "  [FAIL] " << testName << " -> " << message << "\n";
    }
  }

  void report(const std::string &suiteName) const {
    std::cout << "[" << suiteName << "] Total: " << total
              << " | Passed: " << passed << " | Failed: " << failed << "\n";
  }

  int exitCode() const { return failed == 0 ? 0 : 1; }
};

// ============================================================
// Test Fixtures: RecordingObserver (Test Spy) & TestPlayer
// ============================================================
class RecordingObserver : public Observer {
public:
  std::vector<GameEvent> recordedEvents;

  void onNotify(const GameEvent &event) override {
    recordedEvents.push_back(event);
  }

  void clear() { recordedEvents.clear(); }

  std::size_t count() const { return recordedEvents.size(); }
};

class TestPlayer : public Character {
public:
  TestPlayer(float x = 0.f, float y = 0.f) : Character(x, y) {}

  sf::IntRect getTextureRectForTest() const { return sprite.getTextureRect(); }
};

// ============================================================
// Suite 1: Observer & Subject Tests (SOLID-08)
// ============================================================
void testObserverNormalDispatch(TestRunner &runner) {
  Subject subject;
  RecordingObserver observer1;
  RecordingObserver observer2;

  ObserverConnection conn1 = subject.addObserver(&observer1);
  ObserverConnection conn2 = subject.addObserver(&observer2);

  runner.expect(conn1.isConnected(), "NormalDispatch",
                "Connection 1 is connected");
  runner.expect(conn2.isConnected(), "NormalDispatch",
                "Connection 2 is connected");

  subject.notify(GameEvent::coinCollected(200));
  subject.notify(GameEvent::enemyDefeated(100));

  runner.expect(observer1.count() == 2, "NormalDispatch",
                "Observer 1 received 2 events");
  runner.expect(observer2.count() == 2, "NormalDispatch",
                "Observer 2 received 2 events");

  if (observer1.count() >= 2) {
    runner.expect(observer1.recordedEvents[0].type ==
                      GameEventType::COIN_COLLECTED,
                  "NormalDispatch", "Event 1 is COIN_COLLECTED");
    runner.expect(observer1.recordedEvents[0].value == 200, "NormalDispatch",
                  "Event 1 value is 200");
    runner.expect(observer1.recordedEvents[1].type ==
                      GameEventType::ENEMY_DEFEATED,
                  "NormalDispatch", "Event 2 is ENEMY_DEFEATED");
    runner.expect(observer1.recordedEvents[1].value == 100, "NormalDispatch",
                  "Event 2 value is 100");
  }
}

void testObserverDuplicatePrevention(TestRunner &runner) {
  Subject subject;
  RecordingObserver observer;

  ObserverConnection conn1 = subject.addObserver(&observer);
  ObserverConnection conn2 = subject.addObserver(&observer); // Duplicate

  runner.expect(conn1.isConnected(), "DuplicatePrevention",
                "First connection is connected");
  runner.expect(!conn2.isConnected(), "DuplicatePrevention",
                "Duplicate connection is empty / not connected");

  subject.notify(GameEvent::enemyDefeated(100));

  runner.expect(observer.count() == 1, "DuplicatePrevention",
                "Duplicate registration does not double-notify");
}

void testObserverLifetimeSafety(TestRunner &runner) {
  Subject subject;
  ObserverConnection conn;

  {
    RecordingObserver localObserver;
    conn = subject.addObserver(&localObserver);
    runner.expect(conn.isConnected(), "LifetimeSafety",
                  "Connection active while observer lives");
    subject.notify(GameEvent::enemyDefeated(100));
    runner.expect(localObserver.count() == 1, "LifetimeSafety",
                  "Local observer received event");
  } // localObserver destroyed here, but conn still exists in outer scope

  runner.expect(!conn.isConnected(), "LifetimeSafety",
                "Connection detects dead observer token");

  // Must NOT crash or dereference dangling pointer
  subject.notify(GameEvent::enemyDefeated(200));
  runner.expect(true, "LifetimeSafety",
                "Notifying after observer destruction does not crash");
}

void testObserverRAIIConnection(TestRunner &runner) {
  Subject subject;
  RecordingObserver observer;

  {
    ObserverConnection conn = subject.addObserver(&observer);
    runner.expect(conn.isConnected(), "RAIIConnection",
                  "Connection is connected");
    subject.notify(GameEvent::coinCollected(200));
    runner.expect(observer.count() == 1, "RAIIConnection",
                  "Observer notified before disconnect");
    conn.disconnect();
    runner.expect(!conn.isConnected(), "RAIIConnection",
                  "Connection disconnected explicitly");
    subject.notify(GameEvent::coinCollected(200));
    runner.expect(observer.count() == 1, "RAIIConnection",
                  "No notification after explicit disconnect");
  }

  // New connection that destructs naturally
  {
    ObserverConnection conn2 = subject.addObserver(&observer);
    runner.expect(conn2.isConnected(), "RAIIConnection",
                  "Connection 2 is connected");
  } // conn2 destructor calls disconnect()

  subject.notify(GameEvent::coinCollected(200));
  runner.expect(observer.count() == 1, "RAIIConnection",
                "Destructed connection automatically stopped notifications");
}

void testObserverReentrantSafety(TestRunner &runner) {
  Subject subject;

  class SelfDisconnectingObserver : public Observer {
  public:
    ObserverConnection connection;
    int receiveCount = 0;

    void onNotify(const GameEvent &event) override {
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

  runner.expect(selfDiscObserver.receiveCount == 1, "ReentrantSafety",
                "Self-disconnecting observer notified once");
  runner.expect(bystanderObserver.count() == 1, "ReentrantSafety",
                "Bystander observer still received event");

  // Second notify: self-disconnecting observer should no longer receive event
  subject.notify(GameEvent::enemyDefeated(100));
  runner.expect(selfDiscObserver.receiveCount == 1, "ReentrantSafety",
                "Self-disconnected observer no longer receives event");
  runner.expect(bystanderObserver.count() == 2, "ReentrantSafety",
                "Bystander observer continues to receive event");
}

// ============================================================
// Suite 2: HUD Score & Event Integration Tests (SOLID-08)
// ============================================================
void testHUDInitialState(TestRunner &runner) {
  HUD hud;
  runner.expect(hud.getScore() == 0, "HUDInitialState", "Initial score is 0");
  runner.expect(hud.getCoins() == 0, "HUDInitialState", "Initial coins is 0");
  runner.expect(hud.getLives() == 3, "HUDInitialState", "Initial lives is 3");
  runner.expect(std::abs(hud.getTimeRemaining() - 400.f) < 0.001f,
                "HUDInitialState", "Initial time is 400");
  runner.expect(!hud.isTimeFrozen(), "HUDInitialState",
                "Time is not frozen by default");
}

void testHUDCoinCollection(TestRunner &runner) {
  HUD hud;
  hud.onNotify(GameEvent::coinCollected(200));

  runner.expect(hud.getCoins() == 1, "HUDCoinCollection",
                "Coin count incremented to 1");
  runner.expect(hud.getScore() == 200, "HUDCoinCollection",
                "Score increased by 200");

  hud.onNotify(GameEvent::coinCollected(300));
  runner.expect(hud.getCoins() == 2, "HUDCoinCollection",
                "Coin count incremented to 2");
  runner.expect(hud.getScore() == 500, "HUDCoinCollection",
                "Score increased by 300 to 500");
}

void testHUDEnemyDefeated(TestRunner &runner) {
  HUD hud;
  hud.onNotify(GameEvent::enemyDefeated(100));
  runner.expect(hud.getScore() == 100, "HUDEnemyDefeated",
                "Score increased to 100");
  runner.expect(hud.getCoins() == 0, "HUDEnemyDefeated",
                "Coin count remains 0");

  hud.onNotify(GameEvent::enemyDefeated(400));
  runner.expect(hud.getScore() == 500, "HUDEnemyDefeated",
                "Score increased to 500");
}

void testHUDLifeGained(TestRunner &runner) {
  HUD hud;
  hud.onNotify(GameEvent::lifeGained(1, 1000));
  runner.expect(hud.getLives() == 4, "HUDLifeGained",
                "Lives increased from 3 to 4");
  runner.expect(hud.getScore() == 1000, "HUDLifeGained",
                "Score increased by 1000");

  hud.onNotify(GameEvent::lifeGained(2, 500));
  runner.expect(hud.getLives() == 6, "HUDLifeGained",
                "Lives increased by 2 to 6");
  runner.expect(hud.getScore() == 1500, "HUDLifeGained",
                "Score increased by 500 to 1500");
}

void testHUDPlayerDied(TestRunner &runner) {
  HUD hud;
  hud.onNotify(GameEvent::playerDied(0));
  runner.expect(hud.getLives() == 2, "HUDPlayerDied",
                "Lives decremented from 3 to 2");
  hud.onNotify(GameEvent::playerDied(0));
  runner.expect(hud.getLives() == 1, "HUDPlayerDied", "Lives decremented to 1");
}

void testHUDTimerAndFreeze(TestRunner &runner) {
  HUD hud;
  hud.update(10.f);
  runner.expect(std::abs(hud.getTimeRemaining() - 390.f) < 0.01f,
                "HUDTimerAndFreeze", "Time decreased by 10s");

  hud.setTimeFrozen(true);
  runner.expect(hud.isTimeFrozen(), "HUDTimerAndFreeze",
                "Time freeze flag is set");
  hud.update(10.f);
  runner.expect(std::abs(hud.getTimeRemaining() - 390.f) < 0.01f,
                "HUDTimerAndFreeze", "Time remains frozen");

  hud.setTimeFrozen(false);
  hud.update(5.f);
  runner.expect(std::abs(hud.getTimeRemaining() - 385.f) < 0.01f,
                "HUDTimerAndFreeze", "Time resumes countdown");
}

// ============================================================
// Suite 3: Enemy State Transitions Tests (SOLID-11)
// ============================================================
void testGoombaStateTransitions(TestRunner &runner) {
  Goomba goomba(100.f, 200.f);
  runner.expect(goomba.isEnemyAlive(), "GoombaState", "Goomba initially alive");
  runner.expect(goomba.isActive(), "GoombaState", "Goomba initially active");
  runner.expect(!goomba.isSquished(), "GoombaState",
                "Goomba initially not squished");

  goomba.onStomped();
  runner.expect(goomba.isSquished(), "GoombaState",
                "Goomba is squished after stomp");
  runner.expect(goomba.isActive(), "GoombaState",
                "Goomba still active during squish animation");
  runner.expect(std::abs(goomba.getSquishTimer() - 0.f) < 0.001f, "GoombaState",
                "Squish timer reset to 0");

  // Advance squish timer partially (0.25s / 0.5s duration)
  goomba.update(0.25f);
  runner.expect(goomba.isActive(), "GoombaState",
                "Goomba active at 0.25s squish duration");

  // Advance past 0.5s duration
  goomba.update(0.26f);
  runner.expect(!goomba.isActive(), "GoombaState",
                "Goomba deactivated after 0.5s squish duration");
  runner.expect(!goomba.isEnemyAlive(), "GoombaState", "Goomba is marked dead");
}

void testGoombaFireballDefeat(TestRunner &runner) {
  Goomba goomba(100.f, 200.f);
  goomba.onFireball();
  runner.expect(!goomba.isActive(), "GoombaFireball",
                "Goomba deactivated immediately on fireball");
  runner.expect(!goomba.isEnemyAlive(), "GoombaFireball",
                "Goomba marked dead on fireball");
}

void testKoopaShellCycle(TestRunner &runner) {
  Koopa koopa(100.f, 200.f);
  runner.expect(koopa.isEnemyAlive(), "KoopaShellCycle",
                "Koopa initially alive");
  runner.expect(koopa.isActive(), "KoopaShellCycle", "Koopa initially active");
  runner.expect(!koopa.isInShell(), "KoopaShellCycle",
                "Koopa initially walking outside shell");
  runner.expect(koopa.getSpeed() == 50.f, "KoopaShellCycle",
                "Initial walking speed is 50");

  // Stomp 1: Retract into shell (stationary)
  koopa.onStomped();
  runner.expect(koopa.isInShell(), "KoopaShellCycle",
                "First stomp puts Koopa in shell");
  runner.expect(!koopa.isShellSpinning(), "KoopaShellCycle",
                "Shell is not spinning initially");
  runner.expect(koopa.getSpeed() == 0.f, "KoopaShellCycle",
                "Shell is stationary (speed 0)");
  runner.expect(koopa.isActive(), "KoopaShellCycle", "Shell remains active");

  // Stomp 2: Kick shell (spinning at speed 300)
  koopa.onStomped();
  runner.expect(koopa.isInShell(), "KoopaShellCycle", "Koopa remains in shell");
  runner.expect(koopa.isShellSpinning(), "KoopaShellCycle",
                "Second stomp spins the shell");
  runner.expect(koopa.getSpeed() == 300.f, "KoopaShellCycle",
                "Spinning shell speed is 300");
  runner.expect(koopa.isActive(), "KoopaShellCycle",
                "Spinning shell remains active");

  // Stomp 3: Stop / eliminate spinning shell
  koopa.onStomped();
  runner.expect(!koopa.isActive(), "KoopaShellCycle",
                "Third stomp deactivates spinning shell");
}

void testPiranhaPlantStateMachineAndTiming(TestRunner &runner) {
  PiranhaPlant plant(100.f, 200.f);
  plant.setPipeTopY(200.f);

  // Initial state: WAITING_BOT (starts hidden in pipe)
  runner.expect(plant.getCurrentState() == PiranhaPlant::State::WAITING_BOT,
                "PiranhaPlant", "Initial state is WAITING_BOT");
  runner.expect(plant.getCurrentRise() == 0.f, "PiranhaPlant",
                "Initial rise is 0 (hidden)");

  // Stomp immunity: onStomped should do nothing
  plant.onStomped();
  runner.expect(plant.isActive(), "PiranhaPlant",
                "PiranhaPlant is immune to stomping");
  runner.expect(plant.isEnemyAlive(), "PiranhaPlant",
                "PiranhaPlant remains alive after stomp");

  // The default first appearance waits one second.
  plant.update(1.01f);
  runner.expect(plant.getCurrentState() == PiranhaPlant::State::RISING,
                "PiranhaPlant",
                "State transitions to RISING after bottom wait");

  // Rises at 60px/s for height 24px (24/60 = 0.4s) -> reaches WAITING_TOP
  plant.update(0.41f);
  runner.expect(plant.getCurrentState() == PiranhaPlant::State::WAITING_TOP,
                "PiranhaPlant",
                "State transitions to WAITING_TOP at peak rise");
  runner.expect(plant.getCurrentRise() == 24.f, "PiranhaPlant",
                "Rise reaches maximum 24px");

  // Wait at top for duration (2.0s) -> transitions to DESCENDING
  plant.update(2.01f);
  runner.expect(plant.getCurrentState() == PiranhaPlant::State::DESCENDING,
                "PiranhaPlant",
                "State transitions to DESCENDING after top wait");

  // Descends 24px in 0.4s -> reaches WAITING_BOT
  plant.update(0.41f);
  runner.expect(plant.getCurrentState() == PiranhaPlant::State::WAITING_BOT,
                "PiranhaPlant",
                "State transitions to WAITING_BOT after descent");
  runner.expect(plant.getCurrentRise() == 0.f, "PiranhaPlant",
                "Rise is back to 0 (hidden)");

  plant.setCycleTiming(1.25f, 3.f, 0.5f);
  runner.expect(plant.getVisibleDuration() == 1.25f, "PiranhaPlantTiming",
                "Visible duration is configurable");
  runner.expect(plant.getHiddenDuration() == 3.f, "PiranhaPlantTiming",
                "Hidden duration is configurable");
  plant.update(0.49f);
  runner.expect(plant.getCurrentState() == PiranhaPlant::State::WAITING_BOT,
                "PiranhaPlantTiming", "Plant honors its initial hidden delay");
  plant.update(0.02f);
  runner.expect(plant.getCurrentState() == PiranhaPlant::State::RISING,
                "PiranhaPlantTiming", "Plant rises after its initial delay");

  plant.update(0.41f);
  plant.update(1.26f);
  plant.update(0.41f);
  runner.expect(plant.getCurrentState() == PiranhaPlant::State::WAITING_BOT,
                "PiranhaPlantTiming", "Plant returns to its hidden state");
  plant.update(2.99f);
  runner.expect(plant.getCurrentState() == PiranhaPlant::State::WAITING_BOT,
                "PiranhaPlantTiming", "Recurring hidden duration is honored");
  plant.update(0.02f);
  runner.expect(plant.getCurrentState() == PiranhaPlant::State::RISING,
                "PiranhaPlantTiming", "Plant repeats after hidden duration");
}

void testAlternatingPiranhaPlantTiming(TestRunner &runner) {
  PiranhaPlant left(192.f, 176.f);
  PiranhaPlant right(208.f, 176.f);
  left.setCycleTiming(1.5f, 2.3f, 0.f);
  right.setCycleTiming(1.5f, 2.3f, 2.3f);

  std::vector<int> appearanceOrder;
  bool leftWasVisible = false;
  bool rightWasVisible = false;
  bool neverOverlap = true;
  constexpr float FrameTime = 1.f / 60.f;
  for (int frame = 0; frame < 900; ++frame) {
    left.update(FrameTime);
    right.update(FrameTime);
    const bool leftVisible = left.getCurrentRise() > 0.f;
    const bool rightVisible = right.getCurrentRise() > 0.f;
    neverOverlap = neverOverlap && !(leftVisible && rightVisible);
    if (leftVisible && !leftWasVisible) {
      appearanceOrder.push_back(1);
    }
    if (rightVisible && !rightWasVisible) {
      appearanceOrder.push_back(2);
    }
    leftWasVisible = leftVisible;
    rightWasVisible = rightVisible;
  }

  bool alternates = appearanceOrder.size() >= 6 && appearanceOrder.front() == 1;
  for (std::size_t index = 1; index < appearanceOrder.size(); ++index) {
    alternates =
        alternates && appearanceOrder[index] != appearanceOrder[index - 1];
  }
  runner.expect(neverOverlap, "PiranhaPlantAlternation",
                "Staggered plants are never visible together");
  runner.expect(alternates, "PiranhaPlantAlternation",
                "Staggered plants repeatedly appear left then right");
}

void testEnemyDirectionAndProperties(TestRunner &runner) {
  Goomba enemy(50.f, 50.f);
  runner.expect(enemy.getDirection() == -1, "EnemyDirection",
                "Default direction is -1 (left)");

  enemy.reverseDirection();
  runner.expect(enemy.getDirection() == 1, "EnemyDirection",
                "Direction reversed to 1 (right)");

  enemy.setSpeed(75.f);
  runner.expect(enemy.getSpeed() == 75.f, "EnemyDirection",
                "Speed updated to 75");

  enemy.setScoreValue(300);
  runner.expect(enemy.getScoreValue() == 300, "EnemyDirection",
                "Score value updated to 300");
}

// ============================================================
// Suite 4: Item Collection & State Tests (SOLID-11)
// ============================================================
void testCoinCollectionAndPop(TestRunner &runner) {
  Coin coin(100.f, 100.f);
  runner.expect(coin.isActive(), "CoinTest", "Coin initially active");
  runner.expect(!coin.isCollected(), "CoinTest",
                "Coin initially not collected");
  runner.expect(!coin.isPopping(), "CoinTest", "Coin initially not popping");

  TestPlayer player(0.f, 0.f);
  RecordingObserver spy;
  ObserverConnection conn = player.addObserver(&spy);

  bool collected = coin.tryCollect(player);
  runner.expect(collected, "CoinTest", "Coin collected by player");
  runner.expect(coin.isCollected(), "CoinTest", "Coin marked as collected");
  runner.expect(!coin.isActive(), "CoinTest",
                "Coin is deactivated after collection");
  runner.expect(spy.count() == 1, "CoinTest",
                "Player emitted 1 event on coin collection");

  if (spy.count() >= 1) {
    runner.expect(spy.recordedEvents[0].type == GameEventType::COIN_COLLECTED,
                  "CoinTest", "Event type is COIN_COLLECTED");
    runner.expect(spy.recordedEvents[0].value == 200, "CoinTest",
                  "Default coin score value is 200");
  }

  // Idempotency: second collection attempt must return false and not re-emit
  bool secondCollect = coin.tryCollect(player);
  runner.expect(!secondCollect, "CoinTest", "Second collect returns false");
  runner.expect(spy.count() == 1, "CoinTest",
                "No duplicate event on second collect");

  // Pop animation test
  Coin popCoin(50.f, 50.f);
  popCoin.startPop();
  runner.expect(popCoin.isPopping(), "CoinTest",
                "isPopping is true after startPop()");
}

void testMushroomEmergingAndCollect(TestRunner &runner) {
  Mushroom mushroom(100.f, 100.f);
  mushroom.startEmerge();
  runner.expect(mushroom.isEmerging(), "MushroomTest",
                "Mushroom is emerging after startEmerge()");

  // emergeTarget = 16.f, emergeSpeed = 40.f -> takes 16/40 = 0.4s (use 0.45s to
  // complete)
  mushroom.update(0.45f);
  runner.expect(!mushroom.isEmerging(), "MushroomTest",
                "Mushroom finishes emerging after 0.45s");

  TestPlayer player(0.f, 0.f);
  runner.expect(player.getCurrentFormName() == "Small", "MushroomTest",
                "Player initially Small");

  RecordingObserver spy;
  ObserverConnection conn = player.addObserver(&spy);

  bool collected = mushroom.tryCollect(player);
  runner.expect(collected, "MushroomTest", "Mushroom collected successfully");
  runner.expect(player.getCurrentFormName() == "Super", "MushroomTest",
                "Player transformed into Super Mario");
  runner.expect(mushroom.isCollected(), "MushroomTest",
                "Mushroom marked collected");
  runner.expect(spy.count() == 1, "MushroomTest", "Powerup event emitted");
  if (spy.count() >= 1) {
    runner.expect(spy.recordedEvents[0].type ==
                      GameEventType::POWERUP_COLLECTED,
                  "MushroomTest", "Event is POWERUP_COLLECTED");
  }
}

void testFireFlowerCollectWithPrecondition(TestRunner &runner) {
  TestPlayer player(0.f, 0.f);
  RecordingObserver spy;
  ObserverConnection conn = player.addObserver(&spy);

  // Precondition: Mario must be Super first before FireFlower can upgrade to
  // Fire
  Mushroom mushroom(0.f, 0.f);
  mushroom.tryCollect(player);
  runner.expect(player.getCurrentFormName() == "Super", "FireFlowerTest",
                "Player upgraded to Super first");

  FireFlower flower(100.f, 100.f);
  bool collected = flower.tryCollect(player);
  runner.expect(collected, "FireFlowerTest",
                "FireFlower collected successfully");
  runner.expect(player.getCurrentFormName() == "Fire", "FireFlowerTest",
                "Player transformed into Fire Mario");
  runner.expect(flower.isCollected(), "FireFlowerTest",
                "Flower marked collected");

  // Fire Mario collects another Super Mushroom -> remains Fire Mario
  Mushroom mushroom2(50.f, 50.f);
  bool shroom2Collected = mushroom2.tryCollect(player);
  runner.expect(shroom2Collected, "FireFlowerTest",
                "Fire Mario collects Mushroom");
  runner.expect(player.getCurrentFormName() == "Fire", "FireFlowerTest",
                "Player remains Fire Mario after eating Mushroom");
  runner.expect(mushroom2.isCollected(), "FireFlowerTest",
                "Mushroom marked collected");

  const std::size_t eventsBeforeExpiry = spy.count();
  runner.expect(player.expireFireForm(), "FireFlowerTest",
                "Timed rules can explicitly expire Fire form");
  runner.expect(player.getCurrentFormName() == "Super", "FireFlowerTest",
                "Expired Fire form returns to Super");
  runner.expect(spy.count() == eventsBeforeExpiry, "FireFlowerTest",
                "Fire expiry does not emit a fake collection event");
  runner.expect(!player.expireFireForm(), "FireFlowerTest",
                "Expiring a non-Fire form is a no-op");
}

void testPoweredCharacterCrouch(TestRunner &runner) {
  CrawlCommand crawlCommand;
  TestPlayer smallPlayer(20.f, 30.f);
  smallPlayer.setGrounded(true);
  crawlCommand.execute(smallPlayer, 0.f);
  runner.expect(!smallPlayer.isCrouching(), "PoweredCrouch",
                "Small form ignores crouch input");

  TestPlayer player(100.f, 50.f);
  Mushroom mushroom(0.f, 0.f);
  mushroom.tryCollect(player);
  player.setGrounded(true);

  const sf::FloatRect standingBefore = player.getBounds();
  const float feetBefore = standingBefore.top + standingBefore.height;
  crawlCommand.execute(player, 0.f);
  const sf::FloatRect crouchingBounds = player.getBounds();

  runner.expect(player.isCrouching(), "PoweredCrouch",
                "Super form enters crouch while grounded");
  runner.expect(player.getTextureRectForTest() == sf::IntRect{17, 25, 16, 32},
                "PoweredCrouch", "Super crouch selects powered atlas frame 1");
  runner.expect(std::abs(crouchingBounds.height - 16.f) < 0.001f,
                "PoweredCrouch", "Crouch uses Small-height collision body");
  runner.expect(std::abs(crouchingBounds.top + crouchingBounds.height -
                         feetBefore) < 0.001f,
                "PoweredCrouch", "Crouch keeps the feet anchored");

  player.moveRight(1.f);
  player.jump();
  runner.expect(player.getVelocity().x > 0.f && player.getVelocity().x <= 70.f,
                "PoweredCrouch",
                "Crouching permits movement at the reduced crawl speed");
  runner.expect(std::abs(player.getVelocity().y) < 0.001f, "PoweredCrouch",
                "Crouching blocks jumping");

  crawlCommand.release(player);
  runner.expect(player.isCrouching(), "PoweredCrouch",
                "Release waits for terrain clearance before standing");
  const sf::FloatRect proposedStanding = player.getStandingBounds();
  const sf::FloatRect proposedHeadroom = player.getStandingHeadroomBounds();
  runner.expect(std::abs(proposedStanding.top - standingBefore.top) < 0.001f,
                "PoweredCrouch",
                "Proposed standing bounds restore full height");
  runner.expect(std::abs(proposedHeadroom.height - 16.f) < 0.001f &&
                    std::abs(proposedHeadroom.top + proposedHeadroom.height -
                             crouchingBounds.top) < 0.001f,
                "PoweredCrouch",
                "Stand-up clearance checks only the added upper headroom");

  player.standUp();
  const sf::FloatRect standingAfter = player.getBounds();
  runner.expect(!player.isCrouching(), "PoweredCrouch",
                "Character stands after clearance is approved");
  runner.expect(std::abs(standingAfter.height - 32.f) < 0.001f, "PoweredCrouch",
                "Standing restores powered collision height");
  runner.expect(
      std::abs(standingAfter.top + standingAfter.height - feetBefore) < 0.001f,
      "PoweredCrouch", "Standing also keeps the feet anchored");

  FireFlower flower(0.f, 0.f);
  flower.tryCollect(player);
  player.setGrounded(true);
  crawlCommand.execute(player, 0.f);
  int projectileRequests = 0;
  player.setProjectileRequestHandler(
      [&projectileRequests](const ProjectileRequest &) {
        ++projectileRequests;
      });
  player.shootFireball();
  runner.expect(player.isCrouching(), "PoweredCrouch",
                "Fire form also supports crouching");
  runner.expect(player.getTextureRectForTest() == sf::IntRect{17, 153, 16, 32},
                "PoweredCrouch", "Fire crouch selects Fire atlas frame 1");
  runner.expect(projectileRequests == 0, "PoweredCrouch",
                "Fire form cannot shoot while crouching");
}

void testCharacterReleaseTiming(TestRunner &runner) {
  Mario mario;
  Luigi luigi;
  Mushroom marioMushroom(0.f, 0.f);
  Mushroom luigiMushroom(0.f, 0.f);
  marioMushroom.tryCollect(mario);
  luigiMushroom.tryCollect(luigi);
  mario.setGrounded(true);
  luigi.setGrounded(true);

  CrawlCommand crawlCommand;
  crawlCommand.execute(mario, 0.f);
  crawlCommand.execute(luigi, 0.f);
  mario.moveRight(1.f);
  luigi.moveRight(1.f);
  mario.update(0.f);
  luigi.update(0.f);
  crawlCommand.release(mario);
  crawlCommand.release(luigi);
  mario.update(0.05f);
  luigi.update(0.05f);

  runner.expect(std::abs(mario.getVelocity().x) < 0.001f &&
                    std::abs(luigi.getVelocity().x) < 0.001f,
                "ReleaseTiming",
                "Mario and Luigi stop from crawl speed in the same time");
}

void testCrawlUsesFinalGroundContact(TestRunner &runner) {
  TestPlayer player(100.f, 50.f);
  Mushroom mushroom(0.f, 0.f);
  mushroom.tryCollect(player);
  player.setGrounded(true);

  CrawlCommand crawlCommand;
  crawlCommand.execute(player, 0.f);

  // Tile collision clears grounded first; a later moving-platform pass
  // restores it before stance finalization.
  player.setGrounded(false);
  player.setGrounded(true);
  player.resolveCrouchState(false);
  runner.expect(player.isCrouching(), "CrawlFinalGroundContact",
                "Moving-platform grounding preserves held crawl");

  // Without a final ground contact, held Down alone must not preserve crawl.
  player.setGrounded(false);
  player.resolveCrouchState(false);
  runner.expect(!player.isCrouching(), "CrawlFinalGroundContact",
                "Walking off a platform restores standing in open air");
}

void testRulesetMovementScale(TestRunner &runner) {
  Mario solo;
  solo.setGrounded(true);
  solo.moveRight(1.f);

  Mario pvp;
  pvp.setHorizontalMovementScale(0.82f);
  pvp.setGrounded(true);
  pvp.moveRight(1.f);

  runner.expect(std::abs(solo.getVelocity().x - 150.f) < 0.01f,
                "RulesetMovementScale", "solo keeps Mario's normal walk speed");
  runner.expect(std::abs(pvp.getVelocity().x - 123.f) < 0.01f,
                "RulesetMovementScale",
                "PvP scales speed without replacing the character profile");
  runner.expect(std::abs(pvp.getHorizontalMovementScale() - 0.82f) < 0.001f,
                "RulesetMovementScale",
                "the selected ruleset scale is observable");
}

void testMarioCanReachPvPSuperPlatform(TestRunner &runner) {
  Mario mario;
  mario.setPosition(0.f, 192.f);
  mario.receivePowerUp(std::make_unique<SuperState>());
  mario.setGrounded(true);

  const float startingFeet = mario.getBounds().top + mario.getBounds().height;
  float highestFeet = startingFeet;
  mario.setJumpHeld(true);
  mario.jump();

  constexpr float frameTime = 1.f / 60.f;
  for (int frame = 0; frame < 120; ++frame) {
    mario.setJumpHeld(true);
    mario.update(frameTime);
    highestFeet =
        std::min(highestFeet, mario.getBounds().top + mario.getBounds().height);
    if (frame > 0 && mario.getVelocity().y >= 0.f) {
      break;
    }
  }

  constexpr float ArenaPlatformRise = 64.f;
  runner.expect(startingFeet - highestFeet >= ArenaPlatformRise,
                "PvPSuperPlatformReach",
                "Super Mario can reach the lowered PvP ledge");
}

struct HeldJumpMetrics {
  float height{0.f};
  float airtime{0.f};
};

HeldJumpMetrics measureHeldJump(Character &character) {
  character.setPosition(0.f, 192.f);
  character.receivePowerUp(std::make_unique<SuperState>());
  character.setGrounded(true);
  const float startingFeet =
      character.getBounds().top + character.getBounds().height;
  float highestFeet = startingFeet;
  constexpr float FrameTime = 1.f / 60.f;

  character.setJumpHeld(true);
  character.jump();
  HeldJumpMetrics metrics;
  for (int frame = 0; frame < 180; ++frame) {
    character.setJumpHeld(true);
    character.update(FrameTime);
    metrics.airtime += FrameTime;
    const float currentFeet =
        character.getBounds().top + character.getBounds().height;
    highestFeet = std::min(highestFeet, currentFeet);
    if (character.getVelocity().y >= 0.f && currentFeet >= startingFeet) {
      break;
    }
  }
  metrics.height = startingFeet - highestFeet;
  return metrics;
}

void testLuigiHeldJumpBalance(TestRunner &runner) {
  Mario mario;
  Luigi luigi;
  const HeldJumpMetrics marioJump = measureHeldJump(mario);
  const HeldJumpMetrics luigiJump = measureHeldJump(luigi);

  runner.expect(luigiJump.height > marioJump.height, "LuigiJumpBalance",
                "Luigi retains his higher-jump identity");
  runner.expect(luigiJump.height <= 6.25f * 16.f, "LuigiJumpBalance",
                "Luigi's held jump stays near 6.2 tiles");
  runner.expect(luigiJump.airtime - marioJump.airtime <= 0.18f,
                "LuigiJumpBalance", "Luigi's airtime advantage is bounded");
}

void testDebugMovementTrailEvents(TestRunner &runner) {
  TestPlayer player;
  player.setGrounded(true);

  DebugMovementTrail trail;
  trail.start(player);
  player.setPosition(2.f, -2.f);
  player.setVelocity(100.f, -120.f);
  player.setGrounded(false);
  trail.update(player, 1.f / 60.f);

  player.setPosition(4.f, -4.f);
  player.setVelocity(0.f, -80.f);
  trail.update(player, 1.f / 60.f);

  player.setPosition(4.f, 0.f);
  player.setVelocity(0.f, 0.f);
  player.setGrounded(true);
  trail.update(player, 1.f / 60.f);

  runner.expect(trail.getEventCount(DebugTrailEvent::Takeoff) == 1,
                "DebugMovementTrail",
                "trail marks the grounded-to-airborne transition");
  runner.expect(trail.getEventCount(DebugTrailEvent::WallImpact) == 1,
                "DebugMovementTrail",
                "trail marks horizontal stopping while airborne");
  runner.expect(trail.getEventCount(DebugTrailEvent::Landing) == 1,
                "DebugMovementTrail",
                "trail marks the airborne-to-grounded transition");

  trail.update(player, 9.f);
  runner.expect(!trail.isActive(), "DebugMovementTrail",
                "trail automatically expires after its debug window");
}

void testStarItemBounceAndCollect(TestRunner &runner) {
  StarItem star(100.f, 100.f);
  star.notifyGrounded(); // Triggers bounceForce
  runner.expect(star.getVelocity().y < 0.f, "StarItemTest",
                "StarItem bounces upward when grounded");

  TestPlayer player(0.f, 0.f);
  RecordingObserver spy;
  ObserverConnection conn = player.addObserver(&spy);

  runner.expect(!player.isStarInvincible(), "StarItemTest",
                "Player initially not star invincible");

  bool collected = star.tryCollect(player);
  runner.expect(collected, "StarItemTest", "StarItem collected successfully");
  runner.expect(player.isStarInvincible(), "StarItemTest",
                "Player gained star invincibility");
  runner.expect(spy.count() == 1, "StarItemTest",
                "Event emitted on star collection");
  if (spy.count() >= 1) {
    runner.expect(spy.recordedEvents[0].type ==
                      GameEventType::POWERUP_COLLECTED,
                  "StarItemTest", "Event is POWERUP_COLLECTED");
    runner.expect(spy.recordedEvents[0].value == 1000, "StarItemTest",
                  "Star powerup score payload is 1000");
  }
}

void testOneUpMushroomCollect(TestRunner &runner) {
  OneUpMushroom oneUp(100.f, 100.f);
  TestPlayer player(0.f, 0.f);
  RecordingObserver spy;
  ObserverConnection conn = player.addObserver(&spy);

  bool collected = oneUp.tryCollect(player);
  runner.expect(collected, "OneUpMushroomTest", "1-Up mushroom collected");
  runner.expect(oneUp.isCollected(), "OneUpMushroomTest",
                "1-Up marked collected");
  runner.expect(spy.count() == 1, "OneUpMushroomTest",
                "Event emitted on 1-Up collection");
  if (spy.count() >= 1) {
    runner.expect(spy.recordedEvents[0].type == GameEventType::LIFE_GAINED,
                  "OneUpMushroomTest", "Event is LIFE_GAINED");
    runner.expect(spy.recordedEvents[0].value == 1, "OneUpMushroomTest",
                  "Life amount payload is 1");
    runner.expect(spy.recordedEvents[0].scoreDelta == 1000, "OneUpMushroomTest",
                  "Score delta is 1000");
  }
}

// ============================================================
// Suite 5: OCP Polymorphism Tests (Lương Nhật Minh)
// ============================================================
void testItemShouldSkipTileCollision(TestRunner &runner) {
  // 1. Mushroom
  {
    Mushroom m;
    runner.expect(!m.shouldSkipTileCollision(), "MushroomTileCollision",
                  "Initial mushroom does not skip tile collision");
    m.startEmerge();
    runner.expect(m.shouldSkipTileCollision(), "MushroomTileCollision",
                  "Emerging mushroom skips tile collision");
    m.update(0.5f); // 40px/s * 0.5s = 20px >= 16px (emergeTarget)
    runner.expect(!m.shouldSkipTileCollision(), "MushroomTileCollision",
                  "Finished emerge mushroom does not skip tile collision");
  }

  // 2. FireFlower
  {
    FireFlower f;
    runner.expect(!f.shouldSkipTileCollision(), "FireFlowerTileCollision",
                  "Initial fire flower does not skip tile collision");
    f.startEmerge();
    runner.expect(f.shouldSkipTileCollision(), "FireFlowerTileCollision",
                  "Emerging fire flower skips tile collision");
  }

  // 3. StarItem
  {
    StarItem s;
    runner.expect(!s.shouldSkipTileCollision(), "StarItemTileCollision",
                  "Initial star does not skip tile collision");
    s.startEmerge();
    runner.expect(s.shouldSkipTileCollision(), "StarItemTileCollision",
                  "Emerging star skips tile collision");
  }

  // 4. Coin
  {
    Coin c;
    runner.expect(!c.shouldSkipTileCollision(), "CoinTileCollision",
                  "Initial coin does not skip tile collision");
    c.startPop();
    runner.expect(c.shouldSkipTileCollision(), "CoinTileCollision",
                  "Popping coin skips tile collision");
  }

  // 5. OneUpMushroom (Inheritance)
  {
    OneUpMushroom oneUp;
    runner.expect(!oneUp.shouldSkipTileCollision(), "OneUpTileCollision",
                  "Initial 1UP does not skip tile collision");
    oneUp.startEmerge();
    runner.expect(oneUp.shouldSkipTileCollision(), "OneUpTileCollision",
                  "Emerging 1UP skips tile collision");
  }
}

void testEnemyCanBeStomped(TestRunner &runner) {
  Goomba g;
  runner.expect(g.canBeStomped(), "EnemyCanBeStomped", "Goomba can be stomped");

  Koopa k;
  runner.expect(k.canBeStomped(), "EnemyCanBeStomped", "Koopa can be stomped");

  PiranhaPlant p;
  runner.expect(!p.canBeStomped(), "EnemyCanBeStomped",
                "PiranhaPlant cannot be stomped");

  GreenParatroopa gp;
  runner.expect(gp.canBeStomped(), "EnemyCanBeStomped",
                "GreenParatroopa can be stomped");
}

void testEntityOnLanded(TestRunner &runner) {
  // StarItem
  {
    StarItem s;
    s.onLanded();
    runner.expect(std::abs(s.getVelocity().y - (-220.f)) < 0.01f,
                  "StarItemOnLanded",
                  "StarItem bounces on landing when not emerging");

    StarItem s2;
    s2.startEmerge();
    s2.onLanded();
    runner.expect(std::abs(s2.getVelocity().y - 0.f) < 0.01f,
                  "StarItemOnLanded",
                  "StarItem does not bounce on landing when emerging");
  }

  // Goomba safe no-op
  {
    Goomba g;
    g.onLanded();
    runner.expect(std::abs(g.getVelocity().y - 0.f) < 0.01f, "GoombaOnLanded",
                  "Goomba landing hook is safe no-op");
  }

  // GreenParatroopa
  {
    GreenParatroopa gp;
    gp.onLanded();
    runner.expect(std::abs(gp.getVelocity().y - 0.f) < 0.01f,
                  "ParatroopaOnLanded",
                  "Paratroopa does not hop immediately before update");

    gp.update(0.016f);
    runner.expect(gp.getVelocity().y < 0.f, "ParatroopaOnLanded",
                  "Paratroopa hops upward on next update after landing");
    const float expectedVel = -220.f + 980.f * 0.016f;
    runner.expect(std::abs(gp.getVelocity().y - expectedVel) < 1.0f,
                  "ParatroopaOnLanded",
                  "Paratroopa hop velocity matches physics formula");

    // Lost wings
    GreenParatroopa gp2;
    gp2.onStomped();
    gp2.onLanded();
    gp2.update(0.016f);
    runner.expect(gp2.getVelocity().y >= 0.f, "ParatroopaOnLanded",
                  "Wingless Paratroopa does not hop on landing");
  }
}

void testItemReverseDirectionPolymorphism(TestRunner &runner) {
  // Mushroom via Item*
  {
    Mushroom m;
    Item *itemM = &m;
    const int initialDir = m.getMoveDirection();
    itemM->reverseDirection();
    runner.expect(m.getMoveDirection() == -initialDir, "ItemReverseDirection",
                  "Mushroom reverses direction via Item*");
  }

  // StarItem via Item*
  {
    StarItem s;
    Item *itemS = &s;
    const int initialDir = s.getMoveDirection();
    itemS->reverseDirection();
    runner.expect(s.getMoveDirection() == -initialDir, "ItemReverseDirection",
                  "StarItem reverses direction via Item*");
  }

  // Safe no-op on static items
  {
    Coin c;
    Item *itemC = &c;
    itemC->reverseDirection();
    runner.expect(true, "ItemReverseDirection",
                  "Coin reverseDirection is safe no-op");

    FireFlower f;
    Item *itemF = &f;
    itemF->reverseDirection();
    runner.expect(true, "ItemReverseDirection",
                  "FireFlower reverseDirection is safe no-op");
  }
}

void testPiranhaPlantStompContract(TestRunner &runner) {
  // PiranhaPlant contract vs Goomba
  {
    PiranhaPlant p;
    runner.expect(!p.canBeStomped(), "PiranhaStompContract",
                  "PiranhaPlant declares canBeStomped false");
    p.onStomped();
    runner.expect(p.isEnemyAlive(), "PiranhaStompContract",
                  "PiranhaPlant is still alive after onStomped");
    runner.expect(!p.isSquished(), "PiranhaStompContract",
                  "PiranhaPlant is not squished after onStomped");
  }

  {
    Goomba g;
    runner.expect(g.canBeStomped(), "PiranhaStompContract",
                  "Goomba declares canBeStomped true");
    g.onStomped();
    runner.expect(g.isSquished(), "PiranhaStompContract",
                  "Goomba is squished after onStomped");
    runner.expect(g.getSpeed() == 0.f, "PiranhaStompContract",
                  "Goomba speed becomes 0 on stomp");
  }
}

void testSecondaryPlayerPalette(TestRunner &runner) {
  sf::Image source;
  source.create(4, 217, sf::Color::Transparent);

  source.setPixel(0, 9, sf::Color{216, 40, 0});
  source.setPixel(1, 9, sf::Color{136, 112, 0});
  source.setPixel(0, 73, sf::Color{0, 148, 0});
  source.setPixel(1, 73, sf::Color{252, 252, 252});
  source.setPixel(0, 153, sf::Color{216, 40, 0});
  source.setPixel(1, 153, sf::Color{252, 152, 56});
  source.setPixel(2, 153, sf::Color{252, 216, 168});
  source.setPixel(3, 73, sf::Color{147, 187, 236});

  const sf::Image secondary = makeSecondaryPlayerPalette(source);
  runner.expect(secondary.getPixel(0, 9) == sf::Color{252, 112, 16} &&
                    secondary.getPixel(1, 9) == sf::Color{36, 84, 204},
                "SecondaryPalette",
                "Mario normal clothing changes to orange and blue");
  runner.expect(secondary.getPixel(0, 73) == sf::Color{252, 112, 16} &&
                    secondary.getPixel(1, 73) == sf::Color{36, 84, 204},
                "SecondaryPalette",
                "Luigi normal clothing changes to orange and blue");
  runner.expect(secondary.getPixel(0, 153) == sf::Color{0, 128, 136} &&
                    secondary.getPixel(1, 153) == sf::Color{252, 252, 252},
                "SecondaryPalette", "Fire clothing changes to cyan and white");
  runner.expect(secondary.getPixel(2, 153) == sf::Color{252, 216, 168} &&
                    secondary.getPixel(3, 73) == sf::Color{147, 187, 236},
                "SecondaryPalette",
                "Skin and Luigi cleanup background colors remain unchanged");
}

void testRespawnInvincibilityEffect(TestRunner &runner) {
  Mario mario(0.f, 0.f);
  mario.update(0.f);

  // Mario dies and respawns
  mario.die(DeathCause::NormalDamage);
  mario.respawn(100.f, 200.f);

  runner.expect(mario.getPosition().x == 100.f && mario.getPosition().y == 200.f,
                "RespawnInvincibility", "Mario respawns at specified coordinates");
  runner.expect(mario.isActive() && !mario.isDying(),
                "RespawnInvincibility", "Mario is active and not dying after respawn");

  // Apply respawn invincibility (2.0s duration, 0.1s flash interval)
  mario.addEffect(std::make_unique<DamageInvincibilityEffect>(2.f));

  // Verify damage absorption during invincibility
  mario.takeDamage();
  runner.expect(!mario.isDying() && mario.isActive(),
                "RespawnInvincibility", "Mario absorbs damage during 2s invincibility");

  // Advance time past 2.0s duration
  mario.update(2.1f);

  // Invincibility has expired, damage should now take effect
  mario.takeDamage();
  runner.expect(mario.isDying(),
                "RespawnInvincibility", "Mario takes lethal damage after invincibility expires");
}

void testPlaneItemAndFlightMechanics(TestRunner &runner) {
  Mario mario(0.f, 0.f);
  mario.update(0.f);

  // Initial form is Small
  runner.expect(mario.getCurrentFormName() == "Small",
                "PlaneMechanics", "Mario starts in Small state");

  // Collect PlaneItem
  PlaneItem planeItem(0.f, 0.f);
  bool collected = planeItem.tryCollect(mario);
  runner.expect(collected, "PlaneMechanics", "PlaneItem collected successfully");
  runner.expect(mario.getCurrentFormName() == "Plane",
                "PlaneMechanics", "Mario transitioned to Plane state");
  runner.expect(mario.hasAbility(PlayerAbility::Fly),
                "PlaneMechanics", "Mario has Fly ability");
  runner.expect(mario.hasAbility(PlayerAbility::ShootFireballs),
                "PlaneMechanics", "Mario has shooting ability in Plane state");

  // Verify Projectile Request is YellowLaser
  ProjectileType requestedType = ProjectileType::Fireball;
  bool requestReceived = false;
  mario.setProjectileRequestHandler([&](const ProjectileRequest& req) {
    requestedType = req.type;
    requestReceived = true;
  });
  mario.useSpecialAbility();
  runner.expect(requestReceived && requestedType == ProjectileType::YellowLaser,
                "PlaneMechanics", "Plane state shoots YellowLaser projectile");

  // Test flight upward
  mario.setJumpHeld(true);
  mario.update(0.016f);
  runner.expect(mario.getVelocity().y < 0.f,
                "PlaneMechanics", "Mario ascends when jump/up is held in Plane state");

  // Test flight downward
  mario.setJumpHeld(false);
  mario.setCrouchRequested(true);
  mario.update(0.016f);
  runner.expect(mario.getVelocity().y > 0.f,
                "PlaneMechanics", "Mario descends when crouch/down is held in Plane state");

  // Test damage absorption: taking damage loses the plane and reverts to Small Mario (surviving!)
  mario.takeDamage();
  runner.expect(mario.getCurrentFormName() == "Small",
                "PlaneMechanics", "Taking damage loses the plane and returns to Small state");
  runner.expect(mario.isActive() && !mario.isDying(),
                "PlaneMechanics", "Mario survives the hit after losing plane");
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
  testAlternatingPiranhaPlantTiming(runner);
  testEnemyDirectionAndProperties(runner);
  runner.report("Suite 3: Enemy State Transitions");

  // Suite 4: Item Collection & State
  testCoinCollectionAndPop(runner);
  testMushroomEmergingAndCollect(runner);
  testFireFlowerCollectWithPrecondition(runner);
  testPoweredCharacterCrouch(runner);
  testCharacterReleaseTiming(runner);
  testCrawlUsesFinalGroundContact(runner);
  testRulesetMovementScale(runner);
  testMarioCanReachPvPSuperPlatform(runner);
  testLuigiHeldJumpBalance(runner);
  testDebugMovementTrailEvents(runner);
  testSecondaryPlayerPalette(runner);
  testStarItemBounceAndCollect(runner);
  testOneUpMushroomCollect(runner);
  testRespawnInvincibilityEffect(runner);
  testPlaneItemAndFlightMechanics(runner);
  runner.report("Suite 4: Item Collection & States");

  // Suite 5: OCP Polymorphism (Lương Nhật Minh)
  testItemShouldSkipTileCollision(runner);
  testEnemyCanBeStomped(runner);
  testEntityOnLanded(runner);
  testItemReverseDirectionPolymorphism(runner);
  testPiranhaPlantStompContract(runner);
  runner.report("Suite 5: OCP Polymorphism");

  std::cout << "\n====================================================\n";
  if (runner.exitCode() == 0) {
    std::cout << " ALL SOLID-11 TESTS PASSED SUCCESSFULLY! (" << runner.passed
              << "/" << runner.total << " assertions)\n";
  } else {
    std::cout << " SOME TESTS FAILED! (" << runner.failed << "/" << runner.total
              << " failures)\n";
  }
  std::cout << "====================================================\n";

  return runner.exitCode();
}
