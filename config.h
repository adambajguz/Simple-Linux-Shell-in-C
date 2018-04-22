//
// Created by Admin on 2018-04-03.
//

#ifndef SHELL_CONFIG_H
#define SHELL_CONFIG_H

#include <stdio.h>

#define MAX_LINE_BUFF BUFSIZ

#define PROMPT "#> "
#define PRINT_PROMT do { \
                            if (write(STDOUT_FILENO, PROMPT, sizeof(PROMPT) - 1) < 0) { \
                                perror("write"); \
                                exit(1); \
                            } \
                        } while(0)

#define FIFO_QUEUE_NAME "ShellQueue"

#define BACKGROUND_TASK_PRINTS_TO_STDOUT 0

#endif //SHELL_CONFIG_H
