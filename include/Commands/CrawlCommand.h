#pragma once

#include "Commands/Command.h"

class CrawlCommand final : public Command {
public:
    void execute(Character& character, float dt) const override;
    void release(Character& character) const;
};
