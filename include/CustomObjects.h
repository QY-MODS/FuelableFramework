#pragma once
#include "Utils.h"

//struct Float3 {
//    float x, y, z;
//
//    Float3(float x, float y, float z) : x(x), y(y), z(z){
//        x = std::max(x, 0.f);
//        y = std::max(y, 0.f);
//        z = std::max(z, 0.f);
//    };
//
//    Float3() : x(0.f), y(0.f), z(0.f) {}
//
//    void operator=(const Float3& other) {
//        x = std::max(other.x,0.f);
//        y = std::max(other.y,0.f);
//        z = std::max(other.z,0.f);
//    };
//    
//    void operator=(const RE::NiPoint3& other) {
//        x = std::max(other.x, 0.f);
//        y = std::max(other.y, 0.f);
//        z = std::max(other.z, 0.f);
//    };
//
//    float GetLength() const { return std::sqrt(x * x + y * y + z * z); };
//};

struct FormID2 {
    uint32_t outerKey;
    uint32_t innerKey;

    // Comparison operator for using Key as std::map key
    bool operator<(const FormID2& other) const {
        return outerKey < other.outerKey || (outerKey == other.outerKey && innerKey < other.innerKey);
    }
};


struct LightSource {
    const float duration;
    std::uint32_t formid;
    const std::string editorid;
    std::uint32_t fuel;
    const std::string fuel_editorid;

    bool init_failed=false;

    LightSource(std::uint32_t id, const std::string id_str, float duration, std::uint32_t fuelid,
                const std::string fuelid_str)
        : formid(id), editorid(id_str), duration(duration), fuel(fuelid), fuel_editorid(fuelid_str){
        if (!formid) {
            if (auto form = RE::TESForm::LookupByEditorID<RE::TESForm>(editorid)) {
                logger::info("Found formid for editorid {}", editorid);
                formid = form->GetFormID();
            } else logger::info("Could not find formid for editorid {}", editorid);
        }
        if (!fuel) {
            if (auto form = RE::TESForm::LookupByEditorID<RE::TESForm>(fuel_editorid)) {
                logger::info("Found formid2 for editorid2 {}", editorid);
                fuel = form->GetFormID();
            } else logger::info("Could not find formid for editorid {}", editorid);
        }

        if (!GetBoundObject() || !GetBoundFuelObject()) InitFailed();
    };

    bool operator < (const LightSource& other) const { return formid < other.formid; }


    float remaining = 0.f;
    float elapsed = 0.f;

    std::string_view GetName(){
        if (auto form = GetFormByID(formid, editorid)) return form->GetName();
        else return "";
    };

    std::string_view GetFuelName(){
        if (auto form = GetFormByID(fuel, fuel_editorid)) return form->GetName();
        else return "";
    };

    RE::TESBoundObject* GetBoundObject() { return GetFormByID<RE::TESBoundObject>(formid, editorid); };

    RE::TESBoundObject* GetBoundFuelObject() { return GetFormByID<RE::TESBoundObject>(fuel, fuel_editorid); };


private:
    void InitFailed() {
        init_failed = true;
        logger::error("Failed to initialize LightSource with formid {} and fuelid {}", formid, fuel);
    }
};

struct UISource {
    std::string name;
    std::string fuel_name;
    std::string duration;
};