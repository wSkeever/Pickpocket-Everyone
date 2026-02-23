#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include <unordered_set>
using namespace RE;

namespace PickpocketEveryone {
    inline std::unordered_set<RE::TESRace*> g_modifiedRaces;
    using ActivateSignature = bool(TESObjectREFR*, TESObjectREFR*, std::uint8_t, TESBoundObject*, std::int32_t, bool);
    REL::Relocation<ActivateSignature> activate_original;

    void SetAllRacesAllowPickpicket() {
        TESDataHandler* dataHandler = TESDataHandler::GetSingleton();
        if (dataHandler) {
            auto races = dataHandler->GetFormArray<TESRace>();
            for (auto* race : races) {
                if (race && !race->data.flags.all(RE::RACE_DATA::Flag::kAllowPickpocket)) {
                    g_modifiedRaces.insert(race);
                    race->data.flags.set(RACE_DATA::Flag::kAllowPickpocket);
                }
            }
        }
    }

    TESRace* GetRefRace(TESObjectREFR* ref) {
        if (!ref) {
            return nullptr;
        }
        auto actor = ref->As<RE::Actor>();
        if (!actor) {
            return nullptr;
        }
        return actor->GetRace();
    }

    bool IsRaceModified(TESRace* race) {
        return race && g_modifiedRaces.contains(race);
    }

    bool ActivateHook(TESObjectREFR* a_targetRef, TESObjectREFR* a_activatorRef, std::uint8_t a_arg3,
                      TESBoundObject* a_object, std::int32_t a_targetCount, bool a_defaultProcessingOnly) {
        auto player = PlayerCharacter::GetSingleton();
        if (player && player->IsSneaking()) {
            auto targetRace = GetRefRace(a_targetRef);
            if (IsRaceModified(targetRace)) {
                targetRace->data.flags.reset(RACE_DATA::Flag::kAllowPickpocket);
                auto returnValue = activate_original(a_targetRef, a_activatorRef, a_arg3, a_object, a_targetCount, a_defaultProcessingOnly);
                targetRace->data.flags.set(RACE_DATA::Flag::kAllowPickpocket);
                return returnValue;
            }
        }
        return activate_original(a_targetRef, a_activatorRef, a_arg3, a_object, a_targetCount, a_defaultProcessingOnly);
    }

    void InstallActivateHook() {
        if (REL::Module::IsVR()) {
            return;
        }
        REL::RelocationID hook{55610, 56139};
        ptrdiff_t offset = REL::Offset(0x4A).offset();
        auto& trampoline = SKSE::GetTrampoline();
        activate_original = trampoline.write_call<5>(hook.address() + offset, ActivateHook);
    }

    SKSEPluginLoad(const SKSE::LoadInterface* skse) {
        SKSE::Init(skse);
        SKSE::AllocTrampoline(14);
        SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message) {
            if (message->type == SKSE::MessagingInterface::kDataLoaded) {
                SetAllRacesAllowPickpicket();
                InstallActivateHook();
            }
        });
        return true;
    }
}