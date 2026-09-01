#include "Observer/Event.h"
#include "PvP/PvPCameraPolicy.h"
#include "PvP/PvPMatchRules.h"
#include "PvP/PvPPlayerSession.h"
#include "PvP/PvPRuleset.h"

#include <cmath>
#include <iostream>

namespace {
int failures = 0;

void expect(bool condition, const char* message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAILED: " << message << '\n';
    }
}
}

int main() {
    const PvPRuleset small = PvPRuleset::forMatch(PvPMatchType::Small);
    const PvPRuleset super = PvPRuleset::forMatch(PvPMatchType::Super);
    const PvPRuleset friendly = PvPRuleset::forMatch(PvPMatchType::Friendly);
    expect(!small.startsPowered && !small.fireFlowersEnabled,
           "Small rules disable powered features");
    expect(super.startsPowered && super.playersCanDamageEachOther,
           "Super rules enable powered combat");
    expect(friendly.timedMatch && !friendly.playersCanDamageEachOther &&
               friendly.refreshArena,
           "Friendly rules are timed and non-combat");

    expect(PvPMatchRules::friendlyDeathScore(50) == 0,
           "Low friendly score drops to zero");
    expect(PvPMatchRules::friendlyDeathScore(700) == 500,
           "Mid friendly score uses proportional penalty");
    expect(PvPMatchRules::friendlyDeathScore(2000) == 1750,
           "High friendly score uses fixed penalty");
    expect(PvPMatchRules::determineWinner(
               PvPMatchType::Super, 0, 1, 0, 0, 0.f) ==
               PvPWinner::PlayerTwo,
           "Remaining life determines combat winner");
    expect(PvPMatchRules::determineWinner(
               PvPMatchType::Friendly, 3, 3, 100, 200, 0.f) ==
               PvPWinner::PlayerTwo,
           "Score determines friendly winner at timeout");

    PvPPlayerSession session{PlayerId::One, 3};
    session.beginSpawnProtection(1.5f);
    session.grantFirePower(1.f);
    session.beginFireCooldown(0.5f);
    expect(session.isSpawnProtected() && !session.canFire(),
           "Session owns protection and cooldown state");
    session.update(0.5f);
    expect(session.canFire(), "Fire becomes available after cooldown");
    session.update(0.6f);
    expect(session.consumeExpiredFirePower(),
           "Temporary fire power reports expiration");
    session.onNotify(GameEvent::coinCollected(200));
    expect(session.coins() == 1 && session.score() == 200,
           "Session observes player score events");
    session.loseLife();
    expect(session.lives() == 2, "Session owns life loss");

    const PvPCameraLayout camera = PvPCameraPolicy::layout(
        {400.f, 225.f}, {480.f, 256.f}, {800u, 600u});
    expect(camera.viewport.height < 1.f && camera.viewport.top > 0.f,
           "Wide PvP world is letterboxed in a 4:3 window");
    const float displayedAspect =
        (camera.viewport.width * 800.f) /
        (camera.viewport.height * 600.f);
    expect(std::abs(displayedAspect - camera.viewSize.x / camera.viewSize.y) <
               0.001f,
           "Viewport preserves world aspect ratio");

    if (failures == 0) {
        std::cout << "PvP rules tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
