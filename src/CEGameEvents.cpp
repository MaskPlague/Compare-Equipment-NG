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
                return true;
            }
        }
        return false;
    }

    void ButtonProcessor(bool btnUp, float heldDuration)
    {
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
                std::string menuToDestroy{CEMenu::openedMenuName};
                CEMenu::DestroyMenu(menuToDestroy);
                CEGlobals::LoadConfig();
                std::thread([]()
                            {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    SKSE::GetTaskInterface()->AddUITask([]()
                                                        {
                        CEMenu::CreateMenu(CEMenu::openedMenuName); 
                        CEMenu::ShowMenuDelayed(); });
                    })
                    .detach(); });
        }

        if (btnUp && !heldForSettingsTriggered)
        {
            auto hitDiff = NanoToLongMilli(currentTime - cycleButtonLastHit);
            if (hitDiff <= CEGlobals::TRIPLE_HIT_WINDOW * 500)
            {
                if (pressTwo)
                {
                    if (Is3dZoomedIn())
                        return;
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
        if (!a_event || (!CEGlobals::HUD_ALLOWED && CEMenu::openedMenuName == "HUDMenu") || (!CEGlobals::QLIE_ALLOWED && CEMenu::openedMenuName == "LootMenu"))
        {
            return RE::BSEventNotifyControl::kContinue;
        }

        if (CEMenu::openedMenuName == "HUDMenu")
        {
            auto pickData = RE::CrosshairPickData::GetSingleton();
            auto target = pickData->target;
            if (!pickData || !target || !target.get())
                return RE::BSEventNotifyControl::kContinue;
        }

        if (auto e = *a_event; e)
        {
            auto eventType = e->GetEventType();
            if (eventType == RE::INPUT_EVENT_TYPE::kThumbstick && CEMenu::openedMenuName != "HUDMenu")
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
        logger::trace("Menu name: {}", menuName);
        if ((menuName == RE::InventoryMenu::MENU_NAME ||
             menuName == RE::ContainerMenu::MENU_NAME ||
             menuName == RE::BarterMenu::MENU_NAME ||
             menuName == RE::GiftMenu::MENU_NAME ||
             (CEGlobals::QLIE_ALLOWED && menuName == "LootMenu")))
        {
            if (a_event->opening)
            {
                logger::trace("{} opened", menuName);
                CEMenu::openedMenus.insert(menuName.c_str());
                CEMenu::CreateMenu(menuName);
                openedMenuTime = std::chrono::steady_clock::now();
            }
            else
            {
                logger::trace("{} closed", menuName);
                CEMenu::openedMenus.erase(menuName.c_str());
                CEMenu::DestroyMenu(menuName.c_str());
                if (CEGlobals::HUD_ALLOWED && CEMenu::openedMenus.empty())
                    CEMenu::CreateMenu("HUDMenu");
                else if (CEMenu::openedMenus.empty())
                    CEMenu::openedMenuName = "HUDMenu";
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    UIEvent *UIEvent::GetSingleton()
    {
        static UIEvent singleton;
        return &singleton;
    }

    void MaybeShowHideHint()
    {
        if (NanoToLongMilli(std::chrono::steady_clock::now() - openedMenuTime) > 50)
        {
            openedMenuTime = std::chrono::steady_clock::now();
            CEMenu::ShowOrHideQLIEHint();
        }
    }

    void QuickLootSelectItemHandler(QuickLoot::Events::SelectItemEvent *event)
    {
        if (!CEGlobals::QLIE_ALLOWED)
            return;
        logger::debug("QuickLoot Select Item Event triggered");
        if (!event->elements->object || !event->elements->object->formID)
            return;
        auto fid = event->elements->object->formID;
        auto form = RE::TESForm::LookupByID(fid);
        if (!form)
            return;
        CEGameMenuUtils::currentFormID = fid;
        CEMenu::HideMenu();
        MaybeShowHideHint();
    }

    void QuickLootCloseHandler(QuickLoot::Events::CloseLootMenuEvent *)
    {
        if (!CEGlobals::QLIE_ALLOWED)
            return;
        CEMenu::HideMenu();
        CEGameMenuUtils::currentFormID = NULL;
        MaybeShowHideHint();
    }

    void QuickLootOpenHandler(QuickLoot::Events::OpenLootMenuEvent *event)
    {
        if (!CEGlobals::QLIE_ALLOWED)
            return;
        logger::debug("QuickLoot Open Event triggered");
        auto cont = event->container->GetBaseObject()->As<RE::TESObjectCONT>();
        int count = cont ? cont->numContainerObjects : 0;
        if (count <= 0)
            return;
        std::vector<std::pair<std::string, RE::FormID>> objects;
        for (int i = 0; i < count; i++)
        {
            std::optional<RE::ContainerObject *> oCObj = cont->GetContainerObjectAt(i);
            RE::ContainerObject *cobj = oCObj ? oCObj.value() : nullptr;
            RE::TESBoundObject *obj = cobj ? cobj->obj : nullptr;
            if (!oCObj || !cobj || !obj)
                continue;
            std::string name = obj->GetName();
            RE::FormID fid = obj->GetFormID();
            if (!name.empty() && fid)
                objects.push_back(std::make_pair(name, fid));
        }
        if (objects.size() <= 0)
            return;
        std::sort(objects.begin(), objects.end(), [](const auto &a, const auto &b)
                  { return a.first < b.first; });
        auto firstObj = objects.at(0);
        CEGameMenuUtils::currentFormID = firstObj.second;
        MaybeShowHideHint();
    }
}