namespace logger = SKSE::log;

namespace CEGameMenuUtils
{
    RE::FormID currentFormID;

    const char *GetArmorTypeString(RE::BGSBipedObjectForm::ArmorType type)
    {
        switch (type)
        {
        case RE::BGSBipedObjectForm::ArmorType::kClothing:
            return "Cloth";
        case RE::BGSBipedObjectForm::ArmorType::kHeavyArmor:
            return "Heavy";
        case RE::BGSBipedObjectForm::ArmorType::kLightArmor:
            return "Light";
        default:
            return "Unknown";
        }
    }

    std::string cleanPercentage(std::string str)
    {
        auto pos1 = str.find('<');
        if (pos1 != std::string::npos)
        {
            str.erase(pos1, 1);
        }
        auto pos2 = str.find('>');
        if (pos2 != std::string::npos)
        {
            str.erase(pos2, 1);
        }
        return str;
    }

    std::string GetEnchantmentString(RE::TESObjectARMO *armor, std::string description, float magnitude)
    {
        auto index = description.find("<mag>");
        if (index != std::string::npos)
            return description.substr(0, index) + std::format("{:.1f}", magnitude) + description.substr(index + 5) + "\n";
        else if (description == "")
        {
            RE::BSString str;
            armor->GetDescription(str, nullptr);
            return cleanPercentage(static_cast<std::string>(str)) + "\n";
        }
        else
            return cleanPercentage(description) + "\n";
    }

    std::string GetArmorDescription(RE::TESObjectARMO *armor, std::string text)
    {
        RE::BSString str;
        armor->GetDescription(str, nullptr);
        std::string description = static_cast<std::string>(str) + "\n";
        if (description.size() > 3)
            text = cleanPercentage(description);
        if (text == "")
            text = "None";
        return text;
    }

    std::string GetArmorSlotsString(RE::TESObjectARMO *armor, std::string slotsString)
    {
        auto biped = armor->bipedModelData.bipedObjectSlots.get();
        uint32_t bipedUint = static_cast<uint32_t>(biped);
        for (uint32_t slot = 30; slot <= 61; ++slot)
        {
            uint32_t mask = 1u << (slot - 30);
            if ((bipedUint & mask) != 0)
            {
                if (slotsString == "")
                    slotsString += std::to_string(slot);
                else
                    slotsString += ", " + std::to_string(slot);
            }
        }
        return slotsString;
    }

    void GetEquippedInSlots(RE::FormID selectedFormId)
    {
        if (auto form = RE::TESForm::LookupByID(selectedFormId))
        {
            if (auto armor = form->As<RE::TESObjectARMO>())
            {
                auto biped = armor->bipedModelData;
                auto slots = biped.bipedObjectSlots;
                auto actor = CEActorUtils::currentActor;
                if (!actor)
                {
                    CEActorUtils::SetActorToNextFollower();
                    return;
                }
                auto player = RE::PlayerCharacter::GetSingleton();
                if (!actor)
                    return;

                using Slot = RE::BGSBipedObjectForm::BipedObjectSlot;

                std::vector<Slot> slotList = {
                    Slot::kAmulet,
                    Slot::kBody,
                    Slot::kCalves,
                    Slot::kCirclet,
                    Slot::kDecapitate,
                    Slot::kDecapitateHead,
                    Slot::kEars,
                    Slot::kFeet,
                    Slot::kForearms,
                    Slot::kFX01,
                    Slot::kHair,
                    Slot::kHands,
                    Slot::kHead,
                    Slot::kLongHair,
                    Slot::kModArmLeft,
                    Slot::kModArmRight,
                    Slot::kModBack,
                    Slot::kModChestPrimary,
                    Slot::kModChestSecondary,
                    Slot::kModFaceJewelry,
                    Slot::kModLegLeft,
                    Slot::kModLegRight,
                    Slot::kModMisc1,
                    Slot::kModMisc2,
                    Slot::kModMouth,
                    Slot::kModNeck,
                    Slot::kModPelvisPrimary,
                    Slot::kModPelvisSecondary,
                    Slot::kModShoulder,
                    Slot::kNone,
                    Slot::kRing,
                    Slot::kShield,
                    Slot::kTail,
                };
                const auto &inv = actor->GetInventory();
                auto selectedName = armor->GetName();
                std::string selectedSlots = "";
                selectedSlots += GetArmorSlotsString(armor, selectedSlots);
                const char *selectedType = GetArmorTypeString(armor->GetArmorType());
                auto selectedValue = armor->GetGoldValue();
                auto selectedEnchantment = armor->formEnchanting;
                std::string selectedEnchantmentInfo = "";
                if (selectedEnchantment)
                {
                    auto effects = selectedEnchantment->effects;
                    for (auto effect : effects)
                    {
                        float magnitude = effect->GetMagnitude();
                        std::string description = static_cast<std::string>(effect->baseEffect->magicItemDescription);
                        selectedEnchantmentInfo += GetEnchantmentString(armor, description, magnitude);
                    }
                }
                else
                    selectedEnchantmentInfo = GetArmorDescription(armor, selectedEnchantmentInfo);

                int32_t equippedAccumulateValue = 0;
                float selectedRating = 0.0f;
                RE::InventoryEntryData *selectedEntryData;
                for (auto &[item, data] : inv)
                {
                    if (item && item->GetFormID() == selectedFormId)
                    {
                        selectedEntryData = data.second.get();
                        selectedRating = player->GetArmorValue(selectedEntryData);
                        break;
                    }
                }
                float equippedAccumulatedRating = 0.0f;

                std::vector<std::array<RE::GFxValue, CEGlobals::EQUIPPED_ITEM_ARRAY_SIZE>> item_arr;
                std::vector<RE::FormID> pushedFormIds;
                const char *equippedName;
                std::string equippedSlots;
                std::string equippedEnchantmentInfo;
                const char *equippedType;
                int32_t equippedValue;
                for (auto slot : slotList)
                {
                    if ((slots & slot) != Slot::kNone)
                    {
                        auto equipped = actor->GetWornArmor(slot);
                        if (equipped)
                        {
                            auto formId = equipped->GetFormID();
                            bool alreadyPushed = std::find(pushedFormIds.begin(), pushedFormIds.end(), formId) != pushedFormIds.end();
                            if (selectedFormId != formId && !alreadyPushed)
                            {
                                pushedFormIds.push_back(formId);
                                equippedName = equipped->GetName();
                                equippedSlots = "";
                                equippedSlots += GetArmorSlotsString(equipped, equippedSlots);
                                equippedType = GetArmorTypeString(equipped->GetArmorType());
                                equippedValue = equipped->GetGoldValue();
                                auto equippedEnchantment = equipped->formEnchanting;
                                equippedEnchantmentInfo = "";
                                if (equippedEnchantment)
                                {
                                    auto effects = equippedEnchantment->effects;
                                    for (auto effect : effects)
                                    {
                                        auto magnitude = effect->GetMagnitude();
                                        std::string description = static_cast<std::string>(effect->baseEffect->magicItemDescription);
                                        equippedEnchantmentInfo += GetEnchantmentString(equipped, description, magnitude);
                                    }
                                }
                                else
                                    equippedEnchantmentInfo += GetArmorDescription(equipped, equippedEnchantmentInfo);

                                equippedAccumulateValue += equippedValue;
                                float equippedRating = 0.0f;
                                RE::InventoryEntryData *equippedEntryData;
                                for (auto &[item, data] : inv)
                                {
                                    if (item && item->GetFormID() == formId)
                                    {
                                        equippedEntryData = data.second.get();
                                        equippedRating = player->GetArmorValue(equippedEntryData);
                                        break;
                                    }
                                }
                                equippedAccumulatedRating += equippedRating;
                                std::array<RE::GFxValue, CEGlobals::EQUIPPED_ITEM_ARRAY_SIZE>
                                    itemInfo = {RE::GFxValue(equippedName), RE::GFxValue(equippedSlots.c_str()), RE::GFxValue(equippedType),
                                                RE::GFxValue(equippedRating), RE::GFxValue(equippedValue), RE::GFxValue(equippedEnchantmentInfo.c_str())};
                                item_arr.push_back(itemInfo);
                            }
                        }
                    }
                }
                if (item_arr.size() > 0)
                {
                    equippedAccumulatedRating = selectedRating - equippedAccumulatedRating;
                    equippedAccumulateValue = selectedValue - equippedAccumulateValue;
                }
                std::array<RE::GFxValue, CEGlobals::SELECTED_ITEM_ARRAY_SIZE>
                    selectedItemInfo = {RE::GFxValue(selectedName), RE::GFxValue(selectedSlots.c_str()), RE::GFxValue(selectedType),
                                        RE::GFxValue(selectedRating), RE::GFxValue(equippedAccumulatedRating),
                                        RE::GFxValue(selectedValue), RE::GFxValue(equippedAccumulateValue),
                                        RE::GFxValue(selectedEnchantmentInfo.c_str())};
                CEMenu::ResetMenu();
                CEMenu::SetActor(actor->GetName());
                CEMenu::CreateSelectedItemCard(selectedItemInfo);
                CEMenu::CreateComparisonItemCards(item_arr);
                CEMenu::ShowMenuInstant();
            }
        }
    }

    bool GetItem()
    {
        RE::GFxValue Menu_mc = CEMenu::GetMenu_mc();
        if (Menu_mc.IsNull())
            return false;
        RE::GFxValue inventoryLists;
        RE::GFxValue itemList;
        RE::GFxValue selectedEntry;
        RE::GFxValue formId;
        if (!Menu_mc.GetMember("inventoryLists", &inventoryLists) ||
            !inventoryLists.GetMember("itemList", &itemList) ||
            !itemList.GetMember("selectedEntry", &selectedEntry) ||
            !selectedEntry.IsObject() ||
            !selectedEntry.GetMember("formId", &formId))
            return false;
        RE::FormID fid = static_cast<RE::FormID>(formId.GetUInt());
        currentFormID = fid;
        GetEquippedInSlots(fid);
        return true;
    }

    void ActorChangedUpdateMenu()
    {
        if (!currentFormID)
        {
            return;
        }
        GetEquippedInSlots(currentFormID);
    }
}