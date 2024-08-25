#include "Events.h"

RE::BSEventNotifyControl OurEventSink::ProcessEvent(const RE::TESEquipEvent* event,
                                                    RE::BSTEventSource<RE::TESEquipEvent>*) {
    if (!event) return RE::BSEventNotifyControl::kContinue;
    // if (!LSM->allow_equip_event_sink) return RE::BSEventNotifyControl::kContinue;
    if (!LSM->IsValidSource(event->baseObject)) return RE::BSEventNotifyControl::kContinue;
    LSM->HandleEquip(event->baseObject, event->equipped);
    return RE::BSEventNotifyControl::kContinue;
}