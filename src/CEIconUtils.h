#pragma once

namespace CEIconUtils
{
    // std::string GetArmorIconLabel(RE::TESObjectARMO *armor);

    RE::GFxValue GetSelectedEntryObject();

    RE::GFxValue GetEquippedEntryObject(RE::FormID equippedFormID);
}