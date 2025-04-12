#include "shell.h"

int main (int argc, char **argv) {  
    int childPid; 
    char * cmdLine; 
    parseInfo *info;  
    while(1){  
        cmdLine= readline(">");  

        // Check for exit command directly here
        if (cmdLine != NULL && strcmp(cmdLine, "exit") == 0) {
            exit(EXIT_SUCCESS);
        }

        info = parse(cmdLine);

        // Check again after parsing
        if (info->argCount > 0 && strcmp(info->args[0], "exit") == 0) {
            exit(EXIT_SUCCESS);
        }

        childPid = fork();    
        if (childPid == 0) { 
            /* child code */ 
            executeCommand(info); //calls  execvp   
            // If we reach here, executeCommand failed
            exit(EXIT_FAILURE);
        } 
        else {  
            /* parent code */ 
            waitpid(childPid); 
            freeParseInfo(info); // Free memory
            free(cmdLine);       // Free command line      
        }   
    }  
} 