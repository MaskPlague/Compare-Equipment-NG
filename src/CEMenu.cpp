namespace CEMenu
{
    std::string temp = "CompareEquipmentMenu_" + CEGlobals::EXPECTED_SWF_VERSION + "_" + std::to_string(CEGlobals::COMPARE_KEY);
    const char *MENU_NAME = temp.c_str();
    std::string_view SWF_PATH{"CompareEquipment.swf"};
    std::string_view openedMenuName = "HUDMenu";
    std::chrono::steady_clock::time_point lastInvalidation;
    std::set<std::string> openedMenus;

    void UpdateMenuName()
    {
        std::string stringMenuName{openedMenuName};
        std::string key = std::to_string(CEGlobals::COMPARE_KEY);
        if (CEGlobals::lastInputDevice == RE::INPUT_DEVICE::kGamepad)
        {
            key = "273";
        }
        temp = "CompareEquipmentMenu_" + CEGlobals::EXPECTED_SWF_VERSION + "_" + key + "_" + stringMenuName;
        MENU_NAME = temp.c_str();
    }

    void Invalidate(RE::GFxValue buttonBar)
    {
        if (CEGameEvents::NanoToLongMilli(std::chrono::steady_clock::now() - lastInvalidation) > 100)
        {
            lastInvalidation = std::chrono::steady_clock::now();
            buttonBar.Invoke("invalidateData");
        }
    }

    void ShowOrHideQLIEHint(bool forceDelete)
    {
        if (!CEGlobals::QLIE_SHOWHINT)
            return;
        auto UISingleton = RE::UI::GetSingleton();
        auto menu = UISingleton ? UISingleton->GetMenu("LootMenu") : nullptr;
        RE::GFxMovieView *view = menu ? menu->uiMovie.get() : nullptr;

        if (!UISingleton || !menu || !view)
            return;
        bool showButton = false;
        if (auto form = RE::TESForm::LookupByID(CEGameMenuUtils::currentFormID))
        {

            if (auto armor = form->As<RE::TESObjectARMO>())
                showButton = true;
            else if (auto weapon = form->As<RE::TESObjectWEAP>())
                showButton = true;
        }
        RE::GFxValue buttonBar;
        RE::GFxValue dataProvider;
        if (view->GetVariable(&buttonBar, "_root.lootMenu.buttonBar"))
        {
            if (buttonBar.GetMember("dataProvider", &dataProvider))
            {
                RE::GFxValue lastObj;
                dataProvider.GetElement(dataProvider.GetArraySize() - 1, &lastObj);
                if (lastObj.IsObject())
                {
                    RE::GFxValue lastObjCENG;
                    lastObj.GetMember("CENG", &lastObjCENG);
                    if (lastObjCENG.IsBool() && (!showButton || forceDelete))
                    {
                        Invalidate(buttonBar);
                        dataProvider.RemoveElement(dataProvider.GetArraySize() - 1);
                        lastObj.SetNull();
                    }
                    if (lastObjCENG.IsBool() && showButton)
                        return;
                }
                if (!showButton)
                    return;

                RE::GFxValue obj;
                view->CreateObject(&obj);
                obj.SetMember("label", CEGlobals::QLIE_HINT_TEXT.c_str());
                obj.SetMember("index", CEGlobals::COMPARE_KEY);
                obj.SetMember("stolen", false);
                obj.SetMember("CENG", true);
                Invalidate(buttonBar);
                dataProvider.PushBack(obj);
            }
        }
    }

    RE::GFxValue GetMenu_mc(std::string_view nameOfMenuToGet)
    {
        logger::trace("nameOfMenuToGet: {}", nameOfMenuToGet);
        auto UISingleton = RE::UI::GetSingleton();
        auto menuToGet = nameOfMenuToGet != "HUDMenu" ? nameOfMenuToGet : RE::HUDMenu::MENU_NAME;
        auto menu = UISingleton ? UISingleton->GetMenu(menuToGet) : nullptr;
        RE::GFxMovieView *view = menu ? menu->uiMovie.get() : nullptr;
        if (!view)
            return nullptr;

        RE::GFxValue Menu_mc;
        bool menuObtained = view->GetVariable(
            &Menu_mc,
            nameOfMenuToGet == "LootMenu"  ? "_root.lootMenu"
            : nameOfMenuToGet == "HUDMenu" ? "_root"
                                           : "_root.Menu_mc");
        if (!menuObtained)
            return nullptr;
        return Menu_mc;
    }

    RE::GFxValue GetCEMenu(RE::GFxValue Menu_mc)
    {
        RE::GFxValue ceMenu;
        if (Menu_mc.IsNull() || !Menu_mc.GetMember(MENU_NAME, &ceMenu))
            return nullptr;
        return ceMenu;
    }

    bool IsMenuVisible()
    {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return false;
        RE::GFxValue result;
        ceMenu.Invoke("getVisible", &result);
        if (result.IsBool())
            return result.GetBool();
        return false;
    }

    void SetMenuOffsets(RE::GFxValue ceMenu)
    {
        double xOffset;
        double yOffset;
        if (openedMenuName == "LootMenu")
        {
            xOffset = CEGlobals::QLIE_X_ORIGIN;
            yOffset = CEGlobals::QLIE_Y_ORIGIN;
        }
        else if (openedMenuName == "HUDMenu")
        {
            xOffset = CEGlobals::HUD_X_ORIGIN;
            yOffset = CEGlobals::HUD_X_ORIGIN;
        }
        else if (openedMenuName == RE::InventoryMenu::MENU_NAME)
        {
            xOffset = CEGlobals::INV_MENU_X_ORIGIN;
            yOffset = CEGlobals::INV_MENU_Y_ORIGIN;
        }
        else if (openedMenuName == RE::ContainerMenu::MENU_NAME)
        {
            xOffset = CEGlobals::CONT_MENU_X_ORIGIN;
            yOffset = CEGlobals::CONT_MENU_Y_ORIGIN;
        }
        else if (openedMenuName == RE::BarterMenu::MENU_NAME)
        {
            xOffset = CEGlobals::BART_MENU_X_ORIGIN;
            yOffset = CEGlobals::BART_MENU_Y_ORIGIN;
        }
        else if (openedMenuName == RE::GiftMenu::MENU_NAME)
        {
            xOffset = CEGlobals::GIFT_MENU_X_ORIGIN;
            yOffset = CEGlobals::GIFT_MENU_Y_ORIGIN;
        }
        if (!ceMenu.SetMember("_x", xOffset))
            return;
        if (!ceMenu.SetMember("_y", yOffset))
            return;
    }

    void ShowMenuInstant()
    {
        SKSE::GetTaskInterface()->AddUITask([]()
                                            {
        if (IsMenuVisible())
            return;
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;
        logger::trace("Showing menu.");
        ceMenu.Invoke("showMenu"); });
    }

    void ShowMenuDelayed()
    {
        SKSE::GetTaskInterface()->AddTask([]()
                                          { std::thread([]()
                                                        {   
                                                            std::this_thread::sleep_for(std::chrono::milliseconds(50)); 
                                                            ShowMenuInstant(); })
                                                .detach(); });
    }

    void HideMenu(bool checked)
    {
        SKSE::GetTaskInterface()->AddUITask([checked]()
                                            {
        if (!checked && !IsMenuVisible())
            return;
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;
        logger::trace("Hiding menu.");
        ceMenu.Invoke("hideAndReset"); });
    }

    void SetTranslations()
    {
        std::thread([]()
                    {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            SKSE::GetTaskInterface()->AddUITask([]()
                                                {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;
        logger::trace("Setting translations.");
        std::array<RE::GFxValue, 29> translationInfo;
        translationInfo[0].SetString(CEGlobals::buttonCompareText);
        translationInfo[1].SetString(CEGlobals::comparingTo);
        translationInfo[2].SetString(CEGlobals::damageLabelText);
        translationInfo[3].SetString(CEGlobals::critLabelText);
        translationInfo[4].SetString(CEGlobals::noneText);
        translationInfo[5].SetString(CEGlobals::armorTypeLabelText);
        translationInfo[6].SetString(CEGlobals::armorRatingLabelText);
        translationInfo[7].SetString(CEGlobals::goldLabelText);
        translationInfo[8].SetString(CEGlobals::effectsLabelText);
        translationInfo[9].SetString(CEGlobals::equippedTo);
        translationInfo[10].SetString(CEGlobals::slotsLabelText);
        translationInfo[11].SetString(CEGlobals::speedLabelText);
        translationInfo[12].SetString(CEGlobals::reachLabelText);
        translationInfo[13].SetString(CEGlobals::staggerLabelText);
        translationInfo[14].SetString(CEGlobals::keyText);
        translationInfo[15].SetString(CEGlobals::keyInfo);
        translationInfo[16].SetString(CEGlobals::totalKey);
        translationInfo[17].SetString(CEGlobals::averageKey);
        translationInfo[18].SetString(CEGlobals::maxKey);
        translationInfo[19].SetString(CEGlobals::notApplicable);
        translationInfo[20].SetString(CEGlobals::betterWrapperStart);
        translationInfo[21].SetString(CEGlobals::betterWrapperEnd);
        translationInfo[22].SetString(CEGlobals::worseWrapperStart);
        translationInfo[23].SetString(CEGlobals::worseWrapperEnd);
        translationInfo[24].SetString(CEGlobals::normalWrapperStart);
        translationInfo[25].SetString(CEGlobals::normalWrapperEnd);
        translationInfo[26].SetNumber(CEGlobals::diffOffset);
        translationInfo[27].SetNumber(CEGlobals::valueOffset);
        translationInfo[28].SetNumber(CEGlobals::columnTwoOffset);
        ceMenu.Invoke("setTranslations", nullptr, translationInfo);
         }); })
            .detach();
    }

    void CreateArmorComparisonItemCard(std::array<RE::GFxValue, CEGlobals::EQUIPPED_ARMOR_ITEM_ARRAY_SIZE> itemInfo, RE::GFxValue ceMenu)
    {
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;
        logger::trace("ceMenu exists, creating comparison item card");
        ceMenu.Invoke("createArmorComparisonItemCard", nullptr, itemInfo);
    }

    void CreateWeaponComparisonItemCard(std::array<RE::GFxValue, CEGlobals::EQUIPPED_WEAPON_ITEM_ARRAY_SIZE> itemInfo, RE::GFxValue ceMenu)
    {
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;
        logger::trace("ceMenu exists, creating comparison item card");
        ceMenu.Invoke("createWeaponComparisonItemCard", nullptr, itemInfo);
    }

    void ResetMenu()
    {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;
        logger::trace("Resetting menu.");
        ceMenu.Invoke("reset");
    }

    void SetActor(std::string actorName)
    {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;
        RE::GFxValue args[1];
        args[0].SetString(actorName);
        ceMenu.Invoke("setActor", nullptr, args, 1);
    }

    void CreateSelectedArmorItemCard(std::array<RE::GFxValue, CEGlobals::SELECTED_ARMOR_ITEM_ARRAY_SIZE> itemInfo, RE::GFxValue ceMenu)
    {
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;
        ceMenu.Invoke("populateSelectedArmorItemCard", nullptr, itemInfo);
    }

    void CreateSelectedWeaponItemCard(std::array<RE::GFxValue, CEGlobals::SELECTED_WEAPON_ITEM_ARRAY_SIZE> itemInfo, RE::GFxValue ceMenu)
    {
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;
        ceMenu.Invoke("populateSelectedWeaponItemCard", nullptr, itemInfo);
    }

    void DestroyMenu(std::string menuToDestroy)
    {
        SKSE::GetTaskInterface()->AddUITask([menuToDestroy]()
                                            {
        logger::debug("Destroying Menu {}", menuToDestroy);
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc(menuToDestroy));
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;

        if (!ceMenu.Invoke("removeMovieClip"))
            return;
        logger::trace("Removed {}", MENU_NAME); });
    }

    void CreateMenu(std::string_view menuName)
    {
        SKSE::GetTaskInterface()->AddUITask([menuName]()
                                            {
        openedMenuName = menuName;
        UpdateMenuName();
        logger::debug("Creating Menu");
        RE::GFxValue Menu_mc = GetMenu_mc();
        if (Menu_mc.IsNull() || Menu_mc.IsUndefined() || !Menu_mc.IsObject())
            return;

        RE::GFxValue _ceMenu = GetCEMenu(Menu_mc);
        if (!_ceMenu.IsNull())
            _ceMenu.Invoke("removeMovieClip");

        RE::GFxValue args[2];
        args[0].SetString(MENU_NAME); // name
        args[1] = 3999;               // depth
        if (!Menu_mc.Invoke("createEmptyMovieClip", nullptr, args, 2))
            return;
        logger::trace("Created menu {}", MENU_NAME);

        RE::GFxValue ceMenu = GetCEMenu(Menu_mc);
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;

        RE::GFxValue args2[1];
        args2[0].SetString(SWF_PATH); // name
        if (!ceMenu.Invoke("loadMovie", nullptr, args2, 1))
            return;
        logger::trace("Loaded {} via invoke", args2[0].GetString());
        CEActorUtils::GetActiveFollowers();
        SetTranslations();
        CEMenu::ShowOrHideQLIEHint(); });
    }
}