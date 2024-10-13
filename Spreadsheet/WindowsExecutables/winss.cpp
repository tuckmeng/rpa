#include <windows.h>
#include <commdlg.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <filesystem> // Include the filesystem header

#define INITIAL_ROWS 100
#define INITIAL_COLS 26

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SaveExcelFile(const std::string& filename, const std::vector<std::vector<std::string>>& data);
void CreateExcelXMLFiles(const std::string& filename, const std::vector<std::vector<std::string>>& data);
void CreateWorkbookXML(std::ofstream& ofs);
void CreateWorksheetXML(std::ofstream& ofs, const std::vector<std::vector<std::string>>& data);
void CreateStylesXML(std::ofstream& ofs);
void CreateContentTypesXML(std::ofstream& ofs);
void CreateRelsXML(std::ofstream& ofs);
void CreateWorkbookRelsXML(std::ofstream& ofs);
void CreateDocPropsFiles();
void CreateSharedStringsXML(std::ofstream& ofs, const std::vector<std::vector<std::string>>& data);
void CreateTheme1XML(std::ofstream& ofs);
void ZipFiles(const std::string& zipFilename);
void InitGrid(HWND hwnd);
void ClearGridData(HWND hwnd);
void ClearExistingFiles();

std::vector<std::vector<std::string>> cellData(INITIAL_ROWS, std::vector<std::string>(INITIAL_COLS, ""));
HWND grid[INITIAL_ROWS][INITIAL_COLS];

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nShowCmd) {
    const char CLASS_NAME[] = "SimpleSpreadsheetClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);
    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Simple Spreadsheet", WS_OVERLAPPEDWINDOW, 
                               CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);
    
    ShowWindow(hwnd, nShowCmd);
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HMENU hMenu;
    
    switch (uMsg) {
    case WM_CREATE:
        {
            HMENU hFileMenu = CreatePopupMenu();
            AppendMenu(hFileMenu, MF_STRING, 1, "New");
            AppendMenu(hFileMenu, MF_STRING, 2, "Save As");

            hMenu = CreateMenu();
            AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hFileMenu, "File");
            SetMenu(hwnd, hMenu);

            InitGrid(hwnd);
        }
        break;

    case WM_COMMAND:
        {
            switch (LOWORD(wParam)) {
            case 1: // New
                ClearGridData(hwnd);
                break;
            case 2: // Save As
                {
                    OPENFILENAME ofn; 
                    char szFile[260]; 
                    ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = szFile;
                    ofn.lpstrFile[0] = '\0';
                    ofn.nMaxFile = sizeof(szFile);
                    ofn.lpstrFilter = "Excel Files (*.xlsx)\0*.xlsx\0All Files (*.*)\0*.*\0";
                    ofn.lpstrTitle = "Save As";
                    ofn.Flags = OFN_OVERWRITEPROMPT;

                    if (GetSaveFileName(&ofn)) {
                        SaveExcelFile(ofn.lpstrFile, cellData);
                    }
                }
                break;
            }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void InitGrid(HWND hwnd) {
    for (int i = 0; i < INITIAL_ROWS; ++i) {
        for (int j = 0; j < INITIAL_COLS; ++j) {
            grid[i][j] = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
                                         j * 80, i * 30, 80, 30, hwnd, NULL, NULL, NULL);
        }
    }
}

void SaveExcelFile(const std::string& filename, const std::vector<std::vector<std::string>>& data) {
    // Clear existing files and directories before creating new ones
    ClearExistingFiles();
    
    // Create necessary directories
    std::filesystem::create_directories("xl/worksheets");
    std::filesystem::create_directories("_rels");
    std::filesystem::create_directories("docProps");

    // Generate XML files
    CreateExcelXMLFiles("xl/workbook.xml", data);
    CreateExcelXMLFiles("xl/styles.xml", data);
    CreateExcelXMLFiles("xl/worksheets/sheet1.xml", data);
    CreateExcelXMLFiles("xl/workbook.xml.rels", data);
    CreateExcelXMLFiles("_rels/.rels", data);
    CreateExcelXMLFiles("[Content_Types].xml", data);
    CreateSharedStringsXML("xl/sharedStrings.xml", data);
    CreateTheme1XML("xl/theme/theme1.xml");
    CreateDocPropsFiles();
    
    ZipFiles(filename);
}

void ClearExistingFiles() {
    // Delete the existing files and directories if they exist
    std::filesystem::remove_all("docProps");
    std::filesystem::remove_all("_rels");
    std::filesystem::remove_all("xl");
    std::filesystem::remove("[Content_Types].xml");
}

void CreateExcelXMLFiles(const std::string& filename, const std::vector<std::vector<std::string>>& data) {
    std::ofstream ofs(filename);
    if (filename == "xl/workbook.xml") {
        CreateWorkbookXML(ofs);
    } else if (filename == "xl/styles.xml") {
        CreateStylesXML(ofs);
    } else if (filename == "xl/worksheets/sheet1.xml") {
        CreateWorksheetXML(ofs, data);
    } else if (filename == "xl/sharedStrings.xml") {
        CreateSharedStringsXML(ofs, data);
    } else if (filename == "xl/workbook.xml.rels") {
        CreateWorkbookRelsXML(ofs);
    } else if (filename == "[Content_Types].xml") {
        CreateContentTypesXML(ofs);
    } else if (filename == "_rels/.rels") {
        CreateRelsXML(ofs);
    } else if (filename == "xl/theme/theme1.xml") {
        CreateTheme1XML(ofs);
    }
    ofs.close(); // Close the file before zipping
}

void CreateDocPropsFiles() {
    std::ofstream coreFile("docProps/core.xml");
    coreFile << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
                 R"(<dc:dc xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties">)"
                 R"(<dc:title>Simple Spreadsheet</dc:title>)"
                 R"(<dc:creator>Your Name</dc:creator>)"
                 R"(<dc:description>A simple spreadsheet created with C++</dc:description>)"
                 R"(<cp:lastModifiedBy>Your Name</cp:lastModifiedBy>)"
                 R"(<dcterms:created xsi:type="dcterms:W3CDTF">2024-10-13T00:00:00Z</dcterms:created>)"
                 R"(<dcterms:modified xsi:type="dcterms:W3CDTF">2024-10-13T00:00:00Z</dcterms:modified>)"
                 R"(</dc:dc>)";
    coreFile.close(); // Close core.xml

    std::ofstream appFile("docProps/app.xml");
    appFile << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
                R"(<Properties xmlns="http://schemas.microsoft.com/office/officeApp" xmlns:vt="http://schemas.microsoft.com/office/officeApp">)"
                R"(<Application>Microsoft Excel</Application>)"
                R"(<DocSecurity>0</DocSecurity>)"
                R"(<ScaleCrop>false</ScaleCrop>)"
                R"(<HeadingPairs>)"
                R"(<vt:vector size="2" baseType="variant">)"
                R"(<vt:variant><vt:lpstr>Worksheets</vt:lpstr></vt:variant>)"
                R"(<vt:variant><vt:i4>1</vt:i4></vt:variant>)"
                R"(</vt:vector>)"
                R"(</HeadingPairs>)"
                R"(<TitlesOfParts>)"
                R"(<vt:vector size="1" baseType="lpstr">)"
                R"(<vt:lpstr>Sheet1</vt:lpstr>)"
                R"(</vt:vector>)"
                R"(</TitlesOfParts>)"
                R"(</Properties>)";
    appFile.close(); // Close app.xml
}

void CreateSharedStringsXML(std::ofstream& ofs, const std::vector<std::vector<std::string>>& data) {
    ofs << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<sst xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" count=")" << data.size() * data[0].size() << R"(" uniqueCount=")" << data.size() * data[0].size() << R"(">";

    for (const auto& row : data) {
        for (const auto& cell : row) {
            ofs << "<si><t>" << cell << "</t></si>";
        }
    }

    ofs << R"(</sst>)";
}

void CreateTheme1XML(std::ofstream& ofs) {
    ofs << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<a:theme xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" name="Office Theme">)"
        R"(</a:theme>)";
}

void CreateWorkbookXML(std::ofstream& ofs) {
    ofs << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">)"
        R"(<sheets>)"
        R"(<sheet name="Sheet1" sheetId="1" r:id="rId1"/>)"
        R"(<sheet name="Shared Strings" sheetId="2" r:id="rId2"/>)"
        R"(</sheets>)"
        R"(</workbook>)";
}

void CreateWorksheetXML(std::ofstream& ofs, const std::vector<std::vector<std::string>>& data) {
    ofs << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">)"
        R"(<dimension ref="A1:Z100"/>)"
        R"(<sheetData>)";

    for (size_t i = 0; i < data.size(); ++i) {
        ofs << "<row r=\"" << (i + 1) << "\">";
        for (size_t j = 0; j < data[i].size(); ++j) {
            ofs << "<c r=\"" << static_cast<char>('A' + j) << (i + 1) << "\" t=\"inlineStr\">"
                << "<is><t>" << data[i][j] << "</t></is></c>";
        }
        ofs << "</row>";
    }

    ofs << R"(</sheetData>)"
        R"(</worksheet>)";
}

void CreateStylesXML(std::ofstream& ofs) {
    ofs << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">)"
        R"(</styleSheet>)";
}

void CreateContentTypesXML(std::ofstream& ofs) {
    ofs << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">)"
        R"(<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main"/>)"
        R"(<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.worksheet+xml"/>)"
        R"(<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>)"
        R"(<Override PartName="/xl/sharedStrings.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"/>)"
        R"(<Override PartName="/docProps/core.xml" ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>)"
        R"(<Override PartName="/docProps/app.xml" ContentType="application/vnd.openxmlformats-officedocument.extended-properties+xml"/>)"
        R"(<Override PartName="/xl/theme/theme1.xml" ContentType="application/vnd.openxmlformats-officedocument.theme+xml"/>)"
        R"(</Types>)";
}

void CreateRelsXML(std::ofstream& ofs) {
    ofs << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">)"
        R"(<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>)"
        R"(</Relationships>)";
}

void CreateWorkbookRelsXML(std::ofstream& ofs) {
    ofs << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">)"
        R"(<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>)"
        R"(<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings" Target="sharedStrings.xml"/>)"
        R"(</Relationships>)";
}

void ZipFiles(const std::string& zipFilename) {
    std::string command = "7z a -tzip \"" + zipFilename + "\" \"[Content_Types].xml\" \"_rels\\.rels\" \"xl\\*\" \"docProps\\*\"";
    int result = system(command.c_str());

    if (result != 0) {
        MessageBox(NULL, "Error creating zip file!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    std::string xlsxFilename = zipFilename;
    if (xlsxFilename.substr(xlsxFilename.length() - 5) != ".xlsx") {
        xlsxFilename += ".xlsx";
    }

    std::string renameCommand = "ren \"" + zipFilename + "\" \"" + xlsxFilename + "\"";
    result = system(renameCommand.c_str());

    if (result != 0) {
        MessageBox(NULL, "Error renaming zip file!", "Error", MB_OK | MB_ICONERROR);
    }
}

void ClearGridData(HWND hwnd) {
    for (int i = 0; i < INITIAL_ROWS; ++i) {
        for (int j = 0; j < INITIAL_COLS; ++j) {
            SetWindowText(grid[i][j], "");
            cellData[i][j] = "";
        }
    }
}
