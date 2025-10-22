#pragma once

namespace CEGameMenuUtils
{
    extern RE::FormID currentFormID;
    extern RE::TESObjectREFR::InventoryItemMap containerInventoryQLIE;

    bool GetItem();

    void ActorChangedUpdateMenu();
}