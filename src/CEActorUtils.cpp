namespace CEActorUtils
{
    RE::Actor *currentActor;
    int index = 0;
    std::vector<RE::Actor *> followers;

    void GetActiveFollowers(void)
    {
        std::vector<RE::Actor *> result;

        auto processLists = RE::ProcessLists::GetSingleton();
        auto faction = RE::TESForm::LookupByID<RE::TESFaction>(0x0005C84E); // CurrentFollowerFaction
        if (!processLists || !faction)
        {
            followers = result;
            return;
        }
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
            logger::trace("No valid followers detected calling SetActorToPlayer()");
            CEActorUtils::SetActorToPlayer();
            return;
        }
        if (index >= followers.size())
            index = 0;
        RE::Actor *potentialActor = followers.at(index);
        if (!IsActorValid(potentialActor))
        {
            logger::trace("Follower invalid, getting next follower.");
            followers.erase(followers.begin() + index);
            SetActorToNextFollower();
            return;
        }
        currentActor = potentialActor;
        index++;
    }
}