#include "MCP.h"

void __stdcall UI::RenderStatus(){
    if (M->is_healthy) ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Working.");
    else ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not working.");
    
    if (ImGui::Checkbox("Mute Notifications", &Settings::enabled_plyrmsg)) {
        CSimpleIniA ini;
        ini.SetUnicode();
        ini.LoadFile(Utilities::path);
        ini.SetBoolValue(Settings::InISections[0], "DepleteReplenishMessages", Settings::enabled_plyrmsg);
        ini.SaveFile(Utilities::path);
    }

    // list all sources
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("tablesources", 3, flags)) {
        ImGui::TableSetupColumn("Light Source");
        ImGui::TableSetupColumn("Fuel");
        ImGui::TableSetupColumn("Duration");
        ImGui::TableHeadersRow();
        for (const auto& item : ui_sources) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(item.name.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(item.fuel_name.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text(item.duration.c_str());
            //ImGui::TableNextColumn();
        };
        ImGui::EndTable();
    }

    


};

void __stdcall UI::RenderQuickLight() {
    // apply changes to duration and radius

    if (Settings::ql_enabled) ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Enabled.");
    else ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Disabled.");
    ImGui::Text(std::format("Fuel: {}", ql_fuel_name).c_str());

    if (!Settings::ql_enabled) return;

    // add button to save settings
    if (ImGui::Button("Save##QL")) {
        CSimpleIniA ini;
        ini.SetUnicode();
        ini.LoadFile(Utilities::path);
        ini.SetValue(Settings::InISections[4], "Duration", std::to_string(Settings::ql_duration).c_str());
        ini.SaveFile(Utilities::path);
    }

    float temp_duration = Settings::ql_duration;
    if (ImGui::InputFloat("Duration", &temp_duration, 0.1f, 1.0f, "%.1f")) {
        Settings::ql_duration = std::clamp(temp_duration, 0.1f, 1500.0f);
    }

    /*if (ImGui::SliderFloat("RadiusX", &Settings::ql_radius.x, 0.0f, 1000.0f, "%.1f")) {
        if (auto* ql_eff = RE::TESForm::LookupByID<RE::LightEffect>(Settings::ql_formid)) {
            auto& radius = ql_eff->light->GetLightRuntimeData().radius;
            radius.x = Settings::ql_radius.x;
        };
    }
    if (ImGui::SliderFloat("RadiusY", &Settings::ql_radius.y, 0.0f, 1000.0f, "%.1f")) {
        if (auto* ql_eff = RE::TESForm::LookupByID<RE::LightEffect>(Settings::ql_formid)) {
            auto& radius = ql_eff->light->GetLightRuntimeData().radius;
            radius.y = Settings::ql_radius.y;
        };
    }
    if (ImGui::SliderFloat("RadiusZ", &Settings::ql_radius.z, 0.0f, 1000.0f, "%.1f")) {
        if (auto* ql_eff = RE::TESForm::LookupByID<RE::LightEffect>(Settings::ql_formid)) {
            auto& radius = ql_eff->light->GetLightRuntimeData().radius;
            radius.z = Settings::ql_radius.z;
        };
    }*/
}
void __stdcall UI::RenderLog() {
    ImGui::Checkbox("Trace", &log_trace);
    ImGui::SameLine();
    ImGui::Checkbox("Info", &log_info);
    ImGui::SameLine();
    ImGui::Checkbox("Warning", &log_warning);
    ImGui::SameLine();
    ImGui::Checkbox("Error", &log_error);

    if (ImGui::Button("Generate Log")) log_lines = Utilities::ReadLogFile();

    for (const auto& line : log_lines) {
        if (line.find("trace") != std::string::npos && !log_trace) continue;
        if (line.find("info") != std::string::npos && !log_info) continue;
        if (line.find("warning") != std::string::npos && !log_warning) continue;
        if (line.find("error") != std::string::npos && !log_error) continue;
        ImGui::Text(line.c_str());
    }
};

void UI::Register(LightSourceManager* manager) {
    if (!SKSEMenuFramework::IsInstalled()) {
        logger::warn("SKSEMenuFramework not installed, MCP will not work.");
        return;
    }

    SKSEMenuFramework::SetSection(Utilities::mod_name);
    SKSEMenuFramework::AddSectionItem("Status", RenderStatus);
    SKSEMenuFramework::AddSectionItem("QuickLight", RenderQuickLight);
    SKSEMenuFramework::AddSectionItem("Log", RenderLog);
    M = manager;

    if (Settings::ql_enabled) {
        ql_fuel_name = std::string(RE::TESForm::LookupByID(Settings::ql_fuel)->GetName());
    }

    for (const auto& source : M->sources) {
        const auto source_name = std::string(RE::TESForm::LookupByID(source.formid)->GetName());
        const auto fuel_name = std::string(RE::TESForm::LookupByID(source.fuel)->GetName());
        const auto duration_str = std::to_string(source.duration);
        ui_sources.push_back({source_name, fuel_name, duration_str});

    }
    // order sources alphabetically
    std::sort(ui_sources.begin(), ui_sources.end(),
                [](const UISource& a, const UISource& b) { return a.name < b.name; });

    logger::info("UI registered.");
};