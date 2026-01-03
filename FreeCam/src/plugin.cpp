#include "plugin.h"

#include <JoJoAPI.h>
#include <imgui/imgui.h>

#include "camera.h"

JAPIModMeta GetModMeta() {
    static JAPIModMeta meta = {
        "FreeCam",
        "Kapilarny & Kojo Bailey",
        "FreeCam",
        "1.3.0",
        "Frees the camera."
    };

    return meta;
}

namespace Nucc {
    struct Vector3 {
        float x, y, z;
    };
}

struct ccCamera
{
    void* vtable;
    BYTE block_alloc[0x10];
    BYTE pad[20];
    uint32_t unk01;
    uint32_t mat[4][4];
    Nucc::Vector3 position_vector;
    Nucc::Vector3 lookAtVector;
    Nucc::Vector3 upDirVector;
    Nucc::Vector3 vec_d;
};

struct ModConfig {
    bool free_cam_enabled = false;
    bool movement_blocked = false;
    bool culling_disabled = false;
    bool disable_ui = false;

    struct Keys {
        char forward = 'I';
        char backward = 'K';
        char left = 'J';
        char right = 'L';
        char up = 'U';
        char down = 'O';
    } keys;
};

struct KeyboardState {
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
};

static Camera camera;
static ModConfig cfg;
static KeyboardState kb;

void* ccUiBattleManagerDraw_ptr = nullptr;

// This is fucking stupid
void SetCursorVisible(const bool visible) {
    if (visible)
        while (ShowCursor(TRUE) < 0) {}
    else
        while (ShowCursor(FALSE) >= 0) {}
}

// ReSharper disable once CppDFAConstantFunctionResult
bool HWNDProcCallback_Listener(HWNDCallbackEvent* e) {
    if (e->uMsg == WM_KEYDOWN && e->wParam == VK_F5) {
        cfg.free_cam_enabled = !cfg.free_cam_enabled;

        if (cfg.free_cam_enabled) {
            RECT rect;
            GetWindowRect(e->hwnd, &rect);
            const auto centerX = (rect.left + rect.right) / 2;
            const auto centerY = (rect.top + rect.bottom) / 2;
            SetCursorPos(centerX, centerY);
        }
    }

    if (!cfg.free_cam_enabled) {
        SetCursorVisible(true);
        return false;
    }

    if (e->uMsg == WM_KEYUP && e->wParam == VK_F8) {
        cfg.movement_blocked = !cfg.movement_blocked;
    }

    if (cfg.movement_blocked) {
        SetCursorVisible(true);
        return false;
    }

    if (e->uMsg == WM_KEYUP || e->uMsg == WM_KEYDOWN) {
        if (e->wParam == cfg.keys.forward) {
            kb.forward = e->uMsg == WM_KEYDOWN;
        } else if (e->wParam == cfg.keys.backward) {
            kb.backward = e->uMsg == WM_KEYDOWN;
        } else if (e->wParam == cfg.keys.left) {
            kb.left = e->uMsg == WM_KEYDOWN;
        } else if (e->wParam == cfg.keys.right) {
            kb.right = e->uMsg == WM_KEYDOWN;
        } else if (e->wParam == cfg.keys.up) {
            kb.up = e->uMsg == WM_KEYDOWN;
        } else if (e->wParam == cfg.keys.down) {
            kb.down = e->uMsg == WM_KEYDOWN;
        }
    }

    if (e->uMsg == WM_MOUSEMOVE) {
        uint16_t x = LOWORD(e->lParam);
        uint16_t y = HIWORD(e->lParam);

        RECT rect;
        GetWindowRect(e->hwnd, &rect);
        auto centerX = rect.left + (rect.right - rect.left) / 2;
        auto centerY = rect.top + (rect.bottom - rect.top) / 2;
        SetCursorPos(centerX, centerY);

        POINT p;
        GetCursorPos(&p);
        ScreenToClient(e->hwnd, &p);
        camera.ProcessMouseMovement((float)(p.x - x), (float)(y - p.y), 1);
    }

    SetCursorVisible(cfg.movement_blocked);

    return false;
}

// 6DF320
// __int64 __fastcall nuccDrawEnv::SetModelCullingEnable(_QWORD *a1, int a2)
typedef void(__fastcall* nuccDrawEnv_SetModelCullingEnable_t)(__int64* a1, int a2);
nuccDrawEnv_SetModelCullingEnable_t nuccDrawEnv_SetModelCullingEnable_orig = nullptr;
void __fastcall nuccDrawEnv_SetModelCullingEnable_Hook(__int64* a1, int a2) {
    nuccDrawEnv_SetModelCullingEnable_orig(a1, cfg.culling_disabled ? 0 : a2);
}

// 0x773350
//ccTaskCertainExe *__fastcall ccTaskCertainExe__ccTaskCertainExe(ccTaskCertainExe *certain_exe, const char *name, int id, int type);
typedef void*(__fastcall* ccTaskCertainExe_ccTaskCertainExe_t)(void* certain_exe, const char* name, int id, int type);
ccTaskCertainExe_ccTaskCertainExe_t ccTaskCertainExe_ccTaskCertainExe_orig = nullptr;

void* __fastcall ccTaskCertainExe_ccTaskCertainExe_Hook(void* certain_exe, const char* name, int id, int type) {
    // JINFO("name: %s", name);
    if (!strcmp(name, "ccUiBattleManager::Draw")) {
        ccUiBattleManagerDraw_ptr = certain_exe;
        // JINFO("Found");
    }

    return ccTaskCertainExe_ccTaskCertainExe_orig(certain_exe, name, id, type);
}

// void __fastcall ccTask::Execute(ccTask *a1, int a2)
typedef void(__fastcall* ccTask_Execute_t)(void* a1, int a2);
ccTask_Execute_t ccTask_Execute_orig = nullptr;
void __fastcall ccTask_Execute_Hook(void* a1, int a2) {
    if (cfg.disable_ui && a1 == ccUiBattleManagerDraw_ptr) {
        return;
    }

    ccTask_Execute_orig(a1, a2);
}

// 7727E0
// void __fastcall ccCamera::Update(ccCamera *a1)
typedef void(__fastcall* ccCamera_Update_t)(ccCamera* a1);
ccCamera_Update_t ccCamera_Update_orig = nullptr;
void __fastcall ccCamera_Update_Hook(ccCamera* a1) {
    // Update the original camera first
    ccCamera_Update_orig(a1);

    if (!cfg.free_cam_enabled) {
        return;
    }

    // 76B4B0
    auto GetCameraDirector = (__int64(*)(__int64, uint32_t)) (JAPI_GetModuleBaseAddress() + 0x76B4B0);

    // 771170
    auto GetActiveCamera = (ccCamera*(*)(__int64)) (JAPI_GetModuleBaseAddress() + 0x771170);

    // (__int64 (*)(__int64, unsigned int))(JAPI_GetModuleBase() + 0x771150);
    auto GetCamera = (ccCamera*(*)(__int64, uint32_t)) (JAPI_GetModuleBaseAddress() + 0x771150);

    __int64 cameraManager = *(__int64*)(JAPI_GetModuleBaseAddress() + 0x12A8A40);

    if (!cameraManager) {
        return;
    }

    __int64 cameraDirector = GetCameraDirector(cameraManager, 0);
    if (cameraDirector == 0) {
        return;
    }

    bool isCurrentGameCamera = false;
    for (int i = 0; i < 3; i++) {
        if (GetCamera(cameraDirector, i) == a1) {
            isCurrentGameCamera = true;
            break;
        }
    }

    if (!isCurrentGameCamera) {
        return;
    }

    camera.ChangeX(kb.right - kb.left);
    camera.ChangeZ(kb.forward - kb.backward);
    camera.ChangeY(kb.up - kb.down);

    glm::mat4 view = camera.GetViewMatrix();

    void* matrix_ptr = &a1->mat;
    void* view_ptr = &view[0][0];

    auto CopyMatrix4x4Inversed = (__int64 (*)(void *, void *))(JAPI_GetModuleBaseAddress() + 0x6C9300);
    CopyMatrix4x4Inversed(matrix_ptr, view_ptr);
}

char ConfigBindChar(const char* identifier, const char defaultValue) {
    char temp[] = {defaultValue, '\0'};
    const auto str = JAPI_ConfigBindString(identifier, &defaultValue);

    if (str->length > 0) {
        temp[0] = str->data[0];
    }

    // Cleanup
    JAPI_FreeString(str);

    return temp[0];
}

void ModInit() {
    JINFO("FreeCam initialized!");

    JAPIHookMeta hooks[] = {
        JAPIHookMeta{
            .target = 0x7727E0,
            .detour = (void*)ccCamera_Update_Hook,
            .original = &ccCamera_Update_orig,
            .name = "ccCamera::Update",
            .game_function = true
        },
        JAPIHookMeta{
            .target = 0x6DF320,
            .detour = (void*)nuccDrawEnv_SetModelCullingEnable_Hook,
            .original = &nuccDrawEnv_SetModelCullingEnable_orig,
            .name = "nuccDrawEnv::SetModelCullingEnable",
            .game_function = true
        },
        JAPIHookMeta{
            .target = 0x773350,
            .detour = (void*)ccTaskCertainExe_ccTaskCertainExe_Hook,
            .original = &ccTaskCertainExe_ccTaskCertainExe_orig,
            .name = "ccTaskCertainExe::ccTaskCertainExe",
            .game_function = true
        },
        JAPIHookMeta{
            .target = 0x773410,
            .detour = (void*)ccTask_Execute_Hook,
            .original = &ccTask_Execute_orig,
            .name = "ccTask::Execute",
            .game_function = true
        }
    };

    for (const auto& hook : hooks) {
        // JTRACE("Registering hook: %s", hook.name);

        if (!JAPI_RegisterHook(hook)) {
            JERROR("Failed to register hook: %s", hook.name);
            return;
        }
    }

    cfg.keys = {
        .forward = ConfigBindChar("KeyForward", 'I'),
        .backward = ConfigBindChar("KeyBackward", 'K'),
        .left = ConfigBindChar("KeyLeft", 'J'),
        .right = ConfigBindChar("KeyRight", 'L'),
        .up = ConfigBindChar("KeyUp", 'U'),
        .down = ConfigBindChar("KeyDown", 'O')
    };

    SPEED = JAPI_ConfigBindFloat("MovementSpeed", 10.f);
    SENSITIVITY = JAPI_ConfigBindFloat("MouseSensitivity", 0.5f);

    // Register HWND proc listener
    JAPI_RegisterCancellableEventListener("HWNDCallbackEvent", reinterpret_cast<JAPI_CancellableEventListener>(HWNDProcCallback_Listener));
}

void DrawImGUI() {
    if (cfg.free_cam_enabled) {
        ImGui::Text("Free Cam position: (%.2f, %.2f, %.2f)", camera.Position.x, camera.Position.y, camera.Position.z);
        ImGui::Separator();
    }

    ImGui::Checkbox("FreeCam enabled", &cfg.free_cam_enabled);
    ImGui::Checkbox("Block movement", &cfg.movement_blocked);
    ImGui::Checkbox("Disable culling", &cfg.culling_disabled);
    ImGui::Checkbox("Disable UI", &cfg.disable_ui);

    if (ImGui::CollapsingHeader("Camera Properties")) {
        if (ImGui::SliderFloat("Movement Speed", &SPEED, 1.f, 500.f)) {
            JAPI_ConfigSetFloat("MovementSpeed", SPEED);
        }
        if (ImGui::SliderFloat("Mouse Sensitivity", &SENSITIVITY, 0.1f, 10.f)) {
            JAPI_ConfigSetFloat("MouseSensitivity", SENSITIVITY);
        }
    }

    if (ImGui::CollapsingHeader("Controls")) {
        ImGui::Text("Toggle FreeCam: F5");
        ImGui::Text("Toggle Movement Block: F8");

        ImGui::Separator();

        ImGui::Text("Movement Keys:");

        char fwd_buf[2] = {cfg.keys.forward, '\0'};
        if (ImGui::InputText("Forward", fwd_buf, 2, ImGuiInputTextFlags_CharsNoBlank)) {
            cfg.keys.forward = fwd_buf[0];
            JAPI_ConfigSetString("KeyForward", fwd_buf);
        }

        char back_buf[2] = {cfg.keys.backward, '\0'};
        if (ImGui::InputText("Backward", back_buf, 2, ImGuiInputTextFlags_CharsNoBlank)) {
            cfg.keys.backward = back_buf[0];
            JAPI_ConfigSetString("KeyBackward", back_buf);
        }

        char left_buf[2] = {cfg.keys.left, '\0'};
        if (ImGui::InputText("Left", left_buf, 2, ImGuiInputTextFlags_CharsNoBlank)) {
            cfg.keys.left = left_buf[0];
            JAPI_ConfigSetString("KeyLeft", left_buf);
        }

        char right_buf[2] = {cfg.keys.right, '\0'};
        if (ImGui::InputText("Right", right_buf, 2, ImGuiInputTextFlags_CharsNoBlank)) {
            cfg.keys.right = right_buf[0];
            JAPI_ConfigSetString("KeyRight", right_buf);
        }

        char up_buf[2] = {cfg.keys.up, '\0'};
        if (ImGui::InputText("Up", up_buf, 2, ImGuiInputTextFlags_CharsNoBlank)) {
            cfg.keys.up = up_buf[0];
            JAPI_ConfigSetString("KeyUp", up_buf);
        }

        char down_buf[2] = {cfg.keys.down, '\0'};
        if (ImGui::InputText("Down", down_buf, 2, ImGuiInputTextFlags_CharsNoBlank)) {
            cfg.keys.down = down_buf[0];
            JAPI_ConfigSetString("KeyDown", down_buf);
        }
    }
}
