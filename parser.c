#include <shell.c>

parseInfo* parse(char* input) {
  parseInfo* info = malloc(sizeof(parseInfo));
  char* token;
  int i = 0;
  info->args = malloc(10 * sizeof(char*)); // Arbitrary size for demonstration
  token = strtok(input, " \n");

  
}