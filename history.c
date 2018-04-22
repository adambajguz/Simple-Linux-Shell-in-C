//
// Created by Admin on 2018-04-03.
//

#include "history.h"

#define HISTORY_FILENAME "/history.txt"
#define HISTORY_FILE_LENGTH sizeof(HISTORY_FILENAME)

const char *const get_historydir() {
    struct passwd pwd, *result;
    char *buf;
    int s;

    // Get maximum size of getpwuid_r() and getpwnam_r() data buffers
    long int bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);

    // If there is no hard limit on the size of the buffer needed to store all the groups returned
    if (bufsize == -1)
        bufsize = 16384;

    // Allocate space for buffer
    buf = malloc(bufsize);

    // If malloc error
    if (!buf) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    s = getpwuid_r(getuid(), &pwd, buf, bufsize, &result);

    // If error while getpwuid_r
    if (!result) {
        if (s == 0) {
            char *error = "User not found: getpwuid_r error!\n";

            if (write(STDERR_FILENO, error, strlen(error)) < 0) {
                perror("write");
                exit(1);
            }

        } else {
            errno = s;
            perror("getpwnam_r");
        }

        exit(EXIT_FAILURE);
    }

    // Save pointer to homedir
    const char *const homedir = result->pw_dir;

    // Allocate space to store path to history file, so that we can free buf
    char *const historydir = malloc((strlen(homedir) + HISTORY_FILE_LENGTH) * sizeof(*historydir));

    // If malloc error
    if (!historydir) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // Copy homedir to history dir
    strcpy(historydir, homedir);

    free(buf);

    // Add history filename to history dir
    strcat(historydir, HISTORY_FILENAME);

    return historydir;
}

// history cyclic buffer <=> hiscbuf

void hiscbuf_init(struct t_hiscbuf *buf, const char *const path) {
    buf->path = path;

    for (unsigned int i = 0; i < HISCBUF_MAX_SIZE; ++i)
        buf->data[i] = NULL;

    buf->head = 0;
}

void hiscbuf_from_lines(struct t_hiscbuf *his, char *text, const __off_t text_len) {
    if (text_len) {
        char *ptr, *start = text;

        // Replace each '\n' with '\0' and add history entry to cyclic redundancy buffer
        while (ptr = strchr(start, '\n')) {
            *ptr = '\0';
            hiscbuf_alloc_insert(his, start, false);

            // Move start to end of current command entry
            start = ptr + 1;
        }
    }
}

void hiscbuf_free(struct t_hiscbuf *buf) {
    for (unsigned int i = 0; i < HISCBUF_MAX_SIZE; ++i)
        free(buf->data[i]);

    free((void *) buf->path);
}

void hiscbuf_insert(struct t_hiscbuf *buf, char *data, bool write_to_file) {
    // When all 20 command places are occupied
    if (buf->data[buf->head] && buf->head == HISCBUF_MAX_SIZE - 1) {
        // Free the first entry
        free(buf->data[0]);

        // Move all entries one up
        for (unsigned int i = 1; i < HISCBUF_MAX_SIZE; ++i)
            buf->data[i - 1] = buf->data[i];
    }

    // Insert data in first empty place
    buf->data[buf->head] = data;

    // Increase head if last place wasn't already occupied
    if (buf->head + 1 < HISCBUF_MAX_SIZE)
        ++buf->head;

    if (write_to_file) {
        int fd_history_out = open(buf->path, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (fd_history_out < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        hiscbuf_write(buf, fd_history_out, false);

        close(fd_history_out);
    }
}

void hiscbuf_alloc_insert(struct t_hiscbuf *buf, char *data, bool write_to_file) {
    char *tmp = malloc(strlen(data) * sizeof(char *));

    // If malloc error
    if (!tmp) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // Copy data to tmp
    strcpy(tmp, data);

    // Insert tmp to buf
    hiscbuf_insert(buf, tmp, write_to_file);
}

void hiscbuf_read(struct t_hiscbuf *his) {
    char *history_content = NULL;
    const __off_t history_content_size = load_file(his->path, &history_content, true);

    hiscbuf_from_lines(his, history_content, history_content_size);

    free(history_content);
}

void hiscbuf_write(struct t_hiscbuf *buf, int fdout, bool numeration) {
    // For each entry
    for (int i = 0; i < HISCBUF_MAX_SIZE; ++i) {
        // If place is ocupied by entry
        if (buf->data[i]) {
            if (numeration) {
                char int_buffer[8];

                unsigned int len = sprintf(int_buffer, "%02d", i + 1);
                int_buffer[len] = '\0';

                if (write(fdout, "[", 1) < 0 || write(fdout, int_buffer, 2) < 0 || write(fdout, "] ", 2) < 0) {
                    perror("write");
                    exit(1);
                }
            }

            if (write(fdout, buf->data[i], strlen(buf->data[i])) < 0 || write(fdout, "\n", 1) < 0) {
                perror("write");
                exit(1);
            }
        } else
            break;
    }
}