#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h>

#include "history.h"
#include "parser.h"

int execute(char *command, int in_background, char *redirection_filename, int fdout);

void SIGQUIT_handler(int);

void init_history(struct t_hiscbuf *his);

void init_signal_handlers();

void log_parse_execute(struct t_hiscbuf *his, char *line, char *redirection_filename, bool bg_execution);

struct t_hiscbuf *global_his_access;

int main(int argc, char **argv) {
    // Call signal handlers initialization
    init_signal_handlers();

    // Make history cyclic buffer structure and call its initialization
    struct t_hiscbuf his;
    init_history(&his);

    // Handle script file loading and pre-parsing i.e. script_omit_shell_path
    char *script_content = NULL; // Variable to store pointer to content of script file
    const __off_t script_size = script_read(argc, argv, &script_content); // Read file and store script file size

    char *script_cmd_start = script_omit_shell_path(script_content,
                                                    script_size); // Pointer to first char of first command
    const __off_t script_cmd_size = script_cmd_start ? script_size - (script_cmd_start - script_content)
                                                     : 0; // Size of commands

    // If script file has only shell path, i.e "#! ..."
    if (script_size > 0 && script_cmd_size == 0)
        return 0;

    // Declare redirection filename buffer and background execution flag
    char redirection_filename[255];
    bool bg_execution;

    // Make FIFO queue
    mkfifo(FIFO_QUEUE_NAME, (O_CREAT | O_TRUNC) & ~0666);

    if (script_cmd_size > 0) {
        char *pch;
        char *rest = script_cmd_start;

        while ((pch = strtok_r(rest, "\r\n", &rest))) {
            // Ensure that after every command execution redirection is empty
            redirection_filename[0] = '\0';

            PRINT_PROMT;

            // Write current command to STDOUT
            if (write(STDOUT_FILENO, pch, strlen(pch)) < 0) {
                perror("write");
                exit(1);
            }

            if (write(STDOUT_FILENO, "\n", 1) < 0) {
                perror("write");
                exit(1);
            }

            log_parse_execute(&his, pch, redirection_filename, bg_execution);
        }

        free(script_content);
    } else {
        // Declare command line buffer
        char line[MAX_LINE_BUFF];

        // Declare Command line length
        ssize_t line_length = 0;
        while (1) {
            // Ensure that after every command execution redirection is empty
            redirection_filename[0] = '\0';
            PRINT_PROMT;

            // Read user input from STDIN to line and save number of bytes read i.e. length of line
            line_length = read(STDIN_FILENO, line, MAX_LINE_BUFF);

            // If line_length == 0 EOF was captured because read(STDIN) returned 0 which means that stdin is closed
            if (line_length == 0)
                break;

            // Add null terminator to the end of line
            line[line_length] = '\0';

            if (line[line_length - 1] == '\n')
                line[--line_length] = '\0';

            log_parse_execute(&his, line, redirection_filename, bg_execution);
        }
    }

    hiscbuf_free(&his);

    return 0;
}

void init_signal_handlers() {
    // Add SIGQUIT signal handler and check for errors
    if (signal(SIGQUIT, SIGQUIT_handler) == SIG_ERR) {
        char *error = "SIGQUIT handler change error!\n";

        if (write(STDERR_FILENO, error, strlen(error)) < 0) {
            perror("write");
            exit(1);
        }

        exit(2);
    }
}

void init_history(struct t_hiscbuf *his) {
    // Get and store history directory
    const char *const historydir = get_historydir();

    // Initialize history cyclic buffer
    hiscbuf_init(his, historydir);

    // Store global pointer in order to have access to history cyclic buffer from SIGQUIT handler
    global_his_access = his;

    // Read/Load content of his.path to buffer
    hiscbuf_read(his);
}

void SIGQUIT_handler(int sig) {
    signal(sig, SIG_IGN);

    char *header = "\nCommand history:\n";
    if (write(STDOUT_FILENO, header, strlen(header)) < 0) {
        perror("write");
        exit(1);
    }



    // Refresh history cyclic buffer
    hiscbuf_read(global_his_access);

    // Write history cyclic buffer to STDOUT
    hiscbuf_write(global_his_access, STDOUT_FILENO, true);

    exit(3);
}

void log_parse_execute(struct t_hiscbuf *his, char *line, char *redirection_filename, bool bg_execution) {
    // If line is not empty
    if (line) {
        // Reload history before write (necessary when more than one Shell is running)
        hiscbuf_read(his);

        // Add current command to history
        hiscbuf_alloc_insert(his, line, true);

        // Parse >> with filename and &
        bg_execution = parse_special_tokens(line, redirection_filename);

        execute(line, bg_execution, strlen(redirection_filename) > 0 ? redirection_filename : NULL, 0);
    }
}

#define FORK_ERROR case -1
#define IN_CHILD case 0
#define IN_PARRENT default

int execute(char *command, int in_background, char *redirection_filename, int fdout) {
    char *cmd_ptr, *args[256];
    cmd_ptr = parse(command, args);

    int tab[2], fs_stdout;

    if (pipe(tab) < 0) {
        perror("pipe");
        exit(1);
    }

    int pid = fork();

    switch (pid) {
        FORK_ERROR:
            perror("fork error");
            exit(1);

        IN_CHILD:
            if (cmd_ptr) {
                dup2(tab[1], STDOUT_FILENO);
                close(tab[0]);
            } else {
                if (redirection_filename)
                    fs_stdout = open(redirection_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                else {
                    if (in_background == 0)
                        fs_stdout = open(FIFO_QUEUE_NAME, O_WRONLY); // Write to FIFO
#if BACKGROUND_TASK_PRINTS_TO_STDOUT == 0
                    else
                        fs_stdout = tab[1]; // Write to nowhere
#endif
                }
                dup2(fs_stdout, STDOUT_FILENO);
            }

            dup2(fdout, STDIN_FILENO);

            if (execvp(args[0], args) == -1) {
                if (write(STDOUT_FILENO, args[0], strlen(args[0])) < 0) {
                    perror("write");
                    exit(1);
                }

                char *error = ": command not found\n";
                if (write(STDOUT_FILENO, error, strlen(error)) < 0) {
                    perror("write");
                    exit(1);
                }

                exit(1);
            }

        IN_PARRENT:
            if (!in_background)
                waitpid(pid, NULL, 0); // Wait for child process execution ending
            else
                printf("## Created background process with PID: %d\n", pid);

            if (cmd_ptr) {
                close(tab[1]);
                execute(cmd_ptr, in_background, redirection_filename, tab[0]);
            }

            break;
    }
}

