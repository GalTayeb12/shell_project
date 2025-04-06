#include <shell.h>


parseInfo* parse(char* input) {
    parseInfo* info = malloc(sizeof(parseInfo));
    if (!info) {
        perror("Failed to allocate memory for parseInfo");
        return NULL;
    }

    // Tokenize the input string
    char* token = strtok(input, " \n");
    if (token == NULL) {
        free(info);
        return NULL; // No command found
    }

    info->command = strdup(token);
    if (!info->command) {
        perror("Failed to allocate memory for command");
        free(info);
        return NULL;
    }

    // Allocate memory for args
    info->args = malloc(10 * sizeof(char*)); // Arbitrary size for demonstration
    if (!info->args) {
        perror("Failed to allocate memory for args");
        free(info->command);
        free(info);
        return NULL;
    }

    int i = 0;
    while ((token = strtok(NULL, " \n")) != NULL) {
        info->args[i] = strdup(token);
        if (!info->args[i]) {
            perror("Failed to allocate memory for argument");
            for (int j = 0; j < i; j++) {
                free(info->args[j]);
            }
            free(info->args);
            free(info->command);
            free(info);
            return NULL;
        }
        i++;
    }
    info->args[i] = NULL; // Null-terminate the args array

    return info;
}


void freeParseInfo(parseInfo* info) {
    if (info) {
        free(info->command);
        for (int i = 0; info->args[i] != NULL; i++) {
            free(info->args[i]);
        }
        free(info->args);
        free(info);
    }
}

void executeCommand(parseInfo* info) {
    if (info == NULL || info->command == NULL) {
        fprintf(stderr, "No command to execute\n");
        return;
    }

    // Fork a child process
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return;
    }

    if (pid == 0) { // Child process
        execvp(info->command, info->args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else { // Parent process
        int status;
        waitpid(pid, &status, 0);
    }
}