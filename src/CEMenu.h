#pragma once

namespace CEMenu
{
    extern const char *MENU_NAME;
    extern std::string_view SWF_PATH;
    extern std::string_view openedMenuName;
    extern std::set<std::string> openedMenus;
    extern bool qliePersistentToggledOn;
    extern bool menuPersistentToggledOn;
    RE::GFxValue GetMenu_mc(std::string_view nameOfMenuToGet = openedMenuName);

    RE::GFxValue GetCEMenu(RE::GFxValue Menu_mc);

    void UpdateMenuName();

    void SetMenuOffsets(RE::GFxValue ceMenu);

    bool Is3dZoomedIn();

    bool IsMenuVisible();

    void ShowOrHideQLIEHint();

    void ShowMenuInstant();

    void ShowMenuDelayed();

    void HideSkyUiItemCard(RE::GFxValue ceMenu);

    void HideMenu(bool checked = false);

    void CreateArmorComparisonItemCard(std::array<RE::GFxValue, CEGlobals::EQUIPPED_ARMOR_ITEM_ARRAY_SIZE> itemInfo, RE::GFxValue ceMenu);

    void CreateWeaponComparisonItemCard(std::array<RE::GFxValue, CEGlobals::EQUIPPED_WEAPON_ITEM_ARRAY_SIZE> itemInfo, RE::GFxValue ceMenu);

    void ResetMenu();

    void SetActor(std::string actorName);

    void CreateSelectedArmorItemCard(std::array<RE::GFxValue, CEGlobals::SELECTED_ARMOR_ITEM_ARRAY_SIZE> itemInfo, RE::GFxValue ceMenu);

    void CreateSelectedWeaponItemCard(std::array<RE::GFxValue, CEGlobals::SELECTED_WEAPON_ITEM_ARRAY_SIZE> itemInfo, RE::GFxValue ceMenu);

    void DestroyMenu(std::string_view menuToDestroy);

    void CreateMenu(std::string_view menuName);

    void PersistentDisplayRun(bool QLIE = false);

    bool RegisterFuncs(RE::GFxMovieView *a_view, RE::GFxValue *a_root);
}