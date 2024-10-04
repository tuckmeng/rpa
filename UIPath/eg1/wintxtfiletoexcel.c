#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libxl.h"  // Make sure to include the libxl header

int main() {
    const char* txtFilePath = "C:\\path\\to\\your\\file.txt";  // Path to your input text file
    const char* excelFilePath = "C:\\path\\to\\your\\txtfiletoexcel.xlsx";  // Output path for Excel file

    // Check if the text file exists
    FILE* txtFile = fopen(txtFilePath, "r");
    if (txtFile == NULL) {
        printf("Error: Text file does not exist: %s\n", txtFilePath);
        return 1;
    }

    // Create a new Excel book
    BookHandle book = xlCreateBook();
    if (!book) {
        printf("Error: Could not create an Excel book.\n");
        fclose(txtFile);
        return 1;
    }

    // Add a new sheet
    SheetHandle sheet = xlBookAddSheet(book, "Sheet1", NULL);
    if (!sheet) {
        printf("Error: Could not add a new sheet.\n");
        xlBookRelease(book);
        fclose(txtFile);
        return 1;
    }

    // Read and write data to Excel
    char line[256];
    int row = 1; // Start writing from the first row
    while (fgets(line, sizeof(line), txtFile)) {
        // Remove the newline character from the line
        line[strcspn(line, "\n")] = 0;
        xlSheetWriteStr(sheet, row, 0, line, 0); // Write each line to column A
        row++;
    }

    // Save the workbook
    if (xlBookSave(book, excelFilePath) == 0) {
        printf("Error: Could not save the Excel file: %s\n", xlGetError());
    } else {
        printf("Data has been written to Excel successfully at: %s\n", excelFilePath);
    }

    // Cleanup
    xlBookRelease(book);
    fclose(txtFile);

    return 0;
}
