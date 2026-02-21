#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
using namespace RE;

namespace PickpocketEveryone {
    void SetAllRacesAllowPickpicket() {
        TESDataHandler* dataHandler = TESDataHandler::GetSingleton();
        if (dataHandler) {
            auto races = dataHandler->GetFormArray<TESRace>();
            for (auto* race : races) {
                race->data.flags.set(RE::RACE_DATA::Flag::kAllowPickpocket);
            }
        }
    }

    SKSEPluginLoad(const SKSE::LoadInterface* skse) {
        SKSE::Init(skse);
        SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message) {
            if (message->type == SKSE::MessagingInterface::kDataLoaded) {
                SetAllRacesAllowPickpicket();
            }
        });
        return true;
    }
}