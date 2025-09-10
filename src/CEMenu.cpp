namespace logger = SKSE::log;

namespace CEMenu
{
    const char *MENU_NAME = "CompareEquipmentMenu";
    std::string_view SWF_PATH{"CompareEquipment.swf"};
    std::string_view openedMenuName = RE::InventoryMenu::MENU_NAME;

    RE::GFxValue GetMenu_mc()
    {
        auto UISingleton = RE::UI::GetSingleton();
        auto inventoryMenu = UISingleton ? UISingleton->GetMenu(openedMenuName) : nullptr;
        RE::GFxMovieView *view = inventoryMenu ? inventoryMenu->uiMovie.get() : nullptr;
        RE::GFxValue Menu_mc;

        if (!UISingleton || !inventoryMenu || !view || !view->GetVariable(&Menu_mc, "_root.Menu_mc"))
        {
            return nullptr;
        }
        return Menu_mc;
    }

    RE::GFxValue GetCEMenu(RE::GFxValue Menu_mc)
    {
        RE::GFxValue ceMenu;
        if (Menu_mc.IsNull() || !Menu_mc.GetMember(MENU_NAME, &ceMenu))
        {
            return nullptr;
        }
        return ceMenu;
    }

    bool IsMenuVisible()
    {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
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
        if (ceMenu.IsNull())
            return;
        logger::trace("Showing menu.");
        ceMenu.Invoke("showMenu");
    }

    void ShowMenuDelayed()
    {
        std::thread([]()
                    {   
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); 
            ShowMenuInstant(); })
            .detach();
    }

    void SetBackgroundAlpha()
    {
        std::thread([]()
                    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
            return;
        logger::trace("Setting Background Alpha");
        RE::GFxValue args[1];
        args[0].SetNumber(CEGlobals::BACKGROUND_ALPHA);
        ceMenu.Invoke("setBackgroundAlpha", nullptr, args, 1); })
            .detach();
    }

    /*void HideMenu()
    {
        if (!IsMenuVisible() || CEGlobals::PERMANENT_OPEN)
            return;
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
            return;
        ceMenu.Invoke("hideMenu");
    }*/

    /*void ToggleMenu()
    void CreateComparisonItemCard(std::array<RE::GFxValue, CEGlobals::EQUIPPED_ITEM_ARRAY_SIZE> itemInfo, RE::GFxValue ceMenu)
    {
        logger::trace("Checking if ceMenu is null");
            return;
        logger::trace("ceMenu exists, creating comparison item card");
        ceMenu.Invoke("createComparisonItemCard", nullptr, itemInfo);
    }

    void ResetMenu()
    {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
            return;
        ceMenu.Invoke("reset");
    }

    void SetActor(std::string actorName)
    {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
            return;
        RE::GFxValue args[1];
        args[0].SetString(actorName);
        ceMenu.Invoke("setActor", nullptr, args, 1);
    }

    void CreateSelectedItemCard(std::array<RE::GFxValue, CEGlobals::SELECTED_ITEM_ARRAY_SIZE> itemInfo)
    {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
            return;
        ceMenu.Invoke("populateSelectedItemCard", nullptr, itemInfo);
    }

    void DestroyMenu()
    {
        logger::debug("Destroying Menu");
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
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
        if (Menu_mc.IsNull())
            return;

        RE::GFxValue _ceMenu = GetCEMenu(Menu_mc);
        if (!_ceMenu.IsNull())
            return;

        RE::GFxValue args[2];
        RE::GFxValue ceMenuMovieClip;
        args[0].SetString(MENU_NAME); // name
        args[1] = 3999;               // depth
        if (!Menu_mc.Invoke("createEmptyMovieClip", &ceMenuMovieClip, args, 2))
            return;

        logger::debug("Created {} movie clip via invoke", MENU_NAME);

        RE::GFxValue ceMenu = GetCEMenu(Menu_mc);
        if (ceMenu.IsNull())
            return;

        logger::debug("Got {}", MENU_NAME);

        RE::GFxValue result2;
        RE::GFxValue args2[1];
        args2[0].SetString(SWF_PATH); // name
        if (!ceMenu.Invoke("loadMovie", &result2, args2, 1))
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

        SetBackgroundAlpha();
        CEActorUtils::GetActiveFollowers();
    }
}