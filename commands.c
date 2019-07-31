#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>
#include "history.h"

#ifndef TOKEN_BUFFER
    #define TOKEN_BUFFER 32
#endif

#define PATH_BUFFER 1024

enum Commands{changeDirectory, stop, external, history};
char oldWorkingDirectory[PATH_BUFFER] = {0};
char currentWorkingDirectory[PATH_BUFFER];
char prevWorkingDirectory[PATH_BUFFER];
bool previousCD = false;
struct history* his = NULL;

/*
 *  No flags to implement for smash1
 * 
 *  This is processing flags, which means the token starts with the '-' and is more than 1 character long.
 *  always start processing characters from [1] rather than [0]
 */
char* processChDirFlag(char* token, char* currentFlags, int currentNumFlags){
    int i;
    int j; 
    int nextFlag = 0;
    bool isDuplicateFlag;
    char* retVal = malloc(TOKEN_BUFFER);
    *retVal = 0;

    if (strlen(token) > (TOKEN_BUFFER - currentNumFlags)){
        retVal = "errorB";
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

    return retVal;
}

int doChangeDirectory(char* inputDirectory, char* flags){
    int exitStatus = 0;

    int cdErrNum;
    const char* targetDirectory;
    char targetFullDirectory[PATH_BUFFER] = {0};

    if (strlen(flags) > 1){
        fprintf(stderr, "Flags not implemented.\n");
        fprintf(stderr, "Usage: cd [directory path]\n");
        exitStatus = 1;
        return exitStatus;
    }

    if (inputDirectory == NULL){
       if ((targetDirectory = getenv("HOME")) == NULL) {
           targetDirectory = getpwuid(getuid())->pw_dir;
       }
    }
    else{
        if (strcmp(inputDirectory, "~") == 0){
            if ((targetDirectory = getenv("HOME")) == NULL){
                targetDirectory = getpwuid(getuid())->pw_dir;
            }
        } 
        else if (strcmp(inputDirectory, "-") == 0){
            targetDirectory = oldWorkingDirectory;
        }
        else{
            targetDirectory = inputDirectory;
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
        strncpy(targetFullDirectory, currentWorkingDirectory, strlen(currentWorkingDirectory));
        strncat(targetFullDirectory, "/", strlen("/"));
        if (inputDirectory[0] == '/'){
            inputDirectory++;
        }
        strncat(targetFullDirectory, inputDirectory, strlen(inputDirectory));
        cdErrNum = chdir(targetFullDirectory);
    }
    if (cdErrNum == 0){
        strncpy(oldWorkingDirectory, prevWorkingDirectory, strlen(prevWorkingDirectory) + 1);

        getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory));
        getcwd(prevWorkingDirectory, sizeof(prevWorkingDirectory));

        printf("%s\n", currentWorkingDirectory);
        previousCD = true;
    }
    else{
        if (inputDirectory[0] == '/'){
                inputDirectory++;
        }
        perror(inputDirectory);
        exitStatus = 1;
    }

    memset(targetFullDirectory, 0, strlen(targetFullDirectory));
    return exitStatus;
}

int doExternal(char** tokens, int numTokens){
    int i;
    for (i = 0; i < numTokens; i++){
        printf("[%d] %s\n", i, tokens[i]);
    }

    return 127;
}

void doExit(char** flags, int numFlags){
    if (numFlags != 0){
        printf("smash: exit: %s -- invalid argument\n", flags[0]);
        exit(255);
    }
    exit(0);
}


void executeCommand(char* userInputTokens){
    // copy the whole string for history
    char historyCommand[MAXLINE];
    strncpy(historyCommand, userInputTokens, strlen(userInputTokens) + 1);

    int commandExitStatus = 0;
    char* token = strtok(userInputTokens, " ");
    bool stopCheckingTokens = false;
    bool firstArg = true;
    int currentNumFlags = 0;
    const char* internalCommands[3] = {"cd", "history", "exit"};
    enum Commands command;
    char flagsToProcess[TOKEN_BUFFER] = {0};
    char* getFlags = calloc(1, sizeof(char*));
    *getFlags = 0;
    char* targetDirectory = NULL;
    char* externalFlags[TOKEN_BUFFER];

    // only initialize once
    if (his == NULL){
        his = init_history(MAXHISTORY);
    }
    // real cd bash stops checking args once it has a directory
    while (token != NULL && !stopCheckingTokens) {
        if (firstArg){
            if (strcmp(internalCommands[0], token) == 0){
                command = changeDirectory;
            }
            else if (strcmp(internalCommands[1], token) == 0){
                command = history;
            }
            else if (strcmp(internalCommands[2], token) == 0){
                command = stop;
            }
            else{
                command = external;
                externalFlags[currentNumFlags] = token;
                currentNumFlags++;
            }

            firstArg = false;
        }
        else{
            switch (command){
                case changeDirectory:
                    // check strlen on '-', since "cd -" prints the directory above the current, then moves there
                    if (token[0] == '-' && strlen(token) > 1){
                        getFlags = processChDirFlag(token, flagsToProcess, currentNumFlags);
                        if (strcmp(getFlags, "errorB") == 0){
                            printf("Error: too many arguments/arguments too long.\n");
                            return; // RETURNS VOID (as opposed to exitStatus), SO PROBABLY PRINT ERROR?
                        }
                        else if (strlen(flagsToProcess) == 0){
                            strncpy(flagsToProcess, getFlags, strlen(getFlags));
                        }
                        else{
                            strncat(flagsToProcess, getFlags, strlen(getFlags));
                        }
                        currentNumFlags = strlen(flagsToProcess);
                    }
                    else if (targetDirectory == NULL){
                        targetDirectory = token;
                        stopCheckingTokens = true;
                    }
                    break;
                case external:
                    externalFlags[currentNumFlags] = token;
                    currentNumFlags++;
                    break;
                case history:
                    stopCheckingTokens = true;
                    break;

                case stop:
                    externalFlags[currentNumFlags] = token;
                    currentNumFlags++;
                    stopCheckingTokens = true;
                    break;
            }
        }

        token = strtok(NULL, " ");
    }

    // wait for while loop to finish, then call the appropriate function
    switch (command){
        case (changeDirectory):
            commandExitStatus = doChangeDirectory(targetDirectory, flagsToProcess);
            break;
        case (external):
            commandExitStatus =  doExternal(externalFlags, currentNumFlags);
            break;
        case (history):
            commandExitStatus = 0;
            add_history(his, historyCommand, commandExitStatus);
            print_history(his);
            break;
        case (stop):
            clear_history(his);
            doExit(externalFlags, currentNumFlags);
            break;
    }

    if (command != history){
        add_history(his, historyCommand, commandExitStatus);
    }

    memset(historyCommand, 0, MAXLINE);
    memset(flagsToProcess, 0, TOKEN_BUFFER);

    return;
}