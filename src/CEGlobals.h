#pragma once

namespace CEGlobals
{
    extern std::string EXPECTED_SWF_VERSION;
    extern double X_ORIGIN;
    extern double Y_ORIGIN;
    extern int SCALE;
    extern int BACKGROUND_ALPHA;
    extern uint32_t COMPARE_KEY;
    extern uint32_t CYCLE_KEY;
    extern std::chrono::milliseconds HOLD_THRESHOLD;
    extern std::chrono::milliseconds TRIPLE_HIT_WINDOW;
    extern std::chrono::milliseconds SETTING_HOLD_THRESHOLD;
    extern int LOG_LEVEL;

    const int EQUIPPED_ITEM_ARRAY_SIZE = 6;
    const int SELECTED_ITEM_ARRAY_SIZE = 8;

    void LoadConfig();
}