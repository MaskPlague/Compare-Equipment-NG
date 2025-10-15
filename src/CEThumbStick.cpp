namespace CEThumbStick
{
    VirtualButtonState UpdateVirtualButton(VirtualButton &vb, float valueX, float valueY)
    {
        bool down = (valueX * CEGlobals::thumbstickX + valueY * CEGlobals::thumbstickY) >= CEGlobals::thumbstickThreshold;
        if (down)
        {
            if (!vb.isDown)
            {
                vb.downStart = std::chrono::steady_clock::now();
                vb.state = VirtualButtonState::kPressed;
            }
            else
            {
                vb.heldDownSecs = static_cast<float>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - vb.downStart)
                        .count() *
                    0.001);
                vb.state = VirtualButtonState::kHeld;
            }
        }
        else
        {
            if (vb.isDown)
            {
                vb.heldDownSecs = 0.0f;
                vb.state = VirtualButtonState::kReleased;
            }
            else
            {
                vb.state = VirtualButtonState::kUp;
            }
        }

        vb.isDown = down;
        return vb.state;
    }
}