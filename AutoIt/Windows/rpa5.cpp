#include <windows.h>
#include <commctrl.h>
#include <string>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateButtons(HWND hwnd);
void UpdateDisplay(HWND hwnd, const std::wstring& text);

double currentValue = 0.0;
std::wstring displayText = L"0";
bool newEntry = true;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nShowCmd) {
    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES; // Specify classes for common controls
    InitCommonControlsEx(&icex);

    // Register the window class
    const wchar_t CLASS_NAME[] = L"CalculatorClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    // Create the window
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Calculator",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 270, // Adjusted width and height
        NULL, NULL, hInstance, NULL
    );

    // Run the message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

void UpdateDisplay(HWND hwnd, const std::wstring& text) {
    SetDlgItemText(hwnd, 1, text.c_str());
}

void CreateButtons(HWND hwnd) {
    CreateWindow(L"EDIT", L"0", WS_VISIBLE | WS_CHILD | ES_READONLY | ES_RIGHT | WS_BORDER,
        8, 2, 270, 23, hwnd, (HMENU)1, NULL, NULL);

    // Adjust button positions and sizes to fit within the new window size
    CreateWindow(L"BUTTON", L"7", WS_TABSTOP | WS_VISIBLE | WS_CHILD, 10, 40, 40, 40, hwnd, (HMENU)7, NULL, NULL);
    CreateWindow(L"BUTTON", L"8", WS_TABSTOP | WS_VISIBLE | WS_CHILD, 60, 40, 40, 40, hwnd, (HMENU)8, NULL, NULL);
    CreateWindow(L"BUTTON", L"9", WS_TABSTOP | WS_VISIBLE | WS_CHILD, 110, 40, 40, 40, hwnd, (HMENU)9, NULL, NULL);
    CreateWindow(L"BUTTON", L"/", WS_TABSTOP | WS_VISIBLE | WS_CHILD, 160, 40, 40, 40, hwnd, (HMENU)20, NULL, NULL);

    CreateWindow(L"BUTTON", L"4", WS_TABSTOP | WS_VISIBLE | WS_CHILD, 10, 90, 40, 40, hwnd, (HMENU)4, NULL, NULL);
    CreateWindow(L"BUTTON", L"5", WS_TABSTOP | WS_VISIBLE | WS_CHILD, 60, 90, 40, 40, hwnd, (HMENU)5, NULL, NULL);
    CreateWindow(L"BUTTON", L"6", WS_TABSTOP | WS_VISIBLE | WS_CHILD, 110, 90, 40, 40, hwnd, (HMENU)6, NULL, NULL);
    CreateWindow(L"BUTTON", L"*", WS_TABSTOP | WS_VISIBLE | WS_CHILD, 160, 90, 40, 40, hwnd, (HMENU)21, NULL, NULL);

    CreateWindow(L"BUTTON", L"1", WS_TABSTOP | WS_VISIBLE | WS_CHILD, 10, 140, 40, 40, hwnd, (HMENU)1, NULL, NULL);
    CreateWindow(L"BUTTON", L"2", WS_TABSTOP | WS_VISIBLE | WS_CHILD, 60, 140, 40, 40, hwnd, (HMENU)2, NULL, NULL);
    CreateWindow(L"BUTTON", L"3", WS_TABSTOP | WS_VISIBLE | WS_CHILD, 110, 140, 40, 40, hwnd, (HMENU)3, NULL, NULL);
    CreateWindow(L"BUTTON", L"-", WS_TABSTOP | WS_VISIBLE | WS_CHILD, 160, 140, 40, 40, hwnd, (HMENU)22, NULL, NULL);

    CreateWindow(L"BUTTON", L"0", WS_TABSTOP | WS_VISIBLE | WS_CHILD, 10, 190, 90, 40, hwnd, (HMENU)0, NULL, NULL);
    CreateWindow(L"BUTTON", L".", WS_TABSTOP | WS_VISIBLE | WS_CHILD, 110, 190, 40, 40, hwnd, (HMENU)10, NULL, NULL);
    CreateWindow(L"BUTTON", L"+", WS_TABSTOP | WS_VISIBLE | WS_CHILD, 160, 190, 40, 40, hwnd, (HMENU)23, NULL, NULL);
    CreateWindow(L"BUTTON", L"=", WS_TABSTOP | WS_VISIBLE | WS_CHILD, 210, 40, 40, 90, hwnd, (HMENU)24, NULL, NULL);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_CREATE) {
        CreateButtons(hwnd);
    }
    else if (uMsg == WM_COMMAND) {
        // Do nothing on button press
    }
    else if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
