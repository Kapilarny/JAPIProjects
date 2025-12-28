#include "plugin.h"

#include <JoJoAPI.h>

JAPIModMeta GetModMeta() {
    static JAPIModMeta meta = {
        "SkipIntro",
        "Kapilarny",
        "SkipIntro",
        "1.0.0",
        "Skips intro scenes of the game."
    };

    return meta;
}

// ccSceneManager::Push
typedef __int64(__fastcall* sub_768730)(void* a1, __int64 a2);
sub_768730 sub_768730_Original;

__int64 __fastcall sub_768730_Hook(void* a1, __int64 a2) {
    if(a2 == 25) {
        return sub_768730_Original(a1, 38);
    }

    if(a2 == 63) {
        return sub_768730_Original(a1, 3);
    }

    if(a2 == 7) {
        return sub_768730_Original(a1, 58);
    }

    return sub_768730_Original(a1, a2);
}

void ModInit() {
    auto h = JAPI_RegisterHook(JAPIHookMeta{
        .target = 0x768730,
        .detour = (void*)sub_768730_Hook,
        .original = &sub_768730_Original,
        .name = "ccSceneManager::Push",
        .game_function = true
    });

    if (!h) {
        JERROR("Failed to initialize hooks!");
        return;
    }

    JINFO("Patches applied!");
}