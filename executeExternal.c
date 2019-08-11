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

int executeExternalCommand(const char* externalCommand, char* const flags[]){
    fflush(stdout);
    int processID = fork();

    if (processID == 0){
        int childExitStatus;
        childExitStatus = execvp(externalCommand, flags);
        exit(childExitStatus);
    }
    else{
        fflush(stdout);
        int exitStatus;
        wait(&exitStatus);

        if (exitStatus == 65280){
            printf("smash: %s: command not found", externalCommand);
        }

        return exitStatus;
    }

    return -1;
}