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
        return text;
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

                std::vector<std::pair<Slot, const char *>> slotList = {
                    {Slot::kAmulet, "Amulet"},
                    {Slot::kBody, "Body"},
                    {Slot::kCalves, "Calves"},
                    {Slot::kCirclet, "Circlet"},
                    {Slot::kDecapitate, "Decapitate"},
                    {Slot::kDecapitateHead, "DecapitateHead"},
                    {Slot::kEars, "Ears"},
                    {Slot::kFeet, "Feet"},
                    {Slot::kForearms, "Forearms"},
                    {Slot::kFX01, "FX01"},
                    {Slot::kHair, "Hair"},
                    {Slot::kHands, "Hands"},
                    {Slot::kHead, "Head"},
                    {Slot::kLongHair, "LongHair"},
                    {Slot::kModArmLeft, "ModArmLeft"},
                    {Slot::kModArmRight, "ModArmRight"},
                    {Slot::kModBack, "ModBack"},
                    {Slot::kModChestPrimary, "ModChestPrimary"},
                    {Slot::kModChestSecondary, "ModChestSecondary"},
                    {Slot::kModFaceJewelry, "FaceJewelry"},
                    {Slot::kModLegLeft, "ModLegLeft"},
                    {Slot::kModLegRight, "ModLefRight"},
                    {Slot::kModMisc1, "ModMisc1"},
                    {Slot::kModMisc2, "ModMisc2"},
                    {Slot::kModMouth, "ModMouth"},
                    {Slot::kModNeck, "ModNeck"},
                    {Slot::kModPelvisPrimary, "ModPelvisPrimary"},
                    {Slot::kModPelvisSecondary, "ModPelvisSecondary"},
                    {Slot::kModShoulder, "ModShoulder"},
                    {Slot::kNone, "None"},
                    {Slot::kRing, "Ring"},
                    {Slot::kShield, "Shield"},
                    {Slot::kTail, "Tail"},
                };
                const auto &inv = actor->GetInventory();
                auto selectedName = armor->GetName();
                const char *selectedType = GetArmorTypeString(armor->GetArmorType());
                auto selectedValue = armor->GetGoldValue();
                auto selectedEnchantment = armor->formEnchanting;
                std::string selectedEnchantmentInfo = "None";
                if (selectedEnchantment)
                {
                    selectedEnchantmentInfo = "";
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

                for (auto &[slot, name] : slotList)
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
                                auto equippedName = equipped->GetName();
                                auto equippedType = GetArmorTypeString(equipped->GetArmorType());
                                auto equippedValue = equipped->GetGoldValue();
                                auto equippedEnchantment = equipped->formEnchanting;
                                std::string equippedEnchantmentInfo = "None";
                                if (equippedEnchantment)
                                {
                                    equippedEnchantmentInfo = "";
                                    auto effects = equippedEnchantment->effects;
                                    for (auto effect : effects)
                                    {
                                        auto magnitude = effect->GetMagnitude();
                                        std::string description = static_cast<std::string>(effect->baseEffect->magicItemDescription);
                                        selectedEnchantmentInfo += GetEnchantmentString(equipped, description, magnitude);
                                    }
                                }
                                else
                                    selectedEnchantmentInfo = GetArmorDescription(armor, selectedEnchantmentInfo);

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
                                    itemInfo = {equippedName, "SlotsTBD", equippedType,
                                                equippedRating, equippedValue, equippedEnchantmentInfo.c_str()};
                                item_arr.push_back(itemInfo);
                            }
                        }
                    }
                }
                if (equippedAccumulatedRating > 0.0f)
                    equippedAccumulatedRating = selectedRating - equippedAccumulatedRating;
                if (equippedAccumulateValue > 0)
                    equippedAccumulateValue = selectedValue - equippedAccumulateValue;
                std::array<RE::GFxValue, CEGlobals::SELECTED_ITEM_ARRAY_SIZE>
                    selectedItemInfo = {selectedName, "SlotsTBD", selectedType,
                                        selectedRating, equippedAccumulatedRating,
                                        selectedValue, equippedAccumulateValue,
                                        selectedEnchantmentInfo.c_str()};
                CEMenu::ResetMenu();
                CEMenu::SetActor(actor->GetName());
                CEMenu::CreateSelectedItemCard(selectedItemInfo);
                CEMenu::CreateComparisonItemCards(item_arr);
                CEMenu::ShowMenu();
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