#pragma once

namespace CEMenu
{
    extern const char *MENU_NAME;
    extern std::string_view SWF_PATH;
    extern std::string_view openedMenuName;
    extern std::set<std::string> openedMenus;

    RE::GFxValue GetMenu_mc(std::string_view nameOfMenuToGet = openedMenuName);

    RE::GFxValue GetCEMenu(RE::GFxValue Menu_mc);

    void UpdateMenuName();

    void SetMenuOffsets(RE::GFxValue ceMenu);

    void SetTranslations();

    bool IsMenuVisible();

    void ShowOrHideQLIEHint(bool forceDelete = false);

    void ShowMenuInstant();

    void ShowMenuDelayed();

    void HideMenu(bool checked = false);

    void CreateArmorComparisonItemCard(std::array<RE::GFxValue, CEGlobals::EQUIPPED_ARMOR_ITEM_ARRAY_SIZE> itemInfo, RE::GFxValue ceMenu);

    void CreateWeaponComparisonItemCard(std::array<RE::GFxValue, CEGlobals::EQUIPPED_WEAPON_ITEM_ARRAY_SIZE> itemInfo, RE::GFxValue ceMenu);

    void ResetMenu();

    void SetActor(std::string actorName);

    void CreateSelectedArmorItemCard(std::array<RE::GFxValue, CEGlobals::SELECTED_ARMOR_ITEM_ARRAY_SIZE> itemInfo, RE::GFxValue ceMenu);

    void CreateSelectedWeaponItemCard(std::array<RE::GFxValue, CEGlobals::SELECTED_WEAPON_ITEM_ARRAY_SIZE> itemInfo, RE::GFxValue ceMenu);

    void DestroyMenu(std::string menuToDestroy);

    void CreateMenu(std::string_view menuName);
}