namespace logger = SKSE::log;

double X_ORIGIN = 500.0f;
double Y_ORIGIN = 300.0f;
int BACKGROUND_ALPHA = 75;
bool DEFAULT_OPEN = false;
uint32_t TOGGLE_KEY = 48;
namespace CEMenu
{
    std::string basename = "CompareEquipmentMenu_";
    auto temp = basename + std::to_string(BACKGROUND_ALPHA) + "_" + (DEFAULT_OPEN ? "1" : "0");
    auto MENU_NAME = temp.c_str();
    // constexpr auto MENU_NAME = "CompareEquipmentMenu";
    static constexpr std::string_view SWF_PATH{"CompareEquipment.swf"};
    bool displayed = true;
    RE::GFxValue GetMenu_mc()
    {
        auto UISingleton = RE::UI::GetSingleton();
        auto inventoryMenu = UISingleton ? UISingleton->GetMenu<RE::InventoryMenu>() : nullptr;
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

    void ShowMenu()
    {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
            return;
        ceMenu.Invoke("showMenu");
        displayed = true;
        logger::debug("showMenu called");
    }

    void HideMenu()
    {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
            return;
        ceMenu.Invoke("hideMenu");
        displayed = false;
        logger::debug("hideMenu called");
    }

    void ToggleMenu()
    {
        if (!displayed)
        {
            ShowMenu();
        }
        else
        {
            HideMenu();
        }
    }

    void ChangeText(std::string_view text)
    {
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
            return;
        RE::GFxValue args[1];
        args[0].SetString(text);
        ceMenu.Invoke("set_text", nullptr, args, 1);
    }

    void DestroyMenu()
    {
        logger::debug("        Destroying Menu");
        RE::GFxValue ceMenu = GetCEMenu(GetMenu_mc());
        if (ceMenu.IsNull())
        {
            logger::debug("Failed to get {}", MENU_NAME);
            return;
        }

        if (!ceMenu.Invoke("removeMovieClip"))
        {
            logger::debug("Failed to remove {}", MENU_NAME);
            return;
        }
        logger::debug("Removed {}", MENU_NAME);
    }
    void CreateMenu()
    {
        logger::debug("          Creating Menu");
        RE::GFxValue Menu_mc = GetMenu_mc();
        if (Menu_mc.IsNull())
        {
            logger::debug("Failed to get Menu_mc");
            return;
        }
        RE::GFxValue _ceMenu = GetCEMenu(Menu_mc);
        if (!_ceMenu.IsNull())
        {
            logger::debug("{} already created", MENU_NAME);
            return;
        }
        RE::GFxValue args[2];
        RE::GFxValue ceMenuMovieClip;
        args[0].SetString(MENU_NAME); // name
        args[1] = 3999;               // depth
        if (!Menu_mc.Invoke("createEmptyMovieClip", &ceMenuMovieClip, args, 2))
        {
            logger::debug("failed to create {} movie clip via invoke", MENU_NAME);
            return;
        }
        logger::debug("Created {} movie clip via invoke", MENU_NAME);

        RE::GFxValue ceMenu = GetCEMenu(Menu_mc);
        if (ceMenu.IsNull())
        {
            logger::debug("Failed to get {}", MENU_NAME);
            return;
        }
        logger::debug("Got {}", MENU_NAME);

        RE::GFxValue result2;
        RE::GFxValue args2[1];
        args2[0].SetString(SWF_PATH); // name
        if (!ceMenu.Invoke("loadMovie", &result2, args2, 1))
        {
            logger::debug("Failed to load {} via invoke", args2[0].GetString());
            return;
        }
        logger::debug("Loaded {} via invoke", args2[0].GetString());

        RE::GFxValue xNumber;
        xNumber.SetNumber(X_ORIGIN);
        if (!ceMenu.SetMember("_x", xNumber))
        {
            logger::debug("Failed to set _x");
            return;
        }
        RE::GFxValue yNumber;
        yNumber.SetNumber(Y_ORIGIN);
        if (!ceMenu.SetMember("_y", yNumber))
        {
            logger::debug("Failed to set _y");
            return;
        }
        if (!DEFAULT_OPEN)
        {
            displayed = false;
        }
    }
}

namespace InventoryMenu
{
    static std::string s_text;

    void GetEquippedInSlots(RE::FormID formId)
    {
        if (auto form = RE::TESForm::LookupByID(formId))
        {
            if (auto armor = form->As<RE::TESObjectARMO>())
            {
                auto biped = armor->bipedModelData;
                auto slots = biped.bipedObjectSlots;
                auto player = RE::PlayerCharacter::GetSingleton();
                if (!player)
                {
                    return;
                }

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
                std::string text = std::string(armor->GetName()) + " would replace slots:";
                for (auto &[slot, name] : slotList)
                {
                    if ((slots & slot) != Slot::kNone)
                    {
                        auto equipped = player->GetWornArmor(slot);
                        if (equipped)
                        {
                            logger::debug("Hovered item would use {} slot, currently equipped: {}", name, equipped->GetName());
                        }
                        else
                        {
                            logger::debug("Hovered item would use {} slot, nothing equipped", name);
                        }
                        text += "\n   " + std::string(name) + " -> ";
                        text += equipped ? equipped->GetName() : "(empty)";
                    }
                }
                s_text = text;
                CEMenu::ChangeText(text.c_str());
            }
        }
    }

    void GetItem()
    {
        RE::GFxValue Menu_mc = CEMenu::GetMenu_mc();
        if (Menu_mc.IsNull())
            return;

        RE::GFxValue inventoryLists;
        RE::GFxValue itemList;
        RE::GFxValue selectedEntry;
        RE::GFxValue formId;
        if (!Menu_mc.GetMember("inventoryLists", &inventoryLists) ||
            !inventoryLists.GetMember("itemList", &itemList) ||
            !itemList.GetMember("selectedEntry", &selectedEntry) ||
            !selectedEntry.IsObject() ||
            !selectedEntry.GetMember("formId", &formId))
            return;

        InventoryMenu::GetEquippedInSlots(static_cast<RE::FormID>(formId.GetUInt()));
    }
}

namespace Events
{
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
                        if (!btn->IsUp())
                        {
                            return RE::BSEventNotifyControl::kContinue;
                        }
                        if (btn->GetIDCode() == 47) // value for 'V' key
                        {
                            InventoryMenu::GetItem();
                        }
                        if (btn->GetIDCode() == TOGGLE_KEY) // value for 'B' key
                        {
                            SKSE::GetTaskInterface()->AddTask([]()
                                                              { CEMenu::ToggleMenu(); });
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
            if (menuName == RE::InventoryMenu::MENU_NAME)
            {
                if (a_event->opening)
                {
                    logger::debug("Inventory Menu opened");
                    CEMenu::CreateMenu();
                }
                else
                {
                    logger::debug("Inventory Menu closed");
                    CEMenu::DestroyMenu();
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

namespace
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

    void OnPostLoadGame()
    {
        logger::info("Creating Event Sinks");
        RE::BSInputDeviceManager::GetSingleton()->AddEventSink(Events::InputEvent::GetSingleton());
        RE::UI::GetSingleton()->AddEventSink(Events::UIEvent::GetSingleton());
        logger::info("Created Event Sinks");
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
