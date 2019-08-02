#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "history.h"

struct history* init_history(int capacity){
    struct history* retVal = malloc(sizeof(struct history));
    retVal->capacity = capacity;
    retVal->offset = 0;
    retVal->entries = calloc((capacity), sizeof(*retVal->entries)); /*
                                                                          * assign the entries member to the value of the pointer returned by malloc.
                                                                          * *retVal->entries dereferences retVal (via ->) and then dereferences
                                                                          * entries (via *).  *entries == cmd*
                                                                          */
    return retVal;
}

int print_history(struct history* toPrint){
    int i;

    for (i = (toPrint->offset) - 1; i >= 0; i--){
        printf("%d [%d] %s\n", i + 1, toPrint->entries[i]->exitStatus, toPrint->entries[i]->cmd);
        fflush(stdout);
    }

    return 0;
}

void add_history(struct history* history, char* command, int exitStatus){
    int i;

    struct cmd* cmdToAdd = malloc(sizeof(*cmdToAdd));
    cmdToAdd->pid = 0;
    cmdToAdd->exitStatus = exitStatus;
    cmdToAdd->cmd = malloc(MAXLINE);
    strncpy(cmdToAdd->cmd, command, MAXLINE);

    if (history->offset > MAXHISTORY){
        free(history->entries[MAXHISTORY]->cmd);
        free(history->entries[MAXHISTORY]);

        history->entries[MAXHISTORY] = calloc(1, sizeof(history->entries[0]));
        history->entries[MAXHISTORY] = history->entries[MAXHISTORY - 1];
        history->entries[MAXHISTORY]->cmd = calloc(1, sizeof(history->entries[0]->cmd));
        history->entries[MAXHISTORY]->cmd = history->entries[MAXHISTORY - 1]->cmd;
        history->offset--;
    }

    for (i = history->offset; i > 0; i--){
        history->entries[i] = history->entries[i-1];
    }
    history->entries[0] = cmdToAdd;

    history->offset++;
}

void clear_history(struct history* history){
    int i;
    for (i = 0; history->entries[i] == 0x0; i++){
        free(history->entries[i]->cmd);
        history->entries[i]->cmd = NULL;
        free(history->entries[i]);
        history->entries[i] = NULL;
    }

    free(history->entries);
    free(history);
}