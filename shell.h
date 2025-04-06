#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>

#ifndef SHELL_H
#define SHELL_H

typedef struct {
  char* command;
  char** args;

} parseInfo;


parseInfo* parse(char* input);
void freeParseInfo(parseInfo* info);
void executeCommand(parseInfo* info);

#endif // SHELL_H