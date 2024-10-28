#include <Windows.h>

int main(void) {
    HANDLE procHeap = GetProcessHeap();
    HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);

    const char* string = "This is a very long string for testing purposes to see whether or not this works. So, I have added even more text to make this a bit longer";

    int buff_size = 0;
    for (int i = 0; string[i] != '\0'; i++) {
        buff_size++;
    }

    int segments = (int)(buff_size / 10) + (int)(((buff_size % 10) + 9) / 10);

    char** buffer = (char**)HeapAlloc(procHeap, HEAP_ZERO_MEMORY, segments * sizeof(char*));

    for (int i = 0; i < segments; i++) {
        buffer[i] = HeapAlloc(procHeap, HEAP_ZERO_MEMORY, 10 * sizeof(char));
    }

    int row = 0;
    int column = 0;
    for (int i = 0; string[i] != '\0'; i++) {
        if (i % 10 == 0 && i != 0) {
            row++;
            column = 0;
        }
        buffer[row][column] = string[i];
        column++;
    }

    for (int i = 0; i < segments; i++) {
        WriteFile(hout, buffer[i], 10, NULL, NULL);
        WriteFile(hout, "\n", 1, NULL, NULL);
    }

    for (int i = 0; i < segments; i++) {
        HeapFree(procHeap, 0, buffer[i]);
    }
    HeapFree(procHeap, 0, buffer);
    

    CloseHandle(procHeap);
    return 0;
}