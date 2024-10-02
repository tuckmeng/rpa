#include <windows.h>
#include <regex>
#include <string>

// Function to perform the task
void PerformTask() {
    const std::string text = "You used 36 of 279 pages.";
    const std::regex regex(R"([0-9]{1,3})");
    std::smatch match;

    // Find the first match
    if (std::regex_search(text, match, regex)) {
        // Convert match to wide string for MessageBox
        std::wstring wideMatch = std::wstring(match.str(0).begin(), match.str(0).end());
        MessageBoxW(NULL, wideMatch.c_str(), L"SRE Example Result", MB_OK | MB_ICONINFORMATION);
    }
}

// WinMain function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    PerformTask();
    return 0;
}
