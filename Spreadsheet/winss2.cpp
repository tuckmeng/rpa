#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <shlobj.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <zip.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

const int INITIAL_ROWS = 100;
const int INITIAL_COLS = 26;
const int CELL_WIDTH = 80;
const int CELL_HEIGHT = 25;

HWND hWnd;
HWND hGrid;
std::vector<std::vector<HWND>> cells;
int current_row = 0;
int current_col = 0;

std::string get_current_time() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

void save_data_xlsx(const char* filename) {
    zip_t* zip = zip_open(filename, ZIP_CREATE | ZIP_TRUNCATE, nullptr);
    if (!zip) {
        MessageBox(hWnd, "Failed to create XLSX file", "Error", MB_ICONERROR);
        return;
    }

    // Create [Content_Types].xml
    std::string content_types = 
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
        "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
        "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
        "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
        "<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
        "</Types>";
    zip_source_t* s = zip_source_buffer(zip, content_types.c_str(), content_types.size(), 0);
    zip_file_add(zip, "[Content_Types].xml", s, ZIP_FL_OVERWRITE);

    // Create _rels/.rels
    std::string rels = 
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
        "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/>"
        "</Relationships>";
    s = zip_source_buffer(zip, rels.c_str(), rels.size(), 0);
    zip_file_add(zip, "_rels/.rels", s, ZIP_FL_OVERWRITE);

    // Create xl/workbook.xml
    std::string workbook = 
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
        "<sheets><sheet name=\"Sheet1\" sheetId=\"1\" r:id=\"rId1\"/></sheets>"
        "</workbook>";
    s = zip_source_buffer(zip, workbook.c_str(), workbook.size(), 0);
    zip_file_add(zip, "xl/workbook.xml", s, ZIP_FL_OVERWRITE);

    // Create xl/_rels/workbook.xml.rels
    std::string workbook_rels = 
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
        "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet1.xml\"/>"
        "</Relationships>";
    s = zip_source_buffer(zip, workbook_rels.c_str(), workbook_rels.size(), 0);
    zip_file_add(zip, "xl/_rels/workbook.xml.rels", s, ZIP_FL_OVERWRITE);

    // Create xl/worksheets/sheet1.xml
    std::ostringstream sheet;
    sheet << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
          << "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
          << "<sheetData>";

    for (size_t i = 0; i < cells.size(); ++i) {
        bool row_has_data = false;
        std::ostringstream row;
        row << "<row r=\"" << (i + 1) << "\">";
        
        for (size_t j = 0; j < cells[i].size(); ++j) {
            char buffer[256];
            GetWindowTextA(cells[i][j], buffer, 256);
            if (buffer[0] != '\0') {
                row_has_data = true;
                char col_name = 'A' + j;
                row << "<c r=\"" << col_name << (i + 1) << "\" t=\"inlineStr\">"
                    << "<is><t>" << buffer << "</t></is></c>";
            }
        }
        
        row << "</row>";
        if (row_has_data) {
            sheet << row.str();
        }
    }

    sheet << "</sheetData></worksheet>";
    std::string sheet_content = sheet.str();
    s = zip_source_buffer(zip, sheet_content.c_str(), sheet_content.size(), 0);
    zip_file_add(zip, "xl/worksheets/sheet1.xml", s, ZIP_FL_OVERWRITE);

    zip_close(zip);
}

void save_data() {
    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = "Excel Files (*.xlsx)\0*.xlsx\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "xlsx";

    if (GetSaveFileName(&ofn)) {
        save_data_xlsx(szFileName);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_COMMAND:
            if (LOWORD(wParam) == 1) {  // Assuming 1 is the ID for the "Save" button
                save_data();
            }
            return 0;
        case WM_KEYDOWN:
            switch (wParam) {
                case VK_TAB:
                    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                        // Shift+Tab pressed
                        if (current_col > 0) {
                            current_col--;
                        } else if (current_row > 0) {
                            current_row--;
                            current_col = INITIAL_COLS - 1;
                        }
                    } else {
                        // Tab pressed
                        if (current_col < INITIAL_COLS - 1) {
                            current_col++;
                        } else if (current_row < INITIAL_ROWS - 1) {
                            current_row++;
                            current_col = 0;
                        }
                    }
                    SetFocus(cells[current_row][current_col]);
                    return 0;
                case VK_RETURN:
                    if (current_row < INITIAL_ROWS - 1) {
                        current_row++;
                        SetFocus(cells[current_row][current_col]);
                    }
                    return 0;
            }
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "SpreadsheetClass";
    RegisterClassEx(&wc);

    RECT wr = {0, 0, CELL_WIDTH * INITIAL_COLS, CELL_HEIGHT * INITIAL_ROWS + 50};  // +50 for the save button
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    hWnd = CreateWindowEx(
        0,
        "SpreadsheetClass",
        "Simple Spreadsheet",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left, wr.bottom - wr.top,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hWnd == NULL) {
        return 0;
    }

    HWND hSaveButton = CreateWindow(
        "BUTTON",
        "Save",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10,
        CELL_HEIGHT * INITIAL_ROWS + 10,
        100,
        30,
        hWnd,
        (HMENU)1,  // ID for the button
        hInstance,
        NULL
    );

    cells.resize(INITIAL_ROWS, std::vector<HWND>(INITIAL_COLS));

    for (int i = 0; i < INITIAL_ROWS; ++i) {
        for (int j = 0; j < INITIAL_COLS; ++j) {
            cells[i][j] = CreateWindowEx(
                WS_EX_CLIENTEDGE,
                "EDIT",
                "",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                j * CELL_WIDTH,
                i * CELL_HEIGHT,
                CELL_WIDTH,
                CELL_HEIGHT,
                hWnd,
                NULL,
                hInstance,
                NULL
            );
        }
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    SetFocus(cells[0][0]);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
