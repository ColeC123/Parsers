#include <stdio.h>
#include <stdbool.h>
#include <Windows.h>

#define LINEMAX 100

//Note for future: do not specify entry point when using subsystem:console, as the latter will specify the starting point to the ucrt setup

// cl /c edge.c /Fo:edge.o
// link edge.o ucrt.lib kernel32.lib vcruntime.lib msvcrt.lib /nodefaultlib /subsystem:console /out:edge.exe

typedef struct INTARRAY {
    int* arr;
    int size;
    int msize;
} INTARRAY;

typedef struct string {
    char* str;
    int size;
} string;

#define default_string (string){.str = NULL, .size = 0}

void appendInt(INTARRAY* iarr, int value);

void intToStr(string* str, int value);

int main(int argc, char** argv) {


    HANDLE file = CreateFileA(
        argv[1],
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, 
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (file == INVALID_HANDLE_VALUE) {
        printf("Failed to open specified file or file does not exist\n");
        return -1;
    }

    int fileSize = GetFileSize(file, NULL);

    char* buffer = (char*)malloc(fileSize);

    if (buffer == NULL) {
        printf("Failed to allocate memory for the buffer\n");
        return -1;
    }

    if (!ReadFile(file, buffer, fileSize, NULL, NULL)) {
        printf("Failed to read from specified file\n");
        return -1;
    }

    buffer[fileSize-1] = '\0';

    int vertexCount = 0;
    for (int i = 1; i < fileSize; i++) {
        if (buffer[i-1] == '\n' && buffer[i] == 'v') {
            vertexCount++;
        }
    }

    char** vertices = (char**)malloc(vertexCount * sizeof(char*));

    for (int i = 0; i < vertexCount; i++) {
        vertices[i] = (char*)malloc(LINEMAX * sizeof(char));
    }

    int currVertex = 0;
    for (int i = 1; i < fileSize; i++) {
        int count = 0;
        if (buffer[i-1] == '\n' && buffer[i] == 'v') {
            for (int j = 0; buffer[i+j+2] != '\n' && buffer[i+j+2] != '\0' && i+j+2 < fileSize && j < LINEMAX - 2; j++) {
                vertices[currVertex][j] = buffer[i+j+2];
                count++;
            }

            vertices[currVertex][count] = '\n';
            vertices[currVertex][count+1] = '\0';
            currVertex++;
        }
    }

    int faceCount = 0;
    for (int i = 1; i < fileSize; i++) {
        if (buffer[i-1] == '\n' && buffer[i] == 'f') {
            faceCount++;
        }
    }

    char** faces = (char**)malloc(faceCount * sizeof(char*));

    for (int i = 0; i < faceCount; i++) {
        faces[i] = (char*)malloc(LINEMAX * sizeof(char));
    }

    int currFace = 0;
    for (int i = 1; i < fileSize; i++) {
        int count = 0;
        if (buffer[i-1] == '\n' && buffer[i] == 'f') {
            for (int j = 0; buffer[i+j+2] != '\n' && buffer[i+j+2] != '\0' && i+j+2 < fileSize && j < LINEMAX - 2; j++) {
                faces[currFace][j] = buffer[i+j+2];
                count++;
            }
            faces[currFace][count] = '\n';
            faces[currFace][count+1] = '\0';
            currFace++;
        }
    }

    INTARRAY intEdges;
    intEdges.msize = 60;
    intEdges.size = 0;
    intEdges.arr = (int*)malloc(intEdges.msize * sizeof(int));

    for (int i = 0; i < faceCount; i++) {
        int dividers[4];
        int dividersCount = 0;

        for (int j = 0; faces[i][j] != '\0' && dividersCount < 4; j++) {
            if (faces[i][j] == ' ' || faces[i][j] == '\n') {
                dividers[dividersCount] = j;
                dividersCount++;
            }
        }

        if (dividersCount == 3) {
            string num1;
            num1.size = (dividers[0] - 0) + 1;
            num1.str = (char*)malloc(num1.size * sizeof(char));
            num1.str[num1.size - 1] = '\0';

            for (int j = 0; j < dividers[0]; j++) {
                num1.str[j] = faces[i][j];
            }

            string num2;
            num2.size = (dividers[1] - (dividers[0] + 1)) + 1;
            num2.str = (char*)malloc(num2.size * sizeof(char));
            num2.str[num2.size - 1] = '\0';

            for (int j = dividers[0] + 1; j < dividers[1]; j++) {
                num2.str[j - dividers[0] - 1] = faces[i][j];
            }

            string num3;
            num3.size = (dividers[2] - (dividers[1] + 1)) + 1;
            num3.str = (char*)malloc(num3.size * sizeof(char));
            num3.str[num3.size - 1] = '\0';

            for (int j = dividers[1] + 1; j < dividers[2]; j++) {
                num3.str[j - dividers[1] - 1] = faces[i][j];
            }

            int n1 = atoi(num1.str);
            int n2 = atoi(num2.str);
            int n3 = atoi(num3.str);

            bool found = false;
            for (int j = 0; j < intEdges.size; j += 2) {
                if ((n1 == intEdges.arr[j] && n2 == intEdges.arr[j+1]) || (n1 == intEdges.arr[j+1] && n2 == intEdges.arr[j])) {
                    found = true;
                    break;
                }
            }

            if (found == false) {
                appendInt(&intEdges, n1);
                appendInt(&intEdges, n2);
            }

            found = false;
            for (int j = 0; j < intEdges.size; j += 2) {
                if ((n1 == intEdges.arr[j] && n3 == intEdges.arr[j+1]) || (n1 == intEdges.arr[j+1] && n3 == intEdges.arr[j])) {
                    found = true;
                    break;
                }
            }

            if (found == false) {
                appendInt(&intEdges, n1);
                appendInt(&intEdges, n3);
            }

            found = false;
            for (int j = 0; j < intEdges.size; j += 2) {
                if ((n2 == intEdges.arr[j] && n3 == intEdges.arr[j+1]) || (n2 == intEdges.arr[j+1] && n3 == intEdges.arr[j])) {
                    found = true;
                    break;
                }
            }
            
            if (found == false) {
                appendInt(&intEdges, n2);
                appendInt(&intEdges, n3);
            }

            free(num1.str);
            free(num2.str);
            free(num3.str);
        } else if (dividersCount == 4) {
            string num1;
            num1.size = (dividers[0] - 0) + 1;
            num1.str = (char*)malloc(num1.size * sizeof(char));
            num1.str[num1.size - 1] = '\0';

            for (int j = 0; j < dividers[0]; j++) {
                num1.str[j] = faces[i][j];
            }

            string num2;
            num2.size = (dividers[1] - (dividers[0] + 1)) + 1;
            num2.str = (char*)malloc(num2.size * sizeof(char));
            num2.str[num2.size - 1] = '\0';

            for (int j = dividers[0] + 1; j < dividers[1]; j++) {
                num2.str[j - dividers[0] - 1] = faces[i][j];
            }

            string num3;
            num3.size = (dividers[2] - (dividers[1] + 1)) + 1;
            num3.str = (char*)malloc(num3.size * sizeof(char));
            num3.str[num3.size - 1] = '\0';

            for (int j = dividers[1] + 1; j < dividers[2]; j++) {
                num3.str[j - dividers[1] - 1] = faces[i][j];
            }

            string num4;
            num4.size = (dividers[3] - (dividers[2] + 1)) + 1;
            num4.str = (char*)malloc(num4.size * sizeof(char));
            num4.str[num4.size - 1] = '\0';

            for (int j = dividers[2] + 1; j < dividers[3]; j++) {
                num4.str[j - dividers[2] - 1] = faces[i][j];
            }

            int n1 = atoi(num1.str);
            int n2 = atoi(num2.str);
            int n3 = atoi(num3.str);
            int n4 = atoi(num4.str);

            bool found = false;
            for (int j = 0; j < intEdges.size; j += 2) {
                if ((n1 == intEdges.arr[j] && n2 == intEdges.arr[j+1]) || (n1 == intEdges.arr[j+1] && n2 == intEdges.arr[j])) {
                    found = true;
                    break;
                }
            }

            if (found == false) {
                appendInt(&intEdges, n1);
                appendInt(&intEdges, n2);
            }

            found = false;
            for (int j = 0; j < intEdges.size; j += 2) {
                if ((n2 == intEdges.arr[j] && n3 == intEdges.arr[j+1]) || (n2 == intEdges.arr[j+1] && n3 == intEdges.arr[j])) {
                    found = true;
                    break;
                }
            }

            if (found == false) {
                appendInt(&intEdges, n2);
                appendInt(&intEdges, n3);
            }

            found = false;
            for (int j = 0; j < intEdges.size; j += 2) {
                if ((n3 == intEdges.arr[j] && n4 == intEdges.arr[j+1]) || (n3 == intEdges.arr[j+1] && n4 == intEdges.arr[j])) {
                    found = true;
                    break;
                }
            }
            
            if (found == false) {
                appendInt(&intEdges, n3);
                appendInt(&intEdges, n4);
            }

            found = false;
            for (int j = 0; j < intEdges.size; j += 2) {
                if ((n4 == intEdges.arr[j] && n1 == intEdges.arr[j+1]) || (n4 == intEdges.arr[j+1] && n1 == intEdges.arr[j])) {
                    found = true;
                    break;
                }
            }

            if (found == false) {
                appendInt(&intEdges, n4);
                appendInt(&intEdges, n1);
            }
            
            free(num1.str);
            free(num2.str);
            free(num3.str);
        }
    }

    int EdgesCount = (int)(intEdges.size / 2);
    char** Edges = (char**)malloc(EdgesCount * sizeof(char*));

    string s1 = default_string;
    string s2 = default_string;
    for (int i = 0; i < intEdges.size; i += 2) {
        int curr_edge_index = (int)(i/2);

        intToStr(&s1, intEdges.arr[i]);
        intToStr(&s2, intEdges.arr[i+1]);

        int size = s1.size + s2.size + 4;
        Edges[curr_edge_index] = (char*)malloc(size * sizeof(char));
        
        int count = 0;
        for (int j = 0; j < s1.size && j < size-2; j++) {
            Edges[curr_edge_index][count] = s1.str[j];
            count++;
        }

        Edges[curr_edge_index][count] = ' ';
        count++;

        for (int j = 0; j < s2.size && j < size-2; j++) {
            Edges[curr_edge_index][count] = s2.str[j];
            count++;
        }

        Edges[curr_edge_index][count] = '\n';
        count++;
        Edges[curr_edge_index][count] = '\0';

    }
    free(s1.str);
    free(s2.str);

    for (int i = 0; i < vertexCount; i++) {
        for (int j = 0; vertices[i][j] != '\0' && j < LINEMAX; j++) {
            if (vertices[i][j] == ' ' || vertices[i][j] == '\n') {
                vertices[i][j] = ',';
            }
        }
    }

    for (int i = 0; i < faceCount; i++) {
        for (int j = 0; faces[i][j] != '\0' && j < LINEMAX; j++) {
            if (faces[i][j] == ' ' || faces[i][j] == '\n') {
                faces[i][j] = ',';
            }
        }
    }

    for (int i = 0; i < EdgesCount; i++) {
        for (int j = 0; Edges[i][j] != '\0'; j++) {
            if (Edges[i][j] == ' ' || Edges[i][j] == '\n') {
                Edges[i][j] = ',';
            }
        }
    }

    printf("Vertices:\n");
    for (int i = 0; i < vertexCount; i++) {
        printf("%s\n", vertices[i]);
    }

    printf("\nFaces:\n");
    for (int i = 0; i < faceCount; i++) {
        printf("%s\n", faces[i]);
    }

    printf("\nEdges:\n");
    for (int i = 0; i < EdgesCount; i++) {
        printf("%s\n", Edges[i]);
    };

    for (int i = 0; i < EdgesCount; i++) {
        free(Edges[i]);
    }
    free(Edges);

    for (int i = 0; i < faceCount; i++) {
        free(faces[i]);
    }
    free(faces);

    for (int i = 0; i < vertexCount; i++) {
        free(vertices[i]);
    }
    free(vertices);

    free(intEdges.arr);

    return 0;
}

void appendInt(INTARRAY* iarr, int value) {
    if (iarr->size >= iarr->msize) {
        iarr->msize *= 2;
        iarr->arr = (int*)realloc(iarr->arr, iarr->msize * sizeof(int));
    }

    iarr->arr[iarr->size] = value;
    iarr->size++;
}

void intToStr(string* str, int value) {
    int size = 0;
    int temp = value;
    
    while (temp > 0) {
        size++;
        temp /= 10;
    }

    if (value == 0) {
        size = 1;
    }

    free(str->str);

    str->size = size;
    str->str = (char*)malloc((str->size + 1) * sizeof(char));

    int temp2 = value;
    for (int i = size - 1; i >= 0; i--) {
        str->str[i] =  (char)((temp2 % 10) + 48);
        temp2 /= 10;
    }
    str->str[size] = '\0';
}