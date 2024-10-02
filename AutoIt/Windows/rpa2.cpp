#include <windows.h>
#include <iostream>
#include <thread>

void SendKey(WORD key, bool isKeyDown) {
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key; // Virtual-key code

    if (isKeyDown) {
        // Send key down event
        SendInput(1, &input, sizeof(INPUT));
    }
    else {
        // Send key up event
        input.ki.dwFlags = KEYEVENTF_KEYUP; // Flag to indicate key up
        SendInput(1, &input, sizeof(INPUT));
    }
}

void SendKeys(const char* keys) {
    while (*keys) {
        // Handle special keys (e.g., Alt)
        if (*keys == '!') {
            SendKey(VK_MENU, true); // Press Alt
            keys++;
            continue;
        }

        // Get the virtual key code for the character
        int vk = VkKeyScan(*keys);
        if (vk == -1) {
            keys++; // Skip if the key is not found
            continue;
        }

        SendKey(static_cast<WORD>(vk), true); // Send key down
        SendKey(static_cast<WORD>(vk), false); // Send key up
        keys++;
    }

    // Release the Alt key if it was pressed
    if (*(keys - 1) == '!') {
        SendKey(VK_MENU, false); // Release Alt
    }
}

void SendAltF4() {
    SendKey(VK_MENU, true); // Press Alt
    SendKey(VK_F4, true);   // Press F4
    SendKey(VK_F4, false);  // Release F4
    SendKey(VK_MENU, false); // Release Alt
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    // Start Notepad
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (CreateProcess(L"C:\\Windows\\System32\\notepad.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        // Wait for Notepad to open
        Sleep(1000); // Simple delay for Notepad to start

        // Find the Notepad window
        HWND hwnd = FindWindowA(NULL, "Untitled - Notepad");
        if (hwnd) {
            // Bring the Notepad window to the foreground
            SetForegroundWindow(hwnd);

            // Send text to Notepad
            SendKeys("This is some text.");

            // Close Notepad using Alt + F4
            SendAltF4();
            Sleep(1000);  // Wait for the prompt

            // Check for the Save dialog
            HWND saveDialog = FindWindowA(NULL, "Notepad");
            if (saveDialog) {
                SetForegroundWindow(saveDialog);
                SendKeys("!n");  // Alt + N to select "Don't Save"
            }
        }
        else {
            std::cerr << "Notepad window not found.\n";
        }

        // Close the process handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        std::cerr << "Failed to start Notepad.\n";
    }

    return 0;
}
