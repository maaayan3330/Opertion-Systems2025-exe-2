// Maayan Ifergan 212437453
#include "buffered_open.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/stat.h>

// Utility to flush the write buffer to the file
static int flush_write_buffer(buffered_file_t *bf) {
    if (bf->write_buffer_pos > 0) {
        if (bf->preappend) {
            struct stat file_stat;
            if (fstat(bf->fd, &file_stat) == -1) {
                return -1;
            }
            
            size_t original_size = file_stat.st_size;
            char *temp_buffer = malloc(original_size);
            if (!temp_buffer) {
                errno = ENOMEM;
                return -1;
            }

            // Read the existing content
            lseek(bf->fd, 0, SEEK_SET);
            read(bf->fd, temp_buffer, original_size);

            // Write new content followed by the original content
            lseek(bf->fd, 0, SEEK_SET);
            if (write(bf->fd, bf->write_buffer, bf->write_buffer_pos) == -1 ||
                write(bf->fd, temp_buffer, original_size) == -1) {
                free(temp_buffer);
                return -1;
            }
            free(temp_buffer);
        } else {
            if (write(bf->fd, bf->write_buffer, bf->write_buffer_pos) == -1) {
                return -1;
            }
        }
        bf->write_buffer_pos = 0;
    }
    return 0;
}

// Open a file with buffering and custom O_PREAPPEND flag
buffered_file_t *buffered_open(const char *pathname, int flags, ...) {
    buffered_file_t *bf = malloc(sizeof(buffered_file_t));
    if (!bf) {
        errno = ENOMEM;
        return NULL;
    }

    memset(bf, 0, sizeof(buffered_file_t));
    bf->read_buffer = malloc(BUFFER_SIZE);
    bf->write_buffer = malloc(BUFFER_SIZE);

    if (!bf->read_buffer || !bf->write_buffer) {
        free(bf->read_buffer);
        free(bf->write_buffer);
        free(bf);
        errno = ENOMEM;
        return NULL;
    }

    bf->read_buffer_size = BUFFER_SIZE;
    bf->write_buffer_size = BUFFER_SIZE;

    // Handle variable arguments for file permissions
    va_list args;
    va_start(args, flags);
    mode_t mode = (flags & O_CREAT) ? va_arg(args, mode_t) : 0;
    va_end(args);

    // Remove O_PREAPPEND for the system call
    bf->preappend = (flags & O_PREAPPEND);
    flags &= ~O_PREAPPEND;

    bf->fd = open(pathname, flags, mode);
    if (bf->fd == -1) {
        free(bf->read_buffer);
        free(bf->write_buffer);
        free(bf);
        return NULL;
    }

    bf->flags = flags;
    return bf;
}

// Buffered write function
ssize_t buffered_write(buffered_file_t *bf, const void *buf, size_t count) {
    if (bf->write_buffer_pos + count > bf->write_buffer_size) {
        if (flush_write_buffer(bf) == -1) {
            return -1;
        }
    }

    if (count > bf->write_buffer_size) {
        // Write directly to the file if the data is larger than the buffer
        if (bf->preappend) {
            struct stat file_stat;
            if (fstat(bf->fd, &file_stat) == -1) {
                return -1;
            }

            size_t original_size = file_stat.st_size;
            char *temp_buffer = malloc(original_size);
            if (!temp_buffer) {
                errno = ENOMEM;
                return -1;
            }

            lseek(bf->fd, 0, SEEK_SET);
            read(bf->fd, temp_buffer, original_size);

            lseek(bf->fd, 0, SEEK_SET);
            if (write(bf->fd, buf, count) == -1 ||
                write(bf->fd, temp_buffer, original_size) == -1) {
                free(temp_buffer);
                return -1;
            }

            free(temp_buffer);
            return count;
        } else {
            return write(bf->fd, buf, count);
        }
    }

    memcpy(bf->write_buffer + bf->write_buffer_pos, buf, count);
    bf->write_buffer_pos += count;
    return count;
}

// Buffered read function
ssize_t buffered_read(buffered_file_t *bf, void *buf, size_t count) {
    if (bf->read_buffer_pos >= bf->read_buffer_size) {
        ssize_t bytes_read = read(bf->fd, bf->read_buffer, bf->read_buffer_size);
        if (bytes_read <= 0) {
            return bytes_read;
        }
        bf->read_buffer_pos = 0;
        bf->read_buffer_size = bytes_read;
    }

    size_t bytes_available = bf->read_buffer_size - bf->read_buffer_pos;
    size_t bytes_to_read = (count < bytes_available) ? count : bytes_available;
    memcpy(buf, bf->read_buffer + bf->read_buffer_pos, bytes_to_read);
    bf->read_buffer_pos += bytes_to_read;
    return bytes_to_read;
}

// Flush function
int buffered_flush(buffered_file_t *bf) {
    return flush_write_buffer(bf);
}

// Close function
int buffered_close(buffered_file_t *bf) {
    if (flush_write_buffer(bf) == -1) {
        return -1;
    }
    int result = close(bf->fd);
    free(bf->read_buffer);
    free(bf->write_buffer);
    free(bf);
    return result;
}
