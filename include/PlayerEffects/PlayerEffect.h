#pragma once

class Character;

class PlayerEffect {
public:
    virtual ~PlayerEffect() = default;

    virtual void onApply(Character& character);
    virtual void onRemove(Character& character);
    virtual void update(Character& character, float dt) = 0;
    virtual bool hasExpired() const = 0;

    virtual float getMoveSpeedMultiplier() const;
    virtual float getJumpForceMultiplier() const;
    virtual bool tryAbsorbDamage(Character& character);
    virtual int getDamagePriority() const;
    virtual bool defeatsEnemiesOnContact() const;
    virtual bool isCharacterVisible() const;
};
