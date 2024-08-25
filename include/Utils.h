#pragma once

#include <windows.h>
#include "ClibUtil/editorID.hpp"

const auto mod_name = static_cast<std::string>(SKSE::PluginDeclaration::GetSingleton()->GetName());

const auto no_src_msgbox = std::format(
    "{}: You currently do not have any sources set up. Check your ini file or see the mod page for instructions.",
    mod_name);
const auto po3_err_msgbox = std::format(
    "{}: You have given an invalid FormID. If you are using Editor IDs, you must have powerofthree's Tweaks installed. "
    "See mod page for further instructions.",
    mod_name);
const auto general_err_msgbox = std::format("{}: Something went wrong. Please contact the mod author.", mod_name);
const auto init_err_msgbox = std::format("{}: The mod failed to initialize and will be terminated.", mod_name);
const auto load_order_msgbox = std::format(
    "The equipped light source from your save game could not be registered. Please unequip and reequip it. If you had "
    "fuel in it, it will be lost. This issue will be solved in the next version.");



template <class T = RE::TESForm>
static T* GetFormByID(const RE::FormID id, const std::string& editor_id = "") {
    if (!editor_id.empty()) {
        if (auto* form = RE::TESForm::LookupByEditorID<T>(editor_id)) return form;
    }
    if (T* form = RE::TESForm::LookupByID<T>(id)) return form;
    return nullptr;
};


namespace Utilities{

    const auto mod_name = static_cast<std::string>(SKSE::PluginDeclaration::GetSingleton()->GetName());

    constexpr auto path = L"Data/SKSE/Plugins/FuelableFramework.ini";
    constexpr auto po3path = "Data/SKSE/Plugins/po3_Tweaks.dll";
    
    inline bool IsPo3Installed() { return std::filesystem::exists(po3path); };

    float Round(float number, int decimalPlaces);
    int Round2Int(float number);

    bool isValidHexWithLength7or8(const char* input);
    std::string DecodeTypeCode(std::uint32_t typeCode);

    std::filesystem::path GetLogPath();

    std::vector<std::string> ReadLogFile();

    namespace String {

        template <typename T>
        std::string join(const T& container, const std::string_view& delimiter) {
            std::ostringstream oss;
            auto iter = container.begin();

            if (iter != container.end()) {
                oss << *iter;
                ++iter;
            }

            for (; iter != container.end(); ++iter) {
                oss << delimiter << *iter;
            }

            return oss.str();
        }

    };
};

namespace MsgBoxesNotifs {
    /*void FormIDError(const RE::FormID id);
    void EditorIDError(const std::string& editor_id);*/
    namespace InGame {
        inline void InitErr() { RE::DebugMessageBox(init_err_msgbox.c_str()); };
        
        inline void LoadOrderError() {
            RE::DebugMessageBox((std::format("{}: ", mod_name) + load_order_msgbox).c_str());
        }

        inline void NoFuel(std::string_view item, std::string_view fuel) {
            RE::DebugNotification(std::format("I need {} to fuel my {}.", fuel, item).c_str());
        }
        
        inline void Refuel(std::string_view item, std::string_view fuel) {
            RE::DebugNotification(std::format("Adding {} to my {}...", fuel, item).c_str());
        }

        void Remaining(int remaining, std::string_view item);

        inline void NoSourceFound() { RE::DebugMessageBox(no_src_msgbox.c_str()); };
    };

    namespace Windows {

        inline int Po3ErrMsg() {
            MessageBoxA(nullptr, po3_err_msgbox.c_str(), "Error", MB_OK | MB_ICONERROR);
            return 1;
        };

        inline int GeneralErr() {
            MessageBoxA(nullptr, general_err_msgbox.c_str(), "Error", MB_OK | MB_ICONERROR);
            return 1;
        };
    };
};

float almost_constant_with_accelerated_cutoff(float x, float start_value = 1.0f, float cutoff = 0.70f,
                                                     int power = 10);

RE::FormID GetFormEditorIDFromString(const std::string& formEditorId);

