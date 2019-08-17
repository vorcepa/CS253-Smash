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
 * Called by processCommands() in commands.c.  This function first checks if the command is exit.  If so,
 * nothing else needs to be done -> exit smash.  Otherwise, the next step is to fork a new process, regardless
 * if its an internal (e.g. cd, history) or external command (ls, pwd, wc, etc.).  After the fork, the parent process
 * is returned.  A signal handler is made since CTRL+C shouldn't exit smash while a command is currently in execution.
 * Next, the file descriptors are checked against STDIN and STDOUT; if they're different, then there's a pipe, and we 
 * set the pipe to read in/out, and close the file descriptors here, since we won't get another chance to.  After that,
 * We determine which command is being executed (cd, history, or some external command).  The appropriate function is called,
 * and all this function needs to do is exit when that process completes - the behaviour of that execution is handled
 * elsewhere.
 * 
 * @param args: a single command within the userinput, tokenized such that the first argument is the command itself,
 * and the following arguments may be a necessary argument, and any optional flags.  Redirect arguments have been
 * filtered out of the user input at this point.
 * @param fileDescriptors: the int values associated with files to be read and written to.  fileDescriptors[0] replaces
 * STDIN to be read from, fileDescriptors[1] replaces STDOUT to write to, and fileDescriptors[2] is a carry-over from
 * processCommands(), where it acts as the pipe for the *next* command after this one.  We simply close the descriptor here.
 */
pid_t doCommand(char** args, int fileDescriptors[3]){
    if (strcmp(args[0], "exit") == 0){
        exit(0);
    }

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

    if (strcmp(args[0], "cd") == 0){
        exit(doChangeDirectory(args));
    }
    else if (strcmp(args[0], "history") == 0){
        exit(print_history(his));
    }
    else{
        exit(execvp(args[0], args));
    }
}
