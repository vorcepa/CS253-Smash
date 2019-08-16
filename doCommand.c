#define _POSIX_SOURCE
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <pthread.h>

#include "history.h"
#include "smash.h"

#ifndef TOKEN_BUFFER
    #define TOKEN_BUFFER 1024
#endif

extern struct history* his;
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
pid_t executeExternal(char** args, int fileDescriptors[3]){
    fflush(stdout);
    pid_t processID = fork();

    if (processID != 0){
        return processID;
    }
    signal(SIGINT, SIG_DFL);

    if (fileDescriptors[0] != STDIN_FILENO){
        dup2(fileDescriptors[0], STDIN_FILENO);
        close(fileDescriptors[0]);
    }
    if (fileDescriptors[1] != STDOUT_FILENO){
        dup2(fileDescriptors[1], STDOUT_FILENO);
        close(fileDescriptors[1]);
    }
    if (fileDescriptors[2] >= 0) {
	close(fileDescriptors[2]);
    }

    exit(execvp(args[0], args));

}

/**
 * Determines if the command to be executed is an internal or external command
 * to be executed.  Sends the appropriate information to the appropriate function for execution.
 *
 * @param args: the arguments consisting of the command to be executed, and any other
 * arguments and flags associated with it
 *
 *
 */
pid_t doCommand(char** args, int fileDescriptors[3]){
    // pthread_t threadID;

    if (strcmp(args[0], "cd") == 0){
        doChangeDirectory(args);
    }
    else if (strcmp(args[0], "history") == 0){
        print_history(his);
    }
    else if (strcmp(args[0], "exit") == 0){
        exit(0);
    }
    else{
        // int status;
        // pid_t pid;

        // pid = wait(&status);
        // fprintf(stderr, "%d exited, status = %d\n", pid, status);
        return executeExternal(args, fileDescriptors);
    }

    return -1;
}
