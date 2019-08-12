#define _POSIX_SOURCE
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#ifndef TOKEN_BUFFER
    #define TOKEN_BUFFER 1024
#endif

// possibly change return type
void executeExternal(char** args, FILE* inputStream, FILE* outputStream){
    printf("execute external\n");
}

int doCommand(char** args, FILE* inputStream, FILE* outputStream){
    
    if (strcmp(args[0], "cd") == 0){
        printf("doCommand(): change directory\n");
    }
    else if (strcmp(args[0], "history") == 0){
        printf("doCommand(): history\n");
    }
    else if (strcmp(args[0], "exit") == 0){
        printf("doCommand(): exit\n");
    }
    else{
        executeExternal(args, inputStream, outputStream);
    }

    return -1;
}