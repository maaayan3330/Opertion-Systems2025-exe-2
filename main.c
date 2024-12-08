#include "buffered_open.h"
#include <stdio.h>
#include <string.h>

int main() {
    // Open the file with the custom buffered open function and O_PREAPPEND flag
    buffered_file_t *bf = buffered_open("example2.txt", O_RDWR | O_CREAT | O_PREAPPEND, 0644);
    if (!bf) {
        perror("buffered_open");
        return 1;
    }

    // Write data to the file using the buffered write function
    const char *text1 = "New content at the beginning.\n";
    if (buffered_write(bf, text1, strlen(text1)) == -1) {
        perror("buffered_write");
        buffered_close(bf);
        return 1;
    }

    // Flush the buffer to ensure the written data is saved to the file
    if (buffered_flush(bf) == -1) {
        perror("buffered_flush");
        buffered_close(bf);
        return 1;
    }

    // Reposition the file descriptor to the beginning of the file
    if (lseek(bf->fd, 0, SEEK_SET) == -1) {
        perror("lseek");
        buffered_close(bf);
        return 1;
    }

    // Read data from the file and print it to the console
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = buffered_read(bf, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        perror("buffered_read");
        buffered_close(bf);
        return 1;
    }

    // Null-terminate the read data and print it
    buffer[bytes_read] = '\0';
    printf("File contents:\n%s", buffer);

    // Write additional data to demonstrate O_PREAPPEND functionality
    const char *text2 = "Another line at the start.\n";
    if (buffered_write(bf, text2, strlen(text2)) == -1) {
        perror("buffered_write");
        buffered_close(bf);
        return 1;
    }

    // Close the buffered file, ensuring all data is flushed
    if (buffered_close(bf) == -1) {
        perror("buffered_close");
        return 1;
    }

    return 0;
}
