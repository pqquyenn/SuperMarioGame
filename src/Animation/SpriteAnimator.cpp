#include "Animation/SpriteAnimator.h"

void SpriteAnimator::play(const AnimationClip& clip, bool restart) {
    if (!clip.isValid()) {
        stop();
        return;
    }

    if (currentClip == &clip && !restart) {
        return;
    }

    currentClip = &clip;
    currentFrame = 0;
    elapsedTime = 0.f;
    finished = false;
}

void SpriteAnimator::stop() {
    currentClip = nullptr;
    currentFrame = 0;
    elapsedTime = 0.f;
    finished = false;
}

void SpriteAnimator::update(float dt) {
    if (!currentClip ||
        !currentClip->isValid() ||
        finished ||
        dt <= 0.f) {
        return;
    }

    elapsedTime += dt;

    while (elapsedTime >= currentClip->frameDuration && !finished) {
        elapsedTime -= currentClip->frameDuration;

        if (currentFrame + 1 < currentClip->frames.size()) {
            ++currentFrame;
        } else if (currentClip->looping) {
            currentFrame = 0;
        } else {
            currentFrame = currentClip->frames.size() - 1;
            elapsedTime = 0.f;
            finished = true;
        }
    }
}

const AnimationFrame* SpriteAnimator::getCurrentFrame() const {
    if (!currentClip ||
        !currentClip->isValid() ||
        currentFrame >= currentClip->frames.size()) {
        return nullptr;
    }

    return &currentClip->frames[currentFrame];
}

bool SpriteAnimator::isPlaying(const AnimationClip& clip) const {
    return currentClip == &clip;
}

bool SpriteAnimator::isFinished() const {
    return finished;
}
