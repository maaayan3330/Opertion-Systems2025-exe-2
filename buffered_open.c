#include "buffered_open.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

// Flushes the write buffer to the file
int buffered_flush(buffered_file_t *bf) {
    if (!bf) return -1;

    if (bf->write_buffer_pos > 0) {
        if (bf->preappend) {
            // Handle O_PREAPPEND: Read existing content, prepend new data, and write back
            off_t file_size = lseek(bf->fd, 0, SEEK_END);
            if (file_size == -1) return -1;

            char *temp_buffer = (char *)malloc(file_size);
            if (!temp_buffer) return -1;

            if (lseek(bf->fd, 0, SEEK_SET) == -1) {
                free(temp_buffer);
                return -1;
            }

            if (read(bf->fd, temp_buffer, file_size) == -1) {
                free(temp_buffer);
                return -1;
            }

            if (lseek(bf->fd, 0, SEEK_SET) == -1) {
                free(temp_buffer);
                return -1;
            }

            if (write(bf->fd, bf->write_buffer, bf->write_buffer_pos) == -1) {
                free(temp_buffer);
                return -1;
            }

            if (write(bf->fd, temp_buffer, file_size) == -1) {
                free(temp_buffer);
                return -1;
            }

            free(temp_buffer);
        } else {
            if (write(bf->fd, bf->write_buffer, bf->write_buffer_pos) == -1) return -1;
        }

        bf->write_buffer_pos = 0;
    }

    return 0;
}

// Closes the buffered file
int buffered_close(buffered_file_t *bf) {
    if (!bf) return -1;

    if (buffered_flush(bf) == -1) return -1;

    free(bf->read_buffer);
    free(bf->write_buffer);

    int result = close(bf->fd);
    free(bf);
    return result;
}

// Reads from the buffered file
ssize_t buffered_read(buffered_file_t *bf, void *buf, size_t count) {
    if (!bf || !buf) return -1;

    size_t bytes_read = 0;
    char *output = (char *)buf;

    while (bytes_read < count) {
        if (bf->read_buffer_pos == 0 || bf->read_buffer_pos == bf->read_buffer_size) {
            bf->read_buffer_size = read(bf->fd, bf->read_buffer, BUFFER_SIZE);
            if (bf->read_buffer_size == -1) return -1;
            if (bf->read_buffer_size == 0) break; // EOF
            bf->read_buffer_pos = 0;
        }

        size_t to_copy = bf->read_buffer_size - bf->read_buffer_pos;
        if (to_copy > count - bytes_read) to_copy = count - bytes_read;

        memcpy(output + bytes_read, bf->read_buffer + bf->read_buffer_pos, to_copy);
        bf->read_buffer_pos += to_copy;
        bytes_read += to_copy;
    }

    return bytes_read;
}

// Writes to the buffered file
ssize_t buffered_write(buffered_file_t *bf, const void *buf, size_t count) {
    if (!bf || !buf) return -1;

    size_t bytes_written = 0;
    const char *input = (const char *)buf;

    while (bytes_written < count) {
        size_t space_left = BUFFER_SIZE - bf->write_buffer_pos;
        if (space_left == 0) {
            if (buffered_flush(bf) == -1) return -1;
            space_left = BUFFER_SIZE;
        }

        size_t to_copy = count - bytes_written;
        if (to_copy > space_left) to_copy = space_left;

        memcpy(bf->write_buffer + bf->write_buffer_pos, input + bytes_written, to_copy);
        bf->write_buffer_pos += to_copy;
        bytes_written += to_copy;
    }

    return bytes_written;
}

// Opens a buffered file
buffered_file_t *buffered_open(const char *pathname, int flags, ...) {
    int mode = 0;
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, int);
        va_end(args);
    }

    buffered_file_t *bf = (buffered_file_t *)malloc(sizeof(buffered_file_t));
    if (!bf) return NULL;

    bf->preappend = flags & O_PREAPPEND;
    flags &= ~O_PREAPPEND;

    bf->fd = open(pathname, flags, mode);
    if (bf->fd == -1) {
        free(bf);
        return NULL;
    }

    bf->read_buffer = (char *)malloc(BUFFER_SIZE);
    bf->write_buffer = (char *)malloc(BUFFER_SIZE);

    if (!bf->read_buffer || !bf->write_buffer) {
        free(bf->read_buffer);
        free(bf->write_buffer);
        free(bf);
        return NULL;
    }

    bf->read_buffer_size = 0;
    bf->write_buffer_size = 0;
    bf->read_buffer_pos = 0;
    bf->write_buffer_pos = 0;
    bf->flags = flags;

    return bf;
}
