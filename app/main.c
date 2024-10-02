#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../src/lab.h"

int main(int argc, char *argv[]) {
    int opt;

    // Parse command line arguments
    while ((opt = getopt(argc, argv, "v")) != -1)
    {
        switch (opt)
        {
        case 'v':
            // Print the version and exit
            printf("Shell Version: %d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
            exit(0); // Exit after printing version
        default:
            fprintf(stderr, "Usage: %s [-v]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    char *prompt = get_prompt("MY_PROMPT");

    struct shell my_shell;
    sh_init(&my_shell);

    // Initialize GNU Readline
    char *line;
    using_history();

    // Main loop to read user input
    while ((line = readline(prompt))) {
        if (strlen(line) > 0) {
            add_history(line); // Add line to history only if it's not empty

            // Trim white spaces from the input
            char *trimmed_line = trim_white(line);
            char **args = cmd_parse(trimmed_line);
            if (args == NULL) {
                fprintf(stderr, "Failed to parse command\n");
                free(trimmed_line);
                free(line); // Free line only here
                continue; // Go to the next iteration
            }

            // Check for built-in commands
            if (do_builtin(&my_shell, args)) {
                // If it's a built-in command, handle it and clean up
                cmd_free(args);
                free(trimmed_line);
                // Freeing line should be handled outside this condition
            }

            // Free the input line after processing
            free(line);
            free(trimmed_line);
        }
    }

    // Cleanup before exiting
    free(prompt);
    sh_destroy(&my_shell); // Clean up the shell structure

    return 0;
}