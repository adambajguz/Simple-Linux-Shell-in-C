//
// Created by Admin on 2018-04-03.
//

#ifndef SHELL_PARSER_H
#define SHELL_PARSER_H

#include <string.h>
#include <stdbool.h>

#include "config.h"

bool parse_special_tokens(char *line, char *redirection_filename);

char *parse(char *line, char *args[]);

#endif //SHELL_PARSER_H
