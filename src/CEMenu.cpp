namespace logger = SKSE::log;

namespace CEMenu
{
    std::string temp = "CompareEquipmentMenu_" + CEGlobals::EXPECTED_SWF_VERSION;
    const char *MENU_NAME = temp.c_str();
    std::string_view SWF_PATH{"CompareEquipment.swf"};
    std::string_view openedMenuName = RE::InventoryMenu::MENU_NAME;

    RE::GFxValue GetMenu_mc()
    {
        logger::trace("Getting ui singleton");
        auto UISingleton = RE::UI::GetSingleton();
        logger::trace("inventoryMenu");
        auto inventoryMenu = UISingleton ? UISingleton->GetMenu(openedMenuName) : nullptr;
        logger::trace("getting view");
        RE::GFxMovieView *view = inventoryMenu ? inventoryMenu->uiMovie.get() : nullptr;
        RE::GFxValue Menu_mc;
        logger::trace("getting menu_mc");
        if (!UISingleton || !inventoryMenu || !view || !view->GetVariable(&Menu_mc, "_root.Menu_mc"))
        {
            return nullptr;
        }
        logger::trace("got menu_mc");
        return Menu_mc;
    }

    RE::GFxValue GetCEMenu(RE::GFxValue Menu_mc)
    {
        RE::GFxValue ceMenu;
        logger::trace("getting ceMenu");
        if (Menu_mc.IsNull() || !Menu_mc.GetMember(MENU_NAME, &ceMenu))
        {
            return nullptr;
        }
        logger::trace("Got ceMenu");
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

    void ShowMenuInstant()
    {
        if (IsMenuVisible())
            return;
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;
        logger::trace("Showing menu.");
        ceMenu.Invoke("showMenu");
    }

    void ShowMenuDelayed()
    {
        logger::trace("ShowMenuDelayed");
        SKSE::GetTaskInterface()->AddTask([]()
                                          { std::thread([]()
                                                        {   
                                                            std::this_thread::sleep_for(std::chrono::milliseconds(50)); 
                                                            ShowMenuInstant(); })
                                                .detach(); });
    }

    void CreateComparisonItemCard(std::array<RE::GFxValue, CEGlobals::EQUIPPED_ITEM_ARRAY_SIZE> itemInfo, RE::GFxValue ceMenu)
    {
        logger::trace("Checking if ceMenu is null");
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;
        logger::trace("ceMenu exists, creating comparison item card");
        ceMenu.Invoke("createComparisonItemCard", nullptr, itemInfo);
    }

    void ResetMenu()
    {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;
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

    void CreateSelectedItemCard(std::array<RE::GFxValue, CEGlobals::SELECTED_ITEM_ARRAY_SIZE> itemInfo)
    {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;
        ceMenu.Invoke("populateSelectedItemCard", nullptr, itemInfo);
    }

    void DestroyMenu()
    {
        logger::debug("Destroying Menu");
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;

        if (!ceMenu.Invoke("removeMovieClip"))
            return;

        logger::debug("Removed {}", MENU_NAME);
    }

    void CreateMenu(std::string_view menuName)
    {
        openedMenuName = menuName;
        logger::debug("Creating Menu");
        RE::GFxValue Menu_mc = GetMenu_mc();
        if (Menu_mc.IsNull() || Menu_mc.IsUndefined() || !Menu_mc.IsObject())
            return;

        RE::GFxValue _ceMenu = GetCEMenu(Menu_mc);
        if (!_ceMenu.IsNull())
            return;

        RE::GFxValue args[2];
        args[0].SetString(MENU_NAME); // name
        args[1] = 3999;               // depth
        if (!Menu_mc.Invoke("createEmptyMovieClip", nullptr, args, 2))
            return;

        logger::debug("Created {} movie clip via invoke", MENU_NAME);

        RE::GFxValue ceMenu = GetCEMenu(Menu_mc);
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;

        logger::debug("Got {}", MENU_NAME);

        RE::GFxValue args2[1];
        args2[0].SetString(SWF_PATH); // name
        if (!ceMenu.Invoke("loadMovie", nullptr, args2, 1))
            return;
        logger::debug("Loaded {} via invoke", args2[0].GetString());

        RE::GFxValue xNumber;
        xNumber.SetNumber(CEGlobals::X_ORIGIN);
        if (!ceMenu.SetMember("_x", xNumber))
            return;

        RE::GFxValue yNumber;
        yNumber.SetNumber(CEGlobals::Y_ORIGIN);
        if (!ceMenu.SetMember("_y", yNumber))
            return;

        CEActorUtils::GetActiveFollowers();
    }
}