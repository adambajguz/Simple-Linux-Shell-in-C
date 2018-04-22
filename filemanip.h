//
// Created by Admin on 2018-04-03.
//

#ifndef SHELL_FILEMANIP_H
#define SHELL_FILEMANIP_H

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>

#include "config.h"

__off_t load_file(const char *const name, char **mem, bool ignore_error);

__off_t script_read(int argc, char *const *argv, char **script_content);

char *script_omit_shell_path(const char *script_content, const __off_t script_size);

#endif //SHELL_FILEMANIP_H
