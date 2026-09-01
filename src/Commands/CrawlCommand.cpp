#include "Commands/CrawlCommand.h"
#include "Entities/Character.h"

void CrawlCommand::execute(Character& character, float) const {
    character.setCrouchRequested(true);
}

void CrawlCommand::release(Character& character) const {
    character.setCrouchRequested(false);
}
