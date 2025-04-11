#include "shell.h"

// קריאת פקודה מהמשתמש
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

// מימוש של פונקציית readline כדי להתאים לקריאה ב-main.c
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

// פירוק הפקודה לארגומנטים
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
    
    char* token;
    char* rest = cmdLine;
    int capacity = 10;
    info->args = malloc(capacity * sizeof(char*));
    
    // בדיקה האם קיים צינור
    char* pipePos = strchr(cmdLine, '|');
    if (pipePos != NULL) {
        info->hasPipe = 1;
        *pipePos = '\0';  // מפריד את החלק הראשון
        char* pipeCmd = pipePos + 1;
        
        // פירוק הפקודה השנייה (אחרי הצינור)
        info->pipeArgs = malloc(capacity * sizeof(char*));
        char* pipeToken;
        char* pipeRest = pipeCmd;
        while ((pipeToken = strtok_r(pipeRest, " \t", &pipeRest))) {
            if (info->pipeArgCount >= capacity) {
                capacity *= 2;
                info->pipeArgs = realloc(info->pipeArgs, capacity * sizeof(char*));
            }
            
            // התעלמות מרווחים מיותרים
            if (strlen(pipeToken) > 0) {
                info->pipeArgs[info->pipeArgCount++] = strdup(pipeToken);
            }
        }
        
        // הוספת NULL בסוף המערך
        if (info->pipeArgCount >= capacity) {
            capacity += 1;
            info->pipeArgs = realloc(info->pipeArgs, capacity * sizeof(char*));
        }
        info->pipeArgs[info->pipeArgCount] = NULL;
    }
    
    // פירוק הפקודה הראשונה
    while ((token = strtok_r(rest, " \t", &rest))) {
        if (strcmp(token, ">") == 0) {
            // הפניית פלט
            info->hasRedirection = 1;
            token = strtok_r(rest, " \t", &rest);
            if (token) {
                info->outputFile = strdup(token);
            }
            break;
        }
        
        if (info->argCount >= capacity) {
            capacity *= 2;
            info->args = realloc(info->args, capacity * sizeof(char*));
        }
        
        // התעלמות מרווחים מיותרים
        if (strlen(token) > 0) {
            info->args[info->argCount++] = strdup(token);
        }
    }
    
    // הוספת NULL בסוף המערך
    if (info->argCount >= capacity) {
        capacity += 1;
        info->args = realloc(info->args, capacity * sizeof(char*));
    }
    info->args[info->argCount] = NULL;
    
    return info;
}

// מימוש הפונקציה parse כדי להתאים לקריאה ב-main.c
parseInfo* parse(char* cmdLine) {
    return parseCommand(cmdLine);
}

// שחרור זיכרון של מבנה parseInfo
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

// ביצוע פקודה מובנית או חיצונית
int executeCommand(parseInfo* info) {
    if (info == NULL || info->argCount == 0) {
        return 0;
    }
    
    // בדיקה האם זו פקודה מובנית
    if (executeBuiltInCommand(info)) {
        return 1;
    }
    
    // בדיקה האם יש צינור
    if (info->hasPipe) {
        return executePipedCommand(info);
    }
    
    // אחרת, זו פקודה חיצונית רגילה
    return executeExternalCommand(info);
}

// ביצוע פקודות מובנות
int executeBuiltInCommand(parseInfo* info) {
    if (strcmp(info->args[0], "exit") == 0) {
        return shellExit(info);
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
    }
    
    return 0;  // לא פקודה מובנית
}

// ביצוע פקודה עם צינור
int executePipedCommand(parseInfo* info) {
    int pipefd[2];
    pid_t pid1, pid2;
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 0;
    }
    
    pid1 = fork();
    if (pid1 == 0) {
        // תהליך ילד ראשון - מוציא לצינור
        close(pipefd[0]);  // סגירת צד הקריאה
        dup2(pipefd[1], STDOUT_FILENO);  // הפניית הפלט לצינור
        close(pipefd[1]);
        
        execvp(info->args[0], info->args);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    
    pid2 = fork();
    if (pid2 == 0) {
        // תהליך ילד שני - קורא מהצינור
        close(pipefd[1]);  // סגירת צד הכתיבה
        dup2(pipefd[0], STDIN_FILENO);  // הפניית הקלט מהצינור
        close(pipefd[0]);
        
        execvp(info->pipeArgs[0], info->pipeArgs);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    
    // תהליך האב
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1);
    waitpid(pid2);
    
    return 1;
}

// ביצוע פקודה חיצונית רגילה
int executeExternalCommand(parseInfo* info) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // תהליך ילד
        if (info->hasRedirection && info->outputFile != NULL) {
            // הפניית פלט לקובץ
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
    } else if (pid < 0) {
        perror("fork");
        return 0;
    } else {
        // תהליך האב
        int status;
        waitpid(pid);
        return 1;
    }
}

// מימוש פקודות מובנות

int shellExit(parseInfo* info) {
    exit(EXIT_SUCCESS);
    return 1;  // לא יגיע לכאן
}

int shellCd(parseInfo* info) {
    if (info->argCount < 2) {
        // ללא פרמטרים, עבור לספריית הבית
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
    printf("\033[H\033[J");  // קוד ANSI לניקוי המסך
    return 1;
}

// מימוש פקודת tree
void print_tree(const char* path, int level) {
    DIR* dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // דילוג על "." ו-".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // הדפסת רווחים לפי רמת העומק
        for (int i = 0; i < level; i++) {
            printf("    ");
        }
        
        printf("|-- %s\n", entry->d_name);
        
        // אם זו ספרייה, קרא אותה רקורסיבית
        if (entry->d_type == DT_DIR) {
            char subpath[1024];
            snprintf(subpath, sizeof(subpath), "%s/%s", path, entry->d_name);
            print_tree(subpath, level + 1);
        }
    }
    
    closedir(dir);
}