namespace CEGlobals
{
    double MENU_X_ORIGIN = 590.0f;
    double MENU_Y_ORIGIN = 250.0f;
    double QLIE_X_ORIGIN = -1000.0f;
    double QLIE_Y_ORIGIN = 0.0f;
    double HUD_X_ORIGIN = 450.0f;
    double HUD_Y_ORIGIN = 250.0f;
    int MENU_SCALE = 100;
    int QLIE_SCALE = 150;
    int HUD_SCALE = 100;
    int MENU_BACKGROUND_ALPHA = 95;
    int QLIE_BACKGROUND_ALPHA = 85;
    int HUD_BACKGROUND_ALPHA = 85;
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

    bool QLIE_ALLOWED = true;
    bool QLIE_SHOWHINT = true;
    bool HUD_ALLOWED = true;
    bool HUD_TOGGLEMODE = true;
    bool USE_ICONS = true;

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
        ROWS = ini.GetLongValue("General", "Maximum Rows", 4);
        if (ROWS > 4)
            ROWS = 4;
        if (ROWS < 1)
            ROWS = 1;

        USE_ICONS = ini.GetBoolValue("General", "UseIcons", true);

        //------------------------------ In Menus ---------------------------------------------------------------

        MENU_X_ORIGIN = ini.GetDoubleValue("InMenu", "X Offset", 590.0f);
        MENU_Y_ORIGIN = ini.GetDoubleValue("InMenu", "Y Offset", 250.0f);
        MENU_SCALE = ini.GetLongValue("InMenu", "Scale", 100);
        if (MENU_SCALE <= 0)
            MENU_SCALE = 100;
        MENU_BACKGROUND_ALPHA = ini.GetLongValue("InMenu", "Background Alpha", 95);
        if (MENU_BACKGROUND_ALPHA < 0 || MENU_BACKGROUND_ALPHA > 100)
            MENU_BACKGROUND_ALPHA = 100;

        //------------------------------ Outside of Menus --------------------------------------------------------

        HUD_ALLOWED = ini.GetBoolValue("OutOfMenu", "Enabled", true);
        HUD_TOGGLEMODE = ini.GetBoolValue("OutOfMenu", "Toggle Mode", true);
        HUD_X_ORIGIN = ini.GetDoubleValue("OutOfMenu", "X Offset", 450.0f);
        HUD_Y_ORIGIN = ini.GetDoubleValue("OutOfMenu", "Y Offset", 250.0f);
        HUD_SCALE = ini.GetLongValue("OutOfMenu", "Scale", 100);
        if (HUD_SCALE <= 0)
            HUD_SCALE = 100;
        HUD_BACKGROUND_ALPHA = ini.GetLongValue("OutOfMenu", "Background Alpha", 85);
        if (HUD_BACKGROUND_ALPHA < 0 || HUD_BACKGROUND_ALPHA > 100)
            HUD_BACKGROUND_ALPHA = 100;

        //------------------------------- QuickLootIE ------------------------------------------------------------

        QLIE_ALLOWED = ini.GetBoolValue("QuickLootIE", "Enabled", true);
        QLIE_SHOWHINT = ini.GetBoolValue("QuickLootIE", "Show Hint", true);
        QLIE_X_ORIGIN = ini.GetDoubleValue("QuickLootIE", "X Offset", -1000.0f);
        QLIE_Y_ORIGIN = ini.GetDoubleValue("QuickLootIE", "Y Offset", 0.0f);
        QLIE_SCALE = ini.GetLongValue("QuickLootIE", "Scale", 150);
        if (QLIE_SCALE <= 0)
            QLIE_SCALE = 150;
        QLIE_BACKGROUND_ALPHA = ini.GetLongValue("QuickLootIE", "Background Alpha", 85);
        if (QLIE_BACKGROUND_ALPHA < 0 || QLIE_BACKGROUND_ALPHA > 100)
            QLIE_BACKGROUND_ALPHA = 100;

        //-------------------------------- Controls ---------------------------------------------------------------

        COMPARE_KEY = ini.GetLongValue("Controls", "Compare Key", 47);
        HOLD_THRESHOLD = static_cast<float>(ini.GetLongValue("Controls", "Hold Duration", 500) * 0.001);
        TRIPLE_HIT_WINDOW = static_cast<float>(ini.GetLongValue("Controls", "Triple Hit Window", 400) * 0.001);

        SETTING_HOLD_THRESHOLD = static_cast<float>(ini.GetLongValue("Controls", "Setting Hold Duration", 3000) * 0.001);
        int angle = ini.GetLongValue("Controls", "Thumbstick Angle", 0);
        if (angle > 180 || angle < -180)
            angle = 0;

        thumbstickThreshold = static_cast<float>(ini.GetDoubleValue("Controls", "Thumbstick Threshold", 0.9f));
        if (thumbstickThreshold >= 1.0)
            thumbstickThreshold = 0.9f;

        float radians = static_cast<float>(angle) * RE::NI_PI / 180.0f;
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
            logger::warn("Effects Check Order is invalid, only '1', '2', and '3' are valid, and there must be one of each.");
            effectCheckOrderNum = 123;
            effectCheckOrder = "DEP";
        }

        LOG_LEVEL = ini.GetLongValue("Debug", "Logging Level", 2);

        logger::debug("Version                  {}", SKSE::PluginDeclaration::GetSingleton()->GetVersion());
        logger::debug("Expected SWF Version:    {}", EXPECTED_SWF_VERSION);
        logger::debug("Maximum Rows:            {}", ROWS);
        logger::debug("Use Icons:               {}", USE_ICONS);
        logger::debug("X Offset:                {:.2f}", MENU_X_ORIGIN);
        logger::debug("Y Offset:                {:.2f}", MENU_Y_ORIGIN);
        logger::debug("Scale:                   {}", MENU_SCALE);
        logger::debug("Background Alpha         {}", MENU_BACKGROUND_ALPHA);
        logger::debug("HUD Enabled:             {}", HUD_ALLOWED);
        logger::debug("HUD Toggle Mode:         {}", HUD_TOGGLEMODE);
        logger::debug("HUD X Offset:            {:.2f}", HUD_X_ORIGIN);
        logger::debug("HUD Offset:              {:.2f}", HUD_Y_ORIGIN);
        logger::debug("HUD Scale:               {}", HUD_SCALE);
        logger::debug("HUD Background Alpha     {}", HUD_BACKGROUND_ALPHA);
        logger::debug("QuickLootIE Enabled:     {}", QLIE_ALLOWED);
        logger::debug("QuickLootIE Show Hint:   {}", QLIE_SHOWHINT);
        logger::debug("QuickLootIE X Offset:    {:.2f}", QLIE_X_ORIGIN);
        logger::debug("QuickLootIE Y Offset:    {:.2f}", QLIE_Y_ORIGIN);
        logger::debug("QuickLootIE Scale:       {}", QLIE_SCALE);
        logger::debug("QLIE Background Alpha    {}", QLIE_BACKGROUND_ALPHA);
        logger::debug("Compare Key:             {}", COMPARE_KEY);
        logger::debug("Hold Duration:           {} milliseconds", HOLD_THRESHOLD * 1000);
        logger::debug("Triple Hit Window:       {} milliseconds", TRIPLE_HIT_WINDOW * 1000);
        logger::debug("Setting Hold Duration:   {} milliseconds", SETTING_HOLD_THRESHOLD * 1000);
        logger::debug("Thumbstick Angle:        {}", angle);
        logger::debug("Thumbstick Threshold:    {}", thumbstickThreshold);
        logger::debug("Effects Check Order:     {}", effectCheckOrderNum);
        logger::debug("Logging Level:           {}", LOG_LEVEL);

        const char *rowsComment = ("#Maximum number of compared item card rows, after this number of rows, a column will be created"
                                   "\n#Default 4, max 4, min 1");
        ini.SetLongValue("General", "Maximum Rows", ROWS, rowsComment);
        ini.SetBoolValue("General", "UseIcons", USE_ICONS, "#If item icons should be displayed when available.");

        //------------------------------ In Menus ---------------------------------------------------------------
        ini.SetDoubleValue("InMenu", "X Offset", MENU_X_ORIGIN, "#Selected Item's item card X offset in menus\n#Default 590.0");
        ini.SetDoubleValue("InMenu", "Y Offset", MENU_Y_ORIGIN, "#Selected Item's item card Y offset in menus\n#Default 250.0");
        ini.SetLongValue("InMenu", "Scale", MENU_SCALE, "#Scale of item cards in menus, default 100");
        ini.SetLongValue("InMenu", "Background Alpha", MENU_BACKGROUND_ALPHA, "#All item card's background alpha value in menus\n#Default 95, max 100, min 0");

        //------------------------------ Outside Of Menus ---------------------------------------------------------------
        ini.SetBoolValue("OutOfMenu", "Enabled", HUD_ALLOWED, "#Toggle for Compare Equipment functionality outside of Menus, doesn't function with controller.\n#Default true");
        ini.SetBoolValue("OutOfMenu", "Toggle Mode", HUD_TOGGLEMODE, "#If pressing the hotkey a second time will hide the item cards.\n#Default true");
        ini.SetDoubleValue("OutOfMenu", "X Offset", HUD_X_ORIGIN, "#Selected Item's item card X offset outside of menus\n#Default 450.0");
        ini.SetDoubleValue("OutOfMenu", "Y Offset", HUD_Y_ORIGIN, "#Selected Item's item card Y offset outside of menus\n#Default 250.0");
        ini.SetLongValue("OutOfMenu", "Scale", HUD_SCALE, "#Scale of item cards outside of menus, default 100");
        ini.SetLongValue("OutOfMenu", "Background Alpha", HUD_BACKGROUND_ALPHA, "#All item card's background alpha value outside of menus\n#Default 85, max 100, min 0");

        //------------------------------ QuickLoot IE ---------------------------------------------------------------
        ini.SetBoolValue("QuickLootIE", "Enabled", QLIE_ALLOWED, "#Toggle for Compare  Equipment functionality for QuickLoot IE, doesn't function with controller.\n#Default true");
        ini.SetBoolValue("QuickLootIE", "Show Hint", QLIE_SHOWHINT, "#Toggle the hint display, it is janky so you may want to disable it.\n#Default true");
        ini.SetDoubleValue("QuickLootIE", "X Offset", QLIE_X_ORIGIN, "#Selected Item's item card X offset for QuickLootIE\n#Default -1000.0");
        ini.SetDoubleValue("QuickLootIE", "Y Offset", QLIE_Y_ORIGIN, "#Selected Item's item card Y offset for QuickLootIE\n#Default 0.0");
        ini.SetLongValue("QuickLootIE", "Scale", QLIE_SCALE, "#Scale of item cards for QuickLootIE, default 150");
        ini.SetLongValue("QuickLootIE", "Background Alpha", QLIE_BACKGROUND_ALPHA, "#All item card's background alpha value for QuickLootIE\n#Default 85, max 100, min 0");

        //------------------------------ Controls------------------------------------------------------------------------
        const char *compareKeyComment = ("#Key that will display the comparison item cards, triple tap to cycle followers, hold to select player."
                                         "\n#Does not work for controller users, see ThumbstickAngle and ThumbstickThreshold"
                                         "\n#Default 47 (V key), Key Codes can be found here : https://ck.uesp.net/wiki/Input_Script");
        ini.SetLongValue("Controls", "Compare Key", COMPARE_KEY, compareKeyComment);
        const char *holdPlayerComment = ("#Duration, in milliseconds, to hold the CompareKey to set the selected actor to the player"
                                         "\n#Default 500");
        ini.SetLongValue("Controls", "Hold Duration", static_cast<long>(HOLD_THRESHOLD * 1000), holdPlayerComment);
        const char *tripleHitComment = ("#Window, in milliseconds, to triple tap the CompareKey to cycle through active loaded followers"
                                        "\n#Recommended to set to around 600 for controller users"
                                        "\n#Default 400(400 / 2 = 200 milliseconds allowed between keypresses)");
        ini.SetLongValue("Controls", "Triple Hit Window", static_cast<long>(TRIPLE_HIT_WINDOW * 1000), tripleHitComment);
        const char *settingHoldComment = ("#Duration, in milliseconds, to hold the CompareKey to reload the settings from this ini file."
                                          "\n#Does not change active logging level."
                                          "\n#Default 3000(3 seconds)");
        ini.SetLongValue("Controls", "Setting Hold Duration", static_cast<long>(SETTING_HOLD_THRESHOLD * 1000), settingHoldComment);
        const char *angleComment = ("#Angle to flick right thumbstick to activate 0 = up, 90 = right, 180/-180 = down, -90 = left."
                                    "\n#You can set whatever angle between -180 to 180"
                                    "\n#Default 0 -> up ");
        ini.SetLongValue("Controls", "Thumbstick Angle", angle, angleComment);
        const char *thumbstickThresholdComment = ("#Percent that is considered activated, 1.0 is all the way to the edge and 0.0 is centered"
                                                  "\n#Default 0.9 -> 90% ");
        ini.SetDoubleValue("Controls", "Thumbstick Threshold", thumbstickThreshold, thumbstickThresholdComment);
        const char *effectCheckOrderComment = ("\n#Order in which to check for effects strings, once a valid string is found it does not check for the others."
                                               "\n#1: Item Description, 2: ESP defined enchantment's MGEF description, 3: Player enchanted enchantment's MGEF description"
                                               "\n#Default is 123, must contain a 1, 2, and 3 in any order.");
        ini.SetLongValue("Internals", "Effects Check Order", effectCheckOrderNum, effectCheckOrderComment);

        ini.SetLongValue("Debug", "Logging Level", LOG_LEVEL, "#0: Errors, 1: Warnings, 2: Info(default), 3: Debug, 4: Trace");

        ini.SaveFile("Data\\SKSE\\Plugins\\CompareEquipmentNG.ini");

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
    std::string unscaledText = "(Unscaled)";
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

    long diffOffset = 0;
    long valueOffset = 2;
    long columnTwoOffset = 155;

    void LoadTranslation()
    {
        CSimpleIniA ini;
        ini.SetUnicode();

        SI_Error rc = ini.LoadFile("Data\\SKSE\\Plugins\\CompareEquipmentNG_translation.ini");
        if (rc < 0)
        {
            logger::warn("Could not load CompareEquipmentNG_translation.ini, using defaults");
        }

        diffOffset = ini.GetLongValue("Customization", "diffOffset", 0);
        valueOffset = ini.GetLongValue("Customization", "valueOffset", 2);
        columnTwoOffset = ini.GetLongValue("Customization", "columnTwoOffset", 155);

        QLIE_HINT_TEXT = ini.GetValue("Translations", "QuickLoot IE Hint Text", "Compare");
        buttonCompareText = ini.GetValue("Translations", "buttonCompareText", "Compare");
        comparingTo = ini.GetValue("Translations", "comparingTo", "<font color=\"#D4D4D4\" size=\"12\">Comparing to</font>");
        damageLabelText = ini.GetValue("Translations", "damageLabelText", "<font color=\"#D4D4D4\" size=\"12\">Damage:</font>");
        critLabelText = ini.GetValue("Translations", "critLabelText", "<font color=\"#D4D4D4\" size=\"12\">Crit:</font>");
        noneText = ini.GetValue("Translations", "noneText", "None");
        armorTypeLabelText = ini.GetValue("Translations", "armorTypeLabelText", "<font color=\"#D4D4D4\" size=\"12\">Armor Type:</font>");
        heavyArmor = ini.GetValue("Translations", "heavyArmor", "Heavy");
        lightArmor = ini.GetValue("Translations", "lightArmor", "Light");
        clothArmor = ini.GetValue("Translations", "clothArmor", "Cloth");
        armorRatingLabelText = ini.GetValue("Translations", "armorRatingLabelText", "<font color=\"#D4D4D4\" size=\"12\">Armor Rating:</font>");
        unscaledText = ini.GetValue("Translations", "unscaledText", "(Unscaled)");
        goldLabelText = ini.GetValue("Translations", "goldLabelText", "<font color=\"#D4D4D4\" size=\"12\">Gold Value:</font>");
        effectsLabelText = ini.GetValue("Translations", "effectsLabelText", "<font color=\"#D4D4D4\" size=\"12\">Effect(s):</font>");
        equippedTo = ini.GetValue("Translations", "equippedTo", "<font color=\"#D4D4D4\" size=\"12\">Equipped to</font>");
        slotsLabelText = ini.GetValue("Translations", "slotsLabelText", "<font color=\"#D4D4D4\" size=\"12\">Slot(s):</font>");
        leftHand = ini.GetValue("Translations", "leftHand", "Left Hand");
        rightHand = ini.GetValue("Translations", "rightHand", "Right Hand");
        bothHands = ini.GetValue("Translations", "bothHands", "Both Hands");
        speedLabelText = ini.GetValue("Translations", "speedLabelText", "<font color=\"#D4D4D4\" size=\"12\">Speed:</font>");
        reachLabelText = ini.GetValue("Translations", "reachLabelText", "<font color=\"#D4D4D4\" size=\"12\">Reach:</font>");
        staggerLabelText = ini.GetValue("Translations", "staggerLabelText", "<font color=\"#D4D4D4\" size=\"12\">Stagger:</font>");
        keyText = ini.GetValue("Translations", "keyText", "<font color=\"#D4D4D4\" size=\"12\">Key:</font>");
        keyInfo = ini.GetValue("Translations", "keyInfo", "(Total : T, Average : A, Max : M)");
        totalKey = ini.GetValue("Translations", "totalKey", "T");
        averageKey = ini.GetValue("Translations", "averageKey", "A");
        maxKey = ini.GetValue("Translations", "maxKey", "M");
        notApplicable = ini.GetValue("Translations", "notApplicable", "N/A");
        betterWrapperStart = ini.GetValue("Translations", "betterWrapperStart", "<font color=\"#00FF00\">(+");
        betterWrapperEnd = ini.GetValue("Translations", "betterWrapperEnd", ")</font>");
        worseWrapperStart = ini.GetValue("Translations", "worseWrapperStart", "<font color=\"#FF0000\">(");
        worseWrapperEnd = ini.GetValue("Translations", "worseWrapperEnd", ")</font>");
        normalWrapperStart = ini.GetValue("Translations", "normalWrapperStart", "<font color=\"#FFFFFF\">");
        normalWrapperEnd = ini.GetValue("Translations", "normalWrapperEnd", "</font>");

        ini.SetLongValue("Customization", "diffOffset", diffOffset,
                         "#Offset of difference values from regular values, default 0");

        ini.SetLongValue("Customization", "valueOffset", valueOffset,
                         "#Offset of values from labels, default 0");

        ini.SetLongValue("Customization", "columnTwoOffset", columnTwoOffset,
                         "#Offset of the second column of labels, default 155");

        ini.SetValue("Translations", "QuickLoot IE Hint Text", QLIE_HINT_TEXT.c_str(),
                     "#This ini is also reloaded with the settings in game."
                     "\n#All text should support the basic html that ActionScript 2.0 supports(granted the font in the SWF supports it)."
                     "\n#Replace the text following the \"=\" in the below:\n"
                     "\n#The hint text that is shown in the QuickLoot Menu."
                     "\n#set one of these if using Oblivion Interaction Icons:"
                     "\n# Use either EE or EhE since EE can look a bit sus: <font face='Daedric'>EE</font>"
                     "\n# Use P or whatever you want from Oblivion Interaction Icons's Docs/OII_cheatsheet.png: <font face=' Iconographia'>P</font>"
                     "\n# Font size 22 is what makes the hotkey symbol correctly sized for me, yours may be different: <font face = 'SkyrimBooks_Handwritten_Bold' size = '22'><b>[][]</ b></ font> ");

        ini.SetValue("Translations", "buttonCompareText", buttonCompareText.c_str(),
                     "# Text to display next to button hint");

        ini.SetValue("Translations", "comparingTo", comparingTo.c_str(),
                     "# The item is being compared to x's equipment");

        ini.SetValue("Translations", "damageLabelText", damageLabelText.c_str(),
                     "# Damage of weapon, example -> \"Damage:\" 10.00");

        ini.SetValue("Translations", "critLabelText", critLabelText.c_str(),
                     "# Critical damage of weapon, example -> \"Crit:\" 4.00");

        ini.SetValue("Translations", "noneText", noneText.c_str(),
                     "# Indicate no effects/no value");

        ini.SetValue("Translations", "armorTypeLabelText", armorTypeLabelText.c_str(),
                     "# The type of armor that an item is, example -> \"Armor Type:\" Cloth");

        ini.SetValue("Translations", "heavyArmor", heavyArmor.c_str(),
                     "# Armor type: Heavy");

        ini.SetValue("Translations", "lightArmor", lightArmor.c_str(),
                     "# Armor type: Light");

        ini.SetValue("Translations", "clothArmor", clothArmor.c_str(),
                     "# Armor type: Cloth");

        ini.SetValue("Translations", "armorRatingLabelText", armorRatingLabelText.c_str(),
                     "# The rating/defence of an armor, example -> \"Armor Rating:\" 20");

        ini.SetValue("Translations", "unscaledText", unscaledText.c_str(),
                     "# Indicates that the armor rating/weapon damage is not scaled with the actor's perks/levels.");

        ini.SetValue("Translations", "goldLabelText", goldLabelText.c_str(),
                     "# How much the item is worth in gold, example -> \"Gold Value:\" 1000");

        ini.SetValue("Translations", "effectsLabelText", effectsLabelText.c_str(),
                     "# What enchantment/effect/item description the item has");

        ini.SetValue("Translations", "equippedTo", equippedTo.c_str(),
                     "# The item is \"Equipped to\" x actor");

        ini.SetValue("Translations", "slotsLabelText", slotsLabelText.c_str(),
                     "# The body slot or slots the item has");

        ini.SetValue("Translations", "leftHand", leftHand.c_str(),
                     "# The hand the weapon is equipped to: left");

        ini.SetValue("Translations", "rightHand", rightHand.c_str(),
                     "# The hand the weapon is equipped to: right");

        ini.SetValue("Translations", "bothHands", bothHands.c_str(),
                     "# The hand(s) the weapon is equipped to: both");

        ini.SetValue("Translations", "speedLabelText", speedLabelText.c_str(),
                     "# The speed at which a weapon attacks.");

        ini.SetValue("Translations", "reachLabelText", reachLabelText.c_str(),
                     "# The reach/range of a weapon when swung/attacking.");

        ini.SetValue("Translations", "staggerLabelText", staggerLabelText.c_str(),
                     "# The stagger damage a weapon does to an enemy to make them stagger when attacking.");

        ini.SetValue("Translations", "keyText", keyText.c_str(),
                     "# The legend or key that explains what the differences are relative to");

        ini.SetValue("Translations", "keyInfo", keyInfo.c_str(),
                     "# The actual key. Total difference, average difference, max difference.");

        ini.SetValue("Translations", "totalKey", totalKey.c_str(),
                     "# The indicator from the above key -> (T)otal: (T)");

        ini.SetValue("Translations", "averageKey", averageKey.c_str(),
                     "# The indicator from the above key -> (A)verage: (A)");

        ini.SetValue("Translations", "maxKey", maxKey.c_str(),
                     "# The indicator from the above key -> (M)ax: (M)");

        ini.SetValue("Translations", "notApplicable", notApplicable.c_str(),
                     "# Simple text that indicates something is not applicable / not available");

        ini.SetValue("Translations", "betterWrapperStart", betterWrapperStart.c_str(),
                     "# Better HTML wrapper start #RRGGBB so a green (+Diff)");

        ini.SetValue("Translations", "betterWrapperEnd", betterWrapperEnd.c_str(),
                     "# Better HTML wrapper end");

        ini.SetValue("Translations", "worseWrapperStart", worseWrapperStart.c_str(),
                     "# Worse HTML wrapper start #RRGGBB so a red (Diff)");

        ini.SetValue("Translations", "worseWrapperEnd", worseWrapperEnd.c_str(),
                     "# Worse HTML wrapper end");

        ini.SetValue("Translations", "normalWrapperStart", normalWrapperStart.c_str(),
                     "# Normal HTML Wrapper start so normal text like the \"32\" in Armor Rating: 32 is white");

        ini.SetValue("Translations", "normalWrapperEnd", normalWrapperEnd.c_str(),
                     "# Normal HTML wrapper end");

        ini.SaveFile("Data\\SKSE\\Plugins\\CompareEquipmentNG_translation.ini");
    }
}