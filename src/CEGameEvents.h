#pragma once
namespace CEGameEvents
{
    long long NanoToLongMilli(std::chrono::nanoseconds time_point);

    extern class InputEvent : public RE::BSTEventSink<RE::InputEvent *>
    {
    public:
        virtual RE::BSEventNotifyControl ProcessEvent(
            RE::InputEvent *const *a_event,
            RE::BSTEventSource<RE::InputEvent *> *) noexcept override;
        static InputEvent *GetSingleton();
    };

    extern class DeviceInputEvent : public RE::BSTEventSink<RE::InputEvent *>
    {
    public:
        virtual RE::BSEventNotifyControl ProcessEvent(
            RE::InputEvent *const *a_event,
            RE::BSTEventSource<RE::InputEvent *> *) noexcept override;
        static DeviceInputEvent *GetSingleton();
    };

    extern class UIEvent : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {
    public:
        virtual RE::BSEventNotifyControl ProcessEvent(
            const RE::MenuOpenCloseEvent *a_event,
            RE::BSTEventSource<RE::MenuOpenCloseEvent> *) override;
        static UIEvent *GetSingleton();
    };

    void QuickLootSelectItemHandler(QuickLoot::Events::SelectItemEvent *event);

    void QuickLootCloseHandler(QuickLoot::Events::CloseLootMenuEvent *);

    void QuickLootOpenHandler(QuickLoot::Events::OpenLootMenuEvent *event);
}