#include <windows.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filepath>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("Cannot open file");
        return 1;
    }

    // Determine file size
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size < 0) {
        fclose(f);
        fputs("Failed to determine file size\n", stderr);
        return 1;
    }

    // Allocate movable global memory for the clipboard
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size + 1);
    if (!hMem) {
        fclose(f);
        fputs("GlobalAlloc failed\n", stderr);
        return 1;
    }

    // Lock memory and read file into it
    void* pMem = GlobalLock(hMem);
    if (!pMem) {
        fclose(f);
        GlobalFree(hMem);
        fputs("GlobalLock failed\n", stderr);
        return 1;
    }

    if (fread(pMem, 1, size, f) != (size_t)size) {
        fclose(f);
        GlobalUnlock(hMem);
        GlobalFree(hMem);
        fputs("File read error\n", stderr);
        return 1;
    }
    ((char*)pMem)[size] = '\0'; // Null-terminate for CF_TEXT

    GlobalUnlock(hMem);
    fclose(f);

    // Open the clipboard and set data
    if (!OpenClipboard(NULL)) {
        GlobalFree(hMem);
        fputs("Cannot open clipboard\n", stderr);
        return 1;
    }

    EmptyClipboard();
    if (!SetClipboardData(CF_TEXT, hMem)) {
        CloseClipboard();
        GlobalFree(hMem);
        fputs("SetClipboardData failed\n", stderr);
        return 1;
    }

    CloseClipboard();
    printf("File copied to clipboard successfully.\n");
    return 0;
}
