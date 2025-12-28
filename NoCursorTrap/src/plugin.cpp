#include "plugin.h"

#include <JoJoAPI.h>

JAPIModMeta GetModMeta() {
    static JAPIModMeta meta = {
        "NoCursorTrap",
        "Kapilarny",
        "NoCursorTrap",
        "1.0.0",
        "A JAPI plugin."
    };

    return meta;
}

// ClipCursor
typedef BOOL(WINAPI* ClipCursor_t)(const RECT*);
static ClipCursor_t Original_ClipCursor = nullptr;
BOOL WINAPI Hooked_ClipCursor(const RECT* lpRect) {
    // Bypass the cursor clipping by always returning TRUE without calling the original function
    return TRUE;
}

void ModInit() {
    // Hook ClipCursor
    HMODULE user32 = GetModuleHandleA("user32.dll");
    if (!user32) {
        JWARN("Failed to get handle for user32.dll");
    }

    Original_ClipCursor = (ClipCursor_t)GetProcAddress(user32, "ClipCursor");

    JAPIHookMeta h = {
        .target = (uint64_t)Original_ClipCursor,
        .detour = (void*)Hooked_ClipCursor,
        .original = (void**)&Original_ClipCursor,
        .name = "ClipCursor",
        .game_function = false
    };

    auto handle = JAPI_RegisterHook(h);
    if (handle == 0) {
        JERROR("Failed to register ClipCursor hook");
    } else {
        JINFO("Successfully hooked ClipCursor");
    }
}