#pragma once
#include "Settings.h"


namespace Hooks {
    
    // QL stuff
    inline float ql_total_delta = 0.f;
    inline float ql_last_t = 0.f;
    inline float ql_elapsed_t = 0.f;
    //inline std::set<RE::FormID> ql_fuels;
    inline bool ql_depleted = false;
    //inline Float3 last_radius;


    struct MagicCastHook {

    public:
        static void InstallHook();

    private:
    

        static void Start(RE::LightEffect* a_eff);

        static void Update(RE::LightEffect* a_eff, float a_delta);

        static void Stop(RE::LightEffect* a_eff);

        static inline REL::Relocation<decltype(Start)> BaseFunction_Start;
        static inline REL::Relocation<decltype(Update)> BaseFunction_Update;
        static inline REL::Relocation<decltype(Stop)> BaseFunction_Stop;
    };

    void InstallHooks();
};

void NoFuelQL(RE::LightEffect* a_eff);
