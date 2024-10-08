#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <filesystem>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#endif

void createXLSX(const std::string& data, const std::string& filename) {
    // Get the current executable directory
    char path[1024];
#ifdef _WIN32
    GetModuleFileNameA(NULL, path, sizeof(path));
    std::string exeDir = std::filesystem::path(path).parent_path().string();
#else
    readlink("/proc/self/exe", path, sizeof(path));
    std::string exeDir = std::filesystem::path(path).parent_path().string();
#endif
    std::string tempDir = exeDir + "/xlsx_temp";

    // Create temporary directory
#ifdef _WIN32
    std::filesystem::create_directories(tempDir + "/xl/worksheets");
    std::filesystem::create_directories(tempDir + "/xl/_rels");
#else
    mkdir(tempDir.c_str(), 0755);
    mkdir((tempDir + "/xl").c_str(), 0755);
    mkdir((tempDir + "/xl/worksheets").c_str(), 0755);
    mkdir((tempDir + "/xl/_rels").c_str(), 0755);
#endif

    // Generate content types file
    std::ofstream contentTypes(tempDir + "/[Content_Types].xml");
    contentTypes << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
                 << "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
                 << "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
                 << "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
                 << "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
                 << "<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
                 << "</Types>";
    contentTypes.close();

    // Generate _rels/.rels file
    std::ofstream rels(tempDir + "/_rels/.rels");
    rels << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
         << "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
         << "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/>"
         << "</Relationships>";
    rels.close();

    // Generate xl/workbook.xml file
    std::ofstream workbook(tempDir + "/xl/workbook.xml");
    workbook << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
             << "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
             << "<sheets>"
             << "<sheet name=\"Sheet1\" sheetId=\"1\" rId=\"rId1\"/>"
             << "</sheets>"
             << "</workbook>";
    workbook.close();

    // Generate xl/_rels/workbook.xml.rels file
    std::ofstream workbookRels(tempDir + "/xl/_rels/workbook.xml.rels");
    workbookRels << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
                  << "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                  << "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet1.xml\"/>"
                  << "</Relationships>";
    workbookRels.close();

    // Generate xl/worksheets/sheet1.xml file
    std::ofstream sheet(tempDir + "/xl/worksheets/sheet1.xml");
    sheet << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
          << "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
          << "<sheetData>";

    std::stringstream ss(data);
    std::string row;
    int rowIndex = 1;

    while (std::getline(ss, row, '~')) {
        sheet << "<row r=\"" << rowIndex++ << "\">";
        std::stringstream cellStream(row);
        std::string cell;
        int colIndex = 1;

        while (std::getline(cellStream, cell, ',')) {
            sheet << "<c r=\"" << (char)('A' + colIndex - 1) << (rowIndex - 1) << "\" t=\"inlineStr\"><is><t>" << cell << "</t></is></c>";
            colIndex++;
        }
        sheet << "</row>";
    }

    sheet << "</sheetData></worksheet>";
    sheet.close();

    // Create the XLSX file using 7z
    std::string cmd;
#ifdef _WIN32
    cmd = "7z.exe a -tzip \"" + filename + "\" \"" + tempDir + "\\*\"";
#else
    cmd = "7z a -tzip \"" + filename + "\" \"" + tempDir + "/*\"";
#endif
    system(cmd.c_str());

    // Clean up temporary directory
    std::filesystem::remove_all(tempDir);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <data> <filename>\n";
        return 1;
    }

    std::string data = argv[1];
    std::string filename = argv[2];

    createXLSX(data, filename);

    std::cout << "Spreadsheet created: " << filename << std::endl;
    return 0;
}
