#pragma once

#include "CustomObjects.h"
#include "SimpleIni.h"


namespace Settings {
    inline bool force_editor_id = false;
    inline bool enabled_plyrmsg = true;
    inline bool enabled_remainingmsg = true;
    inline bool enabled_err_msgbox = true;

    inline bool po3installed = false;

    // QuickLight
    const std::string ql_editorid = "aaaQLLightSelf";
    inline bool ql_enabled = false;
    inline RE::FormID ql_formid = 0;
    inline RE::FormID ql_fuel = 0;
    inline float ql_duration = 0.f;
    //inline Float3 ql_radius(700.f, 700.f, 700.f);


    constexpr std::array<const char*, 5> InISections = {"Light Sources", "Fuel Sources", "Durations", "Other Stuff",
                                                        "QuickLight Support"};
    constexpr std::array<const char*, 4> InIDefaultKeys = {"src1", "src1", "src1"};

    const std::array<std::string, 4> section_comments = {
        ";Make sure to use unique keys, e.g. src1=... NOTsrc1=...",
        std::format(";Make sure to use matching keys with the ones provided in section {}.",
                    static_cast<std::string>(InISections[0])),
        std::format(";Make sure to use matching keys with the ones provided in sections {} and {}.",
                    static_cast<std::string>(InISections[0]), static_cast<std::string>(InISections[1])),
        ";Set boolean values in this section, i.e. true or false."};

    // DO NOT CHANGE THE ORDER OF THESE
    constexpr std::array<const char*, 4> OtherStuffDefKeys = {"ForceEditorID", "DepleteReplenishMessages",
                                                              "RemainingMessages", "ErrMsgBox"};
    const std::map<const char*, std::map<bool, const char*>> other_stuff_defaults = {
        {OtherStuffDefKeys[0],
         {{force_editor_id,
           ";Set to true if you ONLY use EditorIDs and NO FormIDs AND you have powerofthree's Tweaks installed, "
           "otherwise false."}}},
        {OtherStuffDefKeys[1],
         {{enabled_plyrmsg, ";These are the messages that will be displayed when the player's fuel depletes."}}},
        {OtherStuffDefKeys[2],
         {{enabled_remainingmsg,
           ";These are the messages that will be displayed when the player puts away the source."}}},
        {OtherStuffDefKeys[3],
         {{enabled_err_msgbox, ";Pop-up error messages. Do not set to false, unless you have to..."}}}};

    constexpr const char* default_duration = "8";

    std::vector<LightSource> LoadINISettings();
};