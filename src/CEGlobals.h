#pragma once

namespace CEGlobals
{
    extern double INV_MENU_X_ORIGIN;
    extern double INV_MENU_Y_ORIGIN;
    extern double CONT_MENU_X_ORIGIN;
    extern double CONT_MENU_Y_ORIGIN;
    extern double BART_MENU_X_ORIGIN;
    extern double BART_MENU_Y_ORIGIN;
    extern double GIFT_MENU_X_ORIGIN;
    extern double GIFT_MENU_Y_ORIGIN;
    extern double QLIE_X_ORIGIN;
    extern double QLIE_Y_ORIGIN;
    extern double HUD_X_ORIGIN;
    extern double HUD_Y_ORIGIN;
    extern int MENU_SCALE;
    extern int QLIE_SCALE;
    extern int HUD_SCALE;
    extern int MENU_BACKGROUND_ALPHA;
    extern int QLIE_BACKGROUND_ALPHA;
    extern int HUD_BACKGROUND_ALPHA;
    extern int MENU_LAYOUT;
    extern int QLIE_LAYOUT;
    extern int HUD_LAYOUT;
    extern int ROWS;
    extern int SPACING_FROM_SELECTED;
    extern int SPACING_BETWEEN_EQUIPPED_X;
    extern int SPACING_BETWEEN_EQUIPPED_Y;
    extern uint32_t COMPARE_KEY;
    extern float HOLD_THRESHOLD;
    extern float TRIPLE_HIT_WINDOW;
    extern float SETTING_HOLD_THRESHOLD;
    extern float thumbstickX;
    extern float thumbstickY;
    extern float thumbstickThreshold;
    extern std::string effectCheckOrder;
    extern int LOG_LEVEL;

    extern bool QLIE_ALLOWED;
    extern bool QLIE_SHOWHINT;
    extern bool HUD_ALLOWED;
    extern bool HUD_TOGGLEMODE;
    extern bool USE_ICONS;
    extern bool HIDE_3D;
    extern bool HIDE_SKY_UI_ITEM_CARD;

    extern RE::INPUT_DEVICE lastInputDevice;

    const int EQUIPPED_ARMOR_ITEM_ARRAY_SIZE = 8;
    const int SELECTED_ARMOR_ITEM_ARRAY_SIZE = 9;
    const int EQUIPPED_WEAPON_ITEM_ARRAY_SIZE = 10;
    const int SELECTED_WEAPON_ITEM_ARRAY_SIZE = 15;
    const std::string EXPECTED_SWF_VERSION = "5";

    void LoadConfig();

    extern std::string QLIE_HINT_TEXT;

    extern std::string buttonCompareText;
    extern std::string comparingTo;
    extern std::string damageLabelText;
    extern std::string critLabelText;
    extern std::string noneText;
    extern std::string armorTypeLabelText;
    extern std::string heavyArmor;
    extern std::string lightArmor;
    extern std::string clothArmor;
    extern std::string armorRatingLabelText;
    extern std::string unscaledText;
    extern std::string goldLabelText;
    extern std::string effectsLabelText;
    extern std::string equippedTo;
    extern std::string slotsLabelText;
    extern std::string leftHand;
    extern std::string rightHand;
    extern std::string bothHands;
    extern std::string speedLabelText;
    extern std::string reachLabelText;
    extern std::string staggerLabelText;
    extern std::string keyText;
    extern std::string keyInfo;
    extern std::string totalKey;
    extern std::string averageKey;
    extern std::string maxKey;
    extern std::string notApplicable;
    extern std::string betterWrapperStart;
    extern std::string betterWrapperEnd;
    extern std::string worseWrapperStart;
    extern std::string worseWrapperEnd;
    extern std::string normalWrapperStart;
    extern std::string normalWrapperEnd;

    extern long diffOffset;
    extern long valueOffset;
    extern long columnTwoOffset;

    void LoadTranslation();
}