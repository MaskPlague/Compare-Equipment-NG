namespace logger = SKSE::log;

// TODO: Create INI
double X_ORIGIN = 500.0f;
double Y_ORIGIN = 300.0f;
int BACKGROUND_ALPHA = 95;
bool PERMANENT_OPEN = false; // TODO: this first
bool DEFAULT_OPEN = false;   // TODO: Remove
uint32_t COMPARE_KEY = 47;
uint32_t CYCLE_KEY = 48;
std::chrono::milliseconds HOLD_THRESHOLD(500);

const int EQUIPPED_ITEM_ARRAY_SIZE = 6;
const int SELECTED_ITEM_ARRAY_SIZE = 8;

namespace ActorUtils
{
    RE::Actor *currentActor;
    int index = 0;
    std::vector<RE::Actor *> followers;

    void GetActiveFollowers()
    {
        std::vector<RE::Actor *> result;

        auto processLists = RE::ProcessLists::GetSingleton();
        if (!processLists)
        {
            followers = result;
            return;
        }
        logger::debug("Got process lists");
        auto faction = RE::TESForm::LookupByID<RE::TESFaction>(0x0005C84E); // CurrentFollowerFaction
        if (!faction)
        {
            followers = result;
            return;
        }
        logger::debug("Got faction");
        for (auto &handle : processLists->highActorHandles)
        {
            if (auto actor = handle.get().get())
            {
                if (actor->IsInFaction(faction))
                {
                    result.push_back(actor);
                }
            }
        }
        followers = result;
        return;
    }

    void SetActorToPlayer()
    {
        currentActor = RE::PlayerCharacter::GetSingleton();
    }

    bool IsActorValid(RE::Actor *actor)
    {
        if (!actor->Is3DLoaded() || actor->IsDead() || actor->IsDeleted() || actor->IsDisabled())
        {
            return false;
        }
        return true;
    }

    void SetActorToNextFollower()
    {
        if (followers.size() == 0)
        {
            SetActorToPlayer();
            return;
        }
        if (index >= followers.size())
            index = 0;
        RE::Actor *potentialActor = followers.at(index);
        if (!IsActorValid(potentialActor))
        {
            followers.erase(followers.begin() + index);
            SetActorToNextFollower();
            return;
        }
        currentActor = potentialActor;
        index++;
    }
}

namespace CEMenu
{
    std::string basename = "CompareEquipmentMenu_";
    auto temp = basename + std::to_string(BACKGROUND_ALPHA) + "_" + std::to_string(DEFAULT_OPEN);
    auto MENU_NAME = temp.c_str();
    // constexpr auto MENU_NAME = "CompareEquipmentMenu";
    static constexpr std::string_view SWF_PATH{"CompareEquipment.swf"};
    auto openedMenuName = RE::InventoryMenu::MENU_NAME;

    RE::GFxValue GetMenu_mc()
    {
        auto UISingleton = RE::UI::GetSingleton();
        auto inventoryMenu = UISingleton ? UISingleton->GetMenu(openedMenuName) : nullptr;
        RE::GFxMovieView *view = inventoryMenu ? inventoryMenu->uiMovie.get() : nullptr;
        RE::GFxValue Menu_mc;

        if (!UISingleton || !inventoryMenu || !view || !view->GetVariable(&Menu_mc, "_root.Menu_mc"))
        {
            return nullptr;
        }
        return Menu_mc;
    }
    RE::GFxValue GetCEMenu(RE::GFxValue Menu_mc)
    {
        RE::GFxValue ceMenu;
        if (Menu_mc.IsNull() || !Menu_mc.GetMember(MENU_NAME, &ceMenu))
        {
            return nullptr;
        }
        return ceMenu;
    }

    bool IsMenuVisible()
    {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
            return false;
        RE::GFxValue result;
        ceMenu.Invoke("getVisible", &result);
        if (result.IsBool())
            return result.GetBool();
        return false;
    }
    void ShowMenu()
    {
        if (IsMenuVisible())
            return;
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
            return;
        ceMenu.Invoke("showMenu");
    }

    void HideMenu()
    {
        if (!IsMenuVisible())
            return;
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
            return;
        ceMenu.Invoke("hideMenu");
    }

    void ToggleMenu()
    {
        if (IsMenuVisible())
            HideMenu();
        else
            ShowMenu();
    }

    void CreateComparisonItemCards(std::vector<std::array<RE::GFxValue, EQUIPPED_ITEM_ARRAY_SIZE>> item_arr)
    {
        auto manager = RE::Inventory3DManager::GetSingleton();
        if (manager)
            manager->Clear3D();
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
            return;
        RE::GFxValue count[1];
        count[0].SetNumber(static_cast<double>(item_arr.size()));
        ceMenu.Invoke("setItemCardCount", nullptr, count, 1);
        for (auto itemInfo : item_arr)
        {
            ceMenu.Invoke("createComparisonItemCard", nullptr, itemInfo);
        }
    }

    void ResetMenu()
    {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
            return;
        ceMenu.Invoke("reset");
    }

    void SetActor(std::string actorName)
    {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
            return;
        RE::GFxValue args[1];
        args[0].SetString(actorName);
        ceMenu.Invoke("setActor", nullptr, args, 1);
    }

    void CreateSelectedItemCard(std::array<RE::GFxValue, SELECTED_ITEM_ARRAY_SIZE> itemInfo)
    {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
            return;
        ceMenu.Invoke("populateSelectedItemCard", nullptr, itemInfo);
    }

    void DestroyMenu()
    {
        logger::debug("Destroying Menu");
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
            return;

        if (!ceMenu.Invoke("removeMovieClip"))
            return;

        logger::debug("Removed {}", MENU_NAME);
    }
    void CreateMenu(std::string_view menuName)
    {
        openedMenuName = menuName;
        logger::debug("Creating Menu");
        RE::GFxValue Menu_mc = GetMenu_mc();
        if (Menu_mc.IsNull())
            return;

        RE::GFxValue _ceMenu = GetCEMenu(Menu_mc);
        if (!_ceMenu.IsNull())
            return;

        RE::GFxValue args[2];
        RE::GFxValue ceMenuMovieClip;
        args[0].SetString(MENU_NAME); // name
        args[1] = 9999;               // depth
        if (!Menu_mc.Invoke("createEmptyMovieClip", &ceMenuMovieClip, args, 2))
            return;

        logger::debug("Created {} movie clip via invoke", MENU_NAME);

        RE::GFxValue ceMenu = GetCEMenu(Menu_mc);
        if (ceMenu.IsNull())
            return;

        logger::debug("Got {}", MENU_NAME);

        RE::GFxValue result2;
        RE::GFxValue args2[1];
        args2[0].SetString(SWF_PATH); // name
        if (!ceMenu.Invoke("loadMovie", &result2, args2, 1))
            return;

        logger::debug("Loaded {} via invoke", args2[0].GetString());

        RE::GFxValue xNumber;
        xNumber.SetNumber(X_ORIGIN);
        if (!ceMenu.SetMember("_x", xNumber))
            return;

        RE::GFxValue yNumber;
        yNumber.SetNumber(Y_ORIGIN);
        if (!ceMenu.SetMember("_y", yNumber))
            return;

        ActorUtils::GetActiveFollowers();
    }
}

namespace InventoryMenu
{
    RE::FormID currentFormID;

    const char *GetArmorTypeString(RE::BGSBipedObjectForm::ArmorType type)
    {
        switch (type)
        {
        case RE::BGSBipedObjectForm::ArmorType::kClothing:
            return "Cloth";
        case RE::BGSBipedObjectForm::ArmorType::kHeavyArmor:
            return "Heavy";
        case RE::BGSBipedObjectForm::ArmorType::kLightArmor:
            return "Light";
        default:
            return "Unknown";
        }
    }

    std::string cleanPercentage(std::string str)
    {
        auto pos1 = str.find('<');
        if (pos1 != std::string::npos)
        {
            str.erase(pos1, 1);
        }
        auto pos2 = str.find('>');
        if (pos2 != std::string::npos)
        {
            str.erase(pos2, 1);
        }
        return str;
    }

    std::string GetEnchantmentString(RE::TESObjectARMO *armor, std::string description, float magnitude)
    {
        auto index = description.find("<mag>");
        if (index != std::string::npos)
            return description.substr(0, index) + std::format("{:.1f}", magnitude) + description.substr(index + 5) + "\n";
        else if (description == "")
        {
            RE::BSString str;
            armor->GetDescription(str, nullptr);
            return cleanPercentage(static_cast<std::string>(str)) + "\n";
        }
        else
            return cleanPercentage(description) + "\n";
    }

    std::string GetArmorDescription(RE::TESObjectARMO *armor, std::string text)
    {
        RE::BSString str;
        armor->GetDescription(str, nullptr);
        std::string description = static_cast<std::string>(str) + "\n";
        if (description.size() > 3)
            text = cleanPercentage(description);
        return text;
    }

    void GetEquippedInSlots(RE::FormID selectedFormId)
    {
        if (auto form = RE::TESForm::LookupByID(selectedFormId))
        {
            if (auto armor = form->As<RE::TESObjectARMO>())
            {
                auto biped = armor->bipedModelData;
                auto slots = biped.bipedObjectSlots;
                auto actor = ActorUtils::currentActor;
                if (!actor)
                {
                    ActorUtils::SetActorToNextFollower();
                    return;
                }
                auto player = RE::PlayerCharacter::GetSingleton();
                if (!actor)
                    return;

                using Slot = RE::BGSBipedObjectForm::BipedObjectSlot;

                std::vector<std::pair<Slot, const char *>> slotList = {
                    {Slot::kAmulet, "Amulet"},
                    {Slot::kBody, "Body"},
                    {Slot::kCalves, "Calves"},
                    {Slot::kCirclet, "Circlet"},
                    {Slot::kDecapitate, "Decapitate"},
                    {Slot::kDecapitateHead, "DecapitateHead"},
                    {Slot::kEars, "Ears"},
                    {Slot::kFeet, "Feet"},
                    {Slot::kForearms, "Forearms"},
                    {Slot::kFX01, "FX01"},
                    {Slot::kHair, "Hair"},
                    {Slot::kHands, "Hands"},
                    {Slot::kHead, "Head"},
                    {Slot::kLongHair, "LongHair"},
                    {Slot::kModArmLeft, "ModArmLeft"},
                    {Slot::kModArmRight, "ModArmRight"},
                    {Slot::kModBack, "ModBack"},
                    {Slot::kModChestPrimary, "ModChestPrimary"},
                    {Slot::kModChestSecondary, "ModChestSecondary"},
                    {Slot::kModFaceJewelry, "FaceJewelry"},
                    {Slot::kModLegLeft, "ModLegLeft"},
                    {Slot::kModLegRight, "ModLefRight"},
                    {Slot::kModMisc1, "ModMisc1"},
                    {Slot::kModMisc2, "ModMisc2"},
                    {Slot::kModMouth, "ModMouth"},
                    {Slot::kModNeck, "ModNeck"},
                    {Slot::kModPelvisPrimary, "ModPelvisPrimary"},
                    {Slot::kModPelvisSecondary, "ModPelvisSecondary"},
                    {Slot::kModShoulder, "ModShoulder"},
                    {Slot::kNone, "None"},
                    {Slot::kRing, "Ring"},
                    {Slot::kShield, "Shield"},
                    {Slot::kTail, "Tail"},
                };
                const auto &inv = actor->GetInventory();
                auto selectedName = armor->GetName();
                auto selectedType = GetArmorTypeString(armor->GetArmorType());
                auto selectedValue = armor->GetGoldValue();
                auto selectedEnchantment = armor->formEnchanting;
                std::string selectedEnchantmentInfo = "None";
                if (selectedEnchantment)
                {
                    selectedEnchantmentInfo = "";
                    auto effects = selectedEnchantment->effects;
                    for (auto effect : effects)
                    {
                        float magnitude = effect->GetMagnitude();
                        std::string description = static_cast<std::string>(effect->baseEffect->magicItemDescription);
                        selectedEnchantmentInfo += GetEnchantmentString(armor, description, magnitude);
                    }
                }
                else
                    selectedEnchantmentInfo = GetArmorDescription(armor, selectedEnchantmentInfo);

                int32_t equippedAccumulateValue = 0;
                float selectedRating = 0.0f;
                RE::InventoryEntryData *selectedEntryData;
                for (auto &[item, data] : inv)
                {
                    if (item && item->GetFormID() == selectedFormId)
                    {
                        selectedEntryData = data.second.get();
                        selectedRating = player->GetArmorValue(selectedEntryData);
                        break;
                    }
                }
                float equippedAccumulatedRating = 0.0f;

                std::vector<std::array<RE::GFxValue, EQUIPPED_ITEM_ARRAY_SIZE>> item_arr;
                std::vector<RE::FormID> pushedFormIds;

                for (auto &[slot, name] : slotList)
                {
                    if ((slots & slot) != Slot::kNone)
                    {
                        auto equipped = actor->GetWornArmor(slot);
                        if (equipped)
                        {
                            auto formId = equipped->GetFormID();
                            bool alreadyPushed = std::find(pushedFormIds.begin(), pushedFormIds.end(), formId) != pushedFormIds.end();
                            if (selectedFormId != formId && !alreadyPushed)
                            {
                                pushedFormIds.push_back(formId);
                                auto equippedName = equipped->GetName();
                                auto equippedType = GetArmorTypeString(equipped->GetArmorType());
                                auto equippedValue = equipped->GetGoldValue();
                                auto equippedEnchantment = equipped->formEnchanting;
                                std::string equippedEnchantmentInfo = "None";
                                if (equippedEnchantment)
                                {
                                    equippedEnchantmentInfo = "";
                                    auto effects = equippedEnchantment->effects;
                                    for (auto effect : effects)
                                    {
                                        auto magnitude = effect->GetMagnitude();
                                        std::string description = static_cast<std::string>(effect->baseEffect->magicItemDescription);
                                        selectedEnchantmentInfo += GetEnchantmentString(equipped, description, magnitude);
                                    }
                                }
                                else
                                    selectedEnchantmentInfo = GetArmorDescription(armor, selectedEnchantmentInfo);

                                equippedAccumulateValue += equippedValue;
                                float equippedRating = 0.0f;
                                RE::InventoryEntryData *equippedEntryData;
                                for (auto &[item, data] : inv)
                                {
                                    if (item && item->GetFormID() == formId)
                                    {
                                        equippedEntryData = data.second.get();
                                        equippedRating = player->GetArmorValue(equippedEntryData);
                                        break;
                                    }
                                }
                                equippedAccumulatedRating += equippedRating;

                                std::array<RE::GFxValue, EQUIPPED_ITEM_ARRAY_SIZE>
                                    itemInfo = {equippedName, "SlotsTBD", equippedType,
                                                equippedRating, equippedValue, equippedEnchantmentInfo.c_str()};
                                item_arr.push_back(itemInfo);
                            }
                        }
                    }
                }
                if (equippedAccumulatedRating > 0.0f)
                    equippedAccumulatedRating = selectedRating - equippedAccumulatedRating;
                if (equippedAccumulateValue > 0)
                    equippedAccumulateValue = selectedValue - equippedAccumulateValue;
                std::array<RE::GFxValue, SELECTED_ITEM_ARRAY_SIZE>
                    selectedItemInfo = {selectedName, "SlotsTBD", selectedType,
                                        selectedRating, equippedAccumulatedRating,
                                        selectedValue, equippedAccumulateValue,
                                        selectedEnchantmentInfo.c_str()};
                CEMenu::ResetMenu();
                CEMenu::SetActor(actor->GetName());
                CEMenu::CreateSelectedItemCard(selectedItemInfo);
                CEMenu::CreateComparisonItemCards(item_arr);
                CEMenu::ShowMenu();
            }
        }
    }

    bool GetItem()
    {
        RE::GFxValue Menu_mc = CEMenu::GetMenu_mc();
        if (Menu_mc.IsNull())
            return false;
        RE::GFxValue inventoryLists;
        RE::GFxValue itemList;
        RE::GFxValue selectedEntry;
        RE::GFxValue formId;
        if (!Menu_mc.GetMember("inventoryLists", &inventoryLists) ||
            !inventoryLists.GetMember("itemList", &itemList) ||
            !itemList.GetMember("selectedEntry", &selectedEntry) ||
            !selectedEntry.IsObject() ||
            !selectedEntry.GetMember("formId", &formId))
            return false;
        RE::FormID fid = static_cast<RE::FormID>(formId.GetUInt());
        currentFormID = fid;
        InventoryMenu::GetEquippedInSlots(fid);
        return true;
    }

    void ActorChangedUpdateMenu()
    {
        if (!currentFormID)
        {
            return;
        }
        InventoryMenu::GetEquippedInSlots(currentFormID);
    }
}

namespace Events
{
    std::chrono::steady_clock::time_point cycleButtonDownStart;
    bool cycleButtonHit = false;
    class InputEvent : public RE::BSTEventSink<RE::InputEvent *>
    {
    public:
        virtual RE::BSEventNotifyControl ProcessEvent(
            RE::InputEvent *const *a_event,
            RE::BSTEventSource<RE::InputEvent *> *) noexcept override
        {
            if (!a_event)
            {
                return RE::BSEventNotifyControl::kContinue;
            }
            if (auto e = *a_event; e)
            {
                auto eventType = e->GetEventType();
                if (eventType == RE::INPUT_EVENT_TYPE::kButton)
                {
                    if (auto btn = e->AsButtonEvent(); btn)
                    {
                        if (btn->GetIDCode() == COMPARE_KEY && !btn->IsUp()) // value for 'V' key
                        {
                            SKSE::GetTaskInterface()->AddTask([]()
                                                              { InventoryMenu::GetItem(); });
                        }
                        if (btn->GetIDCode() == CYCLE_KEY) // value for 'B' key
                        {
                            if (!btn->IsUp() and !cycleButtonHit)
                            {
                                cycleButtonHit = true;
                                cycleButtonDownStart = std::chrono::steady_clock::now();
                                return RE::BSEventNotifyControl::kContinue;
                            }
                            auto cycleButtonReleased = std::chrono::steady_clock::now();
                            auto heldDuration = std::chrono::duration_cast<std::chrono::milliseconds>(cycleButtonReleased - cycleButtonDownStart).count();
                            if (!btn->IsUp() && heldDuration < HOLD_THRESHOLD.count())
                                return RE::BSEventNotifyControl::kContinue;
                            if (btn->IsUp())
                                cycleButtonHit = false;
                            if (heldDuration >= HOLD_THRESHOLD.count())
                            {
                                SKSE::GetTaskInterface()->AddTask([]()
                                                                  { ActorUtils::SetActorToPlayer(); 
                                                                    if (!InventoryMenu::GetItem())
                                                                        InventoryMenu::ActorChangedUpdateMenu(); });
                            }
                            else
                            {
                                SKSE::GetTaskInterface()->AddTask([]()
                                                                  { ActorUtils::SetActorToNextFollower(); 
                                                                    if (!InventoryMenu::GetItem())
                                                                        InventoryMenu::ActorChangedUpdateMenu(); });
                            }
                        }
                    }
                }
                return RE::BSEventNotifyControl::kContinue;
            }
            return RE::BSEventNotifyControl::kContinue;
        }
        static InputEvent *GetSingleton()
        {
            static InputEvent singleton;
            return &singleton;
        }
    };

    class UIEvent : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {
    public:
        virtual RE::BSEventNotifyControl ProcessEvent(
            const RE::MenuOpenCloseEvent *a_event,
            RE::BSTEventSource<RE::MenuOpenCloseEvent> *) override
        {
            if (!a_event)
            {
                return RE::BSEventNotifyControl::kContinue;
            }
            auto menuName = a_event->menuName;
            if ((menuName == RE::InventoryMenu::MENU_NAME || menuName == RE::ContainerMenu::MENU_NAME ||
                 menuName == RE::BarterMenu::MENU_NAME || menuName == RE::GiftMenu::MENU_NAME))
            {
                if (a_event->opening)
                {
                    logger::debug("{} opened", menuName);
                    CEMenu::CreateMenu(menuName);
                    RE::BSInputDeviceManager::GetSingleton()->AddEventSink(Events::InputEvent::GetSingleton());
                }
                else
                {
                    logger::debug("{} closed", menuName);
                    CEMenu::DestroyMenu();
                    RE::BSInputDeviceManager::GetSingleton()->RemoveEventSink(Events::InputEvent::GetSingleton());
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }
        static UIEvent *GetSingleton()
        {
            static UIEvent singleton;
            return &singleton;
        }
    };

}

namespace CompareEquipmentNG
{
    void SetupLog()
    {
        auto logsFolder = SKSE::log::log_directory();
        if (!logsFolder)
            SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
        auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
        auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
        auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
        auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
        spdlog::set_default_logger(std::move(loggerPtr));
        spdlog::set_level(spdlog::level::trace);
        spdlog::flush_on(spdlog::level::trace);
    }

    void GetSettings()
    {
        /*double X_ORIGIN = 500.0f;
double Y_ORIGIN = 300.0f;
int BACKGROUND_ALPHA = 95;
bool PERMANENT_OPEN = false; // TODO: this first
bool DEFAULT_OPEN = false;   // TODO: Remove
uint32_t COMPARE_KEY = 47;
uint32_t CYCLE_KEY = 48;
std::chrono::milliseconds HOLD_THRESHOLD(500);*/
    }

    void OnPostLoadGame()
    {
        logger::info("Creating Event Sink");
        RE::UI::GetSingleton()->AddEventSink(Events::UIEvent::GetSingleton());
        logger::info("Created Event Sink");
        ActorUtils::SetActorToPlayer();
    }

    void MessageHandler(SKSE::MessagingInterface::Message *msg)
    {
        if (msg->type != SKSE::MessagingInterface::kPostLoadGame)
            return;
        if (!bool(msg->data))
            return;
        OnPostLoadGame();
    }

    extern "C" DLLEXPORT bool SKSEPlugin_Load(const SKSE::LoadInterface *skse)
    {
        SKSE::Init(skse);

        SetupLog();
        spdlog::set_level(spdlog::level::debug);

        logger::info("Compare Equipment NG Plugin Starting");

        auto *messaging = SKSE::GetMessagingInterface();
        messaging->RegisterListener("SKSE", MessageHandler);
        logger::info("Compare Equipment NG Plugin Loaded");
        return true;
    }
}
