// maayan ifergan 212437453
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

// write function - that been provieded
void write_message(const char *message, int count) {
    for (int i = 0; i < count; i++) {
        printf("%s\n", message);
        usleep((rand() % 100) * 1000); // Random delay between 0 and 99 milliseconds
    }
}

// function to ask for the lockfile
int acquire_lock(const char *lockfile) {
    while (open(lockfile, O_CREAT | O_EXCL, 0644) == -1) {
        if (errno != EEXIST) {
            perror("Failed to acquire lock");
            return -1;
        }
        // wait for another try
        usleep(100000); 
    }
    return 0;
}

// to let go the lockfile
void release_lock(const char *lockfile) {
    if (unlink(lockfile) == -1) {
        perror("Failed to release lock");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    // consition
    if (argc <= 4) {
    fprintf(stderr, "Usage: %s <message1> <message2> ... <count>", argv[0]);
    return 1;
    }

    // keep the num for the num of messeges
    int count = atoi(argv[argc - 1]); 
    // without the count and the first
    int numMessages = argc - 2; 
    // name of file
    const char *lockfile = "lockfile.lock"; 

    // keep the all difrrent messeges
    char **messages = (char **)malloc(numMessages * sizeof(char *));
    if (messages == NULL) {
        perror("Memory allocation");
        return 1;
    }

    // coopy the messges
    for (int i = 0; i < numMessages; i++) {
        messages[i] = strdup(argv[i + 1]);
        if (messages[i] == NULL) {
            perror("Memory allocation");
            for (int j = 0; j < i; j++) {
                free(messages[j]);
            }
            free(messages);
            return 1;
        }
    }

    // create forks
    pid_t *pids = malloc(numMessages * sizeof(pid_t));
    if (pids == NULL) {
        perror("Memory allocation");
        for (int i = 0; i < numMessages; i++) {
            free(messages[i]);
        }
        free(messages);
        return 1;
    }

    for (int i = 0; i < numMessages; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("Fork failed");
            for (int j = 0; j < numMessages; j++) {
                free(messages[j]);
            }
            free(messages);
            free(pids);
            return 1;
        }

        if (pids[i] == 0) {
            // child process
            srand(getpid());
            for (int j = 0; j < count; j++) {
                if (acquire_lock(lockfile) == -1) {
                    exit(1);
                }
                // write the messeges
                write_message(messages[i], 1);
                // free the lock
                release_lock(lockfile); 
            }

            for (int j = 0; j < numMessages; j++) {
                free(messages[j]);
            }
            free(messages);
            free(pids);
            exit(0);
        }
    }

    // the parent wait
    for (int i = 0; i < numMessages; i++) {
        waitpid(pids[i], NULL, 0);
    }

    // free
    for (int i = 0; i < numMessages; i++) {
        free(messages[i]);
    }
    free(messages);
    free(pids);

    return 0;
}


