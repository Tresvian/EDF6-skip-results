#include <cassert>
#include <iostream>
#include "policy.h"

int main() {
    assert(ShouldSkipResults("Result_Common", true, true));
    assert(!ShouldSkipResults("Result_Common", false, true));
    assert(!ShouldSkipResults("Result_Common", true, false));
    assert(!ShouldSkipResults("Result_Common", false, false));
    assert(!ShouldSkipResults("HQMain", true, true));
    assert(!ShouldSkipResults("Dialog_Message", true, true));
    assert(!ShouldSkipResults("Result_Offline", true, true));
    assert(!ShouldSkipResults("Result_CommonExtra", true, true));
    assert(!ShouldSkipResults(nullptr, true, true));
    std::cout << "ResultsSkip: all 9 context/input policy checks passed.\n";
}
