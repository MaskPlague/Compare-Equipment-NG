namespace logger = SKSE::log;

namespace CEGlobals
{
    double X_ORIGIN = 500.0f;
    double Y_ORIGIN = 300.0f;
    int BACKGROUND_ALPHA = 95;
    bool PERMANENT_OPEN = false;
    uint32_t COMPARE_KEY = 47;
    std::chrono::milliseconds HOLD_THRESHOLD(500);
    std::chrono::milliseconds TRIPLE_HIT_WINDOW(500);
    std::chrono::milliseconds GET_SETTINGS_THRESHOLD(3000);
    int LOG_LEVEL = 2;
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

    void LoadConfig()
    {
        logger::info("Getting ini settings");
        CSimpleIniA ini;
        ini.SetUnicode();

        SI_Error rc = ini.LoadFile("Data\\SKSE\\Plugins\\CompareEquipmentNG.ini");
        if (rc < 0)
        {
            logger::warn("Could not load CompareEquipmentNG.ini, using defaults");
        }

        X_ORIGIN = ini.GetDoubleValue("General", "Xoffset", 500.0f);
        Y_ORIGIN = ini.GetDoubleValue("General", "Yoffset", 300.0f);
        BACKGROUND_ALPHA = ini.GetLongValue("General", "BackgroundAlpha", 95);
        PERMANENT_OPEN = ini.GetBoolValue("General", "PermanentlyOpen", false);
        COMPARE_KEY = ini.GetLongValue("General", "CompareKey", 47);
        HOLD_THRESHOLD = static_cast<std::chrono::milliseconds>(ini.GetLongValue("General", "HoldDuration", 500));
        TRIPLE_HIT_WINDOW = static_cast<std::chrono::milliseconds>(ini.GetLongValue("General", "TripleHitWindow", 400));
        LOG_LEVEL = ini.GetLongValue("Debug", "LoggingLevel", 2);

        SetLogLevel();

        logger::debug("Version              {}", SKSE::PluginDeclaration::GetSingleton()->GetVersion());
        logger::debug("X Offset:            {:.2f}", X_ORIGIN);
        logger::debug("Y Offset:            {:.2f}", Y_ORIGIN);
        logger::debug("Background Alpha     {}", BACKGROUND_ALPHA);
        logger::debug("Permanently Open:    {}", PERMANENT_OPEN);
        logger::debug("Compare Key:         {}", COMPARE_KEY);
        logger::debug("Hold Duration:       {} milliseconds", HOLD_THRESHOLD.count());
        logger::debug("Triple Hit Window:   {} milliseconds", TRIPLE_HIT_WINDOW.count());

        ini.SetDoubleValue("General", "Xoffset", X_ORIGIN);
        ini.SetDoubleValue("General", "Yoffset", Y_ORIGIN);
        ini.SetLongValue("General", "BackgroundAlpha", BACKGROUND_ALPHA);
        ini.SetBoolValue("General", "PermanentlyOpen", PERMANENT_OPEN);
        ini.SetLongValue("General", "CompareKey", COMPARE_KEY);
        ini.SetLongValue("General", "HoldDuration", static_cast<long>(HOLD_THRESHOLD.count()));
        ini.SetLongValue("General", "TrippleHitWindow", static_cast<long>(TRIPLE_HIT_WINDOW.count()));

        ini.SetLongValue("Debug", "LoggingLevel", LOG_LEVEL);

        ini.SaveFile("Data\\SKSE\\Plugins\\CompareEquipmentNG.ini");
    }

}