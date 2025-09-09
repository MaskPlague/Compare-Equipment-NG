#pragma once

namespace CEActorUtils
{
    extern RE::Actor *currentActor;
    extern int index;
    extern std::vector<RE::Actor *> followers;

    void GetActiveFollowers();

    void SetActorToPlayer();

    bool IsActorValid(RE::Actor *actor);

    void SetActorToNextFollower();
}