#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <spawn.h>

#define N 12
#define MAX_LINE_LENGTH 256
#define MAX_ARGS 21

extern char **environ;

const char *allowed[N] = {"cp", "touch", "mkdir", "ls", "pwd", "cat", "grep", "chmod", "diff", "cd", "exit", "help"};

int isAllowed(const char *cmd) {
    // Check if command is in the allowed list
    for (int i = 0; i < N; i++) {
        if (strcmp(cmd, allowed[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void executeCommand(char **argv) {
    pid_t pid;
    int status;

    // Attempt to spawn a new process for external commands
    status = posix_spawnp(&pid, argv[0], NULL, NULL, argv, environ);
    if (status == 0) {
        waitpid(pid, &status, 0);
    } else {
        // Print an error message if spawning fails
        fprintf(stderr, "Error executing command: %s\n", argv[0]);
    }
}

int main() {
    char line[MAX_LINE_LENGTH];
    char *argv[MAX_ARGS] = {NULL};

    while (1) {
        fprintf(stderr, "rsh> ");
        fflush(stderr);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            break; // Exit on EOF
        }

        // Remove newline character from the input
        line[strcspn(line, "\n")] = 0;

        // Skip empty lines
        if (strlen(line) == 0) {
            continue;
        }

        // Parse input into arguments
        int argc = 0;
        char *token = strtok(line, " \t");
        while (token != NULL && argc < MAX_ARGS - 1) {
            argv[argc++] = token;
            token = strtok(NULL, " \t");
        }
        argv[argc] = NULL;

        // Continue if no command is entered
        if (argc == 0) continue;

        // Check if command is allowed
        if (!isAllowed(argv[0])) {
            printf("NOT ALLOWED!\n");
            continue;
        }

        // Handle built-in commands
        if (strcmp(argv[0], "exit") == 0) {
            return 0; // Exit the shell
        } 
        else if (strcmp(argv[0], "help") == 0) {
            // Display allowed commands
            printf("The allowed commands are:\n");
            for (int i = 0; i < N; i++) {
                printf("%s\n", allowed[i]);
            }
        } 
        else if (strcmp(argv[0], "cd") == 0) {
            // Handle the 'cd' command
            if (argc > 2) {
                fprintf(stderr, "-rsh: cd: too many arguments\n");
            } else if (argc == 2) {
                // Attempt to change directory
                if (chdir(argv[1]) != 0) {
                    perror("cd");
                }
            } else {
                // Change to home directory if no argument is provided
                const char *home = getenv("HOME");
                if (home && chdir(home) != 0) {
                    perror("cd");
                }
            }
        } 
        else {
            // Execute allowed external commands
            executeCommand(argv);
        }
    }

    return 0;
}
