
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

    bool qliePersistentToggledOn = false;
    bool menuPersistentToggledOn = false;
    bool qliePersistentToggledOnce = false;
    bool menuPersistentToggledOnce = false;

    void UpdateMenuName()
    {
        std::string stringMenuName{openedMenuName};
        std::string key = std::to_string(CEGlobals::COMPARE_KEY);
        if (CEGlobals::lastInputDevice == RE::INPUT_DEVICE::kGamepad)
        {
            if (openedMenuName == "LootMenu")
                key = std::to_string(CEGlobals::ConvertSKSEKeyToSkyrimKey(CEGlobals::CONTROLLER_KEY));
            else
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
            if (CEGameMenuUtils::GetItem() && CEGlobals::HIDE_3D)
            {
                auto manager = RE::Inventory3DManager::GetSingleton();
                manager->UnloadInventoryItem();
            }
        }
        else if (openedMenuName == "LootMenu")
        {
            std::thread([]()
                        {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                SKSE::GetTaskInterface()->AddUITask([]()
                                                    {CEGameMenuUtils::GetArmorOrWeapon(CEGameMenuUtils::currentFormID); }); })
                .detach();
        }
    }

    class persistentDisplayAS : public RE::GFxFunctionHandler
    {
    public:
        void Call(Params &) override
        {
            PersistentDisplayRun();
        }
    };

    class isWeaponOrArmorAS : public RE::GFxFunctionHandler
    {
    public:
        void Call(Params &a_params) override
        {
            a_params.retVal->SetBoolean(CEGameMenuUtils::isWeaponOrArmor(CEGameMenuUtils::currentFormID));
        }
    };

    class loggerAS : public RE::GFxFunctionHandler
    {
    public:
        void Call(Params &a_params) override
        {
            logger::info("AS: {}", a_params.args[0].GetString());
        }
    };

    // REGISTER_FN taken from Inventory Injector
    bool RegisterFuncs(RE::GFxMovieView *a_view, RE::GFxValue *a_root)
    {
        REGISTER_FN(persistentDisplayAS);
        REGISTER_FN(isWeaponOrArmorAS);
        REGISTER_FN(loggerAS);
        return true;
    }

    void ShowOrHideQLIEHint()
    {
        logger::debug("ShowOrHideQLIEHint called");
        std::thread([]()
                    {
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
        SKSE::GetTaskInterface()->AddUITask([]()
                                            {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc("LootMenu"));
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;
        ceMenu.Invoke("showOrHideQLIEHint"); }); })
            .detach();
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
        // ceMenu.Invoke("getVisible", &result);
        RE::GFxValue __this;
        ceMenu.GetMember("__this", &__this);
        if (__this.IsObject())
            __this.GetMember("_visible", &result);
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

    void HideSkyUiItemCard(RE::GFxValue ceMenu)
    {
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;
        ceMenu.Invoke("hideSkyUiItemCardAndMore");
    }

    void ShowMenuDelayed()
    {
        SKSE::GetTaskInterface()->AddTask([]()
                                          { std::thread([]()
                                                        {   
                                                            std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
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
        Menu_mc.SetMember("ce_qlieHintText", CEGlobals::QLIE_HINT_TEXT.c_str());
        Menu_mc.SetMember("ce_comparingTo", CEGlobals::comparingTo.c_str());
        Menu_mc.SetMember("ce_damageLabelText", CEGlobals::damageLabelText.c_str());
        Menu_mc.SetMember("ce_critLabelText", CEGlobals::critLabelText.c_str());
        Menu_mc.SetMember("ce_noneText", CEGlobals::noneText.c_str());
        Menu_mc.SetMember("ce_armorTypeLabelText", CEGlobals::armorTypeLabelText.c_str());
        Menu_mc.SetMember("ce_armorRatingLabelText", CEGlobals::armorRatingLabelText.c_str());
        Menu_mc.SetMember("ce_goldLabelText", CEGlobals::goldLabelText.c_str());
        Menu_mc.SetMember("ce_effectsLabelText", CEGlobals::effectsLabelText.c_str());
        Menu_mc.SetMember("ce_equippedTo", CEGlobals::equippedTo.c_str());
        Menu_mc.SetMember("ce_slotsLabelText", CEGlobals::slotsLabelText.c_str());
        Menu_mc.SetMember("ce_speedLabelText", CEGlobals::speedLabelText.c_str());
        Menu_mc.SetMember("ce_reachLabelText", CEGlobals::reachLabelText.c_str());
        Menu_mc.SetMember("ce_staggerLabelText", CEGlobals::staggerLabelText.c_str());
        Menu_mc.SetMember("ce_keyText", CEGlobals::keyText.c_str());
        Menu_mc.SetMember("ce_keyInfo", CEGlobals::keyInfo.c_str());
        Menu_mc.SetMember("ce_totalKey", CEGlobals::totalKey.c_str());
        Menu_mc.SetMember("ce_averageKey", CEGlobals::averageKey.c_str());
        Menu_mc.SetMember("ce_maxKey", CEGlobals::maxKey.c_str());
        Menu_mc.SetMember("ce_notApplicable", CEGlobals::notApplicable.c_str());
        Menu_mc.SetMember("ce_betterWrapperStart", CEGlobals::betterWrapperStart.c_str());
        Menu_mc.SetMember("ce_betterWrapperEnd", CEGlobals::betterWrapperEnd.c_str());
        Menu_mc.SetMember("ce_worseWrapperStart", CEGlobals::worseWrapperStart.c_str());
        Menu_mc.SetMember("ce_worseWrapperEnd", CEGlobals::worseWrapperEnd.c_str());
        Menu_mc.SetMember("ce_normalWrapperStart", CEGlobals::normalWrapperStart.c_str());
        Menu_mc.SetMember("ce_normalWrapperEnd", CEGlobals::normalWrapperEnd.c_str());
        Menu_mc.SetMember("ce_diffOffset", CEGlobals::diffOffset);
        Menu_mc.SetMember("ce_valueOffset", CEGlobals::valueOffset);
        Menu_mc.SetMember("ce_columnTwoOffset", CEGlobals::columnTwoOffset);
        Menu_mc.SetMember("ce_SPACING_FROM_SELECTED", CEGlobals::SPACING_FROM_SELECTED);
        Menu_mc.SetMember("ce_SPACING_BETWEEN_EQUIPPED_X", CEGlobals::SPACING_BETWEEN_EQUIPPED_X);
        Menu_mc.SetMember("ce_SPACING_BETWEEN_EQUIPPED_Y", CEGlobals::SPACING_BETWEEN_EQUIPPED_Y);
        Menu_mc.SetMember("ce_SCROLL_SPEED", CEGlobals::SCROLL_SPEED);
        Menu_mc.SetMember("ce_SCROLL_DELAY", CEGlobals::SCROLL_DELAY);
        Menu_mc.SetMember("ce_maxRows", CEGlobals::ROWS);
        int scale = CEMenu::openedMenuName == "LootMenu" ? CEGlobals::QLIE_SCALE : (CEMenu::openedMenuName == "HUDMenu" ? CEGlobals::HUD_SCALE : CEGlobals::MENU_SCALE);
        Menu_mc.SetMember("ce_scale", scale);
        int alpha = CEMenu::openedMenuName == "LootMenu" ? CEGlobals::QLIE_BACKGROUND_ALPHA : (CEMenu::openedMenuName == "HUDMenu" ? CEGlobals::HUD_BACKGROUND_ALPHA : CEGlobals::MENU_BACKGROUND_ALPHA);
        Menu_mc.SetMember("ce_alpha", alpha);
        int layout = CEMenu::openedMenuName == "LootMenu" ? CEGlobals::QLIE_LAYOUT : CEMenu::openedMenuName == "HUDMenu" ? CEGlobals::HUD_LAYOUT : CEGlobals::MENU_LAYOUT;
        Menu_mc.SetMember("ce_layout", layout);

        RE::GFxValue ceMenu = GetCEMenu(Menu_mc);
        if (ceMenu.IsNull() || ceMenu.IsUndefined() || !ceMenu.IsObject())
            return;

        RE::GFxValue args2[1];
        args2[0].SetString(SWF_PATH); // name
        if (!ceMenu.Invoke("loadMovie", nullptr, args2, 1))
            return;
        logger::trace("Loaded {} via invoke", args2[0].GetString());
        CEActorUtils::GetActiveFollowers();
        if (menuName == "LootMenu"){
            if (!qliePersistentToggledOnce){
                qliePersistentToggledOn = CEGlobals::QLIE_PERSISTENT_DEFAULT_DISPLAY;
                qliePersistentToggledOnce = true;
            }
            ShowOrHideQLIEHint();
            if (CEGlobals::QLIE_PERSISTENT_DISPLAY && CEGameMenuUtils::isWeaponOrArmor(CEGameMenuUtils::currentFormID))
                PersistentDisplayRun(true);
        }
        else if (menuName != "HUDMenu"){
            if (!menuPersistentToggledOnce){
                menuPersistentToggledOn = CEGlobals::MENU_PERSISTENT_DEFAULT_DISPLAY;
                menuPersistentToggledOnce = true;
            }
        } });
    }
}