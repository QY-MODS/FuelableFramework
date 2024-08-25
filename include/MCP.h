#pragma once
#include "Manager.h"
#include "SKSEMCP/SKSEMenuFramework.hpp"

static void HelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}


namespace UI {

    inline std::string ql_fuel_name = "";
    inline std::vector<UISource> ui_sources;

    inline bool log_trace = true;
    inline bool log_info = true;
    inline bool log_warning = true;
    inline bool log_error = true;
    inline std::vector<std::string> log_lines;

    inline LightSourceManager* M;

    void __stdcall RenderStatus();
    void __stdcall RenderQuickLight();
    void __stdcall RenderLog();
    void Register(LightSourceManager* manager);

};