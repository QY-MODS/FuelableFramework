#pragma once

#include "Manager.h"



class OurEventSink : public RE::BSTEventSink<RE::TESEquipEvent>
{
    OurEventSink(LightSourceManager* a_LSM) : LSM(a_LSM) {}
    OurEventSink(const OurEventSink&) = delete;
    OurEventSink(OurEventSink&&) = delete;
    OurEventSink& operator=(const OurEventSink&) = delete;
    OurEventSink& operator=(OurEventSink&&) = delete;

public:
    static OurEventSink* GetSingleton(LightSourceManager* a_LSM) {
        static OurEventSink singleton(a_LSM);
        return &singleton;
    }

    LightSourceManager* LSM;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* event, RE::BSTEventSource<RE::TESEquipEvent>*);

};