namespace logger = SKSE::log;

namespace CEGlobals
{
    double X_ORIGIN = 590.0f;
    double Y_ORIGIN = 250.0f;
    int SCALE = 100;
    int BACKGROUND_ALPHA = 95;
    int ROWS = 4;
    uint32_t COMPARE_KEY = 0;
    float HOLD_THRESHOLD = 500 * 0.001;
    float TRIPLE_HIT_WINDOW = 500 * 0.001;
    float SETTING_HOLD_THRESHOLD = 3000 * 0.001;
    int LOG_LEVEL = 2;
    RE::INPUT_DEVICE lastInputDevice = RE::INPUT_DEVICE::kNone;

    void LoadConfig()
    {
        CSimpleIniA ini;
        ini.SetUnicode();

        SI_Error rc = ini.LoadFile("Data\\SKSE\\Plugins\\CompareEquipmentNG.ini");
        if (rc < 0)
        {
            logger::warn("Could not load CompareEquipmentNG.ini, using defaults");
        }

        X_ORIGIN = ini.GetDoubleValue("General", "Xoffset", 590.0f);
        Y_ORIGIN = ini.GetDoubleValue("General", "Yoffset", 250.0f);
        SCALE = ini.GetLongValue("General", "Scale", 100);
        if (SCALE <= 0)
            SCALE = 100;
        BACKGROUND_ALPHA = ini.GetLongValue("General", "BackgroundAlpha", 95);
        if (BACKGROUND_ALPHA < 0 || BACKGROUND_ALPHA > 100)
            BACKGROUND_ALPHA = 100;
        ROWS = ini.GetLongValue("General", "MaximumRows", 4);
        if (ROWS > 4)
            ROWS = 4;
        if (ROWS < 1)
            ROWS = 1;
        COMPARE_KEY = ini.GetLongValue("General", "CompareKey", 47);
        HOLD_THRESHOLD = static_cast<float>(ini.GetLongValue("General", "HoldDuration", 500) * 0.001);
        TRIPLE_HIT_WINDOW = static_cast<float>(ini.GetLongValue("General", "TripleHitWindow", 400) * 0.001);
        SETTING_HOLD_THRESHOLD = static_cast<float>(ini.GetLongValue("General", "SettingHoldDuration", 3000) * 0.001);
        LOG_LEVEL = ini.GetLongValue("Debug", "LoggingLevel", 2);

        logger::debug("Version                  {}", SKSE::PluginDeclaration::GetSingleton()->GetVersion());
        logger::debug("Expected SWF Version:    {}", EXPECTED_SWF_VERSION);
        logger::debug("X Offset:                {:.2f}", X_ORIGIN);
        logger::debug("Y Offset:                {:.2f}", Y_ORIGIN);
        logger::debug("Scale:                   {}", SCALE);
        logger::debug("Background Alpha         {}", BACKGROUND_ALPHA);
        logger::debug("Maximum Rows:            {}", ROWS);
        logger::debug("Compare Key:             {}", COMPARE_KEY);
        logger::debug("Hold Duration:           {} milliseconds", HOLD_THRESHOLD * 1000);
        logger::debug("Triple Hit Window:       {} milliseconds", TRIPLE_HIT_WINDOW * 1000);
        logger::debug("Setting Hold Duration:   {} milliseconds", SETTING_HOLD_THRESHOLD * 1000);

        ini.SetDoubleValue("General", "Xoffset", X_ORIGIN);
        ini.SetDoubleValue("General", "Yoffset", Y_ORIGIN);
        ini.SetLongValue("General", "Scale", SCALE);
        ini.SetLongValue("General", "BackgroundAlpha", BACKGROUND_ALPHA);
        ini.SetLongValue("General", "MaximumRows", ROWS);
        ini.SetLongValue("General", "CompareKey", COMPARE_KEY);
        ini.SetLongValue("General", "HoldDuration", static_cast<long>(HOLD_THRESHOLD * 1000));
        ini.SetLongValue("General", "TripleHitWindow", static_cast<long>(TRIPLE_HIT_WINDOW * 1000));
        ini.SetLongValue("General", "SettingHoldDuration", static_cast<long>(SETTING_HOLD_THRESHOLD * 1000));

        ini.SetLongValue("Debug", "LoggingLevel", LOG_LEVEL);

        ini.SaveFile("Data\\SKSE\\Plugins\\CompareEquipmentNG.ini");

        CEMenu::UpdateMenuName();
    }
}