#include <windows.h>
#include <string>
#include <regex>
#include <sstream>


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nShowCmd) {
 
    // String to be searched
    std::string input = "You used 36 of 279 pages.";
    std::regex regex(R"(([0-9]{1,3})(?: pages))");
    std::smatch match;

    // Perform regex matching
    if (std::regex_search(input, match, regex)) {
        std::string result = match[1]; // Get the matched number
        // Show the result in a message box
        MessageBoxA(NULL, result.c_str(), "SRE Example 6 Result", MB_OK);
    }
    return 0;
}
