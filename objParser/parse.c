#include <windows.h>

HANDLE procHeap;

// cl parse.c /c /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\um\Windows.h" /Fo:parse.o

// link parse.o kernel32.lib ucrt.lib vcruntime.lib msvcrt.lib /nodefaultlib /entry:main /subsystem:console /out:parse.exe

int main(void) {
    //Need the handle to the standard output in order to print to the console with WriteFile
    HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);

    //The process heap is needed to allocate with the HeapAlloc function. This is the process's default heap
    procHeap = GetProcessHeap();
    if (procHeap == NULL) {
        WriteFile(hout, "\nFailed to get process heap\n", 28, NULL, NULL);
        return -1;
    }

    //Since I decided to try and only use kernel32.lib functions, in order to get access to command line arguments
    //I have to use the below function, but this function returns the entire string of the line at the time
    //when the executable was called and the arguments were passed, which means it needs to be parsed
    char* argv = GetCommandLineA();
    int argv_size = 0;
    for (int i = 0; argv[i] != '\0'; i++) {
        argv_size++;
    }

    //The input indicator will make it easier for me to figure out where the user has typed the file
    const char* input_indicator = "/file";
    int input_indicator_index = 0;

    //This just searches for the first index at which the input indicator string occurrs
    for (int i = 0; i < argv_size - 5; i++) {
        int comparison_counter = 0;
        for (int j = 0; j < 5; j++) {
            if (input_indicator[j] == argv[i+j]) {
                comparison_counter++;
            }
        }

        if (comparison_counter == 5) {
            input_indicator_index = i;
        }
    }

    char filePath[261];
    int filePathSize = 0;

    //The first if statement is for the condition when there are no quotes around file's path
    if (argv[input_indicator_index+5] == ' ' && argv[input_indicator_index+6] != '"') {
        //Add six to get start of where file path was typed (this assumes a space is placed after /file)
        //This will keep reading until a space is found or the end of the argv string is reached
        for (int i = input_indicator_index+6; i < argv_size && argv[i] != ' '; i++) {
            filePath[filePathSize] = argv[i];
            filePathSize++;
        }
    } else if (argv[input_indicator_index + 6] == '"') {
        //Have to add 7 to input indicator index in order to account for size of the input indicator string
        //the space and the quote. The filePath string will then continue reading from argv
        //until either the end of the string is reached or another quotation mark is read
        for (int i = input_indicator_index+7; i < argv_size && argv[i] != '"'; i++) {
            filePath[filePathSize] = argv[i];
            filePathSize++;
        }
    } else {
        WriteFile(hout, "\nArguments were formatted incorrectly. Try:\n ./parse /file pathToFile\nor if the path has spaces:\n./parse /file \"pathToFile\"\n", 124, NULL, NULL);
        return -1;
    }
    filePath[filePathSize] = '\0';

    HANDLE hfile = CreateFileA(
        filePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hfile == INVALID_HANDLE_VALUE) {
        WriteFile(hout, "\nCould not open file or file does not exist\n", 44, NULL, NULL);
        return -1;
    }

    int fileSize = 0;
    fileSize = GetFileSize(hfile, NULL);

    char* buffer = (char*)HeapAlloc(procHeap, HEAP_ZERO_MEMORY, fileSize * sizeof(char));

    if (buffer == NULL) {
        WriteFile(hout, "\nFailed to allocate memory for buffer\n", 38, NULL, NULL);
        return -1;
    }

    if (!ReadFile(hfile, buffer, fileSize * sizeof(char), NULL, NULL)) {
        WriteFile(hout, "\nFailed to read file into buffer\n", 33, NULL, NULL);
        return -1;
    }

    int vertices_count = 0;
    int faces_count = 0;
    for (int i = 0; i < fileSize; i++) {
        if (buffer[i] == 'v') {
            vertices_count++;
        } else if (buffer[i] == 'f') {
            int space_count = 0;
            for (int j = 0; buffer[i + j + 2] != '\n' && i + j + 2 < fileSize; j++) {
                if (buffer[i + j + 2] == ' ') {
                    space_count++;
                }
            }

            if (space_count == 3) {
                faces_count += 2;
            } else {
                faces_count++;
            }
        }
    }

    char** vertices = (char**)HeapAlloc(procHeap, HEAP_ZERO_MEMORY, vertices_count * sizeof(char*));

    if (vertices == NULL) {
        WriteFile(hout, "\nFailed to allocate memory for vertices outer buffer\n", 53, NULL, NULL);
        return -1;
    }

    // Create 2d array for the vertices information
    int vertex_count = 0;
    for (int i = 0; i < fileSize; i++) {
        if (buffer[i] == 'v' && i > 0 && buffer[i - 1] == '\n') {
            vertices[vertex_count] = HeapAlloc(procHeap, HEAP_ZERO_MEMORY, 62);

            if (vertices[vertex_count] == NULL) {
                WriteFile(hout, "\nFailed to allocate memory to inner vertices arrays\n", 52, NULL, NULL);
                return -1;
            }

            int v_size = 0;

            for (int j = 0; j < 60 && buffer[i + j + 2] != '\n' && i + j + 2 < fileSize; j++) {
                vertices[vertex_count][j] = buffer[i + j + 2];
                v_size++;
            }
            vertices[vertex_count][v_size] = '\n';

            vertex_count++;
        }
    }

    char** faces = (char**)HeapAlloc(procHeap, HEAP_ZERO_MEMORY, faces_count * sizeof(char*));

    if (faces == NULL) {
        WriteFile(hout, "\nFailed to allocate memory for the faces\n", 41, NULL, NULL);
        return -1;
    }

    // Create the 2d array for the faces information
    int face_count = 0;
    for (int i = 0; i < fileSize; i++) {
        if (buffer[i] == 'f' && i > 0 && buffer[i - 1] == '\n') {
            int space_count = 0;
            int space_indices[3];

            for (int j = 0; buffer[i + j + 2] != '\n' && i + j + 2 < fileSize; j++) {
                if (buffer[i + j + 2] == ' ' && space_count < 3) {
                    space_indices[space_count] = i + j + 2;

                    space_count++;
                }
            }

            if (space_count == 3) {
                faces[face_count] = HeapAlloc(procHeap, HEAP_ZERO_MEMORY, 62);
                faces[face_count + 1] = HeapAlloc(procHeap, HEAP_ZERO_MEMORY, 62);

                if (faces[face_count] == NULL || faces[face_count+1] == NULL) {
                    WriteFile(hout, "\nFailed to allocate memory for inner faces array\n", 49, NULL, NULL);
                    return -1;
                }

                int f1_size = 0;
                for (int j = i+2; j < space_indices[2] && j < fileSize && f1_size < 60; j++) {
                    faces[face_count][f1_size] = buffer[j];
                    f1_size++;
                }
                faces[face_count][f1_size] = '\n';

                int f2_size = 0;
                for (int j = i+2; j <= space_indices[0] && j < fileSize && f2_size < 60; j++) {
                    faces[face_count+1][f2_size] = buffer[j];
                    f2_size++;
                }

                for (int j = space_indices[1] + 1; buffer[j] != '\n' && f2_size < 60 && j < fileSize; j++) {
                    faces[face_count+1][f2_size] = buffer[j];
                    f2_size++;
                }
                faces[face_count + 1][f2_size] = '\n';

                face_count += 2;
            } else {
                faces[face_count] = HeapAlloc(procHeap, HEAP_ZERO_MEMORY, 62);

                if (faces[face_count] == NULL) {
                    WriteFile(hout, "\nFailed to allocate memory for inner faces array\n", 49, NULL, NULL);
                    return -1;
                }

                int f_size = 0;
                for (int j = 0; j < 60 && buffer[i + j + 2] != '\n' && i + j + 2 < fileSize; j++) {
                    faces[face_count][j] = buffer[i + j + 2];
                    f_size++;
                }
                faces[face_count][f_size] = '\n';

                face_count++;
            }
        }
    }

    // Format the vertices to have commas between them and at the end of the lines
    for (int i = 0; i < vertices_count; i++) {
        int newline_index = 0;

        for (int j = 0; j < 61; j++) {
            if (vertices[i][j] == '\n') {
                newline_index = j;
                break;
            }
        }

        for (int j = 0; j <= newline_index; j++) {
            if (vertices[i][j] == ' ') {
                vertices[i][j] = ',';
            } else if (vertices[i][j] == '\n') {
                vertices[i][j] = ',';
                vertices[i][j + 1] = '\n';
                break;
            }
        }
    }

    // Print out the vertices
    WriteFile(hout, "\nVertices:\n", 11, NULL, NULL);
    for (int i = 0; i < vertices_count; i++) {
        int size = 0;
        for (int j = 0; vertices[i][j] != '\n' && j < 62; j++) {
            size++;
        }

        WriteFile(hout, vertices[i], size + 1, NULL, NULL);
    }

    // Format the faces to have commas between them and at the end of the lines
    for (int i = 0; i < faces_count; i++) {
        int newline_index = 0;

        for (int j = 0; j < 61; j++) {
            if (faces[i][j] == '\n') {
                newline_index = j;
                break;
            }
        }

        for (int j = 0; j <= newline_index; j++) {
            if (faces[i][j] == ' ') {
                faces[i][j] = ',';
            } else if (faces[i][j] == '\n') {
                faces[i][j] = ',';
                faces[i][j + 1] = '\n';
                break;
            }
        }
    }

    // Print out the faces/tris
    WriteFile(hout, "\nFaces/Triangles:\n", 18, NULL, NULL);
    for (int i = 0; i < faces_count; i++) {
        int size = 0;
        for (int j = 0; faces[i][j] != '\n' && j < 62; j++) {
            size++;
        }

        WriteFile(hout, faces[i], size + 1, NULL, NULL);
    }

    // Free Memory and close handles from this point on
    for (int i = 0; i < vertices_count; i++) {
        HeapFree(procHeap, 0, vertices[i]);
    }
    HeapFree(procHeap, 0, vertices);

    for (int i = 0; i < faces_count; i++) {
        HeapFree(procHeap, 0, faces[i]);
    }
    HeapFree(procHeap, 0, faces);

    HeapFree(procHeap, 0, buffer);
    CloseHandle(hfile);
    CloseHandle(procHeap);

    return 0;
}