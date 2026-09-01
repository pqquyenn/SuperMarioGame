#pragma once

#include "Animation/AnimationClip.h"
#include <cstddef>

// Advances frames from a shared AnimationClip. It does not own textures,
// sprites, or gameplay state, so any animated Entity can reuse it.
class SpriteAnimator {
private:
    const AnimationClip* currentClip{nullptr};
    std::size_t currentFrame{0};
    float elapsedTime{0.f};
    bool finished{false};

public:
    void play(const AnimationClip& clip, bool restart = false);
    void stop();
    void update(float dt);

    const AnimationFrame* getCurrentFrame() const;
    bool isPlaying(const AnimationClip& clip) const;
    bool isFinished() const;
};
