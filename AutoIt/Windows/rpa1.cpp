#include <windows.h>

// Entry point of the application
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    // Display a message box with an information icon
    MessageBox(NULL, L"Hello World!", L"Tutorial", MB_ICONINFORMATION);
    return 0;
}
