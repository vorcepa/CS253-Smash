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

pid_t executeExternal(char** args, int inputFD, int outputFD){
    fflush(stdout);
    pid_t processID = fork();
    
    if (processID != 0){
        return processID;
    }

    if (inputFD != STDIN_FILENO){
        dup2(inputFD, STDIN_FILENO);
        close(inputFD);
    }
    if (outputFD != STDOUT_FILENO){
        dup2(outputFD, STDOUT_FILENO);
        close(outputFD);
    }
    
    execvp(args[0], args);

    exit(0);
}

int doCommand(char** args, int inputFD, int outputFD){
    
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
        pid_t processID;
        int status;
        processID = executeExternal(args, inputFD, outputFD);
        waitpid(processID, &status, 0);
    }

    return -1;
}