namespace logger = SKSE::log;

namespace CEGameEvents
{
    std::chrono::steady_clock::time_point cycleButtonDownStart;
    std::chrono::steady_clock::time_point cycleButtonDownForSettingsStart;
    std::chrono::steady_clock::time_point cycleButtonLastHit;
    std::chrono::steady_clock::time_point openedMenuTime;
    bool cycleButtonHeld = false;
    bool heldTriggered = false;
    bool cycleButtonHeldForSettings = false;
    bool heldForSettingsTriggered = false;
    bool pressTwo = false;

    long long NanoToLongMilli(std::chrono::nanoseconds time_point)
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(time_point).count();
    }

    RE::BSEventNotifyControl InputEvent::ProcessEvent(
        RE::InputEvent *const *a_event,
        RE::BSTEventSource<RE::InputEvent *> *) noexcept
    {
        if (!a_event)
        {
            logger::trace("Invalid input event.");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (auto e = *a_event; e)
        {
            auto eventType = e->GetEventType();
            if (eventType == RE::INPUT_EVENT_TYPE::kButton)
            {
                if (auto btn = e->AsButtonEvent(); btn)
                {
                    if (btn->GetIDCode() == CEGlobals::COMPARE_KEY)
                    {
                        auto currentTime = std::chrono::steady_clock::now();
                        if (!btn->IsUp() && !cycleButtonHeld && !cycleButtonHeldForSettings)
                        {
                            cycleButtonHeld = true;
                            heldTriggered = false;
                            cycleButtonDownStart = currentTime;
                        }

                        if (!btn->IsUp() && !cycleButtonHeldForSettings)
                        {
                            cycleButtonHeldForSettings = true;
                            heldForSettingsTriggered = false;
                            cycleButtonDownForSettingsStart = currentTime;
                        }

                        auto heldDuration = NanoToLongMilli(currentTime - cycleButtonDownStart);
                        if (heldDuration >= CEGlobals::HOLD_THRESHOLD.count() && !heldTriggered)
                        {
                            logger::trace("Key held past threshold, setting actor to player.");
                            heldTriggered = true;
                            SKSE::GetTaskInterface()->AddTask([]()
                                                              { CEActorUtils::SetActorToPlayer();
                                                                if (!CEGameMenuUtils::GetItem()) {
                                                                    CEGameMenuUtils::ActorChangedUpdateMenu();
                                                                } });

                            return RE::BSEventNotifyControl::kContinue;
                        }

                        auto heldDurationForSettings = NanoToLongMilli(currentTime - cycleButtonDownForSettingsStart);
                        if (heldDurationForSettings >= CEGlobals::GET_SETTINGS_THRESHOLD.count() && !heldForSettingsTriggered)
                        {
                            heldForSettingsTriggered = true;
                            SKSE::GetTaskInterface()->AddTask([]()
                                                              { CEGlobals::LoadConfig(); 
                                                                CEMenu::DestroyMenu();
                                                                CEMenu::CreateMenu(CEMenu::openedMenuName); 
                                                                CEMenu::ShowMenu(); });
                        }

                        if (btn->IsUp() && !heldForSettingsTriggered)
                        {
                            cycleButtonHeld = false;
                            cycleButtonHeldForSettings = false;
                            auto hitDiff = NanoToLongMilli(currentTime - cycleButtonLastHit);
                            logger::trace("hitDiff: {}", hitDiff);
                            if (hitDiff <= CEGlobals::TRIPLE_HIT_WINDOW.count() / 2)
                            {
                                if (pressTwo)
                                {
                                    logger::trace("Key triple tapped, cycling follower.");
                                    SKSE::GetTaskInterface()->AddTask([]()
                                                                      { CEActorUtils::SetActorToNextFollower();
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
                                logger::trace("Key pressed, getting hovered item.");
                                SKSE::GetTaskInterface()->AddTask([]()
                                                                  { CEGameMenuUtils::GetItem(); });
                                pressTwo = false;
                            }
                            cycleButtonLastHit = currentTime;
                        }
                        else if (btn->IsUp() && heldForSettingsTriggered)
                        {
                            cycleButtonHeld = false;
                            cycleButtonHeldForSettings = false;
                        }
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