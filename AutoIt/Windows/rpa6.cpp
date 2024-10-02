#include <windows.h>
#include <string>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <thread>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Get current time
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm* local_time = std::localtime(&now_time);

        // Format time as HH:MM:SS
        std::ostringstream oss;
        oss << std::put_time(local_time, "%H:%M:%S");
        std::string time_str = oss.str();

        // Set text color and background
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(0, 0, 0)); // Black text
        RECT rect;
        GetClientRect(hwnd, &rect);
        DrawTextA(hdc, time_str.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        EndPaint(hwnd, &ps);
    } return 0;
    case WM_TIMER: {
        InvalidateRect(hwnd, NULL, TRUE); // Request to repaint the window
        return 0;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nShowCmd) {
    const char CLASS_NAME[] = "DigitalClockClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,                            // Optional window styles.
        CLASS_NAME,                  // Window class
        "Digital Clock",             // Window text
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, 250, 100, // Size and position
        NULL,                        // Parent window
        NULL,                        // Menu
        hInstance,                  // Instance handle
        NULL                         // Additional application data
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nShowCmd);

    SetTimer(hwnd, 1, 1000, NULL); // Set a timer to update every second

    // Run the message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
