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

    std::string GetArmorDescription(RE::TESObjectARMO *armor)
    {
        std::string text = "None";
        RE::BSString str;
        armor->GetDescription(str, nullptr);
        std::string description = static_cast<std::string>(str) + "\n";
        if (description.size() > 3)
            text = cleanPercentage(description);
        return text;
    }

    std::string GetArmorSlotsString(RE::TESObjectARMO *armor)
    {
        std::string slotsString = "None";
        auto biped = armor->bipedModelData.bipedObjectSlots.get();
        uint32_t bipedUint = static_cast<uint32_t>(biped);
        for (uint32_t slot = 30; slot <= 61; ++slot)
        {
            uint32_t mask = 1u << (slot - 30);
            if ((bipedUint & mask) != 0)
            {
                if (slotsString == "None")
                    slotsString = std::to_string(slot);
                else
                    slotsString += ", " + std::to_string(slot);
            }
        }
        return slotsString;
    }

    void GetEquippedInSlots(RE::FormID selectedFormId)
    {
        if (auto selectedForm = RE::TESForm::LookupByID(selectedFormId))
        {
            logger::trace("Form exists");
            if (auto selectedArmor = selectedForm->As<RE::TESObjectARMO>())
            {
                logger::trace("Form is armor, getting bipedModelData");
                RE::BIPED_MODEL biped = selectedArmor->bipedModelData;
                logger::trace("Getting bipedObjectSlots");
                SKSE::stl::enumeration<RE::BIPED_MODEL::BipedObjectSlot, uint32_t> slots = biped.bipedObjectSlots;
                logger::trace("Retrieving currentActor");
                auto actor = CEActorUtils::currentActor;
                if (!CEActorUtils::IsActorValid(actor))
                {
                    logger::trace("Current actor isn't valid");
                    CEActorUtils::SetActorToNextFollower();
                    return;
                }

                logger::trace("Resetting menu");
                CEMenu::ResetMenu();
                logger::trace("Setting actor name");
                CEMenu::SetActor(actor->GetName());
                logger::trace("Getting 3d manager");
                auto manager = RE::Inventory3DManager::GetSingleton();
                if (manager)
                    manager->Clear3D();
                logger::trace("Getting ceMenu");
                RE::GFxValue ceMenu = CEMenu::GetCEMenu(CEMenu::GetMenu_mc());

                logger::trace("Retrieving player");
                auto player = RE::PlayerCharacter::GetSingleton();
                if (!player)
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
                logger::trace("Getting actor inventory");
                const auto &inv = actor->GetInventory();
                logger::trace("Getting armor name");
                auto selectedName = selectedArmor->GetName();
                logger::trace("Getting armor slots");
                std::string selectedSlots = "";
                selectedSlots += GetArmorSlotsString(selectedArmor);
                logger::trace("Getting armor type string");
                const char *selectedType = GetArmorTypeString(selectedArmor->GetArmorType());
                logger::trace("Getting armor gold value");
                auto selectedValue = selectedArmor->GetGoldValue();
                logger::trace("Getting enchantment info");
                auto selectedEnchantment = selectedArmor->formEnchanting;
                std::string selectedEnchantmentInfo = "";
                if (selectedEnchantment)
                {
                    auto effects = selectedEnchantment->effects;
                    for (auto effect : effects)
                    {
                        float magnitude = effect->GetMagnitude();
                        std::string description = static_cast<std::string>(effect->baseEffect->magicItemDescription);
                        selectedEnchantmentInfo += GetEnchantmentString(selectedArmor, description, magnitude);
                    }
                }
                else
                    selectedEnchantmentInfo = GetArmorDescription(selectedArmor);

                logger::trace("Getting armor rating");
                int32_t equippedAccumulateValue = 0;
                int selectedRating = 0;
                RE::InventoryEntryData *selectedEntryData;
                bool gotSelectedRating = false;
                std::string selectedRatingString;
                for (auto &[item, data] : inv)
                {
                    if (item && item->GetFormID() == selectedFormId)
                    {
                        selectedEntryData = data.second.get();
                        selectedRating = static_cast<int>(player->GetArmorValue(selectedEntryData));
                        selectedRatingString = std::to_string(selectedRating);
                        gotSelectedRating = true;
                        break;
                    }
                }
                if (!gotSelectedRating)
                {
                    selectedRating = static_cast<int>(selectedArmor->armorRating / 100);
                    selectedRatingString = std::to_string(selectedRating) + " (Unscaled)";
                }

                int equippedAccumulatedRating = 0;
                std::vector<RE::FormID> pushedFormIds;
                for (auto slot : slotList)
                {
                    if ((slots & slot) != Slot::kNone)
                    {
                        auto equippedArmor = actor->GetWornArmor(slot);
                        if (equippedArmor)
                        {
                            auto formId = equippedArmor->GetFormID();
                            bool alreadyPushed = std::find(pushedFormIds.begin(), pushedFormIds.end(), formId) != pushedFormIds.end();
                            if (selectedFormId != formId && !alreadyPushed)
                            {
                                pushedFormIds.push_back(formId);
                                const char *equippedName = equippedArmor->GetName();
                                std::string equippedSlots = GetArmorSlotsString(equippedArmor);
                                std::string equippedEnchantmentInfo = "";
                                const char *equippedType = GetArmorTypeString(equippedArmor->GetArmorType());
                                int32_t equippedValue = equippedArmor->GetGoldValue();
                                auto equippedEnchantment = equippedArmor->formEnchanting;
                                if (equippedEnchantment)
                                {
                                    auto effects = equippedEnchantment->effects;
                                    for (auto effect : effects)
                                    {
                                        auto magnitude = effect->GetMagnitude();
                                        std::string description = static_cast<std::string>(effect->baseEffect->magicItemDescription);
                                        equippedEnchantmentInfo += GetEnchantmentString(equippedArmor, description, magnitude);
                                    }
                                }
                                else
                                    equippedEnchantmentInfo += GetArmorDescription(equippedArmor);

                                equippedAccumulateValue += equippedValue;
                                int equippedRating = 0;
                                RE::InventoryEntryData *equippedEntryData;
                                bool gotEquippedRating = false;
                                std::string equippedRatingString;
                                for (auto &[item, data] : inv)
                                {
                                    if (item && item->GetFormID() == formId)
                                    {
                                        equippedEntryData = data.second.get();
                                        equippedRating = static_cast<int>(player->GetArmorValue(equippedEntryData));
                                        equippedRatingString = std::to_string(equippedRating);
                                        gotEquippedRating = true;
                                        break;
                                    }
                                }
                                if (!gotEquippedRating)
                                {
                                    equippedRating = static_cast<int>(equippedArmor->armorRating / 100);
                                    equippedRatingString = std::to_string(equippedRating) + " (Unscaled)";
                                }

                                equippedAccumulatedRating += equippedRating;
                                std::array<RE::GFxValue, CEGlobals::EQUIPPED_ITEM_ARRAY_SIZE>
                                    itemInfo = {equippedName, equippedSlots.c_str(), equippedType,
                                                equippedRatingString.c_str(), equippedValue, equippedEnchantmentInfo.c_str()};
                                CEMenu::CreateComparisonItemCard(itemInfo, ceMenu);
                            }
                        }
                    }
                }
                if (pushedFormIds.size() > 0)
                {
                    equippedAccumulatedRating = selectedRating - equippedAccumulatedRating;
                    equippedAccumulateValue = selectedValue - equippedAccumulateValue;
                }
                logger::trace("Creating selectedItemInfo");
                std::array<RE::GFxValue, CEGlobals::SELECTED_ITEM_ARRAY_SIZE>
                    selectedItemInfo = {selectedName, selectedSlots.c_str(), selectedType,
                                        selectedRatingString.c_str(), equippedAccumulatedRating,
                                        selectedValue, equippedAccumulateValue,
                                        selectedEnchantmentInfo.c_str()};

                logger::trace("Populating selected item card");
                CEMenu::CreateSelectedItemCard(selectedItemInfo);
                logger::trace("Positioning and displaying item cards");
                std::array<RE::GFxValue, CEGlobals::EQUIPPED_ITEM_ARRAY_SIZE>
                    displayCommand = {"DISPLAY", CEGlobals::BACKGROUND_ALPHA, "", "", "", ""};
                CEMenu::CreateComparisonItemCard(displayCommand, ceMenu);
            }
        }
    }

    bool GetItem()
    {
        logger::trace("Getting Menu_mc");
        RE::GFxValue Menu_mc = CEMenu::GetMenu_mc();
        if (Menu_mc.IsNull())
            return false;
        RE::GFxValue inventoryLists;
        RE::GFxValue itemList;
        RE::GFxValue selectedEntry;
        RE::GFxValue formId;
        logger::trace("Getting InventoryLists > itemList > selectedEntry > formId");
        if (!Menu_mc.GetMember("inventoryLists", &inventoryLists) ||
            !inventoryLists.GetMember("itemList", &itemList) ||
            !itemList.GetMember("selectedEntry", &selectedEntry) ||
            !selectedEntry.IsObject() ||
            !selectedEntry.GetMember("formId", &formId) ||
            formId.IsNull())
            return false;
        logger::trace("Casting from RE::GFxValue to RE::FormID");
        RE::FormID fid = static_cast<RE::FormID>(formId.GetUInt());
        logger::trace("Setting currentFormID");
        currentFormID = fid;
        logger::trace("Calling GetEquippedInSlots()");
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