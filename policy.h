#pragma once
#include <cstring>

inline bool ShouldSkipResults(const char* function, bool escapeDown, bool gameFocused) {
    return escapeDown && gameFocused && function && std::strcmp(function, "Result_Common") == 0;
}
