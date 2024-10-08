#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <iomanip>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

const int INITIAL_ROWS = 100;
const int INITIAL_COLS = 26;

HWND hWnd;
HWND hGrid;
int current_row = 0;
int current_col = 0;

// Function prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateGrid(HWND parent);
void AddRow();
void AddColumn();
void MoveFocus(int row, int col);
void SaveDataXLSX(const char* filename);
void OnMenuFileNew();
void OnMenuFileSaveAs();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Initialize common controls
    INITCOMMONCONTROLSEX icex = {sizeof(INITCOMMONCONTROLSEX), ICC_WIN95_CLASSES};
    InitCommonControlsEx(&icex);

    // Register the window class
    const char CLASS_NAME[] = "Simple Spreadsheet Class";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    // Create the window
    hWnd = CreateWindowEx(
        0,                              // Optional window styles
        CLASS_NAME,                     // Window class
        "Simple Spreadsheet",           // Window text
        WS_OVERLAPPEDWINDOW,            // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, // Size and position
        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hWnd == NULL)
    {
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);

    // Run the message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        // Create menu
        HMENU hMenu = CreateMenu();
        HMENU hFileMenu = CreatePopupMenu();

        AppendMenu(hFileMenu, MF_STRING, 1, "New");
        AppendMenu(hFileMenu, MF_STRING, 2, "Save As");
        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, "File");

        SetMenu(hwnd, hMenu);

        // Create grid
        CreateGrid(hwnd);
        MoveFocus(0, 0);
        return 0;
    }

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case 1:
            OnMenuFileNew();
            break;
        case 2:
            OnMenuFileSaveAs();
            break;
        }
        return 0;
    }

    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_UP:
            MoveFocus(current_row - 1, current_col);
            break;
        case VK_DOWN:
            MoveFocus(current_row + 1, current_col);
            break;
        case VK_LEFT:
            MoveFocus(current_row, current_col - 1);
            break;
        case VK_RIGHT:
            MoveFocus(current_row, current_col + 1);
            break;
        }
        return 0;
    }

    case WM_DESTROY:
        SaveDataXLSX("auto_save.xlsx");
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
    {
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        SetWindowPos(hGrid, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
        return 0;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CreateGrid(HWND parent)
{
    hGrid = CreateWindowEx(
        0,
        WC_LISTVIEW,
        "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EDITLABELS,
        0, 0, 0, 0,
        parent,
        NULL,
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE),
        NULL);

    // Add columns
    for (int i = 0; i < INITIAL_COLS; ++i)
    {
        LVCOLUMN lvc = {LVCF_TEXT | LVCF_WIDTH, 0, 80};
        char colName[3] = { (char)('A' + i), '\0' };
        lvc.pszText = colName;
        ListView_InsertColumn(hGrid, i, &lvc);
    }

    // Add rows
    for (int i = 0; i < INITIAL_ROWS; ++i)
    {
        LVITEM lvi = {LVIF_TEXT, i, 0, 0, 0, NULL, 0};
        ListView_InsertItem(hGrid, &lvi);
    }
}

void AddRow()
{
    int newRow = ListView_GetItemCount(hGrid);
    LVITEM lvi = {LVIF_TEXT, newRow, 0, 0, 0, NULL, 0};
    ListView_InsertItem(hGrid, &lvi);
}

void AddColumn()
{
    int newCol = Header_GetItemCount(ListView_GetHeader(hGrid));
    LVCOLUMN lvc = {LVCF_TEXT | LVCF_WIDTH, 0, 80};
    char colName[3] = { (char)('A' + newCol), '\0' };
    lvc.pszText = colName;
    ListView_InsertColumn(hGrid, newCol, &lvc);
}

void MoveFocus(int row, int col)
{
    if (row < 0 || col < 0) return;

    while (row >= ListView_GetItemCount(hGrid)) AddRow();
    while (col >= Header_GetItemCount(ListView_GetHeader(hGrid))) AddColumn();

    ListView_SetItemState(hGrid, row, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
    ListView_EnsureVisible(hGrid, row, FALSE);
    ListView_EditLabel(hGrid, row);

    current_row = row;
    current_col = col;
}

void SaveDataXLSX(const char* filename)
{
    // Create a temporary directory
    char tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);
    char tempDir[MAX_PATH];
    sprintf_s(tempDir, "%sxlsx_temp_%d", tempPath, GetTickCount());
    CreateDirectory(tempDir, NULL);

    // Create the necessary directories
    char xlDir[MAX_PATH], worksheetsDir[MAX_PATH];
    sprintf_s(xlDir, "%s\\xl", tempDir);
    sprintf_s(worksheetsDir, "%s\\worksheets", xlDir);
    CreateDirectory(xlDir, NULL);
    CreateDirectory(worksheetsDir, NULL);

    // Generate content types file
    std::ofstream contentTypes(std::string(tempDir) + "\\" + "[Content_Types].xml");
    contentTypes << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        << "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
        << "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
        << "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
        << "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
        << "<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
        << "</Types>";
    contentTypes.close();

    // Generate _rels/.rels file
    CreateDirectory((std::string(tempDir) + "\\_rels").c_str(), NULL);
    std::ofstream rels(std::string(tempDir) + "\\_rels\\.rels");
    rels << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        << "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
        << "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/>"
        << "</Relationships>";
    rels.close();

    // Generate xl/workbook.xml file
    std::ofstream workbook(std::string(xlDir) + "\\workbook.xml");
    workbook << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        << "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
        << "<sheets>"
        << "<sheet name=\"Sheet1\" sheetId=\"1\" rId=\"rId1\"/>"
        << "</sheets>"
        << "</workbook>";
    workbook.close();

    // Generate xl/_rels/workbook.xml.rels file
    CreateDirectory((std::string(xlDir) + "\\_rels").c_str(), NULL);
    std::ofstream workbookRels(std::string(xlDir) + "\\_rels\\workbook.xml.rels");
    workbookRels << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        << "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
        << "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet1.xml\"/>"
        << "</Relationships>";
    workbookRels.close();

    // Generate xl/worksheets/sheet1.xml file
    std::ofstream sheet(std::string(worksheetsDir) + "\\sheet1.xml");
    sheet << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        << "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
        << "<sheetData>";

    int rowCount = ListView_GetItemCount(hGrid);
    int colCount = Header_GetItemCount(ListView_GetHeader(hGrid));

    for (int i = 0; i < rowCount; ++i)
    {
        bool rowHasContent = false;
        std::stringstream rowContent;
        rowContent << "<row r=\"" << (i + 1) << "\">";

        for (int j = 0; j < colCount; ++j)
        {
            char cellContent[256];
            ListView_GetItemText(hGrid, i, j, cellContent, sizeof(cellContent));
            if (cellContent[0] != '\0')
            {
                rowHasContent = true;
                char colName[3] = { (char)('A' + j), '\0' };
                rowContent << "<c r=\"" << colName << (i + 1) << "\" t=\"inlineStr\"><is><t>" << cellContent << "</t></is></c>";
            }
        }

        rowContent << "</row>";

        if (rowHasContent)
        {
            sheet << rowContent.str();
        }
    }

    sheet << "</sheetData></worksheet>";
    sheet.close();

    // Create the XLSX file using 7z
    char cmd[MAX_PATH * 3];
    sprintf_s(cmd, "7z.exe a -tzip \"%s\" \"%s\\*\"", filename, tempDir);
    system(cmd);

    // Clean up temporary directory
    sprintf_s(cmd, "rmdir /s /q \"%s\"", tempDir);
    system(cmd);
}

void OnMenuFileNew()
{
    int itemCount = ListView_GetItemCount(hGrid);
    int colCount = Header_GetItemCount(ListView_GetHeader(hGrid));

    for (int i = 0; i < itemCount; ++i)
    {
        for (int j = 0; j < colCount; ++j)
        {
            ListView_SetItemText(hGrid, i, j, "");
        }
    }

    MoveFocus(0, 0);
}

void OnMenuFileSaveAs()
{
    char filename[MAX_PATH] = {0};
    OPENFILENAME ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = "Excel Files (*.xlsx)\0*.xlsx\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename);
    ofn.lpstrDefExt = "xlsx";
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn))
    {
        SaveDataXLSX(filename);
    }
}
