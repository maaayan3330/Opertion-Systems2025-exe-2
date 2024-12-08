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

// function 
int acquire_lock(const char *lockfile) {
    while (open(lockfile, O_CREAT | O_EXCL, 0644) == -1) {
        if (errno != EEXIST) {
            perror("Failed to acquire lock");
            return -1;
        }
        usleep(100000); // ממתין לפני ניסיון נוסף
    }
    return 0;
}

void release_lock(const char *lockfile) {
    if (unlink(lockfile) == -1) {
        perror("Failed to release lock");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <message1> <message2> ... <count>\n", argv[0]);
        return 1;
    }

    int count = atoi(argv[argc - 1]); // מספר החזרות לכל הודעה
    int numMessages = argc - 2; // מספר ההודעות
    const char *lockfile = "lockfile.lock"; // קובץ לניהול נעילה

    // שמירת ההודעות
    char **messages = (char **)malloc(numMessages * sizeof(char *));
    if (messages == NULL) {
        perror("Memory allocation");
        return 1;
    }
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

    // יצירת תהליכים
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
            // תהליך ילד
            srand(getpid()); // איתחול הרנדומליות עבור הילד
            for (int j = 0; j < count; j++) {
                if (acquire_lock(lockfile) == -1) {
                    exit(1);
                }

                // קריאה לפונקציית הכתיבה המקורית
                write_message(messages[i], 1);

                release_lock(lockfile); // שחרור הנעילה
            }

            // ניקוי זיכרון בתהליך הילד
            for (int j = 0; j < numMessages; j++) {
                free(messages[j]);
            }
            free(messages);
            free(pids);
            exit(0);
        }
    }

    // תהליך האב ממתין לילדים
    for (int i = 0; i < numMessages; i++) {
        waitpid(pids[i], NULL, 0);
    }

    // ניקוי זיכרון בתהליך האב
    for (int i = 0; i < numMessages; i++) {
        free(messages[i]);
    }
    free(messages);
    free(pids);

    return 0;
}


