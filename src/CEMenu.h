#pragma once

namespace CEMenu
{
    extern const char *MENU_NAME;
    extern std::string_view SWF_PATH;
    extern std::string_view openedMenuName;

    RE::GFxValue GetMenu_mc();

    RE::GFxValue GetCEMenu(RE::GFxValue Menu_mc);

    void ShowMenuInstant();

    void ShowMenuDelayed();

    void CreateComparisonItemCard(std::array<RE::GFxValue, CEGlobals::EQUIPPED_ITEM_ARRAY_SIZE> itemInfo, RE::GFxValue ceMenu);

    void ResetMenu();

    void SetActor(std::string actorName);

    void CreateSelectedItemCard(std::array<RE::GFxValue, CEGlobals::SELECTED_ITEM_ARRAY_SIZE> itemInfo);

    void DestroyMenu();

    void CreateMenu(std::string_view menuName);
}