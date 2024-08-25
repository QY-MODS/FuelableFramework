#include "Manager.h"

void LightSourceManager::Init() {
    Clear();
    bool init_failed = false;
    // sources direk butun sourcelari tutuyor ayni source with different fuel dahil.
    for (auto& src : sources) {
        if (src.init_failed) {
            init_failed = true;
            break;
        }
        src.remaining = 0.f;
        src.elapsed = 0.f;

        //SetData({src.formid, src.fuel}, src.remaining);
        // check if forms are valid
    }
    if (init_failed) {
        is_healthy = false;
        logger::error("Failed to initialize LightSourceManager.");
        if (Settings::enabled_err_msgbox) MsgBoxesNotifs::InGame::InitErr();
        sources.clear();
        return;
    }

    current_source = nullptr;
    is_burning = false;
    // allow_equip_event_sink = true;

    __restart = true;

    logger::info("LightSourceManager initialized.");
}
void LightSourceManager::UpdateLoop() {
    const auto cal = RE::Calendar::GetSingleton();
    if (__restart) {
        is_burning = true;
        __restart = false;
        __start_h = cal->GetHoursPassed();
    }
    if (RE::UI::GetSingleton()->GameIsPaused()) return;
    if (HasFuel(current_source)) current_source->elapsed = cal->GetHoursPassed() - __start_h;
    else NoFuel();
}
bool LightSourceManager::SetSourceWithRemaningFuel(RE::FormID eqp_obj) {
    for (auto& src : sources) {
        if (eqp_obj == src.formid && HasFuel(&src)) {
            current_source = &src;
            logger::trace("Remainingli olan bi tane buldum: {} {}", src.remaining, current_source->remaining);
            return true;
        }
    }
    return false;
}

bool LightSourceManager::SetSource(RE::FormID eqp_obj, RE::FormID fuel_obj) {
    logger::trace("{}", fuel_obj);
    // fuel_obj dayatmasi varsa onu oncelikle
    if (fuel_obj) {
        for (auto& src : sources) {
            if (eqp_obj == src.formid && fuel_obj == src.fuel) {
                current_source = &src;
                return true;
            }
        }
    }
    // yoksa remaining fuel olanlardan birini bul
    if (SetSourceWithRemaningFuel(eqp_obj)) return true;
    // yoksa ilk buldugunu al
    for (auto& src : sources) {
        if (eqp_obj == src.formid) {
            current_source = &src;
            return true;
        }
    }
    logger::error("Did not find a match!!!");
    return false;
}

void LightSourceManager::LogRemainings() {
    for (auto& src : sources) {
        logger::info("Remaining hours for {}: {}", src.GetName(), src.remaining);
    }
}

void LightSourceManager::Reset() {
    Stop();
    Init();
}
void LightSourceManager::SendData() {
    for (auto& src : sources) {
        SetData({src.formid, src.fuel}, src.remaining);
    }
}
void LightSourceManager::ReceiveData() {
    for (auto& src : sources) {
        for (const auto& [formId2, value] : m_Data) {
            if (formId2.outerKey == src.formid && formId2.innerKey == src.fuel) {
                src.remaining = std::min(value, src.duration);
                break;
            }
        }
    }
}
void LightSourceManager::HandleEquip(RE::FormID a_formid, bool equipped) {
    if (equipped) {
        if (IsCurrentSource(a_formid)) return;
        if (!SetSource(a_formid)) logger::info("Failed to set source. Something is terribly wrong!!!");
        StartBurn();
    } else {
        if (!IsCurrentSource(a_formid)) return;
        StopBurn();
    }
};

float LightSourceManager::PauseBurn() {
    Stop();
    current_source->remaining -= current_source->elapsed;
    current_source->elapsed = 0.f;
    __restart = true;
    return current_source->remaining;  // sadece nofuel icinde kullanmak icin
};


void LightSourceManager::NoFuel() {
    auto plyr = RE::PlayerCharacter::GetSingleton();
    // yanarken fuel kalmadi. simdi elimde ayni source icin baska source-fuel kombinasyonlari var mi diye bakicam.
    const RE::FormID current_source_formid = current_source->formid;
    const RE::FormID current_fuel = current_source->fuel;
    std::vector<std::string_view> fuel_list;
    bool fuel_found = false;
    
    for (auto& src : sources) {
        
        if (src.formid != current_source_formid) continue;
        
        fuel_list.push_back(src.GetFuelName());
        auto fuel_item = src.GetBoundFuelObject();

        if (plyr->GetItemCount(fuel_item) > 0) {
            float to_be_transferred = PauseBurn();
            current_source->remaining = 0.f;  // bekleme sirasinda fuellar arasindaki gecislerde remaining eksiye geciyo diye
            logger::trace("Hi2");
            SetSource(src.formid, src.fuel);
            if (Settings::enabled_plyrmsg) MsgBoxesNotifs::InGame::Refuel(GetName(), GetFuelName());
            plyr->RemoveItem(fuel_item, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);  // Refuel
            current_source->remaining += current_source->duration + to_be_transferred;          // Refuel
            Start();
            fuel_found = true;
            break;
        }
    }
    if (!fuel_found) {
        SetSource(current_source_formid, current_fuel);
        Stop();  // better safe than sorry?
        if (Settings::enabled_plyrmsg){
            MsgBoxesNotifs::InGame::NoFuel(GetName(),
                                           static_cast<std::string_view>(Utilities::String::join(fuel_list, " or ")));
        }
        // This has to be the last thing that happens in this function, which involves current_source because
        // current_source is set to nullptr in the main thread
        RE::ActorEquipManager::GetSingleton()->UnequipObject(plyr, GetBoundObject(), nullptr, 1, nullptr, true,
                                                                false, false);
    }
}
void LightSourceManager::ShowRemaining() {
    if (!Settings::enabled_remainingmsg) return;
    int _remaining = Utilities::Round2Int(current_source->remaining);
    MsgBoxesNotifs::InGame::Remaining(_remaining, GetName());
};

bool LightSourceManager::IsValidSource(RE::FormID eqp_obj) {
    for (auto& src : sources) {
        if (eqp_obj == src.formid) {
            return true;
        }
    }
    return false;
}
bool LightSourceManager::IsCurrentSource(RE::FormID eqp_obj) {
    if (!current_source) return false;
    return eqp_obj == current_source->formid;
}

void LightSourceManager::StartBurn() {
    if (is_burning) return;
    if (!current_source) {
        logger::error("No current source!!No current source!!No current source!!No current source!!");
        MsgBoxesNotifs::Windows::GeneralErr();
        return;
    }
    ShowRemaining();
    __restart = true;
    Start();
}
void LightSourceManager::StopBurn() {
    PauseBurn();
    is_burning = false;
    current_source = nullptr;
};