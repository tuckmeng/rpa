#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_ROWS 100
#define INITIAL_COLS 26

HWND grid[INITIAL_ROWS][INITIAL_COLS];
char cellData[INITIAL_ROWS][INITIAL_COLS][256];
char basePath[MAX_PATH];

void CreateContentTypesXML(FILE* ofs);
void CreateWorkbookXML(FILE* ofs);
void CreateWorksheetXML(FILE* ofs, const char data[INITIAL_ROWS][INITIAL_COLS][256]);
void CreateStylesXML(FILE* ofs);
void CreateSharedStringsXML(FILE* ofs, const char data[INITIAL_ROWS][INITIAL_COLS][256]);
void CreateTheme1XML(FILE* ofs);
void CreateRelsXML(FILE* ofs);
void CreateWorkbookRelsXML(FILE* ofs);
void ZipFiles(const char* zipFilename);
void ClearGridData(HWND hwnd);
void CreateDirectoryIfNotExists(const char* path);
void GetAbsolutePath(const char* relativePath, char* absolutePath);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    // Get the absolute path of the executable
    GetModuleFileNameA(NULL, basePath, sizeof(basePath));
    // Remove the executable name to get the directory path
    char* lastSlash = strrchr(basePath, '\\');
    if (lastSlash) *lastSlash = '\0';

    // Initialize grid and other variables
    memset(cellData, 0, sizeof(cellData));  // Ensure cellData is initialized to zero

    // Create necessary directories
    char xlPath[MAX_PATH];
    char relsPath[MAX_PATH];
    char docPropsPath[MAX_PATH];
    char themePath[MAX_PATH];

    sprintf(xlPath, "%s\\xl", basePath);
    sprintf(relsPath, "%s\\xl\\_rels", basePath);
    sprintf(docPropsPath, "%s\\docProps", basePath);
    sprintf(themePath, "%s\\xl\\theme", basePath);

    CreateDirectoryIfNotExists(xlPath);
    CreateDirectoryIfNotExists(relsPath);
    CreateDirectoryIfNotExists(docPropsPath);
    CreateDirectoryIfNotExists(themePath);
    CreateDirectoryIfNotExists("%s\\xl\\worksheets");

    // Clean up and create the XLSX file
    FILE* ofs;

    // Create necessary files with error checking
    char contentTypesPath[MAX_PATH];
    sprintf(contentTypesPath, "%s\\[Content_Types].xml", basePath);
    ofs = fopen(contentTypesPath, "w");
    if (!ofs) {
        MessageBox(NULL, "Failed to create [Content_Types].xml", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    CreateContentTypesXML(ofs);
    fclose(ofs);

    char relsPathFile[MAX_PATH];
    sprintf(relsPathFile, "%s\\_rels\\.rels", basePath);
	puts("Debug");
	puts(relsPathFile);
    ofs = fopen(relsPathFile, "w");
    if (!ofs) {
        MessageBox(NULL, "Failed to create _rels\\.rels", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    CreateRelsXML(ofs);
    fclose(ofs);

    char workbookPath[MAX_PATH];
    sprintf(workbookPath, "%s\\xl\\workbook.xml", basePath);
    ofs = fopen(workbookPath, "w");
    if (!ofs) {
        MessageBox(NULL, "Failed to create xl\\workbook.xml", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    CreateWorkbookXML(ofs);
    fclose(ofs);

    char worksheetPath[MAX_PATH];
    sprintf(worksheetPath, "%s\\xl\\worksheets\\sheet1.xml", basePath);
    ofs = fopen(worksheetPath, "w");
    if (!ofs) {
        MessageBox(NULL, "Failed to create xl\\worksheets\\sheet1.xml", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    CreateWorksheetXML(ofs, cellData);
    fclose(ofs);

    char sharedStringsPath[MAX_PATH];
    sprintf(sharedStringsPath, "%s\\xl\\sharedStrings.xml", basePath);
    ofs = fopen(sharedStringsPath, "w");
    if (!ofs) {
        MessageBox(NULL, "Failed to create xl\\sharedStrings.xml", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    CreateSharedStringsXML(ofs, cellData);
    fclose(ofs);

    char stylesPath[MAX_PATH];
    sprintf(stylesPath, "%s\\xl\\styles.xml", basePath);
    ofs = fopen(stylesPath, "w");
    if (!ofs) {
        MessageBox(NULL, "Failed to create xl\\styles.xml", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    CreateStylesXML(ofs);
    fclose(ofs);

    char corePath[MAX_PATH];
    sprintf(corePath, "%s\\docProps\\core.xml", basePath);
    ofs = fopen(corePath, "w");
    if (!ofs) {
        MessageBox(NULL, "Failed to create docProps\\core.xml", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    // Create core.xml content here
    fclose(ofs);

    char appPath[MAX_PATH];
    sprintf(appPath, "%s\\docProps\\app.xml", basePath);
    ofs = fopen(appPath, "w");
    if (!ofs) {
        MessageBox(NULL, "Failed to create docProps\\app.xml", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    // Create app.xml content here
    fclose(ofs);

    char theme1Path[MAX_PATH];
    sprintf(theme1Path, "%s\\xl\\theme\\theme1.xml", basePath);
    ofs = fopen(theme1Path, "w");
    if (!ofs) {
        MessageBox(NULL, "Failed to create xl\\theme\\theme1.xml", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    CreateTheme1XML(ofs);
    fclose(ofs);

    // Create workbook.rels file
    char workbookRelsPath[MAX_PATH];
    sprintf(workbookRelsPath, "%s\\xl\\_rels\\workbook.xml.rels", basePath);
    ofs = fopen(workbookRelsPath, "w");
    if (!ofs) {
        MessageBox(NULL, "Failed to create xl\\_rels\\workbook.xml.rels", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    CreateWorkbookRelsXML(ofs);
    fclose(ofs);

    // Zip the files into the final XLSX format
    ZipFiles("testwinss");

    return 0;
}

void CreateContentTypesXML(FILE* ofs) {
    fprintf(ofs, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                  "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
                  "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main\"/>"
                  "<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.worksheet+xml\"/>"
                  "<Override PartName=\"/xl/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>"
                  "<Override PartName=\"/xl/sharedStrings.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml\"/>"
                  "<Override PartName=\"/docProps/core.xml\" ContentType=\"application/vnd.openxmlformats-package.core-properties+xml\"/>"
                  "<Override PartName=\"/docProps/app.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.extended-properties+xml\"/>"
                  "<Override PartName=\"/xl/theme/theme1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.theme+xml\"/>"
                  "</Types>");
}

void CreateWorkbookXML(FILE* ofs) {
    fprintf(ofs, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                  "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
                  "<sheets>"
                  "<sheet name=\"Sheet1\" sheetId=\"1\" r:id=\"rId1\"/>"
                  "<sheet name=\"Shared Strings\" sheetId=\"2\" r:id=\"rId2\"/>"
                  "</sheets>"
                  "</workbook>");
}

void CreateWorksheetXML(FILE* ofs, const char data[INITIAL_ROWS][INITIAL_COLS][256]) {
    fprintf(ofs, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                  "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
                  "<dimension ref=\"A1:Z100\"/>"
                  "<sheetData>");

    for (int i = 0; i < INITIAL_ROWS; ++i) {
        fprintf(ofs, "<row r=\"%d\">", i + 1);
        for (int j = 0; j < INITIAL_COLS; ++j) {
            fprintf(ofs, "<c r=\"%c%d\" t=\"inlineStr\"><is><t>%s</t></is></c>",
                    'A' + j, i + 1, data[i][j]);
        }
        fprintf(ofs, "</row>");
    }

    fprintf(ofs, "</sheetData></worksheet>");
}

void CreateStylesXML(FILE* ofs) {
    fprintf(ofs, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                  "<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
                  "<fonts><font><sz val=\"11\"/><color rgb=\"FF000000\"/><name val=\"Arial\"/><family val=\"2\"/><scheme val=\"minor\"/></font></fonts>"
                  "<fills><fill><patternFill patternType=\"none\"/></fill></fills>"
                  "<borders><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                  "<cellStyleXfs><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
                  "<cellXfs><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellXfs>"
                  "</styleSheet>");
}

void CreateSharedStringsXML(FILE* ofs, const char data[INITIAL_ROWS][INITIAL_COLS][256]) {
    fprintf(ofs, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                  "<sst xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" count=\"%d\" uniqueCount=\"%d\">",
            INITIAL_ROWS * INITIAL_COLS, INITIAL_ROWS * INITIAL_COLS);

    for (int i = 0; i < INITIAL_ROWS; ++i) {
        for (int j = 0; j < INITIAL_COLS; ++j) {
            if (strlen(data[i][j]) > 0) {
                fprintf(ofs, "<si><t>%s</t></si>", data[i][j]);
            }
        }
    }
    fprintf(ofs, "</sst>");
}

void CreateTheme1XML(FILE* ofs) {
    fprintf(ofs, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                  "<a:theme xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" name=\"Office Theme\">"
                  "<a:themeElements>"
                  "<a:clrScheme name=\"Office Colors\">"
                  "<a:dk1><a:sysClr val=\"windowText\" lastClr=\"000000\"/></a:dk1>"
                  "<a:lt1><a:sysClr val=\"window\" lastClr=\"FFFFFF\"/></a:lt1>"
                  "<a:dk2><a:srgbClr val=\"A5A5A5\"/></a:dk2>"
                  "<a:lt2><a:srgbClr val=\"FFFFFF\"/></a:lt2>"
                  "<a:accent1><a:srgbClr val=\"5B9BD5\"/></a:accent1>"
                  "<a:accent2><a:srgbClr val=\"ED7D31\"/></a:accent2>"
                  "<a:accent3><a:srgbClr val=\"A5A5A5\"/></a:accent3>"
                  "<a:accent4><a:srgbClr val=\"F6B8A1\"/></a:accent4>"
                  "<a:accent5><a:srgbClr val=\"3D85C6\"/></a:accent5>"
                  "<a:accent6><a:srgbClr val=\"F6B8A1\"/></a:accent6>"
                  "</a:clrScheme>"
                  "</a:themeElements>"
                  "<a:fontScheme name=\"Office Font\">"
                  "<a:majorFont>"
                  "<a:latin typeface=\"Arial\"/>"
                  "<a:ea typeface=\"\"/>"
                  "<a:cs typeface=\"\"/>"
                  "</a:majorFont>"
                  "<a:minorFont>"
                  "<a:latin typeface=\"Arial\"/>"
                  "<a:ea typeface=\"\"/>"
                  "<a:cs typeface=\"\"/>"
                  "</a:minorFont>"
                  "</a:fontScheme>"
                  "<a:formatScheme name=\"Office Format\">"
                  "<a:fill>"
                  "<a:solidFill>"
                  "<a:srgbClr val=\"FFFFFF\"/>"
                  "</a:solidFill>"
                  "</a:fill>"
                  "<a:line>"
                  "<a:noFill/>"
                  "</a:line>"
                  "<a:effect>"
                  "<a:noFill/>"
                  "</a:effect>"
                  "</a:formatScheme>"
                  "</a:theme>");
}

void CreateRelsXML(FILE* ofs) {
    fprintf(ofs, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                  "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                  "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet1.xml\"/>"
                  "</Relationships>");
}

void CreateWorkbookRelsXML(FILE* ofs) {
    fprintf(ofs, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                  "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                  "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet1.xml\"/>"
                  "<Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings\" Target=\"sharedStrings.xml\"/>"
                  "</Relationships>");
}

void ZipFiles(const char* zipFilename) {
    char command[1024];
    sprintf(command, "7z a -tzip \"%s.xlsx\" \"[Content_Types].xml\" \"_rels\\.rels\" \"xl\\*\" \"docProps\\*\"", zipFilename);
    int result = system(command);

    if (result != 0) {
        MessageBox(NULL, "Error creating zip file!", "Error", MB_OK | MB_ICONERROR);
        return;
    }
}

void ClearGridData(HWND hwnd) {
    for (int i = 0; i < INITIAL_ROWS; ++i) {
        for (int j = 0; j < INITIAL_COLS; ++j) {
            SetWindowText(grid[i][j], "");
            strcpy(cellData[i][j], "");
        }
    }
}

void CreateDirectoryIfNotExists(const char* path) {
    // This function creates a directory if it does not exist
    DWORD ftyp = GetFileAttributesA(path);
    if (ftyp == INVALID_FILE_ATTRIBUTES) {
        // Create the directory
        CreateDirectoryA(path, NULL);
    }
}
