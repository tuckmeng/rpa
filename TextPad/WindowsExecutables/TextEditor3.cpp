#include <windows.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <regex>
#include <sstream>

#define IDC_MAIN_EDIT 101
#define IDC_GOTO_EDIT 102
#define IDC_SEARCH_EDIT 103
#define IDC_REPLACE_EDIT 104
#define IDC_REPLACE_WITH_EDIT 105

#define ID_FILE_NEW 40001
#define ID_FILE_OPEN 40002
#define ID_FILE_SAVE 40003
#define ID_FILE_SAVE_AS 40004
#define ID_EDIT_GOTO 40005
#define ID_EDIT_SEARCH_STRING 40006
#define ID_EDIT_SEARCH_REGEXP 40007
#define ID_EDIT_REPLACE_STRING 40008
#define ID_EDIT_REPLACE_REGEXP 40009

HWND hEdit;
std::string currentFileName;

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK GotoDialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SearchReplaceDialogProc(HWND, UINT, WPARAM, LPARAM);
void LoadFile(const char* fileName);
void SaveFile(const char* fileName);
void ClearEditControl();
void ReplaceInText(const std::string& search, const std::string& replace, bool isRegex);
void ShowGotoDialog(HWND hWnd);
void ShowSearchReplaceDialog(HWND hWnd, bool isRegex);
bool isRegex = FALSE;

// Main program entry
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
                            hWnd, (HMENU)IDC_MAIN_EDIT, hInstance, NULL);

    HMENU hMenu = CreateMenu();
    HMENU hFileMenu = CreateMenu();
    HMENU hEditMenu = CreateMenu();

    AppendMenu(hFileMenu, MF_STRING, ID_FILE_NEW, "New");
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_OPEN, "Open");
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_SAVE, "Save");
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_SAVE_AS, "Save As");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, "File");

    AppendMenu(hEditMenu, MF_STRING, ID_EDIT_GOTO, "Go To...");
    AppendMenu(hEditMenu, MF_STRING, ID_EDIT_SEARCH_STRING, "Search String...");
    AppendMenu(hEditMenu, MF_STRING, ID_EDIT_SEARCH_REGEXP, "Search Regexp...");
    AppendMenu(hEditMenu, MF_STRING, ID_EDIT_REPLACE_STRING, "Replace String...");
    AppendMenu(hEditMenu, MF_STRING, ID_EDIT_REPLACE_REGEXP, "Replace Regexp String...");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hEditMenu, "Edit");

    SetMenu(hWnd, hMenu);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// Window procedure for handling messages
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_FILE_NEW:
                    ClearEditControl();
                    break;
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
                    ShowGotoDialog(hWnd);
                    break;
                case ID_EDIT_SEARCH_STRING:
                    ShowSearchReplaceDialog(hWnd, false);
                    break;
                case ID_EDIT_SEARCH_REGEXP:
                    isRegex = TRUE;
                    ShowSearchReplaceDialog(hWnd, true);
                    break;
                case ID_EDIT_REPLACE_STRING: {
                    ShowSearchReplaceDialog(hWnd, false);
                    break;
                }
                case ID_EDIT_REPLACE_REGEXP: {
                    isRegex= TRUE;
                    ShowSearchReplaceDialog(hWnd, true);
                    break;
                }
            }
            break;
        case WM_SIZE:
            MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam) - 30, TRUE);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Load file function
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

// Save file function
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

// Clear the edit control
void ClearEditControl() {
    SetWindowText(hEdit, "");
    currentFileName.clear();
}

// Show Go To dialog
void ShowGotoDialog(HWND hWnd) {
    HWND hDialog = CreateDialog(GetModuleHandle(NULL), NULL, hWnd, GotoDialogProc);
    ShowWindow(hDialog, SW_SHOW);
}

// Go To dialog procedure
INT_PTR CALLBACK GotoDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static HWND hLineEdit;

    switch (message) {
        case WM_INITDIALOG:
            hLineEdit = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER, 
                                      10, 10, 180, 20, hDlg, NULL, NULL, NULL);
            CreateWindow("BUTTON", "OK", WS_CHILD | WS_VISIBLE, 
                         10, 40, 80, 25, hDlg, (HMENU)IDOK, NULL, NULL);
            CreateWindow("BUTTON", "Cancel", WS_CHILD | WS_VISIBLE, 
                         110, 40, 80, 25, hDlg, (HMENU)IDCANCEL, NULL, NULL);
            return TRUE;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                char lineStr[10];
                GetWindowText(hLineEdit, lineStr, sizeof(lineStr));
                int lineNumber = atoi(lineStr);
                
                // Simple logic to go to the specified line
                // Calculate the position based on line number
                // For demonstration, this will just show a message
                std::ostringstream msg;
                msg << "Going to line: " << lineNumber;
                MessageBox(hDlg, msg.str().c_str(), "Go To", MB_OK);
                
                EndDialog(hDlg, IDOK);
                return TRUE;
            } else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

// Show Search/Replace dialog
void ShowSearchReplaceDialog(HWND hWnd, bool isRegex) {
    HWND hDialog = CreateDialog(GetModuleHandle(NULL), NULL, hWnd, SearchReplaceDialogProc);
    ShowWindow(hDialog, SW_SHOW);
}

// Search/Replace dialog procedure
INT_PTR CALLBACK SearchReplaceDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static HWND hSearchEdit, hReplaceEdit;

    switch (message) {
        case WM_INITDIALOG:
            hSearchEdit = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
                                        10, 10, 180, 20, hDlg, (HMENU)IDC_SEARCH_EDIT, NULL, NULL);
            hReplaceEdit = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
                                         10, 40, 180, 20, hDlg, (HMENU)IDC_REPLACE_WITH_EDIT, NULL, NULL);
            CreateWindow("BUTTON", "Search", WS_CHILD | WS_VISIBLE,
                         10, 70, 80, 25, hDlg, (HMENU)IDOK, NULL, NULL);
            CreateWindow("BUTTON", "Cancel", WS_CHILD | WS_VISIBLE,
                         110, 70, 80, 25, hDlg, (HMENU)IDCANCEL, NULL, NULL);
            return TRUE;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                char searchStr[256], replaceStr[256];
                GetWindowText(hSearchEdit, searchStr, sizeof(searchStr));
                GetWindowText(hReplaceEdit, replaceStr, sizeof(replaceStr));

                if (isRegex) {
                    // Call ReplaceInText with regex
                    ReplaceInText(searchStr, replaceStr, true);
                } else {
                    // Call ReplaceInText without regex
                    ReplaceInText(searchStr, replaceStr, false);
                }

                EndDialog(hDlg, IDOK);
                return TRUE;
            } else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

// Replace in text function
void ReplaceInText(const std::string& search, const std::string& replace, bool isRegex) {
    std::string text;
    int length = GetWindowTextLength(hEdit);
    std::vector<char> buffer(length + 1);
    GetWindowText(hEdit, buffer.data(), length + 1);
    text.assign(buffer.data(), length);

    if (!isRegex) {
        // Simple replace logic
        size_t pos = 0;
        while ((pos = text.find(search, pos)) != std::string::npos) {
            text.replace(pos, search.length(), replace);
            pos += replace.length();
        }
    } else {
        // Regex replace logic
        std::regex re(search);
        text = std::regex_replace(text, re, replace);
    }
    
    SetWindowText(hEdit, text.c_str());
}
