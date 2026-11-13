#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <vector>
#include "policy.h"

// EDFModLoader Plugin API v1: https://github.com/BlueAmulet/EDFModLoader
struct PluginInfo { unsigned long infoVersion; const char* name; std::uint64_t version; };
static_assert(sizeof(PluginInfo) == 24);
static unsigned char* gameBase;

static bool SameBuild(const std::filesystem::path& dll) {
    static constexpr unsigned char expected[32] = {
        0x0d,0x50,0x92,0x39,0x3a,0x1b,0x9d,0x8f,0x1b,0x75,0xb0,0xc9,0x3d,0x8a,0x8f,0x65,
        0x61,0x5a,0x36,0xa9,0x2c,0x46,0xe4,0x03,0x49,0xc2,0xb6,0x6c,0x73,0x3d,0x09,0x16
    };
    std::ifstream file(dll, std::ios::binary);
    if (!file) return false;
    std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)), {});
    if (file.bad() || bytes.empty() || bytes.size() > ULONG_MAX) return false;
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0) return false;
    unsigned char digest[32];
    auto status = BCryptHash(algorithm, nullptr, 0, bytes.data(), static_cast<ULONG>(bytes.size()), digest, sizeof digest);
    BCryptCloseAlgorithmProvider(algorithm, 0);
    return status >= 0 && std::memcmp(digest, expected, sizeof digest) == 0;
}

static bool __fastcall CloseRequested(void* ui) {
    auto original = reinterpret_cast<bool(__fastcall*)(void*)>(gameBase + 0x714c70);
    if (original(ui)) return true;
    if (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000)) return false;
    DWORD foregroundPid = 0;
    GetWindowThreadProcessId(GetForegroundWindow(), &foregroundPid);
    if (foregroundPid != GetCurrentProcessId()) return false;

    // These native AngelScript entry points are gated by the exact EDF.dll hash.
    auto activeContext = reinterpret_cast<void*(*)()>(gameBase + 0x11e5290);
    auto getFunction = reinterpret_cast<void*(__fastcall*)(void*, unsigned)>(gameBase + 0x11e6f90);
    auto getName = reinterpret_cast<const char*(__fastcall*)(void*)>(gameBase + 0x11f2110);
    void* context = activeContext();
    void* function = context ? getFunction(context, 0) : nullptr;
    if (!ShouldSkipResults(function ? getName(function) : nullptr, true, true)) return false;

    // The script has already awarded loot and started its normal save before this wait.
    return true;
}

static void* NearbyPage(std::uintptr_t target) {
    SYSTEM_INFO info{};
    GetSystemInfo(&info);
    auto granularity = static_cast<std::uintptr_t>(info.dwAllocationGranularity);
    auto aligned = target & ~(granularity - 1);
    for (std::uintptr_t distance = granularity; distance < 0x7fff0000; distance += granularity) {
        for (bool above : {true, false}) {
            if (!above && aligned < distance) continue;
            auto candidate = above ? aligned + distance : aligned - distance;
            if (void* page = VirtualAlloc(reinterpret_cast<void*>(candidate), 4096,
                                          MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)) return page;
        }
    }
    return nullptr;
}

extern "C" __declspec(dllexport) bool __fastcall EML6_Load(PluginInfo* info) {
    if (!info) return false;
    gameBase = reinterpret_cast<unsigned char*>(GetModuleHandleW(L"EDF.dll"));
    if (!gameBase) return false;
    wchar_t gamePath[32768];
    DWORD gameLength = GetModuleFileNameW(reinterpret_cast<HMODULE>(gameBase), gamePath, static_cast<DWORD>(std::size(gamePath)));
    if (!gameLength || gameLength >= std::size(gamePath) || !SameBuild(gamePath)) {
        return false;
    }

    // Replace only the close-query CALL inside UI::WaitCloseRequest, not the shared query function.
    auto call = gameBase + 0x71533e;
    static constexpr unsigned char expectedCall[] = {0xe8,0x2d,0xf9,0xff,0xff};
    if (std::memcmp(call, expectedCall, sizeof expectedCall) != 0) {
        return false;
    }
    auto stub = static_cast<unsigned char*>(NearbyPage(reinterpret_cast<std::uintptr_t>(call)));
    if (!stub) return false;
    const unsigned char indirectJump[] = {0xff,0x25,0,0,0,0};
    std::memcpy(stub, indirectJump, sizeof indirectJump);
    auto destination = &CloseRequested;
    std::memcpy(stub + 6, &destination, sizeof destination);
    auto distance = reinterpret_cast<std::intptr_t>(stub) - reinterpret_cast<std::intptr_t>(call + 5);
    if (distance < INT32_MIN || distance > INT32_MAX) { VirtualFree(stub, 0, MEM_RELEASE); return false; }
    DWORD oldProtection;
    if (!VirtualProtect(stub, 4096, PAGE_EXECUTE_READ, &oldProtection)) {
        VirtualFree(stub, 0, MEM_RELEASE); return false;
    }
    FlushInstructionCache(GetCurrentProcess(), stub, 14);
    if (!VirtualProtect(call, 5, PAGE_EXECUTE_READWRITE, &oldProtection)) {
        VirtualFree(stub, 0, MEM_RELEASE); return false;
    }
    // EDFModLoader invokes this during startup, before scripts run. Keep the DLL loaded thereafter.
    auto relative = static_cast<std::int32_t>(distance);
    std::memcpy(call + 1, &relative, sizeof relative);
    DWORD ignored;
    VirtualProtect(call, 5, oldProtection, &ignored);
    FlushInstructionCache(GetCurrentProcess(), call, 5);
    info->infoVersion = 1;
    info->name = "ResultsSkip";
    info->version = (std::uint64_t{1} << 32) | (std::uint64_t{1} << 16); // 0.1.1.0
    return true;
}
