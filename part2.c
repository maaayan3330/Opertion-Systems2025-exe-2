// Maayan Ifergan 212437453
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>


int checkInput(int numOfArgu, char *arguments[]) {
    // to check we got 4 >= argu 
    if (numOfArgu <= 4) {
        fprintf(stderr, "Usage: %s <message1> <message2> ... <count>", arguments[0]);
        return 1;
    }
    return 0;
}

char *keepMessage(char *arguments[], int index) {
    // keep the data that pass in the comeend line- make place
    char *message = (char *)malloc(strlen(arguments[index]) + 1);
    // memory check
    if (message == NULL) {
        perror("Memory allocation failed");
        return NULL;
    }
    strcpy(message, arguments[index]); 
    return message;
}

void write_message(const char *message, int count) {
    for (int i = 0; i < count; i++) {
        printf("%s\n", message);
        usleep((rand() % 100) * 1000); // Random delay between 0 and 99 milliseconds
    }
}

int acquire_lock(const char *lockfile) {
    while (open(lockfile, O_CREAT | O_EXCL, 0644) == -1) {
        if (errno != EEXIST) {
            perror("Failed to acquire lock");
            return -1;
        }
        usleep(1000); // Wait before retrying
    }
    return 0;
}

void release_lock(const char *lockfile) {
    if (unlink(lockfile) == -1) {
        perror("Failed to release lock");
    }
}

int main(int argc, char *argv[]) {

    // check input
    if (checkInput(argc, argv)) {
        return 1; 
    }

    // get the count
    int count = atoi(argv[argc - 1]); 

    // keep the all messages - create a array of strings
    int numMessages = argc - 2;
    char **messages = (char **)malloc(numMessages * sizeof(char *));
    if (messages == NULL) {
        perror("Memory allocation");
        return 1;
    }

    // keep the messges
    for (int i = 1; i <= numMessages; i++) {
        messages[i - 1] = keepMessage(argv, i);
        if (messages[i - 1] == NULL) {
            // free memory
            for (int j = 0; j < i - 1; j++) {
                free(messages[j]);
            }
            free(messages);
            return 1;
        }
    }

    // start with the lock file 
    const char *lockfile = "lockfile.lock";

    // Create output file
    FILE *output = fopen("output2.txt", "w");
    if (output == NULL) {
        perror("Failed to open output file");
        for (int i = 0; i < numMessages; i++) {
            free(messages[i]);
        }
        free(messages);
        return 1;
    }

    // Fork child processes
    pid_t *pids = malloc(numMessages * sizeof(pid_t));
    if (pids == NULL) {
        perror("malloc");
        fclose(output);
        for (int i = 0; i < numMessages; i++) {
            free(messages[i]);
        }
        free(messages);
        return 1;
    }

    for (int i = 0; i < numMessages; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            free(pids);
            fclose(output);
            for (int j = 0; j < numMessages; j++) {
                free(messages[j]);
            }
            free(messages);
            return 1;
        }

        if (pids[i] == 0) {
            // Child process
            srand(getpid());
            for (int j = 0; j < count; j++) {
                if (acquire_lock(lockfile) == -1) {
                    exit(1);
                }

                fprintf(output, "%s\n", messages[i]);
                fflush(output);

                release_lock(lockfile);
            }
            for (int j = 0; j < numMessages; j++) {
                free(messages[j]);
            }
            free(messages);
            fclose(output);
            exit(0);
        }
    }

    // Parent process
    for (int i = 0; i < numMessages; i++) {
        waitpid(pids[i], NULL, 0);
    }

    // Cleanup
    free(pids);
    fclose(output);
    for (int i = 0; i < numMessages; i++) {
        free(messages[i]);
    }
    free(messages);
}