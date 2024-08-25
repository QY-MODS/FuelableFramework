#include "Utils.h"

float Utilities::Round(float number, int decimalPlaces) {
    auto rounded_number =
        static_cast<float>(std::round(number * std::pow(10, decimalPlaces))) / std::pow(10, decimalPlaces);
    return rounded_number;
}

int Utilities::Round2Int(float number) {
    auto rounded_number = static_cast<int>(Round(number, 0));
    return rounded_number;
}

bool Utilities::isValidHexWithLength7or8(const char* input) {
    std::string inputStr(input);
    std::regex hexRegex("^[0-9A-Fa-f]{7,8}$");  // Allow 7 to 8 characters
    bool isValid = std::regex_match(inputStr, hexRegex);
    return isValid;
}

std::string Utilities::DecodeTypeCode(std::uint32_t typeCode) {
    char buf[4];
    buf[3] = char(typeCode);
    buf[2] = char(typeCode >> 8);
    buf[1] = char(typeCode >> 16);
    buf[0] = char(typeCode >> 24);
    return std::string(buf, buf + 4);
}

std::filesystem::path Utilities::GetLogPath() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    return logFilePath;
}

std::vector<std::string> Utilities::ReadLogFile() {
    std::vector<std::string> logLines;

    // Open the log file
    std::ifstream file(GetLogPath().c_str());
    if (!file.is_open()) {
        // Handle error
        return logLines;
    }

    // Read and store each line from the file
    std::string line;
    while (std::getline(file, line)) {
        logLines.push_back(line);
    }

    file.close();

    return logLines;
}

void MsgBoxesNotifs::InGame::Remaining(int remaining, std::string_view item) {
    if (remaining < 0.00001)
        return;
    else if (remaining > 1)
        RE::DebugNotification(std::format("I have about {} hours left in my {}.", remaining, item).c_str());
    else if (remaining > 0.5)
        RE::DebugNotification(std::format("I have about an hour left in my {}.", item).c_str());
    else
        RE::DebugNotification(std::format("I don't have much left in my {}...", item).c_str());
}

float almost_constant_with_accelerated_cutoff(float x, float start_value, float cutoff, int power) { // Ensure x is within [0, 1]
    x = std::clamp(x, 0.0f, 1.0f);

    // Calculate the nonlinear drop-off
    float drop = std::max(0.0f, 1.0f - std::pow((x - cutoff) / (1.0f - cutoff), static_cast<float>(power)));

    // Return the function value
    return (x < cutoff) ? start_value : start_value * drop;
}

RE::FormID GetFormEditorIDFromString(const std::string& formEditorId) {
    logger::trace("Getting formid from editorid: {}", formEditorId);
    if (Utilities::isValidHexWithLength7or8(formEditorId.c_str())) {
        logger::trace("formEditorId is in hex format.");
        int form_id_;
        std::stringstream ss;
        ss << std::hex << formEditorId;
        ss >> form_id_;
        auto temp_form = GetFormByID(form_id_, "");
        if (temp_form) return temp_form->GetFormID();
        else {
            logger::error("Formid is null for editorid {}", formEditorId);
            return 0;
        }
    }
    if (formEditorId.empty()) return 0;
    else if (!Utilities::IsPo3Installed()) {
        logger::error("Po3 is not installed.");
        MsgBoxesNotifs::Windows::Po3ErrMsg();
        return 0;
    }
    auto temp_form = GetFormByID(0, formEditorId);
    if (temp_form) {
        logger::trace("Formid is not null with formid {}", temp_form->GetFormID());
        return temp_form->GetFormID();
    } else {
        logger::trace("Formid is null for editorid {}", formEditorId);
        return 0;
    }
}
