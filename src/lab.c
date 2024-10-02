#include "lab.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <pwd.h>
#include <linux/limits.h>
#include <readline/history.h>

#define MY_ARG_MAX 100

char *get_prompt(const char *env) {
    char *prompt = getenv(env);
    char *result;
    if (prompt == NULL) {
        result = malloc(10);
        if (result != NULL) {
            strcpy(result, "shell> ");
        }
    } else {
        result = strdup(prompt);
    }
    return result;
}

int change_dir(char **dir) {
    // If no directory is specified, get the home directory
    if (dir == NULL || dir[0] == NULL || strcmp(dir[0], "") == 0) {
        const char *home = getenv("HOME");
        if (home == NULL) {
            fprintf(stderr, "Error: Unable to get home directory.\n");
            return -1;
        }
        // Change to the home directory
        if (chdir(home) != 0) {
            perror("chdir failed"); // Print error message if chdir fails
            return -1;
        }
    } else {
        // Change to the specified directory
        if (chdir(dir[0]) != 0) {
            perror("chdir failed"); // Print error message if chdir fails
            return -1;
        }
    }

    return 0; // On success, return 0
}

char **cmd_parse(char const *line) {
    char **argv = malloc(MY_ARG_MAX * sizeof(char*));
    if (argv == NULL) {
        return NULL;
    }

    char *line_copy = strdup(line);
    if (line_copy == NULL) {
        free(argv);
        return NULL;
    }

    char *token;
    int argc = 0;

    token = strtok(line_copy, " ");
    while (token != NULL && argc < MY_ARG_MAX - 1) {
        argv[argc] = strdup(token);
        if (argv[argc] == NULL) {
            // Free previously allocated tokens on failure
            for (int i = 0; i < argc; i++) {
                free(argv[i]);
            }
            free(argv);
            free(line_copy);
            return NULL;
        }
        argc++;
        token = strtok(NULL, " ");
    }

    argv[argc] = NULL;

    free(line_copy);
    return argv;
}

void cmd_free(char ** line) {
     if (line == NULL) return;
    for (int i = 0; line[i] != NULL; i++) {
        free(line[i]);
    }
    free(line);
}

char *trim_white(char *line) {
    char *end;

    while (isspace((unsigned char)*line)) line++;

    end = line + strlen(line) - 1;
    while (end > line && isspace((unsigned char)*end)) end--;

    *(end + 1) = '\0';
    return line;
}

bool do_builtin(struct shell *sh, char **argv) {
    if (argv == NULL || argv[0] == NULL) return false; // Check if argv is NULL

    // Handle the exit command
    if (strcmp(argv[0], "exit") == 0) {
        sh_destroy(sh); // Clean up before exit
        exit(0);
    }
    // Handle the cd command
    else if (strcmp(argv[0], "cd") == 0) {
        if (argv[1] == NULL) {
            fprintf(stderr, "cd: missing argument\n");
            return false; // Handle no argument case
        }
        return change_dir(argv + 1) == 0; // Ensure change_dir handles its own errors
    }
    // Handle the history command
    else if (strcmp(argv[0], "history") == 0) {
        for (int i = 0; i < history_length; i++) {
            printf("%d  %s\n", i + 1, history_get(i)->line);
        }
        return true; // Successfully printed history
    }

    return false; // Not a built-in command
}

void sh_init(struct shell *sh) {
     // Check if the shell is running in an interactive mode
    sh->shell_terminal = STDIN_FILENO; // Standard input
    sh->shell_is_interactive = isatty(sh->shell_terminal);

    if (sh->shell_is_interactive) {
        // Get the shell's process group ID
        sh->shell_pgid = getpid(); 

        // Get the current terminal attributes
        if (tcgetattr(sh->shell_terminal, &sh->shell_tmodes) == -1) {
            perror("tcgetattr");
            exit(EXIT_FAILURE); // Exit if there's an error
        }

        // Optionally set new terminal attributes here
        struct termios new_tmodes = sh->shell_tmodes;
        
        // Apply the new terminal attributes
        if (tcsetattr(sh->shell_terminal, TCSANOW, &new_tmodes) == -1) {
            perror("tcsetattr");
            exit(EXIT_FAILURE); // Exit if there's an error
        }
    }
}

void sh_destroy(struct shell *sh) {
    if (sh->shell_is_interactive) {
        if (tcsetattr(sh->shell_terminal, TCSANOW, &sh->shell_tmodes) == -1) {
            perror("tcsetattr");
        }
    }
}

void parse_args(int argc, char **argv) {
     for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            printf("Shell Version: %d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
            exit(0);
        }
    }
}