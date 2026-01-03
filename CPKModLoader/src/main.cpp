#include "main.h"

#include <filesystem>
#include <JoJoAPI.h>

#include <istream>
#include <fstream>
#include <iostream>

JAPIModMeta GetModMeta() {
    static JAPIModMeta meta = {
        "CPKModLoader",
        "Kapilarny",
        "CPKModLoader",
        "1.0.0",
        "Appends additional CPKs to the game from the japi/cpks/ directory."
    };

    return meta;
}

struct CPKLoadEntry {
    const char* str{};
    uint32_t unk_toggle{};
    BYTE pad01[4]{};
    uint32_t priority{};
    BYTE pad02[4]{};
};

typedef void(*ASBR_LoadCPKFromCPKEntry_t)(CPKLoadEntry*, uint32_t);

// 56C970
typedef void(*sub_56C970_t)();
sub_56C970_t sub_56C970_Original = nullptr;
void sub_56C970_Hook() {
    auto ASBR_LoadCPKFromCPKEntry = reinterpret_cast<ASBR_LoadCPKFromCPKEntry_t>(JAPI_GetModuleBaseAddress() + 0x662378);

    if (!std::filesystem::exists("japi/cpks/")) {
        JINFO("No japi/cpks/ directory found, skipping CPK mod loading.");
        sub_56C970_Original(); // Call the original routine
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator("japi/cpks/")) {
        if (!entry.is_regular_file()) continue;

        // Check if extension is .cpk.info
        if (entry.path().extension() != ".info") continue;

        // Check if filename ends with .cpk
        if (entry.path().stem().extension() != ".cpk") continue;

        std::string filename = entry.path().filename().stem().string();

        JINFO("Found Patch CPK Info: %s", filename.c_str());

        // Read 4 bytes from the .cpk.info file
        std::ifstream infoFile(entry.path(), std::ios::binary);
        if (!infoFile.is_open()) {
            JERROR("Failed to open CPK info file: %s", filename.c_str());
            continue;
        }

        // Read priority
        uint32_t priority = 0;
        infoFile.read(reinterpret_cast<char*>(&priority), sizeof(uint32_t));
        infoFile.close();

        // Set priority
        JAPI_ConfigSetInt(filename.c_str(), priority);
        JINFO("Set Patch CPK Priority: %s -> %u", filename.c_str(), priority);

        // Delete the .cpk.info file after reading
        std::filesystem::remove(entry.path());
    }

    // Load cpks
    for (const auto& entry : std::filesystem::directory_iterator("japi\\cpks")) {
        if (!entry.is_regular_file()) continue;

        // Check if extension is .cpk
        if (entry.path().extension() != ".cpk") continue;
        // filename without extension
        std::string filename = entry.path().filename().stem().string();

        if (filename[0] == '-') {
            JWARN("Skipping disabled CPK: %s", filename.c_str());
            continue;
        }

        std::string fullPath = entry.path().string();

        // Create CPKLoadEntry
        CPKLoadEntry loadEntry = {};
        loadEntry.str = fullPath.c_str();
        loadEntry.unk_toggle = 0;
        loadEntry.priority = JAPI_ConfigBindInt(entry.path().filename().string().c_str(), 9999);

        JTRACE("Adding CPK Load Entry: %s with priority %u", loadEntry.str, loadEntry.priority);

        ASBR_LoadCPKFromCPKEntry(&loadEntry, loadEntry.priority);
    }

    sub_56C970_Original(); // Call the original routine
}

void ModInit() {
    if (!JAPI_RegisterHook(JAPIHookMeta{
        .target = 0x56C970,
        .detour = (void*)sub_56C970_Hook,
        .original = &sub_56C970_Original,
        .name = "ASBR_LoadPatchCPKs",
        .game_function = true
    })) {
        JERROR("Failed to apply hooks!");
        return;
    }

    JINFO("Initialized successfully");
}