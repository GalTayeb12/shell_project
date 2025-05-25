#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CONTENT_LENGTH 1024
#define MAX_LINE_LENGTH 256

typedef struct {
    int index;
    char content[MAX_CONTENT_LENGTH];
    int lastUsed;
    int isValid;
} Page;

Page* secondaryMemory;
Page** primaryMemory;

int m = 0; // Secondary memory size
int n = 0; // Primary memory size
int useLRU = 0;
int timeCounter = 0;

void initializeMemories(int _m, int _n) {
    m = _m;
    n = _n;
    secondaryMemory = (Page*)malloc(sizeof(Page) * m);
    for (int i = 0; i < m; i++) {
        secondaryMemory[i].index = i;
        secondaryMemory[i].content[0] = '\0';
        secondaryMemory[i].lastUsed = 0;
        secondaryMemory[i].isValid = 1;
    }
    primaryMemory = (Page**)malloc(sizeof(Page*) * n);
    for (int i = 0; i < n; i++) {
        primaryMemory[i] = NULL;
    }
    for (int i = 0; i < n && i < m; i++) {
        primaryMemory[i] = &secondaryMemory[i];
    }
}

Page* findPageInPrimary(int index) {
    for (int i = 0; i < n; i++) {
        if (primaryMemory[i] != NULL && primaryMemory[i]->index == index) {
            return primaryMemory[i];
        }
    }
    return NULL;
}

int findPageToReplace() {
    if (useLRU) {
        int oldest = timeCounter;
        int replaceIndex = 0;
        for (int i = 0; i < n; i++) {
            if (primaryMemory[i]->lastUsed < oldest) {
                oldest = primaryMemory[i]->lastUsed;
                replaceIndex = i;
            }
        }
        return replaceIndex;
    } else {
        return 0;
    }
}

Page* loadPageToPrimary(int index) {
    Page* newPage = &secondaryMemory[index];
    timeCounter++;
    for (int i = 0; i < n; i++) {
        if (primaryMemory[i] == NULL) {
            primaryMemory[i] = newPage;
            newPage->lastUsed = timeCounter;
            return newPage;
        }
    }
    int replaceIndex = findPageToReplace();
    Page* evictedPage = primaryMemory[replaceIndex];
    strcpy(secondaryMemory[evictedPage->index].content, evictedPage->content);
    primaryMemory[replaceIndex] = newPage;
    newPage->lastUsed = timeCounter;
    return newPage;
}

Page* getPage(int index) {
    Page* page = findPageInPrimary(index);
    if (page != NULL) {
        timeCounter++;
        page->lastUsed = timeCounter;
        return page;
    }
    return loadPageToPrimary(index);
}

void readCommand(int index, FILE* outputFile) {
    if (index < 0 || index >= m) return;
    Page* page = getPage(index);
    fprintf(outputFile, "%s\n", page->content);
}

void writeCommand(int index, char ch) {
    if (index < 0 || index >= m) return;
    Page* page = getPage(index);
    int len = strlen(page->content);
    if (len < MAX_CONTENT_LENGTH - 1) {
        page->content[len] = ch;
        page->content[len + 1] = '\0';
    }
}

void printCommand(FILE* outputFile) {
    for (int i = 0; i < m; i++) {
        fprintf(outputFile, "%s\n", secondaryMemory[i].content);
    }
}

void processInputFile(const char* inputFile, const char* outputFile) {
    FILE* in = fopen(inputFile, "r");
    FILE* out = fopen(outputFile, "w");
    if (!in || !out) {
        printf("Error opening input or output file.\n");
        return;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), in)) {
        if (strncmp(line, "read", 4) == 0) {
            int index;
            sscanf(line, "read %d", &index);
            readCommand(index, out);
        } else if (strncmp(line, "write", 5) == 0) {
            int index;
            char ch;
            sscanf(line, "write %d %c", &index, &ch);
            writeCommand(index, ch);
        } else if (strncmp(line, "print", 5) == 0) {
            printCommand(out);
        }
    }

    fclose(in);
    fclose(out);
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        printf("Usage: ./MemoryManagement useLRU input.txt output.txt m n\n");
        return 1;
    }
    useLRU = atoi(argv[1]);
    const char* inputFile = argv[2];
    const char* outputFile = argv[3];
    m = atoi(argv[4]);
    n = atoi(argv[5]);

    initializeMemories(m, n);
    processInputFile(inputFile, outputFile);

    free(secondaryMemory);
    free(primaryMemory);
    return 0;
}

