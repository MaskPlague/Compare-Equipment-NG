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

    template <class T>
    void GetEnchantmentInfo(std::string &effectInfo, RE::EnchantmentItem *enchantment, T *itemWA)
    {
        auto effects = enchantment->effects;
        for (auto effect : effects)
        {
            float magnitude = effect->GetMagnitude();
            std::string description = effect->baseEffect->magicItemDescription.c_str();
            effectInfo += GetEnchantmentString(itemWA, description, magnitude);
        }
    }

    template <class T>
    void GetPlayerEnchantment(std::string &effectInfo, T *itemWA, RE::InventoryEntryData *entryData)
    {
        RE::BSSimpleList<RE::ExtraDataList *> *lists = entryData->extraLists;
        if (!lists)
            return;
        for (auto &list : *lists) // player enchantment
        {
            if (!list)
                continue;
            auto xEnch = list->GetByType<RE::ExtraEnchantment>();
            if (!xEnch)
                continue;
            auto ench = xEnch->enchantment;
            if (!ench)
                continue;
            GetEnchantmentInfo(effectInfo, ench, itemWA);
        }
    }

    template <class T>
    void GetEffectsInfo(std::string &effectInfo, RE::EnchantmentItem *enchantment, T *itemWA, RE::InventoryEntryData *entryData)
    {
        for (auto token : CEGlobals::effectCheckOrder)
        {
            if (effectInfo == "")
            {
                if (token == 'D')
                    effectInfo = GetDescription(itemWA);
                if (token == 'E' && enchantment)
                    GetEnchantmentInfo(effectInfo, enchantment, itemWA);
                if (token == 'P')
                    GetPlayerEnchantment(effectInfo, itemWA, entryData);
            }
            else
                break;
        }
        if (effectInfo == "")
            effectInfo = CEGlobals::noneText;
    }

    template <class T>
    void GetInfoFromEntryData(RE::InventoryEntryData *entryData, const char *&name, int32_t &value, float &stat, std::string &statString, bool armor,
                              std::string &effectInfo, T *itemWA)
    {
        auto player = RE::PlayerCharacter::GetSingleton();
        auto armorvalue = player->GetArmorValue(entryData);
        auto damage = player->GetDamage(entryData);
        name = ltrim(entryData->GetDisplayName());
        value = entryData->GetValue();
        if (armor)
        {
            stat = armorvalue;
            statString = std::format("{:.0f}", armorvalue);
        }
        else
        {
            stat = damage;
            statString = std::format("{:.2f}", damage);
        }
        RE::EnchantmentItem *enchantment = itemWA->formEnchanting;
        GetEffectsInfo(effectInfo, enchantment, itemWA, entryData);
    }

    template <class T>
    bool GetInfoFromInventory(RE::FormID formId, const char *&name, int32_t &value, float &stat, std::string &statString, bool armor,
                              std::string &effectInfo, T *itemWA, RE::TESObjectREFR::InventoryItemMap &inv)
    {
        for (auto &[item, data] : inv)
        {
            if (item && item->GetFormID() == formId)
            {
                RE::InventoryEntryData *entryData = data.second.get();
                GetInfoFromEntryData(entryData, name, value, stat, statString, armor, effectInfo, itemWA);
                return true;
            }
        }
        return false;
    }

    template <class T>
    bool GetInfo(RE::FormID formId, const char *&name, int32_t &value, float &stat, std::string &statString, bool armor,
                 bool selected, std::string &effectInfo, T *item)
    {
        if (!selected &&
            (!CEActorUtils::currentActor->IsPlayerRef() ||
             (CEActorUtils::currentActor->IsPlayerRef() && "LootMenu" == CEMenu::openedMenuName)))
        {
            auto inv = CEActorUtils::currentActor->GetInventory();
            return GetInfoFromInventory(formId, name, value, stat, statString, armor, effectInfo, item, inv);
        }
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
        else if ("LootMenu" == CEMenu::openedMenuName)
        {
            return GetInfoFromInventory(formId, name, value, stat, statString, armor, effectInfo, item, containerInventoryQLIE);
        }
        else
            return false;
        RE::InventoryEntryData *entryData = nullptr;
        for (RE::ItemList::Item *itm : items)
        {
            if (itm && itm->data.objDesc->object->GetFormID() == formId)
            {
                entryData = itm->data.objDesc;
                break;
            }
        }
        if (!entryData)
            return false;
        GetInfoFromEntryData(entryData, name, value, stat, statString, armor, effectInfo, item);
        return true;
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
        if (!player || ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
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
        auto selectedName = selectedArmor->GetName();
        std::string selectedSlots = "";
        selectedSlots += GetArmorSlotsString(selectedArmor);
        const char *selectedType = GetArmorTypeString(selectedArmor->GetArmorType());
        int32_t selectedValue = 0;
        std::string selectedEffectInfo = "";
        int32_t equippedAccumulateValue = 0;
        float selectedRatingFloat = 0.0;
        int selectedRating = 0;
        std::string selectedRatingString;
        bool gotScaledInfo = true;
        if (!GetInfo(selectedFormId, selectedName, selectedValue, selectedRatingFloat, selectedRatingString, true,
                     true, selectedEffectInfo, selectedArmor))
        {
            selectedRating = static_cast<int>(selectedArmor->armorRating / 100);
            selectedRatingString = std::to_string(selectedRating) + " " + CEGlobals::unscaledText;
            selectedValue = selectedArmor->GetGoldValue();
            gotScaledInfo = false;
        }
        else
            selectedRating = static_cast<int>(selectedRatingFloat);

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
                        int32_t equippedValue = 0;
                        std::string equippedEffectInfo = "";
                        equippedAccumulateValue += equippedValue;
                        float equippedRatingFloat = 0.0;
                        int equippedRating = 0;
                        std::string equippedRatingString;
                        if (!GetInfo(formId, equippedName, equippedValue, equippedRatingFloat, equippedRatingString, true,
                                     false, equippedEffectInfo, equippedArmor) ||
                            !gotScaledInfo)
                        {
                            equippedRating = static_cast<int>(equippedArmor->armorRating / 100);
                            equippedRatingString = std::to_string(equippedRating) + " " + CEGlobals::unscaledText;
                            equippedValue = equippedArmor->GetGoldValue();
                        }
                        else
                            equippedRating = static_cast<int>(equippedRatingFloat);

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

    bool GetDamageAndNameInfoFromActorInventory(RE::FormID formId, const RE::TESObjectREFR::InventoryItemMap &inv, RE::PlayerCharacter *player,
                                                const char *&name, float &damage, std::string &damageString)
    {
        for (auto &[item, data] : inv)
        {
            if (item && item->GetFormID() == formId)
            {
                RE::InventoryEntryData *entryData = data.second.get();
                damage = player->GetDamage(entryData);
                name = entryData->GetDisplayName();
                damageString = std::format("{:.2f}", damage);
                return true;
            }
        }
        return false;
    }

    void GetFullWeaponInformation(RE::FormID formId, RE::TESObjectWEAP *weapon, RE::GFxValue ceMenu, std::string type, bool flag,
                                  int &valueDiff, float &speedDiff, float &reachDiff, float &staggerDiff, float &critDiff, float &damageDiff, bool getScaledInfo)
    {
        const char *name = weapon->GetName();
        int32_t value = 0;
        float speed = weapon->GetSpeed();
        float reach = weapon->GetReach();
        float stagger = weapon->GetStagger();
        int crit = weapon->GetCritDamage();
        RE::WEAPON_TYPE weaponType = weapon->GetWeaponType();
        std::set<RE::WEAPON_TYPE> rangedWeapons = {RE::WEAPON_TYPE::kBow, RE::WEAPON_TYPE::kCrossbow, RE::WEAPON_TYPE::kStaff};
        if (rangedWeapons.contains(weaponType))
            reach = 99.999f;
        std::string effectInfo = "";
        float damage;
        std::string damageString;
        auto inv = CEActorUtils::currentActor->GetInventory();
        bool isSelected = (type == "selected");
        if (!GetInfo(formId, name, value, damage, damageString, false, isSelected, effectInfo, weapon) || !getScaledInfo)
        {
            damage = weapon->GetAttackDamage();
            damageString = std::format("{:.2f}", damage) + " " + CEGlobals::unscaledText;
            value = weapon->GetGoldValue();
        }
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
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
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
        const char *selectedName = "";
        const char *leftName = "";
        const char *rightName = "";
        RE::TESObjectWEAP *leftWeapon = nullptr;
        RE::TESObjectWEAP *rightWeapon = nullptr;
        int32_t vdummy;
        float sdummy;
        std::string svdummy;
        std::string eisdummy;

        bool getScaledInfo = GetInfo(selectedFormId, selectedName, vdummy, sdummy, svdummy, false, true, eisdummy, selectedWeapon);
        RE::FormID leftFormId = 0;
        if (left && left->IsWeapon())
        {
            leftIsWeapon = true;
            leftFormId = left->GetFormID();
            leftWeapon = left->As<RE::TESObjectWEAP>();
            GetInfo(leftFormId, leftName, vdummy, sdummy, svdummy, false, false, eisdummy, leftWeapon);
        }
        RE::FormID rightFormId = 0;
        if (right && right->IsWeapon())
        {
            rightIsWeapon = true;
            rightFormId = right->GetFormID();
            rightWeapon = right->As<RE::TESObjectWEAP>();
            GetInfo(rightFormId, rightName, vdummy, sdummy, svdummy, false, false, eisdummy, rightWeapon);
        }
        if (leftIsWeapon && rightIsWeapon && leftFormId == rightFormId && std::strcmp(leftName, rightName) == 0)
            both = true;

        bool selectedIsLeft = false;
        if (leftIsWeapon && leftFormId == selectedFormId && std::strcmp(leftName, selectedName) == 0)
            selectedIsLeft = true;
        bool selectedIsRight = false;
        if (rightIsWeapon && rightFormId == selectedFormId && std::strcmp(rightName, selectedName) == 0)
            selectedIsRight = true;
        auto selectedWeaponType = selectedWeapon->GetWeaponType();
        std::set<RE::WEAPON_TYPE> oneHandedTypes = {RE::WEAPON_TYPE::kOneHandAxe,
                                                    RE::WEAPON_TYPE::kOneHandDagger,
                                                    RE::WEAPON_TYPE::kOneHandMace,
                                                    RE::WEAPON_TYPE::kOneHandSword,
                                                    RE::WEAPON_TYPE::kStaff};

        bool aWeaponIsEquipped = false;
        bool oneHanded = oneHandedTypes.contains(selectedWeaponType);
        int weaponCount = 0;
        if (leftIsWeapon && !selectedIsLeft && !selectedIsRight)
        {
            aWeaponIsEquipped = true;
            weaponCount++;
            GetFullWeaponInformation(leftFormId, leftWeapon, ceMenu, "left", both,
                                     valueDiff, speedDiff, reachDiff, staggerDiff, critDiff, damageDiff, getScaledInfo);
            if (oneHanded)
                GetFullWeaponInformation(selectedFormId, selectedWeapon, ceMenu, "selected", aWeaponIsEquipped,
                                         valueDiff, speedDiff, reachDiff, staggerDiff, critDiff, damageDiff, getScaledInfo);
        }
        float reachMax = 0.0f;
        if (rightIsWeapon && !selectedIsRight && !selectedIsLeft && !both)
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
            GetFullWeaponInformation(right->GetFormID(), right->As<RE::TESObjectWEAP>(), ceMenu, "right", both,
                                     valueDiff, speedDiff, reachDiff, staggerDiff, critDiff, damageDiff, getScaledInfo);
            if (oneHanded && !both)
                GetFullWeaponInformation(selectedFormId, selectedWeapon, ceMenu, "selected", aWeaponIsEquipped,
                                         valueDiff, speedDiff, reachDiff, staggerDiff, critDiff, damageDiff, getScaledInfo);
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
            GetFullWeaponInformation(selectedFormId, selectedWeapon, ceMenu, "selected", aWeaponIsEquipped,
                                     valueDiff, speedDiff, reachDiff, staggerDiff, critDiff, damageDiff, getScaledInfo);
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

    bool GetArmorOrWeapon(RE::FormID selectedFormId)
    {
        if (auto selectedForm = RE::TESForm::LookupByID(selectedFormId))
        {
            if (auto selectedArmor = selectedForm->As<RE::TESObjectARMO>())
            {
                GetSelectedAndEquippedArmorInfo(selectedFormId, selectedArmor);
                return true;
            }
            else if (auto selectedWeapon = selectedForm->As<RE::TESObjectWEAP>())
            {
                GetSelectedAndEquippedWeaponInfo(selectedFormId, selectedWeapon);
                return true;
            }
        }
        return false;
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
        logger::debug("GetItem called");
        if (CEMenu::openedMenuName == "LootMenu")
        {
            if (!CEGlobals::QLIE_ALLOWED)
                return false;
            if (CEGlobals::QLIE_PERSISTENT_DISPLAY && CEGlobals::QLIE_PERSISTENT_TOGGLE && CEMenu::IsMenuVisible())
            {
                CEMenu::qliePersistentToggledOn = false;
                CEMenu::HideMenu(true);
                return false;
            }
            GetArmorOrWeapon(currentFormID);
            return true;
        }
        else if (CEMenu::openedMenuName == "HUDMenu")
        {
            if (!CEGlobals::HUD_ALLOWED)
                return false;
            if (CEGlobals::HUD_TOGGLEMODE && CEMenu::IsMenuVisible())
            {
                CEMenu::HideMenu(true);
                return false;
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

        if (CEGlobals::MENU_PERSISTENT_DISPLAY && CEGlobals::MENU_PERSISTENT_TOGGLE && CEMenu::IsMenuVisible())
        {
            CEMenu::menuPersistentToggledOn = false;
            CEMenu::HideMenu(true);
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
        return GetArmorOrWeapon(fid);
    }

    void ActorChangedUpdateMenu()
    {
        if (!currentFormID)
        {
            return;
        }
        GetArmorOrWeapon(currentFormID);
    }

    bool isWeaponOrArmor(RE::FormID formId)
    {
        if (auto form = RE::TESForm::LookupByID(formId))
        {
            if (auto armor = form->As<RE::TESObjectARMO>())
                return true;
            else if (auto weapon = form->As<RE::TESObjectWEAP>())
                return true;
        }
        return false;
    }
}