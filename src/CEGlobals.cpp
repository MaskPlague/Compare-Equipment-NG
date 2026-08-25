namespace CEGlobals
{
    int ROWS = 4;
    int SCROLL_SPEED = 100;
    double SCROLL_DELAY = 2.5f;
    bool INSERT_NEWLINES = false;
    double INV_MENU_X_ORIGIN = 590.0f;
    double INV_MENU_Y_ORIGIN = 250.0f;
    double CONT_MENU_X_ORIGIN = 590.0f;
    double CONT_MENU_Y_ORIGIN = 250.0f;
    double BART_MENU_X_ORIGIN = 590.0f;
    double BART_MENU_Y_ORIGIN = 250.0f;
    double GIFT_MENU_X_ORIGIN = 590.0f;
    double GIFT_MENU_Y_ORIGIN = 250.0f;
    double CRAFTING_MENU_X_ORIGIN = 590.0f;
    double CRAFTING_MENU_Y_ORIGIN = 250.0f;
    double QLIE_X_ORIGIN = 100.0f;
    double QLIE_Y_ORIGIN = 350.0f;
    double HUD_X_ORIGIN = 450.0f;
    double HUD_Y_ORIGIN = 250.0f;
    int MENU_SCALE = 100;
    int QLIE_SCALE = 150;
    int HUD_SCALE = 100;
    int MENU_BACKGROUND_ALPHA = 95;
    int QLIE_BACKGROUND_ALPHA = 85;
    int HUD_BACKGROUND_ALPHA = 85;
    int MENU_LAYOUT = 0;
    int QLIE_LAYOUT = 0;
    int HUD_LAYOUT = 0;
    int SPACING_FROM_SELECTED = 5;
    int SPACING_BETWEEN_EQUIPPED_X = 5;
    int SPACING_BETWEEN_EQUIPPED_Y = 5;
    uint32_t COMPARE_KEY = 0;
    uint32_t CONTROLLER_KEY = 0;
    float HOLD_THRESHOLD = 500 * 0.001;
    float TRIPLE_HIT_WINDOW = 500 * 0.001;
    float SETTING_HOLD_THRESHOLD = 3000 * 0.001;
    float thumbstickX = 0.0f;
    float thumbstickY = 0.9f;
    float thumbstickThreshold = 0.9f;
    std::string effectCheckOrder = "DEP";
    int LOG_LEVEL = 2;

    bool CRASH_PREVENTION = true;

    bool QLIE_ALLOWED = true;
    bool HUD_ALLOWED = true;
    bool HUD_TOGGLEMODE = true;
    bool USE_ICONS = true;
    bool HIDE_3D = true;
    bool HIDE_SKY_UI_ITEM_CARD = true;
    bool QLIE_PERSISTENT_DISPLAY = false;
    bool MENU_PERSISTENT_DISPLAY = false;
    bool QLIE_PERSISTENT_TOGGLE = false;
    bool MENU_PERSISTENT_TOGGLE = false;
    bool QLIE_PERSISTENT_DEFAULT_DISPLAY = true;
    bool MENU_PERSISTENT_DEFAULT_DISPLAY = true;

    RE::INPUT_DEVICE lastInputDevice = RE::INPUT_DEVICE::kNone;

    int ConvertSkyrimKeyToSKSEKey(int key)
    {
        int offset = key - 266;
        switch (offset)
        {
        case 0:
            return 1; // DPAD_UP
        case 1:
            return 2; // DPAD_DOWN
        case 2:
            return 4; // DPAD_LEFT
        case 3:
            return 8; // DPAD_RIGHT
        case 4:
            return 16; // START
        case 5:
            return 32; // BACK
        case 6:
            return 64; // LEFT_THUMB (L3)
        case 7:
            return 128; // RIGHT_THUMB (R3)
        case 8:
            return 256; // LEFT_SHOULDER (LB)
        case 9:
            return 512; // RIGHT_SHOULDER (RB)
        case 10:
            return 4096; // A
        case 11:
            return 8192; // B
        case 12:
            return 16384; // X
        case 13:
            return 32768; // Y
        case 14:
            return 9; // LT
        case 15:
            return 10; // RT

        default:
            return 4;
        }
    }

    int ConvertSKSEKeyToSkyrimKey(int flag)
    {
        constexpr int GAMEPAD_BASE = 266;

        switch (flag)
        {
        case 1:
            return GAMEPAD_BASE + 0; // DPAD_UP
        case 2:
            return GAMEPAD_BASE + 1; // DPAD_DOWN
        case 4:
            return GAMEPAD_BASE + 2; // DPAD_LEFT
        case 8:
            return GAMEPAD_BASE + 3; // DPAD_RIGHT
        case 16:
            return GAMEPAD_BASE + 4; // START
        case 32:
            return GAMEPAD_BASE + 5; // BACK
        case 64:
            return GAMEPAD_BASE + 6; // LEFT_THUMB (L3)
        case 128:
            return GAMEPAD_BASE + 7; // RIGHT_THUMB (R3)
        case 256:
            return GAMEPAD_BASE + 8; // LEFT_SHOULDER (LB)
        case 512:
            return GAMEPAD_BASE + 9; // RIGHT_SHOULDER (RB)
        case 4096:
            return GAMEPAD_BASE + 10; // A
        case 8192:
            return GAMEPAD_BASE + 11; // B
        case 16384:
            return GAMEPAD_BASE + 12; // X
        case 32768:
            return GAMEPAD_BASE + 13; // Y
        case 9:
            return GAMEPAD_BASE + 14; // LT
        case 10:
            return GAMEPAD_BASE + 15; // RT

        default:
            return 268;
        }
    }

    namespace
    {
        constexpr const char *CONFIG_INI_PATH = "Data\\SKSE\\Plugins\\CompareEquipmentNG.ini";
        constexpr const char *TRANSLATION_INI_PATH = "Data\\SKSE\\Plugins\\CompareEquipmentNG_translation.ini";

        constexpr const int LOGPADDING = 22;

        long ClampToRange(long value, long min, long max)
        {
            if (value < min)
                return min;
            if (value > max)
                return max;
            return value;
        }

        long ResetIfOutOfRange(long value, long min, long max, long fallback)
        {
            return (value < min || value > max) ? fallback : value;
        }
    }

    // clang-format off
    // X(Type, Var, Section, Key, Default, Log Label, Comment)

    #define CE_GENERAL_SETTINGS(X) \
        X(Bool,   USE_ICONS,                  "General", "UseIcons",               true,  "Use Icons", \
            "#If item icons should be displayed when available.") \
        X(Bool,   HIDE_3D,                    "General", "Hide 3D Model",          true,  "Hide 3D Models", \
          "#If the 3d model in menus should be hidden.") \
        X(Bool,   HIDE_SKY_UI_ITEM_CARD,      "General", "Hide SkyUI Item Card",   true,  "Hide SkyUI Item Cards", \
          "#If the SkyUI item card should be hidden.") \
        X(Long,   SCROLL_SPEED,               "General", "Effect(s) Scroll Speed", 100,   "Effect(s) Scroll Speed", \
          "#How fast the Effect(s) text box will scroll\n#Default 100") \
        X(Double, SCROLL_DELAY,               "General", "Effect(s) Scroll Delay", 2.5,   "Effect(s) Scroll Delay (seconds)", \
          "#Length of delay before/after scrolling effects, in seconds.\n#Default 2.5") \
        X(Bool,   INSERT_NEWLINES,            "General", "Insert Newlines",        false, "Insert Newlines", \
          "#Insert a newline between enchantment effects ('. ' -> '.\\n') in the Effect(s) section") \
        X(Long,   SPACING_FROM_SELECTED,      "General", "Spacing From Selected",  5,     "Spacing From Selected", \
          "#Spacing of equipped item cards from selected item cards, default 5") \
        X(Long,   SPACING_BETWEEN_EQUIPPED_X, "General", "Spacing Between Equipped X", 5, "Spacing Between X", \
          "#Spacing between equipped item cards on the x-axis, default 5") \
        X(Long,   SPACING_BETWEEN_EQUIPPED_Y, "General", "Spacing Between Equipped Y", 5, "Spacing Between Y", \
          "#Spacing between equipped item cards on the y-axis, default 5")

    #define CE_INMENU_SETTINGS(X) \
        X(Double, INV_MENU_X_ORIGIN,      "InMenu", "Inventory X Offset", 590.0, "Inventory X Offset", \
          "#Selected Item's item card X offset in the Inventory menu\n#Default 590.0") \
        X(Double, INV_MENU_Y_ORIGIN,      "InMenu", "Inventory Y Offset", 250.0, "Inventory Y Offset", \
          "#Selected Item's item card Y offset in the Inventory menu\n#Default 250.0") \
        X(Double, CONT_MENU_X_ORIGIN,     "InMenu", "Container X Offset", 590.0, "Container X Offset", \
          "#Selected Item's item card X offset in the Container menu\n#Default 590.0") \
        X(Double, CONT_MENU_Y_ORIGIN,     "InMenu", "Container Y Offset", 250.0, "Container Y Offset", \
          "#Selected Item's item card Y offset in the Container menu\n#Default 250.0") \
        X(Double, BART_MENU_X_ORIGIN,     "InMenu", "Barter X Offset", 590.0, "Barter X Offset", \
          "#Selected Item's item card X offset in the Barter menu\n#Default 590.0") \
        X(Double, BART_MENU_Y_ORIGIN,     "InMenu", "Barter Y Offset", 250.0, "Barter Y Offset", \
          "#Selected Item's item card Y offset in the Barter menu\n#Default 250.0") \
        X(Double, GIFT_MENU_X_ORIGIN,     "InMenu", "Gift X Offset", 590.0, "Gift X Offset", \
          "#Selected Item's item card X offset in the Gift menu\n#Default 590.0") \
        X(Double, GIFT_MENU_Y_ORIGIN,     "InMenu", "Gift Y Offset", 250.0, "Gift Y Offset", \
          "#Selected Item's item card Y offset in the Gift menu\n#Default 250.0") \
        X(Double, CRAFTING_MENU_X_ORIGIN, "InMenu", "Crafting X Offset", 590.0, "Crafting X Offset", \
          "#Selected Item's item card X offset in the Crafting menu\n#Default 590.0") \
        X(Double, CRAFTING_MENU_Y_ORIGIN, "InMenu", "Crafting Y Offset", 250.0, "Crafting Y Offset", \
          "#Selected Item's item card Y offset in the Crafting menu\n#Default 250.0") \
        X(Bool,   MENU_PERSISTENT_DISPLAY, "InMenu", "Persistent Display", false, "Persistent Display", \
          "#If item cards should persistantly display without hotkey in menus.\n#Default false") \
        X(Bool,   MENU_PERSISTENT_TOGGLE,  "InMenu", "Persistent Toggle", false, "Persistent Toggle", \
          "#If pressing the hotkey will toggle persistent display in menus.\n#Default false") \
        X(Bool,   MENU_PERSISTENT_DEFAULT_DISPLAY, "InMenu", "Persistent Default Display", true, "Persistent Default", \
          "#If item cards should display by default before first toggled." \
          "\n#Requires Persistent Display to be true to have an effect." \
          "\n#Default true")

    #define CE_OUTOFMENU_SETTINGS(X) \
        X(Bool,   HUD_ALLOWED,    "OutOfMenu", "Enabled", true, "HUD Enabled", \
          "#Toggle for Compare Equipment functionality outside of Menus.\n#Default true") \
        X(Bool,   HUD_TOGGLEMODE, "OutOfMenu", "Toggle Mode", true, "HUD Toggle Mode", \
          "#If pressing the hotkey a second time will hide the item cards.\n#Default true") \
        X(Double, HUD_X_ORIGIN,   "OutOfMenu", "X Offset", 450.0, "HUD X Offset", \
          "#Selected Item's item card X offset outside of menus\n#Default 450.0") \
        X(Double, HUD_Y_ORIGIN,   "OutOfMenu", "Y Offset", 250.0, "HUD Y Offset", \
          "#Selected Item's item card Y offset outside of menus\n#Default 250.0")

    #define CE_QUICKLOOTIE_SETTINGS(X) \
        X(Bool,   QLIE_ALLOWED, "QuickLootIE", "Enabled", true, "QuickLootIE Enabled", \
          "#Toggle for Compare Equipment functionality for QuickLoot IE.\n#Default true") \
        X(Double, QLIE_X_ORIGIN, "QuickLootIE", "X Offset", 100.0, "QuickLootIE X Offset", \
          "#Selected Item's item card X offset for QuickLootIE\n#Default 100.0") \
        X(Double, QLIE_Y_ORIGIN, "QuickLootIE", "Y Offset", 350.0, "QuickLootIE Y Offset", \
          "#Selected Item's item card Y offset for QuickLootIE\n#Default 350.0") \
        X(Bool,   QLIE_PERSISTENT_DISPLAY, "QuickLootIE", "Persistent Display", false, "QLIE Persistent Display", \
          "#If item cards should persistantly display without hotkey for QuickLootIE.\n#Default false") \
        X(Bool,   QLIE_PERSISTENT_TOGGLE, "QuickLootIE", "Persistent Toggle", false, "QLIE Persistent Toggle", \
          "#If pressing the hotkey will toggle persistent display for QuickLootIE.\n#Default false") \
        X(Bool,   QLIE_PERSISTENT_DEFAULT_DISPLAY, "QuickLootIE", "Persistent Default Display", true, "QLIE Persistent Default", \
          "#If item cards should display by default before first toggled for QuickLootIE." \
          "\n#Requires Persistent Display to be true to have an effect." \
          "\n#Default true")

    #define CE_CONTROLS_SETTINGS(X) \
        X(Long, COMPARE_KEY, "Controls", "Compare Key", 47, "Compare Key", \
          "#Key that will display the comparison item cards, triple tap to cycle followers, hold to select player." \
          "\n#For controller see Controller Compare Key, ThumbstickAngle and ThumbstickThreshold" \
          "\n#Default 47 (V key), Key Codes can be found here : https://ck.uesp.net/wiki/Input_Script")

    #define CE_INTERNAL_SETTINGS(X) \
        X(Bool, CRASH_PREVENTION, "Internal", "Crash Prevention", true, "Crash Prevention", \
          "#Adds extra crash prevention measures." \
          "\n#This can slightly slow down display speeds and is only noticible when persistent display is enabled." \
          "\n#Only disable this if you are using persistent display and you dislike the item cards flashing.")

    #define CE_DEBUG_SETTINGS(X) \
        X(Long, LOG_LEVEL, "Debug", "Logging Level", 2, "Logging Level", \
          "#0: Errors, 1: Warnings, 2: Info(default), 3: Debug, 4: Trace")

    // Long settings that get clamped against a min/max range
    // X(Var, Section, Key, Default, Min, Max, Fallback, Log Label, Comment)

    #define CE_RANGED_LONG_SETTINGS(X) \
        X(MENU_SCALE,             "InMenu",      "Scale",             100, 1, 999999999L, 100, "Menus Scale", \
          "#Scale of item cards in menus, default 100") \
        X(QLIE_SCALE,             "QuickLootIE", "Scale",             150, 1, 999999999L, 150, "QuickLootIE Scale", \
          "#Scale of item cards for QuickLootIE, default 150") \
        X(HUD_SCALE,              "OutOfMenu",   "Scale",             100, 1, 999999999L, 100, "HUD Scale", \
          "#Scale of item cards outside of menus, default 100") \
        X(MENU_BACKGROUND_ALPHA,  "InMenu",      "Background Alpha",  95,  0, 100,        100, "Menus Background Alpha", \
          "#All item card's background alpha value in menus\n#Default 95, max 100, min 0") \
        X(QLIE_BACKGROUND_ALPHA,  "QuickLootIE", "Background Alpha",  85,  0, 100,        100, "QLIE Background Alpha", \
          "#All item card's background alpha value for QuickLootIE\n#Default 85, max 100, min 0") \
        X(HUD_BACKGROUND_ALPHA,   "OutOfMenu",   "Background Alpha",  85,  0, 100,        100, "HUD Background Alpha", \
          "#All item card's background alpha value outside of menus\n#Default 85, max 100, min 0") \
        X(MENU_LAYOUT,            "InMenu",      "Layout",            0,   0, 2,          0,   "Menus Layout", \
          "#Layout of item cards in menus\n#0: Vertically Centered (Default), 1: Upward, 2: Downward") \
        X(QLIE_LAYOUT,            "QuickLootIE", "Layout",            0,   0, 2,          0,   "QLIE Layout", \
          "#Layout of item cards for QuickLootIE\n#0: Vertically Centered (Default), 1: Upward, 2: Downward") \
        X(HUD_LAYOUT,             "OutOfMenu",   "Layout",            0,   0, 2,          0,   "HUD Layout", \
          "#Layout of item cards outside of menus\n#0: Vertically Centered (Default), 1: Upward, 2: Downward")

    // Expansion macros

    #define LOAD_ROW(Type, Var, Section, Key, Default, LogLabel, Comment) \
        Var = ini.Get##Type##Value(Section, Key, Default);
    #define LOG_ROW_Bool(Var, LogLabel)   logger::debug("{}: {:{}} {}", LogLabel, "", LOGPADDING - std::strlen(LogLabel), Var);
    #define LOG_ROW_Long(Var, LogLabel)   logger::debug("{}: {:{}} {}", LogLabel, "", LOGPADDING - std::strlen(LogLabel), Var);
    #define LOG_ROW_Double(Var, LogLabel) logger::debug("{}: {:{}} {:.2f}", LogLabel, "", LOGPADDING - std::strlen(LogLabel), Var);
    #define LOG_ROW(Type, Var, Section, Key, Default, LogLabel, Comment) \
        LOG_ROW_##Type(Var, LogLabel)
    #define SAVE_ROW(Type, Var, Section, Key, Default, LogLabel, Comment) \
        ini.Set##Type##Value(Section, Key, Var, Comment);

    #define LOAD_RANGED(Var, Section, Key, Default, Min, Max, Fallback, LogLabel, Comment) \
        Var = static_cast<decltype(Var)>(ResetIfOutOfRange(ini.GetLongValue(Section, Key, Default), Min, Max, Fallback));
    #define LOG_RANGED(Var, Section, Key, Default, Min, Max, Fallback, LogLabel, Comment) \
        logger::debug("{}: {:{}} {}", LogLabel, "", LOGPADDING - std::strlen(LogLabel), Var);
    #define SAVE_RANGED(Var, Section, Key, Default, Min, Max, Fallback, LogLabel, Comment) \
        ini.SetLongValue(Section, Key, Var, Comment);

    // clang-format on

    void LoadConfig()
    {
        CSimpleIniA ini;
        ini.SetUnicode();

        SI_Error rc = ini.LoadFile(CONFIG_INI_PATH);
        if (rc < 0)
        {
            logger::warn("Could not load CompareEquipmentNG.ini, using defaults");
        }

        // --------------------------------------------------------------- Load ---------------------------------------------------------------
        CE_GENERAL_SETTINGS(LOAD_ROW)
        ROWS = static_cast<int>(ClampToRange(ini.GetLongValue("General", "Maximum Rows", 4), 1, 4));

        CE_INMENU_SETTINGS(LOAD_ROW)
        CE_RANGED_LONG_SETTINGS(LOAD_RANGED)

        CE_OUTOFMENU_SETTINGS(LOAD_ROW)
        CE_QUICKLOOTIE_SETTINGS(LOAD_ROW)
        CE_CONTROLS_SETTINGS(LOAD_ROW)

        HOLD_THRESHOLD = static_cast<float>(ini.GetLongValue("Controls", "Hold Duration", 500) * 0.001);
        TRIPLE_HIT_WINDOW = static_cast<float>(ini.GetLongValue("Controls", "Triple Hit Window", 400) * 0.001);
        SETTING_HOLD_THRESHOLD = static_cast<float>(ini.GetLongValue("Controls", "Setting Hold Duration", 3000) * 0.001);

        CONTROLLER_KEY = ConvertSkyrimKeyToSKSEKey(ini.GetLongValue("Controls", "Controller Compare Key", 268));

        int angle = static_cast<int>(ResetIfOutOfRange(ini.GetLongValue("Controls", "Thumbstick Angle", 0), -180, 180, 0));

        thumbstickThreshold = static_cast<float>(ini.GetDoubleValue("Controls", "Thumbstick Threshold", 0.9));
        if (thumbstickThreshold >= 1.0f)
            thumbstickThreshold = 0.9f;

        const float radians = static_cast<float>(angle) * RE::NI_PI / 180.0f;
        thumbstickX = std::sin(radians);
        thumbstickY = std::cos(radians);

        long effectCheckOrderNum = ini.GetLongValue("Internal", "Effects Check Order", 123);
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
            logger::warn("Effects Check Order is invalid, only '1', '2', and '3' are valid, and there must be one of each."sv);
            effectCheckOrderNum = 123;
            effectCheckOrder = "DEP";
        }

        CE_INTERNAL_SETTINGS(LOAD_ROW)
        CE_DEBUG_SETTINGS(LOAD_ROW)

        // ---------------------------------------------------------- Debug log ----------------------------------------------------------
        logger::debug("Version                  {}"sv, SKSE::PluginDeclaration::GetSingleton()->GetVersion());
        logger::debug("Expected SWF Version:    {}"sv, EXPECTED_SWF_VERSION);

        logger::debug("-------------- General ------------------"sv);
        CE_GENERAL_SETTINGS(LOG_ROW)
        logger::debug("Maximum Rows: {}", ROWS);

        logger::debug("------------- In Menus ------------------"sv);
        CE_INMENU_SETTINGS(LOG_ROW)
        CE_RANGED_LONG_SETTINGS(LOG_RANGED)

        logger::debug("------------ Out Of Menus ---------------"sv);
        CE_OUTOFMENU_SETTINGS(LOG_ROW)

        logger::debug("----------- QuickLootIE -----------------"sv);
        CE_QUICKLOOTIE_SETTINGS(LOG_ROW)

        logger::debug("-------------- Controls ------------------"sv);
        CE_CONTROLS_SETTINGS(LOG_ROW)
        logger::debug("Hold Duration:           {} milliseconds"sv, HOLD_THRESHOLD * 1000);
        logger::debug("Triple Hit Window:       {} milliseconds"sv, TRIPLE_HIT_WINDOW * 1000);
        logger::debug("Setting Hold Duration:   {} milliseconds"sv, SETTING_HOLD_THRESHOLD * 1000);
        logger::debug("Controller Compare Key:  {}"sv, ConvertSKSEKeyToSkyrimKey(CONTROLLER_KEY));
        logger::debug("Thumbstick Angle:        {}"sv, angle);
        logger::debug("Thumbstick Threshold:    {}"sv, thumbstickThreshold);

        logger::debug("-------------- Internal ------------------"sv);
        logger::debug("Effects Check Order:     {}"sv, effectCheckOrderNum);
        CE_INTERNAL_SETTINGS(LOG_ROW)

        logger::debug("--------------- Debug --------------------"sv);
        CE_DEBUG_SETTINGS(LOG_ROW)

        // ------------------------------------------------------------- Save -------------------------------------------------------------
        CE_GENERAL_SETTINGS(SAVE_ROW)
        ini.SetLongValue("General", "Maximum Rows", ROWS,
                         "#Maximum number of compared item card rows, after this number of rows, a column will be created"
                         "\n#Default 4, max 4, min 1");

        CE_INMENU_SETTINGS(SAVE_ROW)
        CE_RANGED_LONG_SETTINGS(SAVE_RANGED)

        CE_OUTOFMENU_SETTINGS(SAVE_ROW)
        CE_QUICKLOOTIE_SETTINGS(SAVE_ROW)
        CE_CONTROLS_SETTINGS(SAVE_ROW)

        ini.SetLongValue("Controls", "Hold Duration", static_cast<long>(HOLD_THRESHOLD * 1000),
                         "#Duration, in milliseconds, to hold the CompareKey to set the selected actor to the player"
                         "\n#Default 500");
        ini.SetLongValue("Controls", "Triple Hit Window", static_cast<long>(TRIPLE_HIT_WINDOW * 1000),
                         "#Window, in milliseconds, to triple tap the CompareKey to cycle through active loaded followers"
                         "\n#Recommended to set to around 600 for controller users"
                         "\n#Default 400(400 / 2 = 200 milliseconds allowed between keypresses)");
        ini.SetLongValue("Controls", "Setting Hold Duration", static_cast<long>(SETTING_HOLD_THRESHOLD * 1000),
                         "#Duration, in milliseconds, to hold the CompareKey to reload the settings from this ini file."
                         "\n#Does not change active logging level."
                         "\n#Default 3000(3 seconds)");
        ini.SetLongValue("Controls", "Controller Compare Key", ConvertSKSEKeyToSkyrimKey(CONTROLLER_KEY),
                         "#Controller key that will display the comparison item cards. Only works while looking at a valid item in world or in QuickLoot."
                         "\n#In menus the thumbstick angle and threshold are used instead. Does not block input from activating other actions."
                         "\n#Default 268 (Left D-pad), Key Codes can be found here at the end: https://ck.uesp.net/wiki/Input_Script");
        ini.SetLongValue("Controls", "Thumbstick Angle", angle,
                         "#Angle to flick right thumbstick to activate 0 = up, 90 = right, 180/-180 = down, -90 = left."
                         "\n#You can set whatever angle between -180 to 180"
                         "\n#Default 0 -> up ");
        ini.SetDoubleValue("Controls", "Thumbstick Threshold", thumbstickThreshold,
                           "#Percent that is considered activated, 1.0 is all the way to the edge and 0.0 is centered"
                           "\n#Default 0.9 -> 90% ");

        CE_INTERNAL_SETTINGS(SAVE_ROW)
        ini.SetLongValue("Internal", "Effects Check Order", effectCheckOrderNum,
                         "#Order in which to check for effects strings, once a valid string is found it does not check for the others."
                         "\n#1: Item Description, 2: ESP defined enchantment's MGEF description, 3: Player enchanted enchantment's MGEF description"
                         "\n#Default is 123, must contain a 1, 2, and 3 in any order.");

        CE_DEBUG_SETTINGS(SAVE_ROW)

        ini.SaveFile(CONFIG_INI_PATH);

        CEMenu::UpdateMenuName();
        LoadTranslation();
    }

    std::string QLIE_HINT_TEXT = "Compare";
    std::string buttonCompareText = "Compare";
    std::string comparingTo = "<font color=\"#D4D4D4\" size=\"12\">Comparing to</font>";
    std::string damageLabelText = "<font color=\"#D4D4D4\" size=\"12\">Damage:</font>";
    std::string critLabelText = "<font color=\"#D4D4D4\" size=\"12\">Crit:</font>";
    std::string noneText = "None";
    std::string armorTypeLabelText = "<font color=\"#D4D4D4\" size=\"12\">Armor Type:</font>";
    std::string heavyArmor = "Heavy";
    std::string lightArmor = "Light";
    std::string clothArmor = "Cloth";
    std::string armorRatingLabelText = "<font color=\"#D4D4D4\" size=\"12\">Armor Rating:</font>";
    std::string goldLabelText = "<font color=\"#D4D4D4\" size=\"12\">Gold Value:</font>";
    std::string effectsLabelText = "E<font color=\"#D4D4D4\" size=\"12\">ffect(s):</font>";
    std::string equippedTo = "<font color=\"#D4D4D4\" size=\"12\">Equipped to</font>";
    std::string slotsLabelText = "<font color=\"#D4D4D4\" size=\"12\">Slot(s):</font>";
    std::string leftHand = "Left Hand";
    std::string rightHand = "Right Hand";
    std::string bothHands = "Both Hands";
    std::string speedLabelText = "<font color=\"#D4D4D4\" size=\"12\">Speed:</font>";
    std::string reachLabelText = "<font color=\"#D4D4D4\" size=\"12\">Reach:</font>";
    std::string staggerLabelText = "<font color=\"#D4D4D4\" size=\"12\">Stagger:</font>";
    std::string keyText = "<font color=\"#D4D4D4\" size=\"12\">Key:</font>";
    std::string keyInfo = "(Total : T, Average : A, Max : M)";
    std::string totalKey = "T";
    std::string averageKey = "A";
    std::string maxKey = "M";
    std::string notApplicable = "N/A";
    std::string betterWrapperStart = "<font color=\"#00FF00\" size=\"12\">(+";
    std::string betterWrapperEnd = ")</font>";
    std::string worseWrapperStart = "<font color=\"#FF0000\" size=\"12\">(";
    std::string worseWrapperEnd = ")</font>";
    std::string normalWrapperStart = "<font color=\"#FFFFFF\" size=\"12\">";
    std::string normalWrapperEnd = "</font>";
    std::string diffTypeWrapperStart = "<font color=\"#FFFF00\" size=\"12\">";
    std::string diffTypeWrapperEnd = "</font>";
    std::string settingsReloaded = "CompareEquipmentNG: Settings Reloaded";

    long diffOffset = 0;
    long valueOffset = 2;
    long columnTwoOffset = 155;

    // clang-format off

    #define CE_CUSTOMIZATION_SETTINGS(X) \
        X(Long, diffOffset,      "Customization", "diffOffset",      0,   "Diff Offset", \
          "#Offset of difference values from regular values, default 0") \
        X(Long, valueOffset,     "Customization", "valueOffset",     2,   "Value Offset", \
          "#Offset of values from labels, default 0") \
        X(Long, columnTwoOffset, "Customization", "columnTwoOffset", 155, "Column Two Offset", \
          "#Offset of the second column of labels, default 155")

    // X(Var, Section, Key, Default, Comment)
    #define CE_TRANSLATION_SETTINGS(X) \
        X(QLIE_HINT_TEXT, "Translations", "QuickLoot IE Hint Text", "Compare", \
          "#This ini is also reloaded with the settings in game." \
          "\n#All text should support the basic html that ActionScript 2.0 supports(granted the font in the SWF supports it)." \
          "\n#Replace the text following the \"=\" in the below:\n" \
          "\n#The hint text that is shown in the QuickLoot Menu." \
          "\n#set one of these if using Oblivion Interaction Icons:" \
          "\n# Use either EE or EhE since EE can look a bit sus: <font face='Daedric'>EE</font>" \
          "\n# Use P or whatever you want from Oblivion Interaction Icons's Docs/OII_cheatsheet.png: <font face=' Iconographia'>P</font>" \
          "\n# Font size 22 is what makes the hotkey symbol correctly sized for me, yours may be different: <font face = 'SkyrimBooks_Handwritten_Bold' size = '22'><b>[][]</ b></ font> ") \
        X(buttonCompareText, "Translations", "buttonCompareText", "Compare", \
          "# Text to display next to button hint") \
        X(comparingTo, "Translations", "comparingTo", "<font color=\"#D4D4D4\" size=\"12\">Comparing to</font>", \
          "# The item is being compared to x's equipment") \
        X(damageLabelText, "Translations", "damageLabelText", "<font color=\"#D4D4D4\" size=\"12\">Damage:</font>", \
          "# Damage of weapon, example -> \"Damage:\" 10.00") \
        X(critLabelText, "Translations", "critLabelText", "<font color=\"#D4D4D4\" size=\"12\">Crit:</font>", \
          "# Critical damage of weapon, example -> \"Crit:\" 4.00") \
        X(noneText, "Translations", "noneText", "None", \
          "# Indicate no effects/no value") \
        X(armorTypeLabelText, "Translations", "armorTypeLabelText", "<font color=\"#D4D4D4\" size=\"12\">Armor Type:</font>", \
          "# The type of armor that an item is, example -> \"Armor Type:\" Cloth") \
        X(heavyArmor, "Translations", "heavyArmor", "Heavy", "# Armor type: Heavy") \
        X(lightArmor, "Translations", "lightArmor", "Light", "# Armor type: Light") \
        X(clothArmor, "Translations", "clothArmor", "Cloth", "# Armor type: Cloth") \
        X(armorRatingLabelText, "Translations", "armorRatingLabelText", "<font color=\"#D4D4D4\" size=\"12\">Armor Rating:</font>", \
          "# The rating/defence of an armor, example -> \"Armor Rating:\" 20") \
        X(goldLabelText, "Translations", "goldLabelText", "<font color=\"#D4D4D4\" size=\"12\">Gold Value:</font>", \
          "# How much the item is worth in gold, example -> \"Gold Value:\" 1000") \
        X(effectsLabelText, "Translations", "effectsLabelText", "<font color=\"#D4D4D4\" size=\"12\">Effect(s):</font>", \
          "# What enchantment/effect/item description the item has") \
        X(equippedTo, "Translations", "equippedTo", "<font color=\"#D4D4D4\" size=\"12\">Equipped to</font>", \
          "# The item is \"Equipped to\" x actor") \
        X(slotsLabelText, "Translations", "slotsLabelText", "<font color=\"#D4D4D4\" size=\"12\">Slot(s):</font>", \
          "# The body slot or slots the item has") \
        X(leftHand, "Translations", "leftHand", "Left Hand", "# The hand the weapon is equipped to: left") \
        X(rightHand, "Translations", "rightHand", "Right Hand", "# The hand the weapon is equipped to: right") \
        X(bothHands, "Translations", "bothHands", "Both Hands", "# The hand(s) the weapon is equipped to: both") \
        X(speedLabelText, "Translations", "speedLabelText", "<font color=\"#D4D4D4\" size=\"12\">Speed:</font>", \
          "# The speed at which a weapon attacks.") \
        X(reachLabelText, "Translations", "reachLabelText", "<font color=\"#D4D4D4\" size=\"12\">Reach:</font>", \
          "# The reach/range of a weapon when swung/attacking.") \
        X(staggerLabelText, "Translations", "staggerLabelText", "<font color=\"#D4D4D4\" size=\"12\">Stagger:</font>", \
          "# The stagger damage a weapon does to an enemy to make them stagger when attacking.") \
        X(keyText, "Translations", "keyText", "<font color=\"#D4D4D4\" size=\"12\">Key:</font>", \
          "# The legend or key that explains what the differences are relative to") \
        X(keyInfo, "Translations", "keyInfo", "(Total : T, Average : A, Max : M)", \
          "# The actual key. Total difference, average difference, max difference.") \
        X(totalKey, "Translations", "totalKey", "T", \
          "# The indicator from the above key -> (T)otal: (T), to add a space put a semi-colon in front ';T'") \
        X(averageKey, "Translations", "averageKey", "A", \
          "# The indicator from the above key -> (A)verage: (A), to add a space put a semi-colon in front ';A'") \
        X(maxKey, "Translations", "maxKey", "M", \
          "# The indicator from the above key -> (M)ax: (M), to add a space put a semi-colon in front ';M'") \
        X(notApplicable, "Translations", "notApplicable", "N/A", \
          "# Simple text that indicates something is not applicable / not available") \
        X(betterWrapperStart, "Translations", "betterWrapperStart", "<font color=\"#00FF00\">(+", \
          "# Better HTML wrapper start #RRGGBB so a green (+Diff)") \
        X(betterWrapperEnd, "Translations", "betterWrapperEnd", ")</font>", \
          "# Better HTML wrapper end") \
        X(worseWrapperStart, "Translations", "worseWrapperStart", "<font color=\"#FF0000\">(", \
          "# Worse HTML wrapper start #RRGGBB so a red (Diff)") \
        X(worseWrapperEnd, "Translations", "worseWrapperEnd", ")</font>", \
          "# Worse HTML wrapper end") \
        X(normalWrapperStart, "Translations", "normalWrapperStart", "<font color=\"#FFFFFF\">", \
          "# Normal HTML Wrapper start so normal text like the \"32\" in Armor Rating: 32 is white") \
        X(normalWrapperEnd, "Translations", "normalWrapperEnd", "</font>", \
          "# Normal HTML wrapper end") \
        X(diffTypeWrapperStart, "Translations", "diffTypeWrapperStart", "<font color=\"#FFFF00\">", \
          "# Different Type HTML Wrapper start so different type armor types are colored yellow. i.e. Selected Light != Equipped Heavy make Heavy yellow.") \
        X(diffTypeWrapperEnd, "Translations", "diffTypeWrapperEnd", "</font>", \
          "# Different Type HTML wrapper end") \
        X(settingsReloaded, "Translations", "settingsReloaded", "CompareEquipmentNG: Settings Reloaded", \
          "# Notification that appears when you reload the settings via hotkey")

    #define LOAD_STR(Var, Section, Key, Default, Comment) \
        Var = ini.GetValue(Section, Key, Default);
    #define SAVE_STR(Var, Section, Key, Default, Comment) \
        ini.SetValue(Section, Key, Var.c_str(), Comment);

    // clang-format on

    void LoadTranslation()
    {
        CSimpleIniA ini;
        ini.SetUnicode();

        SI_Error rc = ini.LoadFile(TRANSLATION_INI_PATH);
        if (rc < 0)
        {
            logger::warn("Could not load CompareEquipmentNG_translation.ini, using defaults");
        }

        CE_CUSTOMIZATION_SETTINGS(LOAD_ROW)
        CE_TRANSLATION_SETTINGS(LOAD_STR)

        CE_CUSTOMIZATION_SETTINGS(SAVE_ROW)
        CE_TRANSLATION_SETTINGS(SAVE_STR)

        ini.SaveFile(TRANSLATION_INI_PATH);
    }

#undef CE_GENERAL_SETTINGS
#undef CE_INMENU_SETTINGS
#undef CE_OUTOFMENU_SETTINGS
#undef CE_QUICKLOOTIE_SETTINGS
#undef CE_CONTROLS_SETTINGS
#undef CE_INTERNAL_SETTINGS
#undef CE_DEBUG_SETTINGS
#undef CE_RANGED_LONG_SETTINGS
#undef CE_CUSTOMIZATION_SETTINGS
#undef CE_TRANSLATION_SETTINGS
#undef LOAD_ROW
#undef LOG_ROW
#undef LOG_ROW_Bool
#undef LOG_ROW_Long
#undef LOG_ROW_Double
#undef SAVE_ROW
#undef LOAD_RANGED
#undef LOG_RANGED
#undef SAVE_RANGED
#undef LOAD_STR
#undef SAVE_STR
}