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

#define waitpid(pid) waitpid(pid, NULL, 0)

#ifndef SHELL_H
#define SHELL_H

typedef struct {
  char** args;       
    int argCount;      
    int hasRedirection; 
    char* outputFile;  
    int hasPipe;       
    char** pipeArgs;   
    int pipeArgCount;  

} parseInfo;


// Functions for parsing commands
char* readCommand();
char* readline(const char* prompt);  
parseInfo* parseCommand(char* cmdLine);
parseInfo* parse(char* cmdLine);    
void freeParseInfo(parseInfo* info);

// Functions for executing commands
int executeCommand(parseInfo* info);
int executeBuiltInCommand(parseInfo* info);
int executePipedCommand(parseInfo* info);
int executeExternalCommand(parseInfo* info);

// Buildin commands
int shellExit(parseInfo* info);
int shellCd(parseInfo* info);
int shellPwd(parseInfo* info);
int shellClear(parseInfo* info);
int shellGrep(parseInfo* info);

// Function to execute a tree command
void print_tree(const char* path, int level);


#endif 