#include "shell.h"

int main(int argc, char** argv) {
    char* cmdLine;
    parseInfo* info;
    
    // לולאה אינסופית עד שמשתמש מקליד exit
    while (1) {
        printf("> ");
        cmdLine = readCommand();
        info = parseCommand(cmdLine);
        
        if (info->argCount > 0) {
            executeCommand(info);
        }
        
        freeParseInfo(info);
        free(cmdLine);
    }
    
    return 0;
}