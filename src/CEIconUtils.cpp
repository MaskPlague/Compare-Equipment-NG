namespace CEIconUtils
{
    RE::GFxValue GetEquippedEntryObject(RE::FormID equippedFormID)
    {
        if (!CEGlobals::USE_ICONS)
            return nullptr;
        RE::GFxValue Menu_mc = CEMenu::GetMenu_mc();
        if (Menu_mc.IsNull() || Menu_mc.IsUndefined() || !Menu_mc.IsObject())
            return nullptr;
        RE::GFxValue inventoryLists;
        RE::GFxValue itemList;
        RE::GFxValue entryList;
        if (!Menu_mc.GetMember("inventoryLists", &inventoryLists) ||
            !inventoryLists.GetMember("itemList", &itemList) ||
            !itemList.GetMember("entryList", &entryList) ||
            !entryList.IsObject())
            return nullptr;

        RE::GFxValue entry;
        RE::GFxValue entryFormId;

        std::uint32_t count = entryList.GetArraySize();

        for (std::uint32_t i = 0; i < count; i++)
        {
            if (!entryList.GetElement(i, &entry) || !entry.IsObject())
                continue;
            if (!entry.GetMember("formId", &entryFormId))
                continue;
            RE::FormID formID = 0;
            if (entryFormId.IsNumber())
                formID = static_cast<RE::FormID>(entryFormId.GetUInt());
            if (formID == equippedFormID)
                return entry;
        }
        return nullptr;
    }

    RE::GFxValue GetSelectedEntryObject()
    {
        if (!CEGlobals::USE_ICONS)
            return nullptr;
        RE::GFxValue Menu_mc = CEMenu::GetMenu_mc();
        if (Menu_mc.IsNull() || Menu_mc.IsUndefined() || !Menu_mc.IsObject())
            return nullptr;
        if (CEMenu::openedMenuName != "LootMenu")
        {
            RE::GFxValue inventoryLists;
            RE::GFxValue itemList;
            RE::GFxValue selectedEntry;
            if (!Menu_mc.GetMember("inventoryLists", &inventoryLists) ||
                !inventoryLists.GetMember("itemList", &itemList) ||
                !itemList.GetMember("selectedEntry", &selectedEntry) ||
                !selectedEntry.IsObject())
                return nullptr;
            return selectedEntry;
        }
        else
        {
            RE::GFxValue itemList;
            RE::GFxValue dataProvider;
            if (!Menu_mc.GetMember("itemList", &itemList) ||
                !itemList.GetMember("dataProvider", &dataProvider) ||
                !dataProvider.IsObject() ||
                !dataProvider.IsArray())
                return nullptr;

            RE::GFxValue entry;
            RE::GFxValue entryFormId;

            std::uint32_t count = dataProvider.GetArraySize();
            for (std::uint32_t i = 0; i < count; i++)
            {
                if (!dataProvider.GetElement(i, &entry) || !entry.IsObject())
                    continue;
                if (!entry.GetMember("formId", &entryFormId))
                    continue;
                RE::FormID formID = 0;
                if (entryFormId.IsNumber())
                    formID = static_cast<RE::FormID>(entryFormId.GetUInt());
                if (formID == CEGameMenuUtils::currentFormID)
                    return entry;
            }
            return nullptr;
        }
    }
}