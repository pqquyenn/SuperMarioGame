#pragma once

#include <memory>
#include <string_view>

class Character;

enum class PlayerAbility {
    BreakBricks,
    ShootFireballs,
    ShootIceballs,
    Fly,
    TailAttack
};

enum class FormTier {
    Small,
    Powered
};

class PlayerState {
public:
    virtual ~PlayerState() = default;

    // Hooks for applying and removing state-specific visual or gameplay effects.
    virtual void onEnter(Character& character) const = 0;
    virtual void onExit(Character& character) const;

    // Describes the abilities supplied by the current form.
    virtual std::string_view getName() const = 0;
    virtual FormTier getFormTier() const = 0;
    virtual float getHeightMultiplier() const = 0;
    virtual bool hasAbility(PlayerAbility ability) const = 0;
    virtual void useSpecialAbility(Character& character) const;

    // Returns the downgraded state after damage.
    // A null result means that the player dies in this state.
    virtual std::unique_ptr<PlayerState> takeDamage() const = 0;
};
