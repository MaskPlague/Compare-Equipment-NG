namespace CEGameMenuUtils
{
    RE::FormID currentFormID;
    RE::TESObjectREFR::InventoryItemMap containerInventoryQLIE;

    const char *GetArmorTypeString(RE::BGSBipedObjectForm::ArmorType type)
    {
        switch (type)
        {
        case RE::BGSBipedObjectForm::ArmorType::kClothing:
            return CEGlobals::clothArmor.c_str();
        case RE::BGSBipedObjectForm::ArmorType::kHeavyArmor:
            return CEGlobals::heavyArmor.c_str();
        case RE::BGSBipedObjectForm::ArmorType::kLightArmor:
            return CEGlobals::lightArmor.c_str();
        default:
            return "Unknown";
        }
    }

    std::string cleanPercentage(std::string str)
    {
        for (size_t i = 0; i < str.length(); i++)
        {
            auto pos1 = str.find('<', i);
            if (pos1 != std::string::npos)
            {
                str.erase(pos1, 1);
            }
            else
                break;
            i = pos1 - 1;
            auto pos2 = str.find('>', i);
            if (pos2 != std::string::npos)
            {
                str.erase(pos2, 1);
            }
            else
                break;
            i = pos2 - 1;
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

    template <class T>
    std::string GetEnchantmentString(T *item, std::string description, float magnitude)
    {
        ltrim(description);
        auto index = description.find("<mag>");
        if (index != std::string::npos)
            return description.substr(0, index) + std::format("{:.1f}", magnitude) + description.substr(index + 5) + "\n";
        else if (description == "")
        {
            RE::BSString str;
            item->GetDescription(str, nullptr);
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

    template <class T>
    std::string GetDescription(T *item)
    {
        std::string text = "";
        RE::BSString str;
        item->GetDescription(str, nullptr);
        std::string description = static_cast<std::string>(str) + "\n";
        if (description.size() > 3)
            text = cleanPercentage(description);
        return text;
    }

    std::string GetArmorSlotsString(RE::TESObjectARMO *armor)
    {
        std::string slotsString = CEGlobals::noneText;
        auto biped = armor->bipedModelData.bipedObjectSlots.get();
        uint32_t bipedUint = static_cast<uint32_t>(biped);
        for (uint32_t slot = 30; slot <= 61; ++slot)
        {
            uint32_t mask = 1u << (slot - 30);
            if ((bipedUint & mask) != 0)
            {
                if (slotsString == CEGlobals::noneText)
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

    template <class T>
    void GetEnchantmentInfo(std::string &effectInfo, RE::EnchantmentItem *enchantment, T *item)
    {
        auto effects = enchantment->effects;
        for (auto effect : effects)
        {
            float magnitude = effect->GetMagnitude();
            std::string description = effect->baseEffect->magicItemDescription.c_str();
            effectInfo += GetEnchantmentString(item, description, magnitude);
        }
    }

    template <class T>
    bool GetPlayerAddedEnchantmentAndName(RE::FormID formId, std::string &effectInfo, T *item, const char *&name)
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
        RE::InventoryEntryData *desc = nullptr;
        for (RE::ItemList::Item *itm : items)
        {
            if (itm && itm->data.objDesc->object->GetFormID() == formId)
            {
                desc = itm->data.objDesc;
                break;
            }
        }
        if (!desc)
            return false;
        RE::BSSimpleList<RE::ExtraDataList *> *lists = desc->extraLists;
        const char *displayName = desc->GetDisplayName();
        if (displayName)
            name = ltrim(displayName);

        if (!lists)
            return false;

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
            GetEnchantmentInfo(effectInfo, ench, item);
        }
        if (effectInfo == "")
            return false;
        return true;
    }

    template <class T>
    void GetEffectsInfo(RE::FormID formId, std::string &effectInfo, RE::EnchantmentItem *enchantment, T *item, const char *&name)
    {
        for (auto token : CEGlobals::effectCheckOrder)
        {
            if (effectInfo == "")
            {
                if (token == 'D')
                    effectInfo = GetDescription(item);
                if (token == 'E' && enchantment)
                    GetEnchantmentInfo(effectInfo, enchantment, item);
                if (token == 'P')
                    GetPlayerAddedEnchantmentAndName(formId, effectInfo, item, name);
            }
            else
                break;
        }
        if (effectInfo == "")
            effectInfo = CEGlobals::noneText;
    }

    void GetSelectedAndEquippedArmorInfo(RE::FormID selectedFormId, RE::TESObjectARMO *selectedArmor)
    {
        RE::BIPED_MODEL biped = selectedArmor->bipedModelData;
        SKSE::stl::enumeration<RE::BIPED_MODEL::BipedObjectSlot, uint32_t> slots = biped.bipedObjectSlots;
        auto actor = CEActorUtils::currentActor;
        if (!CEActorUtils::IsActorValid(actor))
        {
            CEActorUtils::SetActorToNextFollower();
            return;
        }
        CEMenu::ResetMenu();
        CEMenu::SetActor(actor->GetName());
        auto manager = RE::Inventory3DManager::GetSingleton();
        if (manager && CEGlobals::HIDE_3D)
        {
            auto zoom = manager->GetRuntimeData().zoomProgress;
            if (zoom == 0.0f)
                manager->Clear3D();
        }

        RE::GFxValue menu_mc = CEMenu::GetMenu_mc();
        RE::GFxValue ceMenu = CEMenu::GetCEMenu(menu_mc);

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
        std::string selectedEffectInfo = "";
        GetEffectsInfo(selectedFormId, selectedEffectInfo, selectedEnchantment, selectedArmor, selectedName);

        int32_t equippedAccumulateValue = 0;
        int selectedRating = 0;
        bool gotSelectedRatingScaled = true;
        std::string selectedRatingString;
        if (!GetRatingInfo(selectedFormId, inv, player, selectedRating, selectedRatingString))
        {
            selectedRating = static_cast<int>(selectedArmor->armorRating / 100);
            selectedRatingString = std::to_string(selectedRating) + " " + CEGlobals::unscaledText;
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
                        std::string equippedEffectInfo = "";
                        GetEffectsInfo(formId, equippedEffectInfo, equippedEnchantment, equippedArmor, equippedName);

                        equippedAccumulateValue += equippedValue;
                        int equippedRating = 0;
                        std::string equippedRatingString;
                        if (!gotSelectedRatingScaled || !GetRatingInfo(formId, inv, player, equippedRating, equippedRatingString))
                        {
                            equippedRating = static_cast<int>(equippedArmor->armorRating / 100);
                            equippedRatingString = std::to_string(equippedRating) + " " + CEGlobals::unscaledText;
                        }

                        equippedAccumulatedRating += equippedRating;
                        RE::GFxValue equippedEntry = CEIconUtils::GetEquippedEntryObject(formId);
                        std::array<RE::GFxValue, CEGlobals::EQUIPPED_ARMOR_ITEM_ARRAY_SIZE>
                            itemInfo = {equippedName, equippedSlots.c_str(), equippedType,
                                        equippedRatingString.c_str(), equippedValue, equippedEffectInfo.c_str(),
                                        equippedEntry};
                        CEMenu::CreateArmorComparisonItemCard(itemInfo, ceMenu);
                    }
                }
            }
        }
        if (pushedFormIds.size() > 0 || (pushedFormIds.size() == 0 && !selectedIsEquipped))
        {
            equippedAccumulatedRating = selectedRating - equippedAccumulatedRating;
            equippedAccumulateValue = selectedValue - equippedAccumulateValue;
        }
        RE::GFxValue selectedEntryObject = CEIconUtils::GetSelectedEntryObject();
        std::array<RE::GFxValue, CEGlobals::SELECTED_ARMOR_ITEM_ARRAY_SIZE>
            selectedItemInfo = {selectedName, selectedSlots.c_str(), selectedType,
                                selectedRatingString.c_str(), equippedAccumulatedRating,
                                selectedValue, equippedAccumulateValue,
                                selectedEffectInfo.c_str(), selectedEntryObject};
        CEMenu::CreateSelectedArmorItemCard(selectedItemInfo, ceMenu);
        int scale = CEMenu::openedMenuName == "LootMenu" ? CEGlobals::QLIE_SCALE : (CEMenu::openedMenuName == "HUDMenu" ? CEGlobals::HUD_SCALE : CEGlobals::MENU_SCALE);
        int alpha = CEMenu::openedMenuName == "LootMenu" ? CEGlobals::QLIE_BACKGROUND_ALPHA : (CEMenu::openedMenuName == "HUDMenu" ? CEGlobals::HUD_BACKGROUND_ALPHA : CEGlobals::MENU_BACKGROUND_ALPHA);
        std::array<RE::GFxValue, CEGlobals::EQUIPPED_ARMOR_ITEM_ARRAY_SIZE>
            displayCommand = {"DISPLAY", alpha, scale, CEGlobals::ROWS, "", ""};
        CEMenu::CreateArmorComparisonItemCard(displayCommand, ceMenu);
        CEMenu::SetMenuOffsets(ceMenu);
        if (CEGlobals::HIDE_SKY_UI_ITEM_CARD)
            CEMenu::HideSkyUiItemCard(menu_mc);
    }

    bool GetDamageInfo(RE::FormID formId, const RE::TESObjectREFR::InventoryItemMap &inv, RE::PlayerCharacter *player, float &damage, std::string &damageString)
    {
        for (auto &[item, data] : inv)
        {
            if (item && item->GetFormID() == formId)
            {
                RE::InventoryEntryData *entryData = data.second.get();
                damage = player->GetDamage(entryData);
                damageString = std::format("{:.2f}", damage);
                return true;
            }
        }
        return false;
    }

    void GetFullWeaponInformation(RE::FormID formId, RE::TESObjectWEAP *weapon, RE::PlayerCharacter *player, RE::GFxValue ceMenu, std::string type, bool flag,
                                  int &valueDiff, float &speedDiff, float &reachDiff, float &staggerDiff, float &critDiff, float &damageDiff)
    {
        const char *name = weapon->GetName();
        int value = weapon->GetGoldValue();
        float speed = weapon->GetSpeed();
        float reach = weapon->GetReach();
        float stagger = weapon->GetStagger();
        int crit = weapon->GetCritDamage();
        RE::WEAPON_TYPE weaponType = weapon->GetWeaponType();
        std::set<RE::WEAPON_TYPE> rangedWeapons = {RE::WEAPON_TYPE::kBow, RE::WEAPON_TYPE::kCrossbow, RE::WEAPON_TYPE::kStaff};
        if (rangedWeapons.contains(weaponType))
            reach = 99.999f;
        float damage;
        std::string damageString;
        auto inv = player->GetInventory();
        if (!CEActorUtils::currentActor->IsPlayerRef() || !GetDamageInfo(formId, inv, player, damage, damageString))
        {
            damage = weapon->GetAttackDamage();
            damageString = std::format("{:.2f}", damage) + " " + CEGlobals::unscaledText;
        }
        auto enchantment = weapon->formEnchanting;
        std::string effectInfo = "";
        GetEffectsInfo(formId, effectInfo, enchantment, weapon, name);
        std::string handLabel;
        if (flag)
            handLabel = CEGlobals::bothHands;
        else if (type == "left")
            handLabel = CEGlobals::leftHand;
        else if (type == "right")
            handLabel = CEGlobals::rightHand;
        if (type == "selected")
        {
            if (flag)
            {
                damageDiff = damage - damageDiff;
                critDiff = crit - critDiff;
                speedDiff = speed - speedDiff;
                valueDiff = value - valueDiff;
                staggerDiff = stagger - staggerDiff;
                reachDiff = reach - reachDiff;
            }
            RE::GFxValue itemEntryObject = CEIconUtils::GetSelectedEntryObject();
            std::array<RE::GFxValue, CEGlobals::SELECTED_WEAPON_ITEM_ARRAY_SIZE>
                itemInfo = {
                    name, damageString.c_str(), damageDiff, crit, critDiff, value, valueDiff,
                    speed, speedDiff, stagger, staggerDiff, reach, reachDiff, effectInfo.c_str(), itemEntryObject};
            CEMenu::CreateSelectedWeaponItemCard(itemInfo, ceMenu);
            return;
        }
        damageDiff += damage;
        critDiff += crit;
        speedDiff += speed;
        valueDiff += value;
        staggerDiff += stagger;
        reachDiff += reach;
        RE::GFxValue itemEntryObject = CEIconUtils::GetEquippedEntryObject(formId);
        std::array<RE::GFxValue, CEGlobals::EQUIPPED_WEAPON_ITEM_ARRAY_SIZE>
            itemInfo = {
                handLabel.c_str(), name, damageString.c_str(), crit, value,
                speed, stagger, reach, effectInfo.c_str(), itemEntryObject};
        CEMenu::CreateWeaponComparisonItemCard(itemInfo, ceMenu);
    }

    void GetSelectedAndEquippedWeaponInfo(RE::FormID selectedFormId, RE::TESObjectWEAP *selectedWeapon)
    {
        auto actor = CEActorUtils::currentActor;
        if (!CEActorUtils::IsActorValid(actor))
        {
            CEActorUtils::SetActorToNextFollower();
            return;
        }

        CEMenu::ResetMenu();
        CEMenu::SetActor(actor->GetName());
        auto manager = RE::Inventory3DManager::GetSingleton();
        if (manager && CEGlobals::HIDE_3D)
        {
            auto zoom = manager->GetRuntimeData().zoomProgress;
            if (zoom == 0.0f)
                manager->Clear3D();
        }
        RE::GFxValue menu_mc = CEMenu::GetMenu_mc();
        RE::GFxValue ceMenu = CEMenu::GetCEMenu(menu_mc);
        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player)
            return;

        int valueDiff = 0;
        float speedDiff = 0.0f;
        float reachDiff = 0.0f;
        float staggerDiff = 0.0f;
        float critDiff = 0;
        float damageDiff = 0;
        auto left = actor->GetEquippedObject(true);
        auto right = actor->GetEquippedObject(false);
        bool both = false;
        bool leftIsWeapon = false;
        bool rightIsWeapon = false;
        RE::FormID leftFormId = 0;
        if (left && left->IsWeapon())
        {
            leftIsWeapon = true;
            leftFormId = left->GetFormID();
        }
        RE::FormID rightFormId = 0;
        if (right && right->IsWeapon())
        {
            rightIsWeapon = true;
            rightFormId = right->GetFormID();
        }
        if (leftIsWeapon && rightIsWeapon && leftFormId == rightFormId)
        {
            both = true;
        }
        bool selectedIsLeft = (leftIsWeapon && (leftFormId == selectedFormId));
        bool selectedIsRight = (rightIsWeapon && (rightFormId == selectedFormId));
        auto selectedWeaponType = selectedWeapon->GetWeaponType();
        std::set<RE::WEAPON_TYPE> oneHandedTypes = {RE::WEAPON_TYPE::kOneHandAxe,
                                                    RE::WEAPON_TYPE::kOneHandDagger,
                                                    RE::WEAPON_TYPE::kOneHandMace,
                                                    RE::WEAPON_TYPE::kOneHandSword,
                                                    RE::WEAPON_TYPE::kStaff};

        bool aWeaponIsEquipped = false;
        bool oneHanded = oneHandedTypes.contains(selectedWeaponType);
        int weaponCount = 0;
        if (leftIsWeapon && leftFormId != selectedFormId && !selectedIsRight)
        {
            aWeaponIsEquipped = true;
            weaponCount++;
            GetFullWeaponInformation(left->GetFormID(), left->As<RE::TESObjectWEAP>(), player, ceMenu, "left", both,
                                     valueDiff, speedDiff, reachDiff, staggerDiff, critDiff, damageDiff);
            if (oneHanded)
                GetFullWeaponInformation(selectedFormId, selectedWeapon, player, ceMenu, "selected", aWeaponIsEquipped,
                                         valueDiff, speedDiff, reachDiff, staggerDiff, critDiff, damageDiff);
        }
        float reachMax = 0.0f;
        if (rightIsWeapon && rightFormId != selectedFormId && !selectedIsLeft && !both)
        {
            aWeaponIsEquipped = true;
            weaponCount++;
            if (oneHanded)
            {
                valueDiff = 0;
                speedDiff = 0.0f;
                staggerDiff = 0.0f;
                critDiff = 0.0f;
                damageDiff = 0;
                reachDiff = 0.0f;
            }
            else
            {
                reachMax = reachDiff;
                reachDiff = 0.0f;
            }

            GetFullWeaponInformation(right->GetFormID(), right->As<RE::TESObjectWEAP>(), player, ceMenu, "right", false,
                                     valueDiff, speedDiff, reachDiff, staggerDiff, critDiff, damageDiff);
            if (oneHanded)
                GetFullWeaponInformation(selectedFormId, selectedWeapon, player, ceMenu, "selected", aWeaponIsEquipped,
                                         valueDiff, speedDiff, reachDiff, staggerDiff, critDiff, damageDiff);
        }
        if (!aWeaponIsEquipped || !oneHanded)
        {
            if (weaponCount > 0)
            {
                speedDiff /= weaponCount;
                staggerDiff /= weaponCount;
                critDiff /= weaponCount;
                damageDiff /= weaponCount;
            }
            std::string labelText = "selected_" + std::to_string(oneHanded);
            GetFullWeaponInformation(selectedFormId, selectedWeapon, player, ceMenu, "selected", aWeaponIsEquipped,
                                     valueDiff, speedDiff, reachDiff, staggerDiff, critDiff, damageDiff);
        }
        int scale = CEMenu::openedMenuName == "LootMenu" ? CEGlobals::QLIE_SCALE : (CEMenu::openedMenuName == "HUDMenu" ? CEGlobals::HUD_SCALE : CEGlobals::MENU_SCALE);
        int alpha = CEMenu::openedMenuName == "LootMenu" ? CEGlobals::QLIE_BACKGROUND_ALPHA : (CEMenu::openedMenuName == "HUDMenu" ? CEGlobals::HUD_BACKGROUND_ALPHA : CEGlobals::MENU_BACKGROUND_ALPHA);
        std::array<RE::GFxValue, CEGlobals::EQUIPPED_WEAPON_ITEM_ARRAY_SIZE>
            displayCommand = {"DISPLAY", alpha, scale, "", "", "", "", "", "", ""};
        CEMenu::CreateWeaponComparisonItemCard(displayCommand, ceMenu);
        CEMenu::SetMenuOffsets(ceMenu);
        if (CEGlobals::HIDE_SKY_UI_ITEM_CARD)
            CEMenu::HideSkyUiItemCard(menu_mc);
    }

    void GetArmorOrWeapon(RE::FormID selectedFormId)
    {
        if (auto selectedForm = RE::TESForm::LookupByID(selectedFormId))
        {
            if (auto selectedArmor = selectedForm->As<RE::TESObjectARMO>())
            {
                GetSelectedAndEquippedArmorInfo(selectedFormId, selectedArmor);
            }
            else if (auto selectedWeapon = selectedForm->As<RE::TESObjectWEAP>())
            {
                GetSelectedAndEquippedWeaponInfo(selectedFormId, selectedWeapon);
            }
        }
    }

    void DiffCrosshairTargetCheck(RE::CrosshairPickData *crosshair, RE::ObjectRefHandle target)
    {
        SKSE::GetTaskInterface()->AddTask([crosshair, target]()
                                          { std::thread([crosshair, target]()
                                                        {
                                                        while (crosshair && target && crosshair->target && (crosshair->target.get() == target.get()))
                                                        {
                                                            std::this_thread::sleep_for(std::chrono::milliseconds(250));
                                                        }
                                                        CEMenu::HideMenu(); })
                                                .detach(); });
    }

    bool GetItem()
    {
        if (CEMenu::openedMenuName == "LootMenu")
        {
            GetArmorOrWeapon(currentFormID);
            return true;
        }
        else if (CEMenu::openedMenuName == "HUDMenu")
        {
            if (CEGlobals::HUD_TOGGLEMODE && CEMenu::IsMenuVisible())
            {
                CEMenu::HideMenu(true);
                return true;
            }
            auto crosshair = RE::CrosshairPickData::GetSingleton();
            auto target = crosshair->target;
            auto obj = target ? target.get() : nullptr;
            auto bobj = obj ? obj->GetBaseObject() : nullptr;
            auto fid = bobj ? bobj->GetFormID() : NULL;
            currentFormID = fid;
            GetArmorOrWeapon(currentFormID);
            DiffCrosshairTargetCheck(crosshair, target);
            return true;
        }
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
            !selectedEntry.GetMember("formId", &formId) ||
            formId.IsNull())
            return false;
        RE::FormID fid = static_cast<RE::FormID>(formId.GetUInt());
        currentFormID = fid;
        GetArmorOrWeapon(fid);
        return true;
    }

    void ActorChangedUpdateMenu()
    {
        if (!currentFormID)
        {
            return;
        }
        GetArmorOrWeapon(currentFormID);
    }
}