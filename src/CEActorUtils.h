#pragma once

namespace CEActorUtils
{
    extern RE::Actor *currentActor;
    extern int index;
    extern std::vector<RE::Actor *> followers;

    void GetActiveFollowers();

    void SetActorToPlayer();

    void SetActorToNextFollower();
}