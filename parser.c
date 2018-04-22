//
// Created by Admin on 2018-04-03.
//

#include "parser.h"

bool parse_special_tokens(char *line, char *redirection_filename) {
    char line_copy[MAX_LINE_BUFF];
    // Make copy of line to line_copy
    strcpy(line_copy, line);

    char *token, *rest = line_copy;

    // Check for >> (and extract redirection file path) and &
    while ((token = strtok_r(rest, " ", &rest))) {
        if (strcmp(token, ">>") == 0) {
            token = strtok_r(rest, " ", &rest);
            strncpy(redirection_filename, token, 255);
        } else if (*token == '&')
            return true;
    }

    return false;
}


char *parse(char *line, char *args[]) {
    char *token, *rest = line;

    unsigned int position = 0;
    while ((token = strtok_r(rest, " ", &rest))) {

        // Detect output redirection or background execution
        if (strcmp(token, ">>") == 0 || strcmp(token, "&") == 0) {
            args[position] = NULL;

            // If >> or & was detected it means that the line has ended, so return from function
            return NULL;
        }

        // Detect pipe
        if (strcmp(token, "|") == 0) {
            args[position] = NULL;
            return rest;
        } else {
            args[position] = token;
            position++;
        }
    }

    args[position] = NULL;

    return NULL;
}