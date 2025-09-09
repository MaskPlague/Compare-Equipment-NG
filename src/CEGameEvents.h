#pragma once
namespace CEGameEvents
{
    extern std::chrono::steady_clock::time_point cycleButtonDownStart;
    extern std::chrono::steady_clock::time_point cycleButtonLastHit;
    extern bool cycleButtonHit;

    extern class InputEvent : public RE::BSTEventSink<RE::InputEvent *>
    {
    public:
        virtual RE::BSEventNotifyControl ProcessEvent(
            RE::InputEvent *const *a_event,
            RE::BSTEventSource<RE::InputEvent *> *) noexcept override;
        static InputEvent *GetSingleton();
    };

    extern class UIEvent : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {
    public:
        virtual RE::BSEventNotifyControl ProcessEvent(
            const RE::MenuOpenCloseEvent *a_event,
            RE::BSTEventSource<RE::MenuOpenCloseEvent> *) override;
        static UIEvent *GetSingleton();
    };

}