#pragma once

#include <string_view>

enum class PlayerId {
    One,
    Two
};

enum class PvPMatchType {
    Small,
    Super,
    Friendly
};

enum class PvPDamageSource {
    Stomp,
    Fireball,
    Enemy,
    Void
};

inline std::string_view pvpMatchName(PvPMatchType type) {
    switch (type) {
        case PvPMatchType::Small: return "SMALL MATCH";
        case PvPMatchType::Super: return "SUPER MATCH";
        case PvPMatchType::Friendly: return "FRIENDLY MATCH";
    }
    return "PVP MATCH";
}
