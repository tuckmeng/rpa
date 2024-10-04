#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // Include this header for access function
#include <xlsxwriter.h>
#include <ctype.h>   // For isspace

// Function to display help message
void show_help() {
    printf("Usage: csv_to_xlsx <input.csv> <output.xlsx>\n");
    printf("Converts a CSV file to an XLSX file.\n");
    printf("\nParameters:\n");
    printf("  input.csv   The path to the input CSV file.\n");
    printf("  output.xlsx The path to the output XLSX file.\n");
}

// Function to trim whitespace from a string
char *trim_whitespace(char *str) {
    while (isspace(*str)) str++;  // Trim leading whitespace
    if (*str == 0) return str;     // All spaces?
    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--; // Trim trailing whitespace
    *(end + 1) = 0;  // Null terminate
    return str;
}

// Function to remove quotes from a string
void remove_quotes(char *str) {
    char *src = str, *dst = str;
    while (*src) {
        if (*src != '"') {
            *dst++ = *src;  // Copy non-quote characters
        }
        src++;
    }
    *dst = '\0';  // Null terminate the result
}

// Function to read CSV and write to XLSX
void csv_to_xlsx(const char *input_file, const char *output_file) {
    FILE *csv_file = fopen(input_file, "r");
    if (!csv_file) {
        perror("Error opening input CSV file");
        exit(EXIT_FAILURE);
    }

    lxw_workbook  *workbook  = workbook_new(output_file);
    lxw_worksheet *worksheet = workbook_add_worksheet(workbook, NULL);

    char line[1024];
    int row = 0;

    // Read CSV file line by line
    while (fgets(line, sizeof(line), csv_file)) {
        char *token;
        int col = 0;
        char *start = line;
        int in_quotes = 0;

        for (char *p = line; *p; p++) {
            if (*p == '"') {
                in_quotes = !in_quotes;  // Toggle the in_quotes state
            }
            // If we reach a comma and we are not inside quotes, we have a token
            if (*p == ',' && !in_quotes) {
                *p = '\0';  // Replace comma with null terminator
                remove_quotes(start);  // Remove quotes from the token
                worksheet_write_string(worksheet, row, col++, trim_whitespace(start), NULL);
                start = p + 1;  // Move start to the next character
            }
        }
        // Write the last token
        remove_quotes(start);  // Remove quotes from the last token
        worksheet_write_string(worksheet, row, col++, trim_whitespace(start), NULL);
        row++;
    }

    fclose(csv_file);
    workbook_close(workbook);
}

int main(int argc, char *argv[]) {
    // Check the number of arguments
    if (argc != 3) {
        fprintf(stderr, "Error: Invalid number of arguments.\n");
        show_help();
        return EXIT_FAILURE;
    }

    const char *input_file = argv[1];
    const char *output_file = argv[2];

    // Check for the existence of input file
    if (access(input_file, F_OK) == -1) {
        perror("Error: Input file does not exist");
        return EXIT_FAILURE;
    }

    // Call function to convert CSV to XLSX
    csv_to_xlsx(input_file, output_file);

    printf("Conversion complete: %s -> %s\n", input_file, output_file);
    return EXIT_SUCCESS;
}
