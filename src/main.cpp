namespace logger = SKSE::log;
namespace InventoryMenu
{
    static std::string s_text;
    void ShowNotification()
    {
        logger::info("Showing Notification");
        RE::DebugMessageBox(s_text.c_str());
        logger::info("Message: {}", s_text.c_str());
        logger::info("should have notified");
    }

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
                            logger::info("Hovered item would use {} slot, currently equipped: {}", name, equipped->GetName());
                        }
                        else
                        {
                            logger::info("Hovered item would use {} slot, nothing equipped", name);
                        }
                        text += "\n   " + std::string(name) + " -> ";
                        text += equipped ? equipped->GetName() : "(empty)";
                    }
                }
                s_text = text;
                InventoryMenu::ShowNotification();
            }
        }
    }

    void GetItem()
    {
        if (!RE::UI::GetSingleton())
        {
            return;
        }

        auto inventoryMenu = RE::UI::GetSingleton()->GetMenu<RE::InventoryMenu>();
        if (!inventoryMenu)
        {
            return;
        }

        RE::GFxMovieView *view = inventoryMenu->uiMovie.get();
        if (!view)
        {
            return;
        }

        RE::GFxValue itemList;
        if (!view->GetVariable(&itemList, "_root.Menu_mc.inventoryLists.itemList"))
        {
            return;
        }

        RE::GFxValue selectedEntry;
        if (!itemList.GetMember("selectedEntry", &selectedEntry))
        {
            return;
        }

        RE::GFxValue formId;
        if (!selectedEntry.GetMember("formId", &formId))
        {
            return;
        }

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
                        if (btn->GetIDCode() == 47)
                            InventoryMenu::GetItem();
                        if (btn->GetIDCode() == 48)
                            InventoryMenu::ShowNotification();
                    }
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }
        static InputEvent *GetSingleton()
        {
            static InputEvent singleton;
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

    void
    OnPostLoadGame()
    {
        logger::info("Creating Event Sink");
        RE::BSInputDeviceManager::GetSingleton()->AddEventSink(Events::InputEvent::GetSingleton());
        logger::info("Created Event Sink");
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
        spdlog::set_level(spdlog::level::info);

        logger::info("Compare Equipment NG Plugin Starting");

        auto *messaging = SKSE::GetMessagingInterface();
        messaging->RegisterListener("SKSE", MessageHandler);

        logger::info("Compare Equipment NG Plugin Loaded");

        return true;
    }
}
