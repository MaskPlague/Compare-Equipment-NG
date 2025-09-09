namespace logger = SKSE::log;

namespace CEActorUtils
{
    RE::Actor *currentActor;
    int index = 0;
    std::vector<RE::Actor *> followers;

    void GetActiveFollowers(void)
    {
        std::vector<RE::Actor *> result;

        auto processLists = RE::ProcessLists::GetSingleton();
        if (!processLists)
        {
            followers = result;
            return;
        }
        logger::debug("Got process lists");
        auto faction = RE::TESForm::LookupByID<RE::TESFaction>(0x0005C84E); // CurrentFollowerFaction
        if (!faction)
        {
            followers = result;
            return;
        }
        logger::debug("Got faction");
        for (auto &handle : processLists->highActorHandles)
        {
            if (auto actor = handle.get().get())
            {
                if (actor->IsInFaction(faction))
                {
                    result.push_back(actor);
                }
            }
        }
        followers = result;
        return;
    }

    void SetActorToPlayer()
    {
        logger::trace("CEActorUtils: Setting currentActor to player");
        currentActor = RE::PlayerCharacter::GetSingleton();
    }

    bool IsActorValid(RE::Actor *actor)
    {
        if (!actor->Is3DLoaded() || actor->IsDead() || actor->IsDeleted() || actor->IsDisabled())
        {
            return false;
        }
        return true;
    }

    void SetActorToNextFollower()
    {
        if (followers.size() == 0)
        {
            logger::trace("CEActorUtils: No valid followers detected calling SetActorToPlayer()");
            CEActorUtils::SetActorToPlayer();
            return;
        }
        if (index >= followers.size())
            index = 0;
        RE::Actor *potentialActor = followers.at(index);
        if (!IsActorValid(potentialActor))
        {
            logger::trace("CEActorUtils: Follower invalid, getting next follower.");
            followers.erase(followers.begin() + index);
            SetActorToNextFollower();
            return;
        }
        currentActor = potentialActor;
        index++;
    }
}