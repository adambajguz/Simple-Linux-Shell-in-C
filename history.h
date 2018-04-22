//
// Created by Admin on 2018-04-03.
//

#ifndef SHELL_HISTORY_H
#define SHELL_HISTORY_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <pwd.h>
#include <stdbool.h>

#include "filemanip.h"

// history cyclic buffer <=> hiscbuf

const char *const get_historydir();

#define HISCBUF_MAX_SIZE 20

struct t_hiscbuf {
    char *data[HISCBUF_MAX_SIZE];
    const char *path;
    unsigned int head;
};

void hiscbuf_init(struct t_hiscbuf *buf, const char *const path);

void hiscbuf_from_lines(struct t_hiscbuf *his, char *text, const __off_t text_len);

void hiscbuf_free(struct t_hiscbuf *buf);

void hiscbuf_insert(struct t_hiscbuf *buf, char *data, bool write_to_file);

void hiscbuf_alloc_insert(struct t_hiscbuf *buf, char *data, bool write_to_file);

void hiscbuf_read(struct t_hiscbuf *his);

void hiscbuf_write(struct t_hiscbuf *buf, int fdout, bool numeration);

#endif //SHELL_HISTORY_H
