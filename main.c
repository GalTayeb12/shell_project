#include "shell.h"

int main (int argc, char **argv) {  
    int childPid; 
    char * cmdLine; 
    parseInfo *info;  
    while(1){  
        cmdLine= readline(">");  
        info = parse(cmdLine);
        if (info == 'exit') {
            exit();
        }  
        childPid = fork();    
        if (childPid == 0) { 
            /* child code */ 
            executeCommand(info); //calls  execvp   
        } 
        else {  
            /* parent code */ 
            waitpid(childPid);       
        }   
    }  
} 