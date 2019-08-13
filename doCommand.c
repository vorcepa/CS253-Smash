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

/**
 * Forks a new process to execute a command that is not built-in to the smash program.
 * 
 * @param args: the arguments consisting of the command to be executed, and any other
 * arguments and flags associated with it
 * @param inputFD: the input file descriptor.  By default is set to stdin, but if the user
 * specified a file, it may be redirected to some other descriptor.
 * @param outputFD: the output file descriptor.  By default is set to stdout, but if the user
 * specified a file, it may be redirected to some other descriptor.
 * 
 * @return the processID returned by the fork() call; this will be a child process of the smash
 * program
 */
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

/**
 * Determines if the command to be executed is an internal or external command
 * to be executed.  Sends the appropriate information to the appropriate function for execution.
 * 
 * @param args: the arguments consisting of the command to be executed, and any other
 * arguments and flags associated with it
 * @param inputFD: the input file descriptor.  By default is set to stdin, but if the user
 * specified a file, it may be redirected to some other descriptor.
 * @param outputFD: the output file descriptor.  By default is set to stdout, but if the user
 * specified a file, it may be redirected to some other descriptor.
 * 
 * @return the exit code after the command has completed.  0 for a clean execution; any other number
 * represents an error.
 */
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