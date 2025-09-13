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

    void ltrim(std::string &s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
                                        { return !std::isspace(ch); }));
    }

    const char *ltrim(const char *s)
    {
        if (!s)
            return nullptr;
        while (*s && std::isspace(static_cast<unsigned char>(*s)))
            ++s;
        return s;
    }

    std::string GetEnchantmentString(RE::TESObjectARMO *armor, std::string description, float magnitude)
    {
        ltrim(description);
        auto index = description.find("<mag>");
        if (index != std::string::npos)
            return description.substr(0, index) + std::format("{:.1f}", magnitude) + description.substr(index + 5) + "\n";
        else if (description == "")
        {
            RE::BSString str;
            armor->GetDescription(str, nullptr);
            std::string string = static_cast<std::string>(str);
            ltrim(string);
            cleanPercentage(string);
            if (string == "")
                return "";
            else
                return cleanPercentage(string) + "\n";
        }
        else
            return cleanPercentage(description) + "\n";
    }

    std::string GetArmorDescription(RE::TESObjectARMO *armor)
    {
        std::string text = "";
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

    bool GetRatingInfo(RE::FormID formId, const RE::TESObjectREFR::InventoryItemMap &inv, RE::PlayerCharacter *player, int &rating, std::string &ratingString)
    {
        for (auto &[item, data] : inv)
        {
            if (item && item->GetFormID() == formId)
            {
                RE::InventoryEntryData *entryData = data.second.get();
                rating = static_cast<int>(player->GetArmorValue(entryData));
                ratingString = std::to_string(rating);
                return true;
            }
        }
        return false;
    }

    void GetEnchantmentInfo(std::string &effectInfo, RE::EnchantmentItem *enchantment, RE::TESObjectARMO *armor)
    {
        auto effects = enchantment->effects;
        for (auto effect : effects)
        {
            float magnitude = effect->GetMagnitude();
            std::string description = effect->baseEffect->magicItemDescription.c_str();
            effectInfo += GetEnchantmentString(armor, description, magnitude);
        }
    }

    bool GetUserEnchantmentAndName(RE::FormID formId, std::string &effectInfo, RE::TESObjectARMO *armor, const char *&name)
    {
        RE::UI *uiSingleton = RE::UI::GetSingleton();
        RE::BSTArray<RE::ItemList::Item *> items;
        if (RE::InventoryMenu::MENU_NAME == CEMenu::openedMenuName)
        {
            RE::GPtr<RE::InventoryMenu> menu = uiSingleton->GetMenu<RE::InventoryMenu>();
            if (!menu)
                return false;
            items = menu->GetRuntimeData().itemList->items;
        }
        else if (RE::ContainerMenu::MENU_NAME == CEMenu::openedMenuName)
        {
            RE::GPtr<RE::ContainerMenu> menu = uiSingleton->GetMenu<RE::ContainerMenu>();
            if (!menu)
                return false;
            items = menu->GetRuntimeData().itemList->items;
        }
        else if (RE::BarterMenu::MENU_NAME == CEMenu::openedMenuName)
        {
            RE::GPtr<RE::BarterMenu> menu = uiSingleton->GetMenu<RE::BarterMenu>();
            if (!menu)
                return false;
            items = menu->GetRuntimeData().itemList->items;
        }
        else if (RE::GiftMenu::MENU_NAME == CEMenu::openedMenuName)
        {
            RE::GPtr<RE::GiftMenu> menu = uiSingleton->GetMenu<RE::GiftMenu>();
            if (!menu)
                return false;
            items = menu->GetRuntimeData().itemList->items;
        }
        else
            return false;
        logger::trace("Got items");
        RE::InventoryEntryData *desc = nullptr;
        for (RE::ItemList::Item *item : items)
        {
            if (item && item->data.objDesc->object->GetFormID() == formId)
            {
                desc = item->data.objDesc;
                break;
            }
        }
        if (!desc)
            return false;
        logger::trace("Got item to check");
        RE::BSSimpleList<RE::ExtraDataList *> *lists = desc->extraLists;
        const char *displayName = desc->GetDisplayName();
        if (displayName)
            name = ltrim(displayName);

        if (!lists)
            return false;

        logger::trace("Got extra lists");
        for (auto &list : *lists)
        {
            if (!list)
                continue;
            auto xEnch = list->GetByType<RE::ExtraEnchantment>();
            if (!xEnch)
                continue;

            auto ench = xEnch->enchantment;
            if (!ench)
                continue;
            GetEnchantmentInfo(effectInfo, ench, armor);
        }
        if (effectInfo == "")
            return false;
        return true;
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
        {
            auto zoom = manager->GetRuntimeData().zoomProgress;
            if (zoom == 0.0f)
                manager->Clear3D();
        }
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
                const RE::TESObjectREFR::InventoryItemMap &inv = actor->GetInventory();
                auto selectedName = selectedArmor->GetName();
                std::string selectedSlots = "";
                selectedSlots += GetArmorSlotsString(selectedArmor);
                const char *selectedType = GetArmorTypeString(selectedArmor->GetArmorType());
                auto selectedValue = selectedArmor->GetGoldValue();
                auto selectedEnchantment = selectedArmor->formEnchanting;
                std::string selectedEffectInfo = GetArmorDescription(selectedArmor);
                if (selectedEnchantment && selectedEffectInfo == "")
                {
                    GetEnchantmentInfo(selectedEffectInfo, selectedEnchantment, selectedArmor);
                }
                if (selectedEffectInfo == "")
                    if (!GetUserEnchantmentAndName(selectedFormId, selectedEffectInfo, selectedArmor, selectedName))
                        selectedEffectInfo = "None";

                int32_t equippedAccumulateValue = 0;
                int selectedRating = 0;
                bool gotSelectedRatingScaled = true;
                std::string selectedRatingString;
                if (!GetRatingInfo(selectedFormId, inv, player, selectedRating, selectedRatingString))
                {
                    selectedRating = static_cast<int>(selectedArmor->armorRating / 100);
                    selectedRatingString = std::to_string(selectedRating) + " (Unscaled)";
                    gotSelectedRatingScaled = false;
                }

                int equippedAccumulatedRating = 0;
                std::vector<RE::FormID> pushedFormIds;
                bool selectedIsEquipped = false;
                for (auto slot : slotList)
                {
                    if ((slots & slot) != Slot::kNone)
                    {
                        auto equippedArmor = actor->GetWornArmor(slot);
                        if (equippedArmor)
                        {
                            auto formId = equippedArmor->GetFormID();
                            if (selectedFormId == formId)
                                selectedIsEquipped = true;
                            bool alreadyPushed = std::find(pushedFormIds.begin(), pushedFormIds.end(), formId) != pushedFormIds.end();
                            if (selectedFormId != formId && !alreadyPushed)
                            {
                                pushedFormIds.push_back(formId);
                                const char *equippedName = equippedArmor->GetName();
                                std::string equippedSlots = GetArmorSlotsString(equippedArmor);
                                const char *equippedType = GetArmorTypeString(equippedArmor->GetArmorType());
                                int32_t equippedValue = equippedArmor->GetGoldValue();
                                auto equippedEnchantment = equippedArmor->formEnchanting;
                                std::string equippedEffectInfo = GetArmorDescription(equippedArmor);
                                if (equippedEnchantment && equippedEffectInfo == "")
                                {
                                    GetEnchantmentInfo(equippedEffectInfo, equippedEnchantment, equippedArmor);
                                }
                                if (equippedEffectInfo == "")
                                    if (!GetUserEnchantmentAndName(formId, equippedEffectInfo, equippedArmor, equippedName))
                                        equippedEffectInfo = "None";

                                equippedAccumulateValue += equippedValue;
                                int equippedRating = 0;
                                std::string equippedRatingString;
                                if (!gotSelectedRatingScaled || !GetRatingInfo(formId, inv, player, equippedRating, equippedRatingString))
                                {
                                    equippedRating = static_cast<int>(equippedArmor->armorRating / 100);
                                    equippedRatingString = std::to_string(equippedRating) + " (Unscaled)";
                                }

                                equippedAccumulatedRating += equippedRating;
                                std::array<RE::GFxValue, CEGlobals::EQUIPPED_ITEM_ARRAY_SIZE>
                                    itemInfo = {equippedName, equippedSlots.c_str(), equippedType,
                                                equippedRatingString.c_str(), equippedValue, equippedEffectInfo.c_str()};
                                CEMenu::CreateComparisonItemCard(itemInfo, ceMenu);
                            }
                        }
                    }
                }
                if (pushedFormIds.size() > 0 || (pushedFormIds.size() == 0 && !selectedIsEquipped))
                {
                    equippedAccumulatedRating = selectedRating - equippedAccumulatedRating;
                    equippedAccumulateValue = selectedValue - equippedAccumulateValue;
                }
                logger::trace("Creating selectedItemInfo");
                std::array<RE::GFxValue, CEGlobals::SELECTED_ITEM_ARRAY_SIZE>
                    selectedItemInfo = {selectedName, selectedSlots.c_str(), selectedType,
                                        selectedRatingString.c_str(), equippedAccumulatedRating,
                                        selectedValue, equippedAccumulateValue,
                                        selectedEffectInfo.c_str()};

                logger::trace("Populating selected item card");
                CEMenu::CreateSelectedItemCard(selectedItemInfo);
                logger::trace("Positioning and displaying item cards");
                std::array<RE::GFxValue, CEGlobals::EQUIPPED_ITEM_ARRAY_SIZE>
                    displayCommand = {"DISPLAY", CEGlobals::BACKGROUND_ALPHA, CEGlobals::SCALE, CEGlobals::ROWS, "", ""};
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