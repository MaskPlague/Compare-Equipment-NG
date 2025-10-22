
#define REGISTER_FN(fn)                                \
    static auto handler_##fn = new fn();               \
    RE::GFxValue value_##fn;                           \
    a_view->CreateFunction(&value_##fn, handler_##fn); \
    a_root->SetMember(#fn, value_##fn)

namespace CEMenu
{
    std::string temp = "CompareEquipmentMenu_" + CEGlobals::EXPECTED_SWF_VERSION + "_" + std::to_string(CEGlobals::COMPARE_KEY);
    const char *MENU_NAME = temp.c_str();
    std::string_view SWF_PATH{"CompareEquipment_script.swf"};
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

    bool Is3dZoomedIn()
    {
        auto manager = RE::Inventory3DManager::GetSingleton();
        if (manager)
        {
            auto zoom = manager->GetRuntimeData().zoomProgress;
            if (zoom != 0.0f)
            {
                return true;
            }
        }
        return false;
    }

    void PersistentDisplayRun(bool QLIE)
    {
        logger::debug("PersistentDisplayRun called");
        if (Is3dZoomedIn())
            return;
        if (openedMenuName == "LootMenu" && !qliePersistentToggledOn)
            return;
        if (openedMenuName != "LootMenu" && openedMenuName != "HUDMenu" && !menuPersistentToggledOn)
            return;
        if (openedMenuName == "LootMenu" && !CEGlobals::QLIE_PERSISTENT_DISPLAY)
            return;
        if (openedMenuName != "LootMenu" && openedMenuName != "HUDMenu" && !CEGlobals::MENU_PERSISTENT_DISPLAY)
            return;
        if (!QLIE && openedMenuName != "HUDMenu")
        {
            logger::debug("Persitent display triggered for menu: {}", openedMenuName);
            if (CEGameMenuUtils::GetItem() && CEGlobals::HIDE_3D)
            {
                auto manager = RE::Inventory3DManager::GetSingleton();
                manager->Clear3D();
            }
        }
        else if (openedMenuName == "LootMenu")
        {
            std::thread([]()
                        {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                SKSE::GetTaskInterface()->AddUITask([]()
                                                    {   logger::debug("Persistent display triggered for LootMenu");
                                                        CEGameMenuUtils::GetArmorOrWeapon(CEGameMenuUtils::currentFormID); }); })
                .detach();
        }
    }

    class PersistentDisplay : public RE::GFxFunctionHandler
    {
    public:
        void Call(Params &) override
        {
            logger::debug("calling persistent display from AS");
            PersistentDisplayRun();
        }
    };

    class ASIsWeaponOrArmor : public RE::GFxFunctionHandler
    {
    public:
        void Call(Params &a_params) override
        {
            a_params.retVal->SetBoolean(CEGameMenuUtils::isWeaponOrArmor(CEGameMenuUtils::currentFormID));
        }
    };

    // REGISTER_FN taken from Inventory Injector
    bool RegisterFuncs(RE::GFxMovieView *a_view, RE::GFxValue *a_root)
    {
        REGISTER_FN(PersistentDisplay);
        REGISTER_FN(ASIsWeaponOrArmor);
        return true;
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
        double xOffset = 0.0;
        double yOffset = 0.0;
        if (openedMenuName == "LootMenu")
        {
            xOffset = CEGlobals::QLIE_X_ORIGIN;
            yOffset = CEGlobals::QLIE_Y_ORIGIN;
        }
        else if (openedMenuName == "HUDMenu")
        {
            xOffset = CEGlobals::HUD_X_ORIGIN;
            yOffset = CEGlobals::HUD_Y_ORIGIN;
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
        RE::GFxValue args[2];
        args[0].SetNumber(xOffset);
        args[1].SetNumber(yOffset);
        ceMenu.Invoke("SetXYOffsets", nullptr, args, 2);
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

    void HideSkyUiItemCard(RE::GFxValue menu_mc)
    {
        if (menu_mc.IsNull() || menu_mc.IsUndefined() || !menu_mc.IsObject())
            return;
        RE::GFxValue itemCard;
        if (menu_mc.GetMember("itemCard", &itemCard) && itemCard.IsObject())
        {
            itemCard.SetMember("_visible", false);
            itemCard.SetMember("ce_hidden", true);
        }
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
        RE::GFxValue menu_mc = GetMenu_mc();
        RE::GFxValue ceMenu = GetCEMenu(menu_mc);
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;
        logger::trace("Hiding menu.");
        ceMenu.Invoke("hideAndReset"); });
    }

    void SetSwfVariables()
    {
        std::thread([]()
                    {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            SKSE::GetTaskInterface()->AddUITask([]()
                                                {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;
        logger::trace("Setting SWF variables.");
        std::array<RE::GFxValue, 33> variableInfo;
        variableInfo[0].SetString(CEGlobals::buttonCompareText);
        variableInfo[1].SetString(CEGlobals::comparingTo);
        variableInfo[2].SetString(CEGlobals::damageLabelText);
        variableInfo[3].SetString(CEGlobals::critLabelText);
        variableInfo[4].SetString(CEGlobals::noneText);
        variableInfo[5].SetString(CEGlobals::armorTypeLabelText);
        variableInfo[6].SetString(CEGlobals::armorRatingLabelText);
        variableInfo[7].SetString(CEGlobals::goldLabelText);
        variableInfo[8].SetString(CEGlobals::effectsLabelText);
        variableInfo[9].SetString(CEGlobals::equippedTo);
        variableInfo[10].SetString(CEGlobals::slotsLabelText);
        variableInfo[11].SetString(CEGlobals::speedLabelText);
        variableInfo[12].SetString(CEGlobals::reachLabelText);
        variableInfo[13].SetString(CEGlobals::staggerLabelText);
        variableInfo[14].SetString(CEGlobals::keyText);
        variableInfo[15].SetString(CEGlobals::keyInfo);
        variableInfo[16].SetString(CEGlobals::totalKey);
        variableInfo[17].SetString(CEGlobals::averageKey);
        variableInfo[18].SetString(CEGlobals::maxKey);
        variableInfo[19].SetString(CEGlobals::notApplicable);
        variableInfo[20].SetString(CEGlobals::betterWrapperStart);
        variableInfo[21].SetString(CEGlobals::betterWrapperEnd);
        variableInfo[22].SetString(CEGlobals::worseWrapperStart);
        variableInfo[23].SetString(CEGlobals::worseWrapperEnd);
        variableInfo[24].SetString(CEGlobals::normalWrapperStart);
        variableInfo[25].SetString(CEGlobals::normalWrapperEnd);
        variableInfo[26].SetNumber(CEGlobals::diffOffset);
        variableInfo[27].SetNumber(CEGlobals::valueOffset);
        variableInfo[28].SetNumber(CEGlobals::columnTwoOffset);
        variableInfo[29].SetNumber(CEGlobals::SPACING_FROM_SELECTED);
        variableInfo[30].SetNumber(CEGlobals::SPACING_BETWEEN_EQUIPPED_X);
        variableInfo[31].SetNumber(CEGlobals::SPACING_BETWEEN_EQUIPPED_Y);
        int layout = CEMenu::openedMenuName == "LootMenu" ? CEGlobals::QLIE_LAYOUT
                    : CEMenu::openedMenuName == "HUDMenu" ? CEGlobals::HUD_LAYOUT
                                                          : CEGlobals::MENU_LAYOUT;
        variableInfo[32].SetNumber(layout);
        ceMenu.Invoke("setVariables", nullptr, variableInfo);
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

        Menu_mc.SetMember("ce_buttonCompareText", CEGlobals::buttonCompareText.c_str());

        RE::GFxValue ceMenu = GetCEMenu(Menu_mc);
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;

        RE::GFxValue args2[1];
        args2[0].SetString(SWF_PATH); // name
        if (!ceMenu.Invoke("loadMovie", nullptr, args2, 1))
            return;
        logger::trace("Loaded {} via invoke", args2[0].GetString());
        CEActorUtils::GetActiveFollowers();
        SetSwfVariables();
        if (menuName == "LootMenu")
            CEMenu::ShowOrHideQLIEHint(); });
    }
}