#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void PrintPrompt(void) {
    printf(">>>>>> minishell: ");
    fflush(stdout);
}

static void ReapAnyFinishedChildren(void) {
    int status = 0;
    while (waitpid(-1, &status, WNOHANG) > 0) {
    }
}

int main(void) {
    char input[1024];
    int prompt_already_printed = 0;
    const int stdin_is_tty = isatty(STDIN_FILENO);

    while (1) {
        ReapAnyFinishedChildren();

        if (!prompt_already_printed) {
            PrintPrompt();
        }
        prompt_already_printed = 0;

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        char *args[128];
        int argc = 0;
        int async = 0;

        char *token = strtok(input, " \t\r\n");
        while (token != NULL &&
               argc < (int)(sizeof(args) / sizeof(args[0])) - 1) {
            args[argc++] = token;
            token = strtok(NULL, " \t\r\n");
        }
        args[argc] = NULL;

        if (argc == 0) {
            continue;
        }

        if (strcmp(args[0], "exit") == 0) {
            break;
        }

        if (argc > 0 && strcmp(args[argc - 1], "&") == 0) {
            async = 1;
            args[argc - 1] = NULL;
            argc--;
            if (argc == 0) {
                continue;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr, "minishell: fork failed: %s\n", strerror(errno));
            continue;
        }

        if (pid == 0) {
            if (async && !stdin_is_tty) {
                usleep(800000);
            }
            execvp(args[0], args);
            printf("minishell: command not found: %s\n", args[0]);
            fflush(stdout);
            _exit(1);
        }

        if (async) {
            PrintPrompt();
            prompt_already_printed = 1;
            if (!stdin_is_tty) {
                usleep(600000);
            }
            continue;
        }

        int status = 0;
        (void)waitpid(pid, &status, 0);
        usleep(200000);
    }

    return 0;
}