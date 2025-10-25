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

    void SetLogLevel()
    {
        switch (CEGlobals::LOG_LEVEL)
        {
        case 0:
            spdlog::set_level(spdlog::level::err);
            break;
        case 1:
            spdlog::set_level(spdlog::level::warn);
            break;
        case 2:
            spdlog::set_level(spdlog::level::info);
            break;
        case 3:
            spdlog::set_level(spdlog::level::debug);
            break;
        case 4:
            spdlog::set_level(spdlog::level::trace);
            break;
        default:
            CEGlobals::LOG_LEVEL = 2;
            spdlog::set_level(spdlog::level::info);
        }
    }

    void OnPostDataLoaded()
    {
        CEActorUtils::SetActorToPlayer();

        static bool eventSinksRegistered = false;
        if (!eventSinksRegistered)
        {
            logger::debug("Creating Event Sinks");
            RE::UI::GetSingleton()->AddEventSink(CEGameEvents::UIEvent::GetSingleton());
            RE::BSInputDeviceManager::GetSingleton()->AddEventSink(CEGameEvents::DeviceInputEvent::GetSingleton());
            RE::BSInputDeviceManager::GetSingleton()->AddEventSink(CEGameEvents::InputEvent::GetSingleton());
            logger::debug("Created Event Sinks");
            eventSinksRegistered = true;
        }

        const auto dllHandle = GetModuleHandleA("QuickLootIE");
        if (dllHandle == NULL)
        {
            logger::debug("QuickLoot IE not installed, skipping QuickLoot IE API initialization");
            return;
        }
        static bool quickLootIEInitialized = false;
        if (!quickLootIEInitialized)
        {
            logger::debug("Initializing QuickLoot IE API");
            QuickLoot::QuickLootAPI::Init();
            QuickLoot::QuickLootAPI::RegisterSelectItemHandler(CEGameEvents::QuickLootSelectItemHandler);
            QuickLoot::QuickLootAPI::RegisterCloseLootMenuHandler(CEGameEvents::QuickLootCloseHandler);
            QuickLoot::QuickLootAPI::RegisterOpenLootMenuHandler(CEGameEvents::QuickLootOpenHandler);
            quickLootIEInitialized = true;
        }
    }

    void MessageHandler(SKSE::MessagingInterface::Message *msg)
    {
        if (msg->type == SKSE::MessagingInterface::kDataLoaded)
        {
            logger::debug("Data has loaded.");
            OnPostDataLoaded();
        }
        if (msg->type == SKSE::MessagingInterface::kPostLoadGame && bool(msg->data) && CEGlobals::HUD_ALLOWED)
        {
            logger::debug("PostLoadGame: HUD_ALLOWED, creating the HUDMenu CEMenu");
            CEMenu::CreateMenu("HUDMenu");
        }
    }

    extern "C" DLLEXPORT bool SKSEPlugin_Load(const SKSE::LoadInterface *skse)
    {
        SKSE::Init(skse);
        SetupLog();
        CEGlobals::LoadConfig();
        SetLogLevel();
        logger::info("Compare Equipment NG Plugin Starting");
        logger::info("Registered scaleform functions: {}", SKSE::GetScaleformInterface()->Register(&CEMenu::RegisterFuncs, "CompareEquipment"));
        auto *messaging = SKSE::GetMessagingInterface();
        messaging->RegisterListener("SKSE", MessageHandler);
        logger::info("Compare Equipment NG Plugin Loaded");
        return true;
    }
}
