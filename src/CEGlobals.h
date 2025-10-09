#pragma once

namespace CEGlobals
{
    extern double MENU_X_ORIGIN;
    extern double MENU_Y_ORIGIN;
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
    extern int ROWS;
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

    extern RE::INPUT_DEVICE lastInputDevice;

    const int EQUIPPED_ARMOR_ITEM_ARRAY_SIZE = 6;
    const int SELECTED_ARMOR_ITEM_ARRAY_SIZE = 8;
    const int EQUIPPED_WEAPON_ITEM_ARRAY_SIZE = 9;
    const int SELECTED_WEAPON_ITEM_ARRAY_SIZE = 14;
    const std::string EXPECTED_SWF_VERSION = "1";

    void LoadConfig();
}