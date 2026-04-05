#include <iostream>

#include "HookManager/HookManager.hpp"
#include "Utils/Utils.h"
#include <imgui.h>

static bool ModExitTag = false;
static bool* Tag = nullptr;
static int Id = 0;
static void* imgui_print = nullptr;

// 实现日志函数 注意C++的变量放置位置
void LogPrint(const char* fmt, ...) {
    if(Tag && Tag[0] && imgui_print) {
        char buffer[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        ((void(__fastcall*)(const char*))imgui_print)(buffer);
    }
}

int blockDestroy = 100;

HookInstance* hook = nullptr;
// 被Hook函数的实现
void Block_playerDestroy(void* block, void* player, void* pos) {
    auto original = hook->oriForSign(Block_playerDestroy);
    for(int i = 0; i < blockDestroy - 1; i++) {
        original(block, player, pos);
    }
    original(block, player, pos);
}

void main(HMODULE hModule) {
    // 拿到地址 1.21.51.01
    //uintptr_t addr = (uintptr_t)GetModuleHandleA("Minecraft.Windows.exe") + 0x3A7CED0;

    SignCode sign("Block::playerDestroy");
    sign << "40 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 48 89 AC 24";
    sign.AddSignCall("E8 ? ? ? ? 4C 8B CE 4D 8B C6 48 8B 57 ? 48");
    sign.AddSignCall("75 ? 48 8B 17 4C 8B 47 08 48 8B 52 08 49 8B", 16);

    sign.AddSign("48 8B 8A ? ? ? ? 4D 8B F8 48 8B 01", [](uintptr_t add)->uintptr_t { for(int i = 0; i < 1024; i++) { if(*(byte*)(add - i) == 0xCC) { return add - i + 1; } }return 0; });
    sign.AddSign("48 89 9C 24 ? ? ? ? 48 8B CF 4C 89 74 24", [](uintptr_t add)->uintptr_t { for(int i = 0; i < 1024; i++) { if(*(byte*)(add - i) == 0xCC) { return add - i + 1; } }return 0; });
    // 视角移动飞快xxsign.AddSign("83 ? ? 75 ? 48 8B ? 48 8B ? 48 8B 40 ? FF 15 ? ? ? ? 49 8D 4E ? E8 ? ? ? ? 48", [](uintptr_t add)->uintptr_t { for(int i = 0; i < 1000; i++) { if(*(byte*)(add - i) == 0xCC) { return add - i + 1; } }return 0; });
    //Hook
    if(sign) {
        hook = HookManager::getInstance()->addHook(*sign, Block_playerDestroy);
        hook->hook();
    }
    else {
        LogPrint("[error] 没有一个可用的特征码");
    }
}

void exit(HMODULE hModule) {
    ModExitTag = true;
    if(Tag && Id && Tag[0]) Tag[Id] = false;
    if(hook) hook->unhook();
}


// 此为ImGui用于渲染的接口
extern "C" __declspec(dllexport) void __stdcall ImGuiRender(ImGuiIO* io, ImGuiContext* ctx) {
    ImGui::SetCurrentContext(ctx);
    memcpy_s(&ImGui::GetIO(), sizeof(ImGuiIO), io, sizeof(ImGuiIO));
    

    if(ImGui::Begin("百倍掉落物", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("方块掉落数量");
        ImGui::SliderInt("数量", &blockDestroy, 1, 200);
    }
    ImGui::End();
}

// 此为通知标记更改和日志函数的接口
extern "C" __declspec(dllexport) void __stdcall ImGuiUpdateData(bool tag[], int id, void* funptr) {
    Tag = tag;
    Id = id;
    tag[id] = !ModExitTag;
    imgui_print = funptr;
}


// Dll入口函数
auto APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) -> BOOL {
    if(ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)main, hModule, NULL, nullptr);
    }
    else if(ul_reason_for_call == DLL_PROCESS_DETACH) {
        exit(hModule);
    }

    return TRUE;
}


// 调用者匿名函数开头：48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 4C 8B EA 48 89 54 24 ? 4C 8B F1 48 89 4D ? C7
// 目标函数内部定位：
// 48 8B 59 78 48 85 DB 0F 84 ? ? ? ? 80
// 80 BB ? ? ? ? 00 75 ? E8 ? ? ? ? 84 ? 74 ? 48 8B 83 ? ? ? ? 48 8B 93 ? ? ? ? 48 3B ? 74 ? 48 8B 08 80 79 ? ? 74 ? 48 83 C0 ? 48 3B C2 75 ? EB
// 48 85 ? 74 ? F2 41 0F 10 45 00 F2 0F 11 45 ? 41
// 41 8B 45 ? 89 45 ? 48 8D ? ? ? ? ? 48 89 45 ? 4C
// 4C 89 75 ? 48 89 7D ? 48 8B ? 48
// 48 8D 55 ? 48 8B 40 ? FF 15 ? ? ? ? 90 49 8B 8E
// 48 8B 80 ? ? ? ? FF 15 ? ? ? ? 84 ? 0F 85 ? ? ? ? 49 8B CE
// E8 ? ? ? ? 4C 8B ? 0F 57 ? 33 ? 0F 11 45 ? 48 89 45 ? 49
// 49 8B CE E8 ? ? ? ? 4C 8B ? 0F 57 ? F3 0F 7F 45 ? 33
// 48 89 4D ? 48 83 C0 ? 4D 85 ? 48 0F 44 ? 8D
// 8D 71 ? 48 85 ? 74 ? 48 8B 48
// 48 85 ? 74 ? F0 FF 41 ? 48 8B 48 ? 48 8B 40 ? 48 89 45 ? 48 8B 5D ? 48 89 4D ? 48 85 ? 74 ? 8B C6 F0 0F C1 ? ? 83 F8 ? 75 ? 48 8B ? 48 8B ? 48 8B ? FF ? ? ? ? ? 8B C6 F0 0F C1 43 ? 83 F8 ? 75 ? 48 8B ? 48 8B ? 48 8B 40 ? FF ? ? ? ? ? 48
// 74 ? 4C 89 7D ? 49
// 49 8B 96 ? ? ? ? 80 BA ? ? ? ? ? 75 ? 48 8B 8A ? ? ? ? 48 8B ? 8B 52 ? 48 8B 40 ? FF 15 ? ? ? ? 48 8B ? EB ? 48 8D 1D
// 49 8B ? ? 48 8D ? ? 49 8B ? 48 8B ? ? ? ? ? FF ? ? ? ? ? 8B
// 45 ? ? 44 89 7D ? C7 45 ? ? ? ? ? 48 89
// F2 41 0F 10 ? ? F2 0F 11 45 ?  41 8B 4D ? 89 4D ? 89 45 E3
// 4C 89 65 ? 49 8B ? ? 49 8B ? 48 8B 80 ? ? ? ? FF ? ? ? ? ? 48 8B
// 48 8B ? 48 8B ? ? ? ? ? 48 8B ? FF ? ? ? ? ? 84 ? 75 ? 48 8B ? ? 48 85 ? 0F 84 ? ? ? ? 48 8D 45 ? 48 89 44 24 ? 48 8D 45 ? 48 89 44 24 ? 4C 8B ? 4D 8B ? 49
// E8 ? ? ? ? 90 4C 89 7D ? 48 8B 7D ? 49 8B ? 48 89 5D ? 48 85 ? 74 ? 8B ? F0 0F C1 ? ? 83 F8 ? 75 ? 48 8B ? 48 8B ? 48 8B ? FF ? ? ? ? ? 8B ? F0 0F C1 ? ? 83 F8 ? 75 ? 48 8B ? 48 8B ? 48 8B ? ? FF 15 ? ? ? ? 48 8B 5D ? 4C 89 ? ? 48 85 ? 74 ? 8B ? F0 0F C1 ? ? 83 F8 ? 75 ? 48 8B ? 48 8B ? 48 8B ? FF 15 ? ? ? ? F0 0F
// 83 ? ? 75 ? 48 8B ? 48 8B ? 48 8B 40 ? FF 15 ? ? ? ? 49 8D 4E ? E8 ? ? ? ? 48
// 74 ? F3 0F 10 ? ? 49 8B ? E8 ? ? ? ? 48 8B 4D ? 48 33 ? E8
// 
// 直接正则查找：\(a2, \*\(float \*\)\(v\d\d \+ 12\)\);
