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
    float thumbstickX = 0.0f;
    float thumbstickY = 0.9f;
    float thumbstickThreshold = 0.9f;
    std::string effectCheckOrder = "DEP";
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

        X_ORIGIN = ini.GetDoubleValue("Display", "Xoffset", 590.0f);
        Y_ORIGIN = ini.GetDoubleValue("Display", "Yoffset", 250.0f);
        SCALE = ini.GetLongValue("Display", "Scale", 100);
        if (SCALE <= 0)
            SCALE = 100;
        BACKGROUND_ALPHA = ini.GetLongValue("Display", "BackgroundAlpha", 95);
        if (BACKGROUND_ALPHA < 0 || BACKGROUND_ALPHA > 100)
            BACKGROUND_ALPHA = 100;
        ROWS = ini.GetLongValue("Display", "MaximumRows", 4);
        if (ROWS > 4)
            ROWS = 4;
        if (ROWS < 1)
            ROWS = 1;
        COMPARE_KEY = ini.GetLongValue("Controls", "CompareKey", 47);
        HOLD_THRESHOLD = static_cast<float>(ini.GetLongValue("Controls", "HoldDuration", 500) * 0.001);
        TRIPLE_HIT_WINDOW = static_cast<float>(ini.GetLongValue("Controls", "TripleHitWindow", 400) * 0.001);

        SETTING_HOLD_THRESHOLD = static_cast<float>(ini.GetLongValue("Controls", "SettingHoldDuration", 3000) * 0.001);
        int angle = ini.GetLongValue("Controls", "ThumbstickAngle", 0);
        if (angle > 180 || angle < -180)
            angle = 0;

        thumbstickThreshold = static_cast<float>(ini.GetDoubleValue("Controls", "ThumbstickThreshold", 0.9f));
        if (thumbstickThreshold >= 1.0)
            thumbstickThreshold = 0.9f;

        float radians = static_cast<float>(angle) * RE::NI_PI / 180.0f;
        thumbstickX = std::sin(radians);
        thumbstickY = std::cos(radians);

        long effectCheckOrderNum = ini.GetLongValue("Internal", "EffectsCheckOrder", 123);
        effectCheckOrder.clear();
        bool valid = true;
        bool d = false;
        bool e = false;
        bool p = false;
        std::string temp = std::to_string(effectCheckOrderNum);
        for (char c : temp)
        {
            switch (c - '0')
            {
            case 1:
                effectCheckOrder += 'D';
                d = true;
                break;
            case 2:
                effectCheckOrder += 'E';
                e = true;
                break;
            case 3:
                effectCheckOrder += 'P';
                p = true;
                break;
            default:
                valid = false;
                break;
            }
            if (!valid)
                break;
        }

        if (!valid || !d || !e || !p || temp.length() > 3)
        {
            logger::warn("EffectsCheckOrder is invalid, only '1', '2', and '3' are valid, and there must be one of each.");
            effectCheckOrderNum = 123;
            effectCheckOrder = "DEP";
        }

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
        logger::debug("Thumbstick Angle:        {}", angle);
        logger::debug("Thumbstick Threshold:    {}", thumbstickThreshold);
        logger::debug("Effects Check Order:     {}", effectCheckOrderNum);

        ini.SetDoubleValue("Display", "Xoffset", X_ORIGIN, "#Selected Item's Item Card X offset\n#Default 590.0");
        ini.SetDoubleValue("Display", "Yoffset", Y_ORIGIN, "#Selected Item's Item Card Y offset\n#Default 250.0");
        ini.SetLongValue("Display", "Scale", SCALE, "#Scale of item cards, default 100");
        ini.SetLongValue("Display", "BackgroundAlpha", BACKGROUND_ALPHA, "#All item card's background alpha value\n#Default 95, max 100, min 0");
        const char *rowsComment = ("#Maximum number of compared item card rows, after this number of rows, a column will be created"
                                   "\n#Default 4, max 4, min 1");
        ini.SetLongValue("Display", "MaximumRows", ROWS, rowsComment);
        const char *compareKeyComment = ("#Key that will display the comparison item cards, triple tap to cycle followers, hold to select player."
                                         "\n#Does not work for controller users, see ThumbstickAngle and ThumbstickThreshold"
                                         "\n#Default 47(V key)Key Codes can be found here : https://ck.uesp.net/wiki/Input_Script");
        ini.SetLongValue("Controls", "CompareKey", COMPARE_KEY, compareKeyComment);
        const char *holdPlayerComment = ("#Duration, in milliseconds, to hold the CompareKey to set the selected actor to the player"
                                         "\n#Default 500");
        ini.SetLongValue("Controls", "HoldDuration", static_cast<long>(HOLD_THRESHOLD * 1000), holdPlayerComment);
        const char *tripleHitComment = ("#Window, in milliseconds, to triple tap the CompareKey to cycle through active loaded followers"
                                        "\n#Recommended to set to around 600 for controller users"
                                        "\n#Default 400(400 / 2 = 200 milliseconds allowed between keypresses)");
        ini.SetLongValue("Controls", "TripleHitWindow", static_cast<long>(TRIPLE_HIT_WINDOW * 1000), tripleHitComment);
        const char *settingHoldComment = ("#Duration, in milliseconds, to hold the CompareKey to reload the settings from this ini file. Does not change active logging level."
                                          "\n#Default 3000(3 seconds)");
        ini.SetLongValue("Controls", "SettingHoldDuration", static_cast<long>(SETTING_HOLD_THRESHOLD * 1000), settingHoldComment);
        const char *angleComment = ("#Angle to flick right thumbstick to activate 0 = up, 90 = right, 180/-180 = down, -90 = left. You can set whatever angle between -180 to 180"
                                    "\n#Default 0 -> up ");
        ini.SetLongValue("Controls", "ThumbstickAngle", angle, angleComment);
        const char *thumbstickThresholdComment = ("#Percent that is considered activated, 1.0 is all the way to the edge and 0.0 is centered"
                                                  "\n#Default 0.9 -> 90% ");
        ini.SetDoubleValue("Controls", "ThumbstickThreshold", thumbstickThreshold, thumbstickThresholdComment);
        const char *effectCheckOrderComment = ("\n#Order in which to check for effects strings, once a valid string is found it does not check for the others."
                                               "\n#1: Item Description, 2: ESP defined enchantment's MGEF description, 3: Player enchanted enchantment's MGEF description"
                                               "\n#Default is 123");
        ini.SetLongValue("Internals", "EffectsCheckOrder", effectCheckOrderNum, effectCheckOrderComment);

        ini.SetLongValue("Debug", "LoggingLevel", LOG_LEVEL, "#0: Errors, 1: Warnings, 2: Info(default), 3: Debug, 4: Trace");

        ini.SaveFile("Data\\SKSE\\Plugins\\CompareEquipmentNG.ini");

        CEMenu::UpdateMenuName();
    }
}