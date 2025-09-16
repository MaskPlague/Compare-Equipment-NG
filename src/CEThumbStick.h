#pragma once

namespace CEThumbStick
{
    enum class VirtualButtonState
    {
        kUp,      // released
        kPressed, // just pressed
        kHeld,    // being held
        kReleased // just released
    };

    struct VirtualButton
    {
        bool isDown = false;
        std::chrono::steady_clock::time_point downStart;
        float heldDownSecs = 0.0f;
        VirtualButtonState state = VirtualButtonState::kUp;
        bool IsUp() { return (state != VirtualButtonState::kPressed && state != VirtualButtonState::kHeld); }
    };

    VirtualButtonState UpdateVirtualButton(VirtualButton &vb, float valueX, float valueY);
}