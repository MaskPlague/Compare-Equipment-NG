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
            if (CEMenu::Is3dZoomedIn())
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
            if (CEMenu::Is3dZoomedIn())
                return;
            heldForSettingsTriggered = true;
            SKSE::GetTaskInterface()->AddTask([]()
                                              { 
                CEMenu::DestroyMenu(CEMenu::openedMenuName);
                CEGlobals::LoadConfig();
                RE::DebugNotification(CEGlobals::settingsReloaded.c_str());
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
                    if (CEMenu::Is3dZoomedIn())
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
                if (CEMenu::Is3dZoomedIn())
                    return;
                SKSE::GetTaskInterface()->AddTask([]()
                                                  { 
                                                    if (CEGlobals::MENU_PERSISTENT_DISPLAY && CEMenu::openedMenuName != "LootMenu" && CEMenu::openedMenuName != "HUDMenu")
                                                        CEMenu::menuPersistentToggledOn = true;
                                                    else if(CEGlobals::QLIE_PERSISTENT_DISPLAY && CEMenu::openedMenuName == "LootMenu")
                                                        CEMenu::qliePersistentToggledOn = true;
                                                    CEGameMenuUtils::GetItem(); });
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
            if (!pickData || !target || !target->get())
                return RE::BSEventNotifyControl::kContinue;
        }

        if (auto e = *a_event; e)
        {
            auto eventType = e->GetEventType();
            if (eventType == RE::INPUT_EVENT_TYPE::kThumbstick && CEMenu::openedMenuName != "HUDMenu" && CEMenu::openedMenuName != "LootMenu")
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
                    auto device = e->GetDevice();

                    if (device != RE::INPUT_DEVICE::kGamepad && btn->GetIDCode() == CEGlobals::COMPARE_KEY)
                    {
                        ButtonProcessor(btn->IsUp(), btn->HeldDuration());
                        return RE::BSEventNotifyControl::kContinue;
                    }
                    else if (device == RE::INPUT_DEVICE::kGamepad &&
                             ((CEGlobals::HUD_ALLOWED && CEMenu::openedMenuName == "HUDMenu") ||
                              (CEGlobals::QLIE_ALLOWED && CEMenu::openedMenuName == "LootMenu")) &&
                             btn->GetIDCode() == CEGlobals::CONTROLLER_KEY)
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
        logger::trace("Menu name: {}", menuName.c_str());
        if ((menuName == RE::InventoryMenu::MENU_NAME ||
             menuName == RE::ContainerMenu::MENU_NAME ||
             menuName == RE::BarterMenu::MENU_NAME ||
             menuName == RE::GiftMenu::MENU_NAME ||
             (CEGlobals::QLIE_ALLOWED && menuName == "LootMenu")))
        {
            if (a_event->opening)
            {
                logger::trace("{} opened", menuName.c_str());
                CEMenu::openedMenus.insert(menuName.c_str());
                CEMenu::CreateMenu(menuName);
                openedMenuTime = std::chrono::steady_clock::now();
            }
            else
            {
                logger::trace("{} closed", menuName.c_str());
                CEMenu::openedMenus.erase(menuName.c_str());
                CEMenu::DestroyMenu(menuName.c_str());
                if (CEMenu::openedMenus.empty())
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
        if (NanoToLongMilli(std::chrono::steady_clock::now() - openedMenuTime) >= 15)
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
        if (CEGlobals::QLIE_PERSISTENT_DISPLAY && CEGameMenuUtils::isWeaponOrArmor(fid))
            CEMenu::PersistentDisplayRun(true);
        else
            CEMenu::HideMenu();
        MaybeShowHideHint();
    }

    void QuickLootCloseHandler(QuickLoot::Events::CloseLootMenuEvent *)
    {
        if (!CEGlobals::QLIE_ALLOWED)
            return;
        logger::debug("QuickLoot Close Event triggered");
        CEMenu::HideMenu();
        CEGameMenuUtils::currentFormID = NULL;
        MaybeShowHideHint();
    }

    void QuickLootOpenHandler(QuickLoot::Events::OpenLootMenuEvent *event)
    {
        if (!CEGlobals::QLIE_ALLOWED || !event || !event->container)
            return;
        logger::debug("QuickLoot Open Event triggered");
        try
        {
            CEGameMenuUtils::containerInventoryQLIE = event->container->GetInventory();
        }
        catch (...)
        {
            logger::debug("QLIE OpenEventHandler: Failed to get container inventory.");
            return;
        }
        std::vector<std::pair<std::string, RE::FormID>> objects;
        for (auto &[item, data] : CEGameMenuUtils::containerInventoryQLIE)
        {
            if (item && data.second)
            {
                RE ::InventoryEntryData *entryData;
                try
                {
                    entryData = data.second.get();
                }
                catch (...)
                {
                    logger::debug("QLIE OpenEventHandler: Failed to get data.second.get()");
                    continue;
                }
                if (entryData)
                {
                    try
                    {
                        std::string name = entryData->GetDisplayName();
                        RE::FormID fid = item->GetFormID();
                        if (!name.empty() && std::strcmp(name.c_str(), "<Missing Name>") != 0 && fid)
                            objects.push_back(std::make_pair(name, fid));
                    }
                    catch (...)
                    {
                        logger::debug("QLIE OpenEventHandler: Failed to get DisplayName or FID");
                        continue;
                    }
                }
            }
        }
        if (objects.size() <= 0)
            return;
        std::sort(objects.begin(), objects.end(), [](const auto &a, const auto &b)
                  { return a.first < b.first; });
        auto firstObj = objects.at(0);
        CEGameMenuUtils::currentFormID = firstObj.second;
        MaybeShowHideHint();
        if (CEGlobals::QLIE_PERSISTENT_DISPLAY && CEGameMenuUtils::isWeaponOrArmor(CEGameMenuUtils::currentFormID))
            CEMenu::PersistentDisplayRun(true);
    }
}