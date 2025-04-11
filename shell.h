#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

// מאקרו לעקיפת הבעיה עם waitpid
#define waitpid(pid) waitpid(pid, NULL, 0)

#ifndef SHELL_H
#define SHELL_H

typedef struct {
  char** args;       // מערך של ארגומנטים
    int argCount;      // מספר הארגומנטים
    int hasRedirection; // האם יש הפניית פלט
    char* outputFile;  // קובץ היעד להפניית פלט
    int hasPipe;       // האם יש צינור
    char** pipeArgs;   // ארגומנטים אחרי הצינור
    int pipeArgCount;  // מספר הארגומנטים אחרי הצינור

} parseInfo;


// פונקציות לניתוח פקודות
char* readCommand();
char* readline(const char* prompt);  // הוספת הפונקציה כדי להתאים לקריאה ב-main.c
parseInfo* parseCommand(char* cmdLine);
parseInfo* parse(char* cmdLine);     // הוספת הפונקציה כדי להתאים לקריאה ב-main.c
void freeParseInfo(parseInfo* info);

// פונקציות לביצוע פקודות
int executeCommand(parseInfo* info);
int executeBuiltInCommand(parseInfo* info);
int executePipedCommand(parseInfo* info);
int executeExternalCommand(parseInfo* info);

// פקודות מובנות
int shellExit(parseInfo* info);
int shellCd(parseInfo* info);
int shellPwd(parseInfo* info);
int shellClear(parseInfo* info);

// פונקציה לביצוע פקודת tree
void print_tree(const char* path, int level);


#endif // SHELL_H