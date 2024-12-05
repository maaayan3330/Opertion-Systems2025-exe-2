// Maayan Ifergan 212437453
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>


int checkInput(int numOfArgu, char *arguments[]) {
    // to check we got 4 argu 
    if (numOfArgu != 5) {
        fprintf(stderr, "Usage: %s <parent_message> <child1_message> <child2_message> <count>", arguments[0]);
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

int createFork(char *parentM[], char *chlid1M [], char *child2M[], int count) {
    // create 2 kids
    pid_t pid_child_1, pid_child_2;

    // create the first child
    if((pid_child_1=fork()) < 0) {
        perror("faild fork child 1");
        return 1;
    }

    if(pid_child_1==0) {
        // make sure that the seoned child will create also
        sleep(1);
        // write to the file 
        for (int i = 0; i < count; i++){
            
        }
        

        exit(1);
    }

    // create the seconed child
    if((pid_child_2=fork()) < 0) {
        perror("faild fork child 2");
        return 1;
    }

    if(pid_child_2==0) {
        // make sure that the seoned child will create also
        sleep(1);
        // print

        exit(1);
    }

    // the parent will wait untill the childs will finish



}

int main(int argc, char *argv[]) {
    // check input
    if (checkInput(argc, argv)) {
        return 1; 
    }

    // keep the messeges
    char *parent_message = keepMessage(argv, 1);
    char *child1_message = keepMessage(argv, 2);
    char *child2_message = keepMessage(argv, 3);
    int count = atoi(argv[4]);

    // check it is not failed
    if (parent_message == NULL || child1_message == NULL || child2_message == NULL) {
        free(parent_message);
        free(child1_message);
        free(child2_message);
        return 1;
    }

    // create a file
    FILE *file = fopen("output.txt", "w"); 
    if (file == NULL) {
        perror("Failed to create file"); 
        return 1;
    }

    // close file
    fclose(file);

    // free memory
    free(parent_message);
    free(child1_message);
    free(child2_message);

    return 0;

}