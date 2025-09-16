namespace logger = SKSE::log;
using namespace CEThumbStick;

namespace CEGameEvents
{
    std::chrono::steady_clock::time_point cycleButtonLastHit;
    std::chrono::steady_clock::time_point openedMenuTime;
    bool heldTriggered = false;
    bool heldForSettingsTriggered = false;
    bool pressTwo = false;

    VirtualButton rightThumbStick;

    long long NanoToLongMilli(std::chrono::nanoseconds time_point)
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(time_point).count();
    }

    bool Is3dZoomedIn()
    {
        auto manager = RE::Inventory3DManager::GetSingleton();
        if (manager)
        {
            auto zoom = manager->GetRuntimeData().zoomProgress;
            if (zoom != 0.0f)
            {
                logger::debug("3d menu open, aborting");
                return true;
            }
        }
        return false;
    }

    void ButtonProcessor(bool btnUp, float heldDuration)
    {
        logger::trace("buttonUp : {}, heldDuration: {}", btnUp, heldDuration);
        auto currentTime = std::chrono::steady_clock::now();
        if (btnUp && heldDuration < CEGlobals::HOLD_THRESHOLD)
        {
            heldTriggered = false;
            heldForSettingsTriggered = false;
        }

        if (heldDuration >= CEGlobals::HOLD_THRESHOLD && !heldTriggered)
        {
            if (Is3dZoomedIn())
                return;
            logger::debug("Key held past threshold, setting actor to player.");
            heldTriggered = true;
            SKSE::GetTaskInterface()->AddTask([]()
                                              { 
                CEActorUtils::SetActorToPlayer();
                if (!CEGameMenuUtils::GetItem()) {
                    CEGameMenuUtils::ActorChangedUpdateMenu();
                } });

            return;
        }

        if (heldDuration >= CEGlobals::SETTING_HOLD_THRESHOLD && !heldForSettingsTriggered)
        {
            if (Is3dZoomedIn())
                return;
            heldForSettingsTriggered = true;
            SKSE::GetTaskInterface()->AddTask([]()
                                              { 
                CEMenu::DestroyMenu();
                CEGlobals::LoadConfig();
                std::thread([]()
                            {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    CEMenu::CreateMenu(CEMenu::openedMenuName); 
                    CEMenu::ShowMenuDelayed(); })
                    .detach(); });
        }

        if (btnUp && !heldForSettingsTriggered)
        {
            auto hitDiff = NanoToLongMilli(currentTime - cycleButtonLastHit);
            logger::trace("hitDiff: {}", hitDiff);
            if (hitDiff <= CEGlobals::TRIPLE_HIT_WINDOW * 500)
            {
                if (pressTwo)
                {
                    if (Is3dZoomedIn())
                        return;
                    logger::debug("Key triple tapped, cycling follower.");
                    SKSE::GetTaskInterface()->AddTask([]()
                                                      { 
                        CEActorUtils::SetActorToNextFollower();
                        if (!CEGameMenuUtils::GetItem()) {
                            CEGameMenuUtils::ActorChangedUpdateMenu();
                        } });
                    currentTime = openedMenuTime;
                    pressTwo = false;
                }
                else if (!pressTwo)
                {
                    pressTwo = true;
                }
            }
            else
            {
                if (Is3dZoomedIn())
                    return;
                logger::debug("Key pressed, getting hovered item.");
                SKSE::GetTaskInterface()->AddTask([]()
                                                  { CEGameMenuUtils::GetItem(); });
                pressTwo = false;
            }
            cycleButtonLastHit = currentTime;
        }
    }

    RE::BSEventNotifyControl InputEvent::ProcessEvent(
        RE::InputEvent *const *a_event,
        RE::BSTEventSource<RE::InputEvent *> *) noexcept
    {
        if (!a_event)
        {
            return RE::BSEventNotifyControl::kContinue;
        }

        if (auto e = *a_event; e)
        {
            auto eventType = e->GetEventType();
            if (eventType == RE::INPUT_EVENT_TYPE::kThumbstick)
            {
                if (auto id = e->AsIDEvent(); id)
                {
                    if (id->GetIDCode() == 12)
                    {
                        if (auto ts = static_cast<RE::ThumbstickEvent *>(e); ts)
                        {
                            auto state = UpdateVirtualButton(rightThumbStick, ts->xValue, ts->yValue);
                            if (state != VirtualButtonState::kUp)
                            {
                                bool buttonUp = (state != VirtualButtonState::kHeld && state != VirtualButtonState::kPressed);
                                ButtonProcessor(buttonUp, rightThumbStick.heldDownSecs);
                            }
                            return RE::BSEventNotifyControl::kContinue;
                        }
                    }
                }
            }

            if (eventType == RE::INPUT_EVENT_TYPE::kButton)
            {
                if (auto btn = e->AsButtonEvent(); btn)
                {
                    if (btn->GetIDCode() == CEGlobals::COMPARE_KEY)
                    {
                        ButtonProcessor(btn->IsUp(), btn->HeldDuration());
                        return RE::BSEventNotifyControl::kContinue;
                    }
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    InputEvent *InputEvent::GetSingleton()
    {
        static InputEvent singleton;
        return &singleton;
    }

    RE::BSEventNotifyControl DeviceInputEvent::ProcessEvent(
        RE::InputEvent *const *a_event,
        RE::BSTEventSource<RE::InputEvent *> *) noexcept
    {
        if (!a_event)
        {
            return RE::BSEventNotifyControl::kContinue;
        }

        if (auto e = *a_event; e)
        {
            CEGlobals::lastInputDevice = e->GetDevice();
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    DeviceInputEvent *DeviceInputEvent::GetSingleton()
    {
        static DeviceInputEvent singleton;
        return &singleton;
    }

    RE::BSEventNotifyControl UIEvent::ProcessEvent(
        const RE::MenuOpenCloseEvent *a_event,
        RE::BSTEventSource<RE::MenuOpenCloseEvent> *)
    {
        if (!a_event)
        {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto menuName = a_event->menuName;
        if ((menuName == RE::InventoryMenu::MENU_NAME ||
             menuName == RE::ContainerMenu::MENU_NAME ||
             menuName == RE::BarterMenu::MENU_NAME ||
             menuName == RE::GiftMenu::MENU_NAME))
        {
            if (a_event->opening)
            {
                logger::trace("{} opened", menuName);
                CEMenu::CreateMenu(menuName);
                openedMenuTime = std::chrono::steady_clock::now();
                RE::BSInputDeviceManager::GetSingleton()->AddEventSink(CEGameEvents::InputEvent::GetSingleton());
            }
            else
            {
                logger::trace("{} closed", menuName);
                CEMenu::DestroyMenu();
                RE::BSInputDeviceManager::GetSingleton()->RemoveEventSink(CEGameEvents::InputEvent::GetSingleton());
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    UIEvent *UIEvent::GetSingleton()
    {
        static UIEvent singleton;
        return &singleton;
    }
}