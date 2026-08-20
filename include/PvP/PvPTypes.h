#pragma once

#include <string_view>

enum class PlayerId {
    One,
    Two
};

enum class PvPMatchType {
    Small,
    Super
};

enum class PvPDamageSource {
    Stomp,
    Fireball,
    Enemy,
    Void
};

inline std::string_view pvpMatchName(PvPMatchType type) {
    return type == PvPMatchType::Small ? "SMALL MATCH" : "SUPER MATCH";
}
