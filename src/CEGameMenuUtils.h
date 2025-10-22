#pragma once

namespace CEGameMenuUtils
{
    extern RE::FormID currentFormID;
    extern RE::TESObjectREFR::InventoryItemMap containerInventoryQLIE;

    bool GetItem();

    void ActorChangedUpdateMenu();

    bool isWeaponOrArmor(RE::FormID formId);

    bool GetArmorOrWeapon(RE::FormID formId);
}