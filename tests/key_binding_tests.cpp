#include "Input/KeyBinding.h"
#include "Input/KeyBindingService.h"

#include <cassert>
#include <filesystem>
#include <iostream>

namespace {
void removeTestFiles(const std::filesystem::path& path) {
    std::error_code error;
    std::filesystem::remove(path, error);
    std::filesystem::remove(path.string() + ".tmp", error);
}
}

int main() {
    const BindingProfiles defaults = makeDefaultBindingProfiles();
    assert(validateBindingProfiles(defaults).valid());
    assert(defaults[static_cast<std::size_t>(BindingTarget::Solo)]
               .get(InputAction::Jump) == sf::Keyboard::Space);
    assert(defaults[static_cast<std::size_t>(BindingTarget::PvPPlayerTwo)]
               .get(InputAction::Action) == sf::Keyboard::J);

    const std::filesystem::path configPath =
        std::filesystem::temp_directory_path() /
        "solid03_key_binding_tests.cfg";
    removeTestFiles(configPath);

    KeyBindingService service{configPath};
    assert(service.getKey(BindingTarget::Solo, InputAction::Jump) ==
           sf::Keyboard::Space);

    BindingUpdateResult result = service.tryUpdate(
        {BindingTarget::Solo, InputAction::Jump, sf::Keyboard::X});
    assert(result.status == BindingUpdateStatus::Applied);
    assert(service.getKey(BindingTarget::Solo, InputAction::Jump) ==
           sf::Keyboard::X);

    KeyBindingService reloaded{configPath};
    assert(reloaded.getKey(BindingTarget::Solo, InputAction::Jump) ==
           sf::Keyboard::X);

    result = reloaded.tryUpdate(
        {BindingTarget::Solo, InputAction::Jump, sf::Keyboard::Left});
    assert(result.status == BindingUpdateStatus::Rejected);
    assert(!result.issues.empty());
    assert(result.issues.front().code == BindingIssueCode::ActionConflict);

    result = reloaded.tryUpdate(
        {BindingTarget::Solo, InputAction::Jump, sf::Keyboard::Escape});
    assert(result.status == BindingUpdateStatus::Rejected);
    assert(!result.issues.empty());
    assert(result.issues.front().code == BindingIssueCode::ReservedKey);

    result = reloaded.tryUpdate(
        {BindingTarget::DuoPlayerTwo, InputAction::Jump, sf::Keyboard::A});
    assert(result.status == BindingUpdateStatus::Rejected);
    assert(!result.issues.empty());
    assert(result.issues.front().code ==
           BindingIssueCode::CrossPlayerConflict);

    result = reloaded.resetProfile(BindingTarget::Solo);
    assert(result.status == BindingUpdateStatus::Applied);
    assert(reloaded.getKey(BindingTarget::Solo, InputAction::Jump) ==
           sf::Keyboard::Space);

    assert(keyDisplayName(sf::Keyboard::LShift) == "LEFT SHIFT");
    assert(keyDisplayName(sf::Keyboard::Numpad1) == "NUMPAD 1");

    result = reloaded.resetAll();
    assert(result.status == BindingUpdateStatus::Applied);
    assert(validateBindingProfiles(makeDefaultBindingProfiles()).valid());

    removeTestFiles(configPath);
    std::cout << "Key binding tests passed\n";
    return 0;
}
