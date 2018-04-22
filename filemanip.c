//
// Created by Admin on 2018-04-03.
//

#include "filemanip.h"

__off_t load_file(const char *const name, char **mem, bool ignore_error) {
    int fdin; // Script file descriptor

    // Open Script file in read only mode
    if ((fdin = open(name, O_RDONLY)) < 0) {
        if (ignore_error)
            return 0;
        else {
            // On script file open error - print error & exit
            perror("open");
            exit(EXIT_FAILURE);
        }
    }

    // Get script file size
    struct stat buf;
    fstat(fdin, &buf);
    __off_t fdsize = buf.st_size;

    // Allocate memory equal to file size
    (*mem) = malloc(fdsize + 1);

    // If malloc error
    if (!(*mem)) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    char *fbuf[MAX_LINE_BUFF]; // Script file read buffer
    char *script_content_pos = (*mem); // Pointer to next free space in script_content
    unsigned int fdread, fdread_total = 0;

    // Script file reading
    while ((fdread = read(fdin, fbuf, MAX_LINE_BUFF)) > 0) {
        // Copy fdread bytes of Script file from fbuf to script_content_pos
        memcpy(script_content_pos, fbuf, fdread);

        // Move script_content_pos pointer to next free
        script_content_pos += fdread;

        // Add number of read bytes (fdread) in current iteration to fdread_total
        fdread_total += fdread;
    }
    close(fdin);

    // Simple Script file read verification
    if (fdread_total != fdsize) {
        char *error = "File read error: read and actual sizes does not match!\n";

        if (write(STDERR_FILENO, error, strlen(error)) < 0) {
            perror("write");
            exit(1);
        }

        exit(EXIT_FAILURE);
    }

    // Null terminate script_content
    (*mem)[fdsize] = '\0';

    return fdsize;
}


__off_t script_read(int argc, char *const *argv, char **script_content) {
    // If additional arguments were passed
    if (argc > 1)
        return load_file(argv[1], script_content, false);

    return 0;
}

char *script_omit_shell_path(const char *script_content, const __off_t script_size) {
    char *script_content_start = NULL; // Variable to store pointer to first character in first command

    // If script_content != NULL AND script_size > 0 AND script_content starts with #!
    if (script_content && script_size > 0 && strncmp(script_content, "#!", 2) == 0) {
        // Set script_content_start to first occurance of '\n'
        script_content_start = strchr(script_content, '\n');

        // If script_content_start != NULL
        if (script_content_start)
            ++script_content_start;
    }

    return script_content_start;
}