namespace logger = SKSE::log;

namespace CEGameEvents
{
    std::chrono::steady_clock::time_point cycleButtonDownStart;
    std::chrono::steady_clock::time_point cycleButtonLastHit;
    bool cycleButtonHit = false;
    bool pressOne = false;
    bool pressTwo = false;

    std::chrono::steady_clock::time_point GetNow()
    {
        return std::chrono::steady_clock::now();
    }

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
            logger::trace("CEGameEvents: Invalid input event.");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (auto e = *a_event; e)
        {
            auto eventType = e->GetEventType();
            if (eventType == RE::INPUT_EVENT_TYPE::kButton)
            {
                if (auto btn = e->AsButtonEvent(); btn)
                {
                    if (btn->GetIDCode() == CEGlobals::COMPARE_KEY && !btn->IsUp())
                    {
                        logger::trace("CEGameEvents: Compare Key pressed, getting hovered item.");
                        SKSE::GetTaskInterface()->AddTask([]()
                                                          { CEGameMenuUtils::GetItem(); });
                    }

                    if (btn->GetIDCode() == CEGlobals::CYCLE_KEY)
                    {
                        if (btn->IsUp())
                        {
                            auto upNow = GetNow();
                            auto hitDiff = NanoToLongMilli(upNow - cycleButtonLastHit);
                            if (hitDiff <= CEGlobals::TRIPLE_HIT_TIME.count() / 2)
                            {
                                if (pressOne && pressTwo)
                                    logger::info("Triple hit detected");
                                if (!pressTwo && pressOne)
                                    pressTwo = true;
                                if (!pressOne)
                                    pressOne = true;
                            }
                            else
                            {
                                pressOne = false;
                                pressTwo = false;
                            }
                        }
                        if (!btn->IsUp() && !cycleButtonHit)
                        {
                            logger::trace("CEGameEvents: Cycle key pressed, setting start time.");
                            cycleButtonHit = true;
                            cycleButtonDownStart = GetNow();
                            return RE::BSEventNotifyControl::kContinue;
                        }

                        auto cycleButtonReleased = GetNow();
                        auto heldDuration = NanoToLongMilli(cycleButtonReleased - cycleButtonDownStart);

                        if (!btn->IsUp() && heldDuration < CEGlobals::HOLD_THRESHOLD.count())
                            return RE::BSEventNotifyControl::kContinue;

                        if (btn->IsUp())
                            cycleButtonHit = false;

                        if (heldDuration >= CEGlobals::HOLD_THRESHOLD.count())
                        {
                            logger::trace("CEGameEvents: Cycle key held past threshold, setting actor to player.");
                            SKSE::GetTaskInterface()->AddTask([]()
                                                              { CEActorUtils::SetActorToPlayer();
                                                                if (!CEGameMenuUtils::GetItem()) {
                                                                    CEGameMenuUtils::ActorChangedUpdateMenu();
                                                                } });
                        }
                        else
                        {
                            logger::trace("CEGameEvents: Cycle key pressed, cycling Actor.");
                            SKSE::GetTaskInterface()->AddTask([]()
                                                              { CEActorUtils::SetActorToNextFollower();
                                                                if (!CEGameMenuUtils::GetItem()) {
                                                                    CEGameMenuUtils::ActorChangedUpdateMenu();
                                                                } });
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
                logger::trace("CEGameEvents: {} opened", menuName);
                CEMenu::CreateMenu(menuName);
                RE::BSInputDeviceManager::GetSingleton()->AddEventSink(CEGameEvents::InputEvent::GetSingleton());
            }
            else
            {
                logger::trace("CEGameEvents: {} closed", menuName);
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