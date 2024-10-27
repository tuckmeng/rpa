#include <windows.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <regex>
#include <sstream>

#define IDC_EDIT 101
#define ID_FILE_OPEN 40001
#define ID_FILE_SAVE 40002
#define ID_FILE_SAVE_AS 40003
#define ID_EDIT_GOTO 40004
#define ID_EDIT_SEARCH_STRING 40005
#define ID_EDIT_SEARCH_REGEX 40006
#define ID_EXECUTE 40007
#define ID_COPY_PATH 40008
#define IDC_SEARCH 20001
#define IDC_REPLACE 20002

HWND hEdit;
std::string currentFileName;
struct CHARINFO {
    int iChar;  // Character index
    int iLine;  // Line index (optional, if needed)
};
// Create a CHARINFO structure to hold character info
CHARINFO charInfo;

void LoadFile(const char* fileName) {
    FILE* file = fopen(fileName, "r");
    if (file) {
        fseek(file, 0, SEEK_END);
        long length = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        std::vector<char> buffer(length + 1);
        fread(buffer.data(), 1, length, file);
        buffer[length] = '\0';
        SetWindowText(hEdit, buffer.data());
        
        currentFileName = fileName;
        fclose(file);
    }
}

void SaveFile(const char* fileName) {
    FILE* file = fopen(fileName, "w");
    if (file) {
        int length = GetWindowTextLength(hEdit);
        std::vector<char> buffer(length + 1);
        GetWindowText(hEdit, buffer.data(), length + 1);
        fwrite(buffer.data(), 1, length, file);
        currentFileName = fileName;
        fclose(file);
    }
}

void UpdateTitle(HWND hWnd) {
    int start, end;
    SendMessage(hEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
    int line = SendMessage(hEdit, EM_LINEFROMCHAR, (WPARAM)start, 0) + 1;
    int column = start - SendMessage(hEdit, EM_LINEINDEX, (WPARAM)line - 1, 0) + 1;
    std::string title = "[" + currentFileName + "]:L" + std::to_string(line) + ":C" + std::to_string(column);
    SetWindowText(hWnd, title.c_str());
}

INT_PTR CALLBACK GotoDialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static int* lineNumber = nullptr;

    switch (message) {
        case WM_INITDIALOG:
            lineNumber = (int*)lParam;
            return TRUE;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                char buffer[10];
                GetDlgItemText(hwndDlg, IDC_EDIT, buffer, sizeof(buffer));
                *lineNumber = atoi(buffer);
                EndDialog(hwndDlg, IDOK);
            }
            break;
        case WM_CLOSE:
            EndDialog(hwndDlg, 0);
            break;
    }
    return FALSE;
}

void ShowGotoDialog(HWND hWnd) {
    int lineNumber = 0;
    if (DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(101), hWnd, GotoDialogProc, (LPARAM)&lineNumber) == IDOK) {
        int lineIndex = SendMessage(hEdit, EM_LINEINDEX, lineNumber - 1, 0);
        SendMessage(hEdit, EM_SETSEL, lineIndex, lineIndex);
        SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
        UpdateTitle(hWnd);
    }
}

INT_PTR CALLBACK SearchReplaceDialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static char* search = nullptr;
    static char* replace = nullptr;

    switch (message) {
        case WM_INITDIALOG:
            search = new char[256];
            replace = new char[256];
            return TRUE;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                GetDlgItemText(hwndDlg, IDC_SEARCH, search, 256);
                GetDlgItemText(hwndDlg, IDC_REPLACE, replace, 256);
                EndDialog(hwndDlg, IDOK);
            }
            break;
        case WM_CLOSE:
            EndDialog(hwndDlg, 0);
            break;
    }
    return FALSE;
}

void ShowSearchReplaceDialog(HWND hWnd, bool isRegex) {
    char search[256] = "", replace[256] = "";
    if (DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(102), hWnd, SearchReplaceDialogProc, 0) == IDOK) {
        std::vector<char> textBuffer(GetWindowTextLength(hEdit) + 1);
        GetWindowText(hEdit, textBuffer.data(), textBuffer.size());
        
        std::string text(textBuffer.data());
        std::string result;

        if (isRegex) {
            std::regex regexSearch(search);
            result = std::regex_replace(text, regexSearch, replace);
        } else {
            std::string searchStr(search);
            std::string replaceStr(replace);
            size_t pos = 0;
            while ((pos = text.find(searchStr, pos)) != std::string::npos) {
                text.replace(pos, searchStr.length(), replaceStr);
                pos += replaceStr.length();
            }
            result = text;
        }

        SetWindowText(hEdit, result.c_str());
        UpdateTitle(hWnd);
    }
}

void ExecuteCommand(const std::string& command) {
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    ZeroMemory(&pi, sizeof(pi));

    HANDLE hRead, hWrite;
    CreatePipe(&hRead, &hWrite, NULL, 0);
    si.hStdOutput = hWrite;

    if (CreateProcess(NULL, (LPSTR)command.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(hWrite);

        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD bytesRead;
        char buffer[4096];
        std::string output;
        while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            output += buffer;
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hRead);

        SetWindowText(hEdit, output.c_str());
        UpdateTitle(NULL);
    }
}

void CopyPathToClipboard(const std::string& path) {
    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, path.size() + 1);
        memcpy(GlobalLock(hGlob), path.c_str(), path.size() + 1);
        GlobalUnlock(hGlob);
        SetClipboardData(CF_TEXT, hGlob);
        CloseClipboard();
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_FILE_OPEN: {
                    OPENFILENAME ofn = { sizeof(ofn) };
                    char szFile[260] = "";
                    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
                    ofn.lpstrFile = szFile;
                    ofn.nMaxFile = sizeof(szFile);
                    ofn.lpstrTitle = "Open a File";
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                    if (GetOpenFileName(&ofn)) {
                        LoadFile(ofn.lpstrFile);
                    }
                    break;
                }
                case ID_FILE_SAVE:
                    if (currentFileName.empty()) {
                        OPENFILENAME ofn = { sizeof(ofn) };
                        char szFile[260] = "";
                        ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
                        ofn.lpstrFile = szFile;
                        ofn.nMaxFile = sizeof(szFile);
                        ofn.lpstrTitle = "Save As";
                        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

                        if (GetSaveFileName(&ofn)) {
                            SaveFile(ofn.lpstrFile);
                        }
                    } else {
                        SaveFile(currentFileName.c_str());
                    }
                    break;
                case ID_FILE_SAVE_AS: {
                    OPENFILENAME ofn = { sizeof(ofn) };
                    char szFile[260] = "";
                    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
                    ofn.lpstrFile = szFile;
                    ofn.nMaxFile = sizeof(szFile);
                    ofn.lpstrTitle = "Save As";
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

                    if (GetSaveFileName(&ofn)) {
                        SaveFile(ofn.lpstrFile);
                    }
                    break;
                }
                case ID_EDIT_GOTO:
                    puts("Debug: Goto menu option clicked");
                    ShowGotoDialog(hWnd);
                    break;
                case ID_EDIT_SEARCH_STRING:
                    ShowSearchReplaceDialog(hWnd, false);
                    break;
                case ID_EDIT_SEARCH_REGEX:
                    ShowSearchReplaceDialog(hWnd, true);
                    break;
                case ID_EXECUTE: {
                    char command[256];
                    GetDlgItemText(hEdit, IDC_EDIT, command, sizeof(command));
                    ExecuteCommand(command);
                    break;
                }
                case ID_COPY_PATH:
                    CopyPathToClipboard(currentFileName);
                    break;
            }
            break;
        case WM_SIZE:
            MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam) - 30, TRUE);
            break;
        case WM_CHAR:
            UpdateTitle(hWnd);
            break;
        case WM_LBUTTONDOWN:
            {
                POINT pt;
                pt.x = LOWORD(lParam);
                pt.y = HIWORD(lParam);
                
                // Convert mouse coordinates to character index in the edit control
                SendMessage(hEdit, EM_SETSEL, 0, 0); // Deselect any current selection
                SendMessage(hEdit, EM_CHARFROMPOS, (WPARAM)&pt, (LPARAM)&charInfo);
                
                // Set the selection based on the character index
                SendMessage(hEdit, EM_SETSEL, charInfo.iChar, charInfo.iChar);
                
                UpdateTitle(hWnd); // Update the title with the new cursor position
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    UpdateTitle(hWnd);
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "TextEditor";
    RegisterClass(&wc);

    HWND hWnd = CreateWindow(wc.lpszClassName, "Text Editor", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                             CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | 
                            ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, 0, 0, 800, 570,
                            hWnd, (HMENU)IDC_EDIT, hInstance, NULL);

    HMENU hMenu = CreateMenu();
    HMENU hFileMenu = CreateMenu();
    HMENU hEditMenu = CreateMenu();
    HMENU hToolsMenu = CreateMenu();

    AppendMenu(hFileMenu, MF_STRING, ID_FILE_OPEN, "Open");
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_SAVE, "Save");
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_SAVE_AS, "Save As");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, "File");

    AppendMenu(hEditMenu, MF_STRING, ID_EDIT_GOTO, "Go To...");
    AppendMenu(hEditMenu, MF_STRING, ID_EDIT_SEARCH_STRING, "Search String...");
    AppendMenu(hEditMenu, MF_STRING, ID_EDIT_SEARCH_REGEX, "Search Regex...");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hEditMenu, "Edit");

    AppendMenu(hToolsMenu, MF_STRING, ID_EXECUTE, "Execute Program...");
    AppendMenu(hToolsMenu, MF_STRING, ID_COPY_PATH, "Copy Path");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hToolsMenu, "Tools");

    SetMenu(hWnd, hMenu);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
