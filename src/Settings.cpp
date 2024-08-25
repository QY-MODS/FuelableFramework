#include "Settings.h"

std::vector<LightSource> Settings::LoadINISettings() {
    logger::info("Loading ini settings");
    if (Utilities::IsPo3Installed()) {
        logger::info("powerofthree's Tweaks is installed. Enabling EditorID support.");
        po3installed = true;
    } else {
        logger::info("powerofthree's Tweaks is not installed. Disabling EditorID support.");
        po3installed = false;
    }

    CSimpleIniA ini;
    CSimpleIniA::TNamesDepend source_names;
    std::vector<LightSource> lightSources;

    ini.SetUnicode();
    ini.LoadFile(Utilities::path);

    // Create Sections with defaults if they don't exist
    for (int i = 0; i < InISections.size(); ++i) {
        
        logger::info("Checking section {}", InISections[i]);
        if (ini.SectionExists(InISections[i])) continue;

        logger::info("Section {} does not exist. Creating it.", InISections[i]);
        ini.SetValue(InISections[i], nullptr, nullptr);
        logger::info("Setting default keys for section {}", InISections[i]);
        if (i < 3) {
            ini.SetValue(InISections[i], InIDefaultKeys[i], nullptr, section_comments[i].c_str());
            logger::info("Default values set for section {}", InISections[i]);
        } else if (i==3) {
            logger::info("Creating Other Stuff section");
            for (auto it = other_stuff_defaults.begin(); it != other_stuff_defaults.end(); ++it) {
                auto it2 = it->second.begin();
                ini.SetBoolValue(InISections[i], it->first, it2->first, it2->second);
            }
        } else if (i == 4) {
            logger::info("Creating QuickLight Support section");
            ini.SetValue(InISections[i], "Enabled", "false");
            ini.SetValue(InISections[i], "Fuel", "0");
            ini.SetValue(InISections[i], "Duration", default_duration);
            //ini.SetValue(InISections[i], "RadiusX", "700");
            //ini.SetValue(InISections[i], "RadiusY", "700");
            //ini.SetValue(InISections[i], "RadiusZ", "700");
        }
    }

    // Sections: QuickLight Support
    ql_enabled = ini.GetBoolValue(InISections[4], "Enabled", false);
    if (ql_enabled) {
        const auto temp_fuel_id = std::string(ini.GetValue(InISections[4], "Fuel"));
        ql_fuel = GetFormEditorIDFromString(temp_fuel_id);
        ql_formid = GetFormEditorIDFromString(ql_editorid);
        if (!ql_formid || !ql_fuel) {
            logger::error("QuickLight is enabled but formid or fuel is invalid. Disabling QuickLight.");
            ql_enabled = false;
        }
        else if (!RE::TESForm::LookupByID<RE::TESBoundObject>(ql_fuel)) {
            logger::error("QuickLight is enabled but fuel formid is invalid. Disabling QuickLight.");
            ql_enabled = false;
        }
        else {
            ql_duration = std::stof(ini.GetValue(InISections[4], "Duration"));
            if (ql_duration < 0.1f) {
                logger::warn("QuickLight duration is invalid. Using default value.");
                ql_duration = std::stof(default_duration);
                ini.SetValue(InISections[4], "Duration", default_duration);
            }
            /*const auto temp_radius_x = std::clamp(std::stof(ini.GetValue(InISections[4], "RadiusX","700")), 100.f, 1500.f);
            const auto temp_radius_y = std::clamp(std::stof(ini.GetValue(InISections[4], "RadiusY","700")), 100.f, 1500.f);
            const auto temp_radius_z = std::clamp(std::stof(ini.GetValue(InISections[4], "RadiusZ","700")), 100.f, 1500.f);
            ql_radius = Float3(temp_radius_x, temp_radius_y, temp_radius_z);
            logger::info("QuickLight enabled with formid: {}, duration: {}, radius: x {}, y {}, z {}", ql_formid,
                         ql_duration, ql_radius.x, ql_radius.y, ql_radius.z);*/
        }
    };
    

    // Sections: Other stuff
    // get from user
    CSimpleIniA::TNamesDepend other_stuff_userkeys;
    ini.GetAllKeys(InISections[3], other_stuff_userkeys);
    for (CSimpleIniA::TNamesDepend::const_iterator it = other_stuff_userkeys.begin(); it != other_stuff_userkeys.end();
         ++it) {
        for (auto it2 = other_stuff_defaults.begin(); it2 != other_stuff_defaults.end(); ++it2) {
            if (static_cast<std::string_view>(it->pItem) == static_cast<std::string_view>(it2->first)) {
                logger::info("Found key: {}", it->pItem);
                auto it3 = it2->second.begin();
                auto val = ini.GetBoolValue(InISections[3], it->pItem, it3->first);
                ini.SetBoolValue(InISections[3], it->pItem, val, it3->second);
                break;
            }
        }
    }

    // set stuff which is not found
    for (auto it = other_stuff_defaults.begin(); it != other_stuff_defaults.end(); ++it) {
        if (ini.KeyExists(InISections[3], it->first)) continue;
        auto it2 = it->second.begin();
        ini.SetBoolValue(InISections[3], it->first, it2->first, it2->second);
    }

    force_editor_id = ini.GetBoolValue(InISections[3],
                                       OtherStuffDefKeys[0]);  // logger::info("force_editor_id: {}", force_editor_id);
    enabled_plyrmsg = ini.GetBoolValue(InISections[3],
                                       OtherStuffDefKeys[1]);  // logger::info("enabled_plyrmsg: {}", enabled_plyrmsg);
    enabled_remainingmsg = ini.GetBoolValue(
        InISections[3], OtherStuffDefKeys[2]);  // logger::info("enabled_remainingmsg: {}", enabled_remainingmsg);
    enabled_err_msgbox = ini.GetBoolValue(
        InISections[3], OtherStuffDefKeys[3]);  // logger::info("enabled_err_msgbox: {}", enabled_err_msgbox);

    // Sections: Light Sources, Fuel Sources, Durations
    ini.GetAllKeys(InISections[0], source_names);
    auto numSources = source_names.size();
    logger::info("source_names size {}", numSources);

    lightSources.reserve(numSources);

    for (CSimpleIniA::TNamesDepend::const_iterator it = source_names.begin(); it != source_names.end(); ++it) {
        logger::info("source name {}", it->pItem);
        const char* val1 = ini.GetValue(InISections[0], it->pItem);
        const char* val2 = ini.GetValue(InISections[1], it->pItem);
        const char* val3 = ini.GetValue(InISections[2], it->pItem);
        if (!val1 || !val2) {
            logger::warn("Source {} is missing a value. Skipping.", it->pItem);
            continue;
        } else
            logger::info("Source {} has a value of {}", it->pItem, val1);
        if (std::strlen(val1) && std::strlen(val2)) {
            logger::info("We have valid entries for source: {} and fuel: {}", val1, val2);
            // Duration of the source
            float duration;
            if (!val3 || !std::strlen(val3)) {
                logger::warn("Source {} is missing a duration value. Using default value of {}.", it->pItem,
                             default_duration);
                duration = std::stof(default_duration);
                ini.SetValue(InISections[2], it->pItem, default_duration);
            } else {
                logger::info("Source {} has a duration value of {}.", it->pItem, val3);
                duration = std::stof(val3);
                ini.SetValue(InISections[2], it->pItem, val3);
            }
            // back to id and fuelid
            auto id = static_cast<uint32_t>(std::strtoul(val1, nullptr, 16));
            auto id_str = static_cast<std::string>(val1);
            auto fuelid = static_cast<uint32_t>(std::strtoul(val2, nullptr, 16));
            auto fuelid_str = static_cast<std::string>(val2);
            // Our job is easy if the user wants to force editor ids
            if (force_editor_id && po3installed) lightSources.emplace_back(0, id_str, duration, 0, fuelid_str);
            // if both formids are valid hex, use them
            else if (Utilities::isValidHexWithLength7or8(val1) && Utilities::isValidHexWithLength7or8(val2))
                lightSources.emplace_back(id, "", duration, fuelid, "");
            // one of them is not formid so if powerofthree's Tweaks is not installed we have problem
            else if (!po3installed) {
                MsgBoxesNotifs::Windows::Po3ErrMsg();
                return lightSources;
            }
            // below editor id is allowed
            // if the source is hex, fuel isnt vice versa
            else if (Utilities::isValidHexWithLength7or8(val1))
                lightSources.emplace_back(id, "", duration, 0, fuelid_str);
            else if (Utilities::isValidHexWithLength7or8(val2))
                lightSources.emplace_back(0, id_str, duration, fuelid, "");
            // both are not hex,
            else
                lightSources.emplace_back(0, id_str, duration, 0, fuelid_str);

            ini.SetValue(InISections[0], it->pItem, val1);
            ini.SetValue(InISections[1], it->pItem, val2);
            logger::info("Loaded source: {} with duration: {} and fuel: {}", val1, duration, val2);
        }
    }

    ini.SaveFile(Utilities::path);

    return lightSources;
}