#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 65536  // 64 KB buffer

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        printf("Usage: %s <filepath> [mode]\n", argv[0]);
        printf("Modes:\n");
        printf("  + or a    Append to the file if it exists (default mode is overwrite with prompt)\n");
        printf("\nIf the file exists and append mode is not used, you will be prompted before overwriting.\n");
        printf("End input with Ctrl+D (Linux/macOS) or Ctrl+Z then Enter (Windows).\n");
        printf("If the EOF characters do not end input, end the input using Ctrl+C.\n");
        return 1;
    }

    const char *filepath = argv[1];
    const char *mode = "wb"; // default: write (may prompt)

    // Check for append mode (second argument is '+' or 'a'/'A')
    if (argc == 3) {
        char m = tolower(argv[2][0]);
        if (m == '+' || m == 'a') {
            mode = "ab"; // append
        }
    } else if (argc == 2) {
        // Prompt if file exists
        FILE *f = fopen(filepath, "rb");
        if (f) {
            fclose(f);
            char response[10];
            printf("File '%s' already exists. Overwrite? (y/N): ", filepath);
            if (!fgets(response, sizeof(response), stdin)) {
                fprintf(stderr, "Error reading input.\n");
                return 1;
            }
            if (response[0] != 'y' && response[0] != 'Y') {
                printf("Aborted.\n");
                return 0;
            }
        }
    }

    FILE *file = fopen(filepath, mode);
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    unsigned char buffer[BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, stdin)) > 0) {
        size_t bytesWritten = fwrite(buffer, 1, bytesRead, file);
        if (bytesWritten != bytesRead) {
            perror("Error writing to file");
            fclose(file);
            return 1;
        }
    }

    if (ferror(stdin)) {
        perror("Error reading from stdin");
        fclose(file);
        return 1;
    }

    fclose(file);
    return 0;
}
