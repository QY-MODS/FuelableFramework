#include "Events.h"
#include "MCP.h"

LightSourceManager* LSM = nullptr;

void OnMessage(SKSE::MessagingInterface::Message* message) {
    switch (message->type) {
        case SKSE::MessagingInterface::kNewGame:
            if (LSM->sources.empty() && !Settings::ql_enabled) {
                logger::info("No sources found.");
                if (Settings::enabled_err_msgbox) MsgBoxesNotifs::InGame::NoSourceFound();
                return;
            }
            LSM->Reset();
            logger::info("Newgame LSM reset succesful.");
            break;
        case SKSE::MessagingInterface::kPreLoadGame:
            LSM->Reset();
            logger::info("Preload LSM reset succesful.");
            break;
        case SKSE::MessagingInterface::kPostLoadGame:
            if (LSM->sources.empty() && !Settings::ql_enabled) {
                logger::info("No sources found (PostLoad).");
                if (Settings::enabled_err_msgbox) MsgBoxesNotifs::InGame::NoSourceFound();
                return;
            } else LSM->LogRemainings();
            if (LSM->current_source) LSM->StartBurn();
            logger::info("Postload LSM succesful.");
            break;
        case SKSE::MessagingInterface::kDataLoaded:
            // Start
            auto sources = Settings::LoadINISettings();
            
            Hooks::InstallHooks();
            logger::info("Hooks installed.");
            
            LSM = LightSourceManager::GetSingleton(sources, 500);
            auto* ourEventSink = OurEventSink::GetSingleton(LSM);
            auto* eventSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
            eventSourceHolder->AddEventSink<RE::TESEquipEvent>(ourEventSink);
            logger::info("Event sinks added.");

            UI::Register(LSM);
            logger::info("MCP registered.");
    }
}


void SaveCallback(SKSE::SerializationInterface* serializationInterface) {
    if (!LSM->current_source) logger::info("Saving...btw, current_source is nullptr.");
    if (LSM->is_burning) LSM->PauseBurn();
    LSM->SendData();
    LSM->LogRemainings();
    if (!LSM->Save(serializationInterface, kDataKey, kSerializationVersion)) {
        logger::critical("Failed to save Data");
    }
    uint32_t equipped_obj_id = LSM->current_source ? LSM->current_source->formid : 0;
    uint32_t fuel_id = LSM->current_source ? LSM->current_source->fuel : 0;
    serializationInterface->WriteRecordData(equipped_obj_id);
    serializationInterface->WriteRecordData(fuel_id);
    // QL data
    serializationInterface->WriteRecordData(Hooks::ql_elapsed_t);
    serializationInterface->WriteRecordData(Hooks::ql_depleted);
    logger::trace("QL data saved. {} {}", Hooks::ql_elapsed_t, Hooks::ql_depleted);

    if (LSM->is_burning) {
        LSM->UnPauseBurn();
        logger::info("Data Saved");
    }
}

void LoadCallback(SKSE::SerializationInterface* serializationInterface) {
    std::uint32_t type;
    std::uint32_t version;
    std::uint32_t length;
    uint32_t equipped_obj_id;
    uint32_t fuel_id;

    while (serializationInterface->GetNextRecordInfo(type, version, length)) {
        auto temp = Utilities::DecodeTypeCode(type);

        if (version != kSerializationVersion-1 && version != kSerializationVersion) {
            logger::critical("Loaded data has incorrect version. Recieved ({}) - Expected ({}) for Data Key ({})",
                             version, kSerializationVersion, temp);
            continue;
        }
        switch (type) {
            case kDataKey: {
                if (!LSM->Load(serializationInterface)) {
                    logger::critical("Failed to Load Data");
                }
                serializationInterface->ReadRecordData(equipped_obj_id);
                serializationInterface->ReadRecordData(fuel_id);
                if (equipped_obj_id) {
                    if (!LSM->SetSource(equipped_obj_id, fuel_id)) {  // burasi sakat olabilir
                        MsgBoxesNotifs::InGame::LoadOrderError();
                    }
                }
            } break;
            default:
                logger::critical("Unrecognized Record Type: {}", temp);
                break;
        }
        if (version == kSerializationVersion) {
            serializationInterface->ReadRecordData(Hooks::ql_elapsed_t);
            serializationInterface->ReadRecordData(Hooks::ql_depleted);
            logger::trace("QL data loaded. {} {}", Hooks::ql_elapsed_t, Hooks::ql_depleted);
        } else {
            Hooks::ql_depleted = true;
        }
    }
    LSM->ReceiveData();
    logger::info("Data loaded from skse co-save.");
}

void InitializeSerialization() {
    auto* serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID(kDataKey);
    serialization->SetSaveCallback(SaveCallback);
    serialization->SetLoadCallback(LoadCallback);
    logger::info("Cosave serialization initialized.");
}

void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
#ifndef NDEBUG
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
#else
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);
#endif
    logger::info("Name of the plugin is {}.", pluginName);
    logger::info("Version of the plugin is {}.", SKSE::PluginDeclaration::GetSingleton()->GetVersion());
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {

    SetupLog();
    //spdlog::set_pattern("%v");
    SKSE::Init(skse);
    logger::info("Plugin loaded");
    InitializeSerialization();
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    return true;
}