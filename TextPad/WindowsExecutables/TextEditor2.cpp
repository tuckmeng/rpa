#include <windows.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <regex>
#include <sstream>

// Control IDs
#define IDC_MAIN_EDIT 101
#define IDC_GOTO_EDIT 102
#define IDC_SEARCH_EDIT 103
#define IDC_REPLACE_EDIT 104

// Menu IDs
#define ID_FILE_OPEN 40001
#define ID_FILE_SAVE 40002
#define ID_FILE_SAVE_AS 40003
#define ID_EDIT_GOTO 40004
#define ID_EDIT_SEARCH_STRING 40005
#define ID_EDIT_SEARCH_REGEX 40006
#define ID_EXECUTE 40007
#define ID_COPY_PATH 40008

HWND hEdit;
std::string currentFileName;

struct CHARINFO {
    int iChar;
    int iLine;
};
CHARINFO charInfo;

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK GotoDialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SearchReplaceDialogProc(HWND, UINT, WPARAM, LPARAM);

// Function to create menus
HMENU CreateCustomMenu() {
    HMENU hMenu = CreateMenu();
    HMENU hFileMenu = CreatePopupMenu();
    HMENU hEditMenu = CreatePopupMenu();

    AppendMenu(hFileMenu, MF_STRING, ID_FILE_OPEN, "Open");
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_SAVE, "Save");
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_SAVE_AS, "Save As");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, "File");

    AppendMenu(hEditMenu, MF_STRING, ID_EDIT_GOTO, "Go To Line");
    AppendMenu(hEditMenu, MF_STRING, ID_EDIT_SEARCH_STRING, "Search (String)");
    AppendMenu(hEditMenu, MF_STRING, ID_EDIT_SEARCH_REGEX, "Search (Regex)");
    AppendMenu(hEditMenu, MF_STRING, ID_EXECUTE, "Execute Command");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hEditMenu, "Edit");

    return hMenu;
}

// Dialog template creation functions
LPCDLGTEMPLATE CreateGotoDialogTemplate() {
    static WORD wGotoBuffer[128];
    WORD* pw = wGotoBuffer;

    // Dialog header
    *pw++ = LOWORD(DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU);
    *pw++ = HIWORD(DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU);
    *pw++ = 0; // Number of items
    *pw++ = 10; // x
    *pw++ = 10; // y
    *pw++ = 200; // cx
    *pw++ = 100; // cy
    *pw++ = 0; // Menu
    *pw++ = 0; // Class

    // Dialog title
    const wchar_t title[] = L"Go To Line";
    wcscpy((wchar_t*)pw, title);
    pw += wcslen(title) + 1;

    // Font
    *pw++ = 8; // Point size
    const wchar_t font[] = L"MS Shell Dlg";
    wcscpy((wchar_t*)pw, font);
    pw += wcslen(font) + 1;

    // Edit control
    DLGITEMTEMPLATE* pItem = (DLGITEMTEMPLATE*)pw;
    pItem->style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER;
    pItem->dwExtendedStyle = 0;
    pItem->x = 10;
    pItem->y = 10;
    pItem->cx = 180;
    pItem->cy = 14;
    pItem->id = IDC_GOTO_EDIT;

    pw = (WORD*)(pItem + 1);
    *pw++ = 0xFFFF;
    *pw++ = 0x0081; // Edit control
    *pw++ = 0;      // No creation data

    // OK button
    pItem = (DLGITEMTEMPLATE*)pw;
    pItem->style = WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON;
    pItem->dwExtendedStyle = 0;
    pItem->x = 60;
    pItem->y = 70;
    pItem->cx = 50;
    pItem->cy = 14;
    pItem->id = IDOK;

    pw = (WORD*)(pItem + 1);
    *pw++ = 0xFFFF;
    *pw++ = 0x0080; // Button
    const wchar_t okText[] = L"OK";
    wcscpy((wchar_t*)pw, okText);
    pw += wcslen(okText) + 1;
    *pw++ = 0;      // No creation data

    // Cancel button
    pItem = (DLGITEMTEMPLATE*)pw;
    pItem->style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
    pItem->dwExtendedStyle = 0;
    pItem->x = 120;
    pItem->y = 70;
    pItem->cx = 50;
    pItem->cy = 14;
    pItem->id = IDCANCEL;

    pw = (WORD*)(pItem + 1);
    *pw++ = 0xFFFF;
    *pw++ = 0x0080; // Button
    const wchar_t cancelText[] = L"Cancel";
    wcscpy((wchar_t*)pw, cancelText);
    pw += wcslen(cancelText) + 1;
    *pw++ = 0;      // No creation data

    wGotoBuffer[2] = 3; // Update number of items

    return (LPCDLGTEMPLATE)wGotoBuffer;
}

LPCDLGTEMPLATE CreateSearchReplaceDialogTemplate() {
    static WORD wSearchBuffer[256];
    WORD* pw = wSearchBuffer;

    // Dialog header
    *pw++ = LOWORD(DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU);
    *pw++ = HIWORD(DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU);
    *pw++ = 0; // Number of items
    *pw++ = 10; // x
    *pw++ = 10; // y
    *pw++ = 250; // cx
    *pw++ = 120; // cy
    *pw++ = 0; // Menu
    *pw++ = 0; // Class

    // Dialog title
    const wchar_t title[] = L"Search and Replace";
    wcscpy((wchar_t*)pw, title);
    pw += wcslen(title) + 1;

    // Font
    *pw++ = 8; // Point size
    const wchar_t font[] = L"MS Shell Dlg";
    wcscpy((wchar_t*)pw, font);
    pw += wcslen(font) + 1;

    // Search edit control
    DLGITEMTEMPLATE* pItem = (DLGITEMTEMPLATE*)pw;
    pItem->style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;
    pItem->dwExtendedStyle = 0;
    pItem->x = 70;
    pItem->y = 10;
    pItem->cx = 170;
    pItem->cy = 14;
    pItem->id = IDC_SEARCH_EDIT;

    pw = (WORD*)(pItem + 1);
    *pw++ = 0xFFFF;
    *pw++ = 0x0081; // Edit control
    *pw++ = 0;      // No creation data

    // Replace edit control
    pItem = (DLGITEMTEMPLATE*)pw;
    pItem->style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;
    pItem->dwExtendedStyle = 0;
    pItem->x = 70;
    pItem->y = 30;
    pItem->cx = 170;
    pItem->cy = 14;
    pItem->id = IDC_REPLACE_EDIT;

    pw = (WORD*)(pItem + 1);
    *pw++ = 0xFFFF;
    *pw++ = 0x0081; // Edit control
    *pw++ = 0;      // No creation data

    // OK button
    pItem = (DLGITEMTEMPLATE*)pw;
    pItem->style = WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON;
    pItem->dwExtendedStyle = 0;
    pItem->x = 80;
    pItem->y = 90;
    pItem->cx = 50;
    pItem->cy = 14;
    pItem->id = IDOK;

    pw = (WORD*)(pItem + 1);
    *pw++ = 0xFFFF;
    *pw++ = 0x0080; // Button
    const wchar_t okText[] = L"OK";
    wcscpy((wchar_t*)pw, okText);
    pw += wcslen(okText) + 1;
    *pw++ = 0;      // No creation data

    // Cancel button
    pItem = (DLGITEMTEMPLATE*)pw;
    pItem->style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
    pItem->dwExtendedStyle = 0;
    pItem->x = 140;
    pItem->y = 90;
    pItem->cx = 50;
    pItem->cy = 14;
    pItem->id = IDCANCEL;

    pw = (WORD*)(pItem + 1);
    *pw++ = 0xFFFF;
    *pw++ = 0x0080; // Button
    const wchar_t cancelText[] = L"Cancel";
    wcscpy((wchar_t*)pw, cancelText);
    pw += wcslen(cancelText) + 1;
    *pw++ = 0;      // No creation data

    wSearchBuffer[2] = 5; // Update number of items

    return (LPCDLGTEMPLATE)wSearchBuffer;
}

// File operations
void LoadFile(const char* fileName) {
    FILE* file = fopen(fileName, "r");
    if (!file) return;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    std::vector<char> buffer(length + 1);
    fread(buffer.data(), 1, length, file);
    buffer[length] = '\0';

    SetWindowText(hEdit, buffer.data());
    fclose(file);
    currentFileName = fileName;
}

void SaveFile(const char* fileName) {
    FILE* file = fopen(fileName, "w");
    if (!file) return;

    int length = GetWindowTextLength(hEdit);
    std::vector<char> buffer(length + 1);
    GetWindowText(hEdit, buffer.data(), length + 1);

    fwrite(buffer.data(), 1, length, file);
    fclose(file);
}

// Command execution function
void ExecuteCommand(const std::string& command) {
    // Implement command execution logic
    MessageBoxA(NULL, command.c_str(), "Command Executed", MB_OK);
}

// Dialog procedures
INT_PTR CALLBACK GotoDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_INITDIALOG) {
        SetFocus(GetDlgItem(hwndDlg, IDC_GOTO_EDIT));
        return TRUE;
    }
    if (uMsg == WM_COMMAND) {
        if (LOWORD(wParam) == IDOK) {
            char lineNumber[10];
            GetDlgItemText(hwndDlg, IDC_GOTO_EDIT, lineNumber, sizeof(lineNumber));
            charInfo.iLine = atoi(lineNumber);
            EndDialog(hwndDlg, IDOK);
            return TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hwndDlg, IDCANCEL);
            return TRUE;
        }
    }
    return FALSE;
}

INT_PTR CALLBACK SearchReplaceDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_INITDIALOG) {
        SetFocus(GetDlgItem(hwndDlg, IDC_SEARCH_EDIT));
        return TRUE;
    }
    if (uMsg == WM_COMMAND) {
        if (LOWORD(wParam) == IDOK) {
            char searchText[256], replaceText[256];
            GetDlgItemText(hwndDlg, IDC_SEARCH_EDIT, searchText, sizeof(searchText));
            GetDlgItemText(hwndDlg, IDC_REPLACE_EDIT, replaceText, sizeof(replaceText));
            // Implement search and replace logic using searchText and replaceText
            EndDialog(hwndDlg, IDOK);
            return TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hwndDlg, IDCANCEL);
            return TRUE;
        }
    }
    return FALSE;
}

// WinMain
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "TextEditorClass";

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        "TextEditorClass",
        "Simple Text Editor",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
        NULL, NULL, hInstance, NULL
    );

    HMENU hMenu = CreateCustomMenu();
    SetMenu(hwnd, hMenu);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    hEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "EDIT",
        "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
        0, 0, 400, 300,
        hwnd,
        (HMENU)IDC_MAIN_EDIT,
        hInstance,
        NULL
    );

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

// WndProc
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_SIZE: {
        if (hEdit) {
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            SetWindowPos(hEdit, NULL, 0, 0, clientRect.right, clientRect.bottom, SWP_NOZORDER);
        }
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_FILE_OPEN: {
            OPENFILENAME ofn;
            char szFile[260] = "";
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile;
            ofn.lpstrFile[0] = '\0';
            ofn.lpstrFilter = "Text files (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrTitle = "Open File";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileName(&ofn)) {
                LoadFile(szFile);
            }
            break;
        }
        case ID_FILE_SAVE: {
            if (currentFileName.empty()) {
                // Implement save as dialog
            } else {
                SaveFile(currentFileName.c_str());
            }
            break;
        }
        case ID_FILE_SAVE_AS: {
            OPENFILENAME ofn;
            char szFile[260] = "";
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile;
            ofn.lpstrFile[0] = '\0';
            ofn.lpstrFilter = "Text files (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrTitle = "Save As";
            ofn.Flags = OFN_OVERWRITEPROMPT;

            if (GetSaveFileName(&ofn)) {
                SaveFile(szFile);
                currentFileName = szFile; // Update the current file name
            }
            break;
        }
        case ID_EDIT_GOTO: {
            DialogBox(GetModuleHandle(NULL), (LPCSTR)CreateGotoDialogTemplate(), hwnd, GotoDialogProc);
            // Implement logic to move to the specified line
            break;
        }
        case ID_EDIT_SEARCH_STRING:
        case ID_EDIT_SEARCH_REGEX: {
            DialogBox(GetModuleHandle(NULL), (LPCSTR)CreateSearchReplaceDialogTemplate(), hwnd, SearchReplaceDialogProc);
            break;
        }
        case ID_EXECUTE: {
            char command[256];
            GetDlgItemText(hEdit, IDC_MAIN_EDIT, command, sizeof(command)); // Use IDC_MAIN_EDIT
            ExecuteCommand(command);
            break;
        }
        case ID_COPY_PATH: {
            // Implement copy path logic
            break;
        }
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
