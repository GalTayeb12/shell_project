#include "shell.h"

//Reading a command from the user
char* readCommand() {
    char* buffer = NULL;
    size_t bufsize = 0;
    getline(&buffer, &bufsize, stdin);
    
    // הסרת תו שורה חדשה
    if (buffer[strlen(buffer) - 1] == '\n') {
        buffer[strlen(buffer) - 1] = '\0';
    }
    
    return buffer;
}

char* readline(const char* prompt) {
    printf("%s", prompt);
    fflush(stdout);
    char* buffer = NULL;
    size_t bufsize = 0;
    ssize_t chars_read = getline(&buffer, &bufsize, stdin);
    
    if (chars_read > 0 && buffer[chars_read - 1] == '\n') {
        buffer[chars_read - 1] = '\0';
    }
    
    return buffer;
}

// Breaking a command into arguments
parseInfo* parseCommand(char* cmdLine) {
    parseInfo* info = malloc(sizeof(parseInfo));
    info->args = NULL;
    info->argCount = 0;
    info->hasRedirection = 0;
    info->outputFile = NULL;
    info->hasPipe = 0;
    info->pipeArgs = NULL;
    info->pipeArgCount = 0;
    
    if (cmdLine == NULL || strlen(cmdLine) == 0) {
        return info;
    }
    
    int capacity = 10;
    info->args = malloc(capacity * sizeof(char*));
    
    // check about a pipe   
    char* pipePos = strchr(cmdLine, '|');
    if (pipePos != NULL) {
        info->hasPipe = 1;
        *pipePos = '\0';  // Separation of the first part  
        char* pipeCmd = pipePos + 1;
        
        info->pipeArgs = malloc(capacity * sizeof(char*));
        char* pipeToken;
        char* pipeRest = pipeCmd;
        while ((pipeToken = strtok_r(pipeRest, " \t", &pipeRest))) {
            if (info->pipeArgCount >= capacity) {
                capacity *= 2;
                info->pipeArgs = realloc(info->pipeArgs, capacity * sizeof(char*));
            }
            
            // Ignoring spaces  
            if (strlen(pipeToken) > 0) {
                info->pipeArgs[info->pipeArgCount++] = strdup(pipeToken);
            }
        }
        
        // Adding a Null value to the end of the array 
        if (info->pipeArgCount >= capacity) {
            capacity += 1;
            info->pipeArgs = realloc(info->pipeArgs, capacity * sizeof(char*));
        }
        info->pipeArgs[info->pipeArgCount] = NULL;
    }
    
    // Decompose the first command with quotation mark support
    char* current = cmdLine;
    int i = 0;
    int len = strlen(cmdLine);
    
    while (i < len) {
        // Skipping spaces  
        while (i < len && (cmdLine[i] == ' ' || cmdLine[i] == '\t')) {
            i++;
        }
        
        if (i >= len) break;  
        
        // Check if this is an output redirection character
        if (cmdLine[i] == '>') {
            info->hasRedirection = 1;
            i++;  // Progress beyond the '>' character
            
            while (i < len && (cmdLine[i] == ' ' || cmdLine[i] == '\t')) {
                i++;
            }
            
            if (i < len) {
                // read file name  
                int start = i;
                
                if (cmdLine[i] == '"' || cmdLine[i] == '\'') {
                    char quote = cmdLine[i];
                    i++;  // Skipping the opening quotation mark
                    start = i;
                    
                    // search the closing quotation mark
                    while (i < len && cmdLine[i] != quote) {
                        i++;
                    }
                    
                    cmdLine[i] = '\0';  // finish string in closing quotation mark 
                    info->outputFile = strdup(&cmdLine[start]);
                    i++;  // Skipping the closing quotation mark
                } else {
                    // file name without quotations 
                    while (i < len && cmdLine[i] != ' ' && cmdLine[i] != '\t') {
                        i++;
                    }
                    
                    char temp = cmdLine[i];
                    cmdLine[i] = '\0';  // End of string temporarily
                    info->outputFile = strdup(&cmdLine[start]);
                    cmdLine[i] = temp;  // Restoring the original character
                }
            }
            
            break;  
        }
        
        // Check if this is an argument with quotes
        if (cmdLine[i] == '"' || cmdLine[i] == '\'') {
            char quote = cmdLine[i];
            i++;  
            int start = i;
            
            
            while (i < len && cmdLine[i] != quote) {
                i++;
            }
            
            if (i < len) {
                cmdLine[i] = '\0';  
                
                if (info->argCount >= capacity) {
                    capacity *= 2;
                    info->args = realloc(info->args, capacity * sizeof(char*));
                }
                
                info->args[info->argCount++] = strdup(&cmdLine[start]);
                i++;  
            }
        } else {
            //Regular argument without quotes
            int start = i;
            
            while (i < len && cmdLine[i] != ' ' && cmdLine[i] != '\t' && cmdLine[i] != '>') {
                i++;
            }
            
            char temp = cmdLine[i];
            cmdLine[i] = '\0';  // finish temp string
            
            if (info->argCount >= capacity) {
                capacity *= 2;
                info->args = realloc(info->args, capacity * sizeof(char*));
            }
            
            info->args[info->argCount++] = strdup(&cmdLine[start]);
            cmdLine[i] = temp;  // Restoring the original character
            
            if (temp == '>') {
                i--;  //Backtrack so the next loop recognizes the '>' character
            }
        }
    }
    
    // Adding NULL at the end of the array
    if (info->argCount >= capacity) {
        capacity += 1;
        info->args = realloc(info->args, capacity * sizeof(char*));
    }
    info->args[info->argCount] = NULL;
    
    return info;
}


parseInfo* parse(char* cmdLine) {
    if (cmdLine != NULL && (strcmp(cmdLine, "exit") == 0 || 
                            strncmp(cmdLine, "exit ", 5) == 0)) {
        exit(EXIT_SUCCESS);  
    }
    
    parseInfo* info = parseCommand(cmdLine);
    
    if (info->argCount > 0 && strcmp(info->args[0], "cd") == 0) {
        shellCd(info);
        strcpy(info->args[0], "_CD_EXECUTED_");
    }
    
    return info;
}

// Freeing memory of parseInfo structure
void freeParseInfo(parseInfo* info) {
    if (info == NULL) {
        return;
    }
    
    if (info->args != NULL) {
        for (int i = 0; i < info->argCount; i++) {
            free(info->args[i]);
        }
        free(info->args);
    }
    
    if (info->pipeArgs != NULL) {
        for (int i = 0; i < info->pipeArgCount; i++) {
            free(info->pipeArgs[i]);
        }
        free(info->pipeArgs);
    }
    
    if (info->outputFile != NULL) {
        free(info->outputFile);
    }
    
    free(info);
}

// Execute a built-in or external command
int executeCommand(parseInfo* info) {
    if (info == NULL || info->argCount == 0) {
        return 0;
    }
    
    //Checking whether this is a cd command that has already been executed
    if (strcmp(info->args[0], "_CD_EXECUTED_") == 0) {
        exit(EXIT_SUCCESS);  // End of the child process
    }
    
    //Checking if this is a built-in command
    if (executeBuiltInCommand(info)) {
        exit(EXIT_SUCCESS);  
    }
    
    if (info->hasPipe) {
        return executePipedCommand(info);
    }
    
    return executeExternalCommand(info);
}

// Executing built-in commands
int executeBuiltInCommand(parseInfo* info) {
    if (strcmp(info->args[0], "exit") == 0) {
        exit(EXIT_SUCCESS); 
    } else if (strcmp(info->args[0], "cd") == 0) {
        return shellCd(info);
    } else if (strcmp(info->args[0], "pwd") == 0) {
        return shellPwd(info);
    } else if (strcmp(info->args[0], "clear") == 0) {
        return shellClear(info);
    } else if (strcmp(info->args[0], "tree") == 0) {
        if (info->argCount > 1) {
            print_tree(info->args[1], 0);
        } else {
            print_tree(".", 0);
        }
        return 1;
    } else if (strcmp(info->args[0], "grep") == 0) {
        return shellGrep(info);
    }
    
    return 0; 
}

int executePipedCommand(parseInfo* info) {
    int pipefd[2];
    pid_t pid1, pid2;
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 0;
    }
    
    pid1 = fork();
    if (pid1 == 0) {
        //First child process - extubation
        close(pipefd[0]);  
        dup2(pipefd[1], STDOUT_FILENO);  //Redirecting the output to a pipe
        close(pipefd[1]);
        
        execvp(info->args[0], info->args);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    
    pid2 = fork();
    if (pid2 == 0) {
        //Second child process - calling from the pipe
        close(pipefd[1]);  
        dup2(pipefd[0], STDIN_FILENO);  //Redirecting the output to a pipe
        close(pipefd[0]);
        
        execvp(info->pipeArgs[0], info->pipeArgs);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    
    //The parent process
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1);
    waitpid(pid2);
    
    return 1;
}

//Executing a regular external command
int executeExternalCommand(parseInfo* info) {
    // Checking if it is grep as an external command and we have the built-in functionality
    if (strcmp(info->args[0], "grep") == 0) {
        return shellGrep(info);
    }
    
    if (info->hasRedirection && info->outputFile != NULL) {
        //Redirecting output to a file
        int fd = open(info->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    
    execvp(info->args[0], info->args);
    perror("execvp");
    exit(EXIT_FAILURE);
}

//Implementing built-in commands

int shellExit(parseInfo* info) {
    exit(EXIT_SUCCESS);  
    return 1;  
}

int shellCd(parseInfo* info) {
    if (info->argCount < 2) {
        chdir(getenv("HOME"));
    } else if (chdir(info->args[1]) != 0) {
        perror("cd");
    }
    return 1;
}

int shellPwd(parseInfo* info) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("getcwd");
    }
    return 1;
}

int shellClear(parseInfo* info) {
    printf("\033[H\033[J");  // ANSI code for screen cleaningt
    return 1;
}

// הוסף פונקציה חדשה להרחבת תבניות wildcard
char** expandWildcards(char* pattern, int* count) {
    *count = 0;
    
    // פתיחת הספרייה הנוכחית
    DIR* dir = opendir(".");
    if (!dir) {
        perror("opendir");
        return NULL;
    }
    
    // מספר מקצימלי של התאמות בהתחלה
    int capacity = 10;
    char** matches = malloc(capacity * sizeof(char*));
    if (!matches) {
        closedir(dir);
        return NULL;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // בדיקה אם השם מתאים לתבנית
        if (wildcardMatch(entry->d_name, pattern)) {
            // הגדלת המערך במידת הצורך
            if (*count >= capacity) {
                capacity *= 2;
                char** new_matches = realloc(matches, capacity * sizeof(char*));
                if (!new_matches) {
                    // שחרור הזיכרון במקרה של כישלון
                    for (int i = 0; i < *count; i++) {
                        free(matches[i]);
                    }
                    free(matches);
                    closedir(dir);
                    *count = 0;
                    return NULL;
                }
                matches = new_matches;
            }
            
            // הוספת השם המתאים למערך
            matches[*count] = strdup(entry->d_name);
            (*count)++;
        }
    }
    
    closedir(dir);
    
    // אם אין התאמות, החזר את התבנית המקורית
    if (*count == 0) {
        matches[0] = strdup(pattern);
        *count = 1;
    }
    
    return matches;
}

// פונקציה עזר לבדיקת התאמה של תבנית wildcard
int wildcardMatch(const char* string, const char* pattern) {
    // אם הדפוס הגיע לסופו, אנחנו מצפים שגם המחרוזת הגיעה לסופה
    if (*pattern == '\0') {
        return *string == '\0';
    }
    
    // אם מצאנו כוכבית בדפוס
    if (*pattern == '*') {
        // קידום מעבר לכוכבית
        pattern++;
        
        // הכוכבית יכולה להתאים לכל אורך מחרוזת, כולל 0
        // לכן אנחנו מנסים להתאים את שאר הדפוס לשאר המחרוזת
        // באופן רקורסיבי עבור כל אורך אפשרי
        while (*string) {
            if (wildcardMatch(string, pattern)) {
                return 1;
            }
            string++;
        }
        
        // אם לא מצאנו התאמה בשום אורך, בדוק אם שאר הדפוס יכול להתאים למחרוזת ריקה
        return wildcardMatch(string, pattern);
    }
    
    // אם מצאנו סימן שאלה בדפוס או שהתו הנוכחי בדפוס מתאים לתו הנוכחי במחרוזת
    if (*pattern == '?' || *pattern == *string) {
        // קידום בשני הצדדים והמשך ההתאמה
        return wildcardMatch(string + 1, pattern + 1);
    }
    
    // אין התאמה
    return 0;
}

// פונקציה המעדכנת את shellGrep לתמיכה בwildcards
int shellGrep(parseInfo* info) {
    //Checking a minimum number of parameters
    if (info->argCount < 3) { // Changed from 2 to 3 because we need at least pattern and filename
        printf("Usage: grep [options] pattern filename\n");
        return 1;
    }
    
    int count_only = 0;  //Checks whether to print only a few lines
    int current_arg = 1;
    
    // Check if the -c option exists
    if (strcmp(info->args[current_arg], "-c") == 0) {
        count_only = 1;
        current_arg++;
        
        //Checking that there are enough arguments after the option
        if (current_arg + 1 >= info->argCount) {
            printf("Usage: grep -c pattern filename\n");
            return 1;
        }
    }
    
    // Get the pattern
    char pattern[1024] = "";
    
    // חיפוש אחר ה-wildcard
    int pattern_args = 1;  // מספר הארגומנטים ששייכים לדפוס החיפוש
    int has_wildcard = 0;  // האם יש wildcard בארגומנטים
    int wildcard_arg = 0;  // באיזה ארגומנט נמצא ה-wildcard
    
    // בדיקה האם יש wildcard באחד הארגומנטים
    for (int i = current_arg; i < info->argCount; i++) {
        if (strchr(info->args[i], '*') != NULL || strchr(info->args[i], '?') != NULL) {
            has_wildcard = 1;
            wildcard_arg = i;
            break;
        }
    }
    
    // אם יש wildcard, אז אנחנו מניחים שהדפוס הוא הארגומנט לפני ה-wildcard
    if (has_wildcard) {
        if (wildcard_arg == current_arg) {
            // אם ה-wildcard הוא בארגומנט הראשון, זה מצב לא תקין
            printf("Error: pattern must come before filename wildcards\n");
            return 1;
        }
        
        // שמירת הדפוס - כל מה שבין הדגל והwildcard
        for (int i = current_arg; i < wildcard_arg; i++) {
            if (i > current_arg) {
                strcat(pattern, " ");
            }
            strcat(pattern, info->args[i]);
        }
        
        // הרחבת הwildcard
        int file_count = 0;
        char** files = expandWildcards(info->args[wildcard_arg], &file_count);
        
        if (files == NULL || file_count == 0) {
            printf("No matching files found\n");
            return 1;
        }
        
        // ביצוע grep על כל קובץ שתואם
        int total_matches = 0;
        for (int i = 0; i < file_count; i++) {
            FILE* file = fopen(files[i], "r");
            if (file == NULL) {
                printf("Failed to open file: %s\n", files[i]);
                continue;
            }
            
            char line[1024];
            int file_matches = 0;
            
            while (fgets(line, sizeof(line), file) != NULL) {
                size_t len = strlen(line);
                if (len > 0 && line[len - 1] == '\n') {
                    line[len - 1] = '\0';
                }
                
                if (strstr(line, pattern) != NULL) {
                    file_matches++;
                    
                    if (!count_only) {
                        printf("%s: %s\n", files[i], line);
                    }
                }
            }
            
            total_matches += file_matches;
            fclose(file);
        }
        
        if (count_only) {
            printf("%d\n", total_matches);
        }
        
        // שחרור זיכרון
        for (int i = 0; i < file_count; i++) {
            free(files[i]);
        }
        free(files);
        
        return 1;
    } else {
        // מקרה רגיל ללא wildcards
        // The logic for handling multiple words as a pattern
        strcpy(pattern, "");  // Reset pattern
        char* filename = NULL;
        
        // Case 1: Last argument is the filename, everything before is the pattern
        if (info->argCount - current_arg > 1) {
            // Multiple arguments mode - combine all arguments except the last one into the pattern
            for (int i = current_arg; i < info->argCount - 1; i++) {
                if (i > current_arg) {
                    strcat(pattern, " "); // Add space between words
                }
                strcat(pattern, info->args[i]);
            }
            filename = info->args[info->argCount - 1];
        } else {
            // Simple case - just one pattern and one filename
            strcpy(pattern, info->args[current_arg]);
            current_arg++;
            filename = info->args[current_arg];
        }
        
        FILE* file = fopen(filename, "r");
        if (file == NULL) {
            printf("Failed to open file: %s\n", filename);
            perror("fopen");
            return 1;
        }
        
        // search in file
        char line[1024];
        int match_count = 0;
        
        while (fgets(line, sizeof(line), file) != NULL) {
            //Remove newline character if present
            size_t len = strlen(line);
            if (len > 0 && line[len - 1] == '\n') {
                line[len - 1] = '\0';
                len--;
            }
            
            //Checking whether the line contains the pattern
            if (strstr(line, pattern) != NULL) {
                match_count++;
                
                // Print a line if there is no request for counting
                if (!count_only) {
                    printf("%s\n", line);
                }
            }
        }
        
        // If only a count is requested, print the number of matching lines
        if (count_only) {
            printf("%d\n", match_count);
        }
        
        fclose(file);
        return 1;
    }
}

void print_tree(const char* path, int level) {
    DIR* dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        for (int i = 0; i < level; i++) {
            printf("    ");
        }
        
        printf("|-- %s\n", entry->d_name);
        
        // If it's a directory, read it recursively
        if (entry->d_type == DT_DIR) {
            char subpath[1024];
            snprintf(subpath, sizeof(subpath), "%s/%s", path, entry->d_name);
            print_tree(subpath, level + 1);
        }
    }
    
    closedir(dir);
}