#include <windows.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filepath>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];

    // Open the clipboard
    if (!OpenClipboard(NULL)) {
        fputs("Cannot open clipboard\n", stderr);
        return 1;
    }

    // Get clipboard data in CF_TEXT format
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (!hData) {
        CloseClipboard();
        fputs("No text data on clipboard\n", stderr);
        return 1;
    }

    char* pText = (char*)GlobalLock(hData);
    if (!pText) {
        CloseClipboard();
        fputs("GlobalLock failed\n", stderr);
        return 1;
    }

    // Open output file
    FILE* f = fopen(filename, "wb");
    if (!f) {
        GlobalUnlock(hData);
        CloseClipboard();
        perror("Cannot open file");
        return 1;
    }

    // Write clipboard contents to file
    size_t written = fwrite(pText, 1, strlen(pText), f);
    fclose(f);

    GlobalUnlock(hData);
    CloseClipboard();

    if (written != strlen(pText)) {
        fputs("Failed to write full data to file\n", stderr);
        return 1;
    }

    printf("Clipboard text written to %s\n", filename);
    return 0;
}
