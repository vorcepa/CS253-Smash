#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <pthread.h>

#include "history.h"
#include "doCommand.h"
#include "smash.h"

#ifndef TOKEN_BUFFER
    #define TOKEN_BUFFER 1024
#endif

#define PATH_BUFFER 1024

struct processList jobs = {NULL, 0, 0};

void* threadWaitpid(void* arg){


    return NULL;
}

void enqueueJob(pid_t processID){
    if (jobs.processID == NULL){
        jobs.capacity = 10;
        jobs.processID = malloc(jobs.capacity * sizeof(pid_t));
    }
    else if (jobs.numProcesses >= jobs.capacity){
        jobs.capacity *= 2;
        jobs.processID = realloc(jobs.processID, jobs.capacity * sizeof(pid_t));
    }

    jobs.processID[jobs.numProcesses] = processID;
    jobs.numProcesses++;
}

void resetJobs(void){
    jobs.numProcesses = 0;
    if(jobs.capacity > 16){
        jobs.capacity = 16;
        jobs.processID = realloc(jobs.processID, jobs.capacity * sizeof(pid_t));
    }
}

char oldWorkingDirectory[PATH_BUFFER] = {0};
char currentWorkingDirectory[PATH_BUFFER];
char prevWorkingDirectory[PATH_BUFFER];
extern struct history* his;
bool previousCD = false;

/*
 *  No flags to implement for smash1
 *
 *  This is processing flags, which means the token starts with the '-' and is more than 1 character long.
 *  always start processing characters from [1] rather than [0]
 */
char* processChDirFlag(char* token, char* currentFlags, long unsigned int currentNumFlags){
    int i;
    int j;
    int nextFlag = 0;
    bool isDuplicateFlag;
    char* retVal = malloc(TOKEN_BUFFER);
    *retVal = 0;

    if (strlen(token) > (TOKEN_BUFFER - currentNumFlags)){
        strncpy(retVal, "errorB", TOKEN_BUFFER);
    }
    else{
        for (i = 1; i < strlen(token); i++){
            isDuplicateFlag = false;
            for (j = 0; j < strlen(currentFlags); j++){
                if (currentFlags[j] == token[i]){
                    isDuplicateFlag = true;
                    break;
                }
            }
            for (j = 0; j < strlen(retVal); j++){
                if (retVal[j] == token[i]){
                    isDuplicateFlag = true;
                    break;
                }
            }
            if (!isDuplicateFlag){
                retVal[nextFlag] = token[i];
                nextFlag++;
            }
        }
    }

    retVal[strlen(retVal)] = '\0';

    return retVal;
}

int doChangeDirectory(char** args){
    int exitStatus = 0;
    char* directoryArg = args[1];

    int cdErrNum;
    const char* targetDirectory;
    char targetFullDirectory[PATH_BUFFER] = {0};

    if (directoryArg == NULL){
       if ((targetDirectory = getenv("HOME")) == NULL) {
           targetDirectory = getpwuid(getuid())->pw_dir;
       }
    }
    else{
        if (strcmp(directoryArg, "~") == 0){
            if ((targetDirectory = getenv("HOME")) == NULL){
                targetDirectory = getpwuid(getuid())->pw_dir;
            }
        }
        else if (strcmp(directoryArg, "-") == 0){
            targetDirectory = oldWorkingDirectory;
        }
        else{
            targetDirectory = directoryArg;
        }
    }
    // for "cd -"; roundabout way to stop this from working if cd has not been called before.  $OLDPWD is not set
    // until cd is called once
    if (!previousCD){
        getcwd(prevWorkingDirectory, sizeof(prevWorkingDirectory));
    }
    // make the directory change, and store the error code (0 is success)
    cdErrNum = chdir(targetDirectory);
    // if the attempt to change directory failed, try again, but concatenating the inputDirectory to the end of the current working directory
    if (cdErrNum != 0){
        strncpy(targetFullDirectory, currentWorkingDirectory, PATH_BUFFER);
        strncat(targetFullDirectory, "/", strlen("/"));
        if (directoryArg[0] == '/'){
            directoryArg++;
        }
        strncat(targetFullDirectory, directoryArg, strlen(directoryArg));
        cdErrNum = chdir(targetFullDirectory);
    }
    if (cdErrNum == 0){
        strncpy(oldWorkingDirectory, prevWorkingDirectory, PATH_BUFFER);

        getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory));
        getcwd(prevWorkingDirectory, sizeof(prevWorkingDirectory));

        printf("%s\n", currentWorkingDirectory);
        previousCD = true;
    }
    else{
        if (directoryArg[0] == '/'){
                directoryArg++;
        }
        perror(directoryArg);
        exitStatus = 1;
    }

    memset(targetFullDirectory, 0, strlen(targetFullDirectory));
    return exitStatus;
}

/*
 * Separates each command by each pipe symbol.
 *
 * @param userInput: the unmodified user input string. This is the string
 * that will be searched through to find pipes for separation.
 * @param numCommands: the number of commands in userInput, equal to n + 1 pipe symbols.
 *
 * @return an array of strings, where each string is one command.
 */
char** pipeTokenizer(char* userInput, int numCommands){
    char** retVal = calloc(numCommands, sizeof(char*));
    int i;
    char* token;

    token = strtok(userInput, "|");

    i = 0;
    while (token != NULL){
        retVal[i] = token;
        i++;
        token = strtok(NULL, "|");
    }

    return retVal;
}

/**
 * Separates each argument within a single command in to its own token.
 * This function is called after each command is separated and tokenized by pipe.
 *
 * @param command: one command, a subset of the user input
 * @delimiter: the string that the tokenizer is going to use to separate the command
 * in to its constituent arguments
 *
 * @return an array of strings representing the command now separated in to individual
 * arguments
 */
char** commandTokenizer(char* command, char* delimiter){
    int i = 0;
    char** retVal = calloc(TOKEN_BUFFER, sizeof(char*));
    char* token = strtok(command, delimiter);

    while (token != NULL){
        retVal[i] = token;
        i++;

        token = strtok(NULL, delimiter);
    }

    return retVal;
}

/**
 * Checks the tokenized arguments for an input/output redirect.  By default,
 * the I/O is stdin and stdout.  If the user specifies a different input or output
 * (from or to a file, respectively), the read/write stream is directed to that file
 * instead of the default.
 *
 * @param argv: array of arguments in a single command; these are the values that are going
 * to be looked through to find a redirect symbol ('<'/'>')
 * @param numArgs: the number of elements in argv.
 */
void getIOFileDescriptors(char** argv, int redirects[2]){
    int numArgs = 0;
    for (int i = 0; argv[i] != NULL; i++) {
	    numArgs++;
    }

    char* inputRedirect = NULL;
    char* outputRedirect = NULL;

    redirects[0] = STDIN_FILENO;
    redirects[1] = STDOUT_FILENO;

    int i;
    for (i = 0; i < numArgs; i++){
        switch (argv[i][0])
        {
        case '<':
            inputRedirect = argv[i] + 1;
            memmove(&argv[i], &argv[i+1], (numArgs - i) * sizeof(char*));
            numArgs--;
            i--;
            break;

        case '>':
            outputRedirect = argv[i] + 1;
            memmove(&argv[i], &argv[i+1], (numArgs - i) * sizeof(char*));
            numArgs--;
            i--;
            break;
        }
    }

    if(inputRedirect != NULL){
        redirects[0] = open(inputRedirect, O_RDONLY);
    }
    if (outputRedirect != NULL){
        redirects[1] = open(outputRedirect, O_CREAT|O_TRUNC|O_WRONLY);
    }

    return;
}

/**
 * Takes the user input and calls the functions needed to process it.  A parser is called
 * to separate the commands by pipe symbols.  For each command separated in this fashion,
 * it is further separated by each argument, separated by a space.  These are stored in memory,
 * which this function then passes on to the function that ultimately executes the command.
 *
 * In addition, the full line of the user input is copied.  The parsers change the original string,
 * so this copy is stored so that it may be listed in the history command.
 *
 * @param userInput: the current line written to the stream to be processed.  Can be from
 * typing in to the console, or received from a file stream.
 */
int processCommand(char* userInput){
    while (userInput[0] == ' ' || userInput[0] == '\t'){
        userInput++;
    }
    if (strlen(userInput) == 0){
        return 0;
    }

    char delimiter[3] = " \t";
    int i;
    int numCommands = 1;
    for (i = 0; i < strlen(userInput); i++){
        if (userInput[i] == '|'){
            numCommands++;
        }
    }
    int redirects[2];
    int fds[3] = {STDIN_FILENO, 0, -1};
    pid_t processID;

    // copy the whole string for history
    char historyCommand[MAXLINE];
    strncpy(historyCommand, userInput, strlen(userInput) + 1);

    char** commands = pipeTokenizer(userInput, numCommands);
    char*** tokenizedCommands = calloc(TOKEN_BUFFER, sizeof(char**));

    for (i = 0; i < numCommands; i++){
        tokenizedCommands[i] = commandTokenizer(commands[i], delimiter);
    }

    for (int i = 0; i < numCommands - 1; i++) {
        getIOFileDescriptors(tokenizedCommands[i], redirects);
        if (redirects[0] != STDIN_FILENO) {
            if (fds[0] != STDIN_FILENO) {
                close(fds[0]);
            }
            fds[0] = redirects[0];
        }
        pipe(fds+1);
        int tmp = fds[1];
        fds[1] = fds[2];
        fds[2] = tmp;

        if (redirects[1] != STDOUT_FILENO) {
            close(fds[1]);
            fds[1] = redirects[1];
        }

        processID = doCommand(tokenizedCommands[i], fds);
        enqueueJob(processID);
        if (fds[0] != STDIN_FILENO) {
            close(fds[0]);
        }
        close(fds[1]);
        fds[0] = fds[2]; // pipe output is next input
    }

    getIOFileDescriptors(tokenizedCommands[numCommands-1], redirects);
    if (redirects[0] != STDIN_FILENO) {
	if (fds[0] != STDIN_FILENO) {
	    close(fds[0]);
	}
        fds[0] = redirects[0];
    }
    fds[1] = redirects[1];
    fds[2] = -1;
    processID = doCommand(tokenizedCommands[numCommands-1], fds);
    enqueueJob(processID);
    if (fds[0] != STDIN_FILENO) {
	    close(fds[0]);
    }
    if (fds[1] != STDOUT_FILENO) {
        close(fds[1]);
    }

    for (i = 0; i < numCommands; i++) {
	    free(tokenizedCommands[i]);
    }
    free(tokenizedCommands);
    free(commands);

    int status;
    // waitpid(processID, &status, 0);
    add_history(his, historyCommand, status);
    resetJobs();
    return status;
}
