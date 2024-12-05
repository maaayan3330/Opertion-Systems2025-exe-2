// Maayan Ifergan 212437453
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // to check we got 4 argu 
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <parent_message> <child1_message> <child2_message> <count>", argv[0]);
        return 1;
    }

    // keep the data that pass in the comeend line- make place
    char *parent_message = (char *)malloc(strlen(argv[1]) + 1); 
    char *child1_message= (char *)malloc(strlen(argv[2]) + 1);
    char *child2_message = (char *)malloc(strlen(argv[3]) + 1);
    int count = atoi(argv[4]);

    // memory check
    if (parent_message == NULL || child1_message == NULL || child2_message == NULL) {
        perror("memory faild");
        return 1;
    }

    // copy the data
    strcpy(parent_message, argv[1]);
    strcpy(child1_message, argv[2]);
    strcpy(child2_message, argv[3]);




    
    free(parent_message);
    free(child1_message);
    free(child2_message);

    return 0;

}