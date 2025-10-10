namespace logger = SKSE::log;

namespace CEMenu
{
    std::string temp = "CompareEquipmentMenu_" + CEGlobals::EXPECTED_SWF_VERSION + "_" + std::to_string(CEGlobals::COMPARE_KEY);
    const char *MENU_NAME = temp.c_str();
    std::string_view SWF_PATH{"CompareEquipment.swf"};
    std::string_view openedMenuName = "HUDMenu";
    std::chrono::steady_clock::time_point lastInvalidation;

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

    RE::GFxValue GetMenu_mc()
    {
        auto UISingleton = RE::UI::GetSingleton();
        auto menuToGet = openedMenuName != "HUDMenu" ? openedMenuName : RE::HUDMenu::MENU_NAME;
        auto menu = UISingleton ? UISingleton->GetMenu(menuToGet) : nullptr;
        RE::GFxMovieView *view = menu ? menu->uiMovie.get() : nullptr;
        if (!view)
            return nullptr;

        RE::GFxValue Menu_mc;
        bool menuObtained = view->GetVariable(
            &Menu_mc,
            openedMenuName == "LootMenu"  ? "_root.lootMenu"
            : openedMenuName == "HUDMenu" ? "_root"
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
        RE::GFxValue xNumber;
        double xOffset = openedMenuName == "LootMenu" ? CEGlobals::QLIE_X_ORIGIN : (openedMenuName == "HUDMenu" ? CEGlobals::HUD_X_ORIGIN : CEGlobals::MENU_X_ORIGIN);
        xNumber.SetNumber(xOffset);
        if (!ceMenu.SetMember("_x", xNumber))
            return;

        RE::GFxValue yNumber;
        double yOffset = openedMenuName == "LootMenu" ? CEGlobals::QLIE_Y_ORIGIN : (openedMenuName == "HUDMenu" ? CEGlobals::HUD_Y_ORIGIN : CEGlobals::MENU_Y_ORIGIN);
        yNumber.SetNumber(yOffset);
        if (!ceMenu.SetMember("_y", yNumber))
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

    void DestroyMenu()
    {
        SKSE::GetTaskInterface()->AddUITask([]()
                                            {
        logger::debug("Destroying Menu");
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
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

        RE::GFxValue ceMenu = GetCEMenu(Menu_mc);
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;

        RE::GFxValue args2[1];
        args2[0].SetString(SWF_PATH); // name
        if (!ceMenu.Invoke("loadMovie", nullptr, args2, 1))
            return;
        logger::trace("Loaded {} via invoke", args2[0].GetString());
        CEActorUtils::GetActiveFollowers();
        CEMenu::ShowOrHideQLIEHint(); });
    }
}