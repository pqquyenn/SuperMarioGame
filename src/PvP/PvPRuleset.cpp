#include "PvP/PvPRuleset.h"

PvPRuleset PvPRuleset::forMatch(PvPMatchType type) {
    switch (type) {
        case PvPMatchType::Small:
            return {3, true, false, false, false, false, 0.f};
        case PvPMatchType::Super:
            return {3, true, true, true, false, false, 0.f};
        case PvPMatchType::Friendly:
            return {3, false, true, true, true, true, 15.f};
    }
    return {};
}
