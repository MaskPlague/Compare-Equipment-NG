namespace CEIconUtils
{
    int GetArmorTypeEnum(RE::BGSBipedObjectForm::ArmorType type)
    {
        switch (type)
        {
        case RE::BGSBipedObjectForm::ArmorType::kClothing:
            return 3;
        case RE::BGSBipedObjectForm::ArmorType::kHeavyArmor:
            return 1;
        case RE::BGSBipedObjectForm::ArmorType::kLightArmor:
            return 0;
        default:
            return 2;
        }
    }

    enum class EquipType
    {
        Head = 0,
        Hair = 1,
        LongHair = 2,
        Body = 3,
        Forearms = 4,
        Hands = 5,
        Shield = 6,
        Calves = 7,
        Feet = 8,
        Circlet = 9,
        Amulet = 10,
        Ears = 11,
        Ring = 12,
        Tail = 13,
    };

    inline static std::vector<RE::BIPED_MODEL::BipedObjectSlot> PartMaskPrecedence = {
        RE::BIPED_MODEL::BipedObjectSlot::kBody,
        RE::BIPED_MODEL::BipedObjectSlot::kHair,
        RE::BIPED_MODEL::BipedObjectSlot::kHands,
        RE::BIPED_MODEL::BipedObjectSlot::kForearms,
        RE::BIPED_MODEL::BipedObjectSlot::kFeet,
        RE::BIPED_MODEL::BipedObjectSlot::kCalves,
        RE::BIPED_MODEL::BipedObjectSlot::kShield,
        RE::BIPED_MODEL::BipedObjectSlot::kAmulet,
        RE::BIPED_MODEL::BipedObjectSlot::kRing,
        RE::BIPED_MODEL::BipedObjectSlot::kLongHair,
        RE::BIPED_MODEL::BipedObjectSlot::kEars,
        RE::BIPED_MODEL::BipedObjectSlot::kHead,
        RE::BIPED_MODEL::BipedObjectSlot::kCirclet,
        RE::BIPED_MODEL::BipedObjectSlot::kTail,
        static_cast<RE::BIPED_MODEL::BipedObjectSlot>(1 << 14),
        static_cast<RE::BIPED_MODEL::BipedObjectSlot>(1 << 15),
        static_cast<RE::BIPED_MODEL::BipedObjectSlot>(1 << 16),
        static_cast<RE::BIPED_MODEL::BipedObjectSlot>(1 << 17),
        static_cast<RE::BIPED_MODEL::BipedObjectSlot>(1 << 18),
        static_cast<RE::BIPED_MODEL::BipedObjectSlot>(1 << 19),
        RE::BIPED_MODEL::BipedObjectSlot::kDecapitateHead,
        RE::BIPED_MODEL::BipedObjectSlot::kDecapitate,
        static_cast<RE::BIPED_MODEL::BipedObjectSlot>(1 << 22),
        static_cast<RE::BIPED_MODEL::BipedObjectSlot>(1 << 23),
        static_cast<RE::BIPED_MODEL::BipedObjectSlot>(1 << 24),
        static_cast<RE::BIPED_MODEL::BipedObjectSlot>(1 << 25),
        static_cast<RE::BIPED_MODEL::BipedObjectSlot>(1 << 26),
        static_cast<RE::BIPED_MODEL::BipedObjectSlot>(1 << 27),
        static_cast<RE::BIPED_MODEL::BipedObjectSlot>(1 << 28),
        static_cast<RE::BIPED_MODEL::BipedObjectSlot>(1 << 29),
        static_cast<RE::BIPED_MODEL::BipedObjectSlot>(1 << 30),
        RE::BIPED_MODEL::BipedObjectSlot::kFX01,
    };

    inline EquipType GetEquipTypeFromSlot(RE::BIPED_MODEL::BipedObjectSlot slot)
    {
        using Slot = RE::BIPED_MODEL::BipedObjectSlot;

        switch (slot)
        {
        case Slot::kHead:
            return EquipType::Head;
        case Slot::kHair:
            return EquipType::Hair;
        case Slot::kLongHair:
            return EquipType::LongHair;
        case Slot::kBody:
            return EquipType::Body;
        case Slot::kForearms:
            return EquipType::Forearms;
        case Slot::kHands:
            return EquipType::Hands;
        case Slot::kShield:
            return EquipType::Shield;
        case Slot::kCalves:
            return EquipType::Calves;
        case Slot::kFeet:
            return EquipType::Feet;
        case Slot::kCirclet:
            return EquipType::Circlet;
        case Slot::kAmulet:
            return EquipType::Amulet;
        case Slot::kEars:
            return EquipType::Ears;
        case Slot::kRing:
            return EquipType::Ring;
        case Slot::kTail:
            return EquipType::Tail;
        default:
            return static_cast<EquipType>(-1);
        }
    }

    std::string GetArmorIconLabel(RE::TESObjectARMO *armor)
    {
        if (!armor)
            return "default_misc";

        int typeEnum = GetArmorTypeEnum(armor->GetArmorType());

        std::string prefix;
        switch (typeEnum)
        {
        case 0:
            prefix = "lightarmor_";
            break;
        case 1:
            prefix = "armor_";
            break;
        case 3:
            prefix = "clothing_";
            break;
        default:
            prefix = "default_";
            break;
        }

        RE::BIPED_MODEL::BipedObjectSlot mainSlot = RE::BIPED_MODEL::BipedObjectSlot::kNone;
        for (auto slot : PartMaskPrecedence)
        {
            if ((armor->bipedModelData.bipedObjectSlots & slot) == slot)
            {
                mainSlot = slot;
                break;
            }
        }

        EquipType equip = GetEquipTypeFromSlot(mainSlot);

        std::string suffix;
        switch (equip)
        {
        case EquipType::Head:
            suffix = "head";
            break;
        case EquipType::Body:
            suffix = "body";
            break;
        case EquipType::Forearms:
            suffix = "forearms";
            break;
        case EquipType::Hands:
            suffix = "hands";
            break;
        case EquipType::Shield:
            suffix = "shield";
            break;
        case EquipType::Calves:
            suffix = "calves";
            break;
        case EquipType::Feet:
            suffix = "feet";
            break;
        case EquipType::Circlet:
            suffix = "circlet";
            break;
        case EquipType::Amulet:
            suffix = "amulet";
            break;
        case EquipType::Ring:
            suffix = "ring";
            break;
        case EquipType::Ears:
            suffix = "ears";
            break;
        case EquipType::Tail:
            suffix = "tail";
            break;
        default:
            suffix = "misc";
            break;
        }

        std::string iconLabel = prefix + suffix;
        // logger::info("armor: {}, iconLabel: {}", armor->GetName(), iconLabel);
        return iconLabel;
    }

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