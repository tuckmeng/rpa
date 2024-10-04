#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xlsxwriter.h"

void print_help(const char *prog_name) {
    printf("Usage: %s <input_txt_file> <output_excel_file>\n", prog_name);
    printf("Converts a text file to an Excel file.\n");
    printf("  <input_txt_file>   Path to the input text file.\n");
    printf("  <output_excel_file> Path to the output Excel file (e.g., output.xlsx).\n");
}

int main(int argc, char *argv[]) {
    // Check command line arguments
    if (argc != 3) {
        print_help(argv[0]);
        return 1;
    }

    const char* txtFilePath = argv[1];   // Path to your input text file from command line
    const char* excelFilePath = argv[2]; // Output path for Excel file from command line

    // Check if the text file exists
    FILE* txtFile = fopen(txtFilePath, "r");
    if (txtFile == NULL) {
        printf("Error: Text file does not exist: %s\n", txtFilePath);
        return 1;
    }

    // Create a new Excel workbook
    lxw_workbook  *workbook  = workbook_new(excelFilePath);
    lxw_worksheet *worksheet = workbook_add_worksheet(workbook, NULL);

    // Read and write data to Excel
    char line[256];
    int row = 0; // Start writing from the first row
    while (fgets(line, sizeof(line), txtFile)) {
        // Remove the newline character from the line
        line[strcspn(line, "\n")] = 0;
        worksheet_write_string(worksheet, row, 0, line, NULL); // Write each line to column A
        row++;
    }

    // Close the workbook
    workbook_close(workbook);
    fclose(txtFile);

    printf("Data has been written to Excel successfully at: %s\n", excelFilePath);

    return 0;
}
