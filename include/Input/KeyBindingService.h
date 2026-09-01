#pragma once

#include "Input/KeyBinding.h"

#include <filesystem>

class KeyBindingService final
    : public IKeyBindingProvider,
      public IKeyBindingEditor {
public:
    static KeyBindingService& getInstance();

    explicit KeyBindingService(
        std::filesystem::path configPath =
            "assets/state/keybindings.cfg");

    sf::Keyboard::Key getKey(
        BindingTarget target,
        InputAction action) const override;
    const BindingProfile& getProfile(
        BindingTarget target) const override;

    BindingUpdateResult tryUpdate(
        const BindingChange& change) override;
    BindingUpdateResult resetProfile(
        BindingTarget target) override;
    BindingUpdateResult resetAll() override;

    bool reload();
    const std::filesystem::path& getConfigPath() const {
        return configPath;
    }

private:
    BindingProfiles profiles;
    std::filesystem::path configPath;

    bool load();
    bool save(const BindingProfiles& candidate) const;
};
